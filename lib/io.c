/*
 *	BIRD Internet Routing Daemon -- Unix I/O
 *
 *	(c) 1998--2004 Martin Mares <mj@ucw.cz>
 *      (c) 2004       Ondrej Filip <feela@network.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/* Unfortunately, some glibc versions hide parts of RFC 3542 API
   if _GNU_SOURCE is not defined. */
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/icmp6.h>

#include "nest/bird.h"
#include "lib/list.h"
#include "lib/resource.h"
#include "lib/timer.h"
#include "lib/socket.h"
#include "lib/event.h"
#include "lib/string.h"
#include "nest/iface.h"

#include "lib/unix.h"
#include "lib/sysio.h"
#include "conf/conf.h"

#include "libubox/usock.h"

/* Maximum number of calls of tx handler for one socket in one
 * poll iteration. Should be small enough to not monopolize CPU by
 * one protocol instance.
 */
#define MAX_STEPS 4

/* Maximum number of calls of rx handler for all sockets in one poll
   iteration. RX callbacks are often much more costly so we limit
   this to gen small latencies */
#define MAX_RX_STEPS 4

#define HAVE_UBOX
#define EVENT_LOOPTIME 1000 /* ms */

#define EVENT_LOG_LENGTH 32


static int sk_read(struct birdsock *s, int revents);
static int sk_write(struct birdsock *s);
void io_log_event(void *hook, void *data);

struct event_log_entry {
	void *hook;
	void *data;
	btime timestamp;
	btime duration;
};

static struct event_log_entry event_log[EVENT_LOG_LENGTH];
static struct event_log_entry *event_open;
static int event_log_pos, event_log_num;
static btime last_time;
static struct list_head ospf_sk_list;


/*
 *	Tracked Files
 */

struct rfile {
	struct resource r;
	FILE *f;
};

static void rf_free(struct resource *r)
{
	struct rfile *a = (struct rfile *)r;

	fclose(a->f);
}

static void rf_dump(struct resource *r)
{
	struct rfile *a = (struct rfile *)r;

	debug("(FILE *%p)\n", a->f);
}

static struct resclass rf_class = {
	"FILE",
	sizeof(struct rfile),
	rf_free,
	rf_dump,
	NULL,
	NULL
};

void *tracked_fopen(struct pool *p, char *name, char *mode)
{
	FILE *f = fopen(name, mode);

	if (f) {
		struct rfile *r = ralloc(p, &rf_class);
		r->f = f;
	}
	return f;
}

/**
 * DOC: Timers
 *
 * Timers are resources which represent a wish of a module to call
 * a function at the specified time. The platform dependent code
 * doesn't guarantee exact timing, only that a struct timer function
 * won't be called before the requested time.
 *
 * In BIRD, time is represented by values of the &bird_clock_t type
 * which are integral numbers interpreted as a relative number of seconds since
 * some fixed time point in past. The current time can be read
 * from variable @now with reasonable accuracy and is monotonic. There is also
 * a current 'absolute' time in variable @now_real reported by OS.
 *
 * Each struct timer is described by a &timer structure containing a pointer
 * to the handler function (@hook), data private to this function (@data),
 * time the function should be called at (@expires, 0 for inactive timers),
 * for the other fields see |timer.h|.
 */

bird_clock_t now = 1, now_real, boot_time;

static inline void update_times(void)
{
	struct timespec ts;
	int rv;

	rv = clock_gettime(CLOCK_MONOTONIC, &ts);
	if (rv != 0)
		die("clock_gettime: %m");


	if (ts.tv_sec != now) {
		if (ts.tv_sec < now)
			log(L_ERR "Monotonic struct timer is broken");

		now = ts.tv_sec;
		now_real = time(NULL);
	}

	last_time = ((s64) ts.tv_sec S) + (ts.tv_nsec / 1000);
	if (event_open) {
		event_open->duration = last_time - event_open->timestamp;

		if (event_open->duration > config->latency_limit)
			log(L_WARN "Event 0x%p 0x%p took %d ms",
			    event_open->hook, event_open->data,
			    (int)(event_open->duration TO_MS));

		event_open = NULL;
	}
}

static void tm_free(struct resource *r)
{
	struct timer *t = (struct timer *)r;

	tm_stop(t);
}

static void tm_dump(struct resource *r)
{
	struct timer *t = (struct timer *)r;

	debug("(code %p, data %p, ", t->hook, t->data);
	if (t->randomize)
		debug("rand %d, ", t->randomize);
	if (t->recurrent)
		debug("recur %d, ", t->recurrent);
	if (t->expires)
		debug("expires in %d sec)\n", t->expires - now);
	else
		debug("inactive)\n");
}

static struct resclass tm_class = {
	"Timer",
	sizeof(struct timer),
	tm_free,
	tm_dump,
	NULL,
	NULL
};

/**
 * tm_new - create a timer
 * @p: pool
 *
 * This function creates a new struct timer struct resource and returns
 * a pointer to it. To use the timer, you need to fill in
 * the structure fields and call tm_start() to start timing.
 */
struct timer *tm_new(struct pool *p)
{
	struct timer *t = ralloc(p, &tm_class);
	return t;
}

static void tm_cb(struct uloop_timeout *timeout)
{
	struct timer *t = container_of(timeout, struct timer, timeout);

	update_times();

	if (t->recurrent){
		uloop_timeout_set(&t->timeout, t->recurrent * 1000);
	}
	io_log_event(t->hook, t->data);
	t->hook(t);
}

/**
 * tm_start - start a timer
 * @t: timer
 * @after: number of seconds the struct timer should be run after
 *
 * This function schedules the hook function of the struct timer to
 * be called after @after seconds. If the struct timer has been already
 * started, it's @expire time is replaced by the new value.
 *
 * You can have set the @randomize field of @t, the timeout
 * will be increased by a random number of seconds chosen
 * uniformly from range 0 .. @randomize.
 *
 * You can call tm_start() from the handler function of the timer
 * to request another run of the timer. Also, you can set the @recurrent
 * field to have the struct timer re-added automatically with the same timeout.
 */
void tm_start(struct timer *t, unsigned after)
{
	uloop_timeout_set(&t->timeout, after * 1000);
	t->timeout.cb = tm_cb;
}

/**
 * tm_stop - stop a timer
 * @t: timer
 *
 * This function stops a timer. If the struct timer is already stopped,
 * nothing happens.
 */
void tm_stop(struct timer *t)
{
	uloop_timeout_cancel(&t->timeout);
}

void tm_dump_all(void)
{
	debug("not support\n");
}


/**
 * tm_parse_datetime - parse a date and time
 * @x: datetime string
 *
 * tm_parse_datetime() takes a textual representation of
 * a date and time (dd-mm-yyyy hh:mm:ss)
 * and converts it to the corresponding value of type &bird_clock_t.
 */
bird_clock_t tm_parse_datetime(char *x)
{
	struct tm tm;
	int n;
	time_t t;

	if (sscanf
	    (x, "%d-%d-%d %d:%d:%d%n", &tm.tm_mday, &tm.tm_mon, &tm.tm_year,
	     &tm.tm_hour, &tm.tm_min, &tm.tm_sec, &n) != 6 || x[n])
		return tm_parse_date(x);
	tm.tm_mon--;
	tm.tm_year -= 1900;
	t = mktime(&tm);
	if (t == (time_t) - 1)
		return 0;
	return t;
}

