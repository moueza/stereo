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

/* #define DEBUG */

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
#include "coollocal.h"

#include "drawings.h"
#include "edit.h"
#include "editcmddef.h"
#include "widget3d.h"

#include "mad.h"

int last_region = 0;


int (*global_alarm_callback) (CWidget *, XEvent *, CEvent *);
extern Pixmap Cswitchon, Cswitchoff;
extern Window current_focus;
extern Atom DndProtocol;

void add_to_focus_stack (Window w);

static Window window_is_resizing = 0;

/* converts button presses from buttons 2 through 5 to button presses from 2 only, also gets double clicks */
void Cresolvebutton (XEvent * xevent, CEvent * cwevent)
{
    static Time time = 0;
    static Window window = 0;
    static int x, y;
    long t;
    unsigned long s;
    switch (xevent->type) {
    case ButtonRelease:
    case ButtonPress:
	cwevent->state = s = xevent->xbutton.state;
	if (s & (Button2Mask | Button3Mask | Button4Mask | Button5Mask))
	    cwevent->state |= Button2Mask;
	cwevent->button = xevent->xbutton.button;
	if (cwevent->button == Button2 || cwevent->button == Button3 ||
	    cwevent->button == Button4 || cwevent->button == Button5)
	    cwevent->button = Button2;
	cwevent->x = xevent->xbutton.x;
	cwevent->y = xevent->xbutton.y;
	t = xevent->xmotion.time - time;
	if (abs(t) < 250 &&
	    window == xevent->xany.window &&
	    abs (x - cwevent->x) < 4 && abs (y - cwevent->y) < 4)
	    cwevent->double_click = 1;
	x = xevent->xbutton.x;
	y = xevent->xbutton.y;
	time = xevent->xbutton.time;
	break;
    case MotionNotify:
	cwevent->state = s = xevent->xmotion.state;
	if (s & (Button2Mask | Button3Mask | Button4Mask | Button5Mask))
	    cwevent->state |= Button2Mask;
	x = cwevent->x = xevent->xmotion.x;
	y = cwevent->y = xevent->xmotion.y;
	break;
    }
    window = xevent->xany.window;
}

/*sends all windows that are marked top_bottom = 1 to the bottom */
void Clowerwindows ()
{
    int j = 0;
    while (CLastwidget > j++)
	if (CW (j) != NULL)
	    if (CW (j)->position & CALWAYS_ON_BOTTOM)
		XLowerWindow (CDisplay, CW (j)->winid);
}

/*sends all windows that are marked top_bottom = 2 to the top */
void Craisewindows ()
{
    int j = 0;
    while (CLastwidget > j++)
	if (CW (j) != NULL)
	    if (CW (j)->position & CALWAYS_ON_TOP)
		XRaiseWindow (CDisplay, CW (j)->winid);
}



/* {{{ here is an internal event queue handler to send events without going through XLib */

/*
   We want to be able to send our own events internally
   because XSendEvent sometimes doesn't force the an
   event to be processed and the event sits on the queue
   with its thumb up its arse.
 */

#define NUM_EVENTS_CACHED (1 << 9)

static unsigned int event_send_last = 0;
static unsigned int event_read_last = 0;
static XEvent event_sent[NUM_EVENTS_CACHED];

#define queue_size (event_send_last - event_read_last >= 0 ? \
	    event_send_last - event_read_last : \
	    NUM_EVENTS_CACHED + event_send_last - event_read_last)

/* returns 0, if buffer is full, else returns 1 */
int push_event (XEvent * ev)
{
    if (event_read_last == (event_send_last + 1) % NUM_EVENTS_CACHED) {		/* no more space */
#ifdef DEBUG
	fprintf (stderr, "cooledit:%s:%d: warning event stack full\n", __FILE__, __LINE__);
#endif
	/* we are just going to ignore this */
	return 0;
    }
    if (ev->type == Expose || ev->type == InternalExpose) {	/* must handle expose counts also */
	unsigned int i = (event_send_last - 1) % NUM_EVENTS_CACHED;
	XEvent *e;
	ev->xexpose.count = 0;	/* this is the very last expose by definition */
	while (i != ((event_read_last - 1) % NUM_EVENTS_CACHED)) {	/* search backwards until a similar event is found */
	    if ((e = &(event_sent[i]))->xany.window == ev->xany.window) {
		if (e->type == ev->type) {
		    e->xexpose.count = 1;	/* we are not going to actually "count", but we must indicate if the queue isn't empty with a "1" */
		    break;
		}
	    }
	    i = (i - 1) % NUM_EVENTS_CACHED;
	}
    }
    memcpy (&event_sent[event_send_last], ev, sizeof (XEvent));
    event_send_last = (event_send_last + 1) % NUM_EVENTS_CACHED;
    return 1;
}



/* pops the oldest event, returns 0 if empty */
int pop_event (XEvent * ev)
{
    if (event_read_last == event_send_last)
	return 0;		/* "stack" is empty */
    memcpy (ev, &event_sent[event_read_last], sizeof (XEvent));
    event_read_last = (event_read_last + 1) % NUM_EVENTS_CACHED;
    return 1;
}

