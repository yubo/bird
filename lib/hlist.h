// Copyright 2016 yubo. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.
#ifndef __HLIST_H__
#define __HLIST_H__
#include <stdlib.h>
#include "lib/list.h"

/*
 * hlist   l
 *   snode  n
 *     siterator i
 */

struct hlist {
	struct list_head n;
	struct list_head l;
	struct list_head *h;
};

#define HLIST (struct hlist *)
#define HLIST_NEXT(p) ((void *)(((struct list_head *) (p))->next))

/*
 * i: siterator
 */
static inline struct hlist *h_get(struct hlist *i)
{
	list_del_init(&i->n);
	return (struct hlist *)i->h;
}

/* 
 * new: siterator
 */
static inline void h_put(struct hlist *new, struct hlist *snode)
{
	list_add_tail(&new->n, &snode->l);
	new->h = (struct list_head *)snode;
}

/*
 * l: slist
 * n: snode
 * add n to l tail
 */
static inline void h_add_tail(struct hlist *l, struct hlist *new)
{
	list_add_tail(&new->n, &l->n);
}

/*
 * l: slist
 * n: snode
 * add n to l
 */
static inline void h_add_head(struct hlist *l, struct hlist *new)
{
	list_add(&new->n, &l->n);
}

static inline void h_insert_node(struct hlist *new, struct hlist *after)
{
	list_add(&new->n, &after->n);
}

/*
 * from: snode
 * to: snode
 */
static inline void h_merge(struct hlist *from, struct hlist *to)
{
	struct hlist *p;

	if (list_empty(&from->l))
		return;

	list_for_each_entry(p, &from->l, n){
		p->h = (struct list_head *)to;
	}

	list_splice_tail(&from->l, &to->l);
}

/* 
 * n : snode 
 */
static inline void h_rem_node(struct hlist *n)
{
	struct hlist *next;
	next = (struct hlist *)n->n.next;

	list_del_init(&n->n);

	h_merge(n, next);
}

static inline void h_init_list(struct hlist *l)
{
	INIT_LIST_HEAD(&l->n);
	INIT_LIST_HEAD(&l->l);
}

/*
 * new: siterator
 * l: slist
 */
static inline void h_init(struct hlist *_new, struct hlist *l)
{
	h_put(_new, (struct hlist *)l->n.next);
}

/*
void s_add_tail_list(struct lists_head *to, struct lists_head *new)
{
	struct snode *p = to->tail;
	struct snode *q = l->head;

	p->next = q;
	q->prev = p;
	q = l->tail;
	q->next = (struct snode *)&to->null;
	to->tail = q;
	h_merge((struct snode *)&l->null, (struct snode *)&to->null);
}
*/

#ifdef TEST

#include "lib/resource.h"
#include <stdio.h>

static void dump(char *c, struct hlist *a)
{
	struct hlist *n, *i;

	puts(c);
	list_for_each_entry(n, &a->n, n){
		printf("snode %p", n);

		list_for_each_entry(i, &n->l, n){
			printf(" iterator [%p:%p]", i, i->h);
		}
		putchar('\n');
	}

	printf("slist %p", &a);
	list_for_each_entry(i, &a->l, l){
		printf(" iterator [%p:%p]", i, i->h);
	}
	putchar('\n');

	puts("---");
}

int main(void)
{
	struct hlist a, b;
	struct hlist *x, *y;
	struct hlist i, j, k;

	h_init_list(&a);
	h_init_list(&b);
	x = malloc(sizeof(*x));
	INIT_LIST_HEAD(&x->n);
	INIT_LIST_HEAD(&x->l);
	h_add_tail(&a, x);

	x = malloc(sizeof(*x));
	INIT_LIST_HEAD(&x->n);
	INIT_LIST_HEAD(&x->l);
	h_add_tail(&a, x);

	x = malloc(sizeof(*x));
	INIT_LIST_HEAD(&x->n);
	INIT_LIST_HEAD(&x->l);
	h_add_tail(&a, x);

	dump("1", &a);

	h_init(&i, &a);
	dump("2", &a);

	h_init(&j, &a);
	dump("2", &a);

	x = h_get(&i);
	printf("Got %p\n", x);
	dump("3", &a);

	h_put(&i, x);
	dump("4", &a);

	y = h_get(&j);
	while (y != &a) {
		h_put(&j, y);
		dump("5*", &a);
		y = container_of(h_get(&j)->n.next, struct hlist, n);
	}

	dump("5 done", &a);

	h_rem_node(list_first_entry(&a.n, struct hlist, n));
	dump("6 (deletion)", &a);

	h_put(&i, container_of(h_get(&i)->n.next, struct hlist, n));
	dump("6 (relink)", &a);

	h_init(&k, &b);
	x = malloc(sizeof(*x));
	INIT_LIST_HEAD(&x->n);
	INIT_LIST_HEAD(&x->l);
	h_add_tail(&b, x);
	dump("7 (second list)", &b);
/*
	s_add_tail_list(&b, &a);
	dump("8 (after merge)", &b);
*/
	return 0;
}

#endif
#endif
