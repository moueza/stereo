#ifndef STRING_TOOLS_H
#define STRING_TOOLS_H

#include "global.h"
#include "my_string.h"

#define clear(x,type) memset((x), 0, sizeof(type));

short *shortset (short *s, int c, size_t n);

/*move to col character from beginning of line with i in the line somewhere. */
/*If col is past the end of the line, it returns position of end of line. */
/*Can be used as movetobeginning of line if col = 0. */
long strfrombeginline (const char *str, int i, int col);

/*move forward from i, where `lines' can be negative --- moving backward */
/*returns pos of begin of line moved to */
long strmovelines (const char *str, long i, long lines, int width);

/*returns a positive or negative count of lines
   from i to i + amount */
long strcountlines (const char *str, long i, long amount, int width);

char *str_strip_nroff (char *t, int *l);

/*returns a null terminated string. The string
   is a copy of the line beginning at p and ending at '\n' 
   in the string src.
   The result must be free'd. */
char *strline (const char *src, int p);

int strcolmove (unsigned char *str, int i, int col);

/*  cat many strings together. Result must not be free'd.
   Free's your result after 32 calls. */
char *catstrs (const char *first,...);
void catstrs_clean (void);

/* for regexp */
enum {
    match_file, match_normal
};
extern int easy_patterns;
char *convert_pattern (char *pattern, int match_type, int do_group);
int regexp_match (char *pattern, char *string, int match_type);

char *name_trunc (const char *txt, int trunc_len);
char *path_compress (const char *dir, const char *file);

#ifdef HAVE_MAD
char *mad_vsprintf_alloc (const char *fmt, va_list ap, char *file, int line);
#define vsprintf_alloc(f,a) mad_vsprintf_alloc(f, a, __FILE__, __LINE__)
#else
char *vsprintf_alloc (const char *fmt, va_list ap);
#endif

char *sprintf_alloc (const char *fmt,...);
char *itoa (int i);
size_t strnlen (const char *s, size_t count);
void destroy (void **p);
char *get_temp_file_name (void);
int change_directory (const char *path);
char *get_current_wd (char *buffer, int size);
char *strcasechr (const char *s, int c);

#endif
