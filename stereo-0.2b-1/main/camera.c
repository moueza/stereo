/*****************************************************************************************/
/* camera.c - undestort camera coordinates                                               */
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
#include "matrix.h"
#include "camera.h"

Matrix *Macamundestortmany (Camera * c, Matrix * x)
{
    int i = 0;
    for (; i < x->columns; i++)
	imagetocamera (c, (Mard (*x, 0, i)), (Mard (*x, 1, i)));
    return x;
}

void camundestortmany (Camera * cam, double *x, double *y, int n)
{
    int i;
    for (i = 0; i < n; i++)
	imagetocamera (cam, x[i], y[i]);
}
