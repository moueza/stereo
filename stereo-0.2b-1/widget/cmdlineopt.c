/*
   cmdlineopt.c and cmdlineopt.h are for processing command line options.

   Copyright (C) 1996 the Free Software Foundation

   Authors: 1996 Paul Sheer

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cmdlineopt.h"

#define our_malloc malloc


int get_cmdline_options (int argc, char **argv, struct prog_options *args)
{
    int i, j, c;
    int other = 0;
    for (i = 1; i < argc; i++) {
	if (*argv[i] != '-') {	/* something that is not an option */
	    for (j = 0; args[j].type; j++)
		if (args[j].char_opt == ' ') {
		    args[j].strs[other] = our_malloc (strlen (argv[i]) + 1);
		    strcpy (args[j].strs[other], argv[i]);
		    other++;
		    goto cont;
		}
	    return i;
	}
	c = 0;
	while (++c > 0) {	/* try each letter in a combined option eg 'tar -xvzf' */
	    for (j = 0; args[j].type; j++) {
		if (!strcmp (args[j].long_opt, argv[i]) || !strcmp (args[j].short_opt, argv[i])) {
		    c = -1;	/* not a combined option */
		    goto valid_opt;
		}
		if (argv[i][0] == '-' && argv[i][c] == args[j].char_opt) {
		    if (!argv[i][c + 1])	/* this must be the last letter in the combined option */
			c = -1;
		    goto valid_opt;
		}
		continue;

	      valid_opt:;
		switch (args[j].type) {
		case ARG_SET:{
			int *t;
			t = (int *) args[j].option;
			*t = 1;
			goto next;
		    }
		case ARG_CLEAR:{
			int *t;
			t = (int *) args[j].option;
			*t = 0;
			goto next;
		    }
		case ARG_IGNORE:
		    /* do nothing with this option */
		    goto next;
		}

		if (i + 1 != argc && argv[i + 1]
		    && c < 0	/* must be the last option if a combined option */
		    ) {
		    ++i;
		    switch (args[j].type) {
			int *t;
			double *f;
		    case ARG_ON_OFF:
			if (strcmp (argv[i], "on") == 0) {
			    t = (int *) args[j].option;
			    *t = 1;
			} else if (strcmp (argv[++i], "off") == 0) {
			    t = (int *) args[j].option;
			    *t = 0;
			} else
			    return i;
			goto next;
		    case ARG_YES_NO:
			if (strcmp (argv[i], "yes") == 0) {
			    t = (int *) args[j].option;
			    *t = 1;
			} else if (strcmp (argv[++i], "no") == 0) {
			    t = (int *) args[j].option;
			    *t = 0;
			} else
			    return i;
			goto next;
		    case ARG_STRING:
			*(args[j].str) = our_malloc (strlen (argv[i]) + 1);
			strcpy (*(args[j].str), argv[i]);
			goto next;
		    case ARG_STRINGS:{
			    /* get all argv's after this option until we reach another option */
			    int k = 0;
			    while (i < argc && *argv[i] != '-') {
				args[j].strs[k] = our_malloc (strlen (argv[i]) + 1);
				strcpy (args[j].strs[k], argv[i]);
				k++;
				i++;
			    }
			    i--;	/* will be incremented at end of loop */
			    goto next;
			}
		    case ARG_INT:
			t = (int *) args[j].option;
			*t = atoi (argv[i]);
			goto next;
		    case ARG_DOUBLE:
			f = (double *) args[j].option;
			*f = atof (argv[i]);
			goto next;
		    }
		    i--;
		}
		return i;	/* option parameter not found */
	    }			/* j */
	    return i;		/* option not found */
	  next:;
	}			/* c */
      cont:;
    }
    return 0;
}
