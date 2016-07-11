/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         cf_parse
#define yylex           cf_lex
#define yyerror         cf_error
#define yydebug         cf_debug
#define yynerrs         cf_nerrs

#define yylval          cf_lval
#define yychar          cf_char

/* Copy the first part of user declarations.  */
#line 1 "cf-parse.y" /* yacc.c:339  */

/* Headers from ../../conf/confbase.Y */

#define PARSER 1

#include "nest/bird.h"
#include "conf/conf.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "lib/timer.h"
#include "lib/string.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "nest/route.h"
#include "nest/cli.h"
#include "filter/filter.h"

/* FIXME: Turn on YYERROR_VERBOSE and work around lots of bison bugs? */

/* Defines from ../../conf/confbase.Y */

static void
check_u16(unsigned val)
{
  if (val > 0xFFFF)
    cf_error("Value %d out of range (0-65535)", val);
}

/* Headers from ../../sysdep/unix/config.Y */

#include "lib/unix.h"
#include <stdio.h>

/* Headers from ../../sysdep/unix/krt.Y */

#include "lib/krt.h"

/* Defines from ../../sysdep/unix/krt.Y */

#define THIS_KRT ((struct krt_config *) this_proto)
#define THIS_KIF ((struct kif_config *) this_proto)

/* Headers from ../../sysdep/linux/netlink.Y */

/* Headers from ../../nest/config.Y */

#include "nest/rt-dev.h"
#include "nest/password.h"
#include "nest/cmds.h"
#include "lib/lists.h"

/* Defines from ../../nest/config.Y */

static struct proto_config *this_proto;
static struct iface_patt *this_ipatt;
static struct iface_patt_node *this_ipn;
static struct roa_table_config *this_roa_table;
static union list *this_p_list;
static struct password_item *this_p_item;
static int password_id;

static void
iface_patt_check(void)
{
  struct iface_patt_node *pn;

  WALK_LIST(pn, this_ipatt->ipn_list)
    if (!pn->pattern || pn->pxlen)
      cf_error("Interface name/mask expected, not IP prefix");
}


static inline void
reset_passwords(void)
{
  this_p_list = NULL;
}

static inline union list *
get_passwords(void)
{
  union list *rv = this_p_list;
  this_p_list = NULL;
  return rv;
}

#define DIRECT_CFG ((struct rt_dev_config *) this_proto)

/* Headers from ../../filter/config.Y */

/* Defines from ../../filter/config.Y */

#define P(a,b) ((a << 8) | b)

static inline u32 pair(u32 a, u32 b) { return (a << 16) | b; }
static inline u32 pair_a(u32 p) { return p >> 16; }
static inline u32 pair_b(u32 p) { return p & 0xFFFF; }


/*
 * Sets and their items are during parsing handled as lists, linked
 * through left ptr. The first item in a union list also contains a pointer
 * to the last item in a union list (right ptr). For convenience, even items
 * are handled as one-item lists. Lists are merged by f_merge_items().
 */
static int
f_valid_set_type(int type)
{
  switch (type)
  {
  case T_INT:
  case T_PAIR:
  case T_QUAD:
  case T_ENUM:
  case T_IP:
  case T_EC:
    return 1;

  default:
    return 0;
  }
}

static inline struct f_tree *
f_new_item(struct f_val from, struct f_val to)
{
  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from = from;
  t->to = to;
  return t;
}

static inline struct f_tree *
f_merge_items(struct f_tree *a, struct f_tree *b)
{
  if (!a) return b;
  a->right->left = b;
  a->right = b->right;
  b->right = NULL;
  return a;
}

static inline struct f_tree *
f_new_pair_item(int fa, int ta, int fb, int tb)
{
  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_PAIR;
  t->from.val.i = pair(fa, fb);
  t->to.val.i = pair(ta, tb);
  return t;
}

static inline struct f_tree *
f_new_pair_set(int fa, int ta, int fb, int tb)
{
  struct f_tree *lst = NULL;
  int i;

  if ((fa == ta) || ((fb == 0) && (tb == 0xFFFF)))
    return f_new_pair_item(fa, ta, fb, tb);
  
  if ((ta < fa) || (tb < fb))
    cf_error( "From value cannot be higher that To value in pair sets");

  for (i = fa; i <= ta; i++)
    lst = f_merge_items(lst, f_new_pair_item(i, i, fb, tb));

  return lst;
}

#define EC_ALL 0xFFFFFFFF

static struct f_tree *
f_new_ec_item(u32 kind, u32 ipv4_used, u32 key, u32 vf, u32 vt)
{
  u64 fm, to;

  if (ipv4_used || (key >= 0x10000)) {
    check_u16(vf);
    if (vt == EC_ALL)
      vt = 0xFFFF;
    else
      check_u16(vt);
  }

  if (kind == EC_GENERIC) {
    fm = ec_generic(key, vf);
    to = ec_generic(key, vt);
  }
  else if (ipv4_used) {
    fm = ec_ip4(kind, key, vf);
    to = ec_ip4(kind, key, vt);
  }
  else if (key < 0x10000) {
    fm = ec_as2(kind, key, vf);
    to = ec_as2(kind, key, vt);
  }
  else {
    fm = ec_as4(kind, key, vf);
    to = ec_as4(kind, key, vt);
  }

  struct f_tree *t = f_new_tree();
  t->right = t;
  t->from.type = t->to.type = T_EC;
  t->from.val.ec = fm;
  t->to.val.ec = to;
  return t;
}

static inline struct f_inst *
f_generate_empty(struct f_inst *dyn)
{ 
  struct f_inst *e = f_new_inst();
  e->code = 'E';

  switch (dyn->aux & EAF_TYPE_MASK) {
    case EAF_TYPE_AS_PATH:
      e->aux = T_PATH;
      break;
    case EAF_TYPE_INT_SET:
      e->aux = T_CLIST;
      break;
    case EAF_TYPE_EC_SET:
      e->aux = T_ECLIST;
      break;
    default:
      cf_error("Can't empty that attribute");
  }

  dyn->code = P('e','S');
  dyn->a1.p = e;
  return dyn;
}


static inline struct f_inst *
f_generate_dpair(struct f_inst *t1, struct f_inst *t2)
{
  struct f_inst *rv;

  if ((t1->code == 'c') && (t2->code == 'c')) {
    if ((t1->aux != T_INT) || (t2->aux != T_INT))
      cf_error( "Can't operate with value of non-integer type in pair constructor");

    check_u16(t1->a2.i);
    check_u16(t2->a2.i);

    rv = f_new_inst();
    rv->code = 'c';
    rv->aux = T_PAIR;
    rv->a2.i = pair(t1->a2.i, t2->a2.i);
  }
  else {
    rv = f_new_inst();
    rv->code = P('m', 'p');
    rv->a1.p = t1;
    rv->a2.p = t2;
  }

  return rv;
}

static inline struct f_inst *
f_generate_ec(u16 kind, struct f_inst *tk, struct f_inst *tv)
{
  struct f_inst *rv;
  int c1 = 0, c2 = 0, ipv4_used = 0;
  u32 key = 0, val2 = 0;

  if (tk->code == 'c') {
    c1 = 1;

    if (tk->aux == T_INT) {
      ipv4_used = 0; key = tk->a2.i;
    }
    else if (tk->aux == T_QUAD) {
      ipv4_used = 1; key = tk->a2.i;
    }
    else
      cf_error("Can't operate with key of non-integer/IPv4 type in EC constructor");
  }

#ifndef IPV6
  /* IP->Quad implicit conversion */
  else if (tk->code == 'C') {
    c1 = 1;
    struct f_val *val = tk->a1.p;

    if (val->type == T_INT) {
      ipv4_used = 0; key = val->val.i;
    }
    else if (val->type == T_QUAD) {
      ipv4_used = 1; key = val->val.i;
    }
    else if (val->type == T_IP) {
      ipv4_used = 1; key = ipa_to_u32(val->val.px.ip);
    }
    else
      cf_error("Can't operate with key of non-integer/IPv4 type in EC constructor");
  }
#endif

  if (tv->code == 'c') {
    if (tv->aux != T_INT)
      cf_error("Can't operate with value of non-integer type in EC constructor");
    c2 = 1;
    val2 = tv->a2.i;
  }

  if (c1 && c2) {
    u64 ec;
  
    if (kind == EC_GENERIC) {
      ec = ec_generic(key, val2);
    }
    else if (ipv4_used) {
      check_u16(val2);
      ec = ec_ip4(kind, key, val2);
    }
    else if (key < 0x10000) {
      ec = ec_as2(kind, key, val2);
    }
    else {
      check_u16(val2);
      ec = ec_as4(kind, key, val2);
    }

    NEW_F_VAL;
    rv = f_new_inst();
    rv->code = 'C';
    rv->a1.p = val;    
    val->type = T_EC;
    val->val.ec = ec;
  }
  else {
    rv = f_new_inst();
    rv->code = P('m','c');
    rv->aux = kind;
    rv->a1.p = tk;
    rv->a2.p = tv;
  }

  return rv;
}



/* Headers from ../../proto/bfd/config.Y */

#include "proto/bfd/bfd.h"

/* Defines from ../../proto/bfd/config.Y */

#define BFD_CFG ((struct bfd_config *) this_proto)
#define BFD_IFACE ((struct bfd_iface_config *) this_ipatt)
#define BFD_NEIGHBOR this_bfd_neighbor

static struct bfd_neighbor *this_bfd_neighbor;

extern struct bfd_config *bfd_cf;

/* Headers from ../../proto/bgp/config.Y */

#include "proto/bgp/bgp.h"

/* Defines from ../../proto/bgp/config.Y */

#define BGP_CFG ((struct bgp_config *) this_proto)

/* Headers from ../../proto/ospf/config.Y */

#include "proto/ospf/ospf.h"

/* Defines from ../../proto/ospf/config.Y */

#define OSPF_CFG ((struct ospf_config *) this_proto)
#define OSPF_PATT ((struct ospf_iface_patt *) this_ipatt)

static struct ospf_area_config *this_area;
static struct nbma_node *this_nbma;
static union list *this_nets;
static struct area_net_config *this_pref;
static struct ospf_stubnet_config *this_stubnet;

static inline int ospf_cfg_is_v2(void) { return OSPF_CFG->ospf2; }
static inline int ospf_cfg_is_v3(void) { return ! OSPF_CFG->ospf2; }

static void
ospf_iface_finish(void)
{
  struct ospf_iface_patt *ip = OSPF_PATT;

  if (ip->deadint == 0)
    ip->deadint = ip->deadc * ip->helloint;

  if (ip->waitint == 0)
    ip->waitint = ip->deadc * ip->helloint;

  ip->passwords = get_passwords();

  if ((ip->autype == OSPF_AUTH_CRYPT) && (ip->helloint < 5))
    log(L_WARN "Hello or poll interval less that 5 makes cryptographic authenication prone to replay attacks");

  if ((ip->autype == OSPF_AUTH_NONE) && (ip->passwords != NULL))
    log(L_WARN "Password option without authentication option does not make sense");
}

static void
ospf_area_finish(void)
{
  if ((this_area->areaid == 0) && (this_area->type != OPT_E))
    cf_error("Backbone area cannot be stub/NSSA");

  if (this_area->summary && (this_area->type == OPT_E))
    cf_error("Only stub/NSSA areas can use summary propagation");

  if (this_area->default_nssa && ((this_area->type != OPT_N) || ! this_area->summary))
    cf_error("Only NSSA areas with summary propagation can use NSSA default route");

  if ((this_area->default_cost & LSA_EXT3_EBIT) && ! this_area->default_nssa)
    cf_error("Only NSSA default route can use type 2 metric");
}

static void
ospf_proto_finish(void)
{
  struct ospf_config *cf = OSPF_CFG;

  if (EMPTY_LIST(cf->area_list))
    cf_error( "No configured areas in OSPF");

  int areano = 0;
  int backbone = 0;
  int nssa = 0;
  struct ospf_area_config *ac;
  WALK_LIST(ac, cf->area_list)
  {
    areano++;
    if (ac->areaid == 0)
      backbone = 1;
    if (ac->type == OPT_N)
      nssa = 1;
  }

  cf->abr = areano > 1;

  /* Route export or NSSA translation (RFC 3101 3.1) */
  cf->asbr = (this_proto->out_filter != FILTER_REJECT) || (nssa && cf->abr);

  if (cf->abr && !backbone)
  {
    struct ospf_area_config *ac = cfg_allocz(sizeof(struct ospf_area_config));
    ac->type = OPT_E; /* Backbone is non-stub */
    add_head(&cf->area_list, NODE ac);
    init_list(&ac->patt_list);
    init_list(&ac->net_list);
    init_list(&ac->enet_list);
    init_list(&ac->stubnet_list);
  }

  if (!cf->abr && !EMPTY_LIST(cf->vlink_list))
    cf_error("Vlinks cannot be used on single area router");

  if (cf->asbr && (areano == 1) && (this_area->type == 0))
    cf_error("ASBR must be in non-stub area");
}

static inline void
ospf_check_defcost(int cost)
{
  if ((cost <= 0) || (cost >= LSINFINITY))
   cf_error("Default cost must be in range 1-%d", LSINFINITY-1);
}

static inline void
ospf_check_auth(void)
{
  if (ospf_cfg_is_v3())
    cf_error("Authentication not supported in OSPFv3");
}


/* Headers from ../../proto/pipe/config.Y */

#include "proto/pipe/pipe.h"

/* Defines from ../../proto/pipe/config.Y */

#define PIPE_CFG ((struct pipe_config *) this_proto)

/* Headers from ../../proto/rip/config.Y */

#include "proto/rip/rip.h"
#include "nest/iface.h"

/* Defines from ../../proto/rip/config.Y */

#define RIP_CFG ((struct rip_config *) this_proto)
#define RIP_IFACE ((struct rip_iface_config *) this_ipatt)

static inline int rip_cfg_is_v2(void) { return RIP_CFG->rip2; }
static inline int rip_cfg_is_ng(void) { return ! RIP_CFG->rip2; }

static inline void
rip_check_auth(void)
{
  if (rip_cfg_is_ng())
    cf_error("Authentication not supported in RIPng");
}


/* Headers from ../../proto/static/config.Y */

#include "proto/static/static.h"

/* Defines from ../../proto/static/config.Y */

#define STATIC_CFG ((struct static_config *) this_proto)
static struct static_route *this_srt, *this_srt_nh, *last_srt_nh;
static struct f_inst **this_srt_last_cmd;

static void
static_route_finish(void)
{
  struct static_route *r;

  /* Update undefined use_bfd entries in multipath nexthops */
  if (this_srt->dest == RTD_MULTIPATH)
    for (r = this_srt->mp_next; r; r = r->mp_next)
      if (r->use_bfd < 0)
        r->use_bfd = this_srt->use_bfd;
}


#line 612 "cf-parse.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "cf-parse.tab.h".  */
#ifndef YY_CF_CF_PARSE_TAB_H_INCLUDED
# define YY_CF_CF_PARSE_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int cf_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    END = 258,
    CLI_MARKER = 259,
    INVALID_TOKEN = 260,
    ELSECOL = 261,
    DDOT = 262,
    GEQ = 263,
    LEQ = 264,
    NEQ = 265,
    AND = 266,
    OR = 267,
    PO = 268,
    PC = 269,
    NUM = 270,
    ENUM = 271,
    RTRID = 272,
    IPA = 273,
    SYM = 274,
    TEXT = 275,
    PREFIX_DUMMY = 276,
    DEFINE = 277,
    ON = 278,
    OFF = 279,
    YES = 280,
    NO = 281,
    S = 282,
    MS = 283,
    US = 284,
    PORT = 285,
    LOG = 286,
    SYSLOG = 287,
    ALL = 288,
    DEBUG = 289,
    TRACE = 290,
    INFO = 291,
    REMOTE = 292,
    WARNING = 293,
    ERROR = 294,
    AUTH = 295,
    FATAL = 296,
    BUG = 297,
    STDERR = 298,
    SOFT = 299,
    TIMEFORMAT = 300,
    ISO = 301,
    OLD = 302,
    SHORT = 303,
    LONG = 304,
    BASE = 305,
    NAME = 306,
    CONFIRM = 307,
    UNDO = 308,
    CHECK = 309,
    TIMEOUT = 310,
    LATENCY = 311,
    LIMIT = 312,
    WATCHDOG = 313,
    CONFIGURE = 314,
    DOWN = 315,
    KERNEL = 316,
    PERSIST = 317,
    SCAN = 318,
    TIME = 319,
    LEARN = 320,
    DEVICE = 321,
    ROUTES = 322,
    GRACEFUL = 323,
    RESTART = 324,
    KRT_SOURCE = 325,
    KRT_METRIC = 326,
    MERGE = 327,
    PATHS = 328,
    TABLE = 329,
    KRT_PREFSRC = 330,
    KRT_REALM = 331,
    KRT_MTU = 332,
    KRT_WINDOW = 333,
    KRT_RTT = 334,
    KRT_RTTVAR = 335,
    KRT_SSTRESH = 336,
    KRT_CWND = 337,
    KRT_ADVMSS = 338,
    KRT_REORDERING = 339,
    KRT_HOPLIMIT = 340,
    KRT_INITCWND = 341,
    KRT_RTO_MIN = 342,
    KRT_INITRWND = 343,
    KRT_QUICKACK = 344,
    KRT_LOCK_MTU = 345,
    KRT_LOCK_WINDOW = 346,
    KRT_LOCK_RTT = 347,
    KRT_LOCK_RTTVAR = 348,
    KRT_LOCK_SSTRESH = 349,
    KRT_LOCK_CWND = 350,
    KRT_LOCK_ADVMSS = 351,
    KRT_LOCK_REORDERING = 352,
    KRT_LOCK_HOPLIMIT = 353,
    KRT_LOCK_RTO_MIN = 354,
    KRT_FEATURE_ECN = 355,
    KRT_FEATURE_ALLFRAG = 356,
    ROUTER = 357,
    ID = 358,
    PROTOCOL = 359,
    TEMPLATE = 360,
    PREFERENCE = 361,
    DISABLED = 362,
    DIRECT = 363,
    INTERFACE = 364,
    IMPORT = 365,
    EXPORT = 366,
    FILTER = 367,
    NONE = 368,
    STATES = 369,
    FILTERS = 370,
    RECEIVE = 371,
    ACTION = 372,
    WARN = 373,
    BLOCK = 374,
    DISABLE = 375,
    KEEP = 376,
    FILTERED = 377,
    PASSWORD = 378,
    FROM = 379,
    PASSIVE = 380,
    TO = 381,
    EVENTS = 382,
    PACKETS = 383,
    PROTOCOLS = 384,
    INTERFACES = 385,
    PRIMARY = 386,
    STATS = 387,
    COUNT = 388,
    FOR = 389,
    COMMANDS = 390,
    PREEXPORT = 391,
    NOEXPORT = 392,
    GENERATE = 393,
    ROA = 394,
    LISTEN = 395,
    BGP = 396,
    V6ONLY = 397,
    DUAL = 398,
    ADDRESS = 399,
    PASSWORDS = 400,
    DESCRIPTION = 401,
    SORTED = 402,
    RELOAD = 403,
    IN = 404,
    OUT = 405,
    MRTDUMP = 406,
    MESSAGES = 407,
    RESTRICT = 408,
    MEMORY = 409,
    IGP_METRIC = 410,
    CLASS = 411,
    DSCP = 412,
    WAIT = 413,
    MAX = 414,
    FLUSH = 415,
    AS = 416,
    SHOW = 417,
    STATUS = 418,
    SUMMARY = 419,
    ROUTE = 420,
    SYMBOLS = 421,
    ADD = 422,
    DELETE = 423,
    DUMP = 424,
    RESOURCES = 425,
    SOCKETS = 426,
    NEIGHBORS = 427,
    ATTRIBUTES = 428,
    EVAL = 429,
    ECHO = 430,
    ENABLE = 431,
    FUNCTION = 432,
    PRINT = 433,
    PRINTN = 434,
    UNSET = 435,
    RETURN = 436,
    ACCEPT = 437,
    REJECT = 438,
    QUITBIRD = 439,
    INT = 440,
    BOOL = 441,
    IP = 442,
    PREFIX = 443,
    PAIR = 444,
    QUAD = 445,
    EC = 446,
    SET = 447,
    STRING = 448,
    BGPMASK = 449,
    BGPPATH = 450,
    CLIST = 451,
    ECLIST = 452,
    IF = 453,
    THEN = 454,
    ELSE = 455,
    CASE = 456,
    TRUE = 457,
    FALSE = 458,
    RT = 459,
    RO = 460,
    UNKNOWN = 461,
    GENERIC = 462,
    GW = 463,
    NET = 464,
    MASK = 465,
    PROTO = 466,
    SOURCE = 467,
    SCOPE = 468,
    CAST = 469,
    DEST = 470,
    IFNAME = 471,
    IFINDEX = 472,
    LEN = 473,
    DEFINED = 474,
    CONTAINS = 475,
    RESET = 476,
    PREPEND = 477,
    FIRST = 478,
    LAST = 479,
    LAST_NONAGGREGATED = 480,
    MATCH = 481,
    ROA_CHECK = 482,
    EMPTY = 483,
    WHERE = 484,
    BFD = 485,
    MIN = 486,
    IDLE = 487,
    RX = 488,
    TX = 489,
    INTERVAL = 490,
    MULTIPLIER = 491,
    MULTIHOP = 492,
    NEIGHBOR = 493,
    DEV = 494,
    LOCAL = 495,
    SESSIONS = 496,
    HOLD = 497,
    CONNECT = 498,
    RETRY = 499,
    KEEPALIVE = 500,
    STARTUP = 501,
    VIA = 502,
    NEXT = 503,
    HOP = 504,
    SELF = 505,
    DEFAULT = 506,
    PATH = 507,
    METRIC = 508,
    START = 509,
    DELAY = 510,
    FORGET = 511,
    AFTER = 512,
    BGP_PATH = 513,
    BGP_LOCAL_PREF = 514,
    BGP_MED = 515,
    BGP_ORIGIN = 516,
    BGP_NEXT_HOP = 517,
    BGP_ATOMIC_AGGR = 518,
    BGP_AGGREGATOR = 519,
    BGP_COMMUNITY = 520,
    BGP_EXT_COMMUNITY = 521,
    RR = 522,
    RS = 523,
    CLIENT = 524,
    CLUSTER = 525,
    AS4 = 526,
    ADVERTISE = 527,
    IPV4 = 528,
    CAPABILITIES = 529,
    PREFER = 530,
    OLDER = 531,
    MISSING = 532,
    LLADDR = 533,
    DROP = 534,
    IGNORE = 535,
    REFRESH = 536,
    INTERPRET = 537,
    COMMUNITIES = 538,
    BGP_ORIGINATOR_ID = 539,
    BGP_CLUSTER_LIST = 540,
    IGP = 541,
    GATEWAY = 542,
    RECURSIVE = 543,
    MED = 544,
    TTL = 545,
    SECURITY = 546,
    DETERMINISTIC = 547,
    SECONDARY = 548,
    ALLOW = 549,
    AWARE = 550,
    LINK = 551,
    EXTENDED = 552,
    SETKEY = 553,
    OSPF = 554,
    AREA = 555,
    OSPF_METRIC1 = 556,
    OSPF_METRIC2 = 557,
    OSPF_TAG = 558,
    OSPF_ROUTER_ID = 559,
    RFC1583COMPAT = 560,
    STUB = 561,
    TICK = 562,
    COST = 563,
    COST2 = 564,
    RETRANSMIT = 565,
    HELLO = 566,
    TRANSMIT = 567,
    PRIORITY = 568,
    DEAD = 569,
    TYPE = 570,
    BROADCAST = 571,
    BCAST = 572,
    NONBROADCAST = 573,
    NBMA = 574,
    POINTOPOINT = 575,
    PTP = 576,
    POINTOMULTIPOINT = 577,
    PTMP = 578,
    SIMPLE = 579,
    AUTHENTICATION = 580,
    STRICT = 581,
    CRYPTOGRAPHIC = 582,
    ELIGIBLE = 583,
    POLL = 584,
    NETWORKS = 585,
    HIDDEN = 586,
    VIRTUAL = 587,
    ONLY = 588,
    BUFFER = 589,
    LARGE = 590,
    NORMAL = 591,
    STUBNET = 592,
    TAG = 593,
    EXTERNAL = 594,
    LSADB = 595,
    ECMP = 596,
    WEIGHT = 597,
    NSSA = 598,
    TRANSLATOR = 599,
    STABILITY = 600,
    GLOBAL = 601,
    LSID = 602,
    INSTANCE = 603,
    REAL = 604,
    NETMASK = 605,
    LENGTH = 606,
    LSA = 607,
    SUPPRESSION = 608,
    TOPOLOGY = 609,
    STATE = 610,
    PIPE = 611,
    PEER = 612,
    MODE = 613,
    OPAQUE = 614,
    TRANSPARENT = 615,
    RIP = 616,
    INFINITY = 617,
    UPDATE = 618,
    GARBAGE = 619,
    MULTICAST = 620,
    VERSION = 621,
    SPLIT = 622,
    HORIZON = 623,
    POISON = 624,
    REVERSE = 625,
    ZERO = 626,
    PLAINTEXT = 627,
    MD5 = 628,
    RIP_METRIC = 629,
    RIP_TAG = 630,
    STATIC = 631,
    PROHIBIT = 632,
    MULTIPATH = 633,
    BLACKHOLE = 634,
    UNREACHABLE = 635
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 541 "cf-parse.y" /* yacc.c:355  */

  int i;
  u32 i32;
  ip_addr a;
  struct symbol *s;
  char *t;
  struct rtable_config *r;
  struct f_inst *x;
  struct filter *f;
  struct f_tree *e;
  struct f_trie *trie;
  struct f_val v;
  struct f_path_mask *h;
  struct password_item *p;
  struct rt_show_data *ra;
  struct roa_show_data *ro;
  struct sym_show_data *sd;
  struct lsadb_show_data *ld;
  struct iface *iface;
  struct roa_table *rot;
  void *g;
  bird_clock_t time;
  struct prefix px;
  struct proto_spec ps;
  struct timeformat *tf;

#line 1060 "cf-parse.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE cf_lval;

int cf_parse (void);

