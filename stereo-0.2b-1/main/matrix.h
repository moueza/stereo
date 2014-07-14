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
#ifndef MATRIX_H
#define MATRIX_H 

#include "../config.h"
#include <stdlib.h>
#include <math.h>
#include <quickmath.h>

/*
   Uncomment this line to let any matrix reference check whether it is within
   bounds of the matrix. This will easily show up code that references
   outside the dimensions of the matrix. It gives the responce:
   Fatal error: trying to access outside matrix bounds, <file>:<line>
*/
/* #define CHECK_MATRIX_BOUNDS */

/*uncomment to let the matrix diagonalisation pick up zero divides*/
#define CHECK_MATRIX_SINGULAR 1

/*all matrix operations start with Ma */


typedef struct {
    int rows;			/* size */
    int columns;		/* of matrix */
    int datatype;		/* is one of 'd', 'f', 'l', 'c', 's', or 'i' */
			/* see below for definitions */
    void **d;
} Matrix;

int Macheckmatrixbounds(int rows, int columns, int j, int i, \
						    const char *file, int line);


#ifdef CHECK_MATRIX_BOUNDS

/* Macheckmatrixbounds always returns zero or aborts if out of bounds */

/* eg: 'Ma'trix 'r'eturn 'd'ouble */
#define Mard(m,j,i) (*((double *) *((m).d + j) + i \
	+ Macheckmatrixbounds((m).rows, (m).columns, j, i, __FILE__, __LINE__)))
#define Marf(m,j,i) (*((float *) *((m).d + j) + i \
	+ Macheckmatrixbounds((m).rows, (m).columns, j, i, __FILE__, __LINE__)))
#define Marl(m,j,i) (*((long *) *((m).d + j) + i \
	+ Macheckmatrixbounds((m).rows, (m).columns, j, i, __FILE__, __LINE__)))
#define Marc(m,j,i) (*((unsigned char *) *((m).d + j) + i \
	+ Macheckmatrixbounds((m).rows, (m).columns, j, i, __FILE__, __LINE__)))
/* 's'hort unsigned */
#define Mars(m,j,i) (*((unsigned short *) *((m).d + j) + i \
	+ Macheckmatrixbounds((m).rows, (m).columns, j, i, __FILE__, __LINE__)))
/* short 'i'nteger: */
#define Mari(m,j,i) (*((signed short *) *((m).d + j) + i \
	+ Macheckmatrixbounds((m).rows, (m).columns, j, i, __FILE__, __LINE__)))

#else

#define Mard(m,j,i) (*((double *) *((m).d + j) + i))
#define Marf(m,j,i) (*((float *) *((m).d + j) + i))
#define Marl(m,j,i) (*((long *) *((m).d + j) + i))
#define Marc(m,j,i) (*((unsigned char *) *((m).d + j) + i))
/* 's'hort */
#define Mars(m,j,i) (*((unsigned short *) *((m).d + j) + i))
/* 'i'nteger: */
#define Mari(m,j,i) (*((signed short *) *((m).d + j) + i))

#endif

/*
    do operation 'o' to all elements in a matrix.
    This is as fast as its going to get;
*/
#define Maallelements(m, t, o) \
		{ \
		    t *p_to_element = (t *) (*(m).d); \
		    long num_elements = (long) (m).rows * (m).columns; \
		    do { \
			*(p_to_element++) o; \
		    } while (--num_elements); \
		}


Matrix *Magetrotation (Matrix * m, double phi, double theta, double tsi);

Matrix *Mamultiply (Matrix * C, Matrix * A, Matrix * B);

Matrix *Maadd (Matrix * C, Matrix * A, Matrix * B);

Matrix *Masubtract (Matrix * C, Matrix * A, Matrix * B);

double Ma3dtoscreen (Matrix * m, Matrix * v, Matrix * V, double f, \
							double *x, double *y);

double Manormal (Matrix * A);

Matrix *Madiag (Matrix * C);

Matrix *Matranspose (Matrix * C, Matrix * A);

Matrix *Maaugment (Matrix * C, Matrix * A, Matrix * Y);

Matrix *Masubmatrix (Matrix * C, Matrix * A, int row, int column, \
						    int numrows, int numcolumns);

Matrix *Maminimize (Matrix * X, Matrix * A, Matrix * Y);

void Maprintmatrix (Matrix * Y);

void Maprint (Matrix * Y);

double Madistance (Matrix * A, Matrix * B);

Matrix * Macross(Matrix *a, Matrix *b);

double Madot (Matrix * A, Matrix * B);

Matrix *Mascale (Matrix * A, double s);

Matrix *Maaddscalar (Matrix * A, double s);

Matrix *Mainsert (Matrix * C, Matrix * A, int row, int column);

Matrix *Mainitmatrix (Matrix * A, int rows, int columns, int datatype);

Matrix *Maallocmatrix (int rows, int columns, int datatype);

void Madestroymatrix (Matrix * A);

void Mafreematrix (Matrix * A);

Matrix *Maasciitomatrix (const char *text);

void *Mamalloc (size_t size);


Matrix *Madoublestomatrix (int rows, double first,...);

Matrix *Mainitarray(Matrix *R, int rows, int columns, int d, void **p);

Matrix *Maallocarray(int rows, int columns, int d, void **p);

Matrix *Mainituser(Matrix *R, int rows, int columns, int d, void *q);

Matrix *Maallocuser(int rows, int columns, int d, void *q);

void Mafreeuser(Matrix *A);

void Madestroyuser(Matrix *A);

void Maerror(const char *e);

Matrix *Mareinit (Matrix * A, int rows, int columns, int data_type);

long Magettotalelements ();

Matrix *Maaddmanyscalar (Matrix * A, double *s,...);

void Maclearmatrix(Matrix *A);

Vec Mamatrixtovec (Matrix * V);

Matrix *Mavectomatrix (Vec v);

void getrotation(Vec *m0, Vec *m1, Vec *m2, double phi, double theta, double tsi);

#endif

