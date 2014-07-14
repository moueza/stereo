/*****************************************************************************************/
/* fitline.c - line, circle, ellipse, plane, and cylinder fitting routines               */
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
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "matrix.h"
#include "camera.h"
#include "simplex.h"
#include "fitline.h"
#include "quickmath.h"

/* \end{verbatim} \begin{verbatim} */

/* fast gauss-jordan elimination to diagonalise a 3x3 matrix 
    with partial pivotting */
Vec solve3x3 (double a[4][3])
{
    Vec v =
    {10e20, 10e20, 10e20};
    int i, j, k, max;
    double temp, r;
    for (i = 0; i < 3; i++) {
	max = i;
	for (j = i + 1; j < 3; j++) {
	    if (fabs (a[i][j]) >= fabs (a[i][max]))
		max = j;
	}
	for (k = i; k < 4; k++) {
	    temp = a[k][i];
	    a[k][i] = a[k][max];
	    a[k][max] = temp;
	}
	for (j = i + 1; j < 3; j++) {
	    if (!a[i][i])
		return v;
	    r = a[i][j] / a[i][i];
	    for (k = 3; k >= i; k--)
		a[k][j] -= a[k][i] * r;
	}
    }
    for (i = 2; i > 0; i--) {
	if (!a[i][i])
	    return v;
	r = 1 / a[i][i] * a[3][i];
	for (j = i - 1; j >= 0; j--) {
	    a[3][j] -= a[i][j] * r;
	    a[i][j] = 0;
	}
    }
    if (!a[0][0] || !a[1][1] || !a[2][2])
	return v;
    v.x = a[3][0] / a[0][0];
    v.y = a[3][1] / a[1][1];
    v.z = a[3][2] / a[2][2];
    return v;
}


static inline Vec screen (Camera * cam, Vec p)
{
    Vec v;
    v.z = 0;
    phystocamera (cam, p, v.x, v.y);
    return v;
}


/* fitline fits a 2D line to 'numpoints' points whose x and y values 
   are given as arrays xvals and yvals.

   returns a, b and c of the line
   a*x + b*y = c
   in aa, ab, and ac.

   return rules:
   a^2 + b^2 = 1
   b >= 0

   errors are minimised normal to the line no-matter what the line's
   orientation.
   Fitline returns error.
 */
double fitline (double *xvals, double *yvals, int numpoints, \
	    double *aa, double *ab, double *ac)
{
    double e1, e2, a, b, c, p, q, r, s, t, m1, m2, c1, c2;
    double sumx = 0, sumy = 0, sumxy = 0, sumxx = 0, sumyy = 0;
    int i;

    for (i = 0; i < numpoints; i++) {
	sumx += xvals[i];
	sumy += yvals[i];
	sumxy += xvals[i] * yvals[i];
	sumxx += xvals[i] * xvals[i];
	sumyy += yvals[i] * yvals[i];
    }

/*Need to divide by 'numpoints' because of an algeabraic error. Usually 'numpoints' */
/*would be included in the minimisation expression. */

    p = sumx / numpoints;
    q = sumy / numpoints;
    r = sumxx / numpoints;
    s = sumyy / numpoints;
    t = sumxy / numpoints;

/*coefficents of quadratic in m, (a m^2 + b m + c = 0): */

    a = (double) p *q - t;
    b = (double) p *p - (q * q + r - s);
    c = (double) -a;

    if (fabs (b) < fabs (10000 * a)) {
		/*Indicating a not to steep slope. 
		Otherwise minimisation must be handled differently. */

/*There are two solutions; one for the maximum and one for minimum: */
	m1 = (-b - sqrt (b * b - 4 * a * c)) / (2 * a);
	m2 = (-b + sqrt (b * b - 4 * a * c)) / (2 * a);
	c1 = q - m1 * p;
	c2 = q - m2 * p;

/*To determine the min from the max, check the actually error: */
	e1 = (double) ((double) c1 * c1 + 2 * m1 * c1 * p + \
		    m1 * m1 * r - 2 * c1 * q - 2 * m1 * t + s) / (m1 * m1 + 1);
	e2 = (double) ((double) c2 * c2 + 2 * m2 * c2 * p + \
		    m2 * m2 * r - 2 * c2 * q - 2 * m2 * t + s) / (m2 * m2 + 1);

	if (e1 < e2) {
	    m2 = m1;
	    c2 = c1;
	    e2 = e1;
	}
/*Return coefficients (a*x + b*y = c): */
	if (fabs (m2) > 1) {
	    *ab = (double) 1 / m2;
	    *aa = (double) -1;
	    *ac = (double) c2 / m2;
	} else {
	    *ab = 1;
	    *aa = -m2;
	    *ac = c2;
	}
    } else {

/*For very steep lines (either close to vertical or close to horizontal), */
/*use usual least squares fit, reversing x and y axis if */
/*vertical. */

	if ((sumxx - numpoints * p * p) > (sumyy - numpoints * q * q)) {
		    /* gives an indication of vertical or horizontal. */
	    *ac = ((double) sumxx * sumy - sumxy * sumx) / \
			(numpoints * sumxx - sumx * sumx);
	    *aa = -((double) numpoints * sumxy - sumx * sumy) / \
			(numpoints * sumxx - sumx * sumx);
	    *ab = 1;
	    e2 = sumyy + (*aa) * (*aa) * sumxx + 2 * (*ac) * (*aa) * sumx
		+ (*ac) * (*ac) + 2 * (*aa) * sumxy + 2 * (*ac) * sumy;
	    /*almost horizontal line */
	} else {
	    *ac = ((double) sumyy * sumx - (double) sumxy * sumy) / \
		    ((double) numpoints * sumyy - (double) sumy * sumy);
	    *ab = -((double) numpoints * sumxy - (double) sumy * sumx) / \
		    ((double) numpoints * sumyy - (double) sumy * sumy);
	    *aa = 1;
/*****check this error value as it seldom happens*/
	    e2 = sumxx + (*ab) * (*ab) * sumyy + 2 * (*ac) * (*ab) * sumy
		+ (*ac) * (*ac) + 2 * (*ab) * sumxy + 2 * (*ac) * sumx;
	    /*almost vertical line */
	}
    }

    r = sqrt (sqr (*aa) + sqr (*ab));
    if (*ab < 0)
	r = -r;
    *aa /= r;
    *ab /= r;
    *ac /= r;

    return e2;
}