/* use this instead of XSextEvent to send an event to your own application */
int CSendEvent (XEvent * e)
{
    return push_event (e);
}

int CQueueSize ()
{
    return queue_size;
}

/* returns nonzero if pending on either internal or X queue */
int CPending ()
{
    if (queue_size)
	return 1;
    if (XPending (CDisplay))
	return 1;
    return 0;
}

/* searches the local queue for an event matching the window */
/* does not remove the event, returns non-zero if found */
int CWindowPending (Window w)
{
    unsigned int i = (event_send_last - 1) % NUM_EVENTS_CACHED;
    while (i != ((event_read_last - 1) % NUM_EVENTS_CACHED)) {
	if ((&(event_sent[i]))->xany.window == w)
	    return 1;
	i = (i - 1) % NUM_EVENTS_CACHED;
    }
    return 0;
}



/* 
   does checks for expose events pending, if there is one on either queue then
   it removes it and returns it.
 */
int CExposePending (Window w, XEvent * ev)
{
    XEvent *e;
    unsigned int i = event_read_last;

    while (i != event_send_last) {
	if ((e = &(event_sent[i]))->xany.window == w)
	    if (e->type == Expose) {
		memcpy (ev, e, sizeof (XEvent));
		e->type = 0;
		return 1;
	    }
	i = (i + 1) % NUM_EVENTS_CACHED;
    }
    return XCheckWindowEvent (CDisplay, w, ExposureMask, ev);
}




/* send an expose event via the internal queue */
int CSendExpose (Window win, int x, int y, int w, int h)
{
    XEvent e;
    e.xexpose.type = Expose;
    e.xexpose.serial = 0;
    e.xexpose.send_event = 1;
    e.xexpose.display = CDisplay;
    e.xexpose.window = win;
    e.xexpose.x = x;
    e.xexpose.y = y;
    e.xexpose.width = w;
    e.xexpose.height = h;
    return CSendEvent (&e);
}


/* }}} end of internal queue handler */


/* {{{ here is an expose caching-amalgamating stack system */

typedef struct {
    short x1, y1, x2, y2;
    Window w;
    long error;
    int count;
} CRegion;

#define MAX_NUM_REGIONS 63
static CRegion regions[MAX_NUM_REGIONS + 1];

#define area(c) abs(((c).x1-(c).x2)*((c).y1-(c).y2))

static CRegion add_regions (CRegion r1, CRegion r2)
{
    CRegion r;
    r.x2 = max (max (r1.x1, r1.x2), max (r2.x1, r2.x2));
    r.x1 = min (min (r1.x1, r1.x2), min (r2.x1, r2.x2));
    r.y2 = max (max (r1.y1, r1.y2), max (r2.y1, r2.y2));
    r.y1 = min (min (r1.y1, r1.y2), min (r2.y1, r2.y2));
    r.w = r2.w;
    r.error = (long) area (r) - area (r1) - area (r2);
    r.error = max (r.error, 0);
    r.error += r1.error + r2.error;
    r.count = min (r1.count, r2.count);
    return r;
}

/* returns 1 when the stack is full, 0 otherwise */
static int push_region (XExposeEvent * e)
{
    CRegion p;


    p.x1 = e->x;
    p.x2 = e->x + e->width;
    p.y1 = e->y;
    p.y2 = e->y + e->height;
    p.w = e->window;
    p.error = 0;
    p.count = e->count;

    if (last_region) {		/* this amalgamates p with a region on the stack of the same window */
	CRegion q;
	int i;
	for (i = last_region - 1; i >= 0; i--) {
	    if (regions[i].w == p.w) {
		q = add_regions (regions[i], p);
		if (q.error < 100) {
		    regions[i] = q;	/* amalgamate region, else... */
		    return 0;
		}
	    }
	}
    }

    regions[last_region++] = p;		/* ...store a completely new region */
    if (last_region >= MAX_NUM_REGIONS) {
	printf ("push_region(): last_region >= MAX_NUM_REGIONS\n");
	return 1;
    }
    return 0;
}

/*
   Pops the first region matching w, if w == 0 then pops 
   the first region, returns 1 on empty . (It actually 
   searches the "stack" from the bottom to the top so
   that exposes are returned in order.)
 */
static int pop_region (XExposeEvent * e, Window w)
{
    e->type = 0;
    if (last_region) {
	int i = 0;
	if (w == 0)
	    goto any_window;
	for (i = last_region - 1; i >= 0; i--) {
	    if (regions[i].w == w) {
	      any_window:;
		e->type = Expose;
		e->serial = e->send_event = 0;
		e->display = CDisplay;
		e->window = regions[i].w;
		e->x = min (regions[i].x1, regions[i].x2);
		e->y = min (regions[i].y1, regions[i].y2);
		e->width = abs (regions[i].x1 - regions[i].x2);
		e->height = abs (regions[i].y1 - regions[i].y2);
		e->count = regions[i].count;
		last_region--;
		memmove (&(regions[i]), &(regions[i + 1]), (last_region - i) * sizeof (CRegion));
		return 0;
	    }
	}
    }
    return 1;
}

