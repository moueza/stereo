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
#ifndef CAMERA_H
#define CAMERA_H

#include "matrix.h"
#include "quickmath.h"

typedef struct {
    double phi;
    double theta;
    double tsi;
    double f;
    double sig;			/*sigma */
    double s;
    Vec x;			/* centre */
    Vec m_x, m_y, m_s;
/*
   distortion correction could come here:
 */
    double a1;
    double a2;
    double a3;
    double a4;
    double a5;
    double a6;

/* sum squared error in calculation */
    double e;
} Camera;

/* 
   some definitions:
   a PICTURE is the structure containing black and white of that
   view. It has coords top left and down.

   an IMAGE is the physical photograph. It has coords centre and up.

   a CAMERA is same as the IMAGE but undestorted

   finally PHYS is the unrotated actual 3D coord.

   sequence of transformation is.

   phystocamera (cam,v,x,y)  /+ result in x, y +/
   cameratoimage (cam, v.x, v.y)        /+ v.x and v.y altered +/
   imagetopic (pic, v.x, v.y)   /+ v.x and v.y altered +/
   the result is the picture coord (from the top left corner and
   always positive)
   the reverse is just pictoimage, imagetocamera,
   camtophys (c1, c2, v, x1, y1, x2, y2)
 */

#define imagetocamera(c,x,y) { \
    (y) /= (1 + (c)->sig / 100); \
}

#define cameratoimage(c,x,y) { \
    (y) *= (1 + (c)->sig / 100); \
}

#define phystocamera(c,v,xs,ys) { \
    Vec ___v; \
    double ___s = dot ((c)->m_s, ___v = minus ((v), (c)->x)); \
    if (___s <= 0) \
	___s = -1e-10; \
    (xs) = -(c)->f * dot ((c)->m_x, ___v) / ___s; \
    (ys) = -(c)->f * dot ((c)->m_y, ___v) / ___s; \
}

/* camtophys(Camera *c1, Camera *c2, Vec v, double x1, 
   double y1, double x2, double y2);  */

#define cameratophys(c1,c2,v,x1,y1,x2,y2) { \
    Camera *___c[2]; \
    double ___x[2], ___y[2]; \
    ___x[0] = (x1); \
    ___x[1] = (x2); \
    ___y[0] = (y1); \
    ___y[1] = (y2); \
    ___c[0] = (c1); \
    ___c[1] = (c2); \
    (v) = triangulate_camera_point (___x, ___y, c, 2); \
}

void camundestortmany (Camera * cam, double *x, double *y, int n);

#endif
