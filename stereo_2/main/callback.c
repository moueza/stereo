/*****************************************************************************************/
/* callback.c - main callback routines from button presses                               */
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
#include "widget3d.h"
#include "main/marker.h"
#include "main/displaycam.h"
#include "loadcalfile.h"
#include "imagehandler.h"
#include "picsetup.h"
#include "edit.h"
#include "dialog.h"
#include "loadfile.h"
#include "desktop.h"
#include "imagefit.h"
#include "calibrate.h"

extern Desktop desktop;

int save_window_to_file (Window win, int x, int y, int width, int height);

int cb_save_window (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    CWidget *w;
    w = Cwidget ("3dview");
    save_window_to_file (w->winid, 0, 0, w->width, w->height);
    return 0;
}


int cb_getsurface (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_surface (&desktop);
    return 0;
}

int cb_getcircle (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_circle (&desktop);
    return 0;
}

int cb_getellipse (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_ellipse (&desktop);
    return 0;
}

int cb_getpoint (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_point (&desktop);
    return 0;
}

int cb_getline (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_line (&desktop);
    return 0;
}

int cb_getcircleedge (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_circle_edge (&desktop);
    return 0;
}

int cb_getlineedge (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_line_edge (&desktop);
    return 0;
}

int cb_getcylinderedge (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_cylinder_edge (&desktop);
    return 0;
}

int cb_getcylinder (CWidget * none, XEvent * xevent, CEvent * cwevent)
{
    output_cylinder (&desktop);
    return 0;
}

int cb_save_desktop (CWidget * w, XEvent *xe, CEvent *ce)
{
    save_desktop (&desktop);
    return 0;
}

int cb_load_desktop (CWidget * w, XEvent *xe, CEvent *ce)
{
    load_desktop (&desktop);
    return 0;
}

int cb_leave (CWidget * w, XEvent *xe, CEvent *ce)
{
    Picture *p;
    set_current_from_pointer (&desktop, ce);
    p = &desktop.view[desktop.current_view].pic;
    Credrawtext (p->Tleave->ident, " %d ", (atoi (p->Tleave->text + 1) + 1) % 3);
    return 0;
}

int cb_sigma (CWidget * w, XEvent *xe, CEvent *ce)
{
    Picture *p;
    set_current_from_pointer (&desktop, ce);

    p = &desktop.view[desktop.current_view].pic;
    if (p->Isigma) {
	Cundrawwidget (p->Isigma->ident);
	p->Isigma = 0;
    } else {
	p->Isigma = Cdrawtextinput (catstrs (p->Ssig->ident, ".input", 0),
		p->main_win->winid, 120, p->main_win->height - 60, 50, 20, 10, "0");
    }
    return 0;
}


int cb_calibrate (CWidget * w, XEvent *xe, CEvent *ce)
{
    int calc_distortion = 0;
    Picture *p;
    double sigma;
    set_current_from_pointer (&desktop, ce);
    p = &desktop.view[desktop.current_view].pic;
    if (p->Isigma) {
	sigma = atof (p->Isigma->text);
    } else {
	sigma = 0;
	calc_distortion |= CALC_SIGMA;
    }
    desktop.view[desktop.current_view].cam.sig = sigma;
    if(calibrate_view (&desktop, ce, atoi (p->Tleave->text + 1), calc_distortion) == 1)
	clear (&desktop.view[desktop.current_view].cam, Camera);
    Cundrawwidget ("calprogw");
    return 0;
}

int cb_showcal (CWidget * w, XEvent *xe, CEvent *ce)
{
    show_cal_points (&desktop, ce);
    return 0;
}

void insert_marker (Picture *p, CEvent *e)
{
    Vec v;
    v = zoom_event_to_image_coord (p, e);
    new_marker (&desktop, v);
    draw_markers (&desktop);
}

void move_marker (Picture *p, CEvent *e)
{
    Vec v;
    v = zoom_event_to_image_coord (p, e);
    move_closest_marker (&desktop, v);
    draw_markers (&desktop);
}

