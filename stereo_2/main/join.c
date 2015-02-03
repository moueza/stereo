/*****************************************************************************************/
/* join.c - for a number of grid surfaces, this zips up edges of one                     */
/*          surface that are close to an adjacent surface                                */
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
#include "stdio.h"
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
#include "imagefit.h"

#define NORMAL_LENGTH 256


static inline double normal (Vec x)
{
    double l;
    l = norm (x);
    return l == 0 ? 1e-10 : l;
}

#define norm(x) normal(x)


/*
   This is an isolated file to do two things to an 3D object.

   The first is to join together edges of a surface that are close.
   So if two surfaces are adjacent, but not quite touching, this will
   move the edges so that they touch exactly.

   The other thing it does is to set all the normal vectors perfectly.
   Adjacent surfaces, if their edges have normal that different by less
   than AngleTheshold, then the normals are made the same at the edges.
   This avoid bad shadowing on edges.
 */


struct close_line {
    TD_Surface *s;
    int i, j, dir;
    double d;
};


/* returns 1 if on the line, zero otherwise, [requires p1, u, and l] */
int ispointonline (Vec x, LineSegment * l);

/* [requires l->p1, l->u] */
double distancetoline (Vec x, LineSegment * l);


/*

   width ---->

   		|
   length 	|
   		V

 */

#define DIR_RIGHT 0
#define DIR_LEFT 1
#define DIR_DOWN 2
#define DIR_UP 3

Vec surfpoint (TD_Surface * s, int i, int j)
{
    Vec x;
    TD_Point *p;

    if (i < 0 || i >= s->w || j < 0 || j >= s->l) {
	printf ("surfpoint aborting\n");
	abort ();
    }
    p = s->point + i + j * s->w;
    x.x = p->x;
    x.y = p->y;
    x.z = p->z;
    return x;
}

void insertsurfpoint (Vec x, TD_Surface * s, int i, int j)
{
    TD_Point *p;

    if (i < 0 || i >= s->w || j < 0 || j >= s->l) {
	printf ("insertsurfpoint aborting\n");
	abort ();
    }
    p = s->point + i + j * s->w;
    p->x = x.x;
    p->y = x.y;
    p->z = x.z;
}



LineSegment getlinefromsurface (TD_Surface * s, int i, int j, int dir)
{
    LineSegment l;
    TD_Point *p;

    if (i < 0 || i >= s->w || j < 0 || j >= s->l) {
	printf ("getlinefromsurface aborting\n");
	abort ();
    }
    p = s->point + i + j * s->w;
    l.p1.x = p->x;
    l.p1.y = p->y;
    l.p1.z = p->z;
    switch (dir) {
    case DIR_RIGHT:
	i++;
	p += 1;
	break;
    case DIR_LEFT:
	i--;
	p -= 1;
	break;
    case DIR_DOWN:
	j++;
	p += s->w;
	break;
    case DIR_UP:
	j--;
	p -= s->w;
    }

    if (i < 0 || i >= s->w || j < 0 || j >= s->l) {
	printf ("getlinefromsurface aborting\n");
	abort ();
    }
    l.p2.x = p->x;
    l.p2.y = p->y;
    l.p2.z = p->z;
    l.u = minus (l.p2, l.p1);
    if (norm (l.u) == 0) {
	l.u.x = 10e-10;
	l.u.y = 10e-10;
	l.u.z = 10e-10;
    }
    l.l = norm (l.u);
    l.u = times (l.u, 1 / l.l);
    return l;
}


#define IS_MIN \
    l = getlinefromsurface (s, i, j, dir); \
    if (ispointonline (x, &l)) { \
	d = distancetoline (x, &l); \
	if (d < dmin) { \
	    dmin = d; \
	    imin = i; \
	    jmin = j; \
	    dirmin = dir; \
	} \
    } \


