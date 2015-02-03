/*
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

/* uncomment this line for some machines that don't have standard
   headers/string-functions and cause a seg-fault on startup */

 #define CRASHES_ON_STARTUP


#include <config.h>
#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include "my_string.h"
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "stringtools.h"
#include "regex.h"
#include "time.h"

#include "mad.h"

/*
   This cats a whole lot of strings together.
   It has the advantage that the return result will
   be free'd automatically, and MUST NOT be free'd
   by the caller.
   It will hold the most recent NUM_STORED strings.
 */
#define NUM_STORED 32

static char *stacked[NUM_STORED] =
    {0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0};

char *catstrs (const char *first,...)
{
    static int i = 0;
    va_list ap;
    int len;
    char *data;

    if (!first)
	return 0;

    len = strlen (first);
    va_start (ap, first);

    while ((data = va_arg (ap, char *)) != 0)
	 len += strlen (data);

    len++;

    i = (i + 1) % NUM_STORED;
    if (stacked[i])
	free (stacked[i]);

    stacked[i] = malloc (len);
    va_end (ap);
    va_start (ap, first);
    strcpy (stacked[i], first);
    while ((data = va_arg (ap, char *)) != 0)
	 strcat (stacked[i], data);
    va_end (ap);

    return stacked[i];
}

void catstrs_clean (void)
{
    int i;
    for (i = 0; i < NUM_STORED; i++)
	if (stacked[i]) {
	    free (stacked[i]);
	    stacked[i] = 0;
	}
}


/* alternative to free() */
void destroy (void **p)
{
    if (*p) {
	free (*p);
	*p = 0;
    }
}

#define my_lower_case(x) ((x) < 'a' ? (x) - 'A' + 'a' : (x))

char *strcasechr (const char *s, int c)
{
    for (; my_lower_case (*s) != my_lower_case ((char) c); ++s)
	if (*s == '\0')
	    return 0;
    return (char *) s;
}

char *itoa (int i)
{
    static char t[20];
    char *s = t + 19;
    int j = i;
    i = abs (i);
    *s-- = 0;
    do {
	*s-- = i % 10 + '0';
    } while ((i = i / 10));
    if (j < 0)
	*s-- = '-';
    return ++s;
}

char *itoh (int i)
{
    static char t[20];
    char *s = t + 19;
    int j = i;
    i = abs (i);
    *s-- = 0;
    do {
	*s-- = i % 10 + '0';
    } while ((i = i / 10));
    if (j < 0)
	*s-- = '-';
    return ++s;
}

/*
   returns a name based on the time times the pid, result must be
   copied immediately and must not be free'd.
 */
char *get_temp_file_name (void)
{
    return catstrs (PACKAGE, itoa ((int) abs((int) time (0) * (int) getpid ())), ".tmp", 0);
}

/* this comes from the Midnight Commander src/tools.c */
char *get_current_wd (char *buffer, int size)
{
    char *p;

#ifdef HAVE_GETWD
    p = (char *) getwd (buffer);
#else
    p = getcwd (buffer, size);
#endif
    return p;
}

/*
   cd's to path and sets current_dir variable if getcwd works, else set
   current_dir to "".
 */
extern char current_dir[];
int change_directory (const char *path)
{
    int e;
    e = chdir (path);
    if (e < 0)
	return e;
    if (!get_current_wd (current_dir, MAX_PATH_LEN))
	strcpy (current_dir, "/");
    return 0;
}


short *shortset (short *s, int c, size_t n)
{
    short *r = s;
    while (n--)
	*s++ = c;
    return r;
}


char *name_trunc (const char *txt, int trunc_len)
{
    static char x[1024];
    int txt_len, y;

    txt_len = strlen (txt);
    if (txt_len <= trunc_len) {
	strcpy (x, txt);
	return x;
    }
    y = trunc_len % 2;
    strncpy (x, txt, (trunc_len / 2) + y);
    strncpy (x + (trunc_len / 2) + y, txt + txt_len - (trunc_len / 2), trunc_len / 2);
    x[(trunc_len / 2) + y] = '~';
    x[trunc_len] = 0;
    return x;
}

/* cats together dir/file considering any possibility and removes
   // /./ spaces etc */
