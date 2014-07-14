/*****************************************************************************************/
/* output.c - this outputs an object (line, circle, etc) to the editor                   */
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
#include "imagefit.h"
#include "app_glob.c"
#include "hugeimage.h"
#include "widget3d.h"
#include "marker.h"
#include "displaycam.h"
#include "picsetup.h"
#include "dialog.h"
#include "stringtools.h"
#include "callback.h"
#include "matrix.h"
#include "edit.h"

/* \end{verbatim} \begin{verbatim} */

int edit_print_string (WEdit * e, const char *s);

void oprint (const char *fmt,...)
{
    char *s;
    va_list ap;
    va_start (ap, fmt);
    s = vsprintf_alloc (fmt, ap);
    edit_print_string (Cwidget ("editor")->editor, s);
    free (s);
    va_end (ap);
}



void print_surface (Surface * s)
{
    int i;
    oprint ("\nsurface %d %d ", s->w, s->h);
    for (i = 0; i < s->w * s->h; i++)
	oprint ("%f %f %f\n", (float) s->p[i].x, (float) s->p[i].y, \
						    (float) s->p[i].z);
    oprint ("\n");
}

void print_circle (Circle * c)
{
    oprint ("\ncircle %f %f %f %f\n", c->p.x, c->p.y, c->p.z, c->r);
}

void print_cylinder (Cylinder * c)
{
    Vec join;
    join = minus (c->l.p2, c->l.p1);
    oprint ("\ncappedcylinder %f %f %f %f %f %f %f\n", \
		    join.x, join.y, join.z, c->l.p1.x, c->l.p1.y, c->l.p1.z, c->r);
}

#define LINE_CYL_RADIUS 100

void print_line (LineSegment * l)
{
    Vec join;
    join = minus (l->p2, l->p1);
    oprint ("\ncappedcylinder %f %f %f %f %f %f %f\n", \
		join.x, join.y, join.z, l->p1.x, l->p1.y, l->p1.z, LINE_CYL_RADIUS);
}

void print_all_object_data (Object * o)
{
    switch (o->type) {
    case NO_TYPE:
	oprint ("# NO_TYPE\n");
    	break;
    case VECTOR:
	oprint ("# p = (%f, %f, %f), direction = (%f, %f, %f)\n", 
	(float) o->vector.p.x, (float) o->vector.p.y, (float) o->vector.p.z,
	(float) o->vector.u.x, (float) o->vector.u.y, (float) o->vector.u.z);
	break;
    case POINT:
	oprint ("# p = (%f, %f, %f)\n", 
	(float) o->point.p.x, (float) o->point.p.y, (float) o->point.p.z);
	break;
    case LINE_SEGMENT:
	oprint ("# p1 = (%f, %f, %f), ",
	(float) o->line.p1.x, (float) o->line.p1.y, (float) o->line.p1.z);
	oprint (" p2 = (%f, %f, %f) \n",
	(float) o->line.p2.x, (float) o->line.p2.y, (float) o->line.p2.z);
	oprint ("# l = %f ", (float) o->line.l);
	oprint (" direction = (%f, %f, %f) \n",
	(float) o->line.u.x, (float) o->line.u.y, (float) o->line.u.z);
	break;
    case CYLINDER:
	oprint ("# p1 = (%f, %f, %f), ", 
	(float) o->cylinder.l.p1.x, (float) o->cylinder.l.p1.y, (float) o->cylinder.l.p1.z);
	oprint (" p2 = (%f, %f, %f) \n",
	(float) o->cylinder.l.p2.x, (float) o->cylinder.l.p2.y, (float) o->cylinder.l.p2.z);
	oprint ("# l = %f ", (float) o->cylinder.l.l);
	oprint (" direction = (%f, %f, %f) \n",
	(float) o->cylinder.l.u.x, (float) o->cylinder.l.u.y, (float) o->cylinder.l.u.z);
	oprint ("# radius = %f \n", (float) o->cylinder.r);
	break;
    case CIRCLE:
	oprint ("# p = (%f, %f, %f), ", 
	(float) o->circle.u.x, (float) o->circle.u.y, (float) o->circle.u.z);
	oprint ("direction = (%f, %f, %f) \n",
	(float) o->circle.p.x, (float) o->circle.p.y, (float) o->circle.p.z);
	oprint ("# radius = %f \n", (float) o->circle.r);
	break;
    case PLANE_SEGMENT:
	break;
    case ELLIPSE:
	break;
    }
}

void output_object (Object * o)
{
    print_all_object_data (o);
    switch (o->type) {
    case VECTOR:
	break;
    case POINT:
	break;
    case LINE_SEGMENT:
	print_line (&(o->line));
	break;
    case CYLINDER:
	print_cylinder (&(o->cylinder));
	break;
    case CIRCLE:
	break;
    case PLANE_SEGMENT:
	break;
    case SURFACE:
	print_surface (&(o->surface));
	break;
    }
}


