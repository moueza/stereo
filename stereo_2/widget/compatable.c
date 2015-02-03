/* compatable.c - these are substitute string and memory functions
   for when the configure script can't find them on the system.
   They may not work properly.

   Copyright (C) 1996 Paul Sheer

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include "my_string.h"
#include "stringtools.h"


#ifndef HAVE_MEMSET
void *memset (void *dest, int c, size_t n)
{
    char *d = (char *) dest;
#ifdef HAVE_BZERO
    if (!c) {
	bzero (dest, n);
	return dest;
    }
#endif
    while (n--)
	*d++ = c;
    return dest;
}
#endif

#ifndef HAVE_MEMCHR
void *memchr (const void *s, int c, size_t n)
{
    unsigned char *m = (unsigned char *) s;
    while (n--) {
	if (*m == c)
	    return m;
	m++;
    }
    return 0;
}
#endif

#ifndef HAVE_MEMCMP
int memcmp (const void *m1, const void *m2, size_t n)
{
    const unsigned char *s1, *s2;
    signed char t = 0;

    if (!n)
	return 0;
    for (s1 = m1, s2 = m2; 0 < n; ++s2, ++s1, n--)
	if ((t = *s1 - *s2) != 0)
	    break;
    return t;
}
#endif


#ifndef HAVE_STRSTR
char *strstr (const char *s1, const char *s2)
{
    int l1, l2;

    l2 = strlen (s2);
    if (!l2)
	return (char *) s1;
    l1 = strlen (s1);
    while (l1 >= l2) {
	l1--;
	if (!memcmp (s1, s2, l2))
	    return (char *) s1;
	s1++;
    }
    return NULL;
}
#endif


#ifndef HAVE_STRSPN
size_t strspn (const char *s, const char *accept)
{
    const char *p;
    const char *a;
    size_t count = 0;

    for (p = s; *p != '\0'; ++p) {
	for (a = accept; *a != '\0'; ++a) {
	    if (*p == *a)
		break;
	}
	if (*a == '\0')
	    return count;
	++count;
    }
    return count;
}
#endif


#ifndef HAVE_MEMMOVE
void *memmove (void *dest, const void *src, size_t n)
{
    char *t, *s;

    if (dest <= src) {
	t = (char *) dest;
	s = (char *) src;
	while (n--)
	    *t++ = *s++;
    } else {
	t = (char *) dest + n;
	s = (char *) src + n;
	while (n--)
	    *--t = *--s;
    }
    return dest;
}
#endif

#define my_lower_case(x) ((x) < 'a' ? (x) - 'A' + 'a' : (x))


#ifndef HAVE_STRCASECMP
int strcasecmp (const char *s1, const char *s2)
{
    signed char c;
    for (;;)
	if ((c = my_lower_case (*s1) - my_lower_case (*s2)) != 0 || !*s1++ || !*s2++)
	    break;
    return c;
}
#endif

#ifndef HAVE_STRNCASECMP
int strncasecmp (const char *s1, const char *s2, size_t n)
{
    signed char c = 0;
    while (n--)
	if ((c = my_lower_case (*s1) - my_lower_case (*s2)) != 0 || !*s1++ || !*s2++)
	    break;
    return c;
}
#endif


#ifndef HAVE_STRDUP
char *strdup (const char *s)
{
    char *p = malloc (strlen (s) + 1);
    if (!p)
	return 0;
    strcpy (p, s);
    return p;
}
#endif


#ifndef HAVE_VPRINTF

#define is_digit(x) ((x) >= '0' && (x) <= '9')

#define snprintf(v) { \
		*p1++ = *p++; \
		*p1++ = '%'; \
		*p1++ = 'n'; \
		*p1 = '\0'; \
		sprintf(s,q1,v,&n); \
		s += n; \
	    }

/* this function uses the sprintf command to do a vsprintf */
int vsprintf (char *str, const char *fmt, va_list ap)
{
    char *q, *p, *s = str;
    int n;
    char q1[32];
    char *p1;

    p = q = (char *) fmt;

    while ((p = strchr (p, '%'))) {
	n = (int) ((unsigned long) p - (unsigned long) q);
	strncpy (s, q, n); /* copy stuff between format specifiers */
	s += n;
	*s = 0;
	q = p;
	p1 = q1;
	*p1++ = *p++;
	if (*p == '%') {
	    p++;
	    *s++ = '%';
	    q = p;
	    continue;
	}
	if (*p == 'n') {
	    p++;
/* print nothing */
	    q = p;
	    *va_arg(ap, int *) = (int) ((unsigned long) s - (unsigned long) str);
	    continue;
	}
	if (*p == '#')
	    *p1++ = *p++;
	if (*p == '0')
	    *p1++ = *p++;
	if (*p == '-')
	    *p1++ = *p++;
	if (*p == '+')
	    *p1++ = *p++;
	if (*p == '*') {
	    p++;
	    strcpy (p1, itoa (va_arg (ap, int))); /* replace field width with a number */
	    p1 += strlen (p1);
	} else {
	    while (is_digit (*p))
		*p1++ = *p++;
	}
	if (*p == '.')
	    *p1++ = *p++;
	if (*p == '*') {
	    p++;
	    strcpy (p1, itoa (va_arg (ap, int)));/* replace precision with a number */
	    p1 += strlen (p1);
	} else {
	    while (is_digit (*p))
		*p1++ = *p++;
	}
/* flags done, now get argument */
	if (*p == 's') {
	    snprintf (va_arg (ap, char *));
	} else if (*p == 'h') {
	    if (strchr ("diouxX", *p))
		snprintf (va_arg (ap, short));
	} else if (*p == 'l') {
	    *p1++ = *p++;
	    if (strchr ("diouxX", *p))
		snprintf (va_arg (ap, long));
	} else if (strchr ("cdiouxX", *p)) {
	    snprintf (va_arg (ap, int));
	} else if (*p == 'L') {
	    *p1++ = *p++;
	    if (strchr ("EefgG", *p))
		snprintf (va_arg (ap, double));	/* should be long double, but gives warnings on some machines */
	} else if (strchr ("EefgG", *p)) {
	    snprintf (va_arg (ap, double));
	} else if (strchr ("DOU", *p)) {
	    snprintf (va_arg (ap, long));
	} else if (*p == 'p') {
	    snprintf (va_arg (ap, void *));
	}
	q = p;
    }
    va_end (ap);
    sprintf (s, q); /* print trailing leftover */
    return ((unsigned long) s - (unsigned long) str) + strlen (s);
}

#endif