/*
   Given 3D point coords this calcs the SS error with the least 3D line in
   the direction (u,v,w). Returns the offset of where the first point is
   closest to the line in (a,b,c). (u,v,w) is the vector
   between where the first and last points are closest
   to the line. The first and last points must
   not coincide.
 */

double error3dline (double *xp, double *yp, double *zp,
		    int num_points,
		    double u, double v, double w,
		    double *a, double *b, double *c)
{
    Matrix *A;
    double d = 0, e = 0, f = 0, g = 0, h = 0, i = 0, j = 0, k = 0, l = 0;
    double m, n, o, x, y, z;
    double r;
    int t;

    r = u * u + v * v + w * w;

    if (r) {
	if (num_points > 2) {
	    for (t = 1; t < num_points - 1; t++) {
		d += xp[t] * yp[t];
		e += yp[t] * zp[t];
		f += xp[t] * zp[t];
		g += xp[t];
		h += yp[t];
		i += zp[t];
		j += xp[t] * xp[t];
		k += yp[t] * yp[t];
		l += zp[t] * zp[t];
	    }
	}
	m = xp[num_points - 1];
	n = yp[num_points - 1];
	o = zp[num_points - 1];
	x = xp[0];
	y = yp[0];
	z = zp[0];

	num_points -= 2;
	A = Madoublestomatrix (3, 4, (v * v + w * w) * num_points + 2 * r,
			       -u * v * num_points,
			       -u * w * num_points,
	   h * u * v + i * u * w - g * (v * v + w * w) - (m + x - u) * r,
			       -u * v * num_points,
			       (u * u + w * w) * num_points + 2 * r,
			       -v * w * num_points,
	   g * u * v + i * v * w - h * (u * u + w * w) - (n + y - v) * r,
			       -u * w * num_points,
			       -v * w * num_points,
			       (u * u + v * v) * num_points + 2 * r,
	  g * u * w + h * v * w - i * (u * u + v * v) - (o + z - w) * r);

	if (Madiag (A)) {

	    (*a) = -Mard ((*A), 0, 3);
	    (*b) = -Mard ((*A), 1, 3);
	    (*c) = -Mard ((*A), 2, 3);

	    Mafreematrix (A);

	    e = sqr (u + (*a) - m) + sqr (v + (*b) - n) + sqr (w + (*c) - o)
		+ sqr ((*a) - x) + sqr ((*b) - y) + sqr ((*c) - z)
		- (sqr (u) * j + 2 * u * v * d + 2 * u * w * f - 2 * \
				    u * g * ((*a) * u + (*b) * v + (*c) * w)
		   + sqr (v) * k + 2 * v * w * e - 2 * v * h * \
					    ((*a) * u + (*b) * v + (*c) * w)
	     + sqr (w) * l - 2 * w * i * ((*a) * u + (*b) * v + (*c) * w)
	    + num_points * (pow ((*a) * u + (*b) * v + (*c) * w, 2))) / r
		+ j - 2 * (*a) * g + k - 2 * (*b) * h + l - 2 * (*c) * i
		+ (sqr (*a) + sqr (*b) + sqr (*c)) * num_points;

	    return e;
	}
	Mafreematrix (A);
    }
    fprintf(stderr, "stereo:%s:%d: in call to error3dline()\n", __FILE__, __LINE__);
    return 10e90;
}



static double *x_points;
static double *y_points;
static double *z_points;
static int num_3d_points;
static double c_x, c_y, c_z;


static double tominimise3dline (double *u)
{
    return error3dline (x_points, y_points, z_points, num_3d_points,
			u[0], u[1], u[2], &c_x, &c_y, &c_z);
}


LineSegment linesegment (Vec p1, Vec p2)
{
    LineSegment l;
    memset (&l, 0, sizeof (LineSegment));
    l.p1 = p1;
    l.p2 = p2;
    l.e = 0;
    l.u = minus (p2, p1);
    l.l = norm (l.u);
    l.u = times (l.u, 1 / l.l);
    l.psi = atan2 (l.u.y, l.u.x);
    l.beta = atan2 (l.u.z, sqrt (sqr (l.u.x) + sqr (l.u.y)));
    l.m = times (plus (l.p1, l.p2), 0.5);
    l.a = -l.u.y;
    l.b = l.u.x;
    l.c = l.a * l.p1.x + l.b * l.p1.y;
    l.type = LINE_SEGMENT;
    return l;
}




/* this fits a line to numpoints 2D or 3D points and returns a lineseg structure */
LineSegment linesegmentfrompoints (double *xvals, double *yvals, \
						double *zvals, int numpoints)
{
    LineSegment l;
    int i;
    double vmax = -10e90, vmin = 10e90, len;
    Vec v0, vi;
    memset (&l, 0, sizeof (LineSegment));
    l.n = numpoints;

    if (zvals) {
	double result[3];

	num_3d_points = numpoints;
	x_points = xvals;
	y_points = yvals;
	z_points = zvals;

	result[0] = 1;		/* initial guess */
	result[1] = 1;
	result[2] = 1;

	simplex_optimise (result, 3, 10e-7, 1, tominimise3dline, NULL);

	l.e = tominimise3dline (result);
	l.u.x = result[0];
	l.u.y = result[1];
	l.u.z = result[2];
	l.u = times (l.u, 1 / norm (l.u));
	v0.x = c_x;
	v0.y = c_y;
	v0.z = c_z;
	for (i = 0; i < l.n; i++) {
	    vi.x = xvals[i];
	    vi.y = yvals[i];
	    vi.z = zvals[i];
	    len = dot (l.u, minus (vi, v0));
	    if (len > vmax)
		vmax = len;
	    if (len < vmin)
		vmin = len;
	}
	l.p1 = plus (times (l.u, vmin), v0);
	l.p2 = plus (times (l.u, vmax), v0);
    } else {
	l.e = fitline (xvals, yvals, numpoints, &l.a, &l.b, &l.c);
			/* returns normalised values */
	l.u.x = -l.b;
	l.u.y = l.a;
	v0.x = l.a * l.c;
	v0.y = l.b * l.c;
	v0.z = 0;
/* v0 is now a point on the line */
	vi.z = 0;
	for (i = 0; i < l.n; i++) {
	    vi.x = xvals[i];
	    vi.y = yvals[i];
	    len = dot (l.u, minus (vi, v0));
	    if (len > vmax)
		vmax = len;
	    if (len < vmin)
		vmin = len;
	}
	l.p1 = plus (times (l.u, vmin), v0);
	l.p2 = plus (times (l.u, vmax), v0);
    }
    l.psi = atan2 (l.u.y, l.u.x);
    l.beta = atan2 (l.u.z, sqrt (sqr (l.u.x) + sqr (l.u.y)));
    l.l = norm (minus (l.p2, l.p1));
    l.m = times (plus (l.p1, l.p2), 0.5);
    l.type = LINE_SEGMENT;
    return l;
}