void checksurf (TD_Surface * s, Vec x, struct close_line *cl)
{
    LineSegment l;
    double d, dmin;
    int i, j, dir, dirmin = 0, imin = 0, jmin = 0;

    dmin = 9e90;

    j = 0;
    i = 0;
    dir = DIR_RIGHT;
    for (; i < s->w - 1; i++) {
	l = getlinefromsurface (s, i, j, dir);
	if (ispointonline (x, &l)) {
	    d = distancetoline (x, &l);
	    if (d < dmin) {
		dmin = d;
		imin = i;
		jmin = j;
		dirmin = dir;
	    }
	}
    }

    j = 0;
    i = s->w - 1;
    dir = DIR_DOWN;
    for (; j < s->l - 1; j++) {
	l = getlinefromsurface (s, i, j, dir);
	if (ispointonline (x, &l)) {
	    d = distancetoline (x, &l);
	    if (d < dmin) {
		dmin = d;
		imin = i;
		jmin = j;
		dirmin = dir;
	    }
	}
    }

    j = s->l - 1;
    i = 0;
    dir = DIR_RIGHT;
    for (; i < s->w - 1; i++) {
	l = getlinefromsurface (s, i, j, dir);
	if (ispointonline (x, &l)) {
	    d = distancetoline (x, &l);
	    if (d < dmin) {
		dmin = d;
		imin = i;
		jmin = j;
		dirmin = dir;
	    }
	}
    }

    j = 0;
    i = 0;
    dir = DIR_DOWN;
    for (; j < s->l - 1; j++) {
	l = getlinefromsurface (s, i, j, dir);
	if (ispointonline (x, &l)) {
	    d = distancetoline (x, &l);
	    if (d < dmin) {
		dmin = d;
		imin = i;
		jmin = j;
		dirmin = dir;
	    }
	}
    }

    if (dmin < cl->d) {
	cl->d = dmin;
	cl->dir = dirmin;
	cl->i = imin;
	cl->j = jmin;
	cl->s = s;
    }
}


/* 'omit' is the surface that we must NOT check that x belongs to */
void checkobject (TD_Solid * o, Vec x, int omit, struct close_line *cl)
{
    int i;
    TD_Surface *s;

    for (i = 0; i < o->num_surfaces; i++) {
	s = o->surf + i;
	if (i == omit || !s)
	    continue;
	checksurf (s, x, cl);
    }
}

/*

   #define CHECK_OBJECT \
   cl.s = 0; \
   cl.d = limit; \
   s = o->surf + k; \
   if (s) { \
   checkobject (o, surfpoint (s, i, j), k, &cl); \
   if (cl.s) \
   (*cb) (&cl, s->point + i + j * s->w); \
   }


 */
/*
   this finds the closest line to each point in object 'o' and calls cb for each point.
   The distance to the line must however fall within limit, or else cb won't be called.
   Only lines along edges of surfaces are considered. Each close line and point is
   passed to cn.
 */
void find_closest_line (TD_Solid * o, double limit, \
				void (*cb) (struct close_line *, TD_Point *))
{
    int i, j, k;
    TD_Surface *s;

    struct close_line cl;

    for (k = 0; k < o->num_surfaces; k++) {
	s = o->surf + k;
	if (!s)
	    continue;
	j = 0;
	i = 0;
	for (; i < s->w - 1; i++) {
	    cl.s = 0;
	    cl.d = limit;
	    s = o->surf + k;
	    if (s) {
		checkobject (o, surfpoint (s, i, j), k, &cl);
		if (cl.s)
		    (*cb) (&cl, s->point + i + j * s->w);
	    }
	}
	j = 1;
	i = 0;
	for (; j < s->l - 1; j++) {
	    cl.s = 0;
	    cl.d = limit;
	    s = o->surf + k;
	    if (s) {
		checkobject (o, surfpoint (s, i, j), k, &cl);
		if (cl.s)
		    (*cb) (&cl, s->point + i + j * s->w);
	    }
	}
	j = 0;
	i = s->w - 1;
	for (; j < s->l; j++) {
	    cl.s = 0;
	    cl.d = limit;
	    s = o->surf + k;
	    if (s) {
		checkobject (o, surfpoint (s, i, j), k, &cl);
		if (cl.s)
		    (*cb) (&cl, s->point + i + j * s->w);
	    }
	}
	j = s->l - 1;
	i = 0;
	for (; i < s->w; i++) {
	    cl.s = 0;
	    cl.d = limit;
	    s = o->surf + k;
	    if (s) {
		checkobject (o, surfpoint (s, i, j), k, &cl);
		if (cl.s)
		    (*cb) (&cl, s->point + i + j * s->w);
	    }
	}
    }
}


/* this moves the point half way to the closest line */
void cb_join (struct close_line *cl, TD_Point * p)
{
    LineSegment l;
    Vec x, y;
    x.x = p->x;
    x.y = p->y;
    x.z = p->z;
    l = getlinefromsurface (cl->s, cl->i, cl->j, cl->dir);
    y = pointonline (x, &l);
    x = times (plus (x, y), 0.5);
    p->x = x.x;
    p->y = x.y;
    p->z = x.z;
}

