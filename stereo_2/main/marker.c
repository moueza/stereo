/*****************************************************************************************/
/* marker.c - marker handling and finding real pointer positions in images               */
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
#include "marker.h"
#include "app_glob.c"
#include "hugeimage.h"
#include "widget3d.h"
#include "main/marker.h"
#include "main/displaycam.h"
#include "picsetup.h"
#include "dialog.h"
#include "stringtools.h"
#include "callback.h"

#define LARGE 10e90

/* \end{verbatim} \begin{verbatim} */

static int dialog_error (const char *head, const char *mess)
{
    Cerrordialog (0, 0, 0, head, mess);
    return 1;
}

int num_views_with_markers (Desktop * d)
{
    int i, v = 0;
    for (i = 0; i < d->num_views; i++)
	if (exists (d, i))
	    if (d->view[i].num_marks)
		v++;
    return v;
}

/* return 0 if not satisifed */
static int markers_in_bounds (Desktop * d, int min_views, int max_views, \
			    int min_markers, int max_markers)
{
    int i, n, v;
    for (i = 0; i < d->num_views; i++) {
	if (exists (d, i)) {
	    n = d->view[i].num_marks;
	    if (n && (n < min_markers || n > max_markers))
		return 0;
	}
    }
    v = num_views_with_markers (d);
    if (v < min_views || v > max_views)
	return 0;
    return 1;
}

/* returns the maximum number of markers in a view */
int max_markers (Desktop * d)
{
    int i, max = 0;
    for (i = 0; i < d->num_views; i++)
	if (exists (d, i))
	    if (d->view[i].num_marks > max)
		max = d->view[i].num_marks;
    return max;
}


/* returned value must be freed with just free */
Marker *get_all_markers (Desktop * d)
{
    int v, i, j;
    Marker *m;

    v = num_views_with_markers (d);
    if (!v)
	return 0;
    m = Cmalloc ((v + 1) * sizeof (Marker));	/* allignment ? ***** */

    i = 0;
    for (j = 0; j < d->num_views; j++)
	if (exists (d, j))
	    if (d->view[j].num_marks) {
		m[i].cam = &d->view[j].cam;
		m[i].v = d->view[j].mark;
		m[i].n = d->view[j].num_marks;
		i++;
	    }
    m[i].v = 0;
    m[i].n = 0;
    return m;
}



/* returns 0 if any are not calibrated */
int images_calibrated (Desktop * d)
{
    int i;
    for (i = 0; i < d->num_views; i++)
	if (exists (d, i))	/* exists */
	    if (d->view[i].num_marks)	/* has markers */
		if (!d->view[i].calibrated)	/* is calibrated */
		    return 0;
    return 1;
}

/* reports an error message if there are not enough markers in enough views */
/* returns 1 if all ok */
int check_markers (Desktop * d, int min_views, int max_views, \
			    int min_markers, int max_markers, const char *head)
{
    if (!d->cal_points) {
	dialog_error (head, " First load calibration points ");
	return 0;
    }
    if (!images_calibrated (d)) {
	dialog_error (head, " Not all images are calibrated ");
	return 0;
    }
    if (!d->num_views) {
	dialog_error (head, " No views open, no markers ");
	return 0;
    }
    if (!markers_in_bounds (d, min_views, max_views, min_markers, max_markers)) {
	char *s;
	s = sprintf_alloc (" Need %d-%d markers in each of %d-%d views ", \
				min_markers, max_markers, min_views, max_views);
	dialog_error (head, s);
	free (s);
	return 0;
    }
    return 1;
}


/* returns 1 on error */
int set_current_view (Desktop * d, int i)
{
    if (i < 0 || i >= d->num_views)
	return 1;
    if (!exists (d, i))
	return 1;
    d->current_view = i;
    return 0;
}

void clear_markers (Desktop * d)
{
    d->view[d->current_view].num_marks = 0;
}

int new_marker (Desktop * d, Vec x)
{
    int i;
    i = d->view[d->current_view].num_marks++;
    d->view[d->current_view].mark[i] = x;
    return i;
}

int remove_last_marker (Desktop * d)
{
    if (d->view[d->current_view].num_marks)
	return --d->view[d->current_view].num_marks;
    return 0;
}

void remove_vec (Vec * vec, int i, int n)
{
    if (i < n && i >= 0 && n > 0)
	memmove (vec + i, vec + i + 1, (n - 1 - i) * sizeof (Vec));
}

void remove_a_marker (Desktop * d, int i)
{
    remove_vec (d->view[d->current_view].mark, i, \
		    d->view[d->current_view].num_marks--);
}