/* [requires l->p1, l->u] */
double distancetoline (Vec x, LineSegment * l)
{
    double r;
    x = minus (x, l->p1);
    r = fsqr (norm (x)) - fsqr (dot (x, l->u));
    if (r <= 0)
	return 0;
    return sqrt (r);
}

#define Distancetoplane(x,u,d) fabs(dot(minus(x, times(u,d)),u))

/*  [requires p->u and p->d] */
double distancetoplane (Vec x, PlaneSegment * p)
{
    return Distancetoplane (x, p->u, p->d);
}


/* [requires p1, u] */
Vec pointonline (Vec x, LineSegment * l)
{
    return plus (l->p1, times (l->u, dot (minus (x, l->p1), l->u)));
}

/* returns the point on the plane closest to x */
/*  [requires p->n and p->d] */
Vec pointonplane (Vec x, PlaneSegment * p)
{
    Vec c = times (p->u, p->d);
    x = minus (x, c);
    return plus (minus (x, times (p->u, dot (x, p->u))), c);
}


/* returns half way between the two lines where they are closest, \
		    [requires p1, p2 and u for both lines] */
Vec interceptionbetweentwolines (LineSegment * l1, LineSegment * l2)
{
    Matrix *A = Madoublestomatrix (2, 3, dot (l1->u, l1->u), \
		-dot (l1->u, l2->u), dot (minus (l2->p1, l1->p1), l1->u),
		dot (l1->u, l2->u), -dot (l2->u, l2->u), \
		dot (minus (l2->p1, l1->p1), l2->u));
    Vec v;
    if (Madiag (A)) {
	v = times (plus (plus (l1->p1, times (l1->u, Mard (*A, 0, 2))),
		    plus (l2->p1, times (l2->u, Mard (*A, 1, 2)))), 0.5);
    } else {
	v = times (plus (plus (l1->p1, l1->p2), plus (l2->p1, l2->p2)), 0.25);
    }
    Mafreematrix (A);
    return v;
}


double shortestdistancebetweentwolines (LineSegment * l1, LineSegment * l2)
{
    Vec c = cross (l1->u, l2->u);
    double n = norm (c);
    if (n) {
	return fabs (dot (minus (l1->p1, l2->p1), times (c, 1 / n)));
    } else {			/* the lines are parallel */
	Vec m = minus (l1->p1, l2->p1);
	return sqrt (fsqr (norm (m)) - fsqr (dot (m, l1->u)));
    }
}



/* returns 1 if on the line, zero otherwise, [requires p1, u, and l] */
int ispointonline (Vec x, LineSegment * l)
{
    if (l->l) {
	double d = dot (minus (x, l->p1), l->u);
	if (d > l->l || d < 0)
	    return 0;
	return 1;
    }
    return 0;
}




/* returns only .p1 and .u of the LineSegement */
LineSegment lineinterceptionoftwoplanes (PlaneSegment * p1, PlaneSegment * p2)
{
    LineSegment l;
    Vec r = cross (p1->u, p2->u);
    Matrix *A;

    memset (&l, 0, sizeof (LineSegment));
    l.u = times (r, 1 / norm (r));
    A = Madoublestomatrix (3, 4, p1->u.x, p1->u.y, p1->u.z, p1->d,
			   p2->u.x, p2->u.y, p2->u.z, p2->d,
			   l.u.x, l.u.y, l.u.z, NULL);
    Madiag (A);
    l.p1.x = Mard (*A, 0, 3);
    l.p1.y = Mard (*A, 1, 3);
    l.p1.z = Mard (*A, 2, 3);
    Mafreematrix (A);
    return l;
}



/* returns only ->c and ->u of the LineSegement */
LineSegment segmentinterceptionoftwoplanes (PlaneSegment * p1, PlaneSegment * p2)
{
    LineSegment l = lineinterceptionoftwoplanes (p1, p2);
    Vec limits[16];
    int nlimits = 0;
    int i, nmax = 0, nmin = 0;
    double vmax = -10e90, vmin = 10e90, len;
    double d;

    for (i = 0; i < p1->num; i++) {
	limits[nlimits] = interceptionbetweentwolines (&l, &p1->l[i]);
	d = shortestdistancebetweentwolines (&l, &p1->l[i]);
	if (ispointonline (limits[nlimits], &p1->l[i]))
	    nlimits++;
    }

    for (i = 0; i < p2->num; i++) {
	limits[nlimits] = interceptionbetweentwolines (&l, &p2->l[i]);
	d = shortestdistancebetweentwolines (&l, &p2->l[i]);
	if (ispointonline (limits[nlimits], &p2->l[i]))
	    nlimits++;
    }

    if (nlimits) {
	for (i = 0; i < nlimits; i++) {
	    len = dot (l.u, minus (l.p1, limits[i]));
	    if (len > vmax) {
		nmax = i;
		vmax = len;
	    }
	    if (len < vmin) {
		nmin = i;
		vmin = len;
	    }
	}
	l.p1 = limits[nmin];
	l.p2 = limits[nmax];
    }
    return linesegment (l.p1, l.p2);
}


/* returns the vector pointing from the camera out to a point */
LineSegment linethroughpointandcamcentre (double x, double y, Camera * cam)
{
    PlaneSegment p1, p2;
    LineSegment l;
    p1.u = plus (times (cam->m_s, x), times (cam->m_x, cam->f));
    p1.d = dot (p1.u, cam->x);
    p2.u = plus (times (cam->m_s, y), times (cam->m_y, cam->f));
    p2.d = dot (p2.u, cam->x);
    l = lineinterceptionoftwoplanes (&p1, &p2);
    l.u = times (l.u, fsgn (dot (cam->m_s, l.u)));
		/* direction is arbitrary, so make it point outward from the camera */
    l.p1 = cam->x;
    l.p2 = plus (l.p1, times (l.u, 10e50));
    return linesegment (l.p1, l.p2);
}



