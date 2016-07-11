/*
 *	BIRD Library -- Safe Linked Lists
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#define _BIRD_SLISTS_C_

#include "nest/bird.h"
#include "lib/slists.h"

static inline void
s_merge(struct snode *from, struct snode *to)
{
  struct siterator *f, *g;

  if (!(f = from->readers))
    return;
  if (!(g = to->readers))
    {
      /* Fast path */
      to->readers = f;
      f->prev = (struct siterator *) to;
    fixup:
      while (f && f->node)
	{
	  f->node = NULL;
	  f = f->next;
	}
      return;
    }
  /* Really merging */
  while (g->next)
    g = g->next;
  g->next = f;
  f->prev = g;
  goto fixup;
}

struct snode *
s_get(struct siterator *i)
{
  struct siterator *f, *g;
  struct snode *n;

  if (!(n = i->node))
    {
      /*
       * No struct node found. We have to walk the iterator union list backwards
       * to find where are we linked.
       */
      f = i;
      while (!f->null)
	f = f->prev;
      n = (struct snode *) f;
    }
  f = i->prev;				/* Maybe the struct snode itself */
  g = i->next;
  f->next = g;
  if (g)
    g->prev = f;

  i->prev = NULL;
  i->next = NULL;
  return n;
}

void
s_put(struct siterator *i, struct snode *n)
{
  struct siterator *f;

  i->node = n;
  if (f = n->readers)
    f->prev = i;
  i->next = f;
  n->readers = i;
  i->prev = (struct siterator *) n;
  i->null = NULL;
}

void
s_add_tail(struct slist *l, struct snode *n)
{
  struct snode *z = l->tail;

  n->next = (struct snode *) &l->null;
  n->prev = z;
  z->next = n;
  l->tail = n;
  n->readers = NULL;
}

void
s_add_head(struct slist *l, struct snode *n)
{
  struct snode *z = l->head;

  n->next = z;
  n->prev = (struct snode *) &l->head;
  z->prev = n;
  l->head = n;
  n->readers = NULL;
}

void
s_insert_node(struct snode *n, struct snode *after)
{
  struct snode *z = after->next;

  n->next = z;
  n->prev = after;
  after->next = n;
  z->prev = n;
  n->readers = NULL;
}

void
s_rem_node(struct snode *n)
{
  struct snode *z = n->prev;
  struct snode *x = n->next;

  z->next = x;
  x->prev = z;
  s_merge(n, x);
}

void
s_init_list(struct slist *l)
{
  l->head = (struct snode *) &l->null;
  l->null = NULL;
  l->tail = (struct snode *) &l->head;
  l->tail_readers = NULL;
}

void
s_add_tail_list(struct slist *to, struct slist *l)
{
  struct snode *p = to->tail;
  struct snode *q = l->head;

  p->next = q;
  q->prev = p;
  q = l->tail;
  q->next = (struct snode *) &to->null;
  to->tail = q;
  s_merge((struct snode *) &l->null, (struct snode *) &to->null);
}

#ifdef TEST

#include "lib/resource.h"
#include <stdio.h>

void dump(char *c, struct slist *a)
{
  struct snode *x;

  puts(c);
  for(x=SHEAD(*a); x; x=x->next)
    {
      struct siterator *i, *j;
      printf("%p", x);
      j = (struct siterator *) x;
      for(i=x->readers; i; i=i->next)
	{
	  if (i->prev != j)
	    printf(" ???");
	  j = i;
	  printf(" [%p:%p]", i, i->node);
	}
      putchar('\n');
    }
  puts("---");
}

int main(void)
{
  struct slist a, b;
  struct snode *x, *y;
  struct siterator i, j;

  s_init_list(&a);
  s_init_list(&b);
  x = xmalloc(sizeof(*x));
  s_add_tail(&a, x);
  x = xmalloc(sizeof(*x));
  s_add_tail(&a, x);
  x = xmalloc(sizeof(*x));
  s_add_tail(&a, x);
  dump("1", &a);

  s_init(&i, &a);
  s_init(&j, &a);
  dump("2", &a);

  x = s_get(&i);
  printf("Got %p\n", x);
  dump("3", &a);

  s_put(&i, x->next);
  dump("4", &a);

  y = s_get(&j);
  while (y)
    {
      s_put(&j, y);
      dump("5*", &a);
      y = s_get(&j)->next;
    }

  dump("5 done", &a);

  s_rem_node(a.head->next);
  dump("6 (deletion)", &a);

  s_put(&i, s_get(&i)->next);
  dump("6 (relink)", &a);

  x = xmalloc(sizeof(*x));
  s_add_tail(&b, x);
  dump("7 (second list)", &b);

  s_add_tail_list(&b, &a);
  dump("8 (after merge)", &b);

  return 0;
}

#endif