#endif /* !YY_CF_CF_PARSE_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 1075 "cf-parse.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  70
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   3183

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  402
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  266
/* YYNRULES -- Number of rules.  */
#define YYNRULES  908
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  1707

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   635

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,     2,     2,    30,     2,     2,
     393,   394,    28,    26,   398,    27,    32,    29,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   395,   392,
      23,    22,    24,   399,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   400,     2,   401,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   396,     2,   397,    25,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   140,   141,   142,   143,   144,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   213,   214,   215,
     216,   217,   218,   219,   220,   221,   222,   223,   224,   225,
     226,   227,   228,   229,   230,   231,   232,   233,   234,   235,
     236,   237,   238,   239,   240,   241,   242,   243,   244,   245,
     246,   247,   248,   249,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   305,
     306,   307,   308,   309,   310,   311,   312,   313,   314,   315,
     316,   317,   318,   319,   320,   321,   322,   323,   324,   325,
     326,   327,   328,   329,   330,   331,   332,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,   351,   352,   353,   354,   355,
     356,   357,   358,   359,   360,   361,   362,   363,   364,   365,
     366,   367,   368,   369,   370,   371,   372,   373,   374,   375,
     376,   377,   378,   379,   380,   381,   382,   383,   384,   385,
     386,   387,   388,   389,   390,   391
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   731,   731,   732,   735,   737,   745,   754,   755,   756,
     763,   764,   765,   773,   774,   775,   776,   777,   778,   784,
     785,   792,   793,   797,   804,   805,   809,   813,   820,   828,
     829,   836,   837,   843,   852,   853,   857,   862,   863,   867,
     868,   872,   873,   877,   878,   879,   880,   881,   882,   883,
     884,   885,   891,   892,   902,   903,   904,   905,   908,   909,
     910,   911,   912,   913,   917,   923,   924,   925,   926,   934,
     937,   944,   947,   950,   953,   957,   958,   962,   963,   964,
     972,   977,   978,   982,   989,   990,   991,   992,   998,  1003,
    1007,  1021,  1037,  1038,  1042,  1043,  1044,  1045,  1052,  1066,
    1068,  1070,  1074,  1075,  1076,  1077,  1082,  1088,  1089,  1093,
    1101,  1105,  1107,  1113,  1114,  1121,  1122,  1126,  1132,  1136,
    1145,  1154,  1156,  1160,  1161,  1162,  1163,  1164,  1165,  1166,
    1167,  1168,  1169,  1170,  1171,  1175,  1176,  1177,  1178,  1182,
    1183,  1184,  1185,  1186,  1190,  1196,  1200,  1208,  1209,  1217,
    1225,  1226,  1227,  1231,  1232,  1236,  1241,  1242,  1246,  1248,
    1256,  1260,  1261,  1267,  1274,  1275,  1276,  1277,  1281,  1289,
    1295,  1296,  1297,  1301,  1302,  1306,  1307,  1308,  1309,  1310,
    1311,  1317,  1318,  1319,  1323,  1324,  1328,  1329,  1335,  1336,
    1339,  1341,  1345,  1346,  1350,  1368,  1369,  1370,  1371,  1372,
    1373,  1381,  1384,  1387,  1390,  1394,  1395,  1398,  1401,  1405,
    1409,  1414,  1420,  1427,  1432,  1437,  1442,  1446,  1450,  1454,
    1463,  1471,  1475,  1482,  1483,  1484,  1489,  1493,  1500,  1507,
    1511,  1519,  1520,  1521,  1526,  1530,  1533,  1534,  1535,  1536,
    1537,  1538,  1539,  1544,  1549,  1557,  1564,  1571,  1579,  1581,
    1583,  1585,  1587,  1589,  1591,  1593,  1596,  1600,  1606,  1607,
    1608,  1612,  1613,  1619,  1621,  1623,  1625,  1627,  1629,  1633,
    1637,  1640,  1644,  1645,  1646,  1650,  1651,  1652,  1660,  1660,
    1670,  1674,  1675,  1676,  1677,  1678,  1679,  1680,  1681,  1682,
    1683,  1684,  1685,  1686,  1707,  1718,  1719,  1726,  1727,  1734,
    1743,  1747,  1751,  1775,  1776,  1780,  1793,  1793,  1809,  1810,
    1813,  1814,  1818,  1821,  1830,  1843,  1844,  1845,  1846,  1847,
    1851,  1859,  1860,  1861,  1862,  1863,  1867,  1870,  1871,  1872,
    1876,  1879,  1888,  1891,  1892,  1893,  1894,  1898,  1899,  1900,
    1904,  1905,  1906,  1907,  1911,  1912,  1913,  1914,  1918,  1919,
    1923,  1924,  1928,  1935,  1936,  1937,  1938,  1945,  1946,  1949,
    1950,  1957,  1969,  1970,  1974,  1975,  1979,  1980,  1981,  1982,
    1983,  1984,  1988,  1989,  1990,  1994,  1995,  1996,  1997,  1998,
    1999,  2000,  2001,  2002,  2003,  2004,  2008,  2009,  2019,  2023,
    2046,  2060,  2061,  2062,  2063,  2064,  2065,  2066,  2067,  2068,
    2069,  2073,  2074,  2075,  2076,  2077,  2078,  2079,  2080,  2081,
    2082,  2083,  2084,  2085,  2086,  2087,  2088,  2090,  2091,  2092,
    2094,  2096,  2098,  2100,  2101,  2102,  2103,  2104,  2105,  2115,
    2116,  2117,  2118,  2119,  2120,  2121,  2123,  2124,  2129,  2152,
    2153,  2154,  2155,  2156,  2157,  2161,  2164,  2165,  2166,  2174,
    2181,  2190,  2191,  2195,  2201,  2211,  2220,  2226,  2231,  2238,
    2243,  2249,  2250,  2251,  2259,  2260,  2261,  2262,  2263,  2269,
    2281,  2282,  2283,  2284,  2287,  2289,  2293,  2297,  2308,  2309,
    2310,  2311,  2312,  2313,  2316,  2318,  2321,  2323,  2326,  2329,
    2334,  2335,  2336,  2340,  2341,  2345,  2346,  2349,  2368,  2374,
    2399,  2401,  2402,  2406,  2407,  2408,  2409,  2410,  2411,  2417,
    2418,  2419,  2420,  2421,  2422,  2423,  2424,  2425,  2426,  2427,
    2428,  2429,  2430,  2431,  2432,  2433,  2434,  2435,  2436,  2437,
    2438,  2439,  2440,  2441,  2442,  2443,  2444,  2445,  2446,  2447,
    2448,  2449,  2450,  2451,  2452,  2453,  2454,  2455,  2461,  2462,
    2463,  2464,  2465,  2466,  2467,  2468,  2469,  2470,  2471,  2472,
    2473,  2474,  2475,  2486,  2496,  2497,  2501,  2502,  2503,  2504,
    2505,  2506,  2507,  2508,  2509,  2512,  2527,  2530,  2532,  2536,
    2537,  2538,  2539,  2540,  2541,  2542,  2543,  2544,  2545,  2545,
    2546,  2546,  2547,  2548,  2549,  2553,  2554,  2558,  2566,  2568,
    2572,  2573,  2574,  2578,  2579,  2582,  2584,  2587,  2588,  2589,
    2590,  2591,  2592,  2593,  2594,  2595,  2596,  2597,  2600,  2620,
    2621,  2622,  2623,  2624,  2625,  2626,  2627,  2628,  2629,  2630,
    2631,  2632,  2633,  2634,  2635,  2636,  2637,  2638,  2639,  2640,
    2641,  2642,  2643,  2644,  2645,  2646,  2647,  2648,  2649,  2650,
    2651,  2652,  2653,  2654,  2655,  2656,  2657,  2658,  2661,  2663,
    2666,  2668,  2677,  2679,  2680,  2683,  2685,  2689,  2690,  2693,
    2703,  2724,  2726,  2730,  2730,  2733,  2735,  2738,  2740,  2744,
    2748,  2749,  2754,  2757,  2760,  2765,  2768,  2773,  2776,  2780,
    2784,  2787,  2788,  2789,  2790,  2791,  2792,  2793,  2794,  2800,
    2807,  2808,  2809,  2814,  2815,  2821,  2834,  2835,  2836,  2837,
    2838,  2841,  2843,  2847,  2851,  2874,  2890,  2891,  2892,  2893,
    2894,  2895,  2896,  2900,  2901,  2902,  2903,  2904,  2905,  2906,
    2907,  2908,  2909,  2910,  2911,  2912,  2913,  2914,  2915,  2916,
    2917,  2921,  2922,  2923,  2924,  2927,  2929,  2932,  2934,  2938,
    2944,  2947,  2954,  2961,  2962,  2963,  2964,  2965,  2968,  2978,
    2987,  2991,  2997,  2998,  3002,  3007,  3013,  3016,  3021,  3022,
    3023,  3024,  3025,  3029,  3030,  3033,  3035,  3038,  3040,  3044,
    3051,  3051,  3051,  3051,  3051,  3051,  3051,  3051,  3051,  3051,
    3051,  3051,  3051,  3051,  3051,  3051,  3052,  3052,  3052,  3052,
    3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,
    3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,
    3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,
    3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,  3052,
    3052,  3052,  3052,  3052,  3052,  3053,  3053,  3053,  3053,  3053,
    3053,  3053,  3053,  3053,  3054,  3054,  3054,  3054,  3055,  3055,
    3055,  3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,
    3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,
    3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,  3056,
    3056,  3057,  3057,  3058,  3059,  3060,  3061,  3062,  3063,  3064,
    3065,  3066,  3067,  3068,  3068,  3068,  3068,  3068,  3068
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "END", "CLI_MARKER", "INVALID_TOKEN",
  "ELSECOL", "DDOT", "GEQ", "LEQ", "NEQ", "AND", "OR", "PO", "PC", "NUM",
  "ENUM", "RTRID", "IPA", "SYM", "TEXT", "PREFIX_DUMMY", "'='", "'<'",
  "'>'", "'~'", "'+'", "'-'", "'*'", "'/'", "'%'", "'!'", "'.'", "DEFINE",
  "ON", "OFF", "YES", "NO", "S", "MS", "US", "PORT", "LOG", "SYSLOG",
  "ALL", "DEBUG", "TRACE", "INFO", "REMOTE", "WARNING", "ERROR", "AUTH",
  "FATAL", "BUG", "STDERR", "SOFT", "TIMEFORMAT", "ISO", "OLD", "SHORT",
  "LONG", "BASE", "NAME", "CONFIRM", "UNDO", "CHECK", "TIMEOUT", "LATENCY",
  "LIMIT", "WATCHDOG", "CONFIGURE", "DOWN", "KERNEL", "PERSIST", "SCAN",
  "TIME", "LEARN", "DEVICE", "ROUTES", "GRACEFUL", "RESTART", "KRT_SOURCE",
  "KRT_METRIC", "MERGE", "PATHS", "TABLE", "KRT_PREFSRC", "KRT_REALM",
  "KRT_MTU", "KRT_WINDOW", "KRT_RTT", "KRT_RTTVAR", "KRT_SSTRESH",
  "KRT_CWND", "KRT_ADVMSS", "KRT_REORDERING", "KRT_HOPLIMIT",
  "KRT_INITCWND", "KRT_RTO_MIN", "KRT_INITRWND", "KRT_QUICKACK",
  "KRT_LOCK_MTU", "KRT_LOCK_WINDOW", "KRT_LOCK_RTT", "KRT_LOCK_RTTVAR",
  "KRT_LOCK_SSTRESH", "KRT_LOCK_CWND", "KRT_LOCK_ADVMSS",
  "KRT_LOCK_REORDERING", "KRT_LOCK_HOPLIMIT", "KRT_LOCK_RTO_MIN",
  "KRT_FEATURE_ECN", "KRT_FEATURE_ALLFRAG", "ROUTER", "ID", "PROTOCOL",
  "TEMPLATE", "PREFERENCE", "DISABLED", "DIRECT", "INTERFACE", "IMPORT",
  "EXPORT", "FILTER", "NONE", "STATES", "FILTERS", "RECEIVE", "ACTION",
  "WARN", "BLOCK", "DISABLE", "KEEP", "FILTERED", "PASSWORD", "FROM",
  "PASSIVE", "TO", "EVENTS", "PACKETS", "PROTOCOLS", "INTERFACES",
  "PRIMARY", "STATS", "COUNT", "FOR", "COMMANDS", "PREEXPORT", "NOEXPORT",
  "GENERATE", "ROA", "LISTEN", "BGP", "V6ONLY", "DUAL", "ADDRESS",
  "PASSWORDS", "DESCRIPTION", "SORTED", "RELOAD", "IN", "OUT", "MRTDUMP",
  "MESSAGES", "RESTRICT", "MEMORY", "IGP_METRIC", "CLASS", "DSCP", "WAIT",
  "MAX", "FLUSH", "AS", "SHOW", "STATUS", "SUMMARY", "ROUTE", "SYMBOLS",
  "ADD", "DELETE", "DUMP", "RESOURCES", "SOCKETS", "NEIGHBORS",
  "ATTRIBUTES", "EVAL", "ECHO", "ENABLE", "FUNCTION", "PRINT", "PRINTN",
  "UNSET", "RETURN", "ACCEPT", "REJECT", "QUITBIRD", "INT", "BOOL", "IP",
  "PREFIX", "PAIR", "QUAD", "EC", "SET", "STRING", "BGPMASK", "BGPPATH",
  "CLIST", "ECLIST", "IF", "THEN", "ELSE", "CASE", "TRUE", "FALSE", "RT",
  "RO", "UNKNOWN", "GENERIC", "GW", "NET", "MASK", "PROTO", "SOURCE",
  "SCOPE", "CAST", "DEST", "IFNAME", "IFINDEX", "LEN", "DEFINED",
  "CONTAINS", "RESET", "PREPEND", "FIRST", "LAST", "LAST_NONAGGREGATED",
  "MATCH", "ROA_CHECK", "EMPTY", "WHERE", "BFD", "MIN", "IDLE", "RX", "TX",
  "INTERVAL", "MULTIPLIER", "MULTIHOP", "NEIGHBOR", "DEV", "LOCAL",
  "SESSIONS", "HOLD", "CONNECT", "RETRY", "KEEPALIVE", "STARTUP", "VIA",
  "NEXT", "HOP", "SELF", "DEFAULT", "PATH", "METRIC", "START", "DELAY",
  "FORGET", "AFTER", "BGP_PATH", "BGP_LOCAL_PREF", "BGP_MED", "BGP_ORIGIN",
  "BGP_NEXT_HOP", "BGP_ATOMIC_AGGR", "BGP_AGGREGATOR", "BGP_COMMUNITY",
  "BGP_EXT_COMMUNITY", "RR", "RS", "CLIENT", "CLUSTER", "AS4", "ADVERTISE",
  "IPV4", "CAPABILITIES", "PREFER", "OLDER", "MISSING", "LLADDR", "DROP",
  "IGNORE", "REFRESH", "INTERPRET", "COMMUNITIES", "BGP_ORIGINATOR_ID",
  "BGP_CLUSTER_LIST", "IGP", "GATEWAY", "RECURSIVE", "MED", "TTL",
  "SECURITY", "DETERMINISTIC", "SECONDARY", "ALLOW", "AWARE", "LINK",
  "EXTENDED", "SETKEY", "OSPF", "AREA", "OSPF_METRIC1", "OSPF_METRIC2",
  "OSPF_TAG", "OSPF_ROUTER_ID", "RFC1583COMPAT", "STUB", "TICK", "COST",
  "COST2", "RETRANSMIT", "HELLO", "TRANSMIT", "PRIORITY", "DEAD", "TYPE",
  "BROADCAST", "BCAST", "NONBROADCAST", "NBMA", "POINTOPOINT", "PTP",
  "POINTOMULTIPOINT", "PTMP", "SIMPLE", "AUTHENTICATION", "STRICT",
  "CRYPTOGRAPHIC", "ELIGIBLE", "POLL", "NETWORKS", "HIDDEN", "VIRTUAL",
  "ONLY", "BUFFER", "LARGE", "NORMAL", "STUBNET", "TAG", "EXTERNAL",
  "LSADB", "ECMP", "WEIGHT", "NSSA", "TRANSLATOR", "STABILITY", "GLOBAL",
  "LSID", "INSTANCE", "REAL", "NETMASK", "LENGTH", "LSA", "SUPPRESSION",
  "TOPOLOGY", "STATE", "PIPE", "PEER", "MODE", "OPAQUE", "TRANSPARENT",
  "RIP", "INFINITY", "UPDATE", "GARBAGE", "MULTICAST", "VERSION", "SPLIT",
  "HORIZON", "POISON", "REVERSE", "ZERO", "PLAINTEXT", "MD5", "RIP_METRIC",
  "RIP_TAG", "STATIC", "PROHIBIT", "MULTIPATH", "BLACKHOLE", "UNREACHABLE",
  "';'", "'('", "')'", "':'", "'{'", "'}'", "','", "'?'", "'['", "']'",
  "$accept", "config", "conf_entries", "definition", "expr", "expr_us",
  "bool", "ipa", "ipa_scope", "prefix", "prefix_or_ipa", "pxlen",
  "datetime", "text", "text_or_none", "log_config", "syslog_name",
  "log_file", "log_mask", "log_mask_list", "log_cat", "mrtdump_base",
  "timeformat_which", "timeformat_spec", "timeformat_base", "debug_unix",
  "cmd_CONFIGURE", "cmd_CONFIGURE_SOFT", "cmd_CONFIGURE_CONFIRM",
  "cmd_CONFIGURE_UNDO", "cmd_CONFIGURE_CHECK", "cmd_DOWN", "cfg_name",
  "cfg_timeout", "kern_proto_start", "kern_item", "kif_proto_start",
  "kif_item", "kern_sys_item", "rtrid", "idval", "listen", "listen_opts",
  "listen_opt", "gr_opts", "tab_sorted", "newtab", "roa_table_start",
  "roa_table_opts", "roa_table", "proto_start", "proto_name", "proto_item",
  "imexport", "limit_action", "limit_spec", "rtable", "debug_default",
  "iface_patt_node_init", "iface_patt_node_body", "iface_negate",
  "iface_patt_node", "iface_patt_list", "iface_patt_list_nopx",
  "iface_patt_init", "iface_patt", "tos", "dev_proto_start", "dev_proto",
  "dev_iface_init", "dev_iface_patt", "debug_mask", "debug_list",
  "debug_flag", "mrtdump_mask", "mrtdump_list", "mrtdump_flag",
  "password_list", "password_items", "password_item",
  "password_item_begin", "password_item_params", "cmd_SHOW_STATUS",
  "cmd_SHOW_MEMORY", "cmd_SHOW_PROTOCOLS", "cmd_SHOW_PROTOCOLS_ALL",
  "optsym", "cmd_SHOW_INTERFACES", "cmd_SHOW_INTERFACES_SUMMARY",
  "cmd_SHOW_ROUTE", "r_args", "export_mode", "cmd_SHOW_ROA", "roa_args",
  "roa_mode", "cmd_SHOW_SYMBOLS", "sym_args", "roa_table_arg",
  "cmd_ADD_ROA", "cmd_DELETE_ROA", "cmd_FLUSH_ROA", "cmd_DUMP_RESOURCES",
  "cmd_DUMP_SOCKETS", "cmd_DUMP_EVENTS", "cmd_DUMP_INTERFACES",
  "cmd_DUMP_NEIGHBORS", "cmd_DUMP_ATTRIBUTES", "cmd_DUMP_ROUTES",
  "cmd_DUMP_PROTOCOLS", "cmd_EVAL", "cmd_ECHO", "echo_mask", "echo_size",
  "cmd_DISABLE", "cmd_ENABLE", "cmd_RESTART", "cmd_RELOAD",
  "cmd_RELOAD_IN", "cmd_RELOAD_OUT", "cmd_DEBUG", "cmd_MRTDUMP",
  "cmd_RESTRICT", "proto_patt", "proto_patt2", "filter_def", "$@1",
  "filter_eval", "type", "one_decl", "decls", "declsn", "filter_body",
  "filter", "where_filter", "function_params", "function_body",
  "function_def", "$@2", "cmds", "cmds_int", "block", "fipa", "set_atom",
  "switch_atom", "pair_expr", "pair_atom", "pair_item", "ec_expr",
  "ec_kind", "ec_item", "set_item", "switch_item", "set_items",
  "switch_items", "fprefix_s", "fprefix", "fprefix_set", "switch_body",
  "bgp_path_expr", "bgp_path", "bgp_path_tail1", "bgp_path_tail2",
  "constant", "constructor", "rtadot", "function_call", "symbol",
  "static_attr", "term", "break_command", "print_one", "print_list",
  "var_listn", "var_list", "cmd", "bfd_proto_start", "bfd_proto_item",
  "bfd_proto_opts", "bfd_proto", "bfd_iface_start", "bfd_iface_item",
  "bfd_iface_opts", "bfd_iface_opt_list", "bfd_iface", "bfd_multihop",
  "bfd_neigh_iface", "bfd_neigh_local", "bfd_neigh_multihop",
  "bfd_neighbor", "cmd_SHOW_BFD_SESSIONS", "bgp_proto_start",
  "bgp_nbr_opts", "bgp_proto", "ospf_proto_start", "ospf_proto",
  "ospf_proto_item", "ospf_area_start", "ospf_area", "ospf_area_opts",
  "ospf_area_item", "$@3", "$@4", "ospf_stubnet", "ospf_stubnet_start",
  "ospf_stubnet_opts", "ospf_stubnet_item", "ospf_vlink",
  "ospf_vlink_opts", "ospf_vlink_item", "ospf_vlink_start",
  "ospf_iface_item", "pref_list", "pref_item", "pref_base", "pref_opt",
  "nbma_list", "nbma_eligible", "nbma_item", "ospf_iface_start",
  "ospf_instance_id", "ospf_iface_patt_list", "$@5", "ospf_iface_opts",
  "ospf_iface_opt_list", "ospf_iface", "opttext", "cmd_SHOW_OSPF",
  "cmd_SHOW_OSPF_NEIGHBORS", "cmd_SHOW_OSPF_INTERFACE",
  "cmd_SHOW_OSPF_TOPOLOGY", "cmd_SHOW_OSPF_TOPOLOGY_ALL",
  "cmd_SHOW_OSPF_STATE", "cmd_SHOW_OSPF_STATE_ALL", "cmd_SHOW_OSPF_LSADB",
  "lsadb_args", "pipe_proto_start", "pipe_proto", "rip_proto_start",
  "rip_proto_item", "rip_proto_opts", "rip_proto", "rip_iface_start",
  "rip_iface_finish", "rip_iface_item", "rip_auth", "rip_iface_opts",
  "rip_iface_opt_list", "rip_iface", "cmd_SHOW_RIP_INTERFACES",
  "cmd_SHOW_RIP_NEIGHBORS", "static_proto_start", "static_proto",
  "stat_route0", "stat_multipath1", "stat_multipath", "stat_route",
  "stat_route_item", "stat_route_opts", "stat_route_opt_list",
  "cmd_SHOW_STATIC", "conf", "cli_cmd", "proto", "kern_proto", "kif_proto",
  "dynamic_attr", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,    61,    60,    62,   126,    43,    45,    42,    47,
      37,    33,    46,   277,   278,   279,   280,   281,   282,   283,
     284,   285,   286,   287,   288,   289,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   308,   309,   310,   311,   312,   313,
     314,   315,   316,   317,   318,   319,   320,   321,   322,   323,
     324,   325,   326,   327,   328,   329,   330,   331,   332,   333,
     334,   335,   336,   337,   338,   339,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,   351,   352,   353,
     354,   355,   356,   357,   358,   359,   360,   361,   362,   363,
     364,   365,   366,   367,   368,   369,   370,   371,   372,   373,
     374,   375,   376,   377,   378,   379,   380,   381,   382,   383,
     384,   385,   386,   387,   388,   389,   390,   391,   392,   393,
     394,   395,   396,   397,   398,   399,   400,   401,   402,   403,
     404,   405,   406,   407,   408,   409,   410,   411,   412,   413,
     414,   415,   416,   417,   418,   419,   420,   421,   422,   423,
     424,   425,   426,   427,   428,   429,   430,   431,   432,   433,
     434,   435,   436,   437,   438,   439,   440,   441,   442,   443,
     444,   445,   446,   447,   448,   449,   450,   451,   452,   453,
     454,   455,   456,   457,   458,   459,   460,   461,   462,   463,
     464,   465,   466,   467,   468,   469,   470,   471,   472,   473,
     474,   475,   476,   477,   478,   479,   480,   481,   482,   483,
     484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
     494,   495,   496,   497,   498,   499,   500,   501,   502,   503,
     504,   505,   506,   507,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   518,   519,   520,   521,   522,   523,
     524,   525,   526,   527,   528,   529,   530,   531,   532,   533,
     534,   535,   536,   537,   538,   539,   540,   541,   542,   543,
     544,   545,   546,   547,   548,   549,   550,   551,   552,   553,
     554,   555,   556,   557,   558,   559,   560,   561,   562,   563,
     564,   565,   566,   567,   568,   569,   570,   571,   572,   573,
     574,   575,   576,   577,   578,   579,   580,   581,   582,   583,
     584,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   598,   599,   600,   601,   602,   603,
     604,   605,   606,   607,   608,   609,   610,   611,   612,   613,
     614,   615,   616,   617,   618,   619,   620,   621,   622,   623,
     624,   625,   626,   627,   628,   629,   630,   631,   632,   633,
     634,   635,    59,    40,    41,    58,   123,   125,    44,    63,
      91,    93
};
# endif

#define YYPACT_NINF -1409

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-1409)))

