/*****************************************************************************************/
/* display.c - main() function                                                           */
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
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "app_glob.c"
#include "coolwidget.h"
#include "imagewidget.h"

#include "stringtools.h"
#include "dirtools.h"
#include "hugeimage.h"
#include "widget3d.h"
#include "edit.h"
#include "camera.h"
#include "fitline.h"

#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "loadfile.h"
#include "display.h"
#include "main/callback.h"
#include "main/imagefit.h"
#include "dialog.h"
#include "matrix.h"
#include "calibrate.h"
#include "3dkit.h"
#include "desktop.h"
#include "mad.h"

Desktop desktop;

#define WIDTH3D 640
#define HEIGHT3D 480

/* options */

char *editor_options_file = 0;

char *option_display = 0;
char *option_geometry = 0;
char *option_background_color = 0;
char *option_foreground_red = 0;
char *option_foreground_green = 0;
char *option_foreground_blue = 0;
char *option_font = 0;


int leave_out_calibration_points = 0;

void goto_error (char *message)
{

}

void init_desktop (Desktop * d)
{
    clear (d, Desktop);
    d->image_dir = strdup (DATADIR);
    d->temp_dir = strdup (TEMPDIR);
}


void stereo_init (char *name)
{
    CInitData stereo_startup =
    {
	20, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0
    };

    stereo_startup.height_plus = 900;
    stereo_startup.lines = 0;
    stereo_startup.width_plus = 700;
    stereo_startup.columns = 0;
    stereo_startup.options |= CINIT_OPTION_USE_GREY;

    stereo_startup.name = name;

/* initialise: */
    Cinit (&stereo_startup);
}

