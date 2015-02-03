/*****************************************************************************************/
/* simplex.c - simplex optimisation algorithm                                            */
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
#include <math.h>
#include <stdio.h>
#include "matrix.h"

/*
   Finds an n'th dimensional simplex about the origin.
   The n+1 vertices of the simplex are stored as rows
   in the returned matrix. Each vertex is d units
   euclidian distance from the origin.
 */
Matrix *Mageneratesimplex (int n, double d)
{
    Matrix *A = Maallocmatrix (n + 1, n, 'd');
    Matrix *v1 = NULL, *v2 = NULL;
    double s, a, b;
    int i, j;

    d *= d;

    Mard ((*A), 0, 0) = 1;
    Mard ((*A), 1, 0) = -1;

    for (i = 1; i < n; i++) {

	v1 = Masubmatrix (v1, A, i - 1, 0, 1, i);
	v2 = Masubmatrix (v2, A, i, 0, 1, i);

	s = Madistance (v1, v2);
	s *= s;

	b = 0.5 * (s - 2 * d) / (sqrt (s - d));
	a = sqrt (d / (d + b * b));

	Mascale (A, a);

	a *= b;

	for (j = 0; j < i; j++) {
	    Mard ((*A), j, i) = -a;
	    Mard ((*A), i + 1, j) = 0;
	}

	Mard ((*A), i, i) = -a;
	Mard ((*A), i + 1, i) = sqrt (d);

    }

    Mafreematrix (v2);
    Mafreematrix (v1);

    return A;
}

/*
   Same as above, but r (row vector) is added to each vertice (i.e. row).
   Matrix must be Mafreematrix'd
 */
Matrix *Mageneratesimplexabout (Matrix * r, int n, double d)
{
    int i, j;
    Matrix *A;

    A = Mageneratesimplex (n, d);

    for (i = 0; i < n + 1; i++)
	for (j = 0; j < n; j++)
	    Mard ((*A), i, j) += Mard ((*r), 0, j);

    return A;
}

#define NMAX 5000
#define GET_PSUM \
	for(j=0;j<ndim;j++) { \
	    for(sum=0.0,i=0;i<mpts;i++) sum += p[i][j]; \
	    psum[j]=sum; \
	}

int amoeba (double **p, double y[], int ndim, double ftol,
	    double (*funk) (double[]),
	    int (*callback) (double, double, double *, int), int *nfunk)
{
    double amotry (double **p, double y[], double psum[], int ndim,
		   double (*funk) (double[]), int ihi, double fac);
    int i, ihi, ilo, inhi, j, mpts = ndim + 1, ret_val = 0;
    double rtol, sum, ysave, ytry, *psum;

    psum = Mamalloc ((ndim + 2) * sizeof (double));
    *nfunk = 0;
    GET_PSUM;
    for (;;) {
	ilo = 0;
	ihi = y[0] > y[1] ? (inhi = 1, 0) : (inhi = 0, 1);
	for (i = 0; i < mpts; i++) {
	    if (y[i] <= y[ilo])
		ilo = i;
	    if (y[i] > y[ihi]) {
		inhi = ihi;
		ihi = i;
	    } else if (y[i] > y[inhi] && i != ihi)
		inhi = i;
	}

	rtol = 2.0 * fabs (y[ihi] - y[ilo]) / (fabs (y[ihi]) + fabs (y[ilo]));

	if (*nfunk >= NMAX) {
	    fprintf (stderr, "Warning:stereo:%s:%d:" \
     " %d functional evalutations exceeded in optimisation - returning\n",
		     __FILE__, __LINE__, (int) NMAX);
	    goto fin;
	}
	if (rtol < ftol || fabs (y[ilo]) < ftol * ftol)
	    goto fin;
	if (callback) {
	    if ((*callback) (rtol, y[ilo], p[ilo], *nfunk)) {
		ret_val = 1;
	      fin:
		fswap (y[0], y[ilo]);
		for (i = 0; i < ndim; i++)
		    fswap (p[0][i], p[ilo][i]);
		break;
	    }
	}
/*      if (*nfunk >= NMAX)
   Maerror ("NMAX exceeded");
 */
	*nfunk += 2;
	ytry = amotry (p, y, psum, ndim, funk, ihi, -1.0);
	if (ytry <= y[ilo])
	    ytry = amotry (p, y, psum, ndim, funk, ihi, 2.0);
	else if (ytry >= y[inhi]) {
	    ysave = y[ihi];
	    ytry = amotry (p, y, psum, ndim, funk, ihi, 0.5);
	    if (ytry >= ysave) {
		for (i = 0; i < mpts; i++) {
		    if (i != ilo) {
			for (j = 0; j < ndim; j++)
			    p[i][j] = psum[j] = 0.5 * (p[i][j] + p[ilo][j]);
			y[i] = (*funk) (psum);
		    }
		}
		*nfunk += ndim;
		GET_PSUM;
	    }
	} else
	    --(*nfunk);
    }
    free (psum);
    return ret_val;
}

