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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "mad.h"

/*Loads a file into memory */
/*Returns the size if the file in filelen and a pointer to the actual file
   which must be free'd. Returns NULL on error. */
/*The returned data is terminated by a null character.*/
char *loadfile (const char *filename, long *filelen)
{
    long filel;
    int file;
    struct stat info;
    char *data;

    if (!filelen)
	filelen = &filel;

    if (stat (filename, &info))
	return NULL;

    if (S_ISDIR (info.st_mode) || S_ISSOCK (info.st_mode)
	|| S_ISFIFO (info.st_mode) || S_ISCHR (info.st_mode)
	|| S_ISBLK (info.st_mode)) {
	return NULL;
    }
    *filelen = info.st_size;
    if ((data = malloc ((*filelen) + 2)) == NULL)
	return NULL;
    if ((file = open (filename, O_RDONLY)) < 0) {
	free (data);
	return NULL;
    }
    if (read (file, data, *filelen) < *filelen) {
	close (file);
	free (data);
	return NULL;
    }
    data[*filelen] = 0;
    close (file);
    return data;
}


int savefile (const char *filename, const char *data, int len, int permissions)
{
    int file;
    if ((file = creat (filename, permissions)) < 0)
	return -1;
    if (write (file, data, len) != len) {
	close (file);
	return -1;
    }
    return close (file);
}