static void pop_all_regions (Window w)
{
    XEvent e;
    while (!pop_region (&(e.xexpose), w)) {
	e.type = InternalExpose;
	CSendEvent (&e);
    }
}


/* }}} end expose amalgamation stack system */


/* {{{ key conversion utuilities */

/* just get a keysym (one of eg XK_... in keysymdef.h */
KeySym CKeySym (XEvent * e)
{
    char xlat;
    KeySym key;
    if (e->type == KeyPress || e->type == KeyRelease) {
	XLookupString (&(e->xkey), &xlat, 1, &key, NULL);
	return key;
    } else
	return 0;
}

static unsigned long toggle_bit (unsigned long x, unsigned long mask)
{
    unsigned long m = -1;
    if (x & mask)
	return x & (m - mask);
    else
	return x | mask;
}


/* get a 15 bit "almost unique" key sym that includes keyboard modifier
   info in the top 3 bits */
short CKeySymMod (XEvent * ev)
{
    KeySym p;
    XEvent e;
    int state;
    e = *ev;
    state = e.xkey.state;
    e.xkey.state = 0;		/* want the raw key */
    p = CKeySym (&e);
    if (p && p != XK_Control_L && p != XK_Control_R && p != XK_Shift_L && p != XK_Shift_R && p != XK_Alt_L && p != XK_Alt_R) {
	p = toggle_bit (p, 0x1000 * ((state & ShiftMask) != 0));
	p = toggle_bit (p, 0x2000 * ((state & ControlMask) != 0));
	p = toggle_bit (p, 0x4000 * ((state & Mod1Mask) != 0));
	p &= 0x7FFF;
    } else
	p = 0;
    return p;
}

/* }}} key conversion utilities */


/* {{{ focus cycling */

#define is_focus_change_key(k) \
    ((k) == XK_Tab || (k) == XK_KP_Tab || (k) == XK_ISO_Left_Tab || (k) == XK_Down || \
    (k) == XK_Up || (k) == XK_Left || (k) == XK_Right || (k) == XK_KP_Down || \
    (k) == XK_KP_Up || (k) == XK_KP_Left || (k) == XK_KP_Right)

#define is_focus_prev_key(k,state) \
    ((k) == XK_ISO_Left_Tab || (((state) & ShiftMask) && \
    ((k) == XK_Tab || (k) == XK_KP_Tab)) || (k) == XK_Left \
    || (k) == XK_Up || (k) == XK_KP_Left || (k) == XK_KP_Up)

#define is_focus_next_key(k,state) not_needed

/*
   This shifts focus to the previous or next sibling widget.
   (usually the tab key is used, but also responds to up, down,
   left and right.)
 */
static int CCheckTab (XEvent * xevent, CEvent * cwevent)
{
    if (xevent->type == KeyPress) {
	KeySym k;
	CWidget *w;
	k = CKeySym (xevent);
	if (!is_focus_change_key (k))
	    return 0;

	w = CW (CWidgetOf (xevent->xany.window));

	if (!w)
	    CFocus (CFindFirstDescendent (xevent->xany.window));
	else if (!w->takes_focus)
	    CFocus (CChildFocus (w));
	else if (is_focus_prev_key (k, xevent->xkey.state))
	    CFocus (CPreviousFocus (w));
	else
	    CFocus (CNextFocus (w));
	return 1;
    }
    return 0;
}

#define r_lcase(x) (((x) >= 'A' && (x) <= 'Z') ? (x) + 'a' - 'A' : (x))

/*
   Check for hot keys of buttons, sends a ButtonPress to the button if the key found.
 */
static int CCheckButtonHotKey (XEvent * xevent, CEvent * cwevent)
{
    if (xevent->type == KeyPress) {
	KeySym k;
	CWidget *w, *p;
	char xlat;
	XLookupString (&(xevent->xkey), &xlat, 1, &k, NULL);
	if (xlat < ' ' || xlat > '~')
	    return 0;
	w = CW (CWidgetOf (xevent->xany.window));
	if (!w)
	    w = CFindFirstDescendent (xevent->xany.window);
	else if (!w->takes_focus)
	    w = CChildFocus (w);
	p = w;
	do {
	    if (!w)
		return 0;
	    if (r_lcase (w->hotkey) == r_lcase (xlat)) {
		XEvent e;
		CFocus (w);
		memset (&e, 0, sizeof (XEvent));
		e.xbutton.type = ButtonPress;
		e.xbutton.display = CDisplay;
		e.xbutton.window = w->winid;
		e.xbutton.button = Button1;
		CSendEvent (&e);
		e.xbutton.type = ButtonRelease;
		CSendEvent (&e);
		return 1;
	    }
	    w = CNextFocus (w);	/* check all sibling buttons for a hotkey */
	} while ((unsigned long) w != (unsigned long) p);
    }
    return 0;
}