char *path_compress (const char *dir, const char *file)
{
    static char estr[1024] = "";
    char *q;

    if (file) {
	if (*file == '/' || !dir) {
	    strcpy (estr, file);
	} else {
	    strcpy (estr, catstrs (dir, "/", file, 0));
	}
    } else {
	if (dir)
	    strcpy (estr, dir);
	else
	    getcwd (estr, 1000);
	if (!*estr)
	    return estr;
    }
    while ((q = strchr (estr, ' ')))
	memmove (q, q + 1, strlen (q));
    while ((q = strstr (estr, "//")))
	memmove (q, q + 1, strlen (q));
    while ((q = strstr (estr, "/./")))
	memmove (q, q + 2, strlen (q) + 1);
    if ((q = strstr (estr, "/.."))) {
	*q = 0;
	if ((q = strrchr (estr, '/')))
	    *(q + 1) = 0;
    }
    q = estr + strlen (estr) - 1;
    if (q > estr)
	if (*(q - 1) == '/' && *q == '.')
	    *(q - 1) = 0;
    if (!strlen (estr))
	strcpy (estr, dir);
    return estr;
}

#define r_is_printable(x) ((x >= ' ' && x <= '~') || x >= 160)

int strcolmove (unsigned char *str, int i, int column)
{
    int col = 0;
    if (column <= 0)
	return i;
    for (col = 0; col < column; i++) {
	while (str[i + 1] == '\b' && r_is_printable (str[i + 2]) && r_is_printable (str[i]))
	    i += 2;
	if (!str[i] || str[i] == '\n')
	    break;
	if (str[i] == '\r')
	    continue;
	if (str[i] == '\t') {
	    col += 8 - (col % 8);
	    continue;
	}
	col++;
    }
    return i;
}

/*move to col character from beginning of line with i in the line somewhere. */
/*If col is past the end of the line, it returns position of end of line */
long strfrombeginline (const char *s, int i, int col)
{
    unsigned char *str = (unsigned char *) s;
    if (i < 0) {
	fprintf (stderr, "strfrombeginline called with negative index.\n");
	exit (1);
    }
    while (i--)
	if (str[i] == '\n') {
	    i++;
	    break;
	}
    if (i < 0)
	i = 0;
    return strcolmove (str, i, col);
}

/*
    strip backspaces from the nroff file to produce normal text.
    returns strlen(result) if l is non null
*/
char *str_strip_nroff (char *t, int *l)
{
    unsigned char *s = (unsigned char *) t;
    unsigned char *r, *q;
    int p;

    q = r = malloc (strlen (t) + 2);
    if (!r)
	return 0;

    for (p = 0; s[p]; p++) {
	while (s[p + 1] == '\b' && r_is_printable (s[p + 2]) && r_is_printable (s[p]))
	    p += 2;
	*q++ = s[p];
    }
    *q = 0;
    if (l)
	*l = ((unsigned long) q - (unsigned long) r);
    return (char *) r;
}

long countlinesforward (const char *text, long from, long amount, long lines, int width)
{
    int col = 0, row = 0, q = 0;
    unsigned char c;
    int p = from;
    if (!(amount | lines))
	return 0;
    for (;from < p + amount || !amount; from++) {
	c = text[from];
	if (!c) {
	    if (lines)
		return q;
	    break;
	}
	if (lines && lines == row)
	    return from;
	if (c == '\n' || col == width) {
	    col = 0;
	    q = from + 1;
	    row++;
	}
	if (c == '\r')
	    continue;
	if (c == '\t') {
	    col = (col / 8) * 8 + 8;
	    continue;
	}
	col++;
    }
    return row;
}

/* returns pos of begin of line moved to */
/* move forward from i, `lines' can be negative --- moveing backward */
long strmovelines (const char *str, long from, long lines, int width)
{
    int p, q;
    if (lines > 0)
	return countlinesforward (str, from, 0, lines, width);
    if (lines == 0)
	return from;
    else {
	int line = 0;
	p = from;
	for (; p > 0;) {
	    q = p;
	    p = strfrombeginline (str, q - 1, 0);
	    line += countlinesforward (str, p, q - p, 0, width);
	    if (line > -lines)
		return countlinesforward (str, p, 0, line + lines, width);
	    if (line == -lines)
		return p;
	}
	return 0;
    }
}



/*returns a positive or negative count of lines */
long strcountlines (const char *str, long i, long amount, int width)
{
    int lines, p;
    if (amount > 0) {
	return countlinesforward (str, i, amount, 0, width);
    }
    if (amount == 0)
	return 0;
    if (i + amount < 0)
	amount = -i;
    p = strfrombeginline (str, i + amount, 0);
    lines = countlinesforward (str, p, i + amount - p, 0, width);
    return -countlinesforward (str, p, i - p, 0, width) + lines;
}

