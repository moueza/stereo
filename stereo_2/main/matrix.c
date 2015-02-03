/*****************************************************************************************/
/* matrix.c - a matrix manipulation library                                              */
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
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "matrix.h"
#include "string.h"
#include "stringtools.h"

#include "mad.h"


/*
   Rules:

   (1) Always set matrix '->d' to NULL if you are going to 
   allow some of the routines to auto initialise the matrix
   and you intend passing the matrix to the routine
   without it ever having been initialised

   eg.  C = Maadd(NULL, A, B);  /+is OK +/
       C = Maadd (C, A, B); /+ is fine provided C is either NULL or has been
       previously 
       Maallocmatrix'd or has C->d = NULL +/
       Madd (NULL, A, B);      /+ NEVER, 'cos you won't be able to free the

   allocated matrix if you don't store the
   returned value. +/

   (2)
 */

static long total_elements = 0;

long Magettotalelements ()
{
    return total_elements;
}

int Macheckmatrixbounds (int rows, int columns, int j, int i, \
		const char *file, int line)
{
    if (j >= rows || i >= columns || j < 0 || i < 0) {
	printf ( \
	"Fatal error:\ntrying to access outside matrix bounds, %s:%d\n", file, line);
	abort ();
    }
    return 0;
}

void Maerror (const char *e)
{
    fprintf (stderr, e);
    exit (1);
}

/*
   This sets the size datatype and data of a matrix that has not
   had data allocated to it. Low level function.
 */
void Maset (Matrix * A, int rows, int columns, int datatype, void **d)
{
    A->rows = rows;
    A->columns = columns;
    A->datatype = 'd';
    A->d = d;
}

void *Mamalloc (size_t size)
{
    void *p;
    if ((p = malloc (size)) == NULL)
	Maerror ("Cannot allocate memory for matrix operation.\n");
    return p;
}

/*
   Sets all the elements of a matrix to zero.
 */


/*
   void *Macalloc (size_t nmemb, size_t size)
   {
   void *p;
   if ((p = calloc (nmemb, size)) == NULL)
   Maerror ("Cannot allocate memory for matrix operation.\n");
   return p;
   }
 */

/*rotates in following order:
   phi about x, theta about y, tsi about z
   for an aeroplane:
   +x = toward nose along fuselage
   +y = along right wing
   +z = downward
   rotation is roll then pitch and then yaw.

   for a camera instead of a plane.
   +x = East
   +y = North
   +z = Upward
   rotation is roll about optical axis, elevation and then pan.

   Hence with no rotation, x points into the image, y horizontal to the left
   and z is vertical down (for camera view or forward view
   from cockpit window). This an inconvinient definition, 
   see below for 3dtocreen.

   Usually a camera is not rotated, only panned and elevated up and down.
   Hence the first rotation angle is close to zero.
 */



Matrix *Magetrotationclassic (Matrix * m, double phi, double theta, double tsi)
{
    m = Mareinit (m, 3, 3, 'd');

    Mard ((*m), 0, 0) = cos (theta) * cos (tsi);
    Mard ((*m), 0, 1) = cos (theta) * sin (tsi);
    Mard ((*m), 0, 2) = -sin (theta);

    Mard ((*m), 1, 0) = sin (phi) * sin (theta) * cos (tsi) - cos (phi) * sin (tsi);
    Mard ((*m), 1, 1) = sin (phi) * sin (theta) * sin (tsi) + cos (phi) * cos (tsi);
    Mard ((*m), 1, 2) = sin (phi) * cos (theta);

    Mard ((*m), 2, 0) = cos (phi) * sin (theta) * cos (tsi) + sin (phi) * sin (tsi);
    Mard ((*m), 2, 1) = cos (phi) * sin (theta) * sin (tsi) - sin (phi) * cos (tsi);
    Mard ((*m), 2, 2) = cos (phi) * cos (theta);

    return m;
}