#define YYTABLE_NINF -452

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-452)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     146,  2933,   118,   760,   799,   504,   230,   799,   799,   720,
     799,   237,    94,  1447,   158,   189,   958,  1493,    69,   799,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409,   273,   507,   338,   357,   177,   302,   368,   282,
   -1409, -1409,   411,   351,   317,   101,  1493,   455, -1409, -1409,
   -1409, -1409, -1409, -1409,   154,   154, -1409, -1409, -1409, -1409,
      89, -1409,   883, -1409,   154,  1419, -1409, -1409, -1409,   154,
   -1409,   154,  2611,   154,  2166,   154,  2192,   154, -1409,   154,
    2381, -1409, -1409,  2263,  2637, -1409, -1409, -1409,    79, -1409,
     480,   502,   522,   480,   448, -1409,   592,   606,   799,   799,
     608,   112, -1409,   488,   830,   116, -1409,   623,   628, -1409,
   -1409,   381,   391,   124,   616,   306,   306,   636,   662,   672,
     685,   688,   691,   706,   729,    34, -1409, -1409, -1409,   675,
     348, -1409,   514,    43,    -2,  1493, -1409,   350,   374,   376,
   -1409, -1409,   382,   388,   402,  1376,    42, -1409, -1409, -1409,
   -1409, -1409,  2307, -1409,  2236, -1409, -1409,  1013,   744,   794,
     779, -1409, -1409,   750, -1409, -1409,   -12,   644,    79,   271,
   -1409, -1409, -1409, -1409,   673,   425,   271,   271,   659,   682,
     114, -1409,   812, -1409,   112,   441,  2675, -1409,   711,   833,
     459,   473, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409,   475,    79,   575,   865,   772,   271,   336, -1409,
     724,   690,   828,   695,   112, -1409,   508,   510,   527,   529,
     -44,   634,   870,   573,   936,   700,   695,   336,   902,   896,
     288,   832,   336,     1,   306,   181,   908,  -168,   915,   743,
     717,   451,   736,   738,   481,   725,   722,   336,   721,   726,
     715,    77,   -33,   749,   718,   727,   336,   774,   336, -1409,
     630,   633,   676,   142,   336,   920,   271,   336,   924, -1409,
   -1409,   648,   649, -1409,   650,   956,   465, -1409,   652,   651,
     653,   741,   306,   992, -1409,   689,   261,   684,   998,   336,
    1014,   336,  1010,  1017,  1009, -1409,   708,   709,   712,  1028,
    1087, -1409,   716,   719, -1409, -1409,   893,  1106,   448, -1409,
   -1409,  1109,   271,  1116, -1409, -1409,  1117,  1118, -1409, -1409,
   -1409,   215,  1122,  1107,  1124, -1409, -1409,   842,  1125, -1409,
    1126,   752, -1409, -1409,  2883,   851,   616, -1409,   616,   616,
   -1409,   364,   370,  1128,   616,   616,  1129, -1409, -1409,     0,
     964,   965, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
      24, -1409,    34,  1493,    34,    34,  1123, -1409,  1121,   872,
    1120,   904,  1127,    -2,    -2,  1115,  1119,  1493,  1493,  1493,
    1493,  1493,  1137, -1409, -1409,  1132, -1409,   759,   155, -1409,
   -1409, -1409, -1409,   900, -1409,  1141, -1409, -1409, -1409,    48,
      49, -1409,    82, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409,  1493,  1493,  1493,  1493,  1493,  1493,
    1493,  1493,  1493,  1493,  1493,  1493,  1493,   643, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409,   468, -1409, -1409,
    1155, -1409,  1493,   695, -1409, -1409,  1013,   767, -1409, -1409,
   -1409, -1409, -1409, -1409,   271,  1493, -1409, -1409, -1409, -1409,
     271,   849,   878, -1409,   855, -1409, -1409,   271, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409,  1493,   782,  2869, -1409,    75,
     792, -1409,   773,  1167, -1409, -1409, -1409,  -116, -1409, -1409,
     336, -1409, -1409,   142, -1409, -1409, -1409, -1409,   265,  2466,
   -1409,  1055,  1493, -1409, -1409,   265, -1409,   265, -1409, -1409,
   -1409, -1409, -1409, -1409,  1133,  1135,   336,   528, -1409,   803,
    1146,   805,   806,   271,   521,   926,   336,  1037,   306,   815,
   -1409,   838,  1202,    55,   271,  1061,   271,  1160,  1161,   271,
    1162,   -58,   271,   271,   336,  1163,   847,  1130,   854,   336,
     856,   336,   457,   336,   865,   336,   857,   859,   336,   336,
     336,   860,  1082,   867, -1409, -1409,   336, -1409, -1409,   336,
   -1409,  1192,   271, -1409, -1409, -1409,  1242,   871,   876, -1409,
   -1409, -1409,   336, -1409,   865, -1409, -1409,   886, -1409,   306,
   -1409,  1006, -1409, -1409, -1409,   882,   271, -1409,   271, -1409,
     336,   336,   336, -1409, -1409, -1409,   271, -1409,   306, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409,   549, -1409, -1409,
    1273, -1409, -1409, -1409, -1409, -1409, -1409, -1409,   561, -1409,
   -1409, -1409, -1409,  1275, -1409, -1409, -1409,  1261, -1409, -1409,
    1266,   306, -1409, -1409,  1264,  1265, -1409,  2466, -1409, -1409,
   -1409, -1409,   306, -1409, -1409, -1409,  1268, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409,  1282,  1269,  1269,   979,
     616,  1269,   616,  1269, -1409,  1269,  1269, -1409,   271,   306,
   -1409,  1276,  1277,  1279, -1409, -1409,   798, -1409, -1409, -1409,
   -1409,    56, -1409,   901, -1409,  1281, -1409, -1409, -1409, -1409,
     294,   335,   590,  1153,   645,   -19, -1409,  1493, -1409,  1493,
   -1409,  1289,   906,   912,  1619,   136,   173, -1409, -1409, -1409,
    1291,  1293, -1409,  2335,  2335,  2335,  3030,  3030,  2335,  2335,
    2335,  2335,   771,   771,  1119,  1119, -1409,   919, -1409, -1409,
   -1409, -1409, -1409,  1013, -1409,  2080, -1409,   564, -1409, -1409,
    1657,  1295, -1409, -1409, -1409, -1409, -1409, -1409, -1409,   921,
   -1409,   925,  1692, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409,    53,   927,   922, -1409,
   -1409,   271, -1409, -1409,   306, -1409, -1409, -1409,  2789,  2869,
   -1409,   306, -1409,   928, -1409,  1296, -1409,   918, -1409,  1199,
   -1409, -1409, -1409, -1409,   336,  2675, -1409, -1409,   822,   271,
     271,   937,   271,   938,   940, -1409,   336, -1409, -1409,   943,
     946,   948,   953,   336,   954,   336,   959, -1409, -1409,  1309,
   -1409,   271,   271, -1409,   961,   271,   962,   271,   271,   963,
     271,   966,   967,   968,   969,   971,   271, -1409,   142, -1409,
     974, -1409,   975,   980,   988,   989,   990,   991,   993, -1409,
   -1409,   994,   995,  1019, -1409,   268, -1409, -1409, -1409,   271,
   -1409,  2494,  1020, -1409, -1409,  2394,  1021,  1022, -1409,  1202,
   -1409,   306,    -3,  1006,    23, -1409, -1409, -1409, -1409, -1409,
    1288, -1409,     0, -1409, -1409, -1409,   893, -1409, -1409,   215,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409,  1354,  1395, -1409, -1409,   142, -1409, -1409,   142,  1386,
   -1409,   142,  1269,  1406,  1269,  1414,  1415,  1418, -1409, -1409,
    1251,  1253,    34, -1409,  1493, -1409,  1402,  1493,  1493,  1493,
   -1409,  1493, -1409,  1493,   674,  1724,  1493,  1186,  1493, -1409,
   -1409,  1493, -1409, -1409,  1036,   675, -1409,  1493, -1409, -1409,
   -1409, -1409, -1409, -1409,   918, -1409, -1409, -1409, -1409,  2869,
     249, -1409, -1409, -1409,  1043,  1042, -1409,  1272, -1409, -1409,
     914, -1409,   647, -1409, -1409, -1409, -1409,   306, -1409, -1409,
    1046,  1041,  1051, -1409,  1052, -1409, -1409,  1053, -1409, -1409,
   -1409, -1409,  1054, -1409,  1056, -1409, -1409,    81, -1409, -1409,
   -1409,  1057, -1409,  1059,  1062, -1409,  1069, -1409, -1409, -1409,
   -1409, -1409,  1070,  1071, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409,  1073, -1409, -1409,   336,
     322,   109, -1409,  1140,   306, -1409, -1409,   701, -1409,  1076,
   -1409,  1111, -1409, -1409,   336,   271, -1409, -1409,  1080, -1409,
   -1409, -1409,  1202,   336,   271,    -3,    16, -1409,  1451, -1409,
   -1409,  1089,  1493, -1409, -1409, -1409,  1493,  1493,   336, -1409,
    2307,  1083,  1440, -1409, -1409,   271, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409,  1480, -1409,  1483, -1409, -1409, -1409,
    1476,  1479, -1409, -1409, -1409,  1755,  1780,  1819,  1847,   762,
    1493, -1409, -1409,  2675,  1102,  1099,  2675,  1619,  1485,  1872,
   -1409,  1101,    84, -1409,  2869, -1409,  1486,   306, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409,  1131, -1409,    47,
   -1409, -1409,   271, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,   271,
     271,   336,   271, -1409,  1134,   142, -1409, -1409,  1142,  1147,
     271, -1409, -1409,   271,  1148, -1409, -1409,  1434, -1409, -1409,
   -1409, -1409, -1409,  1493,   872,  1493, -1409,  2105,  2926,    83,
    1113,  1481,   366, -1409,  2675,  1144,  1136, -1409, -1409, -1409,
     488,   488, -1409, -1409, -1409, -1409,  1493,  1897,  1500,  1461,
    1151, -1409, -1409, -1409, -1409,  1343, -1409,   918,  1131, -1409,
   -1409,  1497,   695,  1274,  1139,   918,  1156, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,  1164,   271,
    2130,  1168,  2155,  2543, -1409,   978, -1409, -1409,  1493,  1493,
     511,  1440, -1409,  1514,  1520,  1922, -1409,  1172,  1173,    12,
   -1409,  1511, -1409,   401, -1409, -1409,   306,  1285, -1409,  1111,
   -1409, -1409,    18,   -48,    32,  1228, -1409, -1409, -1409, -1409,
   -1409, -1409,  1174,   249,  1318, -1409,    29,  2180,  2206,  1176,
    1177,  1178,  1179,  1169, -1409, -1409, -1409, -1409,  1493, -1409,
    1493, -1409,  1181,   336,   728,  1321,   271,   271, -1409,  1182,
   -1409,   336, -1409, -1409,  2640, -1409, -1409, -1409,  -138,   336,
     271,   336, -1409,  1183, -1409,   695,  1184,   271,   271,   271,
    1311,   183,   171, -1409, -1409, -1409,  1187,  1190,  2216, -1409,
    1203,  1201,   978,   120, -1409, -1409, -1409,   900, -1409, -1409,
    1589, -1409, -1409, -1409,   143, -1409, -1409,  1493,  1493,  1493,
    1493, -1409,  1204,  1205, -1409, -1409,  1355,  1357,  1358, -1409,
   -1409, -1409, -1409,  1298,   271,  1212,   336,  1255,   -26,  1313,
     336,  1248,   336,   271,   271,   271,  1347,   271,   315,   895,
    1256,   204,  1297,   271,  1267,  1305, -1409, -1409,  1230, -1409,
     271,  1241, -1409, -1409, -1409, -1409, -1409,  1501, -1409, -1409,
   -1409,   271,   271, -1409, -1409, -1409, -1409,   311, -1409,   271,
    -110,  1559,   336,   306,   336,  1292,   363,   271,  1334,    -7,
    1299,  -156,  1563,  1574,   318,  1271,  1278, -1409, -1409,  1263,
   -1409, -1409, -1409, -1409,  1947,   178,   120,   191,  1972,  1997,
    2030,  2055,  1493, -1409,   271,   271,   271,   336, -1409, -1409,
   -1409,   244,   271,   271,   271,   271, -1409,   570, -1409,  1294,
   -1409, -1409, -1409, -1409,   271, -1409,   271, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409,   336, -1409, -1409, -1409,
     336, -1409,   271,   336, -1409, -1409, -1409,  1259,  1270, -1409,
   -1409,   271,    87,   397,  1260, -1409,   336,   336,   271, -1409,
   -1409, -1409,   271,   271,   271, -1409, -1409,   823, -1409, -1409,
   -1409, -1409, -1409,   271, -1409, -1409,   271,   271,   336, -1409,
     336,   336, -1409, -1409,  1493, -1409, -1409, -1409,  1284,  1300,
    1301,  1303,  1283, -1409, -1409, -1409, -1409,    37, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409,  1316, -1409,   336, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409,  1501,  1304,  1641,  1641,  1641,
    1641, -1409, -1409, -1409, -1409, -1409, -1409, -1409,  1319, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409,  1947, -1409, -1409, -1409,
   -1409, -1409, -1409,  1325, -1409, -1409, -1409, -1409,   311, -1409,
    1306,  1317,  1320,  1330, -1409, -1409,  1333, -1409,   311,   311,
     311,   311, -1409, -1409, -1409, -1409, -1409
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       4,     0,     0,     0,     0,    75,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   388,     0,     0,
     796,   797,   798,   799,   800,   801,   802,   803,   804,   805,
     806,   807,   808,   809,   810,   811,   812,   813,   814,   815,
     816,   817,   818,   819,   820,   821,   822,   823,   824,   825,
     826,   827,   828,   829,   830,   831,   832,   833,   834,   835,
     836,   837,   838,   839,   840,   841,   842,   843,   844,     3,
       1,     2,     0,     0,     0,     0,     0,     0,     0,     0,
     115,   116,     0,     0,     0,     0,   388,     0,   780,   781,
     782,   783,   784,   785,   117,   117,   786,   787,   788,   789,
     113,   790,     0,   792,   117,   121,   793,   794,   795,   117,
     848,   117,   121,   117,   121,   117,   121,   117,   852,   117,
     121,     5,   791,   121,   121,   272,   274,   273,     0,    76,
      75,     0,     0,    75,    77,    74,     0,     0,     0,     0,
       0,     0,   271,   243,   276,     0,   227,     0,     0,   210,
     235,     0,   206,     0,   206,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   371,   375,   384,   381,   314,
     390,   378,     0,     0,   374,   388,   420,     0,     0,     0,
     376,   377,     0,     0,     0,   388,     0,   379,   380,   385,
     418,   419,     0,   417,     0,   259,   258,     0,   261,     0,
       0,    30,    29,    35,    38,    36,     0,    18,     0,     0,
      57,    56,    55,    54,     0,     0,     0,     0,     0,   107,
       0,   278,     0,   100,     0,     0,   280,   306,   118,     0,
       0,     0,   111,    80,    88,   163,   499,   469,   563,   699,
     705,   752,     0,     0,     0,     0,     0,     0,    18,   168,
       0,     0,     0,     0,     0,   847,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,     0,     0,
       0,     0,    18,     0,   500,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,     0,     0,
       0,     0,     0,     0,     0,     0,    18,     0,    18,   849,
       0,     0,     0,     0,    18,     0,     0,    18,     0,   850,
     566,     0,     0,   574,     0,     0,     0,   851,     0,     0,
       0,     0,     0,     0,   853,     0,     0,   777,     0,    18,
       0,    18,     0,     0,     0,   845,     0,     0,     0,     0,
      32,   846,     0,     0,   171,   170,     0,     0,    77,    71,
      72,     0,    78,     0,   265,   263,     0,     0,   266,   182,
     181,     0,     0,     0,     0,   275,   277,   276,     0,   207,
       0,   231,   202,   201,     0,     0,   206,   205,   206,   206,
     690,   206,   206,     0,   206,   206,     0,    19,    20,     0,
       0,     0,   254,   250,   255,   251,   248,   249,   252,   253,
     371,   390,   371,   388,   371,   371,     0,   362,     0,   388,
       0,     0,     0,   374,   374,     0,   415,   388,   388,   388,
     388,   388,     0,   333,   334,     0,   336,     0,     0,   315,
     318,   316,   320,   388,   317,   342,   340,   341,   348,     0,
     353,   357,     0,   891,   861,   862,   863,   864,   865,   866,
     867,   868,   869,   870,   871,   872,   873,   874,   875,   876,
     877,   878,   879,   880,   881,   882,   883,   884,   885,   886,
     887,   888,   889,   391,   890,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   893,   896,   895,   892,   894,   897,
     898,   899,   902,   900,   901,   903,   904,   905,   906,   907,
     908,   421,   422,   256,   388,   388,   388,   388,   388,   388,
     388,   388,   388,   388,   388,   388,   388,     0,    43,    44,
      45,    46,    47,    48,    49,    50,    51,     0,    41,   262,
       0,   264,   388,     0,    37,    39,     0,     0,     7,     9,
      14,    16,    15,    17,     0,   388,    13,    65,   147,   148,
      58,     0,     0,    64,     0,    67,    68,     0,   108,   109,
      94,    96,    97,    98,   159,   388,     0,   295,   110,     0,
       0,    53,     0,     0,   119,   854,   858,     0,   164,   124,
      18,   146,   132,     0,   122,   123,   149,   137,     0,   295,
     138,     0,   388,   126,   136,     0,   127,     0,   134,   125,
     165,   166,   474,   503,     0,     0,    18,    18,   515,     0,
       0,     0,     0,     0,    18,     0,    18,     0,     0,     0,
     516,     0,    21,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    18,     0,     0,     0,     0,    18,
       0,    18,     0,    18,     0,    18,     0,     0,    18,    18,
      18,     0,     0,     0,   504,   564,    18,   575,   567,    18,
     572,   569,     0,   565,   577,   700,     0,     0,     0,   701,
     711,   753,    18,   758,     0,   754,   769,     0,   768,     0,
     772,     0,   770,   771,   775,     0,     0,    81,     0,    83,
      18,    18,    18,   856,   857,   855,     0,    31,     0,   860,
     859,   176,   175,   177,   179,   180,   178,     0,   173,   269,
       0,    73,    79,    69,   267,   268,   186,   187,     0,   184,
     270,   244,   247,     0,   203,   208,   226,     0,   233,   232,
       0,     0,   209,   216,     0,     0,   224,   295,   218,   217,
     221,   222,     0,   223,   225,   211,     0,   215,   234,   242,
     236,   239,   240,   238,   241,   237,     0,   681,   681,     0,
     206,   681,   206,   681,   682,   681,   681,   779,     0,     0,
      23,     0,     0,     0,   366,   368,     0,   369,   370,   364,
     352,   449,   452,     0,   429,     0,   430,   372,   373,   365,
       0,     0,     0,     0,     0,     0,   335,   388,   401,   388,
     329,   327,     0,     0,   326,     0,     0,   382,   354,   355,
       0,     0,   383,   413,   411,   409,   406,   407,   408,   410,
     412,   414,   402,   403,   404,   405,   423,     0,   424,   426,
     427,   428,   260,     0,   257,     0,    34,     0,    33,    66,
       0,     0,    60,    61,    62,    63,    10,    11,    12,     0,
     149,     0,     0,    92,   281,   282,   283,   284,   285,   286,
     287,   288,   289,   290,   291,   292,     0,     0,     0,   279,
     299,     0,   104,   105,     0,    99,   101,    52,     0,   295,
     120,     0,   114,     0,   133,   153,   156,   169,   145,   139,
     129,   300,   301,   135,    18,   302,   130,   128,   121,     0,
       0,     0,     0,     0,     0,   509,    18,   545,   548,     0,
       0,     0,     0,    18,     0,    18,     0,   562,   517,     0,
     500,     0,     0,   507,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   511,     0,   512,
       0,   543,     0,     0,     0,     0,     0,     0,     0,   523,
     524,     0,     0,     0,   550,     0,   546,   571,   568,     0,
     573,     0,     0,   703,   704,   121,     0,     0,   765,    21,
     767,     0,   762,   766,   388,   757,    91,    82,    84,    85,
      86,    89,    25,    24,    90,   172,     0,    70,   183,     0,
     204,   230,   229,   228,   213,   220,   214,   212,   219,   498,
     680,     0,     0,   689,   698,     0,   696,   693,     0,     0,
     691,     0,   681,     0,   681,     0,     0,     0,    26,    27,
       0,     0,   371,   363,   388,   438,     0,   388,   388,   388,
     416,   388,   436,   388,     0,     0,   388,   388,   388,   319,
     314,   388,   343,   349,     0,     0,   358,   388,    42,     6,
      40,     8,    59,   106,   160,    93,    95,   294,   293,   295,
     388,   103,   102,   304,   297,     0,   307,     0,   167,   154,
       0,   149,     0,   144,   131,   477,   477,     0,   476,   470,
       0,     0,     0,   561,     0,   557,   556,     0,   547,   551,
     552,   553,     0,   541,     0,   532,    22,     0,   501,   502,
     505,     0,   513,     0,     0,   536,     0,   519,   518,   531,
     530,   525,     0,     0,   544,   528,   520,   521,   522,   549,
     559,   527,   526,   560,   529,   554,     0,   570,   670,    18,
       0,    18,   588,     0,     0,   590,   580,    18,   576,     0,
     594,   671,   702,   714,    18,     0,   713,   706,     0,   755,
     756,   764,    21,    18,     0,   763,     0,   442,     0,   443,
     444,     0,   388,   440,   441,   439,   388,   388,    18,   778,
       0,     0,   388,   773,   776,     0,   174,   185,   684,   683,
     697,   692,   694,   695,     0,   685,     0,   687,   750,   751,
       0,     0,   367,   450,   431,     0,     0,     0,     0,     0,
     388,   386,   328,   326,     0,     0,   332,     0,     0,     0,
     296,     0,   388,   310,     0,   303,     0,   150,   151,   155,
     157,   142,   140,   141,   143,   149,   471,   486,   472,   490,
     473,   475,     0,   537,   558,   539,   540,   542,   508,   506,
     535,   534,   514,   533,   510,   555,   149,   593,   581,     0,
       0,    18,     0,   579,     0,     0,   597,   592,   596,     0,
       0,   586,   578,     0,   604,   149,   710,   707,   709,   712,
     759,   761,   760,   388,   388,   388,   388,     0,     0,     0,
       0,     0,     0,   462,   445,   447,     0,    87,   686,   688,
     243,   243,   435,   433,   434,   432,   388,     0,   330,   388,
       0,   425,   305,   311,   298,     0,   152,   158,   486,   484,
     489,     0,     0,   493,     0,   673,   677,   583,   584,   582,
     585,   658,   618,   598,   658,   587,   672,   605,   747,     0,
       0,     0,     0,     0,   456,   388,   359,   774,   388,   388,
       0,   388,   461,     0,     0,     0,   387,     0,     0,     0,
     356,     0,   488,     0,   491,   492,     0,   495,   538,   671,
     675,   679,     0,     0,     0,   607,   745,   715,   708,   455,
     389,   459,     0,   388,   453,   312,     0,     0,     0,     0,
       0,     0,     0,     0,   448,   245,   246,   437,   388,   339,
     388,   337,     0,    18,     0,     0,     0,     0,   487,     0,
     494,    18,   497,   674,     0,   589,   661,   659,   662,    18,
       0,    18,   595,     0,   591,     0,     0,     0,     0,     0,
       0,     0,     0,   603,   617,   189,   193,     0,     0,   749,
       0,     0,   388,   388,   321,   325,   323,   388,   463,   324,
     346,   344,   345,   350,     0,   458,   457,   388,   388,   388,
     388,   464,     0,     0,   112,   483,     0,     0,     0,   478,
     482,   485,   496,     0,     0,     0,    18,     0,     0,     0,
      18,     0,    18,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   678,   657,     0,   663,
       0,     0,   601,   602,   600,   599,   194,   190,   611,   609,
     608,     0,     0,   612,   614,   615,   616,   195,   606,     0,
       0,     0,    18,     0,    18,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   748,   740,     0,
     460,   313,   454,   361,   326,     0,   388,     0,     0,     0,
       0,     0,   388,   338,     0,     0,     0,    18,   623,   665,
     655,     0,     0,     0,     0,     0,   650,    18,   656,     0,
     639,   619,   622,   620,     0,   637,     0,   624,   626,   627,
     628,   629,   630,   631,   632,   633,    18,   644,   645,   646,
      18,   621,     0,    18,   676,   664,   660,     0,     0,   610,
     613,     0,     0,     0,     0,   721,    18,    18,     0,   719,
     720,   738,     0,     0,     0,   733,   716,    18,   741,   743,
     742,   744,   739,     0,   718,   717,     0,     0,    18,   722,
      18,    18,   746,   322,   388,   347,   360,   351,     0,     0,
       0,     0,     0,   479,   480,   481,   640,     0,   648,   647,
     649,   161,   162,   651,   652,     0,   653,    18,   636,   625,
     635,   638,   641,   634,   188,   190,     0,     0,     0,     0,
       0,   192,   737,   726,   728,   731,   734,   732,     0,   735,
     730,   727,   729,   723,   724,   725,     0,   468,   466,   467,
     465,   331,   643,   667,   666,   654,   642,   191,   195,    28,
       0,     0,     0,     0,   736,   668,     0,   200,   195,   195,
     195,   195,   669,   196,   197,   198,   199
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1409, -1409, -1409, -1409,  -176,  -202,   -62,  -266,  -921,  -153,
    -716, -1409,  -706,   -81, -1409, -1409, -1409, -1409, -1409,  1154,
     864, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409,   494,  1339, -1409, -1409, -1409, -1409, -1409, -1409,
    -298, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409,  1158,  -102,  1454, -1409,   -15,  -128, -1409, -1409, -1409,
   -1409,   637,  -580,   445, -1409, -1409,   195, -1409, -1409, -1409,
   -1409,   148, -1409,   742,   369, -1409,   740,  -997,    58, -1408,
   -1409,  -646, -1409, -1409, -1409, -1409,  -124, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409,  -315, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409,   984,  1360, -1409, -1409, -1409, -1409,  -824,   671,
     523,  1171,  1002,  1366, -1409,   862, -1409, -1409, -1085, -1409,
     310,  -185,   939,   208, -1019,   723, -1202, -1151,  1560, -1157,
     949,   217, -1409, -1409,  -179,   947, -1409, -1409, -1409, -1409,
    -377,   572, -1409, -1409,  -911, -1409,  -154,   589,   -17, -1409,
   -1409,   420,   746,   497,  -950, -1409, -1409, -1409, -1409,   686,
   -1409, -1409,   460, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409,   852, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409,   449, -1409, -1409, -1409, -1409, -1409, -1409, -1409,   415,
   -1409, -1409, -1409, -1409, -1409,  -295, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409,   802, -1409, -1409, -1409, -1409, -1409, -1409,
   -1409, -1409, -1409, -1409, -1409,  -995
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,    89,   546,   555,   547,   389,   920,   983,
     984,   770,  1690,   205,   698,    90,   534,   206,   537,   527,
     528,    91,   214,   215,    92,    93,    20,    21,    22,    23,
      24,    25,   134,   353,    94,   336,    95,   342,   337,    96,
     566,    97,   569,   876,    98,   559,    99,   100,   577,   101,
     102,   230,   256,   593,  1073,   890,   582,   103,   885,  1219,
    1070,   886,  1307,  1308,   850,   851,  1556,   104,   105,   586,
     257,   347,   707,   708,   362,   718,   719,  1424,  1587,  1425,
    1426,  1594,    26,    27,    28,    29,   383,    30,    31,    32,
     374,   746,    33,   371,   731,    34,   375,   364,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,   198,   530,    48,    49,    50,    51,    52,    53,    54,
      55,    56,   128,   368,   106,   567,   107,   866,   867,   868,
    1065,   892,   893,   594,   879,   870,   108,   572,  1211,  1212,
    1374,   187,   435,  1440,   801,   802,   436,  1205,   803,   437,
     438,  1443,   439,  1444,   188,   441,   442,  1376,   405,   189,
     406,   415,   190,   191,   192,  1171,   193,   501,  1203,  1172,
    1285,  1286,   782,   783,  1213,   109,  1080,   898,   110,  1225,
    1399,  1353,  1310,  1226,  1228,  1313,  1357,  1402,  1230,    57,
     111,   623,   112,   113,   114,   311,   312,   313,   961,  1139,
    1254,  1259,  1257,  1258,  1363,  1413,  1140,  1365,  1427,  1141,
    1488,  1362,  1407,  1408,  1491,  1637,  1696,  1684,  1246,  1264,
    1316,  1359,  1404,  1361,  1247,  1001,    58,    59,    60,    61,
      62,    63,    64,    65,   759,   115,   116,   117,  1148,   965,
     118,  1265,  1429,  1529,  1612,  1428,  1367,  1266,    66,    67,
     119,   120,   326,   972,   973,   327,  1174,   974,   685,    68,
     121,    69,   122,   123,   124,   502
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     194,   434,   390,   391,   225,   657,   887,   440,   622,   625,
     300,   407,   310,   413,   318,   556,   538,  1202,   325,  1390,
     539,   338,   343,   774,  1173,   775,   997,   777,   778,   768,
     386,   773,   535,   549,   881,  1433,   387,   388,  1273,   400,
     554,   554,  1156,   401,  1434,  1435,  1436,  1040,  1151,   400,
     387,   388,   402,   401,  1064,   387,   388,   429,   430,   431,
     169,   432,   402,  1170,   504,   505,   506,   507,   508,   226,
     411,   584,  1057,  1157,   931,   808,   809,  1311,   509,   510,
     511,   512,   513,   514,   515,   516,   646,   627,   517,  1588,
    -309,   504,   505,   506,   507,   508,   921,   621,   628,  -309,
    -309,  -309,  -309,  1156,   195,   509,   510,   511,   512,   513,
     514,   515,   516,   196,   344,   517,   871,  1608,    70,   369,
     201,   202,   921,   345,   538,   604,  -308,  1409,   539,   560,
     660,   561,   562,   563,  1157,  -308,  -308,  -308,  -308,  1156,
    1158,  1552,  1553,   540,   541,   542,   543,   359,  1349,  1170,
       1,   429,   430,   431,  1040,   432,   360,   560,   416,   561,
     562,   563,   644,   504,   505,   506,   507,   508,   428,   673,
    1157,  1614,   598,   228,  1441,  1282,   712,   509,   510,   511,
     512,   513,   514,   515,   516,   611,   585,   517,   429,   430,
     431,  1040,   432,  1434,  1435,  1436,  1040,  1596,   538,   387,
     388,  1158,   539,   932,  1489,   612,  1434,  1435,  1436,  1040,
     619,  1490,  1159,  1160,  1161,  1162,  1163,  1164,  1165,  1442,
    1615,   745,  1657,   605,  1658,   640,   216,   922,   872,   873,
     874,  1270,  1166,   135,   651,  1167,   653,  1158,  1153,  1453,
     142,   224,   658,   217,   143,   661,   407,  1588,   407,   564,
     407,   407,   756,   922,   757,   758,  1058,   761,   763,   538,
     765,   766,  1303,   539,  1168,   384,   647,   687,  1156,   689,
    1054,  1410,  1597,  1159,  1160,  1161,  1162,  1163,  1164,  1165,
     538,   882,   412,   538,   539,   884,   538,   539,  1431,   229,
     539,   370,   200,  1166,  1411,  1504,  1167,  1312,  1554,  1157,
     888,  1170,   504,   505,   506,   507,   508,   385,   155,  1159,
    1160,  1161,  1162,  1163,  1164,  1165,   509,   510,   511,   512,
     513,   514,   515,   516,   387,   388,   517,  1502,  1577,  1166,
     538,  1609,  1167,   538,   539,  1441,  1555,   539,  1372,   156,
     716,   645,   839,   504,   505,   506,   507,   508,  1533,  1412,
    1154,   538,   916,   624,  1218,   539,   548,   509,   510,   511,
     512,   513,   514,   515,   516,  1333,  1158,   517,   554,  1452,
     540,   541,   542,   543,   841,  1032,  1610,  1611,   717,  1033,
    1442,   849,   218,   377,   536,  1375,   776,   219,  1339,   377,
    1064,   579,   781,   620,   545,   769,   220,   414,  1340,   210,
     790,   791,   792,   793,   794,   207,  1391,  1487,   760,  1274,
     377,   969,   889,   970,   762,  1405,   804,   403,   211,   889,
    1169,   889,  1437,   404,  1170,  1591,  1438,   403,  1252,  1414,
     221,  1528,   982,   404,  1682,   433,   222,   909,  1159,  1160,
    1161,  1162,  1163,  1164,  1165,   810,   806,   923,   924,   807,
     926,  1626,   836,   929,  1024,   676,   933,   934,  1166,  1566,
    1592,  1167,  1170,  1002,   615,   197,  1013,   875,  1015,   223,
    1016,  1017,   212,  1238,   227,   346,   982,  -309,   208,  1336,
     811,  -309,  1375,   812,   209,   232,   960,   813,   814,   815,
     816,   817,   818,   819,   820,   821,   822,   823,   824,   825,
     129,  1306,   545,  1019,  1593,   349,  1505,   565,   361,  1506,
     976,   378,   977,  -308,   352,   835,   947,  -308,   883,   677,
     981,  1170,  1170,  1632,   129,   350,   201,   202,   840,  1041,
    1552,  1553,  1659,   213,  1660,   565,   538,  1393,  1536,  1578,
     539,  1537,  1579,   538,   901,   904,   967,   539,   852,   798,
     203,   678,   912,   799,   914,   540,   541,   542,   543,   130,
     679,   204,   540,   541,   542,   543,   433,   131,   132,   133,
     616,  1624,   935,   363,   379,   895,   545,   940,   993,   942,
     896,   946,   897,   948,  1437,   538,   951,   952,   953,   539,
    1638,  1639,  1018,   570,   957,   354,   617,   958,   504,   505,
     506,   507,   508,   902,   540,   541,   542,   543,  1062,   355,
     966,   358,   509,   510,   511,   512,   513,   514,   515,   516,
     434,   434,   517,   599,   348,  1170,   372,   351,   978,   979,
     980,   373,   440,   376,  1379,   377,  1012,   545,  1014,   392,
    1113,  1249,  1250,  1394,  1395,  1192,  -308,  1396,  1397,   680,
     681,   682,   683,   504,   505,   506,   507,   508,   545,   538,
    1125,   545,  1618,   539,   545,   393,  1315,   509,   510,   511,
     512,   513,   514,   515,   516,   394,  1251,   517,   540,   541,
     542,   543,   504,   505,   506,   507,   508,  1603,   395,  1380,
    1381,   396,  1027,   550,   397,  1061,   509,   510,   511,   512,
     513,   514,   515,   516,   408,  1152,   517,  1180,   545,   398,
    1181,   545,   544,  1183,   201,   202,   538,  1184,   943,  1186,
     539,   632,   633,  1081,  1082,  1604,  1084,  1221,  1067,   545,
     551,   552,   399,  1028,   587,   540,   541,   542,   543,   125,
     126,   409,   380,   417,  1382,  1098,  1099,   944,   945,  1101,
    1383,  1103,  1104,   410,  1106,   726,   381,   382,   595,   529,
    1112,   636,   637,    71,   127,   910,   911,   418,   587,   419,
     504,   505,   506,   507,   508,   420,  1222,  1223,  1224,  1126,
    1034,   421,  1035,  1127,   509,   510,   511,   512,   513,   514,
     515,   516,   588,    72,   517,   422,  1079,   531,  1398,   515,
     516,   532,    73,   517,   982,    74,   504,   505,   506,   507,
     508,  1229,   533,   589,   590,  1645,    75,   553,   125,   126,
     509,   510,   511,   512,   513,   514,   515,   516,   557,    76,
     517,   568,  1074,   571,   903,   667,   668,   727,   538,    77,
     558,   826,   539,   127,  1087,    78,   573,   589,   590,   365,
     366,  1092,   574,  1094,   748,   575,   591,   540,   541,   542,
     543,   365,   366,  1147,   827,   832,   833,   243,   407,   576,
     749,   578,   828,    79,   367,    80,    81,   829,   830,   831,
     138,   139,   580,    82,   581,   165,   583,   166,   167,   168,
     169,   170,   171,   846,   847,   848,   597,   728,   172,   173,
     600,   174,   601,   175,   387,   388,   968,   245,   842,   843,
      83,    84,   729,   165,   545,   166,   167,   168,   169,   170,
     171,   545,    85,   602,   730,   603,   172,   173,   800,   174,
     592,   175,   387,   388,  1217,   246,   750,   844,   845,   247,
     248,   606,  1075,   250,   251,    86,   985,   986,    87,   252,
     607,   982,  1691,  1692,  1693,   233,   609,  1322,   988,   989,
     234,  1050,   833,   545,   592,   608,   751,   752,   610,  1268,
     613,   701,  1456,  1457,   753,  1343,  1344,   631,  1272,   253,
     614,  1256,  1003,   626,   254,   787,   788,   618,  1029,   176,
     629,   136,   137,   140,   141,   177,   630,  1156,  1004,  1287,
     634,   754,   235,   199,   635,   638,   639,   781,   641,   643,
    1195,  1196,  1197,   648,  1198,   642,  1199,   176,   702,   703,
     649,  1206,   654,   177,  1207,   652,   656,   650,  1157,   655,
    1209,   704,   705,   659,   706,   236,   157,   545,   662,   755,
     663,   666,  1697,  1031,   669,   664,   665,   670,   672,   671,
     178,   179,  1703,  1704,  1705,  1706,  1314,  1260,   518,   519,
     520,   521,   522,   523,   524,   525,   526,  1248,  1668,  1253,
    1076,  1077,  1200,  1317,  1318,  1261,  1320,   674,   178,   179,
     684,   675,  1267,   686,  1325,   180,   181,  1326,   690,   688,
    1400,  1271,  1005,   692,   545,  1158,   158,   691,   159,   160,
     693,   694,   182,   696,   695,   183,  1280,   697,   699,   709,
     184,   700,   711,   180,   181,   423,   424,   425,   426,   713,
     714,   715,   356,   357,   237,   720,   721,   722,   724,   725,
     182,   764,   767,   183,   771,   772,   780,   779,   184,   161,
     162,   163,   164,   785,   789,  1277,   784,   796,   805,  1278,
    1279,   517,    88,  1368,   786,  1284,   795,   797,   834,   838,
    1296,   504,   505,   506,   507,   508,   878,  1159,  1160,  1161,
    1162,  1163,  1164,  1165,   853,   509,   510,   511,   512,   513,
     514,   515,   516,  1297,   877,   517,   880,  1166,   894,  1319,
    1167,  1439,  1023,   238,  1459,   905,   906,   907,   908,   165,
     915,   166,   167,   168,   169,   170,   171,   917,   899,  1406,
     900,  1406,   172,   173,   800,   174,   545,   175,   913,  1078,
     554,  1460,  1568,  1569,  1570,  1571,  1572,  1573,  1574,  1575,
     918,  1355,   919,   925,  1493,   927,   928,   930,   936,   937,
    1006,  1498,  1499,  1500,   938,  1503,   939,  1600,   941,   949,
     239,   950,   954,   231,   955,   240,  1330,   781,  1332,   956,
     959,   962,   242,   963,   971,   185,  -451,   258,   964,   259,
     241,   301,   186,   314,   975,   319,   987,   320,   990,  1345,
     991,   992,  1206,   994,   995,   999,  1007,   998,  1548,  1000,
    1008,  1020,  1021,   185,  1022,  1025,  1036,  1561,  1562,  1563,
     186,  1565,  1567,   176,  1037,  1009,  1044,  1581,  1026,   177,
    1038,  1045,  1047,  1053,  1585,  1052,  1071,  1055,  1060,  1059,
    1068,  1377,  1378,  1069,  1284,  1589,  1590,  1072,  1096,  1083,
    1085,  1455,  1086,  1595,  1496,  1088,  1010,  1011,  1089,  1462,
    1090,  1606,  1633,  1634,  1635,  1091,  1093,  1492,  1619,  1494,
    1439,  1095,  1439,  1100,  1102,  1105,  1175,  1178,  1107,  1108,
    1109,  1110,  1415,  1111,   178,   179,  1114,  1115,   554,   554,
     554,  1683,  1116,  1206,  1373,  1640,  1641,  1642,  1643,  1644,
    1117,  1118,  1119,  1120,  1416,  1121,  1122,  1123,  1648,   165,
    1649,   166,   167,   168,   169,   170,   171,  1417,  1179,   180,
     181,  1182,   172,   173,  1550,   174,  1652,   175,  1558,  1185,
    1560,  1124,  1142,  1149,  1150,  1656,   182,  1187,  1188,   183,
    1534,  1189,  1664,  1190,   184,  1191,  1665,  1666,  1667,  1194,
    1538,  1539,  1540,  1541,  1208,  1214,  1215,  1670,  1231,  1232,
    1671,  1672,  1216,  1233,  1234,  1235,  1236,  1255,  1237,  1239,
    1599,  1240,  1601,   165,  1241,   166,   167,   168,   169,   170,
     171,  1242,  1243,  1244,   243,  1245,   172,   173,  1262,   174,
    1263,   175,  1269,  1275,   165,  1283,   166,   167,   168,   169,
     170,   171,  1276,  1288,   244,  1636,  1289,   172,   173,  1348,
     174,  1290,   175,   176,  1291,  1646,  1298,  1299,  1302,   177,
    1300,  1305,  1329,  1338,   245,  1337,   165,  1347,   166,   167,
     168,   169,   170,   171,  1650,  1351,  1354,  1385,  1651,   172,
     173,  1653,   174,  1386,   175,  1356,  1392,  1309,  1342,  1432,
    1321,  1358,   246,  1401,  1662,  1663,   247,   248,  1323,   249,
     250,   251,  1341,  1324,  1327,  1669,   252,  1030,  1350,  1418,
    1419,  1420,  1360,  1421,   178,   179,  1673,   176,  1674,  1675,
    1366,  1451,  1370,   177,  1422,  1388,  1458,  1389,  1430,  1447,
    1448,  1449,  1450,  1454,  1461,  1495,   253,  1501,   176,   185,
    1497,   254,  1508,  1507,   177,  1686,   186,   144,   145,   180,
     181,   423,   424,   425,   426,  1530,  1535,   146,  1531,  1543,
    1551,  1544,  1542,  1545,  1546,  1547,   182,  1676,  1549,   183,
     176,  1559,   147,  1564,   184,  1557,   177,  1576,   178,   179,
    1582,   148,  1584,   149,   150,  1423,  1580,   504,   505,   506,
     507,   508,  1583,  1586,  1598,  1415,  1607,  1602,  1616,   178,
     179,   509,   510,   511,   512,   513,   514,   515,   516,  1617,
    1620,   517,  1613,   180,   181,  1622,  1654,  1661,  1647,  1621,
    1685,  1689,  1655,  1694,  1695,   504,   505,   506,   507,   508,
     182,   178,   179,   183,   180,   181,  1677,  1681,   184,   509,
     510,   511,   512,   513,   514,   515,   516,   710,   151,   517,
     837,   182,  1678,  1679,   183,  1680,  1688,  1048,  1698,   184,
     504,   505,   506,   507,   508,   596,   180,   181,  1220,  1699,
    1328,  1605,  1700,  1687,   509,   510,   511,   512,   513,   514,
     515,   516,  1701,   182,   517,  1702,   183,   723,  1176,  1177,
    1210,   184,   504,   505,   506,   507,   508,  1304,   869,   996,
     747,  1066,  1532,  1625,  1042,   427,   509,   510,   511,   512,
     513,   514,   515,   516,  1627,  1043,   517,   152,  1046,  1281,
    1204,  1384,  1227,   504,   505,   506,   507,   508,  1352,   185,
    1193,  1331,  1097,  1364,  1403,  1155,   186,   509,   510,   511,
     512,   513,   514,   515,   516,     0,     0,   517,   504,   505,
     506,   507,   508,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   509,   510,   511,   512,   513,   514,   515,   516,
       0,     0,   517,     0,     0,     0,   255,     0,     0,   153,
       0,     0,     0,     0,     0,     0,     0,   504,   505,   506,
     507,   508,  -446,   185,   154,     0,     0,     0,     0,     0,
     186,   509,   510,   511,   512,   513,   514,   515,   516,     0,
       0,   517,     0,     0,   185,   504,   505,   506,   507,   508,
       0,   186,     0,     0,     0,     0,     0,     0,     0,   509,
     510,   511,   512,   513,   514,   515,   516,     0,     0,   517,
     504,   505,   506,   507,   508,     0,   185,     0,     0,     0,
       0,     0,     0,   186,   509,   510,   511,   512,   513,   514,
     515,   516,     0,     0,   517,   504,   505,   506,   507,   508,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   509,
     510,   511,   512,   513,   514,   515,   516,     0,     0,   517,
     504,   505,   506,   507,   508,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   509,   510,   511,   512,   513,   514,
     515,   516,     0,     0,   517,   504,   505,   506,   507,   508,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   509,
     510,   511,   512,   513,   514,   515,   516,     0,     0,   517,
     504,   505,   506,   507,   508,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   509,   510,   511,   512,   513,   514,
     515,   516,     0,     0,   517,   504,   505,   506,   507,   508,
       0,     0,     0,  1039,     0,     0,     0,     0,     0,   509,
     510,   511,   512,   513,   514,   515,   516,     0,     0,   517,
       0,     0,     0,     0,     0,     0,     0,     0,   504,   505,
     506,   507,   508,     0,     0,     0,     0,     0,     0,     0,
       0,  1051,   509,   510,   511,   512,   513,   514,   515,   516,
       0,     0,   517,   504,   505,   506,   507,   508,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   509,   510,   511,
     512,   513,   514,   515,   516,     0,  1056,   517,   504,   505,
     506,   507,   508,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   509,   510,   511,   512,   513,   514,   515,   516,
       0,     0,   517,   504,   505,   506,   507,   508,  1201,     0,
       0,     0,     0,     0,     0,     0,     0,   509,   510,   511,
     512,   513,   514,   515,   516,     0,     0,   517,   504,   505,
     506,   507,   508,     0,     0,     0,     0,     0,     0,  1292,
       0,     0,   509,   510,   511,   512,   513,   514,   515,   516,
       0,     0,   517,   504,   505,   506,   507,   508,     0,     0,
       0,     0,     0,     0,  1293,     0,     0,   509,   510,   511,
     512,   513,   514,   515,   516,     0,     0,   517,   504,   505,
     506,   507,   508,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   509,   510,   511,   512,   513,   514,   515,   516,
       0,   243,   517,  1294,   504,   505,   506,   507,   508,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   509,   510,
     511,   512,   513,   514,   515,   516,     0,   243,   517,   503,
       0,  1295,     0,     0,   504,   505,   506,   507,   508,   302,
       0,   245,     0,     0,     0,     0,     0,  1509,   509,   510,
     511,   512,   513,   514,   515,   516,  1301,     0,   517,     0,
       0,     0,     0,     0,     0,     0,     0,   245,     0,   246,
       0,  1510,  1511,   247,   248,     0,     0,   250,   251,     0,
       0,  1346,     0,   252,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   246,     0,     0,   243,   247,
     248,     0,   443,   250,   251,     0,  1387,     0,     0,   252,
       0,     0,     0,   253,     0,     0,     0,     0,   254,     0,
       0,     0,     0,     0,     0,   328,   329,   330,     0,   331,
     332,  1623,   333,  -452,  -452,  -452,   334,     0,   245,   253,
    1415,     0,  1512,     0,   254,     0,     0,  -452,  -452,  -452,
    -452,   513,   514,   515,   516,     0,  1628,   517,     0,     0,
       0,  1513,  1416,     0,     0,     0,   246,     0,     0,     0,
     247,   248,     0,     0,   250,   251,     0,     0,   444,   445,
     252,  1629,     0,   446,   447,   448,   449,   450,   451,   452,
     453,   454,   455,   456,   457,   458,   459,   460,   461,   462,
     463,   464,   465,   466,   467,   468,   469,   470,   471,   472,
     253,     0,     0,     0,  1630,   254,   243,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   243,
       0,     0,   473,     0,     0,     0,   321,     0,     0,  1631,
       0,     0,     0,     0,     0,     0,     0,  1514,     0,     0,
    1515,  1516,     0,     0,     0,     0,   245,     0,     0,     0,
       0,     0,  1049,   474,     0,     0,     0,   303,     0,   245,
    1517,     0,   304,   305,   306,   891,     0,     0,     0,     0,
       0,     0,     0,     0,   246,     0,     0,  1334,   247,   248,
       0,     0,   250,   251,     0,     0,     0,   246,   252,     0,
       0,   247,   248,     0,  1143,   250,   251,  1518,   307,     0,
       0,   252,  1369,     0,     0,   308,   475,   476,     0,   477,
     478,   479,   480,   481,   482,   483,     0,     0,   253,     0,
       0,     0,     0,   254,     0,     0,     0,  1371,   443,     0,
       0,   253,  1519,     0,     0,     0,   254,   322,     0,     0,
     315,   316,     0,   309,     0,     0,     0,     0,  1520,     0,
       0,     0,  1445,     0,     0,     0,   484,   485,   486,   487,
     488,   489,   490,   491,   492,  1521,     0,     0,     0,   317,
    1522,  1523,     0,  1524,  1525,     0,  1526,     0,  1446,     0,
       0,     0,   493,   494,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1527,  1128,     0,     0,     0,     0,   495,
     496,   497,   498,     0,   444,   445,     0,     0,     0,   446,
     447,   448,   449,   450,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     467,   468,   469,   470,   471,   472,   243,     0,     0,     0,
     335,   260,   854,   855,   856,   857,   858,   859,   860,  1129,
     861,   862,   863,   864,   865,     0,   261,     0,   323,     0,
       0,     0,   243,   504,   505,   506,   507,   508,     0,     0,
     262,     0,   499,   500,     0,     0,   245,   509,   510,   511,
     512,   513,   514,   515,   516,  1463,     0,   517,     0,   474,
       0,   339,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   245,     0,   246,     0,     0,     0,   247,   248,
     263,   264,   250,   251,     0,     0,     0,     0,   252,     0,
       0,     0,   265,     0,     0,   266,  1144,   267,     0,     0,
     246,     0,     0,     0,   247,   248,  1130,     0,   250,   251,
       0,     0,     0,     0,   252,     0,     0,  1145,   253,     0,
       0,     0,     0,   254,  1415,     0,     0,     0,   324,   340,
       0,     0,     0,     0,     0,     0,     0,   268,     0,   269,
       0,  1146,     0,     0,   253,     0,  1416,     0,   270,   254,
       0,     0,     0,     0,     0,     0,     0,     0,     0,  1464,
       0,  1131,   484,   485,   486,   487,   488,   489,   490,   491,
     492,     0,     0,  1465,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   271,  1132,     0,  1133,   493,   494,
       0,     0,  1134,     0,  1135,     0,     0,     0,  1136,  1137,
       0,     0,   272,     0,     0,   495,   496,   497,   498,   273,
     274,     0,   275,     0,   276,   277,     0,   278,   279,     0,
     280,     0,     0,   281,   282,     0,   283,     0,     0,     0,
       0,  1466,     0,     0,  1467,  1468,   732,     0,     0,   284,
     285,  1138,     0,     0,   286,     0,   287,   288,     0,   289,
       0,   387,   388,     0,   290,     0,     0,     0,   291,   292,
       0,   293,   294,     0,   295,   296,   297,     0,     0,     0,
     298,     0,     0,     0,     0,     0,     0,   733,   499,   500,
       0,     0,     0,     0,   504,   505,   506,   507,   508,     0,
       0,  1469,     0,     0,  1470,     0,     0,  1471,   509,   510,
     511,   512,   513,   514,   515,   516,     0,  1472,   517,  1473,
       0,  1474,  1475,  1476,  1477,  1478,  1479,     0,   734,     0,
       0,     0,  1480,     0,     0,     0,  1481,  1482,     4,     0,
    1483,     0,     0,     0,     0,   854,   855,   856,   857,   858,
     859,   860,  1484,   861,   862,   863,   864,   865,   735,     0,
    1485,     0,     0,     5,     6,   736,   737,     0,   299,     0,
       0,     0,     0,     7,     0,     0,   738,     0,     0,     0,
       0,     0,     0,     0,     0,   739,   740,   741,   742,     0,
     743,   744,     0,     0,   341,     0,     0,  1486,   504,   505,
     506,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   509,   510,   511,   512,   513,   514,   515,   516,
       0,     0,   517,     0,     8,   854,   855,   856,   857,   858,
     859,   860,     0,   861,   862,   863,   864,   865,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     9,     0,     0,    10,     0,    11,     0,     0,
       0,     0,     0,     0,    12,     0,    13,     0,     0,     0,
       0,    14,    15,    16,     0,     0,     0,     0,    17,    18,
      19,     0,     0,   592,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1335,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,  1063
};