/*
   this takes the point and its closest line from the edge of the nearest surface
   and computes the mean normal. This smooths the rendering.
 */
static double norm_threshold;
void cb_norm (struct close_line *cl, TD_Point * p)
{
    int dir2;
    int d = 0;
    Vec x, y;
    double a;
    if (cl->dir == DIR_LEFT || cl->dir == DIR_RIGHT) {
	if (cl->j)
	    dir2 = DIR_UP;
	else
	    dir2 = DIR_DOWN;
	x = getlinefromsurface (cl->s, cl->i, cl->j, cl->dir).u;
	y = getlinefromsurface (cl->s, cl->i, cl->j, dir2).u;
    } else {
	d = 1;
	if (cl->i)
	    dir2 = DIR_LEFT;
	else
	    dir2 = DIR_RIGHT;
	x = getlinefromsurface (cl->s, cl->i, cl->j, dir2).u;
	y = getlinefromsurface (cl->s, cl->i, cl->j, cl->dir).u;
    }

    if (dir2 + cl->dir == 3)	/* quick way to check clockwise/anti-clockwise */
	y = cross (y, x);
    else
	y = cross (x, y);
    x.x = p->dirx;
    x.y = p->diry;
    x.z = p->dirz;
    x = times (x, 1 / norm (x));
    y = times (y, 1 / norm (y));
    a = acos (norm (minus (x, y)) / 2);
    if (a < norm_threshold) {
	x = plus (x, y);	/* average them */
	x = times (x, NORMAL_LENGTH / norm (x));
						/* normalise to NORMAL_LENGTH */
	p->dirx = x.x;		/* store the point */
	p->diry = x.y;
	p->dirz = x.z;
    }
}

void cb_both (struct close_line *cl, TD_Point * p)
{
    cb_join (cl, p);
    cb_norm (cl, p);
}


/*
   t is the threshold. When surface edge points are less than t away from the closest
   edge, they are deemed to be 'touching', and the surface edges are zipped together.
 */
void join_up_surface_edges (TD_Solid * o, double t)
{
    find_closest_line (o, t, cb_join);
}

/*
   t is the threshold. When surface edge points panels are more than an angle t
   from the panel of the closest edge, they are deemed to be not 'touching',
   and the surface normals are not averaged.
 */
void join_up_normals (TD_Solid * o, double t, double a)
{
    norm_threshold = a;
    find_closest_line (o, t, cb_norm);
}

void join_up_normals_and_surface_edges (TD_Solid * o, double t, double a)
{
    norm_threshold = a;
    find_closest_line (o, t, cb_both);
}

static inline Vec unit (Vec v)
{
    return times (v, norm (v));
}

void calc_all_normals (TD_Solid * o)
{
    Vec n;
    TD_Point *p;
    int i, j, k;
    TD_Surface *s;

    for (k = 0; k < o->num_surfaces; k++) {
	s = o->surf + k;
	if (!s)
	    continue;
	p = s->point;
	for (j = 0; j < s->l; j++) {
	    for (i = 0; i < s->w; i++) {
		if (i < s->w - 1 && j < s->l - 1)
		    n = unit (cross (getlinefromsurface (s, i, j, DIR_RIGHT).u, \
		    getlinefromsurface (s, i, j, DIR_DOWN).u));
		if (i < s->w - 1 && j > 0)
		    n = plus (n, unit (cross ( \
			getlinefromsurface (s, i, j, DIR_UP).u, \
			getlinefromsurface (s, i, j, DIR_RIGHT).u)));
		if (i > 0 && j > 0)
		    n = plus (n, unit (cross ( \
			getlinefromsurface (s, i, j, DIR_LEFT).u, \
			getlinefromsurface (s, i, j, DIR_UP).u)));
		if (i > 0 && j < s->l - 1)
		    n = plus (n, unit (cross ( \
			getlinefromsurface (s, i, j, DIR_DOWN).u, \
			getlinefromsurface (s, i, j, DIR_LEFT).u)));
		n = times (n, NORMAL_LENGTH / norm (n));
		p->dirx = n.x;
		p->diry = n.y;
		(p++)->dirz = n.z;
	    }
	}
    }
}
