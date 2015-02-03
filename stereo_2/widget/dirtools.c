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

#define MAX_LS_SIZE 65536

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <my_string.h>
#include "stringtools.h"
#include "dirtools.h"
#include <sys/types.h>

#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#include <my_string.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mad.h"

/*should use realloc with error check ***** */
/*Returns a \n seperated list of directories or files in the directory *directory.
   The files are sorted alphabetacally */
/*The list is malloc'ed and must be free'd */
/*if f is '/' then only directories are returned. If f is '*' all files
   are returned. If f is 'f', only files are returned */
char *getfilelist (const char *directory, char f)
{
    struct dirent *directentry;
    struct stat stats;
    DIR *dir;
    char *list = malloc (MAX_LS_SIZE);	/* <--- this is just asking for shit */
    int l, listsize = 0;
    int numentries = 0;
    int p, i, q;
    char *firststr, *secondstr;
    char path_fname[MAX_PATH_LEN];

    if (!list)
	return strdup ("Error: getfilelist, error allocating memory.\n");
    *list = 0;

/*
    if (chdir (directory))
	return strdup ("Error: No such directory.\n");
*/
    if ((dir = opendir (directory)) == NULL)
	return strdup ("Error: Cannot open directory.\n");

    while ((directentry = readdir (dir))) {
	strcpy (path_fname, directory);
	strcat (path_fname, "/");
	strcat (path_fname, directentry->d_name);
	if (!stat (path_fname, &stats)) {
	    if (S_ISDIR (stats.st_mode)) {
		if (f == '/' || f == '*') {
		    l = NAMLEN (directentry);
		    strcpy (list + listsize, "/");
		    strcpy (list + listsize + 1, directentry->d_name);
		    strcpy (list + listsize + l + 1, "\n");
		    listsize += l + 2;
		    numentries++;
		}
	    } else {
		if (f == 'f' || f == '*') {
		    l = NAMLEN (directentry);
		    strcpy (list + listsize, directentry->d_name);
		    strcpy (list + listsize + l, "\n");
		    listsize += l + 1;
		    numentries++;
		}
	    }
	}
    }

/*
   now do a bubble sort on the list.
   (a directory list isn't long enough to warrant
   a quick sort) and the qsort command won't work on unevenly sized entries.
 */
    f = 1;
    if (numentries) {
	while (f) {
	    numentries--;
	    p = 0;
	    f = 0;
	    for (i = 0; i < numentries; i++) {
		q = strmovelines (list, p, 1, 32000);
		if (strcmp (firststr = strline (list, p), secondstr = strline (list, q)) > 0) {
		    strcpy (list + p, secondstr);
		    p += strlen (secondstr);
		    *(list + p++) = '\n';
		    memcpy (list + p, firststr, strlen (firststr));
		    f = 1;
		} else
		    p = q;
	    }
	}

	list[listsize - 1] = 0;	/* remove the last \n */
    }
    closedir (dir);
    return list;
}