Matrix *Magetrotation (Matrix * m, double phi, double theta, double tsi)
{
    m = Mareinit (m, 3, 3, 'd');

    Mard ((*m), 1, 1) = cos (theta) * cos (tsi);
    Mard ((*m), 1, 0) = cos (theta) * sin (tsi);
    Mard ((*m), 1, 2) = -sin (theta);

    Mard ((*m), 0, 1) = sin (phi) * sin (theta) * cos (tsi) - cos (phi) * sin (tsi);
    Mard ((*m), 0, 0) = sin (phi) * sin (theta) * sin (tsi) + cos (phi) * cos (tsi);
    Mard ((*m), 0, 2) = sin (phi) * cos (theta);

    Mard ((*m), 2, 1) = cos (phi) * sin (theta) * cos (tsi) + sin (phi) * sin (tsi);
    Mard ((*m), 2, 0) = cos (phi) * sin (theta) * sin (tsi) - sin (phi) * cos (tsi);
    Mard ((*m), 2, 2) = cos (phi) * cos (theta);

    return m;
}


void getrotation (Vec * m0, Vec * m1, Vec * m2, double phi, double theta, double tsi)
{
    m1->y = cos (theta) * cos (tsi);
    m1->x = cos (theta) * sin (tsi);
    m1->z = -sin (theta);

    m0->y = sin (phi) * sin (theta) * cos (tsi) - cos (phi) * sin (tsi);
    m0->x = sin (phi) * sin (theta) * sin (tsi) + cos (phi) * cos (tsi);
    m0->z = sin (phi) * cos (theta);

    m2->y = cos (phi) * sin (theta) * cos (tsi) + sin (phi) * sin (tsi);
    m2->x = cos (phi) * sin (theta) * sin (tsi) - sin (phi) * cos (tsi);
    m2->z = cos (phi) * cos (theta);
}


/*calculates C =  AB
 */
Matrix *Mamultiply (Matrix * C, Matrix * A, Matrix * B)
{
    int i, j, k;
    int n = B->columns, m = A->rows, p;
    double sum;

    if ((p = A->columns) != B->rows) {
	printf ("rows = %d, columns = %d and rows = %d, columns = %d.\n",
		A->rows, A->columns, B->rows, B->columns);
	Maerror ("trying to multiply matrices of incompatable size.\n");
    }
    C = Mareinit (C, m, n, 'd');

    for (i = 0; i < n; i++)
	for (j = 0; j < m; j++) {
	    sum = 0;
	    for (k = 0; k < p; k++)
		sum += Mard ((*A), j, k) * Mard ((*B), k, i);
	    Mard ((*C), j, i) = sum;
	}
    return C;
}

/*calculates C = A+B
 */
Matrix *Maadd (Matrix * C, Matrix * A, Matrix * B)
{
    int i, j;
    int n = A->columns, m = A->rows;

    if (n != B->columns || m != B->rows) {
	printf ("rows = %d, columns = %d and rows = %d, columns = %d.\n",
		A->rows, A->columns, B->rows, B->columns);
	Maerror ("trying to add matrices of incompatable size.\n");
    }
    C = Mareinit (C, m, n, 'd');

    for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
	    Mard ((*C), j, i) = Mard ((*A), j, i) + Mard ((*B), j, i);

    return C;
}

/*calculates C = A-B
 */
Matrix *Masubtract (Matrix * C, Matrix * A, Matrix * B)
{
    int i, j;
    int n = A->columns, m = A->rows;

    if (n != B->columns || m != B->rows) {
	printf ("rows = %d, columns = %d and rows = %d, columns = %d.\n",
		A->rows, A->columns, B->rows, B->columns);
	Maerror ("trying to subtract matrices of incompatable size.\n");
    }
    C = Mareinit (C, m, n, 'd');

    for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
	    Mard ((*C), j, i) = Mard ((*A), j, i) - Mard ((*B), j, i);
    return C;
}


/*
   For a camera view, with no rotation, this is routine effectively

   phi and theta. With no rotation y is into the screen
   x is to the right ans z upward.
   Increasing theta rotates the view. Increasing phi tilts the view upward
   (scrolls downward), and increasing tsi panns to the left (scrolls right).

   This calculates the screen postion of a 3D position vector in space.
   m is rotation matrix, v is 3D position vector, V is camera position vector,
   f is focal length.

   returns
   x = -f(m_row1(v + V))/(m_row2(v + V))
   y = -f(m_row3(v + V))/(m_row2(v + V))
   return z = m_row2(v + V);

   z(into the screen) and is y before rotation,
   x(screen position) is x before rotation, and
   y(screen position) is z before rotation.

   "As if" the screen was the front windscreen of the aeroplane,
   or the camera was pointing north.

   Don't forget that y might have to be changed to -y for
   a raster display with the origin at top-left.
 */

