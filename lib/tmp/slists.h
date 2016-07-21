/*
 *	BIRD Library -- Safe Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_SLISTS_H_
#define _BIRD_SLISTS_H_

/*
 *  These linked lists work in a way similar to standard lists defined
 *  in lib/lists.h, but in addition to all usual struct list_head functions they
 *  provide fast deletion/insertion/everything-safe asynchronous
 *  walking.
 *
 *  Example:
 *		struct slist l;
 *		struct siterator i;
 *		struct snode *n;
 *
 *	       	s_init(&i, &l);		// Initialize iteration
 *		...
 *		n = s_get(&i);		// Some time later, fetch present
 *					// value of the iterator and unlink it
 *					// from the list.
 *		while (n->next) {
 *		     ...
 *		     if (decided_to_stop) {
 *			s_put(&i, n);	// Store current position (maybe even
 *					// that we stay at struct list_head end)
 *			return;		// and return
 *		     }
 *		     ...
 *		}
 *		// After finishing, don't link the iterator back
 */

struct snode {
	struct snode *next, *prev;
	struct siterator *readers;
};

struct slist {			/* In fact two overlayed snodes */
	struct snode *head, *null, *tail;
	struct siterator *tail_readers;
};

struct siterator {
	/*
	 * Caution: Layout of this structure depends hard on layout of the
	 *          snode. Our `next' must be at position of struct snode `readers'
	 *          field, our `null' must be at position of `prev' and it must
	 *          contain NULL in order to distinguish between siterator
	 *          and struct snode (snodes with NULL `prev' field never carry
	 *          iterators). You are not expected to understand this.
	 */
	struct siterator *prev, *null, *next;
	/*
	 * For recently merged nodes this can be NULL, but then it's NULL
	 * for all successors as well. This is done to speed up iterator
	 * merging when there are lots of deletions.
	 */
	struct snode *node;
};

#define SNODE (struct snode *)
#define SHEAD(list) ((void *)((list).head))
#define STAIL(list) ((void *)((list).tail))
#define SNODE_NEXT(n) ((void *)((SNODE (n))->next))
#define SNODE_VALID(n) ((SNODE (n))->next)

#define WALK_SLIST(n,list) for(n=SHEAD(list); SNODE_VALID(n); n=SNODE_NEXT(n))
#define WALK_SLIST_DELSAFE(n,nxt,list) \
     for(n=SHEAD(list); nxt=SNODE_NEXT(n); n=(void *) nxt)
#define EMPTY_SLIST(list) (!(list).head->next)

void s_list_add_tail(struct slist *, struct snode *);
void s_add_head(struct slist *, struct snode *);
void s_list_del(struct snode *);
void s_add_tail_list(struct slist *, struct slist *);
void s_INIT_LIST_HEAD(struct slist *);
void s_list_add(struct snode *, struct snode *);

struct snode *s_get(struct siterator *);
void s_put(struct siterator *, struct snode *n);
static inline void s_init(struct siterator *i, struct slist *l)
{
	s_put(i, SHEAD(*l));
}

static inline int s_is_used(struct siterator *i)
{
	return (i->prev != NULL);
}

#endif