/**
 * tm_parse_date - parse a date
 * @x: date string
 *
 * tm_parse_date() takes a textual representation of a date (dd-mm-yyyy)
 * and converts it to the corresponding value of type &bird_clock_t.
 */
bird_clock_t tm_parse_date(char *x)
{
	struct tm tm;
	int n;
	time_t t;

	if (sscanf(x, "%d-%d-%d%n", &tm.tm_mday, &tm.tm_mon, &tm.tm_year, &n) !=
	    3 || x[n])
		return 0;
	tm.tm_mon--;
	tm.tm_year -= 1900;
	tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
	t = mktime(&tm);
	if (t == (time_t) - 1)
		return 0;
	return t;
}

static void tm_format_reltime(char *x, struct tm *tm, bird_clock_t delta)
{
	static char *month_names[12] =
	    { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	if (delta < 20 * 3600)
		bsprintf(x, "%02d:%02d", tm->tm_hour, tm->tm_min);
	else if (delta < 360 * 86400)
		bsprintf(x, "%s%02d", month_names[tm->tm_mon], tm->tm_mday);
	else
		bsprintf(x, "%d", tm->tm_year + 1900);
}


/**
 * tm_format_datetime - convert date and time to textual representation
 * @x: destination struct buffer of size %TM_DATETIME_BUFFER_SIZE
 * @fmt_spec: specification of resulting textual representation of the time
 * @t: time
 *
 * This function formats the given relative time value @t to a textual
 * date/time representation (dd-mm-yyyy hh:mm:ss) in real time.
 */
void tm_format_datetime(char *x, struct timeformat *fmt_spec, bird_clock_t t)
{
	const char *fmt_used;
	struct tm *tm;
	bird_clock_t delta = now - t;
	t = now_real - delta;
	tm = localtime(&t);

	if (fmt_spec->fmt1 == NULL)
		return tm_format_reltime(x, tm, delta);

	if ((fmt_spec->limit == 0) || (delta < fmt_spec->limit))
		fmt_used = fmt_spec->fmt1;
	else
		fmt_used = fmt_spec->fmt2;

	int rv = strftime(x, TM_DATETIME_BUFFER_SIZE, fmt_used, tm);
	if (((rv == 0) && fmt_used[0]) || (rv == TM_DATETIME_BUFFER_SIZE))
		strcpy(x, "<too-long>");
}

/**
 * DOC: Sockets
 *
 * Socket resources represent network connections. Their data structure (&socket)
 * contains a lot of fields defining the exact type of the socket, the local and
 * remote addresses and ports, pointers to socket buffers and finally pointers to
 * hook functions to be called when new data have arrived to the receive buffer
 * (@rx_hook), when the contents of the transmit struct buffer have been transmitted
 * (@tx_hook) and when an error or connection close occurs (@err_hook).
 *
 * Freeing of sockets from inside socket hooks is perfectly safe.
 */

#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#endif

#ifndef SOL_IPV6
#define SOL_IPV6 IPPROTO_IPV6
#endif

#ifndef SOL_ICMPV6
#define SOL_ICMPV6 IPPROTO_ICMPV6
#endif

/*
 *	Sockaddr helper functions
 */

static inline int sockaddr_length(int af)
{
	return (af == AF_INET)
		? sizeof(struct sockaddr_in)
		: sizeof(struct sockaddr_in6);
}

static inline void
sockaddr_fill4(struct sockaddr_in *sa, ip_addr a, struct iface *ifa, uint port)
{
	memset(sa, 0, sizeof(struct sockaddr_in));
#ifdef HAVE_SIN_LEN
	sa->sin_len = sizeof(struct sockaddr_in);
#endif
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	sa->sin_addr = ipa_to_in4(a);
}

static inline void
sockaddr_fill6(struct sockaddr_in6 *sa, ip_addr a, struct iface *ifa, uint port)
{
	memset(sa, 0, sizeof(struct sockaddr_in6));
#ifdef SIN6_LEN
	sa->sin6_len = sizeof(struct sockaddr_in6);
#endif
	sa->sin6_family = AF_INET6;
	sa->sin6_port = htons(port);
	sa->sin6_flowinfo = 0;
	sa->sin6_addr = ipa_to_in6(a);

	if (ifa && ipa_is_link_local(a))
		sa->sin6_scope_id = ifa->index;
}

void
sockaddr_fill(struct sockaddr_bird * sa, int af, ip_addr a, struct iface *ifa, uint port)
{
	if (af == AF_INET)
		sockaddr_fill4((struct sockaddr_in *)sa, a, ifa, port);
	else if (af == AF_INET6)
		sockaddr_fill6((struct sockaddr_in6 *)sa, a, ifa, port);
	else
		bug("Unknown AF");
}

static inline void
sockaddr_read4(struct sockaddr_in *sa, ip_addr * a, struct iface **ifa,
	       uint * port)
{
	*port = ntohs(sa->sin_port);
	*a = ipa_from_in4(sa->sin_addr);
}

static inline void
sockaddr_read6(struct sockaddr_in6 *sa, ip_addr * a, struct iface **ifa,
	       uint * port)
{
	*port = ntohs(sa->sin6_port);
	*a = ipa_from_in6(sa->sin6_addr);

	if (ifa && ipa_is_link_local(*a))
		*ifa = if_find_by_index(sa->sin6_scope_id);
}

int
sockaddr_read(struct sockaddr_bird * sa, int af, ip_addr * a, struct iface **ifa,
	      uint * port)
{
	if (sa->sa.sa_family != af)
		goto fail;

	if (af == AF_INET)
		sockaddr_read4((struct sockaddr_in *)sa, a, ifa, port);
	else if (af == AF_INET6)
		sockaddr_read6((struct sockaddr_in6 *)sa, a, ifa, port);
	else
		goto fail;

	return 0;

fail:
	*a = IPA_NONE;
	*port = 0;
	return -1;
}

/*
 *	IPv6 multicast syscalls
 */

/* Fortunately standardized in RFC 3493 */

#define INIT_MREQ6(maddr,ifa) \
  { .ipv6mr_multiaddr = ipa_to_in6(maddr), .ipv6mr_interface = ifa->index }

static inline int sk_setup_multicast6(struct birdsock *s)
{
	int index = s->iface->index;
	int ttl = s->ttl;
	int n = 0;

	if (setsockopt
	    (s->fd, SOL_IPV6, IPV6_MULTICAST_IF, &index, sizeof(index)) < 0)
		ERR("IPV6_MULTICAST_IF");

	if (setsockopt(s->fd, SOL_IPV6, IPV6_MULTICAST_HOPS, &ttl, sizeof(ttl))
	    < 0)
		ERR("IPV6_MULTICAST_HOPS");

	if (setsockopt(s->fd, SOL_IPV6, IPV6_MULTICAST_LOOP, &n, sizeof(n)) < 0)
		ERR("IPV6_MULTICAST_LOOP");

	return 0;
}

static inline int sk_join_group6(struct birdsock *s, ip_addr maddr)
{
	struct ipv6_mreq mr = INIT_MREQ6(maddr, s->iface);

	if (setsockopt(s->fd, SOL_IPV6, IPV6_JOIN_GROUP, &mr, sizeof(mr)) < 0)
		ERR("IPV6_JOIN_GROUP");

	return 0;
}

static inline int sk_leave_group6(struct birdsock *s, ip_addr maddr)
{
	struct ipv6_mreq mr = INIT_MREQ6(maddr, s->iface);

	if (setsockopt(s->fd, SOL_IPV6, IPV6_LEAVE_GROUP, &mr, sizeof(mr)) < 0)
		ERR("IPV6_LEAVE_GROUP");

	return 0;
}

/*
 *	IPv6 packet control messages
 */

/* Also standardized, in RFC 3542 */

/*
 * RFC 2292 uses IPV6_PKTINFO for both the socket option and the cmsg
 * type, RFC 3542 changed the socket option to IPV6_RECVPKTINFO. If we
 * don't have IPV6_RECVPKTINFO we suppose the OS implements the older
 * RFC and we use IPV6_PKTINFO.
 */
#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif
/*
 * Same goes for IPV6_HOPLIMIT -> IPV6_RECVHOPLIMIT.
 */
#ifndef IPV6_RECVHOPLIMIT
#define IPV6_RECVHOPLIMIT IPV6_HOPLIMIT
#endif

#define CMSG6_SPACE_PKTINFO CMSG_SPACE(sizeof(struct in6_pktinfo))
#define CMSG6_SPACE_TTL CMSG_SPACE(sizeof(int))

static inline int sk_request_cmsg6_pktinfo(struct birdsock *s)
{
	int y = 1;

	if (setsockopt(s->fd, SOL_IPV6, IPV6_RECVPKTINFO, &y, sizeof(y)) < 0)
		ERR("IPV6_RECVPKTINFO");

	return 0;
}

static inline int sk_request_cmsg6_ttl(struct birdsock *s)
{
	int y = 1;

	if (setsockopt(s->fd, SOL_IPV6, IPV6_RECVHOPLIMIT, &y, sizeof(y)) < 0)
		ERR("IPV6_RECVHOPLIMIT");

	return 0;
}

static inline void sk_process_cmsg6_pktinfo(struct birdsock *s,
					    struct cmsghdr *cm)
{
	if (cm->cmsg_type == IPV6_PKTINFO) {
		struct in6_pktinfo *pi = (struct in6_pktinfo *)CMSG_DATA(cm);
		s->laddr = ipa_from_in6(pi->ipi6_addr);
		s->lifindex = pi->ipi6_ifindex;
	}
}

static inline void sk_process_cmsg6_ttl(struct birdsock *s, struct cmsghdr *cm)
{
	if (cm->cmsg_type == IPV6_HOPLIMIT)
		s->rcv_ttl = *(int *)CMSG_DATA(cm);
}

static inline void
sk_prepare_cmsgs6(struct birdsock *s, struct msghdr *msg, void *cbuf,
		  size_t cbuflen)
{
	struct cmsghdr *cm;
	struct in6_pktinfo *pi;
	int controllen = 0;

	msg->msg_control = cbuf;
	msg->msg_controllen = cbuflen;

	cm = CMSG_FIRSTHDR(msg);
	cm->cmsg_level = SOL_IPV6;
	cm->cmsg_type = IPV6_PKTINFO;
	cm->cmsg_len = CMSG_LEN(sizeof(*pi));
	controllen += CMSG_SPACE(sizeof(*pi));

	pi = (struct in6_pktinfo *)CMSG_DATA(cm);
	pi->ipi6_ifindex = s->iface ? s->iface->index : 0;
	pi->ipi6_addr = ipa_to_in6(s->saddr);

	msg->msg_controllen = controllen;
}

/*
 *	Miscellaneous socket syscalls
 */

static inline int sk_set_ttl4(struct birdsock *s, int ttl)
{
	if (setsockopt(s->fd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
		ERR("IP_TTL");

	return 0;
}

static inline int sk_set_ttl6(struct birdsock *s, int ttl)
{
	if (setsockopt(s->fd, SOL_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl)) <
	    0)
		ERR("IPV6_UNICAST_HOPS");

	return 0;
}

static inline int sk_set_tos4(struct birdsock *s, int tos)
{
	if (setsockopt(s->fd, SOL_IP, IP_TOS, &tos, sizeof(tos)) < 0)
		ERR("IP_TOS");

	return 0;
}

static inline int sk_set_tos6(struct birdsock *s, int tos)
{
	if (setsockopt(s->fd, SOL_IPV6, IPV6_TCLASS, &tos, sizeof(tos)) < 0)
		ERR("IPV6_TCLASS");

	return 0;
}

static inline int sk_set_high_port(struct birdsock *s)
{
	/* Port range setting is optional, ignore it if not supported */

#ifdef IP_PORTRANGE
	if (sk_is_ipv4(s)) {
		int range = IP_PORTRANGE_HIGH;
		if (setsockopt
		    (s->fd, SOL_IP, IP_PORTRANGE, &range, sizeof(range)) < 0)
			ERR("IP_PORTRANGE");
	}
#endif

#ifdef IPV6_PORTRANGE
	if (sk_is_ipv6(s)) {
		int range = IPV6_PORTRANGE_HIGH;
		if (setsockopt
		    (s->fd, SOL_IPV6, IPV6_PORTRANGE, &range,
		     sizeof(range)) < 0)
			ERR("IPV6_PORTRANGE");
	}
#endif

	return 0;
}

static inline byte *sk_skip_ip_header(byte * pkt, int *len)
{
	if ((*len < 20) || ((*pkt & 0xf0) != 0x40))
		return NULL;

	int hlen = (*pkt & 0x0f) * 4;
	if ((hlen < 20) || (hlen > *len))
		return NULL;

	*len -= hlen;
	return pkt + hlen;
}

byte *sk_rx_buffer(struct birdsock * s, int *len)
{
	if (sk_is_ipv4(s) && (s->type == SK_IP))
		return sk_skip_ip_header(s->rbuf, len);
	else
		return s->rbuf;
}

/*
 *	Public socket functions
 */

/**
 * sk_setup_multicast - enable multicast for given socket
 * @s: socket
 *
 * Prepare transmission of multicast packets for given datagram socket.
 * The socket must have defined @iface.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_setup_multicast(struct birdsock *s)
{
	ASSERT(s->iface);

	if (sk_is_ipv4(s))
		return sk_setup_multicast4(s);
	else
		return sk_setup_multicast6(s);
}

/**
 * sk_join_group - join multicast group for given socket
 * @s: socket
 * @maddr: multicast address
 *
 * Join multicast group for given datagram socket and associated interface.
 * The socket must have defined @iface.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_join_group(struct birdsock *s, ip_addr maddr)
{
	if (sk_is_ipv4(s))
		return sk_join_group4(s, maddr);
	else
		return sk_join_group6(s, maddr);
}

/**
 * sk_leave_group - leave multicast group for given socket
 * @s: socket
 * @maddr: multicast address
 *
 * Leave multicast group for given datagram socket and associated interface.
 * The socket must have defined @iface.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_leave_group(struct birdsock *s, ip_addr maddr)
{
	if (sk_is_ipv4(s))
		return sk_leave_group4(s, maddr);
	else
		return sk_leave_group6(s, maddr);
}

/**
 * sk_setup_broadcast - enable broadcast for given socket
 * @s: socket
 *
 * Allow reception and transmission of broadcast packets for given datagram
 * socket. The socket must have defined @iface. For transmission, packets should
 * be send to @brd address of @iface.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_setup_broadcast(struct birdsock *s)
{
	int y = 1;

	if (setsockopt(s->fd, SOL_SOCKET, SO_BROADCAST, &y, sizeof(y)) < 0)
		ERR("SO_BROADCAST");

	return 0;
}

/**
 * sk_set_ttl - set transmit TTL for given socket
 * @s: socket
 * @ttl: TTL value
 *
 * Set TTL for already opened connections when TTL was not set before. Useful
 * for accepted connections when different ones should have different TTL.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_set_ttl(struct birdsock *s, int ttl)
{
	s->ttl = ttl;

	if (sk_is_ipv4(s))
		return sk_set_ttl4(s, ttl);
	else
		return sk_set_ttl6(s, ttl);
}

/**
 * sk_set_min_ttl - set minimal accepted TTL for given socket
 * @s: socket
 * @ttl: TTL value
 *
 * Set minimal accepted TTL for given socket. Can be used for TTL security.
 * implementations.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_set_min_ttl(struct birdsock *s, int ttl)
{
	if (sk_is_ipv4(s))
		return sk_set_min_ttl4(s, ttl);
	else
		return sk_set_min_ttl6(s, ttl);
}

#if 0
/**
 * sk_set_md5_auth - add / remove MD5 security association for given socket
 * @s: socket
 * @local: IP address of local side
 * @remote: IP address of remote side
 * @ifa: Interface for link-local IP address
 * @passwd: Password used for MD5 authentication
 * @setkey: Update also system SA/SP database
 *
 * In TCP MD5 handling code in kernel, there is a set of security associations
 * used for choosing password and other authentication parameters according to
 * the local and remote address. This function is useful for listening socket,
 * for active sockets it may be enough to set s->password field.
 *
 * When called with passwd != NULL, the new pair is added,
 * When called with passwd == NULL, the existing pair is removed.
 *
 * Note that while in Linux, the MD5 SAs are specific to socket, in BSD they are
 * stored in global SA/SP database (but the behavior also must be enabled on
 * per-socket basis). In case of multiple sockets to the same neighbor, the
 * socket-specific state must be configured for each socket while global state
 * just once per src-dst pair. The @setkey argument controls whether the global
 * state (SA/SP database) is also updated.
 *
 * Result: 0 for success, -1 for an error.
 */

int
sk_set_md5_auth(struct birdsock *s, ip_addr local, ip_addr remote,
		struct iface *ifa, char *passwd, int setkey)
{
	DUMMY;
}
#endif

/**
 * sk_set_ipv6_checksum - specify IPv6 checksum offset for given socket
 * @s: socket
 * @offset: offset
 *
 * Specify IPv6 checksum field offset for given raw IPv6 socket. After that, the
 * kernel will automatically fill it for outgoing packets and check it for
 * incoming packets. Should not be used on ICMPv6 sockets, where the position is
 * known to the kernel.
 *
 * Result: 0 for success, -1 for an error.
 */

int sk_set_ipv6_checksum(struct birdsock *s, int offset)
{
	if (setsockopt(s->fd, SOL_IPV6, IPV6_CHECKSUM, &offset, sizeof(offset))
	    < 0)
		ERR("IPV6_CHECKSUM");

	return 0;
}

int sk_set_icmp6_filter(struct birdsock *s, int p1, int p2)
{
	/* a bit of lame interface, but it is here only for Radv */
	struct icmp6_filter f;

	ICMP6_FILTER_SETBLOCKALL(&f);
	ICMP6_FILTER_SETPASS(p1, &f);
	ICMP6_FILTER_SETPASS(p2, &f);

	if (setsockopt(s->fd, SOL_ICMPV6, ICMP6_FILTER, &f, sizeof(f)) < 0)
		ERR("ICMP6_FILTER");

	return 0;
}

void sk_log_error(struct birdsock *s, const char *p)
{
	log(L_ERR "%s: Socket error: %s%#m", p, s->err);
}

/*
 *	Actual struct birdsock code
 */

//static struct list_head sock_list;
/*
static inline struct birdsock *sk_next(struct birdsock *s)
{
	if (list_is_last(&s->n, &sock_list))
		return NULL;
	else
		return container_of(s->n.next, struct birdsock, n);
}*/

static void sk_alloc_bufs(struct birdsock *s)
{
	if (!s->rbuf && s->rbsize)
		s->rbuf = s->rbuf_alloc = xmalloc(s->rbsize);
	s->rpos = s->rbuf;
	if (!s->tbuf && s->tbsize)
		s->tbuf = s->tbuf_alloc = xmalloc(s->tbsize);
	s->tpos = s->ttx = s->tbuf;
}

static void sk_free_bufs(struct birdsock *s)
{
	if (s->rbuf_alloc) {
		xfree(s->rbuf_alloc);
		s->rbuf = s->rbuf_alloc = NULL;
	}
	if (s->tbuf_alloc) {
		xfree(s->tbuf_alloc);
		s->tbuf = s->tbuf_alloc = NULL;
	}
}

static void sk_free(struct resource *r)
{
	struct birdsock *s = (struct birdsock *)r;

	DBG("sk_free() fd:%d\n", s->sock.fd);

	sk_free_bufs(s);
	if (s->sock.fd >= 0)
		close(s->sock.fd);
	uloop_fd_delete(&s->sock);
}

void sk_set_rbsize(struct birdsock *s, uint val)
{
	ASSERT(s->rbuf_alloc == s->rbuf);

	if (s->rbsize == val)
		return;

	s->rbsize = val;
	xfree(s->rbuf_alloc);
	s->rbuf_alloc = xmalloc(val);
	s->rpos = s->rbuf = s->rbuf_alloc;
}

void sk_set_tbsize(struct birdsock *s, uint val)
{
	ASSERT(s->tbuf_alloc == s->tbuf);

	if (s->tbsize == val)
		return;

	byte *old_tbuf = s->tbuf;

	s->tbsize = val;
	s->tbuf = s->tbuf_alloc = xrealloc(s->tbuf_alloc, val);
	s->tpos = s->tbuf + (s->tpos - old_tbuf);
	s->ttx = s->tbuf + (s->ttx - old_tbuf);
}

void sk_set_tbuf(struct birdsock *s, void *tbuf)
{
	s->tbuf = tbuf ? : s->tbuf_alloc;
	s->ttx = s->tpos = s->tbuf;
}

void sk_reallocate(struct birdsock *s)
{
	sk_free_bufs(s);
	sk_alloc_bufs(s);
}

static void sk_dump(struct resource *r)
{
	struct birdsock *s = (struct birdsock *)r;
	static char *sk_type_names[] =
	    { "TCP<", "TCP>", "TCP", "UDP", NULL, "IP", NULL, "MAGIC", "UNIX<",
		"UNIX", "DEL!"
	};

	debug
	    ("(%s, ud=%p, sa=%I, sp=%d, da=%I, dp=%d, tos=%d, ttl=%d, if=%s)\n",
	     sk_type_names[s->type], s->data, s->saddr, s->sport, s->daddr,
	     s->dport, s->tos, s->ttl, s->iface ? s->iface->name : "none");
}

static struct resclass sk_class = {
	"Socket",
	sizeof(struct birdsock),
	sk_free,
	sk_dump,
	NULL,
	NULL
};

/**
 * sk_new - create a socket
 * @p: pool
 *
 * This function creates a new socket resource. If you want to use it,
 * you need to fill in all the required fields of the structure and
 * call sk_open() to do the actual opening of the socket.
 *
 * The real function name is sock_new(), sk_new() is a macro wrapper
 * to avoid collision with OpenSSL.
 */
struct birdsock *sock_new(struct pool *p)
{
	struct birdsock *s = ralloc(p, &sk_class);
	s->pool = p;
	// s->saddr = s->daddr = IPA_NONE;
	s->tos = s->priority = s->ttl = -1;
	s->fd = -1;
	return s;
}

static int sk_setup(struct birdsock *s)
{
	int y = 1;
	int fd = s->fd;

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		ERR("O_NONBLOCK");

	if (!s->af)
		return 0;

	if (ipa_nonzero(s->saddr) && !(s->flags & SKF_BIND))
		s->flags |= SKF_PKTINFO;

#ifdef CONFIG_USE_HDRINCL
	if (sk_is_ipv4(s) && (s->type == SK_IP) && (s->flags & SKF_PKTINFO)) {
		s->flags &= ~SKF_PKTINFO;
		s->flags |= SKF_HDRINCL;
		if (setsockopt(fd, SOL_IP, IP_HDRINCL, &y, sizeof(y)) < 0)
			ERR("IP_HDRINCL");
	}
#endif

	if (s->iface) {
#ifdef SO_BINDTODEVICE
		struct ifreq ifr = { };
#ifdef CONFIG_DPVS
		strcpy(ifr.ifr_name, "eth1");
#else
		strcpy(ifr.ifr_name, s->iface->name);
#endif
		if (setsockopt(s->fd, SOL_SOCKET, SO_BINDTODEVICE,
					&ifr, sizeof(ifr)) < 0)
			ERR("SO_BINDTODEVICE");
#endif

#ifdef CONFIG_UNIX_DONTROUTE
		if (setsockopt(s->fd, SOL_SOCKET, SO_DONTROUTE, &y, sizeof(y)) <
		    0)
			ERR("SO_DONTROUTE");
#endif
	}

	if (s->priority >= 0)
		if (sk_set_priority(s, s->priority) < 0)
			return -1;

	if (sk_is_ipv4(s)) {
		if (s->flags & SKF_LADDR_RX)
			if (sk_request_cmsg4_pktinfo(s) < 0)
				return -1;

		if (s->flags & SKF_TTL_RX)
			if (sk_request_cmsg4_ttl(s) < 0)
				return -1;

		if ((s->type == SK_UDP) || (s->type == SK_IP))
			if (sk_disable_mtu_disc4(s) < 0)
				return -1;

		if (s->ttl >= 0)
			if (sk_set_ttl4(s, s->ttl) < 0)
				return -1;

		if (s->tos >= 0)
			if (sk_set_tos4(s, s->tos) < 0)
				return -1;
	}

	if (sk_is_ipv6(s)) {
		if (s->flags & SKF_V6ONLY)
			if (setsockopt(fd, SOL_IPV6, IPV6_V6ONLY, &y, sizeof(y))
			    < 0)
				ERR("IPV6_V6ONLY");

		if (s->flags & SKF_LADDR_RX)
			if (sk_request_cmsg6_pktinfo(s) < 0)
				return -1;

		if (s->flags & SKF_TTL_RX)
			if (sk_request_cmsg6_ttl(s) < 0)
				return -1;

		if ((s->type == SK_UDP) || (s->type == SK_IP))
			if (sk_disable_mtu_disc6(s) < 0)
				return -1;

		if (s->ttl >= 0)
			if (sk_set_ttl6(s, s->ttl) < 0)
				return -1;

		if (s->tos >= 0)
			if (sk_set_tos6(s, s->tos) < 0)
				return -1;
	}

	return 0;
}

typedef uint32_t __be32;
void ospf_recv_cb(char *data, uint16_t data_len, uint8_t port,
		__be32 saddr, __be32 daddr){
	int c;
	struct birdsock *s;

	printf("ospf_recv_cb, len:%d\n", data_len);
	/*int i = 0;
	for (; i < data_len; i++){
		printf("%02x", (uint8_t)data[i]);
		if (i % 4 == 3) printf(" ");
		if (i % 32 == 31) printf("\n");
	}
	printf("\n");*/

	saddr = ntohl(saddr);
	daddr = ntohl(daddr);

	update_times();
	list_for_each_entry(s, &ospf_sk_list, n) {
		s->faddr.addr = saddr;
		s->laddr.addr = daddr;
		if (s->rx_hook && s->iface->index == port){
			io_log_event(s->rx_hook, s->data);
			while (data_len > 0){
				c =  MIN(data_len, s->rbsize);
				memcpy(s->rbuf, data, c);
				s->rpos = s->rbuf + c;
				s->rx_hook(s, c);
				data_len -= c;
			}
		}
	}
}

static void sk_cb(struct uloop_fd *sock, unsigned int events)
{
	int e;
	int steps;
	struct birdsock *s;

	s = container_of(sock, struct birdsock, sock);
	steps = MAX_STEPS;
	update_times();

	log(L_ERR "events: 0x%x", events);

	if ((events & ULOOP_READ) && s->rx_hook){
		do {
			steps--;
			io_log_event(s->rx_hook, s->data);
			e = sk_read(s, events);
		} while (e && s->rx_hook && steps);
	}

	steps = MAX_STEPS;
	if ((events & ULOOP_WRITE) && s->tx_hook){
		do {
			steps--;
			io_log_event(s->tx_hook, s->data);
			e = sk_write(s);
		} while (e && steps);
	}

}

static void sk_insert(struct birdsock *s)
{
	int ret;
	unsigned int flags = 0;

	DBG("sk_insert() fd:%d\n", s->fd);
	if (s->rx_hook){
		flags |= ULOOP_READ;
	}
	if (s->tx_hook && s->ttx != s->tpos){
		flags |= ULOOP_WRITE;
	}

	if (flags){
		s->sock.cb = sk_cb;
		s->sock.fd = s->fd;
		ret = uloop_fd_add(&s->sock,
				flags | ULOOP_EDGE_TRIGGER
				| USOCK_NOCLOEXEC);
		DBG("uloop_fd_add fd %d %d\n", s->sock.fd, ret);
		if (ret == -1){
			log(L_ERR "uloop_fd_add fd %d error(%d:%s)",
					s->sock.fd, errno, strerror(errno));
			s->err_hook(s, errno);
		}
	}
}

static int sk_passive_connected(struct birdsock *s, int type)
{
	struct sockaddr_bird loc_sa, rem_sa;
	int loc_sa_len = sizeof(loc_sa);
	int rem_sa_len = sizeof(rem_sa);

	int fd = accept(s->fd, ((type == SK_TCP) ? &rem_sa.sa : NULL),
			&rem_sa_len);
	if (fd < 0) {
		if ((errno != EINTR) && (errno != EAGAIN))
			s->err_hook(s, errno);
		return 0;
	}

	struct birdsock *t = sk_new(s->pool);
	t->type = type;
	t->fd = fd;
	t->af = s->af;
	t->ttl = s->ttl;
	t->tos = s->tos;
	t->rbsize = s->rbsize;
	t->tbsize = s->tbsize;

	if (type == SK_TCP) {
		if ((getsockname(fd, &loc_sa.sa, &loc_sa_len) < 0) ||
		    (sockaddr_read (&loc_sa, s->af, &t->saddr,
				    &t->iface, &t->sport) < 0))
			log(L_WARN
			    "SOCK: Cannot get local IP address for TCP<");

		if (sockaddr_read(&rem_sa, s->af, &t->daddr, &t->iface,
					&t->dport) < 0)
			log(L_WARN "SOCK: Cannot get remote IP address for TCP<");
	}

	if (sk_setup(t) < 0) {
		/* FIXME: Call err_hook instead ? */
		log(L_ERR "SOCK: Incoming connection: %s%#m", t->err);

		/* FIXME: handle it better in rfree() */
		close(t->fd);
		t->fd = -1;
		rfree(t);
		return 1;
	}

	sk_insert(t);
	sk_alloc_bufs(t);
	s->rx_hook(t, 0);
	return 1;
}

int ospf_sk_open(struct birdsock *sk)
{
	sk->af = AF_INET;
	sk_alloc_bufs(sk);
	list_add_tail(&sk->n, &ospf_sk_list);
	return 0;
}

/**
 * sk_open - open a socket
 * @s: socket
 *
 * This function takes a socket struct resource created by sk_new() and
 * initialized by the user and binds a corresponding network connection
 * to it.
 *
 * Result: 0 for success, -1 for an error.
 */
int sk_open(struct birdsock *s)
{
	int af = BIRD_AF;
	int fd = -1;
	int do_bind = 0;
	int bind_port = 0;
	ip_addr bind_addr = IPA_NONE;
	struct sockaddr_bird sa;

	switch (s->type) {
	//case SK_TCP_ACTIVE:
	//	s->ttx = "";	/* Force s->ttx != s->tpos */
		/* Fall thru */
	case SK_TCP_PASSIVE:
		fd = socket(af, SOCK_STREAM, IPPROTO_TCP);
		bind_port = s->sport;
		bind_addr = s->saddr;
		do_bind = bind_port || ipa_nonzero(bind_addr);
		break;

	case SK_UDP:
		fd = socket(af, SOCK_DGRAM, IPPROTO_UDP);
		bind_port = s->sport;
		bind_addr = (s->flags & SKF_BIND) ? s->saddr : IPA_NONE;
		do_bind = 1;
		break;

	case SK_IP:
		fd = socket(af, SOCK_RAW, s->dport);
		bind_port = 0;
		bind_addr = (s->flags & SKF_BIND) ? s->saddr : IPA_NONE;
		do_bind = ipa_nonzero(bind_addr);
		break;

	case SK_MAGIC:
		af = 0;
		fd = s->fd;
		break;

	default:
		bug("sk_open() called for invalid struct birdsock type %d",
		    s->type);
	}

	if (fd < 0)
		ERR("socket");

	s->af = af;
	s->fd = fd;

	if (sk_setup(s) < 0)
		goto err;

	if (do_bind) {
		if (bind_port) {
			int y = 1;

			if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &y,
						sizeof(y)) < 0)
				ERR2("SO_REUSEADDR");

#ifdef CONFIG_NO_IFACE_BIND
			/* Workaround missing ability to bind to an iface */
			if ((s->type == SK_UDP) && s->iface &&
					ipa_zero(bind_addr))
				if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
							&y, sizeof(y)) < 0)
					ERR2("SO_REUSEPORT");
#endif
		} else if (s->flags & SKF_HIGH_PORT)
			if (sk_set_high_port(s) < 0)
				log(L_WARN "Socket error: %s%#m", s->err);

		sockaddr_fill(&sa, af, bind_addr, s->iface, bind_port);
		if (bind(fd, &sa.sa, SA_LEN(sa)) < 0)
			ERR2("bind");
	}

	if (s->password)
		if (sk_set_md5_auth(s, s->saddr, s->daddr, s->iface,
					s->password, 0) < 0)
			goto err;

	switch (s->type) {
	case SK_TCP_PASSIVE:
		if (listen(fd, 8) < 0)
			ERR2("listen");
		break;

	case SK_MAGIC:
		break;

	default:
		sk_alloc_bufs(s);
	}

	sk_insert(s);
	return 0;