double Ma3dtoscreen (Matrix * m, Matrix * v, Matrix * V, double f, \
		double *x, double *y)
{
    Matrix *X = NULL;
    Matrix *C = NULL;
    Matrix *D = NULL;
    double *q;

    C = Masubtract (C, v, V);

    if (C->rows == 1) {
	D = Matranspose (NULL, C);
	Mafreematrix (C);
	C = D;
    }
    X = Mamultiply (X, m, C);

    q = &(Mard ((*X), 0, 0));

    if (q[1] <= 0)
	q[1] = -1e-10;

    *x = -f * q[0] / q[1];
    *y = -f * q[2] / q[1];

    Mafreematrix (C);
    Mafreematrix (X);

    return q[1];
}

Vec Mamatrixtovec (Matrix * V)
{
    double *p = &Mard (*V, 0, 0);
    Vec v;
    v.x = *p;
    v.y = *(p + 1);
    v.z = *(p + 2);
    return v;
}

/* returns a column vector */
Matrix *Mavectomatrix (Vec v)
{
    return Madoublestomatrix (1, 3, v.x, v.y, v.z);
}

/*
   inline Vec times3x3(Matrix *m, Vec c)
   {
   Vec v;
   v.x = Mard(*m, 0, 0) * c.x + Mard(*m, 0, 1) * c.y + Mard(*m, 0, 2) * c.z;
   v.y = Mard(*m, 1, 0) * c.x + Mard(*m, 1, 1) * c.y + Mard(*m, 1, 2) * c.z;
   v.z = Mard(*m, 2, 0) * c.x + Mard(*m, 2, 1) * c.y + Mard(*m, 2, 2) * c.z;
   return v;
   }
 */




/*returns the sum of all the element of A squared */
double Manormal (Matrix * A)
{
    int i, j;
    double sum = 0;

    for (i = 0; i < A->columns; i++)
	for (j = 0; j < A->rows; j++)
	    sum += pow (Mard ((*A), j, i), 2);
    return sum;
}



/*performs guass jordan elimination (with partial pivoting)
   to diagonalise an augmented square matrix.
   matrix must be of size n+1 columns by n rows.
   The results are hence left in the last column of
   C, and the rest of C is an identity matrix. */

Matrix *Madiag (Matrix * C)
{
    int i, j, k, matrixsize, max;
    double temp;

    if (C->rows + 1 != C->columns)
	Maerror ("Matrix not square augmented in call to Madiag.\n");

    matrixsize = C->rows;

/*Guass-Jordan elimination with partial pivotting: */

    for (i = 0; i < matrixsize; i++) {
	max = i;
/*Pivot: */
	for (j = i + 1; j < matrixsize; j++) {
	    if (fabs (Mard ((*C), j, i)) >= fabs (Mard ((*C), max, i)))
		max = j;
	}
	for (k = i; k < matrixsize + 1; k++) {
	    temp = Mard ((*C), i, k);
	    Mard ((*C), i, k) = Mard ((*C), max, k);
	    Mard ((*C), max, k) = temp;
	}
/*Resolve into uppertriangular form: */
	for (j = i + 1; j < matrixsize; j++)
	    for (k = matrixsize; k >= i; k--) {
#ifdef CHECK_MATRIX_SINGULAR
		if (Mard ((*C), i, i) == 0)
		    return NULL;
/*                  Maerror ("Matrix singular in call to Madiag\n"); */
#endif
		Mard ((*C), j, k) -= Mard ((*C), i, k) * Mard ((*C), j, i) / \
								    Mard ((*C), i, i);
	    }
    }

/*Perform back substitution: */
    for (i = matrixsize - 1; i > 0; i--)
	for (j = i - 1; j >= 0; j--) {
#ifdef CHECK_MATRIX_SINGULAR
	    if (Mard ((*C), i, i) == 0)
		return NULL;
/*              Maerror ("Matrix singular in call to Madiag\n"); */
#endif
	    Mard ((*C), j, matrixsize) -= Mard ((*C), j, i) / \
				    Mard ((*C), i, i) * Mard ((*C), i, matrixsize);
	    Mard ((*C), j, i) = 0;
	}

/*Normalise diagonal to 1: */
    for (i = 0; i < matrixsize; i++) {
#ifdef CHECK_MATRIX_SINGULAR
	if (Mard ((*C), i, i) == 0)
	    return NULL;
/*          Maerror ("Matrix singular in call to Madiag\n"); */
#endif
	Mard ((*C), i, matrixsize) /= Mard ((*C), i, i);
	Mard ((*C), i, i) = 1;
    }

    return C;
}