/* checks all widgets for a hotkey if alt is pressed */
static int CCheckGlobalHotKey (XEvent * xevent, CEvent * cwevent)
{
    if (xevent->type == KeyPress && (xevent->xkey.state & Mod1Mask)) {
	int i = CLastwidget;
	KeySym k;
	CWidget *w;
	char xlat;
	XLookupString (&(xevent->xkey), &xlat, 1, &k, NULL);
	if (xlat < ' ' || xlat > '~')
	    return 0;

	while (--i > 0)
	    if ((w = CW (i)) != NULL)
		if (w->takes_focus && !w->disabled)
		    if (r_lcase (w->hotkey) == r_lcase (xlat)) {
			XEvent e;
			CFocus (w);
			memset (&e, 0, sizeof (XEvent));
			e.xbutton.type = ButtonPress;
			e.xbutton.display = CDisplay;
			e.xbutton.window = w->winid;
			e.xbutton.button = Button1;
			CSendEvent (&e);
			e.xbutton.type = ButtonRelease;
			CSendEvent (&e);
			return 1;
		    }
    }
    return 0;
}


/* }}} */

/* sets the mapped member of widget whose window is w, returning the previous state */
static int set_mapped (Window w, int i)
{
    int x, y;
    x = CWidgetOf (w);
    if (!x)
	return i;
    y = CW (x)->mapped;
    CW (x)->mapped = i;
    return y;
}


/*
   This is the core of the library. CNextEvent must be called continually.
   The calling application must only use CNextEvent as a block.
   CNextEvent does the following in sequence:

   1 check if event is AlarmEvent, yes: toggle cursor and return
   2 check MappingNotify and return
   3 cache expose event for later handling. No "raw" exposes are ever processed.
   they are first merged into courser exposes and resent via the internal
   queue; return
   4 check if an internal expose resulting from 3 above. If so rename it to Expose
   and continue
   5 look for a widget whose window matches .xany.window. If the widget is not
   a picture widget then call the widget event handling routine: eh_*
   Then call the widgets user event handler: ->callback
   6 do the same for picture widgets. These must come last so that the get drawn
   on top of other things if there is for exampla a image and a picture in
   the same window.
   8 if the event was a key event, and none of the handlers returned 1
   check the tab key for focus cycling.
 */


