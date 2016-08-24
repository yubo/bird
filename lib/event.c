/*
 *	BIRD Library -- Event Processing
 *
 *	(c) 1999 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Events
 *
 * Events are there to keep track of deferred execution.
 * Since BIRD is single-threaded, it requires long lasting tasks to be split to smaller
 * parts, so that no module can monopolize the CPU. To split such a task, just create
 * an &event resource, point it to the function you want to have called and call ev_schedule()
 * to ask the core to run the struct event when nothing more important requires attention.
 *
 * You can also define your own struct event lists (the &event_list structure), enqueue your
 * events in them and explicitly ask to run them.
 */

#include "nest/bird.h"
#include "lib/event.h"


event_list global_event_list;

inline void ev_postpone(struct event *e)
{
	if (ev_active(e)) {
		list_del_init(&e->n);
		/*e->n.next = NULL;*/
	}
}

static void ev_dump(struct resource *r)
{
	struct event *e = (struct event *)r;

	debug("(code %p, data %p, %s)\n",
	      e->hook, e->data, list_empty(&e->n) ? "scheduled" : "inactive");
}

static struct resclass ev_class = {
	"Event",
	sizeof(struct event),
	(void (*)(struct resource *))ev_postpone,
	ev_dump,
	NULL,
	NULL
};

/**
 * ev_new - create a new event
 * @p: struct resource pool
 *
 * This function creates a new struct event resource. To use it,
 * you need to fill the structure fields and call ev_schedule().
 */
struct event *ev_new(struct pool *p)
{
	struct event *e = ralloc(p, &ev_class);
	return e;
}

/**
 * ev_run - run an event
 * @e: an event
 *
 * This function explicitly runs the struct event @e (calls its hook
 * function) and removes it from an struct event struct list_head if it's linked to any.
 *
 * From the hook function, you can call ev_enqueue() or ev_schedule()
 * to re-add the event.
 */
inline void ev_run(struct event *e)
{
	ev_postpone(e);
	e->hook(e->data);
}

/**
 * ev_enqueue - enqueue an event
 * @l: an struct event list
 * @e: an event
 *
 * ev_enqueue() stores the struct event @e to the specified event
 * struct list_head @l which can be run by calling ev_run_list().
 */
inline void ev_enqueue(event_list * l, struct event *e)
{
	ev_postpone(e);
	list_add_tail(&e->n, l);
}

/**
 * ev_schedule - schedule an event
 * @e: an event
 *
 * This function schedules an struct event by enqueueing it to a system-wide
 * struct event struct list_head which is run by the platform dependent code whenever
 * appropriate.
 */
void ev_schedule(struct event *e)
{
	ev_enqueue(&global_event_list, e);
}

void io_log_event(void *hook, void *data);

/**
 * ev_run_list - run an struct event list
 * @l: an struct event list
 *
 * This function calls ev_run() for all events enqueued in the struct list_head @l.
 */
int ev_run_list(event_list * l)
{
	struct list_head tmp_list;
	struct event *e;

	INIT_LIST_HEAD(&tmp_list);
	list_splice_tail(l, &tmp_list);
	INIT_LIST_HEAD(l);
	while(!list_empty(&tmp_list)){
		e = container_of(tmp_list.next, struct event, n);

		/* This is ugly hack, we want to log just events executed from the main I/O loop */
		if (l == &global_event_list)
			io_log_event(e->hook, e->data);

		ev_run(e);
	}
	return !list_empty(l);
}

