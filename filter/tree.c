/*
 *	Filters: utility functions
 *
 *	Copyright 1998 Pavel Machek <pavel@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/signal.h>
#include <setjmp.h>

#include "nest/bird.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "conf/conf.h"
#include "filter/filter.h"

/* Finds n-th item in list linked by right. Trashes pointers in right. */
static struct f_tree *
find_nth(struct f_tree *from, int nth)
{
  struct f_tree *pivot;
  int lcount = 0, rcount = 0;
  struct f_tree *left, *right, *next;

  pivot = from;

  left = right = NULL;
  next = from->right;
  while (from = next) {
    next = from->right;
    if (val_compare(pivot->from, from->from)==1) {
      from->right = left;
      left = from;
      lcount++;
    } else {
      from->right = right;
      right = from;
      rcount++;
    }
  }
  if (lcount == nth) 
    return pivot;
  if (lcount < nth)
    return find_nth(right, nth-lcount-1);
  return find_nth(left, nth);
}

/* Gets list linked by left, finds its median, trashes pointers in right */
static struct f_tree *
find_median(struct f_tree *from)
{
  struct f_tree *t = from;
  int cnt = 0;

  if (!from)
    return NULL;
  do {
    t->right = t->left;
    cnt++;
  } while (t = t->left);
  return find_nth(from, cnt/2);
}

struct f_tree *
find_tree(struct f_tree *t, struct f_val val)
{
  if (!t)
    return 0;
  if ((val_compare(t->from, val) != 1) &&
      (val_compare(t->to, val) != -1))
    return t;
  if (val_compare(t->from, val) == -1)
    return find_tree(t->right, val);
  else
    return find_tree(t->left, val);
}

/* Gets list linked by left */
struct f_tree *
build_tree(struct f_tree *from)
{
  struct f_tree *median, *t = from, *next, *left = NULL, *right = NULL;

  median = find_median(from);
  if (!median)
    return NULL;

  do {
    next = t->left;
    if (t == median)
      continue;

    if (val_compare(median->from, t->from)==1) {
      t->left = left;
      left = t;
    } else {
      t->left = right;
      right = t;
    }
  } while(t = next);

  median->left = build_tree(left);
  median->right = build_tree(right);
  return median;
}

struct f_tree *
f_new_tree(void)
{
  struct f_tree * ret;
  ret = cfg_alloc(sizeof(struct f_tree));
  ret->left = ret->right = NULL;
  ret->from.type = ret->to.type = T_VOID;
  ret->from.val.i = ret->to.val.i = 0;
  ret->data = NULL;
  return ret;
}