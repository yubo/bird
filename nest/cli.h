/*
 *	BIRD Internet Routing Daemon -- Command-Line Interface
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_CLI_H_
#define _BIRD_CLI_H_

#include "lib/resource.h"
#include "lib/event.h"

#define CLI_RX_BUF_SIZE 4096
#define CLI_TX_BUF_SIZE 4096
#define CLI_MAX_ASYNC_QUEUE 4096

#define CLI_MSG_SIZE 500
#define CLI_LINE_SIZE 512

struct cli_out {
	struct cli_out *next;
	byte *wpos, *outpos, *end;
	byte buf[0];
};

struct cli {
	struct list_head n;		/* Node in struct list_head of all log hooks */
	struct pool *pool;
	void *priv;		/* Private to sysdep layer */
	byte *rx_buf, *rx_pos, *rx_aux;	/* sysdep */
	struct cli_out *tx_buf, *tx_pos, *tx_write;
	struct event *event;
	void (*cont) (struct cli * c);
	void (*cleanup) (struct cli * c);
	void *rover;		/* Private to continuation routine */
	int last_reply;
	int restricted;		/* CLI is restricted to read-only commands */
	struct linpool *parser_pool;	/* Pool used during parsing */
	byte *ring_buf;		/* Ring struct buffer for asynchronous messages */
	byte *ring_end, *ring_read, *ring_write;	/* Pointers to the ring struct buffer */
	uint ring_overflow;	/* Counter of ring overflows */
	uint log_mask;		/* Mask of allowed message levels */
	uint log_threshold;	/* When free < log_threshold, store only important messages */
	uint async_msg_size;	/* Total size of async messages queued in tx_buf */
};

extern struct pool *cli_pool;
extern struct cli *this_cli;	/* Used during parsing */

#define CLI_ASYNC_CODE 10000

/* Functions to be called by command handlers */

void cli_printf(struct cli *, int, char *, ...);
#define cli_msg(x...) cli_printf(this_cli, x)
void cli_set_log_echo(struct cli *, uint mask, uint size);

/* Functions provided to sysdep layer */

struct cli *cli_new(void *);
void cli_init(void);
void cli_free(struct cli *);
void cli_kick(struct cli *);
void cli_written(struct cli *);
void cli_echo(uint class, byte * msg);

static inline int cli_access_restricted(void)
{
	if (this_cli && this_cli->restricted){
		/*cli_printf(this_cli, 8007, "Access denied");*/
		return 1;
	} else
		return 0;
}

/* Functions provided by sysdep layer */

void cli_write_trigger(struct cli *);
int cli_get_command(struct cli *);

#endif