err:
	close(fd);
	s->fd = -1;
	return -1;
}

int sk_open_unix(struct birdsock *s, char *name)
{
	struct sockaddr_un sa;
	int fd;

	/* We are sloppy during error (leak fd and not set s->err), but we die anyway */

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		return -1;

	/*if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		return -1;*/

	/* Path length checked in test_old_bird() */
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, name);

	if (bind(fd, (struct sockaddr *)&sa, SUN_LEN(&sa)) < 0)
		return -1;

	if (listen(fd, 8) < 0)
		return -1;

	s->fd = fd;
	sk_insert(s);
	return 0;
}

#define CMSG_RX_SPACE MAX(CMSG4_SPACE_PKTINFO+CMSG4_SPACE_TTL, \
			  CMSG6_SPACE_PKTINFO+CMSG6_SPACE_TTL)
#define CMSG_TX_SPACE MAX(CMSG4_SPACE_PKTINFO,CMSG6_SPACE_PKTINFO)
/*
static void
sk_prepare_cmsgs(struct birdsock *s, struct msghdr *msg, void *cbuf,
		 size_t cbuflen)
{
	if (sk_is_ipv4(s))
		sk_prepare_cmsgs4(s, msg, cbuf, cbuflen);
	else
		sk_prepare_cmsgs6(s, msg, cbuf, cbuflen);
}
*/

