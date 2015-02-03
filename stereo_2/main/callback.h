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
#ifndef _CALLBACK_H
#define _CALLBACK_H

void move_marker (Picture *p, CEvent *e);
void remove_marker (Picture *p, CEvent *e);
void insert_marker (Picture *p, CEvent *e);
int cb_calibrate (CWidget * w, XEvent *xe, CEvent *ce);
int cb_loadcal (CWidget * w, XEvent *xe, CEvent *ce);
int cb_zoomimage (CWidget * w, XEvent * xe, CEvent * ce);
int cb_mainimage (CWidget * w, XEvent * xe, CEvent * ce);
int cb_newimage (CWidget * w, XEvent *xe, CEvent *ce);
int cb_killimage (CWidget * w, XEvent *xe, CEvent *ce);
int cb_clearallmarkers (CWidget *none, XEvent * xe, CEvent * ce);
int cb_removelastmarker (CWidget *none, XEvent * xe, CEvent * ce);
int cb_newrender (CWidget *w, XEvent * xevent, CEvent * cwevent);
int cb_editor (CWidget *editor, XEvent *xevent, CEvent *cwevent);
int cb_3dplane (CWidget *w, XEvent * xevent, CEvent * cwevent);
int cb_draw_cam_data (CWidget * w, XEvent *xe, CEvent *ce);
int cb_showcal (CWidget * w, XEvent *xe, CEvent *ce);
int cb_leave (CWidget * w, XEvent *xe, CEvent *ce);
int cb_sigma (CWidget * w, XEvent *xe, CEvent *ce);
int cb_save_desktop (CWidget * w, XEvent *xe, CEvent *ce);
int cb_load_desktop (CWidget * w, XEvent *xe, CEvent *ce);
int cb_getsurface (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getcircle (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getellipse (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getline (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_save_window (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getcylinderedge (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getlineedge (CWidget * none, XEvent * xevent, CEvent * cwevent);
int cb_getcircleedge (CWidget * none, XEvent * xevent, CEvent * cwevent);

#endif

