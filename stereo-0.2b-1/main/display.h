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
#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <config.h>
#include "global.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <my_string.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <stdarg.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "regex.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "coolwidget.h"

#include "camera.h"
#include "fitline.h"


struct picwithzoom {
    CWidget *main_image;		/* thumbnail image */
    CWidget *zoom_image;		/* zoombox image */
    CWidget *main_win;			/* window of thumbnail image */
    CWidget *zoom_win;			/* window of zoombox image */
    CWidget *main_rect;			/* zoombox rectagle rectangle 
							    in thumnail image */
    CWidget *zoom_rect;			/* ..in zoom image */
    CWidget *main_markers;		/* markers in thumbnail */
    CWidget *zoom_markers;		/* markers in zoombox */


    CWidget *Ttext;			/* text display of image info */
    CWidget *Binfo;			/* display cam info button */
    CWidget *Blast;			/* remove last marker button */
    CWidget *Ball;			/* remove all markers button */
    CWidget *Bcalibrate;		/* calibrate camera button */
    CWidget *Bkill;			/* kill this window button */

    CWidget *Bleave;			/* for later use */
    CWidget *Tleave;
    CWidget *Ssig;
    CWidget *Isigma;
    CWidget *B5;
    int last_pointer;

    long width, height;			/* width and height of the thumbnail image */
    double real_width, real_height;	/* width and height of the original image */
    double x0, y0;			/* image centre */
    int zwidth, zheight;		/* width and height of the zoomed area */
    long xzoom, yzoom;			/* position of the zoomed area */
    int mag;				/* zoombox magnification */
};

typedef struct picwithzoom Picture;

#define MAX_NUM_VIEWS 32
#define MAX_NUM_MARKS 256

typedef struct marks Mark;

struct view {
    Picture pic;
    char *filename;		/* a tiff or targa file. 0 if non-existant */
    Camera cam;			/* camera data structure */
    int calibrated;		/* always 1 if calibrated */
    Vec mark[MAX_NUM_MARKS];
    int num_marks;
};

typedef struct view View;

struct desktop {
    View view[MAX_NUM_VIEWS];
    Vec *cal_points;
    int num_cal_points;
    int leave_out_calibration_points;
    int optimise_sigma;
    char *cal_file;
    int current_view;
    int num_views;
    char *temp_dir;
    char *image_dir;
    Vec centre_offset_for_3d;
    double scale_units_for_3d;
};

typedef struct desktop Desktop;


int load_calibration_points(Desktop *d);
int save_calibration_points(Desktop *d);
int load_new_view(Desktop *d);
/* int save_view(Desktop *d); */

/* fits an object (see fitline.h) to the markers */
int fit_object (Desktop * d, Object * object, int type);

#define ZOOMSIZE (192*2)

#define THESE_2D_PROJECTIONS_HAVE_BEEN_ADJUSTED_FOR_LENS_DISTORTION YES

#define MAX_CAP 1024

#define MONITOR_GAMMA 1.2

#define MAX_CAL_FILE_SIZE 65536

void Dcoordtotext (char *widgetevent, char *strx, char *stry, \
		struct picwithzoom *image);

int Cundrawrectangle (const char *ident);

int Cdrawrectangle (const char *ident, int x, int y, int w, \
			int h, unsigned long color);

void Dcheckinsertcoordfromimage (CEvent * cwevent, const char *ident, \
		struct picwithzoom *image, int p);

void Dimagecoord (CEvent * cwevent, double *x, double *y, struct picwithzoom *image);

#endif /* _DISPLAY_H */
