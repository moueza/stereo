/*****************************************************************************************/
/* loadcalfile.c - loads a calibration file into the desktop structure                   */
/*****************************************************************************************/
/*
   Copyright (C) 1996, 1997 Paul Sheer

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
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <my_string.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <stdarg.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "regex.h"

#include "display.h"
#include "dialog.h"
#include "my_string.h"
#include "stringtools.h"

#define MAX_CAL_POINTS 256

int load_calibration_file (Desktop * d, const char *filename)
{
    FILE *f;
    int n, i = 0;
    Vec *v;

    f = fopen (filename, "r");
    if (!f) {
	Cerrordialog (0, 0, 0, " Load Calibration File ", \
		    get_sys_error (" Error trying to load file. "));
	return 1;
    }
    v = Cmalloc (MAX_CAL_POINTS * sizeof (Vec));

    while (!feof (f)) {
	n = fscanf (f, "%lf %lf %lf ", &v[i].x, &v[i].y, &v[i].z);
	if (n == 3) {
	    i++;
	} else {
	    Cerrordialog (0, 0, 0, " Load Calibration File ", \
		    " Format error in calibration file ");
	    i = 0;
	    break;
	}
    }
    fclose (f);
    if (i) {
	d->num_cal_points = i;
	destroy ((void *) &(d->cal_points));
	d->cal_points = Cmalloc (i * sizeof (Vec));
	memcpy (d->cal_points, v, i * sizeof (Vec));
	destroy ((void *) &(d->cal_file));
	d->cal_file = strdup (filename);
    }
/* else { cal points not changed */
    destroy ((void *) &v);
 /*   fclose (f);  TLD 2/6/08 */
    return !i;
}

int load_calibration (Desktop * d)
{
    char *s;
    s = Cgetfile (0, 0, 0, d->image_dir, d->cal_file ? d->cal_file : "", \
			    " Load Calibration File ");
    if (s)
	if (*s) {
	    load_calibration_file (d, s);
	    return 0;
	}
    return 1;
}