/* xevent or cwevent or both my be passed as NULL */
void CNextEvent (XEvent * xevent, CEvent * cwevent)
{
    static char idle = 1;
    static char no_ident[33];
    int i = 0;
    int handled = 0;
    CWidget *w;
    XEvent private_xevent;
    CEvent private_cwevent;
    Window win;
    static Window drop_window = 0;
    int type;
    static Window last_events_window1 = -2;
    static Window last_events_window2 = -2;
    static int last_widget1 = 0;
    static int last_widget2 = 0;

    if (!xevent)
	xevent = &private_xevent;
    if (!cwevent)
	cwevent = &private_cwevent;

    if (!CPending ()) {		/* We want to make sure XNextEvent never blocks waiting for an event */
	pop_all_regions (0);	/* just make sure not outstanding exposes */
	pause ();
    }
    if (!pop_event (xevent))	/* first check our own events */
	XNextEvent (CDisplay, xevent);	/* if none of our own coming, _then_ we look at the server */

    memset (cwevent, 0, sizeof (CEvent));
    memset (no_ident, 0, 33);
    cwevent->text = no_ident;
    cwevent->ident = no_ident;

    win = xevent->xany.window;
    type = xevent->type;

    switch (type) {
    case TickEvent:
	if (idle == 1)		/* this will XSync at the end of a burst of events */
	    XSync (CDisplay, 0);	/* this is the only place in the library where XSync is called */
	idle++;
	return;
    case AlarmEvent:
	cwevent->type = AlarmEvent;
	Ctoggle_cursor ();
	if (global_alarm_callback) {
	    cwevent->type = type;
	    cwevent->kind = CALARM_WIDGET;
	    (*(global_alarm_callback)) (0, xevent, cwevent);
	}
	return;
    case MappingNotify:
	XRefreshKeyboardMapping (&(xevent->xmapping));
	break;
    case Expose:{
	    XEvent eev;
/* here we amalgamate exposes of the same window together and re-send them as InternalExpose events */
	    if (push_region (&(xevent->xexpose))) {
		pop_all_regions (win);
	    } else {
		for (;;) {
		    if (CExposePending (win, &eev)) {
			if (!push_region (&(eev.xexpose)))
			    continue;
		    }
		    pop_all_regions (win);
		    break;
		}
	    }
	}
	return;
    case InternalExpose:
	type = xevent->type = Expose;
	break;
    case EnterNotify:
/* The dnd drag will trap all events except enter and leave. These can
   be used to trace which window the pointer has gotten into during
   a drag. */
	drop_window = xevent->xbutton.window;
	break;
    case MapNotify:
	if (set_mapped (xevent->xmap.window, MAPPED) & FOCUS_WHEN_MAPPED)
	    CFocusWindow (xevent->xmap.window);
	break;
    case FocusOut:
	if (current_focus == win)
	    current_focus = -1;
	break;
    case FocusIn:		/* if some other app caused a focus,
				   we must update our local focus history */
	{
	    CWidget *w;
	    w = CW (CWidgetOf (win));
	    if (w)
		if (w->takes_focus && !w->disabled) {
		    add_to_focus_stack (win);
		    current_focus = win;
		    break;
		}
	    break;
	}
/* here we want key presses to go to our idea of what window is focussed,
   no the window manager's, or the X Server's idea. */
    case KeyPress:
    case KeyRelease:{
	    Window w;
	    w = CGetFocus ();
	    if (w != -1 && w)
		win = xevent->xany.window = w;
	    break;
	}
    case UnmapNotify:
	set_mapped (xevent->xmap.window, 0);
	break;
    case ClientMessage:
/* If we recieve a drop from dnd, we need to find the window in which the
   drop occurred. This will be the last window with an EnterNotify (above).
   Now we find the pointer coords relative to that window, and change the
   event to go to that window */
	if (xevent->xclient.message_type == DndProtocol && xevent->xclient.data.l[4] == 1) {
	    int x, y, rx, ry;
	    Window root, child;
	    unsigned int mask;
	    win = xevent->xclient.window = drop_window;
	    XQueryPointer (CDisplay, drop_window, &root, &child, &rx, &ry, &x, &y, &mask);
	    xevent->xclient.data.l[3] = (long) x + (long) y * 65536L;
	}
	break;
    }

    idle = 0;

    if (last_events_window1 == win && CW (last_widget1))	/* this will speed up the search a bit */
	i = last_widget1 - 1;					/* by remembering the last two windows */
    else if (last_events_window2 == win && CW (last_widget2))
	i = last_widget2 - 1;

/*Now find if the event belongs to any of the widgets */
    while (CLastwidget > i++) {
	if (!(w = CW (i)))
	    continue;
	if (w->winid != win)
	    continue;
	if (w->disabled && type != Expose)
	    break;
	if (w->kind == CPICTURE_WIDGET)
	    continue;

	last_widget2 = last_widget1;
	last_widget1 = i;
	last_events_window2 = last_events_window1;
	last_events_window1 = win;

	cwevent->type = type;
	cwevent->kind = w->kind;
	cwevent->window = win;

	if (w->eh)
	    handled |= (*(w->eh)) (w, xevent, cwevent);
	if (w->callback && cwevent->ident[0]) {		/*irrelevent eh's won't set ident */
	    handled |= (*(w->callback)) (w, xevent, cwevent);
	}

	break;
    }

#ifdef HAVE_PICTURE

    i = 0;
/* picture exposes must come last so that they can be drawn on top of
   other widgets */
    while (CLastwidget > i++) {
	if (!(w = CW (i)))
	    continue;
	if (w->kind != CPICTURE_WIDGET)
	    continue;
	if (w->parentid != xevent->xany.window)
	    continue;
	if (w->disabled && type != Expose)
	    continue;
	cwevent->type = type;
	cwevent->kind = w->kind;
	cwevent->window = xevent->xany.window;

	if (w->eh)
	    handled |= (*(w->eh)) (w, xevent, cwevent);
	if (w->callback && cwevent->ident[0]) {		/*relevent events will set ident */
	    handled |= (*(w->callback)) (w, xevent, cwevent);
	}
	/*break; *//*no break here 'cos there may be two picture widgets in the same window */
    }

#endif		/* ! HAVE_PICTURE */

    if (type == KeyPress) {
	cwevent->handled = handled;
	if (!handled)
	    handled = CCheckTab (xevent, cwevent);
	if (!handled)
	    handled = CCheckButtonHotKey (xevent, cwevent);
	if (!handled)
	    handled = CCheckGlobalHotKey (xevent, cwevent);
    }
#ifdef DEBUG
    Ccheck ();
#endif
}





