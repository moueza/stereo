/*****************************************************************************************/
/* imagefit.c - commands to fit objects to desktop markers                               */
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
#include "main/imagefit.h"
#include "app_glob.c"
#include "hugeimage.h"
#include "widget3d.h"
#include "main/marker.h"
#include "main/displaycam.h"
#include "picsetup.h"
#include "dialog.h"
#include "stringtools.h"
#include "callback.h"
#include "matrix.h"
#include "output.h"

/* \end{verbatim} \begin{verbatim} */

extern Desktop desktop;

Matrix *get_3d_points (Desktop * d)
{
    return 0;
}


#define LOTS 30000

static int get_min_points (Marker * m)
{
    int i, min = LOTS;
    for (i = 0; m[i].v; i++)
	if (m[i].n < min)
	    min = m[i].n;
    return min;
}


/* minimise an overconstrained nx3 matrix */
Vec vec_invert (Vec * s, double *r, int n)
{
    int i;
    double a[4][3] =
    {
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0}};
    if (n < 3) {
	fprintf (stderr, \
	    "stereo:%s:%d: call to vec_invert with less than three rows\n", \
	    __FILE__, __LINE__);
	abort ();
    }
    if (n > 3) {
	for (i = 0; i < n; i++) {	/* s transpose multiplied by s */
	    a[0][0] += s[i].x * s[i].x;
	    a[0][1] += s[i].y * s[i].x;
	    a[0][2] += s[i].z * s[i].x;
	    a[1][0] += s[i].x * s[i].y;
	    a[1][1] += s[i].y * s[i].y;
	    a[1][2] += s[i].z * s[i].y;
	    a[2][0] += s[i].x * s[i].z;
	    a[2][1] += s[i].y * s[i].z;
	    a[2][2] += s[i].z * s[i].z;
	    a[3][0] += r[i] * s[i].x;
	    a[3][1] += r[i] * s[i].y;
	    a[3][2] += r[i] * s[i].z;
	}
	return solve3x3 (a);
    }
    a[0][0] = s[0].x;
    a[0][1] = s[1].x;
    a[0][2] = s[2].x;
    a[1][0] = s[0].y;
    a[1][1] = s[1].y;
    a[1][2] = s[2].y;
    a[2][0] = s[0].z;
    a[2][1] = s[1].z;
    a[2][2] = s[2].z;
    a[3][0] = r[0];
    a[3][1] = r[1];
    a[3][2] = r[2];
    return solve3x3 (a);
}




Vec triangulate_camera_point (double *x, double *y, Camera ** c, int n)
{
/* Here we find the best estimate for the points position: a linear problem. */
    int i, j;
    Vec *s, v;
    double *r;
    r = Cmalloc ((n * 2) * sizeof (double));
    s = Cmalloc ((n * 2) * sizeof (Vec));
    j = 0;
    for (i = 0; i < n; i++) {
	s[j] = plus (times (c[i]->m_s, x[i]), times (c[i]->m_x, c[i]->f));
	r[j] = dot (s[j], c[i]->x);
	j++;
	s[j] = plus (times (c[i]->m_s, y[i]), times (c[i]->m_y, c[i]->f));
	r[j] = dot (s[j], c[i]->x);
	j++;
    }
    v = vec_invert (s, r, j);
    free (s);
    free (r);
    return v;
}



Vec *triangulate_image_points (Marker * m, int *n)
{
    Vec *v;
    int num_sets = 0, i, j;
    double *x, *y;
    Camera **c;
    *n = get_min_points (m);
    while (m[num_sets].v)
	num_sets++;

    c = Cmalloc (num_sets * sizeof (Camera *));
    v = Cmalloc (*n * sizeof (Vec));
    x = Cmalloc (num_sets * sizeof (double));
    y = Cmalloc (num_sets * sizeof (double));

    for (i = 0; i < num_sets; i++)
	c[i] = m[i].cam;

    for (j = 0; j < *n; j++) {
	for (i = 0; i < num_sets; i++) {
	    x[i] = m[i].v[j].x;
	    y[i] = m[i].v[j].y;
	    imagetocamera (c[i], x[i], y[i]);
	}
	v[j] = triangulate_camera_point (x, y, c, num_sets);
    }
    free (y);
    free (x);
    free (c);
    return v;
}