/*
   Re-initialises a matrix if differs in size or type from the given
   values. If the given matrix is a NULL pointer than reinit 
   allocates a matrix of the required size and returns a pointer
   to it
 */
Matrix *Mareinit (Matrix * A, int rows, int columns, int datatype)
{
    if (!A)
	return Maallocmatrix (rows, columns, 'd');
    if (A->rows != rows || A->columns != columns ||
	A->datatype != datatype) {
	Madestroymatrix (A);
	Mainitmatrix (A, rows, columns, 'd');
    }
    return A;
}


/*calculates C = A^T */
Matrix *Matranspose (Matrix * C, Matrix * A)
{
    int i, j;
    int n = A->rows, m = A->columns;

    C = Mareinit (C, m, n, 'd');

    for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
	    Mard ((*C), j, i) = Mard ((*A), i, j);

    return C;
}


/* calculates C = [A:Y] */
Matrix *Maaugment (Matrix * C, Matrix * A, Matrix * Y)
{
    int i, j;
    int n, m, o;

    C = Mareinit (C, m = A->rows, n = (o = A->columns) + Y->columns, 'd');

    for (i = 0; i < o; i++)
	for (j = 0; j < m; j++)
	    Mard ((*C), j, i) = Mard ((*A), j, i);
    for (i = o; i < n; i++)
	for (j = 0; j < m; j++)
	    Mard ((*C), j, i) = Mard ((*Y), j, i - o);

    return C;
}

/*
   Extracts a matrix C numrows by numcolumns from a larger matrix A
   starting at the position row, column of A
 */
Matrix *Masubmatrix (Matrix * C, Matrix * A, int row, int column, \
							int numrows, int numcolumns)
{
    int i, j;
    int n, m;

    C = Mareinit (C, m = numrows, n = numcolumns, 'd');

    for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
	    Mard ((*C), j, i) = Mard ((*A), j + row, i + column);

    return C;
}



/*
   The following finds the least squares solution to X where

   AX = Y

   where A has more rows than columns. I.e. it solves
   an overconstrained linear system by least squares.

   if A is square Madiag is called. This consumes more memory than a direct call to
   Madiag as an augmented matrix has to be created first.

   This does not change A or Y.

   returns NULL if A is singular
 */

Matrix *Maminimize (Matrix * X, Matrix * A, Matrix * Y)
{
    int m = A->rows, n = A->columns;
    Matrix *AT = NULL, *C = NULL, *ATY = NULL;

    if (m < n)
/*could do a minimum entropy thing here */
	Maerror ( \
"Number of rows less than or equal to number of columns in call to Maminimise\n");

    if (m == n) {
/*      AT = Maallocmatrix(n, n + 1, 'd'); */
	AT = Maaugment (NULL, A, Y);
    } else {
/*      AT = Maallocmatrix(n, m, 'd'); */
/*      C = Maallocmatrix(n, n, 'd'); */
/*      ATY = Maallocmatrix(n, 1, 'd'); */

	C = Mamultiply (NULL, AT = Matranspose (NULL, A), A);
	ATY = Mamultiply (NULL, AT, Y);

/*now we'll use AT to hold the augmented matrix. */
	Maaugment (AT, C, ATY);
	Mafreematrix (ATY);
	Mafreematrix (C);
/*This is almost to simple. */
    }

/*AT now holds a square augmented matrix */
    if (Madiag (AT) == NULL) {
	return NULL;
    }
    X = Masubmatrix (X, AT, 0, n, n, 1);
    Mafreematrix (AT);
    return X;
}