static const yytype_int16 yycheck[] =
{
      17,   186,   155,   156,    85,   303,   586,   186,   274,   275,
     112,   165,   114,    15,   116,   217,    15,  1036,   120,     7,
      19,   123,   124,   400,   974,   402,   742,   404,   405,    29,
     154,     7,    44,   209,   150,     6,    18,    19,    22,    15,
     216,   217,    19,    19,    15,    16,    17,    18,   969,    15,
      18,    19,    28,    19,   878,    18,    19,    15,    16,    17,
      18,    19,    28,   974,     8,     9,    10,    11,    12,    86,
      27,   247,    19,    50,   132,    26,    27,    30,    22,    23,
      24,    25,    26,    27,    28,    29,   119,   255,    32,  1497,
       6,     8,     9,    10,    11,    12,    41,   273,   266,    15,
      16,    17,    18,    19,    35,    22,    23,    24,    25,    26,
      27,    28,    29,    44,    35,    32,    41,   124,     0,     3,
      19,    20,    41,    44,    15,   169,     6,   175,    19,    15,
     306,    17,    18,    19,    50,    15,    16,    17,    18,    19,
     117,   167,   168,    34,    35,    36,    37,    35,  1299,  1060,
       4,    15,    16,    17,    18,    19,    44,    15,   175,    17,
      18,    19,    85,     8,     9,    10,    11,    12,   185,   322,
      50,   327,   253,    19,  1376,  1170,   352,    22,    23,    24,
      25,    26,    27,    28,    29,   266,   248,    32,    15,    16,
      17,    18,    19,    15,    16,    17,    18,   307,    15,    18,
      19,   117,    19,   261,   342,   267,    15,    16,    17,    18,
     272,   349,   189,   190,   191,   192,   193,   194,   195,  1376,
     376,   374,   135,   267,   137,   287,    49,   172,   153,   154,
     155,  1152,   209,     3,   296,   212,   298,   117,   241,  1390,
       3,   140,   304,    66,   150,   307,   400,  1655,   402,   135,
     404,   405,   376,   172,   378,   379,   203,   381,   382,    15,
     384,   385,  1212,    19,   241,   141,   299,   329,    19,   331,
     850,   319,   382,   189,   190,   191,   192,   193,   194,   195,
      15,   397,   239,    15,    19,   583,    15,    19,  1373,   135,
      19,   175,    19,   209,   342,   124,   212,   250,   324,    50,
      35,  1212,     8,     9,    10,    11,    12,   183,   150,   189,
     190,   191,   192,   193,   194,   195,    22,    23,    24,    25,
      26,    27,    28,    29,    18,    19,    32,   144,   124,   209,
      15,   338,   212,    15,    19,  1537,   362,    19,  1333,   150,
     125,   264,   544,     8,     9,    10,    11,    12,  1433,   397,
     353,    15,   618,   172,  1070,    19,   208,    22,    23,    24,
      25,    26,    27,    28,    29,  1276,   117,    32,   544,  1388,
      34,    35,    36,    37,   550,   394,   383,   384,   163,   398,
    1537,   557,    80,    19,   396,  1335,   403,    19,    22,    19,
    1214,   243,   409,   392,   393,   395,   114,   399,    32,    42,
     417,   418,   419,   420,   421,    67,   394,  1404,    44,   393,
      19,   677,   588,   679,    44,   397,   433,   393,    61,   595,
     397,   597,   393,   399,  1335,   114,   397,   393,   319,   397,
      19,  1428,   698,   399,   397,   393,    85,   613,   189,   190,
     191,   192,   193,   194,   195,   396,   398,   392,   624,   401,
     626,  1536,   533,   629,   398,   194,   632,   633,   209,   144,
     149,   212,  1373,   758,   176,   396,   761,   392,   763,   152,
     765,   766,   115,   392,    19,   396,   742,   393,   140,   396,
     398,   397,  1432,   401,   146,   396,   662,   504,   505,   506,
     507,   508,   509,   510,   511,   512,   513,   514,   515,   516,
      20,  1217,   393,   769,   193,     3,   335,   393,   396,   338,
     686,   120,   688,   393,    66,   532,   644,   397,   580,   258,
     696,  1432,  1433,  1542,    20,     3,    19,    20,   545,   393,
     167,   168,   135,   176,   137,   393,    15,   136,   395,   335,
      19,   398,   338,    15,   606,   607,   674,    19,   565,   394,
      43,   290,   614,   398,   616,    34,    35,    36,    37,    55,
     299,    54,    34,    35,    36,    37,   393,    63,    64,    65,
     282,   393,   634,    85,   183,   592,   393,   639,   731,   641,
     595,   643,   597,   645,   393,    15,   648,   649,   650,    19,
     346,   347,   768,   224,   656,     3,   308,   659,     8,     9,
      10,    11,    12,    75,    34,    35,    36,    37,   874,     3,
     672,     3,    22,    23,    24,    25,    26,    27,    28,    29,
     805,   806,    32,   254,   130,  1536,     3,   133,   690,   691,
     692,     3,   811,   252,   123,    19,   760,   393,   762,     3,
     938,   319,   320,   242,   243,  1022,   397,   246,   247,   388,
     389,   390,   391,     8,     9,    10,    11,    12,   393,    15,
     392,   393,   344,    19,   393,     3,  1246,    22,    23,    24,
      25,    26,    27,    28,    29,     3,   354,    32,    34,    35,
      36,    37,     8,     9,    10,    11,    12,   324,     3,   178,
     179,     3,   398,    20,     3,   871,    22,    23,    24,    25,
      26,    27,    28,    29,    29,   971,    32,  1005,   393,     3,
    1008,   393,    68,  1011,    19,    20,    15,  1012,   261,  1014,
      19,   270,   271,   899,   900,   362,   902,    80,   881,   393,
      57,    58,     3,   398,    44,    34,    35,    36,    37,    19,
      20,   393,   351,   393,   233,   921,   922,   290,   291,   925,
     239,   927,   928,   239,   930,     3,   365,   366,    68,    15,
     936,   280,   281,     3,    44,   244,   245,   393,    44,   393,
       8,     9,    10,    11,    12,   393,   129,   130,   131,   955,
     797,   393,   799,   959,    22,    23,    24,    25,    26,    27,
      28,    29,    68,    33,    32,   393,   898,     3,   397,    28,
      29,    22,    42,    32,  1070,    45,     8,     9,    10,    11,
      12,  1077,    62,   123,   124,   245,    56,   392,    19,    20,
      22,    23,    24,    25,    26,    27,    28,    29,   169,    69,
      32,    19,   894,   392,   306,   370,   371,    85,    15,    79,
     158,   198,    19,    44,   906,    85,   135,   123,   124,    19,
      20,   913,    19,   915,     3,   396,   132,    34,    35,    36,
      37,    19,    20,   965,   221,   397,   398,    45,  1022,   396,
      19,   396,   229,   113,    44,   115,   116,   234,   235,   236,
     160,   161,   307,   123,    19,    13,   114,    15,    16,    17,
      18,    19,    20,    38,    39,    40,    68,   145,    26,    27,
     392,    29,   392,    31,    18,    19,    20,    85,    59,    60,
     150,   151,   160,    13,   393,    15,    16,    17,    18,    19,
      20,   393,   162,   396,   172,   396,    26,    27,    28,    29,
     240,    31,    18,    19,    20,   113,    85,    59,    60,   117,
     118,   307,   120,   121,   122,   185,   397,   398,   188,   127,
      80,  1217,  1658,  1659,  1660,    72,    20,  1255,   397,   398,
      77,   397,   398,   393,   240,   392,   115,   116,   268,  1145,
      68,    78,   244,   245,   123,  1290,  1291,   260,  1154,   157,
      84,  1134,     3,    75,   162,   413,   414,   155,   398,   117,
      75,     7,     8,     9,    10,   123,   253,    19,    19,  1175,
     264,   150,   119,    19,   266,   280,   284,  1024,   287,   294,
    1027,  1028,  1029,   264,  1031,   289,  1033,   117,   125,   126,
     302,  1038,   392,   123,  1041,   251,   350,   300,    50,   396,
    1047,   138,   139,   113,   141,   152,    78,   393,   114,   188,
     392,    85,  1688,   398,   392,   396,   396,   396,   307,   396,
     178,   179,  1698,  1699,  1700,  1701,  1232,   356,    45,    46,
      47,    48,    49,    50,    51,    52,    53,  1129,   245,  1131,
     248,   249,   398,  1249,  1250,  1137,  1252,    85,   178,   179,
     396,   392,  1144,    85,  1260,   213,   214,  1263,    78,    75,
    1356,  1153,   113,    84,   393,   117,   138,    80,   140,   141,
     392,   392,   230,    75,   392,   233,  1168,    20,   392,     3,
     238,   392,     3,   213,   214,   215,   216,   217,   218,     3,
       3,     3,   138,   139,   241,     3,    19,     3,     3,     3,
     230,     3,     3,   233,   170,   170,    15,    14,   238,   181,
     182,   183,   184,   239,    29,  1162,    26,    15,     7,  1166,
    1167,    32,   392,  1329,    27,  1172,    19,   398,     3,   392,
     398,     8,     9,    10,    11,    12,   393,   189,   190,   191,
     192,   193,   194,   195,   392,    22,    23,    24,    25,    26,
      27,    28,    29,  1200,   392,    32,    19,   209,   133,  1251,
     212,  1376,   394,   310,  1396,   392,    50,   392,   392,    13,
     163,    15,    16,    17,    18,    19,    20,   392,    75,  1362,
      75,  1364,    26,    27,    28,    29,   393,    31,   292,   397,
    1396,  1397,   327,   328,   329,   330,   331,   332,   333,   334,
     392,  1312,    30,   172,  1410,    75,    75,    75,    75,   392,
     261,  1417,  1418,  1419,   114,  1421,   392,  1513,   392,   392,
     367,   392,   392,    95,   172,   372,  1273,  1274,  1275,   392,
      68,    19,   104,   392,   258,   393,   394,   109,   392,   111,
     387,   113,   400,   115,   392,   117,     3,   119,     3,  1296,
      19,    15,  1299,    19,    19,     3,   307,    19,  1464,    20,
     311,    15,    15,   393,    15,   394,     7,  1473,  1474,  1475,
     400,  1477,  1478,   117,   398,   326,    15,  1483,    27,   123,
     398,    18,   393,   392,  1490,    20,   398,   392,   396,   392,
     392,  1338,  1339,    27,  1341,  1501,  1502,   128,    19,   392,
     392,  1393,   392,  1509,  1415,   392,   357,   358,   392,  1401,
     392,  1517,  1544,  1545,  1546,   392,   392,  1409,  1524,  1411,
    1535,   392,  1537,   392,   392,   392,    68,     3,   392,   392,
     392,   392,   134,   392,   178,   179,   392,   392,  1544,  1545,
    1546,  1637,   392,  1390,   396,  1551,  1552,  1553,  1554,  1555,
     392,   392,   392,   392,   156,   392,   392,   392,  1564,    13,
    1566,    15,    16,    17,    18,    19,    20,   169,     3,   213,
     214,    15,    26,    27,  1466,    29,  1582,    31,  1470,     3,
    1472,   392,   392,   392,   392,  1591,   230,     3,     3,   233,
    1437,     3,  1598,   172,   238,   172,  1602,  1603,  1604,    27,
    1447,  1448,  1449,  1450,   398,   392,   394,  1613,   392,   398,
    1616,  1617,   170,   392,   392,   392,   392,   307,   392,   392,
    1512,   392,  1514,    13,   392,    15,    16,    17,    18,    19,
      20,   392,   392,   392,    45,   392,    26,    27,   392,    29,
     359,    31,   392,    22,    13,   392,    15,    16,    17,    18,
      19,    20,   393,     3,    65,  1547,     3,    26,    27,    28,
      29,    15,    31,   117,    15,  1557,   394,   398,   397,   123,
      15,    15,    68,    22,    85,   392,    13,     7,    15,    16,
      17,    18,    19,    20,  1576,   172,    19,     3,  1580,    26,
      27,  1583,    29,     3,    31,   251,    15,   396,   392,   211,
     396,   392,   113,   248,  1596,  1597,   117,   118,   396,   120,
     121,   122,   398,   396,   396,  1607,   127,   394,   397,   321,
     322,   323,   396,   325,   178,   179,  1618,   117,  1620,  1621,
     396,   392,   394,   123,   336,   393,   245,   394,   394,   393,
     393,   393,   393,   392,   392,   392,   157,   266,   117,   393,
     396,   162,   392,   396,   123,  1647,   400,   140,   141,   213,
     214,   215,   216,   217,   218,   392,     7,   150,   397,   394,
     345,   246,   398,   246,   246,   307,   230,  1624,   396,   233,
     117,   363,   165,   266,   238,   302,   123,   361,   178,   179,
     353,   174,   392,   176,   177,   397,   329,     8,     9,    10,
      11,    12,   327,   392,    75,   134,   302,   345,    75,   178,
     179,    22,    23,    24,    25,    26,    27,    28,    29,    75,
     379,    32,   353,   213,   214,   392,   397,   397,   364,   381,
     344,    20,   392,   344,   339,     8,     9,    10,    11,    12,
     230,   178,   179,   233,   213,   214,   392,   394,   238,    22,
      23,    24,    25,    26,    27,    28,    29,   348,   241,    32,
     536,   230,   392,   392,   233,   392,   392,   833,   392,   238,
       8,     9,    10,    11,    12,   251,   213,   214,  1071,   392,
    1265,  1516,   392,  1655,    22,    23,    24,    25,    26,    27,
      28,    29,   392,   230,    32,   392,   233,   367,   986,   989,
    1059,   238,     8,     9,    10,    11,    12,  1214,   567,   737,
     374,   879,  1432,  1535,   805,   185,    22,    23,    24,    25,
      26,    27,    28,    29,  1537,   806,    32,   310,   811,  1170,
    1037,  1341,  1076,     8,     9,    10,    11,    12,  1308,   393,
    1024,  1274,   920,  1324,  1359,   973,   400,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    -1,   397,    -1,    -1,   372,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,     8,     9,    10,
      11,    12,   392,   393,   387,    -1,    -1,    -1,    -1,    -1,
     400,    22,    23,    24,    25,    26,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,   393,     8,     9,    10,    11,    12,
      -1,   400,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
       8,     9,    10,    11,    12,    -1,   393,    -1,    -1,    -1,
      -1,    -1,    -1,   400,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,     8,     9,    10,    11,    12,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
       8,     9,    10,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    32,     8,     9,    10,    11,    12,
      -1,    -1,    -1,   394,    -1,    -1,    -1,    -1,    -1,    22,
      23,    24,    25,    26,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   394,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,   394,    32,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,     8,     9,    10,    11,    12,   394,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,   394,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,     8,     9,    10,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,   394,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,    -1,    32,     8,     9,
      10,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    45,    32,   394,     8,     9,    10,    11,    12,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,    45,    32,     3,
      -1,   394,    -1,    -1,     8,     9,    10,    11,    12,    83,
      -1,    85,    -1,    -1,    -1,    -1,    -1,    41,    22,    23,
      24,    25,    26,    27,    28,    29,   394,    -1,    32,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    85,    -1,   113,
      -1,    65,    66,   117,   118,    -1,    -1,   121,   122,    -1,
      -1,   394,    -1,   127,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   113,    -1,    -1,    45,   117,
     118,    -1,     5,   121,   122,    -1,   394,    -1,    -1,   127,
      -1,    -1,    -1,   157,    -1,    -1,    -1,    -1,   162,    -1,
      -1,    -1,    -1,    -1,    -1,    72,    73,    74,    -1,    76,
      77,   394,    79,     8,     9,    10,    83,    -1,    85,   157,
     134,    -1,   136,    -1,   162,    -1,    -1,    22,    23,    24,
      25,    26,    27,    28,    29,    -1,   394,    32,    -1,    -1,
      -1,   155,   156,    -1,    -1,    -1,   113,    -1,    -1,    -1,
     117,   118,    -1,    -1,   121,   122,    -1,    -1,    81,    82,
     127,   394,    -1,    86,    87,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     157,    -1,    -1,    -1,   394,   162,    45,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    45,
      -1,    -1,   135,    -1,    -1,    -1,    65,    -1,    -1,   394,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   241,    -1,    -1,
     244,   245,    -1,    -1,    -1,    -1,    85,    -1,    -1,    -1,
      -1,    -1,   392,   166,    -1,    -1,    -1,   311,    -1,    85,
     264,    -1,   316,   317,   318,    19,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   113,    -1,    -1,   392,   117,   118,
      -1,    -1,   121,   122,    -1,    -1,    -1,   113,   127,    -1,
      -1,   117,   118,    -1,   120,   121,   122,   301,   352,    -1,
      -1,   127,   392,    -1,    -1,   359,   219,   220,    -1,   222,
     223,   224,   225,   226,   227,   228,    -1,    -1,   157,    -1,
      -1,    -1,    -1,   162,    -1,    -1,    -1,   392,     5,    -1,
      -1,   157,   336,    -1,    -1,    -1,   162,   176,    -1,    -1,
     368,   369,    -1,   397,    -1,    -1,    -1,    -1,   352,    -1,
      -1,    -1,   392,    -1,    -1,    -1,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   369,    -1,    -1,    -1,   397,
     374,   375,    -1,   377,   378,    -1,   380,    -1,   392,    -1,
      -1,    -1,   295,   296,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   397,   120,    -1,    -1,    -1,    -1,   312,
     313,   314,   315,    -1,    81,    82,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    45,    -1,    -1,    -1,
     397,    50,   196,   197,   198,   199,   200,   201,   202,   175,
     204,   205,   206,   207,   208,    -1,    65,    -1,   297,    -1,
      -1,    -1,    45,     8,     9,    10,    11,    12,    -1,    -1,
      79,    -1,   385,   386,    -1,    -1,    85,    22,    23,    24,
      25,    26,    27,    28,    29,    65,    -1,    32,    -1,   166,
      -1,    74,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    85,    -1,   113,    -1,    -1,    -1,   117,   118,
     119,   120,   121,   122,    -1,    -1,    -1,    -1,   127,    -1,
      -1,    -1,   131,    -1,    -1,   134,   352,   136,    -1,    -1,
     113,    -1,    -1,    -1,   117,   118,   262,    -1,   121,   122,
      -1,    -1,    -1,    -1,   127,    -1,    -1,   373,   157,    -1,
      -1,    -1,    -1,   162,   134,    -1,    -1,    -1,   397,   142,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   176,    -1,   178,
      -1,   397,    -1,    -1,   157,    -1,   156,    -1,   187,   162,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   169,
      -1,   317,   269,   270,   271,   272,   273,   274,   275,   276,
     277,    -1,    -1,   183,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   223,   341,    -1,   343,   295,   296,
      -1,    -1,   348,    -1,   350,    -1,    -1,    -1,   354,   355,
      -1,    -1,   241,    -1,    -1,   312,   313,   314,   315,   248,
     249,    -1,   251,    -1,   253,   254,    -1,   256,   257,    -1,
     259,    -1,    -1,   262,   263,    -1,   265,    -1,    -1,    -1,
      -1,   241,    -1,    -1,   244,   245,     3,    -1,    -1,   278,
     279,   397,    -1,    -1,   283,    -1,   285,   286,    -1,   288,
      -1,    18,    19,    -1,   293,    -1,    -1,    -1,   297,   298,
      -1,   300,   301,    -1,   303,   304,   305,    -1,    -1,    -1,
     309,    -1,    -1,    -1,    -1,    -1,    -1,    44,   385,   386,
      -1,    -1,    -1,    -1,     8,     9,    10,    11,    12,    -1,
      -1,   301,    -1,    -1,   304,    -1,    -1,   307,    22,    23,
      24,    25,    26,    27,    28,    29,    -1,   317,    32,   319,
      -1,   321,   322,   323,   324,   325,   326,    -1,    85,    -1,
      -1,    -1,   332,    -1,    -1,    -1,   336,   337,    45,    -1,
     340,    -1,    -1,    -1,    -1,   196,   197,   198,   199,   200,
     201,   202,   352,   204,   205,   206,   207,   208,   115,    -1,
     360,    -1,    -1,    70,    71,   122,   123,    -1,   397,    -1,
      -1,    -1,    -1,    80,    -1,    -1,   133,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   142,   143,   144,   145,    -1,
     147,   148,    -1,    -1,   397,    -1,    -1,   397,     8,     9,
      10,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    24,    25,    26,    27,    28,    29,
      -1,    -1,    32,    -1,   131,   196,   197,   198,   199,   200,
     201,   202,    -1,   204,   205,   206,   207,   208,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   159,    -1,    -1,   162,    -1,   164,    -1,    -1,
      -1,    -1,    -1,    -1,   171,    -1,   173,    -1,    -1,    -1,
      -1,   178,   179,   180,    -1,    -1,    -1,    -1,   185,   186,
     187,    -1,    -1,   240,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   210,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   394
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,   403,   404,    45,    70,    71,    80,   131,   159,
     162,   164,   171,   173,   178,   179,   180,   185,   186,   187,
     428,   429,   430,   431,   432,   433,   484,   485,   486,   487,
     489,   490,   491,   494,   497,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   515,   516,
     517,   518,   519,   520,   521,   522,   523,   591,   628,   629,
     630,   631,   632,   633,   634,   635,   650,   651,   661,   663,
       0,     3,    33,    42,    45,    56,    69,    79,    85,   113,
     115,   116,   123,   150,   151,   162,   185,   188,   392,   405,
     417,   423,   426,   427,   436,   438,   441,   443,   446,   448,
     449,   451,   452,   459,   469,   470,   526,   528,   538,   577,
     580,   592,   594,   595,   596,   637,   638,   639,   642,   652,
     653,   662,   664,   665,   666,    19,    20,    44,   524,    20,
      55,    63,    64,    65,   434,     3,   524,   524,   160,   161,
     524,   524,     3,   150,   140,   141,   150,   165,   174,   176,
     177,   241,   310,   372,   387,   150,   150,    78,   138,   140,
     141,   181,   182,   183,   184,    13,    15,    16,    17,    18,
      19,    20,    26,    27,    29,    31,   117,   123,   178,   179,
     213,   214,   230,   233,   238,   393,   400,   543,   556,   561,
     564,   565,   566,   568,   570,    35,    44,   396,   513,   524,
      19,    19,    20,    43,    54,   415,   419,    67,   140,   146,
      42,    61,   115,   176,   424,   425,    49,    66,    80,    19,
     114,    19,    85,   152,   140,   415,   570,    19,    19,   135,
     453,   453,   396,    72,    77,   119,   152,   241,   310,   367,
     372,   387,   453,    45,    65,    85,   113,   117,   118,   120,
     121,   122,   127,   157,   162,   397,   454,   472,   453,   453,
      50,    65,    79,   119,   120,   131,   134,   136,   176,   178,
     187,   223,   241,   248,   249,   251,   253,   254,   256,   257,
     259,   262,   263,   265,   278,   279,   283,   285,   286,   288,
     293,   297,   298,   300,   301,   303,   304,   305,   309,   397,
     454,   453,    83,   311,   316,   317,   318,   352,   359,   397,
     454,   597,   598,   599,   453,   368,   369,   397,   454,   453,
     453,    65,   176,   297,   397,   454,   654,   657,    72,    73,
      74,    76,    77,    79,    83,   397,   437,   440,   454,    74,
     142,   397,   439,   454,    35,    44,   396,   473,   434,     3,
       3,   434,    66,   435,     3,     3,   524,   524,     3,    35,
      44,   396,   476,    85,   499,    19,    20,    44,   525,     3,
     175,   495,     3,     3,   492,   498,   252,    19,   120,   183,
     351,   365,   366,   488,   141,   183,   488,    18,    19,   409,
     411,   411,     3,     3,     3,     3,     3,     3,     3,     3,
      15,    19,    28,   393,   399,   560,   562,   568,    29,   393,
     239,    27,   239,    15,   399,   563,   570,   393,   393,   393,
     393,   393,   393,   215,   216,   217,   218,   550,   570,    15,
      16,    17,    19,   393,   543,   544,   548,   551,   552,   554,
     556,   557,   558,     5,    81,    82,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   135,   166,   219,   220,   222,   223,   224,
     225,   226,   227,   228,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   295,   296,   312,   313,   314,   315,   385,
     386,   569,   667,     3,     8,     9,    10,    11,    12,    22,
      23,    24,    25,    26,    27,    28,    29,    32,    45,    46,
      47,    48,    49,    50,    51,    52,    53,   421,   422,    15,
     514,     3,    22,    62,   418,    44,   396,   420,    15,    19,
      34,    35,    36,    37,    68,   393,   406,   408,   473,   406,
      20,    57,    58,   392,   406,   407,   407,   169,   158,   447,
      15,    17,    18,    19,   135,   393,   442,   527,    19,   444,
     476,   392,   539,   135,    19,   396,   396,   450,   396,   473,
     307,    19,   458,   114,   406,   408,   471,    44,    68,   123,
     124,   132,   240,   455,   535,    68,   455,    68,   415,   476,
     392,   392,   396,   396,   169,   267,   307,    80,   392,    20,
     268,   415,   408,    68,    84,   176,   282,   308,   155,   408,
     392,   406,   409,   593,   172,   409,    75,   255,   266,    75,
     253,   260,   270,   271,   264,   266,   280,   281,   280,   284,
     408,   287,   289,   294,    85,   264,   119,   299,   264,   302,
     300,   408,   251,   408,   392,   396,   350,   442,   408,   113,
     406,   408,   114,   392,   396,   396,    85,   370,   371,   392,
     396,   396,   307,   411,    85,   392,   194,   258,   290,   299,
     388,   389,   390,   391,   396,   660,    85,   408,    75,   408,
      78,    80,    84,   392,   392,   392,    75,    20,   416,   392,
     392,    78,   125,   126,   138,   139,   141,   474,   475,     3,
     435,     3,   406,     3,     3,     3,   125,   163,   477,   478,
       3,    19,     3,   525,     3,     3,     3,    85,   145,   160,
     172,   496,     3,    44,    85,   115,   122,   123,   133,   142,
     143,   144,   145,   147,   148,   411,   493,   535,     3,    19,
      85,   115,   116,   123,   150,   188,   488,   488,   488,   636,
      44,   488,    44,   488,     3,   488,   488,     3,    29,   395,
     413,   170,   170,     7,   562,   562,   570,   562,   562,    14,
      15,   570,   574,   575,    26,   239,    27,   563,   563,    29,
     570,   570,   570,   570,   570,    19,    15,   398,   394,   398,
      28,   546,   547,   550,   570,     7,   398,   401,    26,    27,
     396,   398,   401,   570,   570,   570,   570,   570,   570,   570,
     570,   570,   570,   570,   570,   570,   198,   221,   229,   234,
     235,   236,   397,   398,     3,   570,   415,   421,   392,   407,
     570,   406,    59,    60,    59,    60,    38,    39,    40,   406,
     466,   467,   570,   392,   196,   197,   198,   199,   200,   201,
     202,   204,   205,   206,   207,   208,   529,   530,   531,   533,
     537,    41,   153,   154,   155,   392,   445,   392,   393,   536,
      19,   150,   397,   408,   442,   460,   463,   464,    35,   406,
     457,    19,   533,   534,   133,   570,   457,   457,   579,    75,
      75,   408,    75,   306,   408,   392,    50,   392,   392,   406,
     244,   245,   408,   292,   408,   163,   409,   392,   392,    30,
     410,    41,   172,   392,   406,   172,   406,    75,    75,   406,
      75,   132,   261,   406,   406,   408,    75,   392,   114,   392,
     408,   392,   408,   261,   290,   291,   408,   458,   408,   392,
     392,   408,   408,   408,   392,   172,   392,   408,   408,    68,
     406,   600,    19,   392,   392,   641,   408,   458,    20,   409,
     409,   258,   655,   656,   659,   392,   406,   406,   408,   408,
     408,   406,   409,   411,   412,   397,   398,     3,   397,   398,
       3,    19,    15,   411,    19,    19,   534,   412,    19,     3,
      20,   627,   627,     3,    19,   113,   261,   307,   311,   326,
     357,   358,   488,   627,   488,   627,   627,   627,   406,   409,
      15,    15,    15,   394,   398,   394,    27,   398,   398,   398,
     394,   398,   394,   398,   570,   570,     7,   398,   398,   394,
      18,   393,   544,   552,    15,    18,   557,   393,   422,   392,
     397,   394,    20,   392,   464,   392,   394,    19,   203,   392,
     396,   406,   409,   394,   530,   532,   537,   411,   392,    27,
     462,   398,   128,   456,   408,   120,   248,   249,   397,   454,
     578,   406,   406,   392,   406,   392,   392,   408,   392,   392,
     392,   392,   408,   392,   408,   392,    19,   593,   406,   406,
     392,   406,   392,   406,   406,   392,   406,   392,   392,   392,
     392,   392,   406,   442,   392,   392,   392,   392,   392,   392,
     392,   392,   392,   392,   392,   392,   406,   406,   120,   175,
     262,   317,   341,   343,   348,   350,   354,   355,   397,   601,
     608,   611,   392,   120,   352,   373,   397,   454,   640,   392,
     392,   410,   409,   241,   353,   655,    19,    50,   117,   189,
     190,   191,   192,   193,   194,   195,   209,   212,   241,   397,
     566,   567,   571,   576,   658,    68,   475,   478,     3,     3,
     442,   442,    15,   442,   627,     3,   627,     3,     3,     3,
     172,   172,   562,   574,    27,   570,   570,   570,   570,   570,
     398,   394,   546,   570,   547,   549,   570,   570,   398,   570,
     531,   540,   541,   576,   392,   394,   170,    20,   412,   461,
     463,    80,   129,   130,   131,   581,   585,   581,   586,   409,
     590,   392,   398,   392,   392,   392,   392,   392,   392,   392,
     392,   392,   392,   392,   392,   392,   620,   626,   408,   319,
     320,   354,   319,   408,   602,   307,   411,   604,   605,   603,
     356,   408,   392,   359,   621,   643,   649,   408,   406,   392,
     410,   408,   406,    22,   393,    22,   393,   570,   570,   570,
     408,   569,   667,   392,   570,   572,   573,   406,     3,     3,
      15,    15,   394,   394,   394,   394,   398,   570,   394,   398,
      15,   394,   397,   576,   532,    15,   412,   464,   465,   396,
     584,    30,   250,   587,   406,   464,   622,   406,   406,   408,
     406,   396,   442,   396,   396,   406,   406,   396,   465,    68,
     570,   575,   570,   566,   392,   210,   396,   392,    22,    22,
      32,   398,   392,   499,   499,   570,   394,     7,    28,   549,
     397,   172,   584,   583,    19,   415,   251,   588,   392,   623,
     396,   625,   613,   606,   613,   609,   396,   648,   406,   392,
     394,   392,   667,   396,   542,   576,   559,   570,   570,   123,
     178,   179,   233,   239,   573,     3,     3,   394,   393,   394,
       7,   394,    15,   136,   242,   243,   246,   247,   397,   582,
     409,   248,   589,   621,   624,   397,   411,   614,   615,   175,
     319,   342,   397,   607,   397,   134,   156,   169,   321,   322,
     323,   325,   336,   397,   479,   481,   482,   610,   647,   644,
     394,   540,   211,     6,    15,    16,    17,   393,   397,   543,
     545,   548,   551,   553,   555,   392,   392,   393,   393,   393,
     393,   392,   546,   549,   392,   408,   244,   245,   245,   407,
     406,   392,   408,    65,   169,   183,   241,   244,   245,   301,
     304,   307,   317,   319,   321,   322,   323,   324,   325,   326,
     332,   336,   337,   340,   352,   360,   397,   479,   612,   342,
     349,   616,   408,   406,   408,   392,   415,   396,   406,   406,
     406,   266,   144,   406,   124,   335,   338,   396,   392,    41,
      65,    66,   136,   155,   241,   244,   245,   264,   301,   336,
     352,   369,   374,   375,   377,   378,   380,   397,   479,   645,
     392,   397,   542,   540,   570,     7,   395,   398,   570,   570,
     570,   570,   398,   394,   246,   246,   246,   307,   406,   396,
     408,   345,   167,   168,   324,   362,   468,   302,   408,   363,
     408,   406,   406,   406,   266,   406,   144,   406,   327,   328,
     329,   330,   331,   332,   333,   334,   361,   124,   335,   338,
     329,   406,   353,   327,   392,   406,   392,   480,   481,   406,
     406,   114,   149,   193,   483,   406,   307,   382,    75,   408,
     409,   408,   345,   324,   362,   468,   406,   302,   124,   338,
     383,   384,   646,   353,   327,   376,    75,    75,   344,   406,
     379,   381,   392,   394,   393,   545,   540,   553,   394,   394,
     394,   394,   546,   407,   407,   407,   408,   617,   346,   347,
     406,   406,   406,   406,   406,   245,   408,   364,   406,   406,
     408,   408,   406,   408,   397,   392,   406,   135,   137,   135,
     137,   397,   408,   408,   406,   406,   406,   406,   245,   408,
     406,   406,   406,   408,   408,   408,   570,   392,   392,   392,
     392,   394,   397,   409,   619,   344,   408,   480,   392,    20,
     414,   414,   414,   414,   344,   339,   618,   483,   392,   392,
     392,   392,   392,   483,   483,   483,   483
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   402,   403,   403,   404,   404,   405,   406,   406,   406,
     407,   407,   407,   408,   408,   408,   408,   408,   408,   409,
     409,   410,   410,   411,   412,   412,   413,   413,   414,   415,
     415,   416,   416,   417,   418,   418,   419,   419,   419,   420,
     420,   421,   421,   422,   422,   422,   422,   422,   422,   422,
     422,   422,   423,   423,   424,   424,   424,   424,   425,   425,
     425,   425,   425,   425,   426,   427,   427,   427,   427,   428,
     429,   430,   431,   432,   433,   434,   434,   435,   435,   435,
     436,   437,   437,   437,   437,   437,   437,   437,   438,   439,
     439,   440,   441,   441,   442,   442,   442,   442,   442,   443,
     444,   444,   445,   445,   445,   445,   446,   447,   447,   448,
     449,   450,   450,   451,   451,   452,   452,   453,   453,   453,
     453,   454,   454,   454,   454,   454,   454,   454,   454,   454,
     454,   454,   454,   454,   454,   455,   455,   455,   455,   456,
     456,   456,   456,   456,   457,   457,   458,   459,   459,   460,
     461,   461,   461,   462,   462,   463,   464,   464,   465,   466,
     467,   468,   468,   469,   470,   470,   470,   470,   471,   472,
     473,   473,   473,   474,   474,   475,   475,   475,   475,   475,
     475,   476,   476,   476,   477,   477,   478,   478,   479,   479,
     480,   480,   481,   481,   482,   483,   483,   483,   483,   483,
     483,   484,   485,   486,   487,   488,   488,   489,   490,   491,
     492,   492,   492,   492,   492,   492,   492,   492,   492,   492,
     492,   492,   492,   493,   493,   493,   494,   495,   495,   495,
     495,   496,   496,   496,   497,   498,   498,   498,   498,   498,
     498,   498,   498,   499,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   513,
     513,   514,   514,   515,   516,   517,   518,   519,   520,   521,
     522,   523,   524,   524,   524,   525,   525,   525,   527,   526,
     528,   529,   529,   529,   529,   529,   529,   529,   529,   529,
     529,   529,   529,   529,   530,   531,   531,   532,   532,   533,
     534,   534,   535,   536,   536,   537,   539,   538,   540,   540,
     541,   541,   542,   542,   543,   544,   544,   544,   544,   544,
     544,   545,   545,   545,   545,   545,   546,   547,   547,   547,
     548,   548,   549,   550,   550,   550,   550,   551,   551,   551,
     552,   552,   552,   552,   553,   553,   553,   553,   554,   554,
     555,   555,   556,   557,   557,   557,   557,   558,   558,   559,
     559,   559,   560,   560,   561,   561,   562,   562,   562,   562,
     562,   562,   563,   563,   563,   564,   564,   564,   564,   564,
     564,   564,   564,   564,   564,   564,   565,   565,   566,   567,
     568,   569,   569,   569,   569,   569,   569,   569,   569,   569,
     569,   570,   570,   570,   570,   570,   570,   570,   570,   570,
     570,   570,   570,   570,   570,   570,   570,   570,   570,   570,
     570,   570,   570,   570,   570,   570,   570,   570,   570,   570,
     570,   570,   570,   570,   570,   570,   570,   570,   570,   571,
     571,   571,   571,   571,   571,   572,   573,   573,   573,   574,
     574,   575,   575,   576,   576,   576,   576,   576,   576,   576,
     576,   576,   576,   576,   576,   576,   576,   576,   576,   577,
     578,   578,   578,   578,   579,   579,   580,   581,   582,   582,
     582,   582,   582,   582,   583,   583,   584,   584,   585,   586,
     587,   587,   587,   588,   588,   589,   589,   590,   591,   592,
     593,   593,   593,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   594,   594,   594,   594,   594,   594,   594,
     594,   594,   594,   595,   596,   596,   597,   597,   597,   597,
     597,   597,   597,   597,   597,   598,   599,   600,   600,   601,
     601,   601,   601,   601,   601,   601,   601,   601,   602,   601,
     603,   601,   601,   601,   601,   604,   604,   605,   606,   606,
     607,   607,   607,   608,   608,   609,   609,   610,   610,   610,
     610,   610,   610,   610,   610,   610,   610,   610,   611,   612,
     612,   612,   612,   612,   612,   612,   612,   612,   612,   612,
     612,   612,   612,   612,   612,   612,   612,   612,   612,   612,
     612,   612,   612,   612,   612,   612,   612,   612,   612,   612,
     612,   612,   612,   612,   612,   612,   612,   612,   613,   613,
     614,   615,   616,   616,   616,   617,   617,   618,   618,   619,
     620,   621,   621,   623,   622,   624,   624,   625,   625,   626,
     627,   627,   628,   629,   630,   631,   632,   633,   634,   635,
     636,   636,   636,   636,   636,   636,   636,   636,   636,   637,
     638,   638,   638,   638,   638,   639,   640,   640,   640,   640,
     640,   641,   641,   642,   643,   644,   645,   645,   645,   645,
     645,   645,   645,   645,   645,   645,   645,   645,   645,   645,
     645,   645,   645,   645,   645,   645,   645,   645,   645,   645,
     645,   646,   646,   646,   646,   647,   647,   648,   648,   649,
     650,   651,   652,   653,   653,   653,   653,   653,   654,   655,
     655,   655,   656,   656,   657,   657,   657,   657,   657,   657,
     657,   657,   657,   658,   658,   659,   659,   660,   660,   661,
     662,   662,   662,   662,   662,   662,   662,   662,   662,   662,
     662,   662,   662,   662,   662,   662,   663,   663,   663,   663,
     663,   663,   663,   663,   663,   663,   663,   663,   663,   663,
     663,   663,   663,   663,   663,   663,   663,   663,   663,   663,
     663,   663,   663,   663,   663,   663,   663,   663,   663,   663,
     663,   663,   663,   663,   663,   663,   663,   663,   663,   663,
     663,   663,   663,   663,   663,   664,   664,   664,   664,   664,
     664,   664,   664,   664,   665,   665,   665,   665,   666,   666,
     666,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667,   667,
     667,   667,   667,   667,   667,   667,   667,   667,   667
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     0,     2,     5,     1,     3,     1,
       2,     2,     2,     1,     1,     1,     1,     1,     0,     1,
       1,     0,     2,     2,     1,     1,     2,     2,     1,     1,
       1,     1,     0,     4,     2,     0,     1,     2,     1,     1,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     4,     3,     1,     1,     1,     1,     2,     4,
       3,     3,     3,     3,     3,     3,     4,     3,     3,     4,
       5,     3,     3,     4,     2,     0,     1,     0,     1,     2,
       2,     2,     3,     2,     3,     3,     3,     5,     2,     3,
       3,     3,     4,     5,     1,     3,     1,     1,     1,     4,
       0,     2,     2,     2,     1,     1,     5,     0,     1,     3,
       3,     0,     8,     1,     4,     1,     1,     0,     1,     2,
       3,     0,     2,     2,     2,     2,     2,     2,     3,     3,
       3,     4,     2,     3,     2,     2,     1,     1,     1,     0,
       2,     2,     2,     2,     2,     1,     1,     3,     3,     0,
       1,     1,     2,     0,     1,     3,     1,     3,     1,     0,
       2,     2,     2,     2,     3,     3,     3,     5,     0,     3,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     1,     3,     1,     1,     4,     1,
       0,     3,     4,     1,     2,     0,     5,     5,     5,     5,
       4,     3,     3,     4,     5,     1,     0,     3,     4,     4,
       0,     2,     3,     3,     3,     2,     2,     2,     2,     3,
       3,     2,     2,     1,     1,     1,     4,     0,     3,     3,
       3,     0,     1,     1,     4,     0,     2,     2,     2,     2,
       2,     2,     2,     0,     2,     9,     9,     4,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     4,     1,     1,
       3,     0,     1,     3,     3,     3,     3,     4,     4,     4,
       4,     2,     1,     1,     1,     1,     0,     1,     0,     4,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     0,     3,     1,     3,     1,
       1,     1,     2,     3,     2,     4,     0,     5,     0,     1,
       1,     2,     1,     3,     1,     1,     1,     1,     1,     3,
       1,     1,     3,     1,     1,     1,     1,     1,     3,     1,
       5,    11,     1,     1,     1,     2,     1,     7,     9,     7,
       1,     1,     1,     3,     1,     1,     1,     3,     1,     3,
       1,     3,     3,     1,     2,     2,     6,     1,     3,     0,
       4,     3,     1,     3,     3,     3,     2,     4,     2,     2,
       2,     0,     2,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     1,     1,     5,     7,     0,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     4,     1,     1,     1,
       1,     2,     2,     3,     3,     6,     3,     3,     3,     3,
       3,     5,     6,     6,     6,     6,     4,     8,     4,     1,
       1,     1,     1,     1,     1,     1,     0,     1,     3,     1,
       3,     0,     1,     4,     6,     4,     3,     5,     5,     4,
       6,     3,     2,     5,     5,     8,     8,     8,     8,     2,
       1,     2,     2,     2,     0,     3,     5,     0,     2,     4,
       4,     4,     2,     2,     0,     3,     0,     3,     3,     2,
       0,     2,     2,     0,     2,     0,     2,     4,     5,     2,
       0,     3,     3,     3,     3,     5,     6,     4,     6,     4,
       6,     4,     4,     5,     6,     3,     3,     4,     5,     5,
       5,     5,     5,     4,     4,     5,     5,     5,     5,     5,
       5,     5,     5,     6,     6,     6,     5,     6,     8,     6,
       6,     5,     6,     4,     5,     4,     4,     5,     4,     5,
       4,     5,     5,     5,     5,     6,     5,     5,     6,     5,
       5,     5,     4,     2,     3,     3,     1,     2,     3,     2,
       4,     3,     2,     3,     1,     2,     4,     0,     3,     2,
       1,     2,     3,     3,     3,     3,     2,     3,     0,     5,
       0,     5,     2,     2,     1,     4,     1,     1,     0,     3,
       2,     2,     2,     5,     2,     0,     3,     0,     2,     2,
       3,     2,     2,     3,     2,     2,     2,     1,     3,     2,
       2,     2,     2,     2,     2,     3,     2,     2,     2,     2,
       2,     2,     2,     2,     3,     3,     3,     2,     3,     2,
       3,     3,     4,     4,     2,     2,     2,     3,     3,     3,
       2,     3,     3,     3,     4,     2,     2,     1,     0,     2,
       3,     1,     0,     1,     2,     0,     2,     0,     1,     3,
       0,     0,     2,     0,     3,     0,     3,     0,     3,     3,
       1,     0,     4,     6,     6,     6,     7,     6,     7,     5,
       0,     2,     3,     2,     3,     3,     2,     3,     2,     2,
       3,     3,     5,     4,     4,     2,     1,     2,     4,     2,
       2,     0,     3,     5,     0,     0,     2,     2,     2,     2,
       2,     2,     2,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     4,     3,     2,     2,
       1,     1,     1,     1,     1,     0,     3,     0,     3,     4,
       6,     6,     2,     3,     3,     5,     5,     4,     2,     3,
       3,     3,     1,     2,     4,     3,     3,     3,     2,     2,
       2,     2,     2,     1,     3,     0,     2,     0,     3,     4,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     2,     1,     2,
       2,     2,     1,     2,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token union list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the union list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 731 "cf-parse.y" /* yacc.c:1646  */
    { return 0; }
#line 3809 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 732 "cf-parse.y" /* yacc.c:1646  */
    { return 0; }
#line 3815 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 745 "cf-parse.y" /* yacc.c:1646  */
    {
     struct f_val *val = cfg_alloc(sizeof(struct f_val));
     *val = f_eval((yyvsp[-1].x), cfg_mem);
     if (val->type == T_RETURN) cf_error("Runtime error");
     cf_define_symbol((yyvsp[-3].s), SYM_CONSTANT | val->type, val);
   }
#line 3826 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 755 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = f_eval_int((yyvsp[-1].x)); }
#line 3832 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 756 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class != (SYM_CONSTANT | T_INT)) cf_error("Number expected");
     (yyval.i) = SYM_VAL((yyvsp[0].s)).i; }