/* alignment **** */
/* this object may be free'd with just free */
Object *get_surface (Marker * m, int w, int h)
{
    Vec *v;
    int n;
    Object *o;

    v = triangulate_image_points (m, &n);
    if (n != w * h) {
	free (v);
	Cerrordialog (0, 0, 0, " Get surface ", \
	    " width * height not the same as the number of points triangulated \n" \
	    " check that you have a representative marker in each image ");
	return 0;
    }
    o = Cmalloc (sizeof (Object) + n * sizeof (Vec));
    o->surface.p = (Vec *) ((char *) o + sizeof (Object));
    memcpy (o->surface.p, v, n * sizeof (Vec));
    free (v);
    o->surface.w = w;
    o->surface.h = h;
    o->type = SURFACE;
    return o;
}

void width_and_height_query (int *w, int *h)
{
    CEvent e;
    Window size_query = Cdrawwindow ("surfsizeQ", CMain, 20, 20, 320, \
							    145, "surfsizeQ");

    Cdrawbitmapbutton ("surfsizeQ.close", size_query, 136, 93,
		       40, 40, Ccolor (6), C_FLAT, tick_bits);

    Cdrawtext ("", size_query, 4, 4, "Enter width\n");
    Cdrawtext ("", size_query, 4, 32, "Enter height\n");
    Cdrawtextinput ("surfsizeQ.width", size_query, 120, 4,
		    50, 20, 20, "");
    Cdrawtextinput ("surfsizeQ.height", size_query, 120, 32,
		    50, 20, 20, "");

    do {
	CNextEvent (NULL, &e);
    } while (strcmp (e.ident, "surfsizeQ.close"));

    *w = atoi (Cgettext ("surfsizeQ.width"));
    *h = atoi (Cgettext ("surfsizeQ.height"));

    Cundrawwidget ("surfsizeQ");
}


Object *fit_surface (Desktop * d)
{
    int w, h;
    Marker *m = 0;
    Object *o;
    if (!check_markers (d, 2, LOTS, 4, LOTS, " Fit Surface "))
	return 0;
    width_and_height_query (&w, &h);
    if ((w | h)) {
	m = get_all_markers (d);
	o = get_surface (m, w, h);
	free (m);
	return o;
    }
    free (m);
    return 0;
}


void output_surface (Desktop * d)
{
    Object *o;
    o = fit_surface (d);
    if (o) {
	output_object (o);
	free (o);
    }
}

Circle circle_from_points (Vec * v, int n)
{
    Circle c;
    double *x, *y, *z;
    int i;
    x = Cmalloc (sizeof (double) * n * 3);
    y = x + n;
    z = y + n;
    for (i = 0; i < n; i++) {
	x[i] = v[i].x;
	y[i] = v[i].y;
	z[i] = v[i].z;
    }

    c = circlefrompoints (x, y, z, n);
    free (x);
    return c;
}

Cylinder cylinder_from_points (Vec * v, int n)
{
    Cylinder c;
    double *x, *y, *z;
    int i;
    x = Cmalloc (sizeof (double) * n * 3);
    y = x + n;
    z = y + n;
    for (i = 0; i < n; i++) {
	x[i] = v[i].x;
	y[i] = v[i].y;
	z[i] = v[i].z;
    }

    c = cylinderfrompoints (x, y, z, n);
    free (x);
    return c;
}


LineSegment line_from_points (Vec * v, int n)
{
    LineSegment l;
    double *x, *y, *z;
    int i;
    x = Cmalloc (sizeof (double) * n * 3);
    y = x + n;
    z = y + n;
    for (i = 0; i < n; i++) {
	x[i] = v[i].x;
	y[i] = v[i].y;
	z[i] = v[i].z;
    }

    l = linesegmentfrompoints (x, y, z, n);
    free (x);
    return l;
}

Ellipse ellipse_from_points (Vec * v, int n)
{
    Ellipse e;
    double *x, *y, *z;
    int i;
    x = Cmalloc (sizeof (double) * n * 3);
    y = x + n;
    z = y + n;
    for (i = 0; i < n; i++) {
	x[i] = v[i].x;
	y[i] = v[i].y;
	z[i] = v[i].z;
    }

    e = ellipsefrompoints (x, y, z, n);
    free (x);
    return e;
}


Object *fit_circle (Desktop * d)
{
    int n;
    Vec *v;
    Marker *m;
    Object *o;
    if (!check_markers (d, 2, LOTS, 3, LOTS, " Fit Circle "))
	return 0;
    m = get_all_markers (d);
    v = triangulate_image_points (m, &n);
    o = Cmalloc (sizeof (Object));
    o->circle = circle_from_points (v, n);
    if (o->type != CIRCLE) {
	free (o);
	o = 0;
    }
    free (v);
    free (m);
    return o;
}