void Maprint (Matrix * Y)
{
    int i, j;
    for (j = 0; j < Y->rows; j++) {
	for (i = 0; i < Y->columns; i++)
	    printf ("% 9.8g ", Mard ((*Y), j, i));
	printf ("\n");
    }
    printf ("\n");
}


void Maprintmatrix (Matrix * Y)
{
    int i, j;
    for (j = 0; j < Y->rows; j++) {
	for (i = 0; i < Y->columns; i++)
	    printf ("% 9.8g ", Mard ((*Y), j, i));
	printf ("\n");
    }
    printf ("\n");
}


/*find sqrt of the sum squared diference of A and B
   using the smaller of A and B as the extents to which it
   compares */
double Madistance (Matrix * A, Matrix * B)
{
    int i, j, n, m;
    double d = 0;

    n = min (A->rows, B->rows);
    m = min (A->columns, B->columns);

    for (j = 0; j < n; j++)
	for (i = 0; i < m; i++)
	    d += pow (Mard ((*A), j, i) - Mard ((*B), j, i), 2);

    return sqrt (d);
}

Matrix *Macross (Matrix * a, Matrix * b)
{
    Matrix *r = Maallocmatrix (1, 3, 'd');
    Mard (*r, 0, 0) = Mard (*r, 0, 1) * Mard (*r, 0, 2) - \
		    Mard (*r, 0, 2) * Mard (*r, 0, 1);
    Mard (*r, 0, 1) = Mard (*r, 0, 2) * Mard (*r, 0, 0) - \
		    Mard (*r, 0, 0) * Mard (*r, 0, 2);
    Mard (*r, 0, 2) = Mard (*r, 0, 0) * Mard (*r, 0, 1) - \
		    Mard (*r, 0, 1) * Mard (*r, 0, 0);
    return r;
}

/*returns the sum of the product of every corresponding element */
double Madot (Matrix * A, Matrix * B)
{
    int i, j, n, m;
    double d = 0;

    n = min (A->rows, B->rows);
    m = min (A->columns, B->columns);

    for (j = 0; j < n; j++)
	for (i = 0; i < m; i++)
	    d += Mard ((*A), j, i) * Mard ((*B), j, i);

    return d;
}


Matrix *Mascale (Matrix * A, double s)
{
    Maallelements (*A, double, *=s);
    return A;
}

/*
   Matrix *Mascalemany (Matrix * A, double *s,...)
   {
   double S, *d;
   va_list ap;

   if (!s)
   return 0;

   S = *s;

   va_start (ap, s);

   while ((d = va_arg (ap, double *))!=NULL)
   S *= *d;

   Maallelements(*A, double, *=S);

   va_end (ap);

   return A;
   }
 */

Matrix *Maaddscalar (Matrix * A, double s)
{
    Maallelements (*A, double, +=s);
    return A;
}

/* args terminated by zero */
Matrix *Maaddmanyscalar (Matrix * A, double *s,...)
{
    double S, *d;
    va_list ap;

    if (!s)
	return 0;

    S = *s;

    va_start (ap, s);

    while ((d = va_arg (ap, double *)) != NULL)
	 S += *d;

    Maallelements (*A, double, +=S);

/*    for (j = 0; j < A->rows; j++)
   for (i = 0; i < A->columns; i++)
   Mard ((*A), j, i) += S;
 */

    va_end (ap);

    return A;
}


/*inserts smaller matrix A into larger matrix C at the given position */
Matrix *Mainsert (Matrix * C, Matrix * A, int row, int column)
{
    int i, j;
    for (j = 0; j < A->rows; j++)
	for (i = 0; i < A->columns; i++)
	    Mard ((*C), j + row, i + column) = Mard ((*A), j, i);
    return C;
}

void Masetupdata (Matrix * A, int rows, int columns, int datatype, int type_size)
{
    int i;

    Maset (A, rows, columns, datatype, Mamalloc (rows * sizeof (void *)));

    *(A->d) = Mamalloc (rows * columns * type_size);
    total_elements += rows * columns;
    if (rows > 1)
	for (i = 1; i < rows; i++)
	    *(A->d + i) = (byte *) * (A->d) + i * columns * type_size;
}

