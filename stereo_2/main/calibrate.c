/* \begin{verbatim} */
/*****************************************************************************************/
/* calibrate.c - camera calibration minimisation algorithms                              */
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
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#include "stringtools.h"
#include "app_glob.c"
#include "dirtools.h"

#include "coolwidget.h"
#include "matrix.h"
#include "calibrate.h"
#include "simplex.h"
#include "display.h"
#include "marker.h"
#include "dialog.h"
#include "quickmath.h"
#include "imagefit.h"

#include "mad.h"

extern unsigned char cross_bits[];

#define FTOL 1e-10
#define ROUGH_TOL 1e-7
#define MIN_MARKS_NEEDED_TO_CALIBRATE 4

static int stop = 0;

/* call once with omit passed as NULL before using. This is to initialise */
/* 
   procedure tries all combinarion of leaving out i of the 3D points
   (because not enough markers where specified) plus the points
   leaveout1 and leavout2 (if they are passed as non-negative)
 */
char *recursive_loop (Desktop * d, char omit[], int from, int to, int i, double tol, \
	    int leaveout1, int leaveout2, int calc_distortion)
{
    int j;
    double e;
    static double emin = 10e90;
    static char *omitmin = 0;
    if (!omit) {
	emin = 10e90;
	destroy ((void *) &omitmin);
	return 0;
    }
    if (i) {
	for (j = from; j <= to - i; j++) {
	    if (stop)
		break;
	    omit[j] = 1;
	    recursive_loop (d, omit, j + 1, to, i - 1, tol, leaveout1, leaveout2, \
				calc_distortion);
	    omit[j] = 0;
	}
    } else {
	Vec *v;
	double *x, *y;
	int leave_out = (leaveout1 >= 0) + (leaveout2 >= 0);
	int k;
	int n;
	for (i = 0, j = 0; j < to; i += omit[j++]);
	n = to - i - leave_out;
	v = Cmalloc (sizeof (Vec) * n);
	x = Cmalloc (sizeof (double) * n);
	y = Cmalloc (sizeof (double) * n);
	i = 0;
	k = 0;
	for (j = 0; j < to; j++) {
	    printf ("%d", (int) omit[j]);
	    if (!omit[j]) {
		if (k != leaveout1 && k != leaveout2) {
		    v[i] = d->cal_points[j];
		    x[i] = d->view[d->current_view].mark[k].x;
		    y[i] = d->view[d->current_view].mark[k].y;
		    i++;
		}
		k++;
	    }
	}
	printf ("\n");
	e = findcameraposition (x, y, v, n, \
		&(d->view[d->current_view].cam), tol, 1, calc_distortion);
	if (e < emin) {
	    emin = e;
	    destroy ((void *) &omitmin);
	    omitmin = Cmalloc (to);
	    memcpy (omitmin, omit, to);
	}
	free (x);
	free (y);
    }
    return omitmin;
}


/*
   leave out specifies the number of calibration points to omit when 
   making the optimisation. The routine will try omitting every 
   combination of points (eg. 1st then 2nd, then 1st then 3rd, etc)
   and find which produces the least error. This allows the program
   to cope with one or two calibration points that are eroneous
   ('leave_out' can be only 0, 1 or 2). The routine also tries
   every combination of omition when the number of markers is less
   than the number of calibration points.
   'ce' is not used.
 */
/* returns 0 if succesful, 2 if cal points 'n markers not ok, and 1 if cancelled
   during operation */