Object *fit_ellipse (Desktop * d)
{
    int n;
    Vec *v;
    Marker *m;
    Object *o;
    if (!check_markers (d, 2, LOTS, 3, LOTS, " Fit Circle "))
	return 0;
    m = get_all_markers (d);
    v = triangulate_image_points (m, &n);
    o = Cmalloc (sizeof (Object));
    o->ellipse = ellipse_from_points (v, n);
    if (o->type != ELLIPSE) {
	free (o);
	o = 0;
    }
    free (v);
    free (m);
    return o;
}

Object *fit_cylinder (Desktop * d)
{
    int n;
    Vec *v;
    Marker *m;
    Object *o;
    if (!check_markers (d, 2, LOTS, 3, LOTS, " Fit Circle "))
	return 0;
    m = get_all_markers (d);
    v = triangulate_image_points (m, &n);
    o = Cmalloc (sizeof (Object));
    o->cylinder = cylinder_from_points (v, n);
    if (o->type != CYLINDER) {
	free (o);
	o = 0;
    }
    free (v);
    free (m);
    return o;
}


void output_circle (Desktop * d)
{
    Object *o;
    o = fit_circle (d);
    if (o) {
	output_object (o);
	free (o);
    }
}


void output_ellipse (Desktop * d)
{
    Object *o;
    o = fit_ellipse (d);
    if (o) {
	output_object (o);
	free (o);
    }
}

/* this object may be free'd with just free */
Object *triangulate_single_point (Desktop *d)
{
    Vec *v;
    Object *o;
    Marker *m;
    int n;

    o = Cmalloc (sizeof (Object));
    if (!check_markers (d, 2, LOTS, 1, 1, " Get Point "))
	return 0;
    m = get_all_markers (d);
    v = triangulate_image_points (m, &n);
    free (m);
    o->point.p = v[0];
    free (v);
    o->type = POINT;
    return o;
}

void output_point (Desktop * d)
{
    Object *o;
    o = triangulate_single_point (d);
    if (o) {
	output_object (o);
	free (o);
    }
}

void output_cylinder (Desktop * d)
{
    Object *o;
    o = fit_cylinder (d);
    if (o) {
	output_object (o);
	free (o);
    }
}


Object *fit_line (Desktop * d)
{
    int n;
    Vec *v;
    Marker *m;
    Object *o;
    if (!check_markers (d, 2, LOTS, 2, LOTS, " Fit Circle "))
	return 0;
    m = get_all_markers (d);
    v = triangulate_image_points (m, &n);
    o = Cmalloc (sizeof (Object));
    o->line = line_from_points (v, n);
    if (o->type != LINE_SEGMENT) {
	free (o);
	o = 0;
    }
    free (v);
    free (m);
    return o;
}


void output_line (Desktop * d)
{
    Object *o;
    o = fit_line (d);
    if (o) {
	output_object (o);
	free (o);
    }
}



static int dividepointsintotwolines (double *x, double *y, \
	int n, double **x1, double **y1, int *n1, double **x2, double **y2, int *n2)
{
    LineSegment l;
    int i = 2;
    Vec v =
    {0, 0, 0};

    *x1 = NULL;
    *y1 = NULL;
    *x2 = NULL;
    *y2 = NULL;
    *n1 = 0;
    *n2 = 0;

    while (i < n) {
	l = linesegmentfrompoints (x, y, NULL, i);
	v.x = x[i];
	v.y = y[i];
	if (distancetoline (v, &l) > DIVIDE_THRESHOLD) {
	    *n1 = i;
	    *x1 = Cmalloc (i * sizeof (double));
	    memcpy (*x1, x, i * sizeof (double));
	    *y1 = Cmalloc (i * sizeof (double));
	    memcpy (*y1, y, i * sizeof (double));
	    *n2 = n - i;
	    *x2 = Cmalloc ((*n2) * sizeof (double));
	    memcpy (*x2, x + i, (*n2) * sizeof (double));
	    *y2 = Cmalloc ((*n2) * sizeof (double));
	    memcpy (*y2, y + i, (*n2) * sizeof (double));
	    return 1;
	}
	i++;
    }
    return 0;
}


/* 
   gets markers from the first two marked images only 
   and returns them as array x1,... length n1, n2. Also removes
   camera distortion
 */