#line 3840 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 763 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = (u32) (yyvsp[-1].i) * 1000000; }
#line 3846 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 764 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = (u32) (yyvsp[-1].i) * 1000; }
#line 3852 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 765 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = (u32) (yyvsp[-1].i) * 1; }
#line 3858 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 773 "cf-parse.y" /* yacc.c:1646  */
    {(yyval.i) = !!(yyvsp[0].i); }
#line 3864 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 774 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 1; }
#line 3870 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 775 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 1; }
#line 3876 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 776 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 3882 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 777 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 3888 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 778 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 1; }
#line 3894 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 785 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class != (SYM_CONSTANT | T_IP)) cf_error("IP address expected");
     (yyval.a) = SYM_VAL((yyvsp[0].s)).px.ip;
   }
#line 3903 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 792 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.iface) = NULL; }
#line 3909 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 793 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.iface) = if_get_by_name((yyvsp[0].s)->name); }
#line 3915 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 797 "cf-parse.y" /* yacc.c:1646  */
    {
     if (!ip_is_prefix((yyvsp[-1].a), (yyvsp[0].i))) cf_error("Invalid prefix");
     (yyval.px).addr = (yyvsp[-1].a); (yyval.px).len = (yyvsp[0].i);
   }
#line 3924 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 805 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.px).addr = (yyvsp[0].a); (yyval.px).len = BITS_PER_IP_ADDRESS; }
#line 3930 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 809 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].i) < 0 || (yyvsp[0].i) > BITS_PER_IP_ADDRESS) cf_error("Invalid prefix length %d", (yyvsp[0].i));
     (yyval.i) = (yyvsp[0].i);
   }
#line 3939 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 813 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.i) = ipa_masklen((yyvsp[0].a));
     if ((yyval.i) < 0) cf_error("Invalid netmask %I", (yyvsp[0].a));
   }
#line 3948 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 820 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.time) = tm_parse_datetime((yyvsp[0].t));
     if (!(yyval.time))
       cf_error("Invalid date and time");
   }
#line 3958 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 829 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class != (SYM_CONSTANT | T_STRING)) cf_error("String expected");
     (yyval.t) = SYM_VAL((yyvsp[0].s)).s;
   }
#line 3967 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 836 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.t) = (yyvsp[0].t); }
#line 3973 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 837 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.t) = NULL; }
#line 3979 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 843 "cf-parse.y" /* yacc.c:1646  */
    {
    struct log_config *c = cfg_allocz(sizeof(struct log_config));
    c->fh = (yyvsp[-2].g);
    c->mask = (yyvsp[-1].i);
    add_tail(&new_config->logfiles, &c->n);
  }
#line 3990 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 852 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.t) = (yyvsp[0].t); }
#line 3996 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 853 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.t) = bird_name; }
#line 4002 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 857 "cf-parse.y" /* yacc.c:1646  */
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[0].t), "a");
     if (!f) cf_error("Unable to open log file `%s': %m", (yyvsp[0].t));
     (yyval.g) = f;
   }
#line 4012 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 862 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.g) = NULL; new_config->syslog_name = (yyvsp[0].t); }
#line 4018 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 863 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.g) = stderr; }
#line 4024 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 867 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ~0; }
#line 4030 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 868 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 4036 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 872 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 1 << (yyvsp[0].i); }
#line 4042 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 873 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-2].i) | (1 << (yyvsp[0].i)); }
#line 4048 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 877 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_DEBUG[0]; }
#line 4054 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 878 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_TRACE[0]; }
#line 4060 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 879 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_INFO[0]; }
#line 4066 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 880 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_REMOTE[0]; }
#line 4072 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 881 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_WARN[0]; }
#line 4078 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 882 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_ERR[0]; }
#line 4084 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 883 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_AUTH[0]; }
#line 4090 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 884 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_FATAL[0]; }
#line 4096 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 885 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = L_BUG[0]; }
#line 4102 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 891 "cf-parse.y" /* yacc.c:1646  */
    { new_config->proto_default_mrtdump = (yyvsp[-1].i); }
#line 4108 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 892 "cf-parse.y" /* yacc.c:1646  */
    {
     FILE *f = tracked_fopen(new_config->pool, (yyvsp[-1].t), "a");
     if (!f) cf_error("Unable to open MRTDump file '%s': %m", (yyvsp[-1].t));
     new_config->mrtdump_file = fileno(f);
   }
#line 4118 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 902 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.tf) = &new_config->tf_route; }
#line 4124 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 903 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.tf) = &new_config->tf_proto; }
#line 4130 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 904 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.tf) = &new_config->tf_base; }
#line 4136 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 905 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.tf) = &new_config->tf_log; }
#line 4142 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 908 "cf-parse.y" /* yacc.c:1646  */
    { *(yyvsp[-1].tf) = (struct timeformat){(yyvsp[0].t), NULL, 0}; }
#line 4148 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 909 "cf-parse.y" /* yacc.c:1646  */
    { *(yyvsp[-3].tf) = (struct timeformat){(yyvsp[-2].t), (yyvsp[0].t), (yyvsp[-1].i)}; }
#line 4154 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 910 "cf-parse.y" /* yacc.c:1646  */
    { *(yyvsp[-2].tf) = (struct timeformat){"%T", "%F", 20*3600}; }
#line 4160 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 911 "cf-parse.y" /* yacc.c:1646  */
    { *(yyvsp[-2].tf) = (struct timeformat){"%F %T", NULL, 0}; }
#line 4166 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 912 "cf-parse.y" /* yacc.c:1646  */
    { *(yyvsp[-2].tf) = (struct timeformat){NULL, NULL, 0}; }
#line 4172 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 913 "cf-parse.y" /* yacc.c:1646  */
    { *(yyvsp[-2].tf) = (struct timeformat){"%d-%m-%Y %T", NULL, 0}; }
#line 4178 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 923 "cf-parse.y" /* yacc.c:1646  */
    { new_config->latency_debug = (yyvsp[0].i); }
#line 4184 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 924 "cf-parse.y" /* yacc.c:1646  */
    { new_config->latency_limit = (yyvsp[0].i32); }
#line 4190 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 925 "cf-parse.y" /* yacc.c:1646  */
    { new_config->watchdog_warning = (yyvsp[0].i32); }
#line 4196 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 926 "cf-parse.y" /* yacc.c:1646  */
    { new_config->watchdog_timeout = ((yyvsp[0].i32) + 999999) TO_S; }
#line 4202 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 935 "cf-parse.y" /* yacc.c:1646  */
    { cmd_reconfig((yyvsp[-2].t), RECONFIG_HARD, (yyvsp[-1].i)); }
#line 4208 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 938 "cf-parse.y" /* yacc.c:1646  */
    { cmd_reconfig((yyvsp[-2].t), RECONFIG_SOFT, (yyvsp[-1].i)); }
#line 4214 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 945 "cf-parse.y" /* yacc.c:1646  */
    { cmd_reconfig_confirm(); }
#line 4220 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 948 "cf-parse.y" /* yacc.c:1646  */
    { cmd_reconfig_undo(); }
#line 4226 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 951 "cf-parse.y" /* yacc.c:1646  */
    { cmd_check_config((yyvsp[-1].t)); }
#line 4232 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 954 "cf-parse.y" /* yacc.c:1646  */
    { cmd_shutdown(); }
#line 4238 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 957 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.t) = NULL; }
#line 4244 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 962 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 4250 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 963 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = UNIX_DEFAULT_CONFIGURE_TIMEOUT; }
#line 4256 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 964 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[0].i); }
#line 4262 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 972 "cf-parse.y" /* yacc.c:1646  */
    { this_proto = krt_init_config((yyvsp[-1].i)); }
#line 4268 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 977 "cf-parse.y" /* yacc.c:1646  */
    { THIS_KRT->persist = (yyvsp[0].i); }
#line 4274 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 978 "cf-parse.y" /* yacc.c:1646  */
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = (yyvsp[0].i);
   }
#line 4283 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 982 "cf-parse.y" /* yacc.c:1646  */
    {
      THIS_KRT->learn = (yyvsp[0].i);
#ifndef KRT_ALLOW_LEARN
      if ((yyvsp[0].i))
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   }
#line 4295 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 989 "cf-parse.y" /* yacc.c:1646  */
    { THIS_KRT->devroutes = (yyvsp[0].i); }
#line 4301 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 990 "cf-parse.y" /* yacc.c:1646  */
    { THIS_KRT->graceful_restart = (yyvsp[0].i); }
#line 4307 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 991 "cf-parse.y" /* yacc.c:1646  */
    { THIS_KRT->merge_paths = (yyvsp[0].i) ? KRT_DEFAULT_ECMP_LIMIT : 0; }
#line 4313 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 992 "cf-parse.y" /* yacc.c:1646  */
    { THIS_KRT->merge_paths = (yyvsp[-2].i) ? (yyvsp[0].i) : 0; if (((yyvsp[0].i) <= 0) || ((yyvsp[0].i) > 255)) cf_error("Merge paths limit must be in range 1-255"); }
#line 4319 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 998 "cf-parse.y" /* yacc.c:1646  */
    { this_proto = kif_init_config((yyvsp[-1].i)); }
#line 4325 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1003 "cf-parse.y" /* yacc.c:1646  */
    {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = (yyvsp[0].i);
   }
#line 4334 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1007 "cf-parse.y" /* yacc.c:1646  */
    {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = (yyvsp[-1].t);
     kpi->prefix = (yyvsp[0].px).addr;
     kpi->pxlen = (yyvsp[0].px).len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   }
#line 4346 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1021 "cf-parse.y" /* yacc.c:1646  */
    {
	THIS_KRT->sys.table_id = (yyvsp[0].i);
   }