int calibrate_view (Desktop * d, CEvent * ce, int leave_out, int calc_distortion)
{
    char *omitted, *omitmin;
    int omit;
    double ermin = 10e90;
    int leaveout1 = -1, leaveout2 = -1;
    int l1 = -1, l2 = -1;

    stop = 0;

    set_current_from_pointer (d, ce);
    omit = d->num_cal_points - d->view[d->current_view].num_marks;

    {
	if (d->num_cal_points - leave_out < MIN_MARKS_NEEDED_TO_CALIBRATE) {
	    Cerrordialog (0, 0, 0, " Calibrate ", " You have at least %d \
		calibration points to calibrate ", MIN_MARKS_NEEDED_TO_CALIBRATE);
	    return 2;
	}
	if (omit < 0) {
	    Cerrordialog (0, 0, 0, " Calibrate ", \
				" You have more markers than calibration points ");
	    return 2;
	}
	if (d->view[d->current_view].num_marks < MIN_MARKS_NEEDED_TO_CALIBRATE) {
	    Cerrordialog (0, 0, 0, " Calibrate ", \
			" You have at least %d markers to calibrate ", \
					MIN_MARKS_NEEDED_TO_CALIBRATE);
	    return 2;
	}
    }

    omitted = Cmalloc (d->num_cal_points);
    memset (omitted, 0, d->num_cal_points);

    recursive_loop (0, 0, 0, 0, 0, 0, 0, 0, 0);		/* reset */

/* try omitting every combination of calibration points without markers and see which 
    gives the minimum error */
/* Here we are not yet leaving out markers: we are first trying to find out which 
    markers correspond to which calibration point */
    printf ("Which markers are which?: Trying all combinations:\n");
    omitmin = recursive_loop (d, omitted, 0, d->num_cal_points, omit, \
			 ROUGH_TOL, -1, -1, calc_distortion);
    if (stop)
	return 1;

/* save the result, there is a 1 in the array if that calibration point is \
	    omitted omitted */
    memcpy (omitted, omitmin, d->num_cal_points);

    printf ("You think that %d calibration points must be scrapped: Trying all \
		combinations:\n", leave_out);

/* now try every combination of leaving out two of the markers 
    (and hence two more calibration points) */
    l1 = l2 = -1;
    if (leave_out == 2) {
	for (leaveout1 = 0; leaveout1 < d->num_cal_points - omit - 1; leaveout1++)
	    for (leaveout2 = leaveout1 + 1; leaveout2 < d->num_cal_points - omit; \
							leaveout2++) {
		recursive_loop (d, omitted, 0, d->num_cal_points, 0, FTOL, \
					 leaveout1, leaveout2, calc_distortion);
		if (d->view[d->current_view].cam.e < ermin) {	/* find the minimum */
		    if (stop)
			return 1;
		    ermin = d->view[d->current_view].cam.e;
		    l1 = leaveout1;
		    l2 = leaveout2;
		}
	    }

	printf ("Best error was in leaving out marker %d and marker %d.\n", \
						    l1 + 1, l2 + 1);

/* or, if specified, ONE of the markers (and hence two more calibration points) */
    } else if (leave_out == 1) {
	leaveout2 = -1;
	for (leaveout1 = 0; leaveout1 < d->num_cal_points - omit; leaveout1++) {
	    recursive_loop (d, omitted, 0, d->num_cal_points, 0, FTOL, leaveout1, \
				 -1, calc_distortion);
	    if (d->view[d->current_view].cam.e < ermin) {	/* find the minimum */
		if (stop)
		    return 1;
		ermin = d->view[d->current_view].cam.e;
		l1 = leaveout1;
	    }
	}

	printf ("Best error was in leaving out marker %d.\n", l1 + 1);

    } else {

	printf ("All markers where used.\n");

    }
    if (stop)
	return 1;

    printf ("Now refine.\n");

    recursive_loop (d, omitted, 0, d->num_cal_points, 0, FTOL, l1, l2, \
			calc_distortion);	/* this is now the minimum */

    recursive_loop (0, 0, 0, 0, 0, 0, 0, 0, 0);		/* reset and free */

    d->view[d->current_view].calibrated = 1;

    return 0;
}


double calerror (double *x, double *y, Vec * v, Camera * c, int n)
{
    int i, j;
    Vec *s;
    double *r;
    double xs, ys, e = 0;
    getrotation (&(c->m_x), &(c->m_s), &(c->m_y), c->phi, c->theta, c->tsi);
    r = Cmalloc ((n * 2) * sizeof (double));
    s = Cmalloc ((n * 2) * sizeof (Vec));
    j = 0;
    for (i = 0; i < n; i++) {
	xs = x[i];
	ys = y[i];
	imagetocamera (c, xs, ys);
	s[j] = plus (times (c->m_s, xs), times (c->m_x, c->f));
	r[j] = dot (s[j], v[i]);
	j++;
	s[j] = plus (times (c->m_s, ys), times (c->m_y, c->f));
	r[j] = dot (s[j], v[i]);
	j++;
    }
    c->x = vec_invert (s, r, j);
    for (i = 0; i < n; i++) {
	phystocamera (c, v[i], xs, ys);
	cameratoimage (c, xs, ys);
	e += fsqr (x[i] - xs) + fsqr (y[i] - ys);
    }
    free (s);
    free (r);
    return e;
}


static double *x_data, *y_data;
static Vec *XYZ_data;
static int num_cal_points;

static Window calprogwin;
static Camera camera;
static Camera fixedvalues;

static XEvent xevent;
static CEvent cwevent;

