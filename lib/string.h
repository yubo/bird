/*
 *	BIRD Library -- String Functions
 *
 *	(c) 1998 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_STRING_H_
#define _BIRD_STRING_H_

#include <stdarg.h>
#include <string.h>
#include <strings.h>

int bsprintf(char *str, const char *fmt, ...);
int bvsprintf(char *str, const char *fmt, va_list args);
int bsnprintf(char *str, int size, const char *fmt, ...);
int bvsnprintf(char *str, int size, const char *fmt, va_list args);

int buffer_vprint(struct buffer *buf, const char *fmt, va_list args);
int buffer_print(struct buffer *buf, const char *fmt, ...);
void buffer_puts(struct buffer *buf, const char *str);

int patmatch(const byte *pat, const byte *str);

static inline char *xbasename(const char *str)
{
  char *s = strrchr(str, '/');
  return s ? s+1 : (char *) str;
}

#define ROUTER_ID_64_LENGTH 23

#endif
