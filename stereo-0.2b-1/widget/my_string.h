#ifndef _MY_STRING_H
#define _MY_STRING_H

#include <stdlib.h> /* this my not be needed */
#include <sys/types.h> /* this my not be needed */
#include <ctype.h> /* this my not be needed */

#include <stdarg.h>

#define MAX_PATH_LEN 512

/* string include, hopefully works across all unixes */

#ifndef INHIBIT_STRING_HEADER
# if defined (HAVE_STRING_H) || defined (STDC_HEADERS) || defined (_LIBC)
#  include <string.h>
# else
#  include <strings.h>
# endif
#endif

#ifndef STDC_HEADERS
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif

    size_t strnlen (const char *s, size_t count);

# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
# endif
# ifndef HAVE_MEMCMP
    int memcmp (const void *cs, const void *ct, size_t count);
# endif
# ifndef HAVE_MEMCHR
    void *memchr (const void *s, int c, size_t n);
# endif
#ifndef HAVE_STRCASECMP
    int strcasecmp (const char *s1, const char *s2);
#endif
#ifndef HAVE_STRNCASECMP
    int strncasecmp (const char *s1, const char *s2, size_t n);
#endif
# ifndef HAVE_STRDUP
    char *strdup (const char *s);
#  endif
#ifndef HAVE_MEMMOVE
    void *memmove (void *dest, const void *src, size_t n);
# endif
# ifndef HAVE_MEMSET
    void *memset (void *dest, int c, size_t n);
# endif
# ifndef HAVE_STRSPN
    size_t strspn (const char *s, const char *accept);
# endif
# ifndef HAVE_STRSTR
    char *strstr (const char *s1, const char *s2);
# endif
# ifndef HAVE_VPRINTF
    int vsprintf (char *buf, const char *fmt, va_list args);
# endif
#endif

#endif				/*  _MY_STRING_H  */