int callback (double tol, double error, double *v, int num_evals)
{
    static int i = 0;

    i++;
    if (!(i % 10)) {
	Cdrawprogress ("caliprog", calprogwin, 20, 20, 260, 25,
		       (1 - log (tol)) * 65535 / (0.5 - log (FTOL)));
	cwevent.ident = "canccali";
	if (Ccheckifevent (&xevent, &cwevent)) {
	    stop = 1;
	    return 1;
	}
    }
    return 0;
}


double tominimise4 (double *x)
{
    camera.phi = x[0];
    camera.theta = x[1];
    camera.tsi = x[2];
    camera.f = x[3];
    camera.sig = fixedvalues.sig;
    return calerror (x_data, y_data, XYZ_data, &camera, num_cal_points);
}

double tominimise5 (double *x)
{
    camera.phi = x[0];
    camera.theta = x[1];
    camera.tsi = x[2];
    camera.f = x[3];
    camera.sig = x[4];
    return calerror (x_data, y_data, XYZ_data, &camera, num_cal_points);
}

/*
   finds f, phi, theta and tsi for a given set of calibration points
   whose 3D positions are given by v and whose 2D picture postions are given
   by x and y.
   pass calc_distortion with CALC_SIGMA set to optimise sigma, otherwise set sigma
 */
double findcameraposition (double *x, double *y, Vec * v, int n, Camera * c, \
		    double tol, int make_initial_guess, int calc_distortion)
{
    int i, j;
    int numitters;
    double d, result[10], emin = 10e90, e;

    if (!Cwidget ("calprogw")) {
	calprogwin = Cdrawwindow ("calprogw", CMain, 50, 50, 300, 110, "");
	Cwidget ("calprogw")->position = CFIXED_POSITION | CALWAYS_ON_TOP;
	Cdrawbitmapbutton ("canccali", calprogwin, 126, 53,
			   40, 40, Ccolor (18), C_FLAT, cross_bits);
    }
    Cdrawprogress ("caliprog", calprogwin, 20, 20, 260, 25, 0);
    Ccheckifevent (NULL, NULL);

    x_data = x;
    y_data = y;
    XYZ_data = v;
    num_cal_points = n;

    memcpy (&fixedvalues, c, sizeof (Camera));

    if (make_initial_guess) {

/*
   first we find an estimate for the order of magnitude of f.
   The calibration points would be spaced across a reasonable proportion
   of the image. A camera would typically have a field angle of 20 degrees
   and the image extents divided by tan(20) gives a rough estimate of -f
 */
	d = 0;
	for (i = 0; i < n - 1; i++)
	    for (j = i + 1; j < n; j++)
		d = fmax (d, sqrt (fsqr (y[i] - y[j]) + fsqr (x[i] - x[j])));

/*
   we will scale this up a bit to over estimate f. This will
   result in XC being further from the calibration point cluster and
   ensure that it does not penetrate it during the guesstimation procedure
   below.
 */

	memset (c, 0, sizeof (Camera));
	c->f = d * (-4.0);

/*search for an initial guess of the entire space (this is quite fast) */
	for (c->theta = -PI / 2; c->theta <= PI / 2; c->theta += PI / 8) {
	    cwevent.ident = "canccali";
	    if (Ccheckifevent (&xevent, &cwevent)) {
		stop = 1;
		return 1;
	    }
	    for (c->tsi = -PI; c->tsi < PI; c->tsi += PI / 8)
		for (c->phi = -PI; c->phi < PI; c->phi += PI / 8) {
		    e = calerror (x_data, y_data, XYZ_data, c, n);
		    numitters = 0;
		    if (emin > e) {
			emin = e;
			memcpy (&camera, c, sizeof (Camera));
		    }
		}
	}
    } else {			/* initial guess is given */
	memcpy (&camera, c, sizeof (Camera));
    }
    /*
       now we have an estimate for f, theta, and tsi and phi.
       First guess for the other camera parameters is zero (if there are any).
     */
    result[0] = camera.phi;
    result[1] = camera.theta;
    result[2] = camera.tsi;
    result[3] = camera.f;

    if (calc_distortion & CALC_SIGMA) {
	result[4] = 0;
	numitters = simplex_optimise (result, 5, tol, PI / 16, tominimise5, callback);
    } else {
	numitters = simplex_optimise (result, 4, tol, PI / 16, tominimise4, callback);
	result[4] = fixedvalues.sig;
    }

    c->phi = result[0];
    c->theta = result[1];
    c->tsi = result[2];
    c->f = result[3];
    c->sig = result[4];

    getrotation (&(c->m_x), &(c->m_s), &(c->m_y), c->phi, c->theta, c->tsi);

    c->e = calerror (x_data, y_data, XYZ_data, c, n);
    return c->e;
}