#line 4354 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1037 "cf-parse.y" /* yacc.c:1646  */
    { new_config->router_id = (yyvsp[-1].i32); }
#line 4360 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1038 "cf-parse.y" /* yacc.c:1646  */
    { new_config->router_id_from = this_ipatt; }
#line 4366 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1042 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = (yyvsp[0].i); }
#line 4372 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1043 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = f_eval_int((yyvsp[-1].x)); }
#line 4378 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1045 "cf-parse.y" /* yacc.c:1646  */
    {
#ifndef IPV6
     (yyval.i32) = ipa_to_u32((yyvsp[0].a));
#else
     cf_error("Router IDs must be entered as hexadecimal numbers or IPv4 addresses in IPv6 version");
#endif
   }
#line 4390 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1052 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class == (SYM_CONSTANT | T_INT) || (yyvsp[0].s)->class == (SYM_CONSTANT | T_QUAD))
       (yyval.i32) = SYM_VAL((yyvsp[0].s)).i;
#ifndef IPV6
     else if ((yyvsp[0].s)->class == (SYM_CONSTANT | T_IP))
       (yyval.i32) = ipa_to_u32(SYM_VAL((yyvsp[0].s)).px.ip);
#endif
     else
       cf_error("Number or IPv4 address constant expected");
   }
#line 4405 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1074 "cf-parse.y" /* yacc.c:1646  */
    { new_config->listen_bgp_addr = (yyvsp[0].a); }
#line 4411 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1075 "cf-parse.y" /* yacc.c:1646  */
    { new_config->listen_bgp_port = (yyvsp[0].i); }
#line 4417 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1076 "cf-parse.y" /* yacc.c:1646  */
    { new_config->listen_bgp_flags = 0; }
#line 4423 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1077 "cf-parse.y" /* yacc.c:1646  */
    { new_config->listen_bgp_flags = 1; }
#line 4429 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1082 "cf-parse.y" /* yacc.c:1646  */
    { new_config->gr_wait = (yyvsp[-1].i); }
#line 4435 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1088 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 4441 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1089 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 1; }
#line 4447 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1093 "cf-parse.y" /* yacc.c:1646  */
    {
   struct rtable_config *cf;
   cf = rt_new_table((yyvsp[-1].s));
   cf->sorted = (yyvsp[0].i);
   }
#line 4457 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1101 "cf-parse.y" /* yacc.c:1646  */
    {
  this_roa_table = roa_new_table_config((yyvsp[0].s));
}
#line 4465 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 1107 "cf-parse.y" /* yacc.c:1646  */
    {
     roa_add_item_config(this_roa_table, (yyvsp[-5].px).addr, (yyvsp[-5].px).len, (yyvsp[-3].i), (yyvsp[-1].i));
   }
#line 4473 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1121 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = SYM_PROTO; }
#line 4479 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1122 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = SYM_TEMPLATE; }
#line 4485 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1126 "cf-parse.y" /* yacc.c:1646  */
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = this_proto->class;
     s->def = this_proto;
     this_proto->name = s->name;
     }
#line 4496 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1132 "cf-parse.y" /* yacc.c:1646  */
    {
     cf_define_symbol((yyvsp[0].s), this_proto->class, this_proto);
     this_proto->name = (yyvsp[0].s)->name;
   }
#line 4505 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1136 "cf-parse.y" /* yacc.c:1646  */
    {
     struct symbol *s = cf_default_name(this_proto->protocol->template, &this_proto->protocol->name_counter);
     s->class = this_proto->class;
     s->def = this_proto;
     this_proto->name = s->name;

     if (((yyvsp[0].s)->class != SYM_TEMPLATE) && ((yyvsp[0].s)->class != SYM_PROTO)) cf_error("Template or protocol name expected");
     proto_copy_config(this_proto, (yyvsp[0].s)->def);
   }
#line 4519 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1145 "cf-parse.y" /* yacc.c:1646  */
    {
     cf_define_symbol((yyvsp[-2].s), this_proto->class, this_proto);
     this_proto->name = (yyvsp[-2].s)->name;

     if (((yyvsp[0].s)->class != SYM_TEMPLATE) && ((yyvsp[0].s)->class != SYM_PROTO)) cf_error("Template or protocol name expected");
     proto_copy_config(this_proto, (yyvsp[0].s)->def);
   }
#line 4531 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1156 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].i) < 0 || (yyvsp[0].i) > 0xFFFF) cf_error("Invalid preference");
     this_proto->preference = (yyvsp[0].i);
   }
#line 4540 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1160 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->disabled = (yyvsp[0].i); }
#line 4546 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1161 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->debug = (yyvsp[0].i); }
#line 4552 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1162 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->mrtdump = (yyvsp[0].i); }
#line 4558 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1163 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->in_filter = (yyvsp[0].f); }
#line 4564 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1164 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->out_filter = (yyvsp[0].f); }
#line 4570 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1165 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->rx_limit = (yyvsp[0].g); }
#line 4576 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1166 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->in_limit = (yyvsp[0].g); }
#line 4582 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1167 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->out_limit = (yyvsp[0].g); }
#line 4588 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1168 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->in_keep_filtered = (yyvsp[0].i); }
#line 4594 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1169 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->table = (yyvsp[0].r); }
#line 4600 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1170 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->router_id = (yyvsp[0].i32); }
#line 4606 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1171 "cf-parse.y" /* yacc.c:1646  */
    { this_proto->dsc = (yyvsp[0].t); }
#line 4612 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1175 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.f) = (yyvsp[0].f); }
#line 4618 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1177 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.f) = FILTER_ACCEPT; }
#line 4624 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1178 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.f) = FILTER_REJECT; }
#line 4630 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1182 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = PLA_DISABLE; }
#line 4636 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1183 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = PLA_WARN; }
#line 4642 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1184 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = PLA_BLOCK; }
#line 4648 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1185 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = PLA_RESTART; }
#line 4654 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1186 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = PLA_DISABLE; }
#line 4660 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1190 "cf-parse.y" /* yacc.c:1646  */
    {
     struct proto_limit *l = cfg_allocz(sizeof(struct proto_limit));
     l->limit = (yyvsp[-1].i);
     l->action = (yyvsp[0].i);
     (yyval.g) = l;
   }
#line 4671 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1196 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.g) = NULL; }
#line 4677 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1200 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class != SYM_TABLE) cf_error("Table name expected");
     (yyval.r) = (yyvsp[0].s)->def;
   }
#line 4686 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1208 "cf-parse.y" /* yacc.c:1646  */
    { new_config->proto_default_debug = (yyvsp[0].i); }
#line 4692 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1209 "cf-parse.y" /* yacc.c:1646  */
    { new_config->cli_debug = (yyvsp[0].i); }
#line 4698 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1217 "cf-parse.y" /* yacc.c:1646  */
    {
     struct iface_patt_node *ipn = cfg_allocz(sizeof(struct iface_patt_node));
     add_tail(&this_ipatt->ipn_list, NODE ipn);
     this_ipn = ipn;
   }
#line 4708 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1225 "cf-parse.y" /* yacc.c:1646  */
    { this_ipn->pattern = (yyvsp[0].t); this_ipn->prefix = IPA_NONE; this_ipn->pxlen = 0; }
#line 4714 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1226 "cf-parse.y" /* yacc.c:1646  */
    { this_ipn->pattern = NULL; this_ipn->prefix = (yyvsp[0].px).addr; this_ipn->pxlen = (yyvsp[0].px).len; }
#line 4720 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1227 "cf-parse.y" /* yacc.c:1646  */
    { this_ipn->pattern = (yyvsp[-1].t); this_ipn->prefix = (yyvsp[0].px).addr; this_ipn->pxlen = (yyvsp[0].px).len; }
#line 4726 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1231 "cf-parse.y" /* yacc.c:1646  */
    { this_ipn->positive = 1; }
#line 4732 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1232 "cf-parse.y" /* yacc.c:1646  */
    { this_ipn->positive = 0; }
#line 4738 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1246 "cf-parse.y" /* yacc.c:1646  */
    { iface_patt_check(); }
#line 4744 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1248 "cf-parse.y" /* yacc.c:1646  */
    {
   /* Generic this_ipatt init */
   this_ipatt = cfg_allocz(sizeof(struct iface_patt));
   init_list(&this_ipatt->ipn_list);
 }
#line 4754 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1260 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[0].i) & 0xfc;        if (((yyvsp[0].i) < 0) || ((yyvsp[0].i) > 255)) cf_error("TX class must be in range 0-255"); }
#line 4760 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1261 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ((yyvsp[0].i) & 0x3f) << 2; if (((yyvsp[0].i) < 0) || ((yyvsp[0].i) > 63))  cf_error("TX DSCP must be in range 0-63"); }
#line 4766 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1267 "cf-parse.y" /* yacc.c:1646  */
    {
     this_proto = proto_config_new(&proto_device, (yyvsp[-1].i));
     init_list(&DIRECT_CFG->iface_list);
   }
#line 4775 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1277 "cf-parse.y" /* yacc.c:1646  */
    { DIRECT_CFG->check_link = (yyvsp[-1].i); }
#line 4781 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1281 "cf-parse.y" /* yacc.c:1646  */
    {
     this_ipatt = cfg_allocz(sizeof(struct iface_patt));
     add_tail(&DIRECT_CFG->iface_list, NODE this_ipatt);
     init_list(&this_ipatt->ipn_list);
   }
#line 4791 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1295 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ~0; }
#line 4797 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1296 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 4803 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1297 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 4809 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1302 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-2].i) | (yyvsp[0].i); }
#line 4815 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1306 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = D_STATES; }
#line 4821 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1307 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = D_ROUTES; }
#line 4827 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1308 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = D_FILTERS; }
#line 4833 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1309 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = D_IFACES; }
#line 4839 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1310 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = D_EVENTS; }
#line 4845 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1311 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = D_PACKETS; }
#line 4851 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1317 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ~0; }
#line 4857 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1318 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 4863 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1319 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 4869 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1324 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-2].i) | (yyvsp[0].i); }
#line 4875 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1328 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = MD_STATES; }
#line 4881 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1329 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = MD_MESSAGES; }
#line 4887 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1350 "cf-parse.y" /* yacc.c:1646  */
    {
     if (!this_p_list) {
     	this_p_list = cfg_alloc(sizeof(union list));
     	init_list(this_p_list);
        password_id = 1;
     }
     this_p_item = cfg_alloc(sizeof (struct password_item));
     this_p_item->password = (yyvsp[0].t);
     this_p_item->genfrom = 0;
     this_p_item->gento = TIME_INFINITY;
     this_p_item->accfrom = 0;
     this_p_item->accto = TIME_INFINITY;
     this_p_item->id = password_id++;
     add_tail(this_p_list, &this_p_item->n);
   }
#line 4907 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1368 "cf-parse.y" /* yacc.c:1646  */
    { }
#line 4913 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1369 "cf-parse.y" /* yacc.c:1646  */
    { this_p_item->genfrom = (yyvsp[-2].time); }
#line 4919 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1370 "cf-parse.y" /* yacc.c:1646  */
    { this_p_item->gento = (yyvsp[-2].time); }
#line 4925 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1371 "cf-parse.y" /* yacc.c:1646  */
    { this_p_item->accfrom = (yyvsp[-2].time); }
#line 4931 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1372 "cf-parse.y" /* yacc.c:1646  */
    { this_p_item->accto = (yyvsp[-2].time); }
#line 4937 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 1373 "cf-parse.y" /* yacc.c:1646  */
    { this_p_item->id = (yyvsp[-2].i); if ((yyvsp[-2].i) <= 0) cf_error("Password ID has to be greated than zero."); }
#line 4943 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 1382 "cf-parse.y" /* yacc.c:1646  */
    { cmd_show_status(); }
#line 4949 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 1385 "cf-parse.y" /* yacc.c:1646  */
    { cmd_show_memory(); }
#line 4955 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 1388 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_show, 0, 0); }
#line 4961 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 1391 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_show, 0, 1); }
#line 4967 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 206:
#line 1395 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.s) = NULL; }
#line 4973 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 1399 "cf-parse.y" /* yacc.c:1646  */
    { if_show(); }
#line 4979 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 1402 "cf-parse.y" /* yacc.c:1646  */
    { if_show_summary(); }
#line 4985 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 1406 "cf-parse.y" /* yacc.c:1646  */
    { rt_show((yyvsp[-1].ra)); }
#line 4991 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 1409 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = cfg_allocz(sizeof(struct rt_show_data));
     (yyval.ra)->pxlen = 256;
     (yyval.ra)->filter = FILTER_ACCEPT;
   }
#line 5001 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 1414 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[0].px).addr;
     (yyval.ra)->pxlen = (yyvsp[0].px).len;
   }
#line 5012 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 1420 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->pxlen != 256) cf_error("Only one prefix expected");
     (yyval.ra)->prefix = (yyvsp[0].px).addr;
     (yyval.ra)->pxlen = (yyvsp[0].px).len;
     (yyval.ra)->show_for = 1;
   }
#line 5024 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 1427 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyvsp[0].s)->class != SYM_TABLE) cf_error("%s is not a table", (yyvsp[0].s)->name);
     (yyval.ra)->table = ((struct rtable_config *)(yyvsp[0].s)->def)->table;
   }
#line 5034 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 1432 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[0].f);
   }
#line 5044 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 1437 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     if ((yyval.ra)->filter != FILTER_ACCEPT) cf_error("Filter specified twice");
     (yyval.ra)->filter = (yyvsp[0].f);
   }
#line 5054 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 1442 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->verbose = 1;
   }
#line 5063 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 1446 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->primary_only = 1;
   }
#line 5072 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 218:
#line 1450 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->filtered = 1;
   }
#line 5081 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 1454 "cf-parse.y" /* yacc.c:1646  */
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[0].s)->def;
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->export_mode) cf_error("Protocol specified twice");
     if ((yyvsp[0].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[0].s)->name);
     (yyval.ra)->export_mode = (yyvsp[-1].i);
     (yyval.ra)->export_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   }
#line 5095 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 1463 "cf-parse.y" /* yacc.c:1646  */
    {
     struct proto_config *c = (struct proto_config *) (yyvsp[0].s)->def;
     (yyval.ra) = (yyvsp[-2].ra);
     if ((yyval.ra)->show_protocol) cf_error("Protocol specified twice");
     if ((yyvsp[0].s)->class != SYM_PROTO || !c->proto) cf_error("%s is not a protocol", (yyvsp[0].s)->name);
     (yyval.ra)->show_protocol = c->proto;
     (yyval.ra)->running_on_config = c->proto->cf->global;
   }
#line 5108 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 1471 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->stats = 1;
   }
#line 5117 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 1475 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ra) = (yyvsp[-1].ra);
     (yyval.ra)->stats = 2;
   }
#line 5126 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 1482 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RSEM_PREEXPORT; }
#line 5132 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 1483 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RSEM_EXPORT; }
#line 5138 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 1484 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RSEM_NOEXPORT; }
#line 5144 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 226:
#line 1490 "cf-parse.y" /* yacc.c:1646  */
    { roa_show((yyvsp[-1].ro)); }
#line 5150 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 1493 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ro) = cfg_allocz(sizeof(struct roa_show_data));
     (yyval.ro)->mode = ROA_SHOW_ALL;
     (yyval.ro)->table = roa_table_default;
     if (roa_table_default == NULL)
       cf_error("No ROA table defined");
   }
#line 5162 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 1500 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ro) = (yyvsp[-2].ro);
     if ((yyval.ro)->mode != ROA_SHOW_ALL) cf_error("Only one prefix expected");
     (yyval.ro)->prefix = (yyvsp[0].px).addr;
     (yyval.ro)->pxlen = (yyvsp[0].px).len;
     (yyval.ro)->mode = (yyvsp[-1].i);
   }
#line 5174 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 1507 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ro) = (yyvsp[-2].ro);
     (yyval.ro)->asn = (yyvsp[0].i);
   }
#line 5183 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 230:
#line 1511 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ro) = (yyvsp[-2].ro);
     if ((yyvsp[0].s)->class != SYM_ROA) cf_error("%s is not a ROA table", (yyvsp[0].s)->name);
     (yyval.ro)->table = ((struct roa_table_config *)(yyvsp[0].s)->def)->table;
   }
#line 5193 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 1519 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ROA_SHOW_PX; }
#line 5199 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 232:
#line 1520 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ROA_SHOW_IN; }
#line 5205 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 233:
#line 1521 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ROA_SHOW_FOR; }
#line 5211 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 1527 "cf-parse.y" /* yacc.c:1646  */
    { cmd_show_symbols((yyvsp[-1].sd)); }
#line 5217 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 1530 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.sd) = cfg_allocz(sizeof(struct sym_show_data));
   }
#line 5225 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 1533 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->type = SYM_TABLE; }
#line 5231 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 1534 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->type = SYM_FUNCTION; }
#line 5237 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 1535 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->type = SYM_FILTER; }
#line 5243 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 1536 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->type = SYM_PROTO; }
#line 5249 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 1537 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->type = SYM_TEMPLATE; }
#line 5255 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 1538 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->type = SYM_ROA; }
#line 5261 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 1539 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.sd) = (yyvsp[-1].sd); (yyval.sd)->sym = (yyvsp[0].s); }
#line 5267 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 1544 "cf-parse.y" /* yacc.c:1646  */
    { 
     if (roa_table_default == NULL)
       cf_error("No ROA table defined");
     (yyval.rot) = roa_table_default;
   }
#line 5277 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 1549 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class != SYM_ROA)
       cf_error("%s is not a ROA table", (yyvsp[0].s)->name);
     (yyval.rot) = ((struct roa_table_config *)(yyvsp[0].s)->def)->table;
   }
#line 5287 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 1558 "cf-parse.y" /* yacc.c:1646  */
    {
  if (! cli_access_restricted())
    { roa_add_item((yyvsp[-1].rot), (yyvsp[-6].px).addr, (yyvsp[-6].px).len, (yyvsp[-4].i), (yyvsp[-2].i), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
#line 5296 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 1565 "cf-parse.y" /* yacc.c:1646  */
    {
  if (! cli_access_restricted())
    { roa_delete_item((yyvsp[-1].rot), (yyvsp[-6].px).addr, (yyvsp[-6].px).len, (yyvsp[-4].i), (yyvsp[-2].i), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
#line 5305 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 1572 "cf-parse.y" /* yacc.c:1646  */
    {
  if (! cli_access_restricted())
    { roa_flush((yyvsp[-1].rot), ROA_SRC_DYNAMIC); cli_msg(0, ""); }
}
#line 5314 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 1580 "cf-parse.y" /* yacc.c:1646  */
    { rdump(&root_pool); cli_msg(0, ""); }
#line 5320 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 1582 "cf-parse.y" /* yacc.c:1646  */
    { sk_dump_all(); cli_msg(0, ""); }
#line 5326 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 1584 "cf-parse.y" /* yacc.c:1646  */
    { io_log_dump(); cli_msg(0, ""); }
#line 5332 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 1586 "cf-parse.y" /* yacc.c:1646  */
    { if_dump_all(); cli_msg(0, ""); }
#line 5338 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 1588 "cf-parse.y" /* yacc.c:1646  */
    { neigh_dump_all(); cli_msg(0, ""); }
#line 5344 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 1590 "cf-parse.y" /* yacc.c:1646  */
    { rta_dump_all(); cli_msg(0, ""); }
#line 5350 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 1592 "cf-parse.y" /* yacc.c:1646  */
    { rt_dump_all(); cli_msg(0, ""); }
#line 5356 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 1594 "cf-parse.y" /* yacc.c:1646  */
    { protos_dump_all(); cli_msg(0, ""); }
#line 5362 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 1597 "cf-parse.y" /* yacc.c:1646  */
    { cmd_eval((yyvsp[-1].x)); }
#line 5368 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 1600 "cf-parse.y" /* yacc.c:1646  */
    {
  cli_set_log_echo(this_cli, (yyvsp[-2].i), (yyvsp[-1].i));
  cli_msg(0, "");
}
#line 5377 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 1606 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = ~0; }
#line 5383 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 1607 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 5389 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 1608 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[-1].i); }
#line 5395 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 1612 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 4096; }
#line 5401 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 1613 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].i) < 256 || (yyvsp[0].i) > 65536) cf_error("Invalid log struct buffer size");
     (yyval.i) = (yyvsp[0].i);
   }
#line 5410 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 1620 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_disable, 1, 0); }
#line 5416 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 1622 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_enable, 1, 0); }
#line 5422 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 1624 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_restart, 1, 0); }
#line 5428 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 1626 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_reload, 1, CMD_RELOAD); }
#line 5434 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 1628 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_reload, 1, CMD_RELOAD_IN); }
#line 5440 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 1630 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-1].ps), proto_cmd_reload, 1, CMD_RELOAD_OUT); }
#line 5446 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 1634 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-2].ps), proto_cmd_debug, 1, (yyvsp[-1].i)); }
#line 5452 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 1638 "cf-parse.y" /* yacc.c:1646  */
    { proto_apply_cmd((yyvsp[-2].ps), proto_cmd_mrtdump, 1, (yyvsp[-1].i)); }
#line 5458 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 1641 "cf-parse.y" /* yacc.c:1646  */
    { this_cli->restricted = 1; cli_msg(16, "Access restricted"); }
#line 5464 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 1644 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ps).ptr = (yyvsp[0].s); (yyval.ps).patt = 0; }
#line 5470 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 1645 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; }
#line 5476 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 1646 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ps).ptr = (yyvsp[0].t); (yyval.ps).patt = 1; }
#line 5482 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 1650 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ps).ptr = (yyvsp[0].s); (yyval.ps).patt = 0; }
#line 5488 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 1651 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ps).ptr = NULL; (yyval.ps).patt = 1; }
#line 5494 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 1652 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ps).ptr = (yyvsp[0].t); (yyval.ps).patt = 1; }
#line 5500 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 1660 "cf-parse.y" /* yacc.c:1646  */
    { (yyvsp[0].s) = cf_define_symbol((yyvsp[0].s), SYM_FILTER, NULL); cf_push_scope( (yyvsp[0].s) ); }
#line 5506 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 1661 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyvsp[-2].s)->def = (yyvsp[0].f);
     (yyvsp[0].f)->name = (yyvsp[-2].s)->name;
     DBG( "We have new filter defined (%s)\n", (yyvsp[-2].s)->name );
     cf_pop_scope();
   }
#line 5517 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 1670 "cf-parse.y" /* yacc.c:1646  */
    { f_eval_int((yyvsp[0].x)); }
#line 5523 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 1674 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_INT; }
#line 5529 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 1675 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_BOOL; }
#line 5535 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 1676 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_IP; }
#line 5541 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 1677 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_PREFIX; }
#line 5547 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 1678 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_PAIR; }
#line 5553 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 1679 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_QUAD; }
#line 5559 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 1680 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_EC; }
#line 5565 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 1681 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_STRING; }
#line 5571 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 1682 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_PATH_MASK; }
#line 5577 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 1683 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_PATH; }
#line 5583 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 1684 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_CLIST; }
#line 5589 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 1685 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = T_ECLIST; }
#line 5595 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 1686 "cf-parse.y" /* yacc.c:1646  */
    { 
	switch ((yyvsp[-1].i)) {
	  case T_INT:
	  case T_PAIR:
	  case T_QUAD:
	  case T_EC:
	  case T_IP:
	       (yyval.i) = T_SET;
	       break;

	  case T_PREFIX:
	       (yyval.i) = T_PREFIX_SET;
	    break;

	  default:
		cf_error( "You can't create sets of this type." );
	}
   }
#line 5618 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 1707 "cf-parse.y" /* yacc.c:1646  */
    {
     struct f_val * val = cfg_alloc(sizeof(struct f_val));
     val->type = T_VOID;
     (yyvsp[0].s) = cf_define_symbol((yyvsp[0].s), SYM_VARIABLE | (yyvsp[-1].i), val);
     DBG( "New variable %s type %x\n", (yyvsp[0].s)->name, (yyvsp[-1].i) );
     (yyvsp[0].s)->aux2 = NULL;
     (yyval.s)=(yyvsp[0].s);
   }
#line 5631 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 1718 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.s) = NULL; }
#line 5637 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 1719 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.s) = (yyvsp[-2].s);
     (yyval.s)->aux2 = (yyvsp[0].s);
   }
#line 5646 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 1726 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.s) = (yyvsp[0].s); }
#line 5652 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 1727 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.s) = (yyvsp[-2].s);
     (yyval.s)->aux2 = (yyvsp[0].s);
   }
#line 5661 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 1734 "cf-parse.y" /* yacc.c:1646  */
    {
     struct filter *f = cfg_alloc(sizeof(struct filter));
     f->name = NULL;
     f->root = (yyvsp[0].x);
     (yyval.f) = f;
   }
#line 5672 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 1743 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[0].s)->class != SYM_FILTER) cf_error("No such filter.");
     (yyval.f) = (yyvsp[0].s)->def;
   }
#line 5681 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 1751 "cf-parse.y" /* yacc.c:1646  */
    {
     /* Construct 'IF term THEN ACCEPT; REJECT;' */
     struct filter *f = cfg_alloc(sizeof(struct filter));
     struct f_inst *i, *acc, *rej;
     acc = f_new_inst();		/* ACCEPT */
     acc->code = P('p',',');
     acc->a1.p = NULL;
     acc->a2.i = F_ACCEPT;
     rej = f_new_inst();		/* REJECT */
     rej->code = P('p',',');
     rej->a1.p = NULL;
     rej->a2.i = F_REJECT;
     i = f_new_inst();			/* IF */
     i->code = '?';
     i->a1.p = (yyvsp[0].x);
     i->a2.p = acc;
     i->next = rej;
     f->name = NULL;
     f->root = i;
     (yyval.f) = f;
  }
#line 5707 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 1775 "cf-parse.y" /* yacc.c:1646  */
    { DBG( "Have function parameters\n" ); (yyval.s)=(yyvsp[-1].s); }
#line 5713 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 1776 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.s)=NULL; }
#line 5719 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 1780 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[-3].s)) {
       /* Prepend instruction to clear local variables */
       (yyval.x) = f_new_inst();
       (yyval.x)->code = P('c','v');
       (yyval.x)->a1.p = (yyvsp[-3].s);
       (yyval.x)->next = (yyvsp[-1].x);
     } else
       (yyval.x) = (yyvsp[-1].x);
   }
#line 5734 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 1793 "cf-parse.y" /* yacc.c:1646  */
    { DBG( "Beginning of function %s\n", (yyvsp[0].s)->name );
     (yyvsp[0].s) = cf_define_symbol((yyvsp[0].s), SYM_FUNCTION, NULL);
     cf_push_scope((yyvsp[0].s));
   }
#line 5743 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 1796 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyvsp[-3].s)->def = (yyvsp[0].x);
     (yyvsp[-3].s)->aux2 = (yyvsp[-1].s);
     DBG("Hmm, we've got one function here - %s\n", (yyvsp[-3].s)->name); 
     cf_pop_scope();
   }
#line 5754 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 308:
#line 1809 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = NULL; }
#line 5760 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 1810 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x)->next; (yyvsp[0].x)->next = NULL; }
#line 5766 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 1813 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); (yyvsp[0].x)->next = (yyvsp[0].x); }
#line 5772 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 1814 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); (yyvsp[0].x)->next = (yyvsp[-1].x)->next ; (yyvsp[-1].x)->next = (yyvsp[0].x); }
#line 5778 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 1818 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x)=(yyvsp[0].x);
   }
#line 5786 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 1821 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x)=(yyvsp[-1].x);
   }
#line 5794 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 1830 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = T_IP; (yyval.v).val.px.ip = (yyvsp[0].a); }
#line 5800 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 1843 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[0].i); }
#line 5806 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 1844 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[0].i32); }
#line 5812 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 1845 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v) = (yyvsp[0].v); }
#line 5818 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 1846 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = pair_a((yyvsp[0].i)); (yyval.v).val.i = pair_b((yyvsp[0].i)); }
#line 5824 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 1847 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.v) = f_eval((yyvsp[-1].x), cfg_mem);
     if (!f_valid_set_type((yyval.v).type)) cf_error("Set-incompatible type");
   }
#line 5833 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 320:
#line 1851 "cf-parse.y" /* yacc.c:1646  */
    {
     if (!cf_symbol_is_constant((yyvsp[0].s))) cf_error("%s: constant expected", (yyvsp[0].s)->name);
     if (!f_valid_set_type(SYM_TYPE((yyvsp[0].s)))) cf_error("%s: set-incompatible type", (yyvsp[0].s)->name);
     (yyval.v) = *(struct f_val *)((yyvsp[0].s)->def);
   }
#line 5843 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 1859 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = T_INT; (yyval.v).val.i = (yyvsp[0].i); }
#line 5849 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 1860 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = T_INT; (yyval.v).val.i = f_eval_int((yyvsp[-1].x)); }
#line 5855 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 1861 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = T_QUAD; (yyval.v).val.i = (yyvsp[0].i32); }
#line 5861 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 1862 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v) = (yyvsp[0].v); }
#line 5867 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 325:
#line 1863 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v).type = pair_a((yyvsp[0].i)); (yyval.v).val.i = pair_b((yyvsp[0].i)); }
#line 5873 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 326:
#line 1867 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = f_eval_int((yyvsp[0].x)); check_u16((yyval.i)); }
#line 5879 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 327:
#line 1870 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = pair((yyvsp[0].i), (yyvsp[0].i)); }
#line 5885 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 328:
#line 1871 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = pair((yyvsp[-2].i), (yyvsp[0].i)); }
#line 5891 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 329:
#line 1872 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = 0xFFFF; }
#line 5897 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 330:
#line 1876 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.e) = f_new_pair_set(pair_a((yyvsp[-3].i32)), pair_b((yyvsp[-3].i32)), pair_a((yyvsp[-1].i32)), pair_b((yyvsp[-1].i32)));
   }
#line 5905 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 331:
#line 1879 "cf-parse.y" /* yacc.c:1646  */
    {
     /* Hack: $2 and $4 should be pair_expr, but that would cause shift/reduce conflict */
     if ((pair_a((yyvsp[-9].i32)) != pair_b((yyvsp[-9].i32))) || (pair_a((yyvsp[-7].i32)) != pair_b((yyvsp[-7].i32))))
       cf_error("syntax error");
     (yyval.e) = f_new_pair_item(pair_b((yyvsp[-9].i32)), (yyvsp[-3].i), pair_b((yyvsp[-7].i32)), (yyvsp[-1].i)); 
   }
#line 5916 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 332:
#line 1888 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i32) = f_eval_int((yyvsp[0].x)); }
#line 5922 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 333:
#line 1891 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = EC_RT; }
#line 5928 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 334:
#line 1892 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = EC_RO; }
#line 5934 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 335:
#line 1893 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[0].i); }
#line 5940 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 336:
#line 1894 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = EC_GENERIC; }
#line 5946 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 337:
#line 1898 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_new_ec_item((yyvsp[-5].i), 0, (yyvsp[-3].i32), (yyvsp[-1].i32), (yyvsp[-1].i32)); }
#line 5952 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 338:
#line 1899 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_new_ec_item((yyvsp[-7].i), 0, (yyvsp[-5].i32), (yyvsp[-3].i32), (yyvsp[-1].i32)); }
#line 5958 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 339:
#line 1900 "cf-parse.y" /* yacc.c:1646  */
    {  (yyval.e) = f_new_ec_item((yyvsp[-5].i), 0, (yyvsp[-3].i32), 0, EC_ALL); }
