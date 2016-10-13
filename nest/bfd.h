/*
 *	BIRD -- Bidirectional Forwarding Detection (BFD)
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_NBFD_H_
#define _BIRD_NBFD_H_

#include "lib/list.h"
#include "lib/resource.h"

struct bfd_session;

struct bfd_request {
	struct resource r;
	struct list_head n;

	ip_addr addr;
	ip_addr local;
	struct iface *iface;

	void (*hook) (struct bfd_request *);
	void *data;

	struct bfd_session *session;

	u8 state;
	u8 diag;
	u8 old_state;
	u8 down;
};

#define BFD_STATE_ADMIN_DOWN	0
#define BFD_STATE_DOWN		1
#define BFD_STATE_INIT		2
#define BFD_STATE_UP		3

#ifdef CONFIG_BFD

struct bfd_request *bfd_request_session(struct pool *p, ip_addr addr,
					ip_addr local, struct iface *iface,
					void (*hook) (struct bfd_request *),
					void *data);

static inline void cf_check_bfd(int use)
{
}

#else

static inline struct bfd_request 
	*bfd_request_session(__attribute__ ((__unused__))struct pool *p,
			      __attribute__ ((__unused__))ip_addr addr,
			      __attribute__ ((__unused__))ip_addr local,
			      __attribute__ ((__unused__))struct iface *iface,
			      __attribute__ ((__unused__))void (*hook) (struct bfd_request *),
			      __attribute__ ((__unused__))void *data)
{
	return NULL;
}

static inline void cf_check_bfd(int use)
{
	if (use)
		cf_error((char *)"BFD not available");
}

#endif /* CONFIG_BFD */

#endif /* _BIRD_NBFD_H_ */
