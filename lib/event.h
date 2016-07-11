/*
 *	BIRD Library -- Event Processing
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_EVENT_H_
#define _BIRD_EVENT_H_

#include "lib/resource.h"

struct event {
	struct resource r;
	void (*hook) (void *);
	void *data;
	struct node n;		/* Internal link */
};

typedef union list event_list;

extern event_list global_event_list;

struct event *ev_new(struct pool *);
void ev_run(struct event *);
#define ev_init_list(el) init_list(el)
void ev_enqueue(event_list *, struct event *);
void ev_schedule(struct event *);
void ev_postpone(struct event *);
int ev_run_list(event_list *);

static inline int ev_active(struct event * e)
{
	return e->n.next != NULL;
}

#endif