#line 5964 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 342:
#line 1906 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_new_item((yyvsp[0].v), (yyvsp[0].v)); }
#line 5970 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 343:
#line 1907 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_new_item((yyvsp[-2].v), (yyvsp[0].v)); }
#line 5976 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 346:
#line 1913 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_new_item((yyvsp[0].v), (yyvsp[0].v)); }
#line 5982 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 347:
#line 1914 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_new_item((yyvsp[-2].v), (yyvsp[0].v)); }
#line 5988 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 349:
#line 1919 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_merge_items((yyvsp[-2].e), (yyvsp[0].e)); }
#line 5994 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 351:
#line 1924 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = f_merge_items((yyvsp[-2].e), (yyvsp[0].e)); }
#line 6000 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 352:
#line 1928 "cf-parse.y" /* yacc.c:1646  */
    {
     if (((yyvsp[0].i) < 0) || ((yyvsp[0].i) > MAX_PREFIX_LENGTH) || !ip_is_prefix((yyvsp[-2].a), (yyvsp[0].i))) cf_error("Invalid network prefix: %I/%d.", (yyvsp[-2].a), (yyvsp[0].i));
     (yyval.v).type = T_PREFIX; (yyval.v).val.px.ip = (yyvsp[-2].a); (yyval.v).val.px.len = (yyvsp[0].i);
   }
#line 6009 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 353:
#line 1935 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v) = (yyvsp[0].v); }
#line 6015 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 354:
#line 1936 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v) = (yyvsp[-1].v); (yyval.v).val.px.len |= LEN_PLUS; }
#line 6021 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 355:
#line 1937 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.v) = (yyvsp[-1].v); (yyval.v).val.px.len |= LEN_MINUS; }
#line 6027 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 356:
#line 1938 "cf-parse.y" /* yacc.c:1646  */
    { 
     if (! ((0 <= (yyvsp[-3].i)) && ((yyvsp[-3].i) <= (yyvsp[-1].i)) && ((yyvsp[-1].i) <= MAX_PREFIX_LENGTH))) cf_error("Invalid prefix pattern range: {%d, %d}.", (yyvsp[-3].i), (yyvsp[-1].i));
     (yyval.v) = (yyvsp[-5].v); (yyval.v).val.px.len |= LEN_RANGE | ((yyvsp[-3].i) << 16) | ((yyvsp[-1].i) << 8);
   }
#line 6036 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 357:
#line 1945 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.trie) = f_new_trie(cfg_mem, sizeof(struct f_trie_node)); trie_add_fprefix((yyval.trie), &((yyvsp[0].v).val.px)); }
#line 6042 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 358:
#line 1946 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.trie) = (yyvsp[-2].trie); trie_add_fprefix((yyval.trie), &((yyvsp[0].v).val.px)); }
#line 6048 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 359:
#line 1949 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.e) = NULL; }
#line 6054 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 360:
#line 1950 "cf-parse.y" /* yacc.c:1646  */
    {
     /* Fill data fields */
     struct f_tree *t;
     for (t = (yyvsp[-2].e); t; t = t->left)
       t->data = (yyvsp[0].x);
     (yyval.e) = f_merge_items((yyvsp[-3].e), (yyvsp[-2].e));
   }
#line 6066 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 361:
#line 1957 "cf-parse.y" /* yacc.c:1646  */
    { 
     struct f_tree *t = f_new_tree();
     t->from.type = t->to.type = T_VOID;
     t->right = t;
     t->data = (yyvsp[0].x);
     (yyval.e) = f_merge_items((yyvsp[-2].e), t);
 }
#line 6078 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 362:
#line 1969 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); }
#line 6084 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 363:
#line 1970 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[-1].x); }
#line 6090 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 364:
#line 1974 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = (yyvsp[-1].h); }
#line 6096 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 365:
#line 1975 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = (yyvsp[-1].h); }
#line 6102 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 366:
#line 1979 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_ASN; (yyval.h)->val = (yyvsp[-1].i); }
#line 6108 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 367:
#line 1980 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_ASN_RANGE; (yyval.h)->val = (yyvsp[-3].i); (yyval.h)->val2 = (yyvsp[-1].i); }
#line 6114 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 368:
#line 1981 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_ASTERISK; }
#line 6120 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 369:
#line 1982 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_QUESTION; }
#line 6126 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 370:
#line 1983 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_ASN_EXPR; (yyval.h)->val = (uintptr_t) (yyvsp[-1].x); }
#line 6132 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 371:
#line 1984 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = NULL; }
#line 6138 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 372:
#line 1988 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_ASN; (yyval.h)->val = (yyvsp[-1].i); }
#line 6144 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 373:
#line 1989 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = cfg_allocz(sizeof(struct f_path_mask)); (yyval.h)->next = (yyvsp[0].h); (yyval.h)->kind = PM_ASTERISK; }
#line 6150 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 374:
#line 1990 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.h) = NULL; }
#line 6156 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 375:
#line 1994 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_INT;  (yyval.x)->a2.i = (yyvsp[0].i); }
#line 6162 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 376:
#line 1995 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 1;  }
#line 6168 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 377:
#line 1996 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_BOOL; (yyval.x)->a2.i = 0;  }
#line 6174 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 378:
#line 1997 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_STRING; (yyval.x)->a2.p = (yyvsp[0].t); }
#line 6180 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 379:
#line 1998 "cf-parse.y" /* yacc.c:1646  */
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[0].v); }
#line 6186 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 380:
#line 1999 "cf-parse.y" /* yacc.c:1646  */
    {NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; (yyval.x)->a1.p = val; *val = (yyvsp[0].v); }
#line 6192 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 381:
#line 2000 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_QUAD;  (yyval.x)->a2.i = (yyvsp[0].i32); }
#line 6198 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 382:
#line 2001 "cf-parse.y" /* yacc.c:1646  */
    { DBG( "We've got a set here..." ); (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_SET; (yyval.x)->a2.p = build_tree((yyvsp[-1].e)); DBG( "ook\n" ); }
#line 6204 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 383:
#line 2002 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = T_PREFIX_SET;  (yyval.x)->a2.p = (yyvsp[-1].trie); }
#line 6210 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 384:
#line 2003 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'c'; (yyval.x)->aux = (yyvsp[0].i) >> 16; (yyval.x)->a2.i = (yyvsp[0].i) & 0xffff; }
#line 6216 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 385:
#line 2004 "cf-parse.y" /* yacc.c:1646  */
    { NEW_F_VAL; (yyval.x) = f_new_inst(); (yyval.x)->code = 'C'; val->type = T_PATH_MASK; val->val.path_mask = (yyvsp[0].h); (yyval.x)->a1.p = val; }
#line 6222 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 386:
#line 2008 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_dpair((yyvsp[-3].x), (yyvsp[-1].x)); }
#line 6228 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 387:
#line 2009 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_ec((yyvsp[-5].i), (yyvsp[-3].x), (yyvsp[-1].x)); }
#line 6234 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 389:
#line 2023 "cf-parse.y" /* yacc.c:1646  */
    {
     struct symbol *sym;
     struct f_inst *inst = (yyvsp[-1].x);
     if ((yyvsp[-3].s)->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", (yyvsp[-3].s)->name);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('c','a');
     (yyval.x)->a1.p = inst;
     (yyval.x)->a2.p = (yyvsp[-3].s)->def;
     sym = (yyvsp[-3].s)->aux2;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", (yyvsp[-3].s)->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = sym->aux2;
       inst = inst->next;
     }
   }
#line 6259 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 390:
#line 2046 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = f_new_inst();

     switch ((yyvsp[0].s)->class & 0xff00) {
       case SYM_CONSTANT: (yyval.x)->code = 'C'; break;
       case SYM_VARIABLE: (yyval.x)->code = 'V'; break;
       default: cf_error("%s: variable expected.", (yyvsp[0].s)->name);
     }

     (yyval.x)->a1.p = (yyvsp[0].s)->def;
     (yyval.x)->a2.p = (yyvsp[0].s)->name;
   }
#line 6276 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 391:
#line 2060 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = SA_FROM;	(yyval.x)->a1.i = 1; }
#line 6282 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 392:
#line 2061 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_IP;         (yyval.x)->a2.i = SA_GW;	(yyval.x)->a1.i = 1; }
#line 6288 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 393:
#line 2062 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_PREFIX;     (yyval.x)->a2.i = SA_NET; }
#line 6294 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 394:
#line 2063 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = SA_PROTO; }
#line 6300 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 395:
#line 2064 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTS;   (yyval.x)->a2.i = SA_SOURCE; }
#line 6306 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 396:
#line 2065 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_SCOPE; (yyval.x)->a2.i = SA_SCOPE;	(yyval.x)->a1.i = 1; }
#line 6312 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 397:
#line 2066 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTC;   (yyval.x)->a2.i = SA_CAST; }
#line 6318 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 398:
#line 2067 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_ENUM_RTD;   (yyval.x)->a2.i = SA_DEST;	(yyval.x)->a1.i = 1; }
#line 6324 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 399:
#line 2068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_STRING;     (yyval.x)->a2.i = SA_IFNAME; }
#line 6330 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 400:
#line 2069 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->aux = T_INT;        (yyval.x)->a2.i = SA_IFINDEX; }
#line 6336 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 401:
#line 2073 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[-1].x); }
#line 6342 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 402:
#line 2074 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '+';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6348 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 403:
#line 2075 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '-';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6354 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 404:
#line 2076 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '*';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6360 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 405:
#line 2077 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '/';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6366 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 406:
#line 2078 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '&';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6372 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 407:
#line 2079 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '|';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6378 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 408:
#line 2080 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('=','='); (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6384 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 409:
#line 2081 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('!','=');     (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6390 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 410:
#line 2082 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6396 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 411:
#line 2083 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6402 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 412:
#line 2084 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '<';        (yyval.x)->a1.p = (yyvsp[0].x); (yyval.x)->a2.p = (yyvsp[-2].x); }
#line 6408 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 413:
#line 2085 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('<','=');     (yyval.x)->a1.p = (yyvsp[0].x); (yyval.x)->a2.p = (yyvsp[-2].x); }
#line 6414 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 414:
#line 2086 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '~';        (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->a2.p = (yyvsp[0].x); }
#line 6420 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 415:
#line 2087 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = '!'; (yyval.x)->a1.p = (yyvsp[0].x); }
#line 6426 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 416:
#line 2088 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('d','e');  (yyval.x)->a1.p = (yyvsp[-1].x); }
#line 6432 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 417:
#line 2090 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); }
#line 6438 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 418:
#line 2091 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); }
#line 6444 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 419:
#line 2092 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); }
#line 6450 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 420:
#line 2094 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'P'; }
#line 6456 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 421:
#line 2096 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); (yyval.x)->code = 'a'; }
#line 6462 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 422:
#line 2098 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); (yyval.x)->code = P('e','a'); }
#line 6468 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 423:
#line 2100 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('c','p'); (yyval.x)->a1.p = (yyvsp[-2].x); (yyval.x)->aux = T_IP; }
#line 6474 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 424:
#line 2101 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'L'; (yyval.x)->a1.p = (yyvsp[-2].x); }
#line 6480 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 425:
#line 2102 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('i','M'); (yyval.x)->a1.p = (yyvsp[-5].x); (yyval.x)->a2.p = (yyvsp[-1].x); }
#line 6486 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 426:
#line 2103 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','f'); (yyval.x)->a1.p = (yyvsp[-2].x); }
#line 6492 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 427:
#line 2104 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','l'); (yyval.x)->a1.p = (yyvsp[-2].x); }
#line 6498 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 428:
#line 2105 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('a','L'); (yyval.x)->a1.p = (yyvsp[-2].x); }
#line 6504 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 429:
#line 2115 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_PATH; }
#line 6510 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 430:
#line 2116 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_CLIST; }
#line 6516 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 431:
#line 2117 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'E'; (yyval.x)->aux = T_ECLIST; }
#line 6522 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 432:
#line 2118 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('A','p'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); }
#line 6528 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 433:
#line 2119 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); (yyval.x)->aux = 'a'; }
#line 6534 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 434:
#line 2120 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); (yyval.x)->aux = 'd'; }
#line 6540 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 435:
#line 2121 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('C','a'); (yyval.x)->a1.p = (yyvsp[-3].x); (yyval.x)->a2.p = (yyvsp[-1].x); (yyval.x)->aux = 'f'; }
#line 6546 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 436:
#line 2123 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_roa_check((yyvsp[-1].s), NULL, NULL); }
#line 6552 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 437:
#line 2124 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_roa_check((yyvsp[-5].s), (yyvsp[-3].x), (yyvsp[-1].x)); }
#line 6558 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 438:
#line 2129 "cf-parse.y" /* yacc.c:1646  */
    {
     struct symbol *sym;
     struct f_inst *inst = (yyvsp[-1].x);
     if ((yyvsp[-3].s)->class != SYM_FUNCTION)
       cf_error("You can't call something which is not a function. Really.");
     DBG("You are calling function %s\n", (yyvsp[-3].s)->name);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('c','a');
     (yyval.x)->a1.p = inst;
     (yyval.x)->a2.p = (yyvsp[-3].s)->def;
     sym = (yyvsp[-3].s)->aux2;
     while (sym || inst) {
       if (!sym || !inst)
	 cf_error("Wrong number of arguments for function %s.", (yyvsp[-3].s)->name);
       DBG( "You should pass parameter called %s\n", sym->name);
       inst->a1.p = sym;
       sym = sym->aux2;
       inst = inst->next;
     }
   }
#line 6583 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 439:
#line 2152 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = F_QUITBIRD; }
#line 6589 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 440:
#line 2153 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = F_ACCEPT; }
#line 6595 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 441:
#line 2154 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = F_REJECT; }
#line 6601 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 442:
#line 2155 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = F_ERROR; }
#line 6607 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 443:
#line 2156 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = F_NOP; }
#line 6613 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 444:
#line 2157 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = F_NONL; }
#line 6619 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 445:
#line 2161 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = 'p'; (yyval.x)->a1.p = (yyvsp[0].x); (yyval.x)->a2.p = NULL; }
#line 6625 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 446:
#line 2164 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = NULL; }
#line 6631 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 447:
#line 2165 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); }
#line 6637 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 448:
#line 2166 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[-2].x)) {
       (yyvsp[-2].x)->next = (yyvsp[0].x);
       (yyval.x) = (yyvsp[-2].x);
     } else (yyval.x) = (yyvsp[0].x);
   }
#line 6648 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 449:
#line 2174 "cf-parse.y" /* yacc.c:1646  */
    { 
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[0].x);
     (yyval.x)->next = NULL;
   }
#line 6660 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 450:
#line 2181 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = NULL;
     (yyval.x)->a2.p = (yyvsp[-2].x);
     (yyval.x)->next = (yyvsp[0].x);
   }
#line 6672 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 451:
#line 2190 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = NULL; }
#line 6678 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 452:
#line 2191 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[0].x); }
#line 6684 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 453:
#line 2195 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = (yyvsp[-2].x);
     (yyval.x)->a2.p = (yyvsp[0].x);
   }
#line 6695 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 454:
#line 2201 "cf-parse.y" /* yacc.c:1646  */
    {
     struct f_inst *i = f_new_inst();
     i->code = '?';
     i->a1.p = (yyvsp[-4].x);
     i->a2.p = (yyvsp[-2].x);
     (yyval.x) = f_new_inst();
     (yyval.x)->code = '?';
     (yyval.x)->a1.p = i;
     (yyval.x)->a2.p = (yyvsp[0].x);
   }
#line 6710 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 455:
#line 2211 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll set value\n" );
     if (((yyvsp[-3].s)->class & ~T_MASK) != SYM_VARIABLE)
       cf_error( "You may set only variables." );
     (yyval.x)->code = 's';
     (yyval.x)->a1.p = (yyvsp[-3].s);
     (yyval.x)->a2.p = (yyvsp[-1].x);
   }
#line 6724 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 456:
#line 2220 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = f_new_inst();
     DBG( "Ook, we'll return the value\n" );
     (yyval.x)->code = 'r';
     (yyval.x)->a1.p = (yyvsp[-1].x);
   }
#line 6735 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 457:
#line 2226 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = (yyvsp[-3].x);
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = (yyvsp[-1].x);
   }
#line 6745 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 458:
#line 2231 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = (yyvsp[-3].x);
     if (!(yyval.x)->a1.i)
       cf_error( "This static attribute is read-only.");
     (yyval.x)->code = P('a','S');
     (yyval.x)->a1.p = (yyvsp[-1].x);
   }
#line 6757 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 459:
#line 2238 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = f_new_inst();
     (yyval.x)->code = P('P','S');
     (yyval.x)->a1.p = (yyvsp[-1].x);
   }
#line 6767 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 460:
#line 2243 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.x) = (yyvsp[-2].x);
     (yyval.x)->aux = EAF_TYPE_UNDEF | EAF_TEMP;
     (yyval.x)->code = P('e','S');
     (yyval.x)->a1.p = NULL;
   }
#line 6778 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 461:
#line 2249 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_inst(); (yyval.x)->code = P('p',','); (yyval.x)->a1.p = (yyvsp[-1].x); (yyval.x)->a2.i = (yyvsp[-2].i); }
#line 6784 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 462:
#line 2250 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = (yyvsp[-1].x); }
#line 6790 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 463:
#line 2251 "cf-parse.y" /* yacc.c:1646  */
    {
      (yyval.x) = f_new_inst();
      (yyval.x)->code = P('S','W');
      (yyval.x)->a1.p = (yyvsp[-3].x);
      (yyval.x)->a2.p = build_tree( (yyvsp[-1].e) );
   }
#line 6801 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 464:
#line 2259 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_empty((yyvsp[-3].x)); }
#line 6807 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 465:
#line 2260 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_complex( P('A','p'), 'x', (yyvsp[-6].x), (yyvsp[-2].x) ); }
#line 6813 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 466:
#line 2261 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_complex( P('C','a'), 'a', (yyvsp[-6].x), (yyvsp[-2].x) ); }
#line 6819 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 467:
#line 2262 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_complex( P('C','a'), 'd', (yyvsp[-6].x), (yyvsp[-2].x) ); }
#line 6825 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 468:
#line 2263 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_generate_complex( P('C','a'), 'f', (yyvsp[-6].x), (yyvsp[-2].x) ); }
#line 6831 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 469:
#line 2270 "cf-parse.y" /* yacc.c:1646  */
    {
  this_proto = proto_config_new(&proto_bfd, (yyvsp[-1].i));
  init_list(&BFD_CFG->patt_list);
  init_list(&BFD_CFG->neigh_list);

  if (bfd_cf)
    cf_error("Only one BFD instance allowed");
  bfd_cf = BFD_CFG;
}
#line 6845 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 477:
#line 2297 "cf-parse.y" /* yacc.c:1646  */
    {
  this_ipatt = cfg_allocz(sizeof(struct bfd_iface_config));
  init_list(&this_ipatt->ipn_list);

  BFD_IFACE->min_rx_int = BFD_DEFAULT_MIN_RX_INT;
  BFD_IFACE->min_tx_int = BFD_DEFAULT_MIN_TX_INT;
  BFD_IFACE->idle_tx_int = BFD_DEFAULT_IDLE_TX_INT;
  BFD_IFACE->multiplier = BFD_DEFAULT_MULTIPLIER;
}
#line 6859 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 478:
#line 2308 "cf-parse.y" /* yacc.c:1646  */
    { BFD_IFACE->min_rx_int = BFD_IFACE->min_tx_int = (yyvsp[0].i32); }
#line 6865 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 479:
#line 2309 "cf-parse.y" /* yacc.c:1646  */
    { BFD_IFACE->min_rx_int = (yyvsp[0].i32); }
#line 6871 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 480:
#line 2310 "cf-parse.y" /* yacc.c:1646  */
    { BFD_IFACE->min_tx_int = (yyvsp[0].i32); }
#line 6877 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 481:
#line 2311 "cf-parse.y" /* yacc.c:1646  */
    { BFD_IFACE->idle_tx_int = (yyvsp[0].i32); }
#line 6883 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 482:
#line 2312 "cf-parse.y" /* yacc.c:1646  */
    { BFD_IFACE->multiplier = (yyvsp[0].i); }
#line 6889 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 483:
#line 2313 "cf-parse.y" /* yacc.c:1646  */
    { BFD_IFACE->passive = (yyvsp[0].i); }
#line 6895 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 488:
#line 2327 "cf-parse.y" /* yacc.c:1646  */
    { add_tail(&BFD_CFG->patt_list, NODE this_ipatt); }
#line 6901 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 489:
#line 2330 "cf-parse.y" /* yacc.c:1646  */
    { BFD_CFG->multihop = BFD_IFACE; }
#line 6907 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 490:
#line 2334 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.iface) = NULL; }
#line 6913 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 491:
#line 2335 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.iface) = if_get_by_name((yyvsp[0].s)->name); }
#line 6919 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 492:
#line 2336 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.iface) = if_get_by_name((yyvsp[0].t)); }
#line 6925 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 493:
#line 2340 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.a) = IPA_NONE; }
#line 6931 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 494:
#line 2341 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.a) = (yyvsp[0].a); }
#line 6937 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 495:
#line 2345 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 6943 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 496:
#line 2346 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = (yyvsp[0].i); }
#line 6949 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 497:
#line 2350 "cf-parse.y" /* yacc.c:1646  */
    {
  this_bfd_neighbor = cfg_allocz(sizeof(struct bfd_neighbor));
  add_tail(&BFD_CFG->neigh_list, NODE this_bfd_neighbor);

  BFD_NEIGHBOR->addr = (yyvsp[-3].a);
  BFD_NEIGHBOR->local = (yyvsp[-1].a);
  BFD_NEIGHBOR->iface = (yyvsp[-2].iface);
  BFD_NEIGHBOR->multihop = (yyvsp[0].i);

  if ((yyvsp[0].i) && (yyvsp[-2].iface))
    cf_error("Neighbor cannot set both interface and multihop");

  if ((yyvsp[0].i) && ipa_zero((yyvsp[-1].a)))
    cf_error("Multihop struct neighbor requires specified local address");
}
#line 6969 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 498:
#line 2369 "cf-parse.y" /* yacc.c:1646  */
    { bfd_show_sessions(proto_get_named((yyvsp[-1].s), &proto_bfd)); }
#line 6975 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 499:
#line 2374 "cf-parse.y" /* yacc.c:1646  */
    {
     this_proto = proto_config_new(&proto_bgp, (yyvsp[-1].i));
     BGP_CFG->remote_port = BGP_PORT;
     BGP_CFG->multihop = -1;	/* undefined */
     BGP_CFG->hold_time = 240;
     BGP_CFG->initial_hold_time = 240;
     BGP_CFG->compare_path_lengths = 1;
     BGP_CFG->igp_metric = 1;
     BGP_CFG->connect_delay_time = 5;
     BGP_CFG->connect_retry_time = 120;
     BGP_CFG->error_amnesia_time = 300;
     BGP_CFG->error_delay_time_min = 60;
     BGP_CFG->error_delay_time_max = 300;
     BGP_CFG->enable_refresh = 1;
     BGP_CFG->enable_as4 = 1;
     BGP_CFG->capabilities = 2;
     BGP_CFG->advertise_ipv4 = 1;
     BGP_CFG->interpret_communities = 1;
     BGP_CFG->default_local_pref = 100;
     BGP_CFG->gr_mode = BGP_GR_AWARE;
     BGP_CFG->gr_time = 120;
     BGP_CFG->setkey = 1;
 }
#line 7003 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 501:
#line 2401 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->remote_port = (yyvsp[0].i); if (((yyvsp[0].i)<1) || ((yyvsp[0].i)>65535)) cf_error("Invalid port number");  }
#line 7009 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 502:
#line 2402 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->remote_as = (yyvsp[0].i); }
#line 7015 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 505:
#line 2408 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->local_as = (yyvsp[-1].i); }
#line 7021 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 506:
#line 2409 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->source_addr = (yyvsp[-3].a); BGP_CFG->local_as = (yyvsp[-1].i); }
#line 7027 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 508:
#line 2411 "cf-parse.y" /* yacc.c:1646  */
    {
     if (ipa_nonzero(BGP_CFG->remote_ip))
       cf_error("Only one struct neighbor per BGP instance is allowed");
     BGP_CFG->remote_ip = (yyvsp[-3].a);
     if ((yyvsp[-2].iface)) BGP_CFG->iface = (yyvsp[-2].iface);
   }
#line 7038 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 509:
#line 2417 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->iface = if_get_by_name((yyvsp[-1].t)); }
#line 7044 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 510:
#line 2418 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->rr_cluster_id = (yyvsp[-1].i32); }
#line 7050 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 511:
#line 2419 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->rr_client = 1; }
#line 7056 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 512:
#line 2420 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->rs_client = 1; }
#line 7062 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 513:
#line 2421 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->hold_time = (yyvsp[-1].i); }
#line 7068 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 514:
#line 2422 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->initial_hold_time = (yyvsp[-1].i); }
#line 7074 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 515:
#line 2423 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->multihop = 0; }
#line 7080 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 516:
#line 2424 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->multihop = 64; }
#line 7086 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 517:
#line 2425 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->multihop = (yyvsp[-1].i); if (((yyvsp[-1].i)<1) || ((yyvsp[-1].i)>255)) cf_error("Multihop must be in range 1-255"); }
#line 7092 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 518:
#line 2426 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->next_hop_self = 1; BGP_CFG->next_hop_keep = 0; }
#line 7098 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 519:
#line 2427 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->next_hop_keep = 1; BGP_CFG->next_hop_self = 0; }
#line 7104 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 520:
#line 2428 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->missing_lladdr = MLL_SELF; }
#line 7110 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 521:
#line 2429 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->missing_lladdr = MLL_DROP; }
#line 7116 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 522:
#line 2430 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->missing_lladdr = MLL_IGNORE; }
#line 7122 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 523:
#line 2431 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->gw_mode = GW_DIRECT; }
#line 7128 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 524:
#line 2432 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->gw_mode = GW_RECURSIVE; }
#line 7134 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 525:
#line 2433 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->compare_path_lengths = (yyvsp[-1].i); }
#line 7140 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 526:
#line 2434 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->med_metric = (yyvsp[-1].i); }
#line 7146 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 527:
#line 2435 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->igp_metric = (yyvsp[-1].i); }
#line 7152 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 528:
#line 2436 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->prefer_older = (yyvsp[-1].i); }
#line 7158 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 529:
#line 2437 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->deterministic_med = (yyvsp[-1].i); }
#line 7164 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 530:
#line 2438 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->default_med = (yyvsp[-1].i); }
#line 7170 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 531:
#line 2439 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->default_local_pref = (yyvsp[-1].i); }
#line 7176 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 532:
#line 2440 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->source_addr = (yyvsp[-1].a); }
#line 7182 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 533:
#line 2441 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->connect_delay_time = (yyvsp[-1].i); log(L_WARN "%s: Start delay time option is deprecated, use connect delay time", this_proto->name); }
#line 7188 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 534:
#line 2442 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->connect_delay_time = (yyvsp[-1].i); }
#line 7194 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 535:
#line 2443 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->connect_retry_time = (yyvsp[-1].i); }
#line 7200 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 536:
#line 2444 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->keepalive_time = (yyvsp[-1].i); }
#line 7206 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 537:
#line 2445 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->error_amnesia_time = (yyvsp[-1].i); }
#line 7212 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 538:
#line 2446 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->error_delay_time_min = (yyvsp[-3].i); BGP_CFG->error_delay_time_max = (yyvsp[-1].i); }
#line 7218 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 539:
#line 2447 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->disable_after_error = (yyvsp[-1].i); }
#line 7224 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 540:
#line 2448 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->enable_refresh = (yyvsp[-1].i); }
#line 7230 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 541:
#line 2449 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->enable_as4 = (yyvsp[-1].i); }
#line 7236 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 542:
#line 2450 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->enable_extended_messages = (yyvsp[-1].i); }
#line 7242 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 543:
#line 2451 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->capabilities = (yyvsp[-1].i); }
#line 7248 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 544:
#line 2452 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->advertise_ipv4 = (yyvsp[-1].i); }
#line 7254 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 545:
#line 2453 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->password = (yyvsp[-1].t); }
#line 7260 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 546:
#line 2454 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->setkey = (yyvsp[-1].i); }
#line 7266 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 547:
#line 2455 "cf-parse.y" /* yacc.c:1646  */
    {
     this_proto->in_limit = cfg_allocz(sizeof(struct proto_limit));
     this_proto->in_limit->limit = (yyvsp[-1].i);
     this_proto->in_limit->action = PLA_RESTART;
     log(L_WARN "%s: Route limit option is deprecated, use import limit", this_proto->name);
   }
#line 7277 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 548:
#line 2461 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->passive = (yyvsp[-1].i); }
#line 7283 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 549:
#line 2462 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->interpret_communities = (yyvsp[-1].i); }
#line 7289 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 550:
#line 2463 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->secondary = (yyvsp[-1].i); }
#line 7295 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 551:
#line 2464 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->add_path = ADD_PATH_RX; }
#line 7301 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 552:
#line 2465 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->add_path = ADD_PATH_TX; }
#line 7307 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 553:
#line 2466 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->add_path = (yyvsp[-1].i) ? ADD_PATH_FULL : 0; }
#line 7313 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 554:
#line 2467 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->allow_local_as = -1; }
#line 7319 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 555:
#line 2468 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->allow_local_as = (yyvsp[-1].i); }
#line 7325 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 556:
#line 2469 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->gr_mode = (yyvsp[-1].i); }
#line 7331 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 557:
#line 2470 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->gr_mode = BGP_GR_AWARE; }
#line 7337 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 558:
#line 2471 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->gr_time = (yyvsp[-1].i); }
#line 7343 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 559:
#line 2472 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->igp_table = (yyvsp[-1].r); }
#line 7349 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 560:
#line 2473 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->ttl_security = (yyvsp[-1].i); }
#line 7355 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 561:
#line 2474 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->check_link = (yyvsp[-1].i); }
#line 7361 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 562:
#line 2475 "cf-parse.y" /* yacc.c:1646  */
    { BGP_CFG->bfd = (yyvsp[-1].i); cf_check_bfd((yyvsp[-1].i)); }
#line 7367 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 563:
#line 2486 "cf-parse.y" /* yacc.c:1646  */
    {
     this_proto = proto_config_new(&proto_ospf, (yyvsp[-1].i));
     init_list(&OSPF_CFG->area_list);
     init_list(&OSPF_CFG->vlink_list);
     OSPF_CFG->tick = OSPF_DEFAULT_TICK;
     OSPF_CFG->ospf2 = OSPF_IS_V2;
  }
#line 7379 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 567:
#line 2502 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->rfc1583 = (yyvsp[0].i); }
#line 7385 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 568:
#line 2503 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->stub_router = (yyvsp[0].i); }
#line 7391 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 569:
#line 2504 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->ecmp = (yyvsp[0].i) ? OSPF_DEFAULT_ECMP_LIMIT : 0; }
#line 7397 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 570:
#line 2505 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->ecmp = (yyvsp[-2].i) ? (yyvsp[0].i) : 0; if ((yyvsp[0].i) < 0) cf_error("ECMP limit cannot be negative"); }
#line 7403 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 571:
#line 2506 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->merge_external = (yyvsp[0].i); }
#line 7409 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 572:
#line 2507 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->tick = (yyvsp[0].i); if((yyvsp[0].i)<=0) cf_error("Tick must be greater than zero"); }
#line 7415 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 573:
#line 2508 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_CFG->instance_id = (yyvsp[0].i); if (((yyvsp[0].i)<0) || ((yyvsp[0].i)>255)) cf_error("Instance ID must be in range 0-255"); }
#line 7421 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 575:
#line 2512 "cf-parse.y" /* yacc.c:1646  */
    {
  this_area = cfg_allocz(sizeof(struct ospf_area_config));
  add_tail(&OSPF_CFG->area_list, NODE this_area);
  this_area->areaid = (yyvsp[0].i32);
  this_area->default_cost = OSPF_DEFAULT_STUB_COST;
  this_area->type = OPT_E;
  this_area->transint = OSPF_DEFAULT_TRANSINT;

  init_list(&this_area->patt_list);
  init_list(&this_area->net_list);
  init_list(&this_area->enet_list);
  init_list(&this_area->stubnet_list);
 }
#line 7439 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 576:
#line 2527 "cf-parse.y" /* yacc.c:1646  */
    { ospf_area_finish(); }
#line 7445 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 579:
#line 2536 "cf-parse.y" /* yacc.c:1646  */
    { this_area->type = (yyvsp[0].i) ? 0 : OPT_E; /* We should remove the option */ }
