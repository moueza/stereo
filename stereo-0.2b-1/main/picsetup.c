/*****************************************************************************************/
/* picsetup.c - setup and destroy pictures and views                                    */
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
#include "app_glob.c"
#include "hugeimage.h"
#include "widget3d.h"
#include "main/marker.h"
#include "main/displaycam.h"
#include "picsetup.h"
#include "dialog.h"
#include "stringtools.h"
#include "callback.h"


/* returns 1 on error */
int setup_picture (const char *ident, Picture * image, int x, int y, \
						    int width, const char *file)
{
    Window win;
    int h;
    image->width = width;
    image->height = 0;
    image->mag = 4;

    win = Cdrawwindow (catstrs (ident, ".main_win", 0), CMain, \
				x, y, image->width + 16, image->width, "");
    image->main_win = Cwidget (catstrs (ident, ".main_win", 0));
    image->main_image = Cdrawhugebwimage (catstrs (ident, ".main_image", 0), \
		win, 6, 6, &(image->width), &(image->height), file);

    if (image->main_image == 0) {
	Cundrawwidget (catstrs (ident, ".main_win", 0));
	memset (image, 0, sizeof (struct picwithzoom));
	return 1;
    }
    Csetwidgetsize (Cwidget (catstrs (ident, ".main_win", 0))->ident, \
			    image->width + 16, h = image->height + 16 + 150);

    image->Ttext = Cdrawtext (catstrs (ident, ".text", 0), win, 10, h - 30, file);
    image->Binfo = Cdrawbutton (catstrs (ident, ".caminfo", 0), win, 10, h - 150, \
				100, 20, " Cam Info ");
    image->Blast = Cdrawbutton (catstrs (ident, ".lastM", 0), win, 120, h - 150, \
				100, 20, " Remove Last ");
    image->Ball = Cdrawbutton (catstrs (ident, ".allM", 0), win, 230, h - 150, \
				100, 20, " Remove All ");
    image->Bcalibrate = Cdrawbutton (catstrs (ident, ".calibrate", 0), win, 10, \
				h - 120, 100, 20, " Calibrate ");
    image->Bkill = Cdrawbutton (catstrs (ident, ".kill", 0), win, \
				120, h - 120, 100, 20, " Kill View ");
    image->Bleave = Cdrawbutton (catstrs (ident, ".leave", 0), win, 10, \
				h - 90, 100, 20, " Leave Out: ");
    image->Tleave = Cdrawtext (catstrs (ident, ".text", 0), win, 120, \
				h - 90, " 0 ");
    image->Ssig = Cdrawbutton (catstrs (ident, ".sig", 0), win, 10, \
				h - 60, 100, 20, " Aspect Rat: ");

    win = Cdrawwindow (catstrs (ident, ".zoom_win", 0), CMain, x, y, \
				ZOOMSIZE + 16, ZOOMSIZE + 16, ".zoom_win");
    image->zoom_win = Cwidget (catstrs (ident, ".zoom_win", 0));
    image->zoom_image = Cdrawzoombox (catstrs (ident, ".zoom_image", 0), \
		    catstrs (ident, ".main_image", 0), win, 6, 6, ZOOMSIZE, \
		    ZOOMSIZE, 0, 0, image->mag);

    Caddcallback (image->main_image->ident, cb_mainimage);
    Caddcallback (image->zoom_image->ident, cb_zoomimage);

    image->main_rect = Cdrawpicture (catstrs (ident, ".main_rect", 0), \
		    image->main_image->winid, 2, 2, 2);
    image->zoom_rect = Cdrawpicture (catstrs (ident, ".zoom_rect", 0), \
		    image->zoom_image->winid, 2, 2, 2);
    image->main_markers = Cdrawpicture (catstrs (ident, ".main_markers", 0), \
		    image->main_image->winid, 2, 2, 1024);
    image->zoom_markers = Cdrawpicture (catstrs (ident, ".zoom_markers", 0), \
		    image->zoom_image->winid, 2, 2, 1024);

    Caddcallback (image->Binfo->ident, cb_draw_cam_data);
    Caddcallback (image->Blast->ident, cb_removelastmarker);
    Caddcallback (image->Ball->ident, cb_clearallmarkers);
    Caddcallback (image->Bcalibrate->ident, cb_calibrate);
    Caddcallback (image->Bleave->ident, cb_leave);
    Caddcallback (image->Ssig->ident, cb_sigma);
    Caddcallback (image->Bkill->ident, cb_killimage);

    image->xzoom = image->yzoom = 0;
    image->real_width = CHugeImageRealWidth (image->main_image->ident);
    image->real_height = CHugeImageRealHeight (image->main_image->ident);
    image->x0 = image->real_width / 2;
    image->y0 = image->real_height / 2;

    return 0;
}



void destroy_pic (Picture * p)
{
    if (p && p->main_image) {
	Cundrawwidget (p->main_markers->ident);
	Cundrawwidget (p->zoom_markers->ident);
	Cundrawwidget (p->main_rect->ident);
	Cundrawwidget (p->zoom_rect->ident);
	Cundrawwidget (p->main_win->ident);
	Cundrawwidget (p->zoom_win->ident);
    }
    clear (p, Picture);
}


void init_view (View * v)
{
    clear (v, View);
}

void destroy_view (View * v)
{
    if (!v->filename)
	return;
    destroy ((void *) &(v->filename));
    destroy_pic (&(v->pic));
    init_view (v);
}

void destroy_current_view (Desktop * d)
{
    if (!d->num_views)
	return;
    destroy_view (&(d->view[d->current_view]));
    if (d->current_view == d->num_views - 1 && d->num_views > 0)
	d->num_views--;
}

int setup_view (Desktop * d, char *filename, int x, int y, int i)
{
    static long count = 0;
    count++;
    clear (&d->view[i].pic, Picture);
    return setup_picture (catstrs ("picture", itoa (count), 0), \
					&(d->view[i].pic), x, y, 380, filename);
}

/* return 1 on error */
int new_view (Desktop * d, char *filename)
{
    int x = 50, y = 50;
    int i = 0;

    if (exists (d, d->current_view)) {
	x = d->view[d->current_view].pic.main_win->x + 20;
	y = d->view[d->current_view].pic.main_win->y + 20;
    }
    if (d->num_views)
	for (i = 0; i < d->num_views; i++)
	    if (!exists (d, i))
		break;
    if (i == d->num_views)
	d->num_views++;
    set_current_view (d, i);

    init_view (&(d->view[i]));
    if (setup_view (d, filename, x, y, i))
	return 1;

    d->view[i].filename = strdup (filename);

    return 0;
}

void load_view (Desktop * d)
{
    char *s;
    s = Cgetfile (0, 0, 0, d->image_dir, "", " Load Image ");
    if (s)
	if (*s)
	    new_view (d, s);
}