/*
   sets up the row pointers A.d[i]. Must be called only
   after A.d has been alloc'ated an array of pointers (obviously).
   p must point to the first byte of the data.
 */

void Mainitrows (Matrix * A, int rows, int columns, int sizeoftype, void *p)
{
    int i = 0;
    total_elements += rows * columns;
    for (; i < rows; i++)
	*(A->d + i) = (byte *) p + i * columns * sizeoftype;
}

int Masizeof (int datatype)
{
    switch (datatype) {
    case 'd':
	return (sizeof (double));
	break;
    case 'f':
	return (sizeof (float));
	break;
    case 'l':
	return (sizeof (long));
	break;
    case 'c':
	return (sizeof (char));
	break;
    case 's':
	return (sizeof (short));
	break;
    case 'i':
	return (sizeof (short));
	break;
    default:
	Maerror ("This matrix type is invalid or not supported.");
	return 0;
    }
}

void Maclearmatrix (Matrix * A)
{
    memset (&(Mard ((*A), 0, 0)), 0, Masizeof (A->datatype) * A->columns * A->rows);
}


/*
   Initializes an existing matrix structure with the passed values,
   allocates memory for its data, and initialises that memory to zero
   Use for initialising a matrix declared as a constant.
 */
Matrix *Mainitmatrix (Matrix * A, int rows, int columns, int datatype)
{
    int s = Masizeof (datatype);
    Maset (A, rows, columns, datatype, Mamalloc (rows * sizeof (void *)));
    Mainitrows (A, rows, columns, s, Mamalloc (rows * columns * s));
    Maclearmatrix (A);
    return A;
}


/*
   Allocates memory for a matrix structure, intialises it with the
   passed values, allocates memory for matrix data, sets matrix data to zero.
   Use for initialising a matrix declared as a pointer.
 */
Matrix *Maallocmatrix (int rows, int columns, int datatype)
{
    Matrix *A = Mamalloc (sizeof (Matrix));
    return Mainitmatrix (A, rows, columns, datatype);
}

/*
   Here you can make a matrix from a list of data
   eg for a 2x3:
   R = Maallocdata (&x, &y, &z, NULL, &a, &b, &c, NULL, NULL);

   each row ends with a NULL and the final rows ends with two NULL's.

 */

/* do not terminate by zero */
/* used like this: 
	Matrix *Madoublestomatrix (int rows, int columns, double first,...) */
Matrix *Madoublestomatrix (int rows, double first,...)
{
    va_list ap;

    Matrix *R = Maallocmatrix (rows, (int) first, 'd');

    va_start (ap, first);
    Maallelements (*R, double, = va_arg (ap, double));
    va_end (ap);

    return R;
}


/*
   This COPIES an existing TWO DIMENSIONAL array p[][] into
   a matrix. p must be larger than or equal to 'rows' and 'columns'.
   If the data is a large contigous block, then rather use Maallocuser
   or Mainituser, to avoid duplicating the data. p may be free'd after
   calling these functions. The returned matrix must be Mafreenatrix'd.
 */

Matrix *Mainitarray (Matrix * A, int rows, int columns, int datatype, void **p)
{
    int j = 0;
    int s = Masizeof (datatype);
    Maset (A, rows, columns, datatype, Mamalloc (rows * sizeof (void *)));
    Mainitrows (A, rows, columns, s, Mamalloc (rows * columns * s));

    for (; j < rows; j++)
	memcpy (A->d[j], p[j], columns * s);

    return A;
}

Matrix *Maallocarray (int rows, int columns, int d, void **p)
{
    Matrix *R = Mamalloc (sizeof (Matrix));
    Mainitarray (R, rows, columns, d, p);
    return R;
}