double amotry (double **p, double y[], double psum[], int ndim,
	       double (*funk) (double[]), int ihi, double fac)
{
    int j;
    double fac1, fac2, ytry, *ptry;

    ptry = Mamalloc ((ndim + 2) * sizeof (double));
    fac1 = (1.0 - fac) / ndim;
    fac2 = fac1 - fac;
    for (j = 0; j < ndim; j++)
	ptry[j] = psum[j] * fac1 - p[ihi][j] * fac2;
    ytry = (*funk) (ptry);
    if (ytry < y[ihi]) {
	y[ihi] = ytry;
	for (j = 0; j < ndim; j++) {
	    psum[j] += ptry[j] - p[ihi][j];
	    p[ihi][j] = ptry[j];
	}
    }
    free (ptry);
    return ytry;
}



/*
   Does Nelder and Mead (Downhill Simplex Method) minimisation of
   the function funk.
   returns minimising vector in result. ftol is the final tolerance
   and itol is the initial tolerance (these are actually the size of the
   final or initial simplexes. funk takes an ndim dimensional vector
   of doubles and returns a function value.
   Returns the number of functional iterations performed.
   callback can be used by the user to to intermediate
   processes during the optimisation --- for example monotoring
   by graphical display. A NULL value my be given if you
   have no use for it.
   callback takes four values: the current tolerance, the current error,
   the vertex associated with that error, and the number of times funk
   has been called. It must return 0. A non-zero return value signals
   for the itteration to stop and the current lowest point to be
   returned.
 */

int simplex_optimise (double *result, int ndim, double ftol, double itol,
      double (*funk) (double *), int (*callback) (double, double, double *, int))
{
    Matrix *r = Maallocmatrix (1, ndim, 'd');
    Matrix *start_simplex;
    int stop = 1;

    double *y = Mamalloc ((ndim + 1) * sizeof (double));
    int i, j, nfunk[3] =
    {0, 0, 0};

    memcpy (&(Mard ((*r), 0, 0)), result, ndim * sizeof (double));

    for (j = 0; j < 3 && stop; j++) {	/* restart the algorithm three times to ensure
					   that there is a true minumim */

	start_simplex = Mageneratesimplexabout (r, ndim, itol);

	for (i = 0; i < ndim + 1; i++)
	    y[i] = funk (&(Mard ((*start_simplex), i, 0)));

	if (amoeba ((double **) start_simplex->d, y, ndim,
		    ftol, funk, callback, &(nfunk[j])))
	    stop = 0;

/* amoeba always returns with the first simplex point having least error */
	for (i = 0; i < ndim; i++)
	    Mard ((*r), 0, i) = Mard ((*start_simplex), 0, i);
/* r now contains the point */

	Mafreematrix (start_simplex);	/*Mageneratsimplexabout */

	itol = ftol;
    }

    memcpy (result, &(Mard ((*r), 0, 0)), ndim * sizeof (double));

    free (y);
    Mafreematrix (r);

    return nfunk[0] + nfunk[1] + nfunk[2];
}