/*
   Calculates:
	[ a*f*m_x + b*f*m_y + c*m_s : (a*f*m_x + b*f*m_y + c*m_s).X_0 ] = [ abc : d ]
   whence
   ax + by + cz = d
   (a,b,c) is a unit vector normal to the plane.
 */
PlaneSegment planethroughlineandcamcentre (LineSegment * l, Camera * cam)
{
    PlaneSegment plane;
    Vec abc;
    double r;
    memset (&plane, 0, sizeof (PlaneSegment));

    abc = plus (plus (times (cam->m_x, cam->f * l->a), \
		times (cam->m_y, cam->f * l->b)), times (cam->m_s, l->c));
    plane.d = dot (abc, cam->x);
    r = norm (abc);
    abc = times (abc, 1 / r);
    plane.u = abc;
    plane.d /= r;

/* now find limits of the plane */
    plane.l[0] = linethroughpointandcamcentre (l->p1.x, l->p1.y, cam);
    plane.l[1] = linethroughpointandcamcentre (l->p2.x, l->p2.y, cam);
    plane.num = 2;

    return plane;
}




LineSegment fit3dlinetoprojections (LineSegment * line1, LineSegment * line2,
				    Camera * cam1, Camera * cam2)
{
    PlaneSegment plane1 = planethroughlineandcamcentre (line1, cam1);
    PlaneSegment plane2 = planethroughlineandcamcentre (line2, cam2);
    return segmentinterceptionoftwoplanes (&plane1, &plane2);
}


/* this undestorts x and y values */
LineSegment linefromstereoedge (double *x1, double *y1, int n1, double *x2, \
				double *y2, int n2, Camera * cam1, Camera * cam2)
{
    LineSegment l1, l2;

    l1 = linesegmentfrompoints (x1, y1, 0, n1);
    l2 = linesegmentfrompoints (x2, y2, 0, n2);

    return fit3dlinetoprojections (&l1, &l2, cam1, cam2);
}






/* how much do the line segments exactly match ? */
double comparelinesegments (LineSegment * l1, LineSegment * l2)
{
    return fmin (
	    norm (minus (l1->p1, l2->p1)) + norm (minus (l1->p2, l2->p2))
		    ,
	    norm (minus (l1->p2, l2->p1)) + norm (minus (l1->p1, l2->p2))
	);
}

/* how much do the line equations exactly match in the region of the lines ? */
double comparelines (LineSegment * l1, LineSegment * l2)
{
    return
	fmax (distancetoline (l1->p1, l2) +
	      distancetoline (l1->p2, l2),
	      distancetoline (l2->p1, l1) +
	      distancetoline (l2->p2, l1));
/* fmax is so that if one of the lines is short, it does not appear
						     to have a small error */
}




void findcylinderedges (Camera * cam, LineSegment * axis, double r, \
			LineSegment * one_edge, LineSegment * other_edge)
{
    double a, b, c, d, e, f;
    double A, B, C;
    Vec r1, r2;
    double alpha1, alpha2;
    Vec p = minus (screen (cam, axis->p2), screen (cam, axis->p1));

    fswap (p.x, p.y);
    p.x = -p.x;		/* p now normal to line of projected axis */

    orth_vectors (axis->u, &r1, &r2, r);

    a = (dot (axis->p1, cam->m_x) - dot (cam->x, cam->m_x)) * p.x + \
	(dot (axis->p1, cam->m_y) - dot (cam->x, cam->m_y)) * p.y;
    b = (dot (r1, cam->m_x)) * p.x + (dot (r1, cam->m_y)) * p.y;
    c = (dot (r2, cam->m_x)) * p.x + (dot (r2, cam->m_y)) * p.y;

    d = dot (axis->p1, cam->m_s) - dot (cam->x, cam->m_s);
    e = dot (r1, cam->m_s);
    f = dot (r2, cam->m_s);

/* (b d - a e) COS (t) + (a f - c d) SIN (t) + b f - c e  =  0 */

    A = a * f - c * d;
    B = b * d - a * e;
    C = b * f - c * e;

    a = sqr (A) + sqr (B);
    b = 2 * B * C;
    c = sqr (C) - sqr (A);
    d = sqrt (sqr (b) - 4 * a * c);
    alpha1 = acos ((-b + d) / (2 * a));
    alpha2 = acos ((-b - d) / (2 * a));

    *one_edge = linesegment (
		screen (cam, plus (plus (times (r1, sin (alpha1)), \
		times (r2, cos (alpha1))), axis->p1)), \
		screen (cam, plus (plus (times (r1, sin (alpha1)), \
		times (r2, cos (alpha1))), axis->p2))); \
    *other_edge = linesegment (
		screen (cam, plus (plus (times (r1, sin (alpha2)), \
		times (r2, cos (alpha2))), axis->p1)), \
		screen (cam, plus (plus (times (r1, sin (alpha2)), \
		times (r2, cos (alpha2))), axis->p2)));
}






