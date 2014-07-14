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
#ifndef FITLINE_H
#define FITLINE_H

#include "quickmath.h"

/* type is one of */
enum {
    NO_TYPE, VECTOR, POINT, LINE_SEGMENT, CYLINDER, CIRCLE, PLANE_SEGMENT, SURFACE, ELLIPSE
};

typedef struct vector {
    int type;
    Vec p;
    Vec u;
} Vector;

typedef struct point {
    int type;
    Vec p;
} Point;

typedef struct lineseg {
    int type;
    Vec p1;			/* begin point */
    Vec p2;			/* end point */
    double l;			/* length x1,y1,z1 -> x2,y2,z2 */
    double psi;			/* psi = atan2(uy, ux) is the azimuth */
    double beta;		/* the elivation = 0 for 2D lines */
    double a, b, c;		/* ax + by = c 2d lines only */
    Vec u;			/* unit vector x1,y1 -> x2,y2 */
    Vec m;			/* position of centre */
    double e;			/* error in fit */
    int n;			/* number of points used in fit */
} LineSegment;

typedef struct cylseg {
    int type;
    LineSegment l;		/* axis */
    double r, e;		/* radius, error */
} Cylinder;

typedef struct circle {
    int type;
    Vec p;			/* centre */
    double r;			/* length x1,y1,z1 -> x2,y2,z2 */
    Vec u;			/* unit vector normal to plane */
    double e;			/* error in fit */
    int n;			/* number of points used in fit */
} Circle;

typedef struct ellipse {
    int type;
    Vec p;			/* centre */
    Vec q;			/* direction of minor axis */
    double r;			/* length x1,y1,z1 -> x2,y2,z2 */
    Vec u;			/* unit vector normal to plane */
    double e;			/* error in fit */
    int n;			/* number of points used in fit */
} Ellipse;


/* dot(n, (x,y,z)) = d */
typedef struct planeseg {
    int type;
    LineSegment l[8];		/* border */
    int num;			/* number of LineSegments used to border */
    Vec u;			/* normal (unit vector) */
    double d;			/* least distance from origin */
    Vec c;			/* centre point */
    double elev;		/* max elivation gradient */
    double gx, gy;		/* gradient in the x and y directions */
    double e;
    int n;
} PlaneSegment;

typedef struct surface {
    int type;
    int w, h;
    Vec *p;
} Surface;

typedef union object {
    int type;
    Point point;
    Vector vector;
    LineSegment line;
    Cylinder cylinder;
    Circle circle;
    Ellipse ellipse;
    PlaneSegment plane;
    Surface surface;
} Object;


Vec solve3x3 (double a[4][3]);

double fitline (double *xvals, double *yvals, int numpoints, double *aa, \
	    double *ab, double *ac);
LineSegment linesegment (Vec p1, Vec p2);
LineSegment linesegmentfrompoints (double *xvals, double *yvals, \
		double *zvals, int numpoints);
double distancetoline (Vec x, LineSegment * l);
Vec pointonline (Vec x, LineSegment * l);
Vec interceptionbetweentwolines (LineSegment * l1, LineSegment * l2);
int ispointonline (Vec x, LineSegment * l);
LineSegment lineinterceptionoftwoplanes (PlaneSegment * p1, PlaneSegment * p2);
LineSegment segmentinterceptionoftwoplanes (PlaneSegment * p1, PlaneSegment * p2);
LineSegment linethroughpointandcamcentre (double x, double y, Camera * cam);
PlaneSegment planethroughlineandcamcentre (LineSegment * l, Camera * cam);
LineSegment fit3dlinetoprojections (LineSegment * line1, LineSegment * line2,
				    Camera * cam1, Camera * cam2);
LineSegment linefromstereoedge (double *x1, double *y1, int n1, double *x2, \
			    double *y2, int n2, Camera * cam1, Camera * cam2);
void linetest3d (void);
void get_perp_vec (Vec X, Vec * r1, Vec * r2, double r);
double comparelinesegments (LineSegment * l1, LineSegment * l2);
double comparelines (LineSegment * l1, LineSegment * l2);
void findcylinderedges (Camera * c, LineSegment * axis, double r, \
			LineSegment * one_edge, LineSegment * other_edge);
Cylinder cylinderfromstereoedge (double *x1l, double *y1l, int n1l, \
		double *x2l, double *y2l, int n2l, double *x1r, \
		double *y1r, int n1r, double *x2r, double *y2r, int n2r, \
		Camera * camr, Camera * caml);
Circle circlefrompoints (double *xvals, double *yvals, double *zvals, int numpoints);
Cylinder cylinderfrompoints (double *xvals, double *yvals, double *zvals, \
	int numpoints);
Ellipse ellipsefrompoints (double *xvals, double *yvals, double *zvals, int numpoints);
Circle circlefromstereoedge (double *x1, double *y1, int n1, \
	double *x2, double *y2, int n2, Camera * cam1, Camera * cam2);

#endif
