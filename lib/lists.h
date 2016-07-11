/*
 *	BIRD Library -- Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_LISTS_H_
#define _BIRD_LISTS_H_

/*
 * I admit the union list structure is very tricky and also somewhat awkward,
 * but it's both efficient and easy to manipulate once one understands the
 * basic trick: The union list head always contains two synthetic nodes which are
 * always present in the list: the head and the tail. But as the `next'
 * entry of the tail and the `prev' entry of the head are both NULL, the
 * nodes can overlap each other:
 *
 *     head    head_node.next
 *     null    head_node.prev  tail_node.next
 *     tail                    tail_node.prev
 */

struct node {
	 struct node *next, *prev;
};

union list {		/* In fact two overlayed nodes */
	struct {		/* Head struct node */
		 struct node head_node;
		void *head_padding;
	};
	struct {		/* Tail struct node */
		void *tail_padding;
		 struct node tail_node;
	};
	struct {		/* Split to separate pointers */
		 struct node *head;
		 struct node *null;
		 struct node *tail;
	};
};

#define NODE (struct node *)
#define HEAD(list) ((void *)((list).head))
#define TAIL(list) ((void *)((list).tail))
#define NODE_NEXT(n) ((void *)((NODE (n))->next))
#define NODE_VALID(n) ((NODE (n))->next)
#define WALK_LIST(n,list) for(n=HEAD(list); NODE_VALID(n); n=NODE_NEXT(n))
#define WALK_LIST2(n,nn,list,pos) \
  for(nn=(list).head; NODE_VALID(nn) && (n=SKIP_BACK(typeof(*n),pos,nn)); nn=nn->next)
#define WALK_LIST_DELSAFE(n,nxt,list) \
     for(n=HEAD(list); nxt=NODE_NEXT(n); n=(void *) nxt)
/* WALK_LIST_FIRST supposes that called code removes each processed struct node */
#define WALK_LIST_FIRST(n,list) \
     while(n=HEAD(list), (NODE (n))->next)
#define WALK_LIST_BACKWARDS(n,list) for(n=TAIL(list);(NODE (n))->prev; \
				n=(void *)((NODE (n))->prev))
#define WALK_LIST_BACKWARDS_DELSAFE(n,prv,list) \
     for(n=TAIL(list); prv=(void *)((NODE (n))->prev); n=(void *) prv)

#define EMPTY_LIST(list) (!(list).head->next)

#ifndef _BIRD_LISTS_C_
#define LIST_INLINE static inline
#include "lib/lists.c"
#undef LIST_INLINE

#else /* _BIRD_LISTS_C_ */
#define LIST_INLINE
void add_tail(union list *, struct node *);
void add_head(union list *, struct node *);
void rem_node(struct node *);
void add_tail_list(union list *, union list *);
void init_list(union list *);
void insert_node(struct node *, struct node *);
#endif

#endif