/* find the closest marker to x in the current view and returns its index */
int find_closest_marker (Desktop * d, Vec x)
{
    int i;
    double e = LARGE, f;
    int min = 0;
    if (!d->view[d->current_view].num_marks)
	return -1;
    for (i = 0; i < d->view[d->current_view].num_marks; i++) {
	f = norm (minus (x, d->view[d->current_view].mark[i]));
	if (f < e) {
	    min = i;
	    e = f;
	}
    }
    return min;
}

void remove_closest_marker (Desktop * d, Vec x)
{
    int min;
    min = find_closest_marker (d, x);
    if (min >= 0)
	remove_a_marker (d, min);
}

void move_closest_marker (Desktop * d, Vec x)
{
    int min;
    min = find_closest_marker (d, x);
    if (min >= 0)
	d->view[d->current_view].mark[min] = x;
}

void undrawmarker (View * v)
{
    if (v->pic.main_markers) {
	Csetdrawingtarget (v->pic.main_markers->ident);
	Cclearpic ();
	Csetdrawingtarget (v->pic.zoom_markers->ident);
	Cclearpic ();
    }
}

void draw_line_to_pic (Picture * p, double x1, double y1, \
			double x2, double y2, unsigned long c)
{
    imagetopic (p, x1, y1);
    imagetopic (p, x2, y2);

    Csetdrawingtarget (p->main_markers->ident);
    Cdrawline ((float) x1 * p->width / p->real_width, \
	    (float) y1 * p->width / p->real_width, \
	    (float) x2 * p->width / p->real_width, \
	    (float) y2 * p->width / p->real_width, c);
    Csetdrawingtarget (p->zoom_markers->ident);
    Cdrawline (x1 * p->mag, y1 * p->mag, x2 * p->mag, y2 * p->mag, c);
}

/* draws two crosses, one in the zoom box, and on in the main image */
static void draw_cross_to_pic (Picture * p, Vec x, int r, unsigned long c)
{
    draw_line_to_pic (p, x.x - r, x.y - r, x.x + r, x.y + r, c);
    draw_line_to_pic (p, x.x + r, x.y - r, x.x - r, x.y + r, c);
}

/* draw the markers of one view */
static void drawmarker (View * v)
{
    int i;
    for (i = 0; i < v->num_marks; i++)
	draw_cross_to_pic (&v->pic, v->mark[i], CROSS_SIZE, CROSS_COLOR);
}

/* redraw all the markers */
void draw_markers (Desktop * d)
{
    int i;
    for (i = 0; i < d->num_views; i++) {
	if (exists (d, i)) {
	    undrawmarker (&d->view[i]);
	    drawmarker (&d->view[i]);
	}
    }
}

/* gets the view that the pointer is in. returns -1 if not found */
int get_pointer_view (Desktop * d, CEvent * e)
{
    int i;
    for (i = 0; i < d->num_views; i++) {
	if (exists (d, i)) {
	    if (d->view[i].pic.main_image->winid == e->window)
		return i;
	    if (d->view[i].pic.zoom_image->winid == e->window)
		return i;
	    if (d->view[i].pic.main_win->winid == Cwidget (e->ident)->parentid)
		return i;
	}
    }
    return -1;
}

/* this sets the 'current_view' member to the view the pointer is in */
/* this must be called before an marker operation on a view so that
the operation will occur in the correct view */
/* (otherwise you may get markers appearing in an image other than the
on you clicked on, for example) */
void set_current_from_pointer (Desktop * d, CEvent * e)
{
    int i;
    i = get_pointer_view (d, e);
    if (i >= 0)
	set_current_view (d, i);
}

/* real position in zoom image */
Vec get_zoom_pointer_pos (Picture * p, CEvent * ev)
{
    Vec x;
    x.x = (double) p->xzoom + (double) (ev->x - 1) / p->mag;
    x.y = (double) p->yzoom + (double) (ev->y - 1) / p->mag;;
    x.z = 0;
    return x;
}

/* real position in main image */
Vec get_main_pointer_pos (Picture * p, CEvent * ev)
{
    Vec x;
    x.x = (ev->x - 1) * p->real_width / p->width;
    x.y = (ev->y - 1) * p->real_width / p->width;
    x.z = 0;
    return x;
}

Vec zoom_event_to_image_coord (Picture * p, CEvent * e)
{
    Vec v;
    v.x = (double) p->xzoom + (double) (e->x - 1) / p->mag;
    v.y = (double) p->yzoom + (double) (e->y - 1) / p->mag;
    pictoimage (p, v.x, v.y);
    v.z = 0;
    return v;
}