Cylinder cylinderfromstereoedge (double *x1l, double *y1l, int n1l, \
	double *x2l, double *y2l, int n2l, double *x1r, double *y1r, \
	int n1r, double *x2r, double *y2r, int n2r, Camera * caml, Camera * camr)
{
    double t, d, dp;
    LineSegment l1r, l2r, l1l, l2l;
    LineSegment ll, lr, edge1l, edge2l, edge1r, edge2r;
    Cylinder cyl;
    double p[4];
    Vec v, u;
    int i;
    double mx, mn;

    l1l = linesegmentfrompoints (x1l, y1l, 0, n1l);
    l2l = linesegmentfrompoints (x2l, y2l, 0, n2l);
    u = times (plus (l1l.u, l2l.u), 0.5);
    u = times (u, 1 / norm (u));

/* is z value zero here ? */
    v = interceptionbetweentwolines (&l1l, &l2l);
    p[0] = dot (u, minus (l1l.p1, v));
    p[1] = dot (u, minus (l1l.p2, v));
    p[2] = dot (u, minus (l2l.p1, v));
    p[3] = dot (u, minus (l2l.p2, v));

    mx = -9e90;
    mn = 9e90;
    for (i = 0; i < 4; i++) {
	if (p[i] > mx)
	    mx = p[i];
	if (p[i] < mn)
	    mn = p[i];
    }
    ll.p1 = plus (v, times (u, mn));
    ll.p2 = plus (v, times (u, mx));

    ll = linesegment (ll.p1, ll.p2);

    l1r = linesegmentfrompoints (x1r, y1r, 0, n1r);
    l2r = linesegmentfrompoints (x2r, y2r, 0, n2r);
    u = times (plus (l1r.u, l2r.u), 0.5);
    u = times (u, 1 / norm (u));

/* is z value zero here ? */
    v = interceptionbetweentwolines (&l1r, &l2r);
    p[0] = dot (u, minus (l1r.p1, v));
    p[1] = dot (u, minus (l1r.p2, v));
    p[2] = dot (u, minus (l2r.p1, v));
    p[3] = dot (u, minus (l2r.p2, v));

    mx = -9e90;
    mn = 9e90;
    for (i = 0; i < 4; i++) {
	if (p[i] > mx)
	    mx = p[i];
	if (p[i] < mn)
	    mn = p[i];
    }
    lr.p1 = plus (v, times (u, mn));
    lr.p2 = plus (v, times (u, mx));

    lr = linesegment (lr.p1, lr.p2);


    cyl.l = fit3dlinetoprojections (&ll, &lr, caml, camr);
    edge1l = fit3dlinetoprojections (&l1r, &l1l, camr, caml);
    cyl.r = distancetoline (edge1l.p1, &cyl.l);		/* estimate */

    t = cyl.r / 10;
    d = 1e90;
    do {
	do {
	    cyl.r += t;
	    dp = d;
	    findcylinderedges (caml, &cyl.l, cyl.r, &edge1l, &edge2l);
	    findcylinderedges (camr, &cyl.l, cyl.r, &edge1r, &edge2r);
	    d = fmin (comparelines (&edge1l, &l1l), comparelines (&edge2l, &l1l));
					/* we don't know which line is which */
	    d += fmin (comparelines (&edge1r, &l1r), comparelines (&edge2r, &l1r));
	} while (d < dp);
	t *= -0.5;
    } while (fabs (t) > 0.00000001);

    cyl.e = l1l.e + l1r.e + l2l.e + l2r.e + d;

    cyl.type = CYLINDER;
    return cyl;
}

double fitcirc (double *xvals, double *yvals, int numpoints, \
			double *ax, double *ay, double *ar);


double circ_x, circ_y, circ_r;
Vec cyl_r1, cyl_r2, cyl_n;

static double tominimise3dcylinder (double *u)
{
    double e, *x, *y;
    Vec v;
    int i;

    cyl_n.x = u[0];
    cyl_n.y = u[1];
    cyl_n.z = u[2];
    if (norm (cyl_n) == 0) {
	cyl_n.z = 1;
    }
    orth_vectors (cyl_n, &cyl_r1, &cyl_r2, 1);

    x = malloc (num_3d_points * sizeof (double));
    y = malloc (num_3d_points * sizeof (double));

    for (i = 0; i < num_3d_points; i++) {
	v.x = x_points[i];
	v.y = y_points[i];
	v.z = z_points[i];
	x[i] = dot (cyl_r1, v);
	y[i] = dot (cyl_r2, v);
    }

    e = fitcirc (x, y, num_3d_points, &circ_x, &circ_y, &circ_r);

    free (y);
    free (x);

    return e;
}





Cylinder cylinderfrompoints (double *xvals, double *yvals, \
						double *zvals, int numpoints)
{
    double result[3];
    Cylinder c;
    Vec v0, vi;
    double len, e, emin = 9e90;
    int i, imin = 0;
    double vmax = -9e90;
    double vmin = 9e90;

    num_3d_points = numpoints;
    x_points = xvals;
    y_points = yvals;
    z_points = zvals;

/* try with various initial guesses */
    for (i = 0; i < 3; i++) {
	result[0] = 0;
	result[1] = 0;
	result[2] = 0;
	result[i] = 1;
	simplex_optimise (result, 3, 10e-3, 1, tominimise3dcylinder, NULL);
	e = tominimise3dcylinder (result);
	if (e < emin) {
	    emin = e;
	    imin = i;
	}
    }

    result[0] = 0;
    result[1] = 0;
    result[2] = 0;
    result[imin] = 1;
    simplex_optimise (result, 3, 10e-7, 1, tominimise3dcylinder, NULL);

    c.e = emin;
    c.r = circ_r;
    cyl_n = times (cyl_n, 1 / norm (cyl_n));
    c.l.u = cyl_n;
    v0 = plus (times (cyl_r1, circ_x), times (cyl_r2, circ_y));

    for (i = 0; i < numpoints; i++) {
	vi.x = xvals[i];
	vi.y = yvals[i];
	vi.z = zvals[i];
	len = dot (c.l.u, minus (vi, v0));
	if (len > vmax)
	    vmax = len;
	if (len < vmin)
	    vmin = len;
    }
    c.l = linesegment (plus (times (c.l.u, vmin), v0), \
						plus (times (c.l.u, vmax), v0));
    c.type = CYLINDER;
    c.l.n = numpoints;
    return c;
}





/* fits a circle numpoints points (xvals, yvals). Places result in (ax,bx) radius r */
double fitcirc (double *xvals, double *yvals, int numpoints, double *ax, \
							double *ay, double *ar)
{
    double s = 0, sx = 0, sy = 0, sxxy = 0, sxyy = 0, sxy = 0, sxx = 0,
     sxxx = 0, syy = 0, syyy = 0, e = 0;
    Matrix *A;
    int i;
    double xs, ys;

    for (i = 0; i < numpoints; i++) {
	s++;
	xs = xvals[i];
	ys = yvals[i];

	sx += xs;
	sy += ys;
	sxy += xs * ys;
	sxxy += ys * xs * xs;
	sxyy += ys * ys * xs;
	sxx += xs * xs;
	sxxx += xs * xs * xs;
	syy += ys * ys;
	syyy += ys * ys * ys;
    }

    A = Madoublestomatrix (3, 4, sxx, sxy, sx, sxxx + sxyy,
			   sxy, syy, sy, syyy + sxxy,
			   sx, sy, s, sxx + syy);

    if (Madiag (A)) {
	*ax = Mard (*A, 0, 3) / 2;
	*ay = Mard (*A, 1, 3) / 2;
	*ar = sqrt (Mard (*A, 2, 3) + *ax * *ax + *ay * *ay);
	for (i = 0; i < numpoints; i++)		/* calculate error */
	    e += fsqr (*ar - sqrt (fsqr (*ax - xvals[i]) + fsqr (*ay - yvals[i])));
    } else
	e = 1e90;		/* singular matrix */

    Mafreematrix (A);
    return e;
}