static void sk_process_cmsgs(struct birdsock *s, struct msghdr *msg)
{
	struct cmsghdr *cm;

	s->laddr = IPA_NONE;
	s->lifindex = 0;
	s->rcv_ttl = -1;

	for (cm = CMSG_FIRSTHDR(msg); cm != NULL; cm = CMSG_NXTHDR(msg, cm)) {
		if ((cm->cmsg_level == SOL_IP) && sk_is_ipv4(s)) {
			sk_process_cmsg4_pktinfo(s, cm);
			sk_process_cmsg4_ttl(s, cm);
		}

		if ((cm->cmsg_level == SOL_IPV6) && sk_is_ipv6(s)) {
			sk_process_cmsg6_pktinfo(s, cm);
			sk_process_cmsg6_ttl(s, cm);
		}
	}
}


/* ugly hack: only use for ospf */
static inline int sk_sendmsg(struct birdsock *s)
{
	int i = 0;
	int len = s->tpos - s->tbuf;
	printf("sk_sendmsg, len:%d\n", len);
	for (; i < len; i++){
		printf("%02x", s->tbuf[i]);
		if (i % 4 == 3) printf(" ");
		if (i % 32 == 31) printf("\n");
	}
	printf("\n");

	ipv4_vs_ospf_xmit(s->saddr.addr, s->daddr.addr, s->iface->index,
			s->tbuf, s->tpos - s->tbuf);

	return 0;
#if 0
	struct iovec iov = { s->tbuf, s->tpos - s->tbuf };
	byte cmsg_buf[CMSG_TX_SPACE];
	struct sockaddr_bird dst;

	sockaddr_fill(&dst, s->af, s->daddr, s->iface, s->dport);

	struct msghdr msg = {
		.msg_name = &dst.sa,
		.msg_namelen = SA_LEN(dst),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

#ifdef CONFIG_USE_HDRINCL
	byte hdr[20];
	struct iovec iov2[2] = { {hdr, 20}, iov };

	if (s->flags & SKF_HDRINCL) {
		sk_prepare_ip_header(s, hdr, iov.iov_len);
		msg.msg_iov = iov2;
		msg.msg_iovlen = 2;
	}
#endif

	if (s->flags & SKF_PKTINFO)
		sk_prepare_cmsgs(s, &msg, cmsg_buf, sizeof(cmsg_buf));

	return sendmsg(s->fd, &msg, 0);
#endif
}

static inline int sk_recvmsg(struct birdsock *s)
{
	struct iovec iov = { s->rbuf, s->rbsize };
	byte cmsg_buf[CMSG_RX_SPACE];
	struct sockaddr_bird src;

	struct msghdr msg = {
		.msg_name = &src.sa,
		.msg_namelen = sizeof(src),	// XXXX ??
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsg_buf,
		.msg_controllen = sizeof(cmsg_buf),
		.msg_flags = 0
	};

	int rv = recvmsg(s->fd, &msg, 0);
	if (rv < 0)
		return rv;

	//ifdef IPV4
	//  if (cf_type == SK_IP)
	//    rv = ipv4_skip_header(pbuf, rv);
	//endif

	sockaddr_read(&src, s->af, &s->faddr, NULL, &s->fport);
	sk_process_cmsgs(s, &msg);

	if (msg.msg_flags & MSG_TRUNC)
		s->flags |= SKF_TRUNCATED;
	else
		s->flags &= ~SKF_TRUNCATED;

	return rv;
}

static inline void reset_tx_buffer(struct birdsock *s)
{
	s->ttx = s->tpos = s->tbuf;
}

static int sk_maybe_write(struct birdsock *s)
{
	int e;

	switch (s->type) {
	case SK_TCP:
	case SK_MAGIC:
	case SK_UNIX:
		while (s->ttx != s->tpos) {
			e = write(s->fd, s->ttx, s->tpos - s->ttx);

			if (e < 0) {
				if (errno != EINTR && errno != EAGAIN) {
					reset_tx_buffer(s);
					/* EPIPE is just a connection close notification during TX */
					s->err_hook(s,
						    (errno !=
						     EPIPE) ? errno : 0);
					return -1;
				}
				return 0;
			}
			s->ttx += e;
		}
		reset_tx_buffer(s);
		return 1;

	case SK_UDP:
	case SK_IP:
		{
			if (s->tbuf == s->tpos)
				return 1;

			e = sk_sendmsg(s);

			if (e < 0) {
				if (errno != EINTR && errno != EAGAIN) {
					reset_tx_buffer(s);
					s->err_hook(s, errno);
					return -1;
				}

				if (!s->tx_hook)
					reset_tx_buffer(s);
				return 0;
			}
			reset_tx_buffer(s);
			return 1;
		}
	default:
		bug("sk_maybe_write: unknown socket type %d", s->type);
	}
}

int sk_rx_ready(struct birdsock *s)
{
	int rv;
	struct pollfd pfd = {.fd = s->fd };
	pfd.events |= POLLIN;

redo:
	rv = poll(&pfd, 1, 0);

	if ((rv < 0) && (errno == EINTR || errno == EAGAIN))
		goto redo;

	return rv;
}

/**
 * sk_send - send data to a socket
 * @s: socket
 * @len: number of bytes to send
 *
 * This function sends @len bytes of data prepared in the
 * transmit struct buffer of the socket @s to the network connection.
 * If the packet can be sent immediately, it does so and returns
 * 1, else it queues the packet for later processing, returns 0
 * and calls the @tx_hook of the socket when the tranmission
 * takes place.
 */
int sk_send(struct birdsock *s, unsigned len)
{
	s->ttx = s->tbuf;
	s->tpos = s->tbuf + len;
	return sk_maybe_write(s);
}

/**
 * sk_send_to - send data to a specific destination
 * @s: socket
 * @len: number of bytes to send
 * @addr: IP address to send the packet to
 * @port: port to send the packet to
 *
 * This is a sk_send() replacement for connection-less packet sockets
 * which allows destination of the packet to be chosen dynamically.
 * Raw IP sockets should use 0 for @port.
 */
int sk_send_to(struct birdsock *s, unsigned len, ip_addr addr, unsigned port)
{
	s->daddr = addr;
	if (port)
		s->dport = port;

	s->ttx = s->tbuf;
	s->tpos = s->tbuf + len;
	return sk_maybe_write(s);
}

/*
int
sk_send_full(struct birdsock *s, unsigned len, struct iface *ifa,
	     ip_addr saddr, ip_addr daddr, unsigned dport)
{
  s->iface = ifa;
  s->saddr = saddr;
  s->daddr = daddr;
  s->dport = dport;
  s->ttx = s->tbuf;
  s->tpos = s->tbuf + len;
  return sk_maybe_write(s);
}
*/

 /* sk_read() and sk_write() are called from BFD's struct event loop */

int sk_read(struct birdsock *s, int revents)
{
	int c;

	switch (s->type) {
	case SK_TCP_PASSIVE:
		return sk_passive_connected(s, SK_TCP);

	case SK_UNIX_PASSIVE:
		return sk_passive_connected(s, SK_UNIX);

	case SK_TCP:
	case SK_UNIX:
		c = read(s->fd, s->rpos, s->rbuf + s->rbsize - s->rpos);

		if (c < 0) {
			if (errno != EINTR && errno != EAGAIN)
				s->err_hook(s, errno);
			else if (errno == EAGAIN && !(revents & POLLIN)) {
				log(L_ERR
				    "Got EAGAIN from read when revents=%x (without POLLIN)",
				    revents);
				s->err_hook(s, 0);
			}
		} else if (!c)
			s->err_hook(s, 0);
		else {
			s->rpos += c;
			if (s->rx_hook(s, s->rpos - s->rbuf)) {
				/* We need to be careful since the socket could have been deleted by the hook */
				s->rpos = s->rbuf;
			}
			return 1;
		}
		return 0;

	case SK_MAGIC:
		return s->rx_hook(s, 0);

	default:
		c = sk_recvmsg(s);

		if (c < 0) {
			if (errno != EINTR && errno != EAGAIN)
				s->err_hook(s, errno);
			return 0;
		}

		s->rpos = s->rbuf + c;
		s->rx_hook(s, c);
		return 1;
	}
}

static int sk_write(struct birdsock *s)
{
	if (s->ttx != s->tpos && sk_maybe_write(s) > 0) {
		if (s->tx_hook)
			s->tx_hook(s);
		return 1;
	}
	return 0;
}

void sk_err(struct birdsock *s, int revents)
{
	int se = 0, sse = sizeof(se);
	if (revents & POLLERR)
		if (getsockopt(s->fd, SOL_SOCKET, SO_ERROR, &se, &sse) < 0) {
			log(L_ERR "IO: Socket error: SO_ERROR: %m");
			se = 0;
		}

	s->err_hook(s, se);
}

void sk_dump_all(void)
{
	debug("Open sockets: not support\n");
}

/*
 *	Internal struct event log and watchdog
 */


/**
 * io_log_event - mark approaching struct event into struct event log
 * @hook: struct event hook address
 * @data: struct event data address
 *
 * Store info (hook, data, timestamp) about the following internal struct event into
 * a circular struct event log (@event_log). When latency tracking is enabled, the log
 * entry is kept open (in @event_open) so the duration can be filled later.
 */
void io_log_event(void *hook, void *data)
{
	struct event_log_entry *en = &event_log[event_log_pos];

	en->hook = hook;
	en->data = data;
	en->timestamp = last_time;
	en->duration = 0;

	event_log_num++;
	event_log_pos++;
	event_log_pos %= EVENT_LOG_LENGTH;

	event_open = config->latency_debug ? en : NULL;
}

/*
 *	Main I/O Loop
 */

volatile int async_config_flag;	/* Asynchronous reconfiguration/dump scheduled */
volatile int async_dump_flag;

static struct uloop_timeout event_timer;

static void event_timer_cb(struct uloop_timeout *timeout)
{
	update_times();
	uloop_timeout_set(timeout, EVENT_LOOPTIME);

	ev_run_list(&global_event_list);

	if (async_config_flag) {
		io_log_event(async_config, NULL);
		async_config();
		async_config_flag = 0;
	}

	if (async_dump_flag) {
		io_log_event(async_dump, NULL);
		async_dump();
		async_dump_flag = 0;
	}

	if (async_shutdown_flag) {
		io_log_event(async_shutdown, NULL);
		async_shutdown();
		async_shutdown_flag = 0;
	}
}

void io_init(void)
{
	krt_io_init();
	update_times();

	INIT_LIST_HEAD(&global_event_list);
	INIT_LIST_HEAD(&ospf_sk_list);
	event_timer.cb = event_timer_cb;
	uloop_timeout_set(&event_timer, EVENT_LOOPTIME);
}



#if 0

static int short_loops = 0;
#define SHORT_LOOP_MAX 10

void io_loop(void)
{
	int poll_tout;
	time_t tout;
	int nfds, events, pout;
	struct birdsock *s;
	int fdmax = 256;
	struct pollfd *pfd = xmalloc(fdmax * sizeof(struct pollfd));

	watchdog_start1();
	for (;;) {
		events = ev_run_list(&global_event_list);
timers:
		poll_tout = (events ? 0 : MIN(tout - now, 3)) * 1000;	/* Time in milliseconds */

		nfds = 0;
		list_for_each_entry(s, &sock_list, n) {
			/* everything other set to 0 by this */
			pfd[nfds] = (struct pollfd) { .fd = -1};

			if (s->rx_hook) {
				pfd[nfds].fd = s->fd;
				pfd[nfds].events |= POLLIN;
			}
			if (s->tx_hook && s->ttx != s->tpos) {
				pfd[nfds].fd = s->fd;
				pfd[nfds].events |= POLLOUT;
			}
			if (pfd[nfds].fd != -1) {
				s->index = nfds;
				nfds++;
			} else
				s->index = -1;

			if (nfds >= fdmax) {
				fdmax *= 2;
				pfd = xrealloc(pfd, fdmax * sizeof(struct pollfd));
			}
		}

		/*
		 * Yes, this is racy. But even if the signal comes before this test
		 * and entering poll(), it gets caught on the next struct timer tick.
		 */

		if (async_config_flag) {
			io_log_event(async_config, NULL);
			async_config();
			async_config_flag = 0;
			continue;
		}
		if (async_dump_flag) {
			io_log_event(async_dump, NULL);
			async_dump();
			async_dump_flag = 0;
			continue;
		}
		if (async_shutdown_flag) {
			io_log_event(async_shutdown, NULL);
			async_shutdown();
			async_shutdown_flag = 0;
			continue;
		}

		/* And finally enter poll() to find active sockets */
		watchdog_stop();
		pout = poll(pfd, nfds, poll_tout);
		watchdog_start();

		if (pout < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;
			die("poll: %m");
		}
		if (pout) {
			/* guaranteed to be non-empty */
			current_sock = list_first_entry(&sock_list, struct birdsock, n);

			while (current_sock) {
				struct birdsock *s = current_sock;
				if (s->index == -1) {
					current_sock = sk_next(s);
					goto next;
				}

				int e;
				int steps;

				steps = MAX_STEPS;
				if (s->fast_rx && (pfd[s->index].revents & POLLIN)
						&& s->rx_hook){
					do {
						steps--;
						io_log_event(s->rx_hook, s->data);
						e = sk_read(s, pfd[s->index].revents);
						if (s != current_sock)
							goto next;
					} while (e && s->rx_hook && steps);
				}

				steps = MAX_STEPS;
				if (pfd[s->index].revents & POLLOUT){
					do {
						steps--;
						io_log_event(s->tx_hook, s->data);
						e = sk_write(s);
						if (s != current_sock)
							goto next;
					} while (e && steps);
				}

				current_sock = sk_next(s);
next:				;
			}

			short_loops++;
			if (events && (short_loops < SHORT_LOOP_MAX))
				continue;
			short_loops = 0;

			int count = 0;
			current_sock = stored_sock;
			if (current_sock == NULL)
				current_sock = list_first_entry(&sock_list,
						struct birdsock, n);

			while (current_sock && count < MAX_RX_STEPS) {
				struct birdsock *s = current_sock;
				if (s->index == -1) {
					current_sock = sk_next(s);
					goto next2;
				}

				if (!s->fast_rx
				    && (pfd[s->index].revents & POLLIN)
				    && s->rx_hook) {
					count++;
					io_log_event(s->rx_hook, s->data);
					sk_read(s, pfd[s->index].revents);
					if (s != current_sock)
						goto next2;
				}

				if (pfd[s->index].revents & (POLLHUP | POLLERR)) {
					sk_err(s, pfd[s->index].revents);
					goto next2;
				}

				current_sock = sk_next(s);
next2:				;
			}

			stored_sock = current_sock;
		}
	}
}
#endif

void test_old_bird(char *path)
{
	int fd;
	struct sockaddr_un sa;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		die("Cannot create socket: %m");
	if (strlen(path) >= sizeof(sa.sun_path))
		die("Socket path too long");
	bzero(&sa, sizeof(sa));
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, path);
	if (connect(fd, (struct sockaddr *)&sa, SUN_LEN(&sa)) == 0)
		die("I found another BIRD running.");
	close(fd);
}