/*
   returns a null terminated string. The string
   is a copy of the line beginning at p and ending at '\n'
   in the string src.
   The result must not be free'd. This routine caches the last
   four results.
 */
char *strline (const char *src, int p)
{
    static char line[4][1024];
    static int last = 0;
    int i = 0;
    char *r;
    while (src[p] != '\n' && src[p] && i < 1000) {
	i++;
	p++;
    }
    r = line[last & 3];
    memcpy (r, src + p - i, i);
    r[i] = 0;
    last++;
    return r;
}

/*
size_t strnlen (const char *s, size_t count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc)
	/* nothing */ ;
/*    return sc - s;
} */


#ifdef CRASHES_ON_STARTUP

/* (this is waistful, but it solves sunos's non-standardness
   that causes a segfault) */
#define vfmtlen(a,b) (strlen(a)+2048)

#else				/* CRASHES_ON_STARTUP */

#define is_digit(x) ((x) >= '0' && (x) <= '9')

#define scount(v) { \
		*p1++ = *p++; \
		*p1++ = '%'; \
		*p1++ = 'n'; \
		*p1 = 0; \
		sprintf(s,q1,v,&n); \
		count += n; \
	    }


/* returns the length of a string that would be printed if this
   command was vprintf, but prints nothing */
size_t vfmtlen (const char *fmt, va_list ap)
{
    char *q, *p, s[66];
    int n;
    char q1[32];
    char *p1;
    size_t count = 0;

    p = q = (char *) fmt;

    while ((p = strchr (p, '%'))) {
	count += (size_t) ((unsigned long) p - (unsigned long) q);
	q = p;
	p1 = q1;
	*p1++ = *p++;
	if (*p == '%') {
	    p++;
	    count++;
	    q = p;
	    continue;
	}
	if (*p == 'n') {
	    p++;
	    q = p;
	    *va_arg (ap, int *) = count;
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
	    strcpy (p1, itoa (va_arg (ap, int)));
	    p1 += strlen (p1);
	} else {
	    while (is_digit (*p))
		*p1++ = *p++;
	}
	if (*p == '.')
	    *p1++ = *p++;
	if (*p == '*') {
	    p++;
	    strcpy (p1, itoa (va_arg (ap, int)));
	    p1 += strlen (p1);
	} else {
	    while (is_digit (*p))
		*p1++ = *p++;
	}
	if (*p == 's') {
	    scount (va_arg (ap, char *));
	} else if (*p == 'h') {
	    if (strchr ("diouxX", *p))
		scount (va_arg (ap, short));
	} else if (*p == 'l') {
	    *p1++ = *p++;
	    if (strchr ("diouxX", *p))
		scount (va_arg (ap, long));
	} else if (strchr ("cdiouxX", *p)) {
	    scount (va_arg (ap, int));
	} else if (*p == 'L') {
	    *p1++ = *p++;
	    if (strchr ("EefgG", *p))
		scount (va_arg (ap, double));	/* should be long double, but gives warnings on some machines */
	} else if (strchr ("EefgG", *p)) {
	    scount (va_arg (ap, double));
	} else if (strchr ("DOU", *p)) {
	    scount (va_arg (ap, long));
	} else if (*p == 'p') {
	    scount (va_arg (ap, void *));
	}
	q = p;
    }
    return count + strlen (q);
}

#endif				/* CRASHES_ON_STARTUP */

#ifndef HAVE_MAD

/* vsprintf with memory allocation. result must be free'd */
char *vsprintf_alloc (const char *fmt, va_list ap)
{
    char *s;
    size_t l;
    s = malloc ((l = vfmtlen (fmt, ap)) + 1);
    if (!s)
	fprintf (stderr, "cooledit:%s:%d: malloc return zero\n", __FILE__, __LINE__);
    s[l] = 0;
    vsprintf (s, fmt, ap);
    if (s[l])
/* this is just in case there is a bug in vfmtlen above (it also
    happens if you pass a incorrect format string) */
	fprintf (stderr, "cooledit:%s:%d: vsprintf wrote out of bounds\n", __FILE__, __LINE__);
    return s;
}

#else

char *mad_vsprintf_alloc (const char *fmt, va_list ap, char *file, int line)
{
    char *s;
    s = mad_alloc (vfmtlen (fmt, ap) + 1, file, line);
    vsprintf (s, fmt, ap);
    return s;
}

#endif

char *sprintf_alloc (const char *fmt,...)
{
    char *s;
    va_list ap;
    va_start (ap, fmt);
    s = vsprintf_alloc (fmt, ap);
    va_end (ap);
    return s;
}