#line 7451 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 580:
#line 2537 "cf-parse.y" /* yacc.c:1646  */
    { this_area->type = OPT_N; }
#line 7457 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 581:
#line 2538 "cf-parse.y" /* yacc.c:1646  */
    { this_area->summary = (yyvsp[0].i); }
#line 7463 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 582:
#line 2539 "cf-parse.y" /* yacc.c:1646  */
    { this_area->default_nssa = (yyvsp[0].i); }
#line 7469 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 583:
#line 2540 "cf-parse.y" /* yacc.c:1646  */
    { this_area->default_cost = (yyvsp[0].i); ospf_check_defcost((yyvsp[0].i)); }
#line 7475 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 584:
#line 2541 "cf-parse.y" /* yacc.c:1646  */
    { this_area->default_cost = (yyvsp[0].i) | LSA_EXT3_EBIT; ospf_check_defcost((yyvsp[0].i)); }
#line 7481 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 585:
#line 2542 "cf-parse.y" /* yacc.c:1646  */
    { this_area->default_cost = (yyvsp[0].i); ospf_check_defcost((yyvsp[0].i)); }
#line 7487 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 586:
#line 2543 "cf-parse.y" /* yacc.c:1646  */
    { this_area->translator = (yyvsp[0].i); }
#line 7493 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 587:
#line 2544 "cf-parse.y" /* yacc.c:1646  */
    { this_area->transint = (yyvsp[0].i); }
#line 7499 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 588:
#line 2545 "cf-parse.y" /* yacc.c:1646  */
    { this_nets = &this_area->net_list; }
#line 7505 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 590:
#line 2546 "cf-parse.y" /* yacc.c:1646  */
    { this_nets = &this_area->enet_list; }
#line 7511 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 597:
#line 2558 "cf-parse.y" /* yacc.c:1646  */
    {
     this_stubnet = cfg_allocz(sizeof(struct ospf_stubnet_config));
     add_tail(&this_area->stubnet_list, NODE this_stubnet);
     this_stubnet->px = (yyvsp[0].px);
     this_stubnet->cost = COST_D;
   }
#line 7522 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 600:
#line 2572 "cf-parse.y" /* yacc.c:1646  */
    { this_stubnet->hidden = (yyvsp[0].i); }
#line 7528 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 601:
#line 2573 "cf-parse.y" /* yacc.c:1646  */
    { this_stubnet->summary = (yyvsp[0].i); }
#line 7534 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 602:
#line 2574 "cf-parse.y" /* yacc.c:1646  */
    { this_stubnet->cost = (yyvsp[0].i); }
#line 7540 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 603:
#line 2578 "cf-parse.y" /* yacc.c:1646  */
    { ospf_iface_finish(); }
#line 7546 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 604:
#line 2579 "cf-parse.y" /* yacc.c:1646  */
    { ospf_iface_finish(); }
#line 7552 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 608:
#line 2588 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->helloint = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Hello interval must be in range 1-65535"); }
#line 7558 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 609:
#line 2589 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->rxmtint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Retransmit int must be greater than one"); }
#line 7564 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 610:
#line 2590 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->inftransdelay = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); }
#line 7570 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 611:
#line 2591 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->waitint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Wait interval must be greater than one"); }
#line 7576 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 612:
#line 2592 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->deadint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead interval must be greater than one"); }
#line 7582 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 613:
#line 2593 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->deadc = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead count must be greater than one"); }
#line 7588 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 614:
#line 2594 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->autype = OSPF_AUTH_NONE;  }
#line 7594 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 615:
#line 2595 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE; ospf_check_auth(); }
#line 7600 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 616:
#line 2596 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT; ospf_check_auth(); }
#line 7606 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 617:
#line 2597 "cf-parse.y" /* yacc.c:1646  */
    { ospf_check_auth(); }
#line 7612 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 618:
#line 2601 "cf-parse.y" /* yacc.c:1646  */
    {
  if (this_area->areaid == 0) cf_error("Virtual link cannot be in backbone");
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&OSPF_CFG->vlink_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->voa = this_area->areaid;
  OSPF_PATT->vid = (yyvsp[0].i32);
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->type = OSPF_IT_VLINK;
  OSPF_PATT->instance_id = OSPF_CFG->instance_id;
  init_list(&OSPF_PATT->nbma_list);
  reset_passwords();
 }
#line 7633 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 619:
#line 2620 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->cost = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Cost must be in range 1-65535"); }
#line 7639 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 620:
#line 2621 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->helloint = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Hello interval must be in range 1-65535"); }
#line 7645 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 621:
#line 2622 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->pollint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=0) cf_error("Poll int must be greater than zero"); }
#line 7651 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 622:
#line 2623 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->rxmtint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Retransmit int must be greater than one"); }
#line 7657 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 623:
#line 2624 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->waitint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Wait interval must be greater than one"); }
#line 7663 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 624:
#line 2625 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->deadint = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead interval must be greater than one"); }
#line 7669 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 625:
#line 2626 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->deadc = (yyvsp[0].i) ; if ((yyvsp[0].i)<=1) cf_error("Dead count must be greater than one"); }
#line 7675 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 626:
#line 2627 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_BCAST ; }
#line 7681 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 627:
#line 2628 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_BCAST ; }
#line 7687 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 628:
#line 2629 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_NBMA ; }
#line 7693 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 629:
#line 2630 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_NBMA ; }
#line 7699 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 630:
#line 2631 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_PTP ; }
#line 7705 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 631:
#line 2632 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_PTP ; }
#line 7711 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 632:
#line 2633 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_PTMP ; }
#line 7717 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 633:
#line 2634 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->type = OSPF_IT_PTMP ; }
#line 7723 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 634:
#line 2635 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->real_bcast = (yyvsp[0].i); if (!ospf_cfg_is_v2()) cf_error("Real broadcast option requires OSPFv2"); }
#line 7729 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 635:
#line 2636 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->ptp_netmask = (yyvsp[0].i); if (!ospf_cfg_is_v2()) cf_error("PtP netmask option requires OSPFv2"); }
#line 7735 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 636:
#line 2637 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->inftransdelay = (yyvsp[0].i) ; if (((yyvsp[0].i)<=0) || ((yyvsp[0].i)>65535)) cf_error("Transmit delay must be in range 1-65535"); }
#line 7741 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 637:
#line 2638 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->priority = (yyvsp[0].i) ; if (((yyvsp[0].i)<0) || ((yyvsp[0].i)>255)) cf_error("Priority must be in range 0-255"); }
#line 7747 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 638:
#line 2639 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->strictnbma = (yyvsp[0].i) ; }
#line 7753 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 639:
#line 2640 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->stub = (yyvsp[0].i) ; }
#line 7759 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 640:
#line 2641 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->check_link = (yyvsp[0].i); }
#line 7765 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 641:
#line 2642 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->ecmp_weight = (yyvsp[0].i) - 1; if (((yyvsp[0].i)<1) || ((yyvsp[0].i)>256)) cf_error("ECMP weight must be in range 1-256"); }
#line 7771 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 642:
#line 2643 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->link_lsa_suppression = (yyvsp[0].i); if (!ospf_cfg_is_v3()) cf_error("Link LSA suppression option requires OSPFv3"); }
#line 7777 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 644:
#line 2645 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->autype = OSPF_AUTH_NONE; }
#line 7783 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 645:
#line 2646 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->autype = OSPF_AUTH_SIMPLE; ospf_check_auth(); }
#line 7789 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 646:
#line 2647 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->autype = OSPF_AUTH_CRYPT; ospf_check_auth(); }
#line 7795 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 647:
#line 2648 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->rx_buffer = 0; }
#line 7801 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 648:
#line 2649 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->rx_buffer = OSPF_MAX_PKT_SIZE; }
#line 7807 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 649:
#line 2650 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->rx_buffer = (yyvsp[0].i); if (((yyvsp[0].i) < OSPF_MIN_PKT_SIZE) || ((yyvsp[0].i) > OSPF_MAX_PKT_SIZE)) cf_error("Buffer size must be in range 256-65535"); }
#line 7813 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 650:
#line 2651 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->tx_tos = (yyvsp[0].i); }
#line 7819 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 651:
#line 2652 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->tx_priority = (yyvsp[0].i); }
#line 7825 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 652:
#line 2653 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->tx_length = (yyvsp[0].i); if (((yyvsp[0].i) < OSPF_MIN_PKT_SIZE) || ((yyvsp[0].i) > OSPF_MAX_PKT_SIZE)) cf_error("TX length must be in range 256-65535"); }
#line 7831 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 653:
#line 2654 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->ttl_security = (yyvsp[0].i); }
#line 7837 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 654:
#line 2655 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->ttl_security = 2; }
#line 7843 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 655:
#line 2656 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->bfd = (yyvsp[0].i); cf_check_bfd((yyvsp[0].i)); }
#line 7849 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 656:
#line 2657 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->bsd_secondary = (yyvsp[0].i); }
#line 7855 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 657:
#line 2658 "cf-parse.y" /* yacc.c:1646  */
    { ospf_check_auth(); }
#line 7861 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 661:
#line 2669 "cf-parse.y" /* yacc.c:1646  */
    {
   this_pref = cfg_allocz(sizeof(struct area_net_config));
   add_tail(this_nets, NODE this_pref);
   this_pref->px.addr = (yyvsp[0].px).addr;
   this_pref->px.len = (yyvsp[0].px).len;
 }
#line 7872 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 663:
#line 2679 "cf-parse.y" /* yacc.c:1646  */
    { this_pref->hidden = 1; }
#line 7878 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 664:
#line 2680 "cf-parse.y" /* yacc.c:1646  */
    { this_pref->tag = (yyvsp[0].i); }
#line 7884 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 667:
#line 2689 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 0; }
#line 7890 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 668:
#line 2690 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = 1; }
#line 7896 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 669:
#line 2694 "cf-parse.y" /* yacc.c:1646  */
    {
   this_nbma = cfg_allocz(sizeof(struct nbma_node));
   add_tail(&OSPF_PATT->nbma_list, NODE this_nbma);
   this_nbma->ip=(yyvsp[-2].a);
   this_nbma->eligible=(yyvsp[-1].i);
 }
#line 7907 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 670:
#line 2703 "cf-parse.y" /* yacc.c:1646  */
    {
  this_ipatt = cfg_allocz(sizeof(struct ospf_iface_patt));
  add_tail(&this_area->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  OSPF_PATT->cost = COST_D;
  OSPF_PATT->helloint = HELLOINT_D;
  OSPF_PATT->pollint = POLLINT_D;
  OSPF_PATT->rxmtint = RXMTINT_D;
  OSPF_PATT->inftransdelay = INFTRANSDELAY_D;
  OSPF_PATT->priority = PRIORITY_D;
  OSPF_PATT->deadc = DEADC_D;
  OSPF_PATT->type = OSPF_IT_UNDEF;
  OSPF_PATT->instance_id = OSPF_CFG->instance_id;
  init_list(&OSPF_PATT->nbma_list);
  OSPF_PATT->ptp_netmask = 2; /* not specified */
  OSPF_PATT->tx_tos = IP_PREC_INTERNET_CONTROL;
  OSPF_PATT->tx_priority = sk_priority_control;
  reset_passwords();
 }
#line 7931 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 672:
#line 2726 "cf-parse.y" /* yacc.c:1646  */
    { OSPF_PATT->instance_id = (yyvsp[0].i); if (((yyvsp[0].i)<0) || ((yyvsp[0].i)>255)) cf_error("Instance ID must be in range 0-255"); }
#line 7937 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 673:
#line 2730 "cf-parse.y" /* yacc.c:1646  */
    { if (ospf_cfg_is_v3()) iface_patt_check(); }
#line 7943 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 679:
#line 2744 "cf-parse.y" /* yacc.c:1646  */
    { ospf_iface_finish(); }
#line 7949 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 681:
#line 2749 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.t) = NULL; }
#line 7955 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 682:
#line 2755 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh(proto_get_named((yyvsp[-1].s), &proto_ospf)); }
#line 7961 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 683:
#line 2758 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_neigh(proto_get_named((yyvsp[-2].s), &proto_ospf), (yyvsp[-1].t)); }
#line 7967 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 684:
#line 2761 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_iface(proto_get_named((yyvsp[-2].s), &proto_ospf), (yyvsp[-1].t)); }
#line 7973 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 685:
#line 2766 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_state(proto_get_named((yyvsp[-2].s), &proto_ospf), 0, 1); }
#line 7979 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 686:
#line 2769 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_state(proto_get_named((yyvsp[-2].s), &proto_ospf), 0, 0); }
#line 7985 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 687:
#line 2774 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_state(proto_get_named((yyvsp[-2].s), &proto_ospf), 1, 1); }
#line 7991 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 688:
#line 2777 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_state(proto_get_named((yyvsp[-2].s), &proto_ospf), 1, 0); }
#line 7997 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 689:
#line 2781 "cf-parse.y" /* yacc.c:1646  */
    { ospf_sh_lsadb((yyvsp[-1].ld)); }
#line 8003 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 690:
#line 2784 "cf-parse.y" /* yacc.c:1646  */
    {
     (yyval.ld) = cfg_allocz(sizeof(struct lsadb_show_data));
   }
#line 8011 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 691:
#line 2787 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-1].ld); (yyval.ld)->scope = LSA_SCOPE_AS; }
#line 8017 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 692:
#line 2788 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-2].ld); (yyval.ld)->scope = LSA_SCOPE_AREA; (yyval.ld)->area = (yyvsp[0].i32); }
#line 8023 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 693:
#line 2789 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-1].ld); (yyval.ld)->scope = 1; /* hack, 0 is no filter */ }
#line 8029 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 694:
#line 2790 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-2].ld); (yyval.ld)->type = (yyvsp[0].i); }
#line 8035 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 695:
#line 2791 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-2].ld); (yyval.ld)->lsid = (yyvsp[0].i32); }
#line 8041 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 696:
#line 2792 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-1].ld); (yyval.ld)->router = SH_ROUTER_SELF; }
#line 8047 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 697:
#line 2793 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-2].ld); (yyval.ld)->router = (yyvsp[0].i32); }
#line 8053 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 698:
#line 2794 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.ld) = (yyvsp[-1].ld); (yyval.ld)->name = (yyvsp[0].s); }
#line 8059 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 699:
#line 2800 "cf-parse.y" /* yacc.c:1646  */
    {
     this_proto = proto_config_new(&proto_pipe, (yyvsp[-1].i));
     PIPE_CFG->mode = PIPE_TRANSPARENT;
  }
#line 8068 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 702:
#line 2809 "cf-parse.y" /* yacc.c:1646  */
    {
     if ((yyvsp[-1].s)->class != SYM_TABLE)
       cf_error("Routing table name expected");
     PIPE_CFG->peer = (yyvsp[-1].s)->def;
   }
#line 8078 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 703:
#line 2814 "cf-parse.y" /* yacc.c:1646  */
    { PIPE_CFG->mode = PIPE_OPAQUE; }
#line 8084 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 704:
#line 2815 "cf-parse.y" /* yacc.c:1646  */
    { PIPE_CFG->mode = PIPE_TRANSPARENT; }
#line 8090 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 705:
#line 2822 "cf-parse.y" /* yacc.c:1646  */
    {
  this_proto = proto_config_new(&proto_rip, (yyvsp[-1].i));
  init_list(&RIP_CFG->patt_list);

  RIP_CFG->rip2 = RIP_IS_V2;
  RIP_CFG->infinity = RIP_DEFAULT_INFINITY;

  RIP_CFG->min_timeout_time = 60;
  RIP_CFG->max_garbage_time = 60;
}
#line 8105 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 707:
#line 2835 "cf-parse.y" /* yacc.c:1646  */
    { RIP_CFG->ecmp = (yyvsp[0].i) ? RIP_DEFAULT_ECMP_LIMIT : 0; }
#line 8111 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 708:
#line 2836 "cf-parse.y" /* yacc.c:1646  */
    { RIP_CFG->ecmp = (yyvsp[-2].i) ? (yyvsp[0].i) : 0; if ((yyvsp[0].i) < 0) cf_error("ECMP limit cannot be negative"); }
#line 8117 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 709:
#line 2837 "cf-parse.y" /* yacc.c:1646  */
    { RIP_CFG->infinity = (yyvsp[0].i); }
#line 8123 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 714:
#line 2851 "cf-parse.y" /* yacc.c:1646  */
    {
  this_ipatt = cfg_allocz(sizeof(struct rip_iface_config));
  add_tail(&RIP_CFG->patt_list, NODE this_ipatt);
  init_list(&this_ipatt->ipn_list);
  reset_passwords();

  RIP_IFACE->metric = 1;
  RIP_IFACE->port = rip_cfg_is_v2() ? RIP_PORT : RIP_NG_PORT;
  RIP_IFACE->version = rip_cfg_is_v2() ? RIP_V2 : RIP_V1;
  RIP_IFACE->split_horizon = 1;
  RIP_IFACE->poison_reverse = 1;
  RIP_IFACE->check_zero = 1;
  RIP_IFACE->ttl_security = rip_cfg_is_v2() ? 0 : 1;
  RIP_IFACE->rx_buffer = rip_cfg_is_v2() ? RIP_MAX_PKT_LENGTH : 0;
  RIP_IFACE->tx_length = rip_cfg_is_v2() ? RIP_MAX_PKT_LENGTH : 0;
  RIP_IFACE->tx_tos = IP_PREC_INTERNET_CONTROL;
  RIP_IFACE->tx_priority = sk_priority_control;
  RIP_IFACE->update_time = RIP_DEFAULT_UPDATE_TIME;
  RIP_IFACE->timeout_time = RIP_DEFAULT_TIMEOUT_TIME;
  RIP_IFACE->garbage_time = RIP_DEFAULT_GARBAGE_TIME;
}
#line 8149 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 715:
#line 2874 "cf-parse.y" /* yacc.c:1646  */
    {
  RIP_IFACE->passwords = get_passwords();

  if (!RIP_IFACE->auth_type != !RIP_IFACE->passwords)
    log(L_WARN "Authentication and password options should be used together");

  /* Default mode is broadcast for RIPv1, multicast for RIPv2 and RIPng */
  if (!RIP_IFACE->mode)
    RIP_IFACE->mode = (rip_cfg_is_v2() && (RIP_IFACE->version == RIP_V1)) ?
      RIP_IM_BROADCAST : RIP_IM_MULTICAST;

  RIP_CFG->min_timeout_time = MIN_(RIP_CFG->min_timeout_time, RIP_IFACE->timeout_time);
  RIP_CFG->max_garbage_time = MAX_(RIP_CFG->max_garbage_time, RIP_IFACE->garbage_time);
}
#line 8168 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 716:
#line 2890 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->metric = (yyvsp[0].i); if (((yyvsp[0].i)<1) || ((yyvsp[0].i)>255)) cf_error("Metric must be in range 1-255"); }
#line 8174 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 717:
#line 2891 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->mode = RIP_IM_MULTICAST; }
#line 8180 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 718:
#line 2892 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->mode = RIP_IM_BROADCAST; if (rip_cfg_is_ng()) cf_error("Broadcast not supported in RIPng"); }
#line 8186 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 719:
#line 2893 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->passive = (yyvsp[0].i); }
#line 8192 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 720:
#line 2894 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->address = (yyvsp[0].a); }
#line 8198 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 721:
#line 2895 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->port = (yyvsp[0].i); if (((yyvsp[0].i)<1) || ((yyvsp[0].i)>65535)) cf_error("Invalid port number"); }
#line 8204 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 722:
#line 2896 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->version = (yyvsp[0].i);
			  if (rip_cfg_is_ng()) cf_error("Version not supported in RIPng");
			  if (((yyvsp[0].i) != RIP_V1) && ((yyvsp[0].i) != RIP_V2)) cf_error("Unsupported version");
			}
#line 8213 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 723:
#line 2900 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->version_only = (yyvsp[0].i); }
#line 8219 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 724:
#line 2901 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->split_horizon = (yyvsp[0].i); }
#line 8225 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 725:
#line 2902 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->poison_reverse = (yyvsp[0].i); }
#line 8231 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 726:
#line 2903 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->check_zero = (yyvsp[0].i); }
#line 8237 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 727:
#line 2904 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->update_time = (yyvsp[0].i); if ((yyvsp[0].i)<=0) cf_error("Update time must be positive"); }
#line 8243 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 728:
#line 2905 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->timeout_time = (yyvsp[0].i); if ((yyvsp[0].i)<=0) cf_error("Timeout time must be positive"); }
#line 8249 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 729:
#line 2906 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->garbage_time = (yyvsp[0].i); if ((yyvsp[0].i)<=0) cf_error("Garbage time must be positive"); }
#line 8255 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 730:
#line 2907 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->ecmp_weight = (yyvsp[0].i) - 1; if (((yyvsp[0].i)<1) || ((yyvsp[0].i)>256)) cf_error("ECMP weight must be in range 1-256"); }
#line 8261 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 731:
#line 2908 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->rx_buffer = (yyvsp[0].i); if (((yyvsp[0].i)<256) || ((yyvsp[0].i)>65535)) cf_error("RX length must be in range 256-65535"); }
#line 8267 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 732:
#line 2909 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->tx_length = (yyvsp[0].i); if (((yyvsp[0].i)<256) || ((yyvsp[0].i)>65535)) cf_error("TX length must be in range 256-65535"); }
#line 8273 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 733:
#line 2910 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->tx_tos = (yyvsp[0].i); }
#line 8279 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 734:
#line 2911 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->tx_priority = (yyvsp[0].i); }
#line 8285 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 735:
#line 2912 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->ttl_security = (yyvsp[0].i); }
#line 8291 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 736:
#line 2913 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->ttl_security = 2; }
#line 8297 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 737:
#line 2914 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->check_link = (yyvsp[0].i); }
#line 8303 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 738:
#line 2915 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->bfd = (yyvsp[0].i); cf_check_bfd((yyvsp[0].i)); }
#line 8309 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 739:
#line 2916 "cf-parse.y" /* yacc.c:1646  */
    { RIP_IFACE->auth_type = (yyvsp[0].i); if ((yyvsp[0].i)) rip_check_auth(); }
#line 8315 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 740:
#line 2917 "cf-parse.y" /* yacc.c:1646  */
    { rip_check_auth(); }
#line 8321 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 741:
#line 2921 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RIP_AUTH_NONE; }
#line 8327 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 742:
#line 2922 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RIP_AUTH_PLAIN; }
#line 8333 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 743:
#line 2923 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RIP_AUTH_CRYPTO; }
#line 8339 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 744:
#line 2924 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.i) = RIP_AUTH_CRYPTO; }
#line 8345 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 750:
#line 2945 "cf-parse.y" /* yacc.c:1646  */
    { rip_show_interfaces(proto_get_named((yyvsp[-2].s), &proto_rip), (yyvsp[-1].t)); }
#line 8351 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 751:
#line 2948 "cf-parse.y" /* yacc.c:1646  */
    { rip_show_neighbors(proto_get_named((yyvsp[-2].s), &proto_rip), (yyvsp[-1].t)); }
#line 8357 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 752:
#line 2954 "cf-parse.y" /* yacc.c:1646  */
    {
     this_proto = proto_config_new(&proto_static, (yyvsp[-1].i));
     static_init_config((struct static_config *) this_proto);
  }
#line 8366 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 755:
#line 2963 "cf-parse.y" /* yacc.c:1646  */
    { STATIC_CFG->check_link = (yyvsp[-1].i); }
#line 8372 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 756:
#line 2964 "cf-parse.y" /* yacc.c:1646  */
    { STATIC_CFG->igp_table = (yyvsp[-1].r); }
#line 8378 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 757:
#line 2965 "cf-parse.y" /* yacc.c:1646  */
    { static_route_finish(); }
#line 8384 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 758:
#line 2968 "cf-parse.y" /* yacc.c:1646  */
    {
     this_srt = cfg_allocz(sizeof(struct static_route));
     add_tail(&STATIC_CFG->other_routes, &this_srt->n);
     this_srt->net = (yyvsp[0].px).addr;
     this_srt->masklen = (yyvsp[0].px).len;
     this_srt_last_cmd = &(this_srt->cmds);
  }
#line 8396 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 759:
#line 2978 "cf-parse.y" /* yacc.c:1646  */
    {
     last_srt_nh = this_srt_nh;
     this_srt_nh = cfg_allocz(sizeof(struct static_route));
     this_srt_nh->dest = RTD_NONE;
     this_srt_nh->via = (yyvsp[-1].a);
     this_srt_nh->via_if = (yyvsp[0].iface);
     this_srt_nh->if_name = (void *) this_srt; /* really */
     this_srt_nh->use_bfd = -1; /* undefined */
   }
#line 8410 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 760:
#line 2987 "cf-parse.y" /* yacc.c:1646  */
    {
     this_srt_nh->masklen = (yyvsp[0].i) - 1; /* really */
     if (((yyvsp[0].i)<1) || ((yyvsp[0].i)>256)) cf_error("Weight must be in range 1-256"); 
   }
#line 8419 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 761:
#line 2991 "cf-parse.y" /* yacc.c:1646  */
    {
     this_srt_nh->use_bfd = (yyvsp[0].i); cf_check_bfd((yyvsp[0].i));
   }
#line 8427 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 762:
#line 2997 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->mp_next = this_srt_nh; }
#line 8433 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 763:
#line 2998 "cf-parse.y" /* yacc.c:1646  */
    { last_srt_nh->mp_next = this_srt_nh; }
#line 8439 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 764:
#line 3002 "cf-parse.y" /* yacc.c:1646  */
    {
      this_srt->dest = RTD_ROUTER;
      this_srt->via = (yyvsp[-1].a);
      this_srt->via_if = (yyvsp[0].iface);
   }
#line 8449 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 765:
#line 3007 "cf-parse.y" /* yacc.c:1646  */
    {
      this_srt->dest = RTD_DEVICE;
      this_srt->if_name = (yyvsp[0].t);
      rem_node(&this_srt->n);
      add_tail(&STATIC_CFG->iface_routes, &this_srt->n);
   }
#line 8460 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 766:
#line 3013 "cf-parse.y" /* yacc.c:1646  */
    {
      this_srt->dest = RTD_MULTIPATH;
   }
#line 8468 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 767:
#line 3016 "cf-parse.y" /* yacc.c:1646  */
    {
      this_srt->dest = RTDX_RECURSIVE;
      this_srt->via = (yyvsp[0].a);
   }
#line 8477 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 768:
#line 3021 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->dest = RTD_BLACKHOLE; }
#line 8483 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 769:
#line 3022 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->dest = RTD_UNREACHABLE; }
#line 8489 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 770:
#line 3023 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->dest = RTD_BLACKHOLE; }
#line 8495 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 771:
#line 3024 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->dest = RTD_UNREACHABLE; }
#line 8501 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 772:
#line 3025 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->dest = RTD_PROHIBIT; }
#line 8507 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 773:
#line 3029 "cf-parse.y" /* yacc.c:1646  */
    { *this_srt_last_cmd = (yyvsp[0].x); this_srt_last_cmd = &((yyvsp[0].x)->next); }
#line 8513 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 774:
#line 3030 "cf-parse.y" /* yacc.c:1646  */
    { this_srt->use_bfd = (yyvsp[-1].i); cf_check_bfd((yyvsp[-1].i)); }
#line 8519 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 779:
#line 3045 "cf-parse.y" /* yacc.c:1646  */
    { static_show(proto_get_named((yyvsp[-1].s), &proto_static)); }
#line 8525 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 849:
#line 3053 "cf-parse.y" /* yacc.c:1646  */
    { bgp_check_config(BGP_CFG); }
#line 8531 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 850:
#line 3053 "cf-parse.y" /* yacc.c:1646  */
    { ospf_proto_finish(); }
#line 8537 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 861:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_KRT_SOURCE); }
#line 8543 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 862:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_KRT_METRIC); }
#line 8549 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 863:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); }
#line 8555 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 864:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); }
#line 8561 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 865:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_MTU); }
#line 8567 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 866:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_WINDOW); }
#line 8573 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 867:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_RTT); }
#line 8579 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 868:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_RTTVAR); }
#line 8585 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 869:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_SSTRESH); }
#line 8591 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 870:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_CWND); }
#line 8597 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 871:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_ADVMSS); }
#line 8603 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 872:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REORDERING); }
#line 8609 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 873:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_HOPLIMIT); }
#line 8615 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 874:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_INITCWND); }
#line 8621 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 875:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_RTO_MIN); }
#line 8627 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 876:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_INITRWND); }
#line 8633 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 877:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_QUICKACK); }
#line 8639 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 878:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_MTU); }
#line 8645 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 879:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_WINDOW); }
#line 8651 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 880:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_RTT); }
#line 8657 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 881:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_RTTVAR); }
#line 8663 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 882:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_SSTHRESH); }
#line 8669 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 883:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_CWND); }
#line 8675 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 884:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_ADVMSS); }
#line 8681 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 885:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_REORDERING); }
#line 8687 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 886:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_HOPLIMIT); }
#line 8693 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 887:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_LOCK_RTO_MIN); }
#line 8699 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 888:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_FEATURE_ECN); }
#line 8705 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 889:
#line 3056 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_BITFIELD, T_BOOL, EA_KRT_FEATURE_ALLFRAG); }
#line 8711 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 890:
#line 3057 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_GEN_IGP_METRIC); }
#line 8717 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 891:
#line 3057 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = NULL; }
#line 8723 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 892:
#line 3058 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_ENUM_BGP_ORIGIN, EA_CODE(EAP_BGP, BA_ORIGIN)); }
#line 8729 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 893:
#line 3059 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_AS_PATH, T_PATH, EA_CODE(EAP_BGP, BA_AS_PATH)); }
#line 8735 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 894:
#line 3060 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_CODE(EAP_BGP, BA_NEXT_HOP)); }
#line 8741 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 895:
#line 3061 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_MULTI_EXIT_DISC)); }
#line 8747 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 896:
#line 3062 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_LOCAL_PREF)); }
#line 8753 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 897:
#line 3063 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_OPAQUE, T_ENUM_EMPTY, EA_CODE(EAP_BGP, BA_ATOMIC_AGGR)); }
#line 8759 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 898:
#line 3064 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_CODE(EAP_BGP, BA_AGGREGATOR)); }
#line 8765 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 899:
#line 3065 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_COMMUNITY)); }
#line 8771 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 900:
#line 3066 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID, T_QUAD, EA_CODE(EAP_BGP, BA_ORIGINATOR_ID)); }
#line 8777 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 901:
#line 3067 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT_SET, T_CLIST, EA_CODE(EAP_BGP, BA_CLUSTER_LIST)); }
#line 8783 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 902:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_EC_SET, T_ECLIST, EA_CODE(EAP_BGP, BA_EXT_COMMUNITY)); }
#line 8789 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 903:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC1); }
#line 8795 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 904:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_METRIC2); }
#line 8801 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 905:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_OSPF_TAG); }
#line 8807 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 906:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_ROUTER_ID | EAF_TEMP, T_QUAD, EA_OSPF_ROUTER_ID); }
#line 8813 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 907:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_METRIC); }
#line 8819 "cf-parse.tab.c" /* yacc.c:1646  */
    break;

  case 908:
#line 3068 "cf-parse.y" /* yacc.c:1646  */
    { (yyval.x) = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_RIP_TAG); }
#line 8825 "cf-parse.tab.c" /* yacc.c:1646  */
    break;


#line 8829 "cf-parse.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 3070 "cf-parse.y" /* yacc.c:1906  */

/* C Code from ../../conf/confbase.Y */

/* C Code from ../../sysdep/unix/config.Y */

/* C Code from ../../sysdep/unix/krt.Y */

/* C Code from ../../sysdep/linux/netlink.Y */

/* C Code from ../../nest/config.Y */

/* C Code from ../../proto/bfd/config.Y */

/* C Code from ../../proto/bgp/config.Y */

/* C Code from ../../proto/ospf/config.Y */

/* C Code from ../../proto/pipe/config.Y */

/* C Code from ../../proto/rip/config.Y */

/* C Code from ../../proto/static/config.Y */

