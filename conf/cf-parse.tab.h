/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

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
#line 541 "cf-parse.y" /* yacc.c:1909  */

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

#line 462 "cf-parse.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE cf_lval;

int cf_parse (void);

#endif /* !YY_CF_CF_PARSE_TAB_H_INCLUDED  */
