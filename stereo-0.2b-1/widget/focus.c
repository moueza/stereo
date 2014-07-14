/* focus.c - records a history of focusses for reverting focus. also does focus cycling

   Copyright (C) 1997 Paul Sheer

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

#include "stringtools.h"
#include "app_glob.c"

#include "coolwidget.h"
#include "coollocal.h"

#include "mad.h"

/* Focus stack: This stack remembers the focus history
so that when widgets are destroyed, the focus returns to
the most recent widget in the history. This is equivalent
to the revert_to in the XSetInputFocus() function,
however XSetInputFocus() has an effective history of
only one.

The commands to change focus are CFocusWindow(Window win)
and CFocus(CWidget *w)
*/

#define FOCUS_STACK_SIZE 128
static Window focus_stack[FOCUS_STACK_SIZE];
static int focus_sp = 0;
Window current_focus = -1;

void add_to_focus_stack (Window w)
{
    int i;
    i = focus_sp;
    while (i--)
	if (focus_stack[i] == w) {
	    focus_sp = i + 1;
	    return;
	}
    if (focus_sp >= FOCUS_STACK_SIZE) {
#ifdef FOCUS_DEBUG
	printf ("add_to_focus_stack(): focus_sp overflow\n");
#endif
	return;
    }
    focus_stack[focus_sp++] = w;
#ifdef FOCUS_DEBUG
    printf ("add_to_focus_stack(): focus_sp = %d\n", focus_sp);
#endif
}

void focus_stack_remove_window (Window w)
{
    int i;
    i = focus_sp;
    while (i--)
	if (focus_stack[i] == w) {
	    focus_stack[i] = 0;
	    while (focus_sp && !focus_stack[focus_sp - 1])
		focus_sp--;
#ifdef FOCUS_DEBUG
    printf ("focus_stack_remove_window(): focus_sp = %d\n", focus_sp);
#endif
	    return;
	}
}

Window CGetFocus(void)
{
    return current_focus;
}

void CFocusLast (void)
{
    Window w;
    if (!focus_sp)
	return;
    w = focus_stack[focus_sp - 1];
    if (w == current_focus)
	return;
    if (w)
	CFocusWindow (w);
}

/* set input focus */
/* 
   If the window is not yet mapped (the 'case MappingNotify:' statement below
   sets the mapped member) then CFocus() sets FOCUS_WHEN_MAPPED and CNextEvent
   will focus on the window when it gets MappingNotify.
   This is to prevent XLib from reporting an error if we focus on a
   window that the server has non created.
*/

/* CFocusWindow() is a macro for */
void CFocusWindowNormal (Window win)
{
    if (win == (Window) -1 || !win)
	return;
    XSetInputFocus(CDisplay, win, RevertToNone, CurrentTime);
    add_to_focus_stack (win);
    current_focus = win;
}

void CFocusWindowDebug (Window win, int line, char *file)
{
    printf ("CFocusWindow(): %s:%d\n", file, line);
    CFocusWindowNormal (win);
}

/* CFocus() is a macro for */
void CFocusNormal (CWidget *w)
{
    if (!w)
	return;
    if(w->mapped & MAPPED) {
	CFocusWindowNormal (w->winid);
    } else {
	w->mapped |= FOCUS_WHEN_MAPPED;
    }
}

void CFocusDebug (CWidget *w, int line, char *file)
{
    printf ("CFocus(): %s:%d\n", file, line);
    CFocusNormal (w);
}


/* get next sibling of w that has takes_focus set (i.e. that takes user input of any sort) */
CWidget *CNextFocus (CWidget * w)
{
    int i, j;
    i = j = Cfindnextchildof (w->parentid, w->winid);
    for (;;) {
	if (!i) {
	    i = Cfindfirstchildof (w->parentid);
	    if (!i)
		return 0;
	}
	if (CW (i)->takes_focus && !CW(i)->disabled)
	    return CW (i);
	w = CW (i);
	i = Cfindnextchildof (w->parentid, w->winid);
	if (i == j) /* done a round trip */
	    return 0;
    }
}

/* previous sibling of same */
CWidget *CPreviousFocus (CWidget * w)
{
    int i, j;
    i = j = Cfindpreviouschildof (w->parentid, w->winid);
    for (;;) {
	if (!i) {
	    i = Cfindlastchildof (w->parentid);
	    if (!i)
		return 0;
	}
	if (CW (i)->takes_focus && !CW(i)->disabled)
	    return CW (i);
	w = CW (i);
	i = Cfindpreviouschildof (w->parentid, w->winid);
	if (i == j) /* done a round trip */
	    return 0;
    }
}

/* first child of widget that takes focus (eg w is a window and
    a button in the window is returned) */
CWidget *CChildFocus (CWidget * w)
{
    int j, i = Cfindfirstchildof (w->winid);
    if(!i)
	return 0;
    w = CW (i);
    if(w->takes_focus)
	return w;
    j = i = Cfindnextchildof (w->parentid, w->winid);
    for (;;) {
	if (!i) {
	    i = Cfindfirstchildof (w->parentid);
	    if (!i)
		return 0;
	}
	if (CW (i)->takes_focus)
	    return CW (i);
	w = CW (i);
	i = Cfindnextchildof (w->parentid, w->winid);
	if (i == j) /* done a round trip */
	    return 0;
    }
}

/* search for two generations down for the first descendent that is a widget.
   If it does not take focus, then its first child is focussed.
   If it has no children, the next descendent is searched for. */
CWidget *CFindFirstDescendent (Window win)
{
    int i, j;

    i = Cfindfirstchildof (win);
    if (i) {			/* is it a child ? */
	if (CW (i)->takes_focus && !CW (i)->disabled) {
	    return (CW (i));
	} else {
	    CWidget *w;
	    w = CChildFocus (CW (i));
	    if (w)
		return w;
	}
    } else {			/* not a child */
	Window root, parent, *children = 0;
	unsigned int nchildren = 0;
	XQueryTree (CDisplay, win, &root, &parent, &children, &nchildren);
	if (!nchildren) {
	    if (children)
		XFree (children);
	    return 0;
	}
	for (j = 0; j < nchildren; j++)
	    if ((i = Cfindfirstchildof (children[j]))) {	/* is it a grandchild ? */
		if (CW (i)->takes_focus && !CW (i)->disabled) {
		    XFree (children);
		    return (CW (i));
		} else {
		    CWidget *w;
		    w = CChildFocus (CW (i));
		    if (w) {
			XFree (children);
			return w;
		    }
		}
	    }
	XFree (children);
    }
    return 0;			/* not a grandchild */
}
