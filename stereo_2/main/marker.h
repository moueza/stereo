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
#ifndef _MARKER_H
#define _MARKER_H

#include "quickmath.h"
#include "display.h"

#define CROSS_SIZE 10
#define CROSS_COLOR Ccolor(6)

/* checks if a view at that index has not been destroyed */
#define exists(d,i) ((d)->view[(i)].filename)

/* returns 0 if any are not calibrated */
int images_calibrated (Desktop * d);

/* reports an error message if there are not enough markers in enough views */
int check_markers (Desktop * d, int min_views, int max_views, int min_markers, \
		    int max_markers, const char *head);

/* returns 1 on error */
int set_current_view (Desktop * d, int i);

void clear_markers (Desktop * d);
int new_marker (Desktop * d, Vec x);
int remove_last_marker (Desktop * d);
void remove_vec (Vec * vec, int i, int n);
void remove_a_marker (Desktop * d, int i);
void remove_closest_marker (Desktop * d, Vec x);
void move_closest_marker (Desktop * d, Vec x);
void undrawmarker (View * v);
void draw_line_to_pic (Picture * p, double x1, double y1, double x2, \
		double y2, unsigned long c);

/* redraw all the markers */
void draw_markers (Desktop * d);

/* gets the view that the pointer is in. returns -1 if not found */
int get_pointer_view (Desktop * d, CEvent * e);

/* this sets the 'current' member to the view the pointer is in */
void set_current_from_pointer (Desktop * d, CEvent * e);

/* real position in zoom image */
Vec get_zoom_pointer_pos (Picture * p, CEvent * ev);

/* real position in main image */
Vec get_main_pointer_pos (Picture * p, CEvent * ev);

/* find the real pos of a mouse click on the zoom box */
Vec zoom_event_to_image_coord (Picture * p, CEvent * e);

/* void imagetopic (Picture *p, int x, int y); */
#define imagetopic(p,x,y) { \
    (x) += (p)->x0; \
    (y) = -(y); \
    (y) += (p)->y0; \
}

#define pictoimage(p,x,y) { \
    (x) -= (p)->x0; \
    (y) -= (p)->y0; \
    (y) = -(y); \
}

int num_views_with_markers (Desktop * d);

struct marker {
    int n;			/* number of markers */
    Vec *v;			/* markers */
    Camera *cam;		/* Camera view of that 2D marker */
};

typedef struct marker Marker;

Marker *get_all_markers (Desktop * d);

#endif	/* _MARKER_H */