void main (int argc, char **argv)
{
    int menuopen = 0;
    int x, y, x1;
    CEvent cwevent;
    XEvent xevent;
    Window choicewin, editorwin, win3d;

    init_desktop (&desktop);

    stereo_init (argv[0]);

    choicewin = Cdrawwindow ("choicewin", CMain, 0, 0, 10, 10, "");
    Cgethintpos (&x, &y);
    x1 = x;
    Cdrawbutton ("load", choicewin, x, y, AUTO_SIZE,
		 " Load Image ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("loadcal", choicewin, x, y, AUTO_SIZE,
		 " Load Calibration File ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("showcal", choicewin, x, y, AUTO_SIZE,
		 " Display Calibration Points ");
    Cgethintpos (0, &y);  x = x1;
    Cdrawbutton ("loadd", choicewin, x, y, AUTO_SIZE,
		 " Load Previous Desktop ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("saved", choicewin, x, y, AUTO_SIZE,
		 " Save Previous Desktop ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("finish", choicewin, x, y, AUTO_SIZE,
		 " Quit ");
    Cgethintpos (0, &y);  x = x1;
    Cdrawbutton ("getpnt", choicewin, x, y, AUTO_SIZE,
		 " Triangulate Point        ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("getsurf", choicewin, x, y, AUTO_SIZE,
		 " Triangulate Multiple Points (Surface)   ");
    Cgethintpos (0, &y);  x = x1;
    Cdrawbutton ("getline", choicewin, x, y, AUTO_SIZE,
		 " Fit Line from Points     ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("getlineedge", choicewin, x, y, AUTO_SIZE,
		 " Fit Line from two Line Projections      ");
    Cgethintpos (0, &y);  x = x1;
    Cdrawbutton ("getcyl", choicewin, x, y, AUTO_SIZE,
		 " Fit Cylinder from Points ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("getcyle", choicewin, x, y, AUTO_SIZE,
		 " Fit Cylinder from four Line Projections ");
    Cgethintpos (0, &y);  x = x1;
    Cdrawbutton ("getcirc", choicewin, x, y, AUTO_SIZE,
		 " Fit Circle from Points   ");
    Cgethintpos (&x, 0);
    Cdrawbutton ("getcircedge", choicewin, x, y, AUTO_SIZE,
		 " Fit Circle from Ellipses Projections    ");
    Cgethintpos (0, &y);  x = x1;
    Cdrawbutton ("getellipse", choicewin, x, y, AUTO_SIZE,
		 " Fit Ellipse from Points  ");
    Cgethintpos (&x, 0);

    Csetsizehintpos ("choicewin");

    editorwin = Cdrawwindow ("editorwin", CMain, 200, 100, 640 + 18, \
			     450 + 40 + 18 + TEXT_PIX_PER_LINE + 11, "");
    Cdraweditor ("editor", editorwin, 6, 6 + 40,
	       640, 450, "", 0, "/home/terry/stereo-0.2b/tmp/");

    CDrawEditMenuButtons ("em", editorwin, Cwidget ("editor")->winid, 10, 10);

    Caddcallback ("editor", cb_editor);


    Cdraw3dobject ("3dview", win3d = Cdrawwindow ("3dplanewin",
		CMain, 150, 150, WIDTH3D + 100, 16 + HEIGHT3D, ""), 6, 6,
		   WIDTH3D, HEIGHT3D, 1, 256);

    Cdrawbutton ("plshr", win3d, WIDTH3D + 20, 10, 70, 20, "Shrink");
    Cdrawbutton ("plenl", win3d, WIDTH3D + 20, 40, 70, 20, "Enlarge");
    Cdrawbutton ("newrender", win3d, WIDTH3D + 20, 70, 70, 20, "New render");
    Cdrawbutton ("density", win3d, WIDTH3D + 20, 100, 70, 20, "Density");
    Cdrawbutton ("flattri", win3d, WIDTH3D + 20, 130, 70, 20, "Flat Triangle");
    Cdrawbutton ("savewin", win3d, WIDTH3D + 20, 160, 70, 20, "Save Window");


    Caddcallback ("getpnt", cb_getpoint);
    Caddcallback ("getellipse", cb_getellipse);
    Caddcallback ("getcircedge", cb_getcircleedge);

    Caddcallback ("load", cb_newimage);

    Caddcallback ("3dview", cb_3dplane);
    Caddcallback ("newrender", cb_newrender);


    Caddcallback ("getsurf", cb_getsurface);
    Caddcallback ("getline", cb_getline);
    Caddcallback ("getcirc", cb_getcircle);
    Caddcallback ("getcyle", cb_getcylinderedge);
    Caddcallback ("getcyl", cb_getcylinder);
    Caddcallback ("getlineedge", cb_getlineedge);

    Caddcallback ("loadcal", cb_loadcal);
    Caddcallback ("showcal", cb_showcal);
    Caddcallback ("saved", cb_save_desktop);
    Caddcallback ("loadd", cb_load_desktop);

    Caddcallback ("savewin", cb_save_window);

    if (argc > 1)
	if (*argv[1] != '-')
	    do_load_desktop (&desktop, argv[1]);

    do {
	CNextEvent (&xevent, &cwevent);

	if (!strcmp (cwevent.ident, "plshr")) {
	    Cwidget ("3dview")->solid->distance += 2400;
	    Cwidget ("3dview")->solid->y_cam += 2400;
	    Credraw3dobject ("3dview", 0);
	}
	if (!strcmp (cwevent.ident, "plenl")) {
	    Cwidget ("3dview")->solid->distance -= 2400;
	    Cwidget ("3dview")->solid->y_cam -= 2400;
	    Credraw3dobject ("3dview", 0);
	}
	if (!strcmp (cwevent.ident, "flattri")) {
	    CWidget *w = Cwidget ("3dview");
	    if (w->solid->option_flags & TDOPTION_FLAT_TRIANGLE) {
		w->solid->option_flags &= 0xFFFFFFFF - TDOPTION_FLAT_TRIANGLE;
	    } else {
		w->solid->option_flags |= TDOPTION_FLAT_TRIANGLE;
	    }
	    Credraw3dobject ("3dview", 0);
	}
	if (!strcmp (cwevent.ident, "kill")) {
	    menuopen = 0;
	    Cundrawwidget ("win1");
	}
    } while (strcmp (cwevent.ident, "finish"));

    Cundrawall ();
    mad_finalize (__FILE__, __LINE__);

}

