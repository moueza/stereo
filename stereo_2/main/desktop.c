/*****************************************************************************************/
/* desktop.c - save and restore the desktop to *.dsk files                               */
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

#include "display.h"
#include "widget3d.h"
#include "marker.h"
#include "displaycam.h"
#include "loadcalfile.h"
#include "imagehandler.h"
#include "picsetup.h"
#include "edit.h"
#include "dialog.h"
#include "loadfile.h"

/* \end{verbatim} \begin{verbatim} */

static void strwrite (int f, const char *s)
{
    int len;
    if (!s) {
	len = -1;
	write (f, (char *) &len, sizeof (int));
    } else {
	len = strlen (s);
	write (f, (char *) &len, sizeof (int));
	write (f, s, len);
    }
}

static char *strread (int f)
{
    int len;
    char *s;
    read (f, (char *) &len, sizeof (int));
    if (len < 0) {
	return 0;
    } else {
	s = Cmalloc (len + 1);
	read (f, s, len);
	s[len] = 0;
	return s;
    }
}


int do_save_desktop (Desktop * d, char *filename)
{
    int f;
    if ((f = creat (filename, 0644)) >= 0) {
	int i;
	Desktop save;
	memcpy (&save, d, sizeof (Desktop));
	save.cal_points = 0;
	save.cal_file = 0;
	save.temp_dir = save.image_dir = 0;
	for (i = 0; i < save.num_views; i++) {
	    save.view[i].filename = 0;
	    memset (&save.view[i].pic.main_image, 0, (unsigned long) \
			(&save.view[i].pic.last_pointer) - (unsigned long) \
			(&save.view[i].pic.main_image));
	}
	strwrite (f, "stereo\n - saved desktop\n\n");
	write (f, &save, sizeof (Desktop));
	write (f, d->cal_points, d->num_cal_points * sizeof (Vec));
	strwrite (f, d->cal_file);
	strwrite (f, d->temp_dir);
	strwrite (f, d->image_dir);
	if (d->num_views)
	    for (i = 0; i < d->num_views; i++)
		strwrite (f, d->view[i].filename);
	close (f);
	return 0;
    } else {
	Cerrordialogue (CMain, 20, 20, " Save Desktop ", \
			    get_sys_error (" Error trying to save file. "));
    }
    return 1;
}


int save_desktop (Desktop * d)
{
    char *filename;

    filename = Cgetfile (0, 0, 0, home_dir, "", " Save Desktop ");

    if (filename)
	if (*filename)
	    return do_save_desktop(d, filename);
    return 1;
}

int setup_view (Desktop * d, char *filename, int x, int y, int i);

int do_load_desktop (Desktop * d, char *filename)
{
    int f;
    if ((f = open (filename, O_RDONLY)) >= 0) {
	int i, x = 40, y = 40;
	char *sign;
	sign = strread (f);
	if (strcmp (sign, "stereo\n - saved desktop\n\n")) {
	    close (f);
	    Cerrordialogue (CMain, 20, 20, " Load Desktop ", \
		    " This is not a desktop file ");
	    free (sign);
	    return 1;
	}
	free (sign);
	for (i = 0; i < d->num_views; i++)
	    destroy_view (&(d->view[i]));
	clear (d, Desktop);
	read (f, d, sizeof (Desktop));
	destroy ((void *) &d->cal_points);
	d->cal_points = Cmalloc (d->num_cal_points * sizeof (Vec));
	read (f, d->cal_points, d->num_cal_points * sizeof (Vec));
	d->cal_file = strread (f);
	d->temp_dir = strread (f);
	d->image_dir = strread (f);
	if (d->num_views)
	    for (i = 0; i < d->num_views; i++) {
		char *v;
		v = strread (f);
		if (v) {
		    setup_view (d, v, x += 20, y += 20, i);
		    d->view[i].filename = v;
		}
	    }
	draw_markers (d);
	close (f);
	return 0;
    } else {
	Cerrordialogue (CMain, 20, 20, " Load Desktop ", \
			get_sys_error (" Error trying to save file. "));
    }
    return 1;
}

int load_desktop (Desktop * d)
{
    char *filename;

    filename = Cgetfile (0, 0, 0, home_dir, "", " Load Desktop ");

    if (filename)
	if (*filename)
	    return do_load_desktop (d, filename);
    return 1;
}