struct sums {
    double c;
    double d;
    double e;
    double f;
    double g;
    double h;
    double i;
    double j;
    double k;
    double l;
    double m;
    double n;
    double o;
    double p;
};

Vec solve3x3 (double a[4][3]);

double ellipfunc (double a, double b, struct sums *s, \
		  double *xu, double *yu, double *zu)
{
    double matrix[4][3];
    double x, y, z;
    double funcf, funcg;

/* Reduce five non-linear equations to two non-linear equations */
/* by solving for three of them: */
    matrix[0][0] = (8 * pow (a, 4) * s->f + 16 * pow (a, 3) * b * s->e + \
		    sqr (a) * (8 * sqr (b) * s->g + 16 * s->f) + 16 * a * b * s->e + 8 * s->f);
    matrix[1][0] = (8 * pow (a, 3) * b * s->f + sqr (a) * (16 * sqr (b) * \
	s->e + 8 * s->e) + a * (8 * pow (b, 3) * s->g + b * (8 * s->f + \
			     8 * s->g)) + 8 * sqr (b) * s->e + 8 * s->e);
    matrix[2][0] = (-4 * sqr (a) * s->c - 4 * a * b * s->d - 4 * s->c);
    matrix[3][0] = -4 * pow (a, 4) * s->j - 12 * pow (a, 3) * b * s->h \
	+sqr (a) * (-12 * sqr (b) * s->i - 4 * s->i - 8 * s->j) + \
	a * (b * (-12 * s->h - 4 * s->k) - 4 * pow (b, 3) * s->k) - \
	4 * sqr (b) * s->i - 4 * s->i - 4 * s->j;

    matrix[0][1] = (8 * pow (a, 3) * b * s->f + sqr (a) * (16 * sqr (b) * \
	s->e + 8 * s->e) + a * (8 * pow (b, 3) * s->g + b * (8 * s->f + \
			     8 * s->g)) + 8 * sqr (b) * s->e + 8 * s->e);
    matrix[1][1] = (8 * sqr (a) * sqr (b) * s->f + a * (16 * pow (b, 3) * \
	 s->e + 16 * b * s->e) + 8 * pow (b, 4) * s->g + 16 * sqr (b) * \
		    s->g + 8 * s->g);
    matrix[2][1] = (-4 * a * b * s->c - 4 * sqr (b) * s->d - 4 * s->d);
    matrix[3][1] = -4 * pow (a, 3) * b * s->j + sqr (a) * (-12 * sqr (b) \
	    *s->h - 4 * s->h) + a * (b * (-12 * s->i - 4 * s->j) - 12 * \
	   pow (b, 3) * s->i) - 4 * pow (b, 4) * s->k + sqr (b) * (-4 * \
				  s->h - 8 * s->k) - 4 * s->h - 4 * s->k;

    matrix[0][2] = (-4 * sqr (a) * s->c - 4 * a * b * s->d - 4 * s->c);
    matrix[1][2] = (-4 * a * b * s->c - 4 * sqr (b) * s->d - 4 * s->d);
    matrix[2][2] = 2;
    matrix[3][2] = 2 * sqr (a) * s->f + 4 * a * b * s->e + 2 * sqr (b) \
	*s->g + 2 * s->f + 2 * s->g;

    solve3x3 (matrix);
    x = -matrix[3][0];
    y = -matrix[3][1];
    z = -matrix[3][2];

#define SQR(x) ((x)*(x))

    /* Return value of two non-linears: */
    funcf = 16 * SQR (x) * pow (a, 3) * s->f + 24 * SQR (x) * SQR (a) * b * \
	s->e + 8 * SQR (x) * a * SQR (b) * s->g + 16 * SQR (x) * a * s->f + 8 * \
	SQR (x) * b * s->e + 24 * x * y * SQR (a) * b * s->f + 32 * x * y * a * SQR (b) * \
	s->e + 16 * x * y * a * s->e + 8 * x * y * pow (b, 3) * s->g + 8 * x * y * b * s->f \
	+8 * x * y * b * s->g - 8 * x * z * a * s->c - 4 * x * z * b * s->d - 16 * x * pow \
	(a, 3) * s->j - 36 * x * SQR (a) * b * s->h - 24 * x * a * SQR (b) * s->i - 16 * x * \
	a * s->j - 8 * x * a * s->i - 4 * x * pow (b, 3) * s->k - 12 * x * b * s->h - 4 * x * \
	b * s->k + 8 * SQR (y) * a * SQR (b) * s->f + 8 * SQR (y) * pow (b, 3) * s->e + 8 * \
	SQR (y) * b * s->e - 4 * y * z * b * s->c - 12 * y * SQR (a) * b * s->j - 24 * y * a \
	*SQR (b) * s->h - 8 * y * a * s->h - 12 * y * pow (b, 3) * s->i - 4 * y * b * s->j \
	-12 * y * b * s->i + 4 * z * a * s->f + 4 * z * b * s->e + 4 * pow (a, 3) * s->o + \
	12 * SQR (a) * b * s->n + 12 * a * SQR (b) * s->m + 4 * a * s->o + 4 * a * s->m + 4 \
	*pow (b, 3) * s->l + 4 * b * s->n + 4 * b * s->l;
    funcg = 8 * SQR (x) * pow (a, 3) * s->e + 8 * SQR (x) * SQR (a) * b * s->g \
	+8 * SQR (x) * a * s->e + 8 * x * y * pow (a, 3) * s->f + 32 * x * y * SQR (a) * b \
	*s->e + 24 * x * y * a * SQR (b) * s->g + 8 * x * y * a * s->f + 8 * x * y * a * \
	s->g + 16 * x * y * b * s->e - 4 * x * z * a * s->d - 12 * x * pow (a, 3) * s->h - \
	24 * x * SQR (a) * b * s->i - 12 * x * a * SQR (b) * s->k - 12 * x * a * s->h - 4 * \
	x * a * s->k - 8 * x * b * s->i + 8 * SQR (y) * SQR (a) * b * s->f + 24 * SQR (y) \
	*a * SQR (b) * s->e + 8 * SQR (y) * a * s->e + 16 * SQR (y) * pow (b, 3) * s->g \
	+16 * SQR (y) * b * s->g - 4 * y * z * a * s->c - 8 * y * z * b * s->d - 4 * y * \
	pow (a, 3) * s->j - 24 * y * SQR (a) * b * s->h - 36 * y * a * SQR (b) * s->i - \
	4 * y * a * s->j - 12 * y * a * s->i - 16 * y * pow (b, 3) * s->k - 8 * y * b * \
	s->h - 16 * y * b * s->k + 4 * z * a * s->e + 4 * z * b * s->g + 4 * pow (a, 3) \
	*s->n + 12 * SQR (a) * b * s->m + 12 * a * SQR (b) * s->l + 4 * a * s->n + 4 * \
	a * s->l + 4 * pow (b, 3) * s->p + 4 * b * s->m + 4 * b * s->p;

    /* Return other variables: */
    *xu = x;
    *yu = y;
    *zu = z;
    return funcf * funcf + funcg * funcg;
}