void remove_marker (Picture *p, CEvent *e)
{
    Vec v;
    v = zoom_event_to_image_coord (p, e);
    remove_closest_marker (&desktop, v);
    draw_markers (&desktop);
}


int cb_draw_cam_data (CWidget * w, XEvent *xe, CEvent *ce)
{
    draw_camera_data (&desktop, ce);
    return 0;
}

int cb_loadcal (CWidget * w, XEvent *xe, CEvent *ce)
{
    load_calibration (&desktop);
    return 0;
}

int cb_zoomimage (CWidget * w, XEvent * xe, CEvent * ce)
{
    handle_zoom_box (&desktop, ce);
    return 0;
}

int cb_mainimage (CWidget * w, XEvent * xe, CEvent * ce)
{
    handle_main_box (&desktop, ce);
    return 0;
}

int cb_newimage (CWidget * w, XEvent *xe, CEvent *ce)
{
    load_view (&desktop);
    return 0;
}

int cb_killimage (CWidget * w, XEvent *xe, CEvent *ce)
{
    set_current_from_pointer (&desktop, ce);
    destroy_current_view (&desktop);
    return 0;
}

int cb_clearallmarkers (CWidget *none, XEvent * xe, CEvent * ce)
{
    set_current_from_pointer (&desktop, ce);
    clear_markers (&desktop);
    draw_markers (&desktop);
    return 0;
}

int cb_removelastmarker (CWidget *none, XEvent * xe, CEvent * ce)
{
    set_current_from_pointer (&desktop, ce);
    remove_last_marker (&desktop);
    draw_markers (&desktop);
    return 0;
}

int cb_newrender (CWidget *w, XEvent * xevent, CEvent * cwevent)
{
    w = Cwidget("3dview");
    switch (w->solid->render) {
    case TD_MESH:
	w->solid->render = TD_MESH_AND_SOLID;
	break;
    case TD_MESH_AND_SOLID:
	w->solid->render = TD_SOLID;
	break;
    case TD_SOLID:
	w->solid->render = TD_EDGES_ONLY;
	break;
    case TD_EDGES_ONLY:
	w->solid->render = TD_MESH;
	break;
    }
    Credraw3dobject ("3dview", 0);
    return 0;
}

#define CK_Save 		101

int Cdraw3d_from_text (const char *ident, const char *text);

int cb_editor (CWidget *editor, XEvent *xevent, CEvent *cwevent)
{
    char *t;
    long tlen;

    if((cwevent->key == XK_r || cwevent->key == XK_R) && (cwevent->state & Mod1Mask)) {
	Cclear_all_surfaces("3dview");
	Cedit_execute_command(editor->editor, CK_Save, -1);
	t = loadfile(path_compress (editor->editor->dir, editor->editor->filename), &tlen);
	if(t && tlen) {
	    Cdraw3d_from_text("3dview", t);
	} else
	    Cerrordialog (0, 0, 0, "Get Surface", \
"Error reloading file \"%s\" in call to cb_editor - check file permissions.", \
			    editor->editor->filename);
    }
    return 0;
}

int cb_3dplane (CWidget *w, XEvent * xevent, CEvent * cwevent)
{
    static int y3dprev = 0, x3dprev = 0;
    static float alphaprev = 0, betaprev = 0;
    if (cwevent->type == ButtonPress && cwevent->button == Button1) {
	x3dprev = cwevent->x;
	y3dprev = cwevent->y;
	alphaprev = w->solid->alpha;
	betaprev = w->solid->beta;
    }
    if ((cwevent->type == MotionNotify && (cwevent->state & Button1Mask)) || 
		(cwevent->type == ButtonRelease && cwevent->button == Button1)) {
w->solid->alpha = (float) alphaprev + (float) ((float) cwevent->x - x3dprev) / 100;
w->solid->beta = (float) betaprev + (float) ((float) cwevent->y - y3dprev) / 100;
	Credraw3dobject ("3dview", 0);
    }
    return 0;
}

