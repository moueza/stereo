/*****************************************************************************************/
/* displaycam.c - draw a list of the camera data                                         */
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


#include "bitmap/camera.bitmap"
#include "bitmap/camphi.bitmap"
#include "bitmap/camtheta.bitmap"
#include "bitmap/campsi.bitmap"
#include "bitmap/camf.bitmap"
#include "bitmap/camsig.bitmap"
#include "bitmap/camxy.bitmap"
#include "bitmap/camz.bitmap"

/* \end{verbatim} \begin{verbatim} */

void draw_camera_data_text (Camera * c, Window win, int x, int y)
{
    if (!Ci ("caldatabitmaps")) {
	Cdrawbitmap ("caldatabitmaps", win, x, y,
		       40, 32, Ccolor (1), Ccolor(25), camphi_bits);
	Cdrawbitmap ("", win, x, y + 50,
		       40, 32, Ccolor (1), Ccolor(25), camtheta_bits);
	Cdrawbitmap ("", win, x, y + 100,
		       40, 32, Ccolor (1), Ccolor(25), campsi_bits);
	Cdrawbitmap ("", win, x, y + 150,
		       40, 32, Ccolor (1), Ccolor(25), camf_bits);
	Cdrawbitmap ("", win, x, y + 200,
		       40, 40, Ccolor (1), Ccolor(25), camxy_bits);
	Cdrawbitmap ("", win, x, y + 258,
		       40, 20, Ccolor (1), Ccolor(25), camz_bits);
	Cdrawbitmap ("", win, x, y + 296,
		       40, 32, Ccolor (1), Ccolor(25), camsig_bits);

	y += 10;
	x += 55;

	Cdrawtext ("tcalphi", win, x, y, " %15.2f ", c->phi * 180 / PI);
	Cdrawtext ("tcaltheta", win, x, y + 50, " %15.2f ", c->theta * 180 / PI);
	Cdrawtext ("tcaltsi", win, x, y + 100, " %15.2f ", c->tsi * 180 / PI);
	Cdrawtext ("tcalf", win, x, y + 150, " %15.8g ", c->f);
	Cdrawtext ("tcalx", win, x, y + 192, " %15.8g ", c->x.x);
	Cdrawtext ("tcaly", win, x, y + 216, " %15.8g ", c->x.y);
	Cdrawtext ("tcalz", win, x, y + 252, " %15.8g ", c->x.z);
	Cdrawtext ("tcalsig", win, x, y + 296, " %15.5g ", c->sig);
	Cdrawtext ("tcale", win, 10, y + 340, \
		    " Sum squared error in pixels: \n %15.5g ", c->e);

    } else { /*merely update the text*/
	y += 10;
	x += 55;

	Credrawtext ("tcalphi", " %15.2f ", c->phi * 180 / PI);
	Credrawtext ("tcaltheta", " %15.2f ", c->theta * 180 / PI);
	Credrawtext ("tcaltsi", " %15.2f ", c->tsi * 180 / PI);
	Credrawtext ("tcalf", " %15.8g ", c->f);
	Credrawtext ("tcalx", " %15.8g ", c->x.x);
	Credrawtext ("tcaly", " %15.8g ", c->x.y);
	Credrawtext ("tcalz", " %15.8g ", c->x.z);
	Credrawtext ("tcalsig", " %15.5g ", c->sig);
    }
}


void draw_calibrarion_data_text (Desktop * d, Window win, int x, int y)
{
    int i;
    if (d->num_cal_points)
	for (i = 0; i < d->num_cal_points; i++)
	    Cdrawtext (catstrs ("calpoint", itoa (i), 0), win, x, y + 30 * i, \
		" %3d: %15f %15f %15f ", i + 1, \
		d->cal_points[i].x, d->cal_points[i].y, d->cal_points[i].z);
    else
	Cdrawtext ("calpoint", win, x, y, " Data not loaded ");
}


void draw_camera_data_window (Camera *c)
{
    Window win;
    static isopen = 0;

    if (isopen) {
	Cundrawwidget ("camdatawin");
	isopen = 0;
    } else {
	win = Cdrawwindow ("camdatawin", CMain, 20, 20, 300, 410, "caldatawin");
	draw_camera_data_text (c, win, 10, 10);
	isopen = 1;
    }
}

void draw_camera_data (Desktop *d, CEvent *e)
{
    set_current_from_pointer (d, e);
    if(d->num_views) {
	draw_camera_data_window (&d->view[d->current_view].cam);
    }
}

void show_cal_points (Desktop *d, CEvent *e)
{
    Window win;
    static int isopen = 0;

    if (isopen) {
	Cundrawwidget ("caldatawin");
	isopen = 0;
    } else {
	int x, y;
	win = Cdrawheadedwindow ("caldatawin", CMain, 20, 20, 460, \
		(d->num_cal_points ? d->num_cal_points : 1) * 30 + 10, \
		    " Calibration Points ");
	Cgethintpos (&x, &y);
	draw_calibrarion_data_text (d, win, x, y);
	Csetsizehintpos	("caldatawin");
	isopen = 1;
    }
}

