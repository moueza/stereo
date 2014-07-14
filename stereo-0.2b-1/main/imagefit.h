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
#ifndef _IMAGE_FIT
#define _IMAGE_FIT

#define EDITOR Cwidget("editor")->editor
#define DIVIDE_THRESHOLD 10
#define LINE_THICKNES_3D (scale_units_for_3d / 1000)

Vec vec_invert (Vec * s, double *r, int n);

void output_surface (Desktop * d);
void output_circle (Desktop * d);
void output_line (Desktop * d);
void output_line_edge (Desktop * d);
void output_circle_edge (Desktop * d);
void output_cylinder_edge (Desktop * d);
void output_cylinder (Desktop * d);

int cb_getcircle (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getlinefrom3D (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getpoint (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getline (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getcylinder (CWidget * none, XEvent * xevent, CEvent * cwevent);

#endif				/* _IMAGE_FIT */