void get_two_sets_of_markers (Marker * m, double **x1, \
			double **y1, int *n1, double **x2, double **y2, int *n2)
{
    int i;
    *n1 = m[0].n;
    *n2 = m[1].n;
    *x1 = Cmalloc (*n1 * sizeof (double));
    *y1 = Cmalloc (*n1 * sizeof (double));
    *x2 = Cmalloc (*n2 * sizeof (double));
    *y2 = Cmalloc (*n2 * sizeof (double));

    for (i = 0; i < *n1; i++) {
	(*x1)[i] = m[0].v[i].x;
	(*y1)[i] = m[0].v[i].y;
	imagetocamera (m[0].cam, (*x1)[i], (*y1)[i]);
    }

    for (i = 0; i < *n2; i++) {
	(*x2)[i] = m[1].v[i].x;
	(*y2)[i] = m[1].v[i].y;
	imagetocamera (m[1].cam, (*x2)[i], (*y2)[i]);
    }
}


Object *fit_edge_line (Desktop * d)
{
    double *x1, *y1, *y2, *x2;
    int n1, n2;
    Marker *m;
    Object *o;

    if (!check_markers (d, 2, 2, 2, LOTS, " Fit Line "))
	return 0;
    o = Cmalloc (sizeof (Object));

    m = get_all_markers (d);
    get_two_sets_of_markers (m, &x1, &y1, &n1, &x2, &y2, &n2);
    o->line = linefromstereoedge (x1, y1, n1, x2, y2, n2, m[0].cam, m[1].cam);
    free (x1);
    free (y1);
    free (x2);
    free (y2);

    if (o->type != LINE_SEGMENT) {
	free (o);
	o = 0;
    }
    free (m);
    return o;
}

Object *fit_edge_circle (Desktop * d)
{
    double *x1, *y1, *y2, *x2;
    int n1, n2;
    Marker *m;
    Object *o;

    if (!check_markers (d, 2, 2, 3, LOTS, " Fit Circle "))
	return 0;
    o = Cmalloc (sizeof (Object));

    m = get_all_markers (d);
    get_two_sets_of_markers (m, &x1, &y1, &n1, &x2, &y2, &n2);
    o->circle = circlefromstereoedge (x1, y1, n1, x2, y2, n2, m[0].cam, m[1].cam);
    free (x1);
    free (y1);
    free (x2);
    free (y2);

    if (o->type != CIRCLE) {
	free (o);
	o = 0;
    }
    free (m);
    return o;
}


Object *fit_edge_cylinder (Desktop * d)
{
    double *xl, *yl, *yr, *xr;
    int nl, nr;
    Marker *m;
    Object *o;
    double *x1l, *y1l, *x2l, *y2l, *x1r, *y1r, *x2r, *y2r;
    int n1l, n2l, n1r, n2r;

    if (!check_markers (d, 2, 2, 2, LOTS, " Fit Circle "))
	return 0;
    o = Cmalloc (sizeof (Object));
    clear (o, Object);

    m = get_all_markers (d);
    get_two_sets_of_markers (m, &xl, &yl, &nl, &xr, &yr, &nr);

    if (dividepointsintotwolines (xl, yl, nl, &x1l, &y1l, &n1l, &x2l, &y2l, &n2l)) {
	if (dividepointsintotwolines (xr, yr, nr, &x1r, &y1r, &n1r, &x2r, &y2r, \
									    &n2r)) {
	    o->cylinder = cylinderfromstereoedge (x1l, y1l, n1l, x2l, y2l, n2l,
		       x1r, y1r, n1r, x2r, y2r, n2r, m[0].cam, m[1].cam);
	    free (x1l);
	    free (y1l);
	    free (x2l);
	    free (y2l);
	}
	free (x1r);
	free (y1r);
	free (x2r);
	free (y2r);
    }
    free (xl);
    free (yl);
    free (xr);
    free (yr);

    if (o->type != CYLINDER) {
	free (o);
	o = 0;
    }
    free (m);
    return o;
}

void output_line_edge (Desktop * d)
{
    Object *o;
    o = fit_edge_line (d);
    if (o) {
	output_object (o);
	free (o);
    }
}

void output_circle_edge (Desktop * d)
{
    Object *o;
    o = fit_edge_circle (d);
    if (o) {
	output_object (o);
	free (o);
    }
}

void output_cylinder_edge (Desktop * d)
{
    Object *o;
    o = fit_edge_cylinder (d);
    if (o) {
	output_object (o);
	free (o);
    }
}