/*
   This initialises a matrix to the values of a ONE DIMENSIONAL
   array q[]. q must have sufficient space allocated to it
   or be of sufficient size.
   The matrix may have width or height greater than one, but
   then the data will scan the matrix from left to right and then
   from top to bottom. q may be free'd after the matrix
   has been Madestroyuser'd or Mafreeuser'd. If q is a
   constant, the matrix must be destoyed only with Madestroyuser()
   or Mafreeuser(). In this case be weary of commands that may like to
   change the size of the matrix by re-malloc'ating it.
   If q is allocated then the resulting matrix will be no different
   (at least in this version) to a matrix created by any of the
   other routines.

   This is useful for turning a structure into a matrix.
   eg.

   typedef struct {
   double x,
   double y,
   double z,
   double a,
   double b,
   double c,
   } Vector

   .
   .
   .

   Vector v;

   Matrix *V = Maallocuser(2,3,'d',&(v.x));

   Mard((*V),1,2) = THE_VALUE_OF_B;
   .
   .
   .

   Mafreeuser(V);

 */

Matrix *Mainituser (Matrix * R, int rows, int columns, int d, void *q)
{
    Maset (R, rows, columns, d, Mamalloc (rows * sizeof (void *)));
    Mainitrows (R, rows, columns, Masizeof (d), q);
    return R;
}

Matrix *Maallocuser (int rows, int columns, int d, void *q)
{
    Matrix *R = Mamalloc (sizeof (Matrix));
    Mainituser (R, rows, columns, 'd', q);
    return R;
}


/*frees the data in A. Initialise the matrix structure and
   frees the data, but does not free the matrix structure itself. */
/*use for freeing a matrix declared as a constant */
void Madestroymatrix (Matrix * A)
{
    if (A) {
	if (*(A->d))
	    free (*(A->d));
	if (A->d)
	    free (A->d);

	total_elements -= A->rows * A->columns;
	Maset (A, 0, 0, 0, NULL);
    } else
	printf ("Warning: trying to destroy a matrix that is a NULL pointer.\n");
}


/* free's the entire matrix, structure and data */
/* for freeing matrices that are declared as pointers */
void Mafreematrix (Matrix * A)
{
    if (A) {
	Madestroymatrix (A);
	free (A);
    }
}

void Mafreeuser (Matrix * A)
{
    *(A->d) = NULL;
    Mafreematrix (A);
}

void Madestroyuser (Matrix * A)
{
    *(A->d) = NULL;
    Madestroymatrix (A);
}

/*
   example:


   main()
   {

   Matrix *A;
   Matrix B;

   Mainitmatrix(B, 3, 4, 'd');
   Maallocmatrix(A, 3, 4, 'd'); /+ type double +/

   .
   .
   .


   Madestroymatrix(B);
   Mafreematrix(A);

   }

 */




/*
   Converts a table of ascii doubles to a matrix.
   The table can be delimited by almost any character that wouldn't
   be in a number. Rows must be delimited by newlines. The last
   line must not end with a newline, or an extra row of zeros will
   be added.
   The number of doubles on the first line dictate the number of
   columns. The remainder of the matrix will be padded with zeros
   where there are not enough doubles on a line. The number of
   lines dictates the number of rows. If there are a greater
   number of columns than suggested by the first line, then
   these will be ignored.

   Returns matrix pointer which must be Mafree'd --- see Mafree().
   Returns NULL on error or empty file --- although it will attempt
   to convert almost anything.
 */

Matrix *Maasciitomatrix (const char *text)
{
    int rows = strcountlines (text, 0, strlen (text), 32000) + 1, columns = 0;
    Matrix *X;

    char *endptr = (char *) text, *nptr;

    int i = 0, j = 0;

    do {
	columns++;
	nptr = endptr + strcspn (endptr, "+-1234567890.\n");
	if (!(*nptr) || *nptr == '\n')
	    break;
	strtod (nptr, &endptr);
    } while (nptr != endptr);

    columns--;

    if (!columns)
	return NULL;

    nptr = (char *) text;

    X = Maallocmatrix (rows, columns, 'd');
    Maclearmatrix (X);

    while (*nptr && i < rows) {
	j = 0;
	endptr = nptr;
	do {
	    j++;
	    nptr = endptr + strcspn (endptr, "+-1234567890.\n");
	    if (!(*nptr) || *nptr == '\n')
		break;
	    Mard ((*X), i, j - 1) = strtod (nptr, &endptr);
	} while (nptr != endptr && j <= columns);
	nptr += strmovelines (nptr, 0, 1, 32000);
	i++;
    }

    return X;
}