double ellipse_x;
double ellipse_y;
double ellipse_z;
struct sums ellipse_sums;

double tominimiseellipse (double *r)
{
    return ellipfunc (r[0], r[1], &ellipse_sums,
		      &ellipse_x, &ellipse_y, &ellipse_z);
}


double fitellip (double *xvals, double *yvals, int numpoints, \
	      double *ax, double *ay, double *aa, double *ab, double *ar)
{
    int i;
    double s = 0, sx = 0, sy = 0, sxxy = 0, sxyy = 0;
    double sxy = 0, sxx = 0, sxxx = 0, syy = 0, syyy = 0;
    double syyyy = 0, sxyyy = 0, sxxyy = 0, sxxxy = 0, sxxxx = 0;
    double result[2];
    struct sums sig;
    float xs, ys;

    for (i = 0; i < numpoints; i++) {
	xs = xvals[i];
	ys = yvals[i];

	s++;
	sx += (double) xs;
	sy += (double) ys;
	sxy += (double) xs *ys;
	sxxy += (double) ys *xs * xs;
	sxyy += (double) ys *ys * xs;
	sxx += (double) xs *xs;
	sxxx += (double) xs *xs * xs;
	syy += (double) ys *ys;
	syyy += (double) ys *ys * ys;
	sxyyy += (double) ys *ys * ys * xs;
	sxxyy += (double) xs *xs * ys * ys;
	sxxxy += (double) xs *xs * xs * ys;
	sxxxx += (double) xs *xs * xs * xs;
	syyyy += (double) ys *ys * ys * ys;
    }

/* Normalise all sums: */
    sig.c = sx / s;
    sig.d = sy / s;
    sig.e = sxy / s;
    sig.f = sxx / s;
    sig.g = syy / s;
    sig.h = sxxy / s;
    sig.i = sxyy / s;
    sig.j = sxxx / s;
    sig.k = syyy / s;
    sig.l = sxyyy / s;
    sig.m = sxxyy / s;
    sig.n = sxxxy / s;
    sig.o = sxxxx / s;
    sig.p = syyyy / s;

    result[0] = 1;
    result[1] = 1;

    simplex_optimise (result, 2, 10e-7, 1, tominimiseellipse, NULL);

    *ax = ellipse_x;
    *ay = ellipse_y;
    *aa = result[0];
    *ab = result[1];
    *ar = sqrt (-ellipse_z + ellipse_x * ellipse_x + ellipse_y * ellipse_y \
	+(result[0] * ellipse_x + result[1] * ellipse_y) * (result[0] * \
				     ellipse_x + result[1] * ellipse_y));
    return tominimiseellipse (result);
}













static double tominimise3dplane (double *u)
{
    Vec n, x;
    double d, e = 0;
    int i;

    n.x = u[0];
    n.y = u[1];
    n.z = u[2];

    d = norm (n);
    n = times (n, 1 / d);
    d = u[3] / d;

    for (i = 0; i < num_3d_points; i++) {
	x.x = x_points[i];
	x.y = y_points[i];
	x.z = z_points[i];
	e += fsqr (Distancetoplane (x, n, d));
    }
    return e;
}






PlaneSegment planefrompoints (double *xvals, double *yvals, \
				    double *zvals, int numpoints)
{
    double result[4];
    PlaneSegment p;
    int i;
    double x = 0, y = 0, z = 0;

    num_3d_points = numpoints;
    x_points = xvals;
    y_points = yvals;
    z_points = zvals;

    result[0] = 1 / sqrt (3);	/* initial guess */
    result[1] = result[0];
    result[2] = result[0];
    result[3] = 0;

    simplex_optimise (result, 4, 10e-7, 1, tominimise3dplane, NULL);

    p.e = tominimise3dplane (result);
    p.u.x = result[0];
    p.u.y = result[1];
    p.u.z = result[2];
    p.u = times (p.u, p.d = 1 / norm (p.u));
    p.d *= result[3];
    p.num = numpoints;
    p.gx = p.u.x / p.u.z;
    p.gy = p.u.y / p.u.z;
    p.elev = sqrt (sqr (p.gx) + sqr (p.gy));
    for (i = 0; i < numpoints; i++) {
	x += xvals[i];
	y += yvals[i];
	z += zvals[i];
    }
    p.c.x = x / numpoints;
    p.c.y = y / numpoints;
    p.c.z = z / numpoints;

    p.c = pointonplane (p.c, &p);
    return p;
}



Circle circlefrompoints (double *xvals, double *yvals, double *zvals, int numpoints)
{
    Circle circ;

    memset (&circ, 0, sizeof (Circle));

    if (zvals) {		/* 3D fit */
	double rx, ry;
	PlaneSegment p;
	double *x = malloc (numpoints * sizeof (double));
	double *y = malloc (numpoints * sizeof (double));
	Vec r1, r2;
	int i;
	p = planefrompoints (xvals, yvals, zvals, numpoints);
	orth_vectors (p.u, &r1, &r2, 1);
	for (i = 0; i < numpoints; i++) {
/* we want to fit a 2D circle to points on the plane. 
					So find positions w.r.t. r1 and r2 */
	    x[i] = (xvals[i] - p.c.x) * r1.x + (yvals[i] - p.c.y) * \
				    r1.y + (zvals[i] - p.c.z) * r1.z;	/* dot */
	    y[i] = (xvals[i] - p.c.x) * r2.x + (yvals[i] - p.c.y) * \
				    r2.y + (zvals[i] - p.c.z) * r2.z;
	}
	circ.e = fitcirc (x, y, numpoints, &rx, &ry, &circ.r) + p.e;
	circ.p = plus (plus (times (r1, rx), times (r2, ry)), p.c);
	circ.u = p.u;
	circ.n = numpoints;
    } else {			/* 2D fit */
	circ.e = fitcirc (xvals, yvals, numpoints, &circ.p.x, &circ.p.y, &circ.r);
	circ.u.z = 1;
	circ.n = numpoints;
    }
    circ.type = CIRCLE;
    return circ;
}