/*-----------------------------------------------------------------------*/
int eh_button (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case FocusIn:
	Crenderbutton (w);
    case FocusOut:
	Crenderbutton (w);
	Cexposewindowarea (w->parentid, 0, w->x - FOCUS_RING, w->y - FOCUS_RING, w->width + FOCUS_RING * 2, w->height + FOCUS_RING * 2);
	break;
    case MotionNotify:
	break;
    case ButtonPress:
	w->options = CBUTTON_PRESSED;
	CFocus (w);
	Crenderbutton (w);
	break;
    case KeyPress:{
	    int c = CKeySym (xevent);
	    cwevent->key = c;
	    if (c != XK_Return && c != XK_space)
		break;
	}
    case ButtonRelease:
	w->options = CBUTTON_HIGHLIGHT;
	Crenderbutton (w);
	cwevent->ident = w->ident;	/* return the event */
	return 1;
    case EnterNotify:
	w->options = CBUTTON_HIGHLIGHT;
	Crenderbutton (w);
	break;
    case Expose:
	if (xevent->xexpose.count)
	    break;
    case LeaveNotify:
	w->options = 0;
	Crenderbutton (w);
	break;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_bitmapbutton (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case ResizeNotify:
	if (w->options == TEXT_CENTRED)
	    Csetwidgetposition (w->ident, 
	    (xevent->xconfigure.width - w->width) / 2 - 4, w->y);
	break;
    case FocusIn:
	Crenderbitmapbutton (w, 0);
    case FocusOut:
	Crenderbitmapbutton (w, 0);
	Cexposewindowarea (w->parentid, 0, w->x - FOCUS_RING, w->y - FOCUS_RING, w->width + FOCUS_RING * 2, w->height + FOCUS_RING * 2);
	break;
    case MotionNotify:
	CFocus (w);
	break;
    case ButtonPress:
	CFocus (w);
	Crenderbitmapbutton (w, 2);
	break;
    case KeyPress:{
	    int c = CKeySym (xevent);
	    cwevent->key = c;
	    if (c != XK_Return && c != XK_space)
		break;
	}
    case ButtonRelease:
	Crenderbitmapbutton (w, 1);
	cwevent->ident = w->ident;
	return 1;
    case EnterNotify:
	Crenderbitmapbutton (w, 1);
	break;
    case Expose:
	if (xevent->xexpose.count)
	    break;
    case LeaveNotify:
	Crenderbitmapbutton (w, 0);
	break;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_switch (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case FocusIn:
	Crenderswitch (w, 0);
    case FocusOut:
	Crenderswitch (w, 0);
	Cexposewindowarea (w->parentid, 0, w->x - FOCUS_RING, w->y - FOCUS_RING, w->width + FOCUS_RING * 2, w->height + FOCUS_RING * 2);
	break;
    case MotionNotify:
	break;
    case ButtonPress:
	CFocus (w);
	Crenderswitch (w, 2);
	break;
    case KeyPress:{
	    int c = CKeySym (xevent);
	    cwevent->key = c;
	    if (c != XK_space)
		break;
	}
    case ButtonRelease:
	w->keypressed = !w->keypressed;
	cwevent->ident = w->ident;
	cwevent->key = w->keypressed;
	Crenderswitch (w, 1);
	return 1;
    case EnterNotify:
	Crenderswitch (w, 1);
	break;
    case Expose:
	if (xevent->xexpose.count)
	    break;
    case LeaveNotify:
	Crenderswitch (w, 0);
	break;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_bitmap (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case Expose:
	if (!xevent->xexpose.count)
	    Crenderbitmapbutton (w, 3);
	break;
    }
    return 0;
}



/*-----------------------------------------------------------------------*/
int eh_window (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    static int windowx, windowy;
    static int wx = 0, wy = 0;
    static int wwidth = 0, wheight = 0;
    static int allowwindowmove = 0;
    static int allowwindowresize = 0;

    switch (xevent->type) {
    case Expose:
	if (!xevent->xexpose.count)
	    Crenderwindow (w);
	break;
    case ButtonRelease:
	window_is_resizing = 0;
	Cresolvebutton (xevent, cwevent);
	allowwindowmove = 0;
	allowwindowresize = 0;
	break;
    case ButtonPress:
	Cresolvebutton (xevent, cwevent);
	if (cwevent->double_click == 1) {
	    CWidget *c = CChildFocus (w);
	    if (c)
		CFocus (c);
	}
	if (cwevent->button == Button1 && !(w->position & CALWAYS_ON_BOTTOM)) {
	    XRaiseWindow (CDisplay, w->winid);
	    Craisewindows ();
	} else if (cwevent->button == Button2 && !(w->position & CALWAYS_ON_TOP)) {
	    XLowerWindow (CDisplay, w->winid);
	    Clowerwindows ();
	}
	windowx = xevent->xbutton.x_root - w->x;
	windowy = xevent->xbutton.y_root - w->y;
	wx = xevent->xbutton.x;
	wy = xevent->xbutton.y;
	wwidth = w->width;
	wheight = w->height;
	if (wx + wy > w->width + w->height - 40 && w->position & CRESIZABLE)
	    allowwindowresize = 1;
	else
	    allowwindowmove = 1;
	break;
    case MotionNotify:
	Cresolvebutton (xevent, cwevent);
	if (!(w->position & CFIXED_POSITION) && allowwindowmove && (cwevent->state & (Button1Mask | Button2Mask))) {
	    w->x = xevent->xmotion.x_root - windowx;
	    w->y = xevent->xmotion.y_root - windowy;
	    if (w->x + xevent->xmotion.x < 2)
		w->x = -wx + 2;
	    if (w->y + xevent->xmotion.y < 2)
		w->y = -wy + 2;
	    XMoveWindow (CDisplay, w->winid, w->x, w->y);
	}
	if ((w->position & CRESIZABLE) && allowwindowresize && (cwevent->state & (Button1Mask | Button2Mask))) {
	    int wi, he;
	    window_is_resizing = w->winid;
	    wi = wwidth + xevent->xmotion.x_root - windowx - w->x;
	    he = wheight + xevent->xmotion.y_root - windowy - w->y;

/* this is actually for the edit windows, and needs to be generalized */
	    if (wi < w->mark1)
		wi = w->mark1;
	    if (he < w->mark2)
		he = w->mark2;

	    wi -= w->firstcolumn;
	    wi -= wi % w->textlength;
	    wi += w->firstcolumn;
	    he -= w->firstline;
	    he -= he % w->numlines;
	    he += w->firstline;

	    Csetwidgetsize (w->ident, wi, he);
	}
	break;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_bar (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case ResizeNotify:	
	Csetwidgetsize (w->ident, xevent->xconfigure.width - WIDGET_SPACING * 2, 3);
	break;
    case Expose:
	if (!xevent->xexpose.count)
	    Crenderbar (w);
	break;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_progress (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case Expose:
	if (!xevent->xexpose.count)
	    Crenderprogress (w);
	break;
    }
    return 0;
}


/*-----------------------------------------------------------------------*/
int eh_text (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case ResizeNotify:
	if (w->options == TEXT_CENTRED)
	    Csetwidgetposition (w->ident, 
	    (xevent->xconfigure.width - w->width) / 2 - 1 - TEXT_RELIEF, w->y);
	break;
    case Expose:
	if (!xevent->xexpose.count)
	    Crendertext (w);
	break;
    }
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_sunken (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    switch (xevent->type) {
    case Expose:
	if (!xevent->xexpose.count)
	    Crendersunken (w);
	break;
    }
    return 0;
}
/*-----------------------------------------------------------------------*/
int eh_bwimage (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
/*      case C8BITIMAGE_WIDGET:
   case CBWIMAGE_WIDGET: */
#ifdef HAVE_BWIMAGE
    switch (xevent->type) {
    case Expose:
	Crenderbwimage (w, xevent->xexpose.x, xevent->xexpose.y, xevent->xexpose.width, xevent->xexpose.height);
	break;
    case ButtonRelease:
    case ButtonPress:
    case MotionNotify:
	Cresolvebutton (xevent, cwevent);
	cwevent->x -= 2;	/*subtract border */
	cwevent->y -= 2;
	cwevent->ident = w->ident;
	break;
    }
#endif
    return 0;
}

/*-----------------------------------------------------------------------*/
int eh_textinput (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    int handled = 0;
    char *intext;
    char xlat = 0;
    int cmd, ch;
    KeySym key;

    switch (xevent->type) {
    case ButtonPress:
	Cresolvebutton (xevent, cwevent);
	CFocusWindow (w->winid);
    case Expose:
	if (xevent->xexpose.count)
	    break;
    case FocusIn:
    case FocusOut:
	Crendertextinput (w);
	break;
    case KeyPress:
	XLookupString (&(xevent->xkey), &xlat, 1, &key, NULL);
	cwevent->ident = w->ident;
	cwevent->key = key;
	cwevent->xlat = xlat;
	cwevent->state = xevent->xkey.state;
	intext = w->text;
	if (edit_translate_key (0, xevent->xkey.keycode, key, xevent->xkey.state, &cmd, &ch)) {
	    cwevent->command = cmd;
	    if (ch > 0) {
		if (w->keypressed) {
		    if (strlen (intext) < w->textlength) {
			memmove (intext + w->cursor + 1, intext + w->cursor, strlen (intext) - w->cursor + 1);
			intext[w->cursor] = ch;
			w->cursor++;
		    }
		} else {
		    w->cursor = 1;
		    intext[0] = ch;
		    intext[1] = 0;
		}
		handled = 1;
	    } else {
		switch (cmd) {
		case CK_BackSpace:
		    if (w->cursor > 0) {
			memmove (intext + w->cursor - 1, intext + w->cursor, strlen (intext) - w->cursor + 1);
			w->cursor--;
		    }
		    w->keypressed = 1;
		    handled = 1;
		    break;
		case CK_Left:
		    if (w->cursor > 0)
			w->cursor--;
		    handled = 1;
		    break;
		case CK_Right:
		    if (w->cursor < strlen (intext))
			w->cursor++;
		    handled = 1;
		    break;
		case CK_Delete:
		    if (w->cursor < strlen (intext))
			memmove (intext + w->cursor, intext + w->cursor + 1, strlen (intext) - w->cursor + 1);
		    handled = 1;
		    break;
		case CK_Home:
		    w->cursor = 0;
		    handled = 1;
		    break;
		case CK_End:
		    w->cursor = strlen (intext);
		    handled = 1;
		    break;
		}
	    }
	    w->keypressed |= handled;
	}
	cwevent->text = w->text;
    }

    Crendertextinput (w);
    return handled;
}

/*-----------------------------------------------------------------------*/
int eh_3d (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
#ifdef HAVE_THREE_D
/* case CTHREED_WIDGET: */
    switch (xevent->type) {
    case Expose:
	Crender3dwidget (w, xevent->xexpose.x, xevent->xexpose.y, xevent->xexpose.width, xevent->xexpose.height);
	break;
    case ButtonRelease:
    case ButtonPress:
    case MotionNotify:
	Cresolvebutton (xevent, cwevent);
	cwevent->x -= 2;	/*subtract border */
	cwevent->y -= 2;
	cwevent->ident = w->ident;
	break;
    }
#endif
    return 0;
}

/*-----------------------------------------------------------------------*/


void linkscrollbartotextbox (CWidget * w, CWidget * textbox, XEvent * xevent, CEvent * cwevent, int whichscrbutton)
{
    int redrawtext = 0;
    if (w->firstline > 65535)
	w->firstline = 65535;
    if (xevent->type == MotionNotify && whichscrbutton == 3) {
	Csettextboxpos (textbox, TEXT_SETLINE, (double) w->firstline * textbox->numlines / 65535.0);
	redrawtext = 1;
    } else if (xevent->type == ButtonPress && (cwevent->button == Button1 || cwevent->button == Button2)) {
	switch (whichscrbutton) {
	case 1:
	    Csettextboxpos (textbox, TEXT_SETLINE, textbox->firstline - (textbox->height / TEXT_PIX_PER_LINE - 2));
	    break;
	case 2:
	    Csettextboxpos (textbox, TEXT_SETLINE, textbox->firstline - 1);
	    break;
	case 5:
	    Csettextboxpos (textbox, TEXT_SETLINE, textbox->firstline + 1);
	    break;
	case 4:
	    Csettextboxpos (textbox, TEXT_SETLINE, textbox->firstline + (textbox->height / TEXT_PIX_PER_LINE - 2));
	    break;
	}
	redrawtext = 1;
    } {
	int count;
	w->firstline = (double) 65535.0 *textbox->firstline / textbox->numlines;
	if (redrawtext) {
	    char *input = catstrs ("sprinp", textbox->ident, 0);
	    Cundrawwidget (input);
	    count = Crendertextbox (textbox, 0);
	} else
	    count = Ccounttextboxlines (textbox, 0);
	w->numlines = (double) 65535.0 *count / textbox->numlines;
    }
}

void linkscrollbartoeditor (CWidget * w, CWidget * editor, XEvent * xevent, CEvent * cwevent, int whichscrbutton);

int eh_vertscroll (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    static int buttonypos;
    static int y;
    static int whichscrbutton = 0;	/*which of the five scroll bar buttons was pressed */

    switch (xevent->type) {
    case LeaveNotify:
    case Expose:
	w->options = 0;
	break;
    case ButtonRelease:
	w->options = 32 + Cscrollwhereis (xevent->xmotion.x, xevent->xmotion.y, w);
	break;
    case ButtonPress:
	Cresolvebutton (xevent, cwevent);
	if (cwevent->button == Button1 || cwevent->button == Button2) {
	    buttonypos = xevent->xbutton.y;
	    y = w->firstline;
	    w->options = whichscrbutton = Cscrollwhereis (cwevent->x, cwevent->y, w);
	    cwevent->ident = w->ident;
	}
	break;
    case MotionNotify:
	Cresolvebutton (xevent, cwevent);
	if (cwevent->state & (Button1Mask | Button2Mask)) {
	    w->options = whichscrbutton;
	    if (whichscrbutton == 3) {
		y += (double) (xevent->xmotion.y - buttonypos) * (double) 65535.0 / (w->height - 10 * w->width / 3 - 10);
		w->firstline = y;
		buttonypos = xevent->xmotion.y;
	    }
	} else
	    w->options = 32 + Cscrollwhereis (xevent->xmotion.x, xevent->xmotion.y, w);
	break;
    default:
	return 0;
    }

    if (cwevent->state & (Button1Mask | Button2Mask) || cwevent->type == ButtonPress) {
	CWidget *editor = (void *) w->editor;
	CWidget *textbox = w->textbox;
/*now check to see if there is a textbox associated with this scrollbar */
	if (textbox)
	    linkscrollbartotextbox (w, textbox, xevent, cwevent, whichscrbutton);
/*now check to see if there is a editor associated with this scrollbar */
	if (editor)
	    linkscrollbartoeditor (w, editor, xevent, cwevent, whichscrbutton);
    }
    if (w->numlines < 0)
	w->numlines = 0;
    if (w->firstline < 0)
	w->firstline = 0;
    if (w->firstline > 65535)
	w->firstline = 65535;
    if (w->firstline + w->numlines >= 65535)
	w->numlines = 65535 - w->firstline;

    if (xevent->type != Expose || !xevent->xexpose.count)
	Crenderscrollbar (w);

    return 0;
}
/*-----------------------------------------------------------------------*/

extern int eh_textbox (CWidget * w, XEvent * xevent, CEvent * cwevent);

int (*Cdefaulthandler (int i)) (CWidget *, XEvent *, CEvent *) {
    switch (i) {
    case CBUTTON_WIDGET:
	return eh_button;
	break;
    case CWINDOW_WIDGET:
	return eh_window;
	break;
    case CBAR_WIDGET:
	return eh_bar;
	break;
    case CSUNKEN_WIDGET:
	return eh_sunken;
	break;
    case CHORSCROLL_WIDGET:
    case CVERTSCROLL_WIDGET:
	return eh_vertscroll;
	break;
    case CTEXTINPUT_WIDGET:
	return eh_textinput;
	break;
    case CTEXTBOX_WIDGET:
	return eh_textbox;
	break;
    case CTEXT_WIDGET:
	return eh_text;
	break;
    case CTHREED_WIDGET:
	return eh_3d;
	break;
    case C8BITIMAGE_WIDGET:
    case CBWIMAGE_WIDGET:
	return eh_bwimage;
	break;
    case CPROGRESS_WIDGET:
	return eh_progress;
	break;
    case CBITMAP_WIDGET:
	return eh_bitmap;
	break;
    case CBITMAPBUTTON_WIDGET:
	return eh_bitmapbutton;
	break;
    case CSWITCH_WIDGET:
	return eh_switch;
	break;
#ifdef HAVE_PICTURE
    case CPICTURE_WIDGET:
	return eh_picture;
	break;
#endif
    case CEDITOR_WIDGET:
	return eh_editor;
    }
    return NULL;
}
