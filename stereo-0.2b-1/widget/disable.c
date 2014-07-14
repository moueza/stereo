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

#include <config.h>
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "dirtools.h"

#include "coolwidget.h"
#include "coollocal.h"
#include "dialog.h"
#include "regex.h"

#include "mad.h"


/* 1 if string matches
   0 if string doesn't match
   -1 if error in pattern */

static char *old_pattern = 0;

int regexp_match (char *pattern, char *string, int match_type)
{
    static regex_t r;
    static int old_type;
    int    rval;

    if (!old_pattern || strcmp (old_pattern, pattern) || old_type != match_type){
	if (old_pattern){
	    regfree (&r);
	    free (old_pattern);
	    old_pattern = 0;
	}
	pattern = convert_pattern (pattern, match_type, 0);
	if (regcomp (&r, pattern, REG_EXTENDED|REG_NOSUB))
	    return -1;
	old_pattern = strdup (pattern);
	old_type = match_type;
    }
    rval = !regexec (&r, string, 0, NULL, 0);
    return rval;
}

int easy_patterns = 1;

static char *maybe_start_group (char *d, int do_group, int *was_wildcard)
{
    if (!do_group)
	return d;
    if (*was_wildcard)
	return d;
    *was_wildcard = 1;
    *d++ = '\\';
    *d++ = '(';
    return d;
}

static char *maybe_end_group (char *d, int do_group, int *was_wildcard)
{
    if (!do_group)
	return d;
    if (!*was_wildcard)
	return d;
    *was_wildcard = 0;
    *d++ = '\\';
    *d++ = ')';
    return d;
}

/* If shell patterns are on converts a shell pattern to a regular
   expression. Called by regexp_match and mask_rename. */
/* Shouldn't we support [a-fw] type wildcards as well ?? */
char *convert_pattern (char *pattern, int match_type, int do_group)
{
    char *s, *d;
    static char new_pattern [100];
    int was_wildcard = 0;

    if (easy_patterns){
	d = new_pattern;
	if (match_type == match_file)
	    *d++ = '^';
	for (s = pattern; *s; s++, d++){
	    switch (*s){
	    case '*':
		d = maybe_start_group (d, do_group, &was_wildcard);
		*d++ = '.';
		*d   = '*';
		break;
		
	    case '?':
		d = maybe_start_group (d, do_group, &was_wildcard);
		*d = '.';
		break;
		
	    case '.':
		d = maybe_end_group (d, do_group, &was_wildcard);
		*d++ = '\\';
		*d   = '.';
		break;

	    default:
		d = maybe_end_group (d, do_group, &was_wildcard);
		*d = *s;
		break;
	    }
	}
	d = maybe_end_group (d, do_group, &was_wildcard);
	if (match_type == match_file)
	    *d++ = '$';
	*d = 0;
	return new_pattern;
    } else
	return pattern;
}



void CSetDisable (const char *ident, int disable)
{
    int i = CLastwidget + 1;
    while (--i)
	if (CW (i))
	    switch (regexp_match ((char *) ident, CW (i)->ident, match_file)) {
	    case 1:
		CW (i)->disabled = disable;
		break;
	    case -1:
		Cfatalerrordialog (20, 20, " Invalid regular expression in call to CDisable() ");
		break;
	    }
}

void CDisable (const char *ident)
{
    if (!ident) {
	if (old_pattern) {
	    free (old_pattern);
	    old_pattern = 0;
	}
    } else
	CSetDisable (ident, 1);
}

void CEnable (const char *ident)
{
    CSetDisable (ident, 0);
}

/* These are to save or restore the present state of the widgets enablement. */
/* You may create or destroy widget between save and restores, but
do not destroy _and_ create, because these won't remember properly which
are have gone and which are new. */

void CBackupState(CState *s)
{
    int i = CLastwidget + 1;
    memset(s, 0, sizeof(CState));
    while (--i) {
	if (CW (i)) {
	    s->mask[i/32] |= (0x1L << (i % 32));
	    if(CW (i)->disabled != 0)
		s->state[i/32] |= (0x1L << (i % 32));
	}
    }
}

void CRestoreState (CState * s)
{
    int i = CLastwidget + 1;
    while (--i)
	if (CW (i))
	    if (((quad_t) s->mask[i / 32] & (0x1L << (i % 32))))
		CW (i)->disabled = ((s->state[i / 32] & (0x1L << (i % 32))) != 0);
}