Ellipse ellipsefrompoints (double *xvals, double *yvals, double *zvals, int numpoints)
{
    Ellipse ellip;

    memset (&ellip, 0, sizeof (Ellipse));

    if (zvals) {		/* 3D fit */
	double rx, ry, ra, rb;
	PlaneSegment p;
	double *x = malloc (numpoints * sizeof (double));
	double *y = malloc (numpoints * sizeof (double));
	Vec r1, r2;
	int i;
	p = planefrompoints (xvals, yvals, zvals, numpoints);
	orth_vectors (p.u, &r1, &r2, 1);
	for (i = 0; i < numpoints; i++) {
/* we want to fit a 2D ellipse to points on the plane. 
					So find positions w.r.t. r1 and r2 */
	    x[i] = (xvals[i] - p.c.x) * r1.x + (yvals[i] - p.c.y) * \
				    r1.y + (zvals[i] - p.c.z) * r1.z;	/* dot */
	    y[i] = (xvals[i] - p.c.x) * r2.x + (yvals[i] - p.c.y) * \
				    r2.y + (zvals[i] - p.c.z) * r2.z;
	}
	ellip.e = fitellip (x, y, numpoints, &rx, &ry, &ra, &rb, &ellip.r) + p.e;
	ellip.p = plus (plus (times (r1, rx), times (r2, ry)), p.c);
	ellip.q = plus (plus (times (r1, ra), times (r2, rb)), p.c);
	ellip.u = p.u;
	ellip.n = numpoints;
    } else {			/* 2D fit */
	ellip.e = fitellip (xvals, yvals, numpoints, \
			&ellip.p.x, &ellip.p.y, &ellip.q.x, &ellip.q.y, &ellip.r);
	ellip.u.z = 1;
	ellip.n = numpoints;
    }
    ellip.type = ELLIPSE;
    return ellip;
}


double *xvals1, *yvals1, *xvals2, *yvals2;
Camera *camera1, *camera2;
int num1points, num2points;


double to_minimise_stereo_circle (double *r)
{
    double error;
    double a, b, c, d, e, f, g, h, i, j, k, l;
    int ic;
    Vec u, r1, r2, p;
    double x, y;
    double theta, ex, ey, dtheta, etot, etotp;

    u.x = r[0];
    u.y = r[1];
    u.z = r[2];
    p.x = r[3];
    p.y = r[4];
    p.z = r[5];

    error = 0;

    orth_vectors (u, &r1, &r2, norm (u));

    a = dot (p, camera1->m_x) - dot (camera1->x, camera1->m_x);
    b = dot (r1, camera1->m_x);
    c = dot (r2, camera1->m_x);

    j = d = dot (p, camera1->m_s) - dot (camera1->x, camera1->m_s);
    k = e = dot (r1, camera1->m_s);
    l = f = dot (r2, camera1->m_s);

    g = dot (p, camera1->m_y) - dot (camera1->x, camera1->m_y);
    h = dot (r1, camera1->m_y);
    i = dot (r2, camera1->m_y);

    for (ic = 0; ic < num1points; ic++) {
	x = xvals1[ic];
	y = yvals1[ic];

	dtheta = 20;
	theta = 0;
	etot = 1e90;
	do {
	    do {
		theta += dtheta;
		etotp = etot;
		ex = x + camera1->f * (a + b * sin (theta) + c * cos (theta)) / \
		    (d + e * sin (theta) + f * cos (theta));
		ey = y + camera1->f * (g + h * sin (theta) + i * cos (theta)) / \
		    (j + k * sin (theta) + l * cos (theta));
		etot = ex * ex + ey * ey;
	    } while (etot < etotp);
	    dtheta *= -0.5;
	} while (fabs (dtheta) > 0.00001);

	error += etotp;
    }

    a = dot (p, camera2->m_x) - dot (camera2->x, camera2->m_x);
    b = dot (r1, camera2->m_x);
    c = dot (r2, camera2->m_x);

    j = d = dot (p, camera2->m_s) - dot (camera2->x, camera2->m_s);
    k = e = dot (r1, camera2->m_s);
    l = f = dot (r2, camera2->m_s);

    g = dot (p, camera2->m_y) - dot (camera2->x, camera2->m_y);
    h = dot (r1, camera2->m_y);
    i = dot (r2, camera2->m_y);

    for (ic = 0; ic < num2points; ic++) {
	x = xvals2[ic];
	y = yvals2[ic];

	dtheta = 20;
	theta = 0;
	etot = 1e90;
	do {
	    do {
		theta += dtheta;
		etotp = etot;
		ex = x + camera2->f * (a + b * sin (theta) + c * cos (theta)) / \
		    (d + e * sin (theta) + f * cos (theta));
		ey = y + camera2->f * (g + h * sin (theta) + i * cos (theta)) / \
		    (j + k * sin (theta) + l * cos (theta));
		etot = ex * ex + ey * ey;
	    } while (etot < etotp);
	    dtheta *= -0.5;
	} while (fabs (dtheta) > 0.00001);

	error += etotp;
    }

    return error;
}



Circle circlefromstereoedge (double *x1, double *y1, int n1, \
	double *x2, double *y2, int n2, Camera * cam1, Camera * cam2)
{
    Circle circ;
    double p[6];

/* This is a test only */
    p[0] = 0;
    p[1] = 250;
    p[2] = 0;
    p[3] = 375;
    p[4] = 0;
    p[5] = 375;

    camera1 = cam1;
    camera2 = cam2;
    xvals1 = x1;
    yvals1 = y1;
    xvals2 = x2;
    yvals2 = y2;
    num1points = n1;
    num2points = n2;

    simplex_optimise (p, 6, 10e-7, 1, to_minimise_stereo_circle, NULL);

    circ.u.x = p[0];
    circ.u.y = p[1];
    circ.u.z = p[2];
    circ.r = norm(circ.u);
    circ.u = times (circ.u, 1/circ.r);
    circ.p.x = p[3];
    circ.p.y = p[4];
    circ.p.z = p[5];
    circ.type = CIRCLE;
    circ.e = to_minimise_stereo_circle (p);
    return circ;
}


