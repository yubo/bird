/*
 *	BIRD Library -- Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

/**
 * DOC: Linked lists
 *
 * The BIRD library provides a set of functions for operating on linked
 * lists. The lists are internally represented as standard doubly linked
 * lists with synthetic head and tail which makes all the basic operations
 * run in constant time and contain no extra end-of-list checks. Each list
 * is described by a &list structure, nodes can have any format as long
 * as they start with a &node structure. If you want your nodes to belong
 * to multiple lists at once, you can embed multiple &node structures in them
 * and use the SKIP_BACK() macro to calculate a pointer to the start of the
 * structure from a &node pointer, but beware of obscurity.
 *
 * There also exist safe linked lists (&slist, &snode and all functions
 * being prefixed with |s_|) which support asynchronous walking very
 * similar to that used in the &fib structure.
 */

#define _BIRD_LISTS_C_

#include "nest/bird.h"
#include "lib/lists.h"

/**
 * add_tail - append a struct node to a list
 * @l: linked list
 * @n: union list node
 *
 * add_tail() takes a struct node @n and appends it at the end of the union list @l.
 */
LIST_INLINE void
add_tail(union list *l, struct node *n)
{
  struct node *z = l->tail;

  n->next = &l->tail_node;
  n->prev = z;
  z->next = n;
  l->tail = n;
}

/**
 * add_head - prepend a struct node to a list
 * @l: linked list
 * @n: union list node
 *
 * add_head() takes a struct node @n and prepends it at the start of the union list @l.
 */
LIST_INLINE void
add_head(union list *l, struct node *n)
{
  struct node *z = l->head;

  n->next = z;
  n->prev = &l->head_node;
  z->prev = n;
  l->head = n;
}

/**
 * insert_node - insert a struct node to a list
 * @n: a new union list node
 * @after: a struct node of a list
 *
 * Inserts a struct node @n to a linked union list after an already inserted
 * struct node @after.
 */
LIST_INLINE void
insert_node(struct node *n, struct node *after)
{
  struct node *z = after->next;

  n->next = z;
  n->prev = after;
  after->next = n;
  z->prev = n;
}

/**
 * rem_node - remove a struct node from a list
 * @n: struct node to be removed
 *
 * Removes a struct node @n from the union list it's linked in. Afterwards, struct node @n is cleared.
 */
LIST_INLINE void
rem_node(struct node *n)
{
  struct node *z = n->prev;
  struct node *x = n->next;

  z->next = x;
  x->prev = z;
  n->next = NULL;
  n->prev = NULL;
}

/**
 * replace_node - replace a struct node in a union list with another one
 * @old: struct node to be removed
 * @new: struct node to be inserted
 *
 * Replaces struct node @old in the union list it's linked in with struct node @new.  Node
 * @old may be a copy of the original node, which is not accessed
 * through the list. The function could be called with @old == @new,
 * which just fixes neighbors' pointers in the case that the node
 * was reallocated.
 */
LIST_INLINE void
replace_node(struct node *old, struct node *new)
{
  old->next->prev = new;
  old->prev->next = new;

  new->prev = old->prev;
  new->next = old->next;
}

/**
 * init_list - create an empty list
 * @l: list
 *
 * init_list() takes a &list structure and initializes its
 * fields, so that it represents an empty list.
 */
LIST_INLINE void
init_list(union list *l)
{
  l->head = &l->tail_node;
  l->null = NULL;
  l->tail = &l->head_node;
}

/**
 * add_tail_list - concatenate two lists
 * @to: destination list
 * @l: source list
 *
 * This function appends all elements of the union list @l to
 * the union list @to in constant time.
 */
LIST_INLINE void
add_tail_list(union list *to, union list *l)
{
  struct node *p = to->tail;
  struct node *q = l->head;

  p->next = q;
  q->prev = p;
  q = l->tail;
  q->next = &to->tail_node;
  to->tail = q;
}
