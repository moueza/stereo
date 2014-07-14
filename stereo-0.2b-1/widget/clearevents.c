/*
   Copyright (C) 1996 Paul Sheer

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
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "dirtools.h"

#include "coolwidget.h"

#include "mad.h"
/*
#ifndef XPointer
#define XPointer char *
#endif
*/
/*this processes all remaining widget events (usually expose) in the event
   queue at the time it was called. */

/*This routine does NOT wait if there are no events left */

/*it returns true if the widget event is associated with the given
   event identifier 'compare' and in this case, will return the entire
   cw event string in return event. */

/*NULL vars can be given if you just want it to process the remaining
   events */


int Ccheckifevent (XEvent *xevent, CEvent *cwevent)
{
    char ident[32];
    int found = 0;
    CEvent eventcw;
    XEvent eventx;

    while(CPending()) {
	CNextEvent (&eventx, &eventcw);
	if (cwevent && xevent) {
	    strcpy(ident, eventcw.ident);
	    if (!strcmp (ident, cwevent->ident)) {
		memcpy(cwevent, &eventcw, sizeof(CEvent));
		memcpy(xevent, &eventx, sizeof(XEvent));
		found = 1;
	    }
	}
    }

    if(!found && cwevent && xevent) {
	memset(cwevent, 0, sizeof(CEvent));
	memset(xevent, 0, sizeof(XEvent));
    }

    return found;
}

Bool keypredicate (Display * d, XEvent * xevent, XPointer w)
{
    if (xevent->xany.type == KeyPress || xevent->xany.type == KeyRelease)
	if (((CWidget *) w)->winid == xevent->xany.window)
	    return 1;
    return 0;
}


int CKeyPending(const char *ident)
{
    XEvent xevent;
    int i = 0;
    if(CPending())
	i = XCheckIfEvent (CDisplay, &xevent, keypredicate, (XPointer) Cwidget(ident));
    if(i)
	XPutBackEvent(CDisplay, &xevent);
    return i;
}



Bool mousepredicate (Display * d, XEvent * xevent, XPointer w)
{
    if (((CWidget *) w)->winid == xevent->xany.window)
	if (xevent->xany.type == ButtonPress
	    || xevent->xany.type == ButtonRelease
	    || (xevent->xany.type == MotionNotify
		&& (xevent->xmotion.state
		    & (Button1Mask | Button2Mask | Button3Mask
		       | Button4Mask | Button5Mask))))
	    return 1;
    return 0;
}


int CMousePending(const char *ident)
{
    XEvent xevent;
    int i = 0;
    if(CPending())
	i = XCheckIfEvent (CDisplay, &xevent, mousepredicate, (XPointer) Cwidget(ident));
    if(i)
	XPutBackEvent(CDisplay, &xevent);
    return i;
}


