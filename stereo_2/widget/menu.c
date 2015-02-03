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
#include "coolwidget.h"
#include "coollocal.h"

#include "mad.h"

int eh_menu (CWidget * w, XEvent * xevent, CEvent * cwevent);

void Crenderbutton (CWidget * wdt);

void destroy_menu (CWidget * w)
{
    int i;
    if (!w)
	return;
    if (!w->menu)
	return;
    for (i = 0; i < w->numlines; i++)
	if (w->menu[i].text)
	    free (w->menu[i].text);
    free (w->menu);
}


#define WB 4
#define WBI 5
#define SEP 4
#define SEPI 2

int whereis_pointer (int x, int y, int w, int h, int n)
{
    int hl;
    hl = TEXT_PIX_PER_LINE;

    if(x < WBI || x > w - 1 - WBI)
	return -1;

    if(y < WBI + SEPI || y > h - 1 - WBI - SEPI)
	return -1;

    return (y - WBI - SEPI) / (hl + SEP);
}

void menu_draw (Window win, int w, int h, struct menu_item m[], int n, int light)
{
    int i, hl;

    hl = TEXT_PIX_PER_LINE;

    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 0);
    Crenderbevel (win, WB, WB, w - 1 - WB, h - 1 - WB, WBI - WB, 1);

    Csetcolor (C_BLACK);
    for (i = 0; i < n; i++) {
	if (i == light && m[i].text[2] != 0) {
	    Csetcolor (C_BLACK);
	    Crect (win, WBI + SEPI, i * (hl + SEP) + SEPI + WBI, w - 2 * (WBI + SEPI), hl);
	    Csetcolor (Ccolor(16));
	} else {
	    Csetcolor (C_FLAT);
	    Crect (win, WBI + SEPI, i * (hl + SEP) + SEPI + WBI, w - 2 * (WBI + SEPI), hl);
	    Csetcolor (C_BLACK);
	}
	if (m[i].text[2] == 0) {
	    Crenderbevel (win, WB + SEPI + 2, i * (hl + SEP) + SEPI + WBI + hl / 2 - (WBI - WB + 1), w - 1 - WB - SEPI - 2, i * (hl + SEP) + SEPI + WBI + hl / 2 - (WBI - WB + 1) + (WBI - WB + 1) * 2 - 1, WBI - WB, 1);
	}
	XDrawString (CDisplay, win, CGC,
		     CFontStruct->max_bounds.width / 2 + WBI + SEPI - 4,
		     FONT_OFFSET_Y - 1 + WBI + i * (hl + SEP) + SEPI, m[i].text,
		     strlen (m[i].text));
    }
}

void Crender_menu (CWidget * w)
{
    int n, hl = TEXT_PIX_PER_LINE;
    if (!w)
	return;
    n = w->numlines;
    w->height = WBI * 2 + n * hl + (n - 1) * SEP + SEPI * 2;
    XResizeWindow (CDisplay, w->winid, w->width, w->height);
    XRaiseWindow (CDisplay, w->winid);
    w->droppedmenu->current = w->current;
    menu_draw (w->winid, w->width, w->height, w->menu, w->numlines, w->current);
}

/* gets a widgets position relative to some ancestor widget */ 
void CGetWindowPosition (Window win, Window ancestor, int *x_return, int *y_return)
{
    CWidget *w;
    int x = 0, y = 0;

    while (win != ancestor) {
	w = CW (CWidgetOf (win));
	if(!w)
	    break;
	win = w->parentid;
	x += w->x;
	y += w->y;
    }

    *x_return = x;
    *y_return = y;
}

CWidget *hold_the_menu = 0;

void pull_up (CWidget * button)
{
    if (button == hold_the_menu)
	return;
    if (!button)
	return;
    if (button->kind != CMENU_BUTTON_WIDGET)
	return;
    if(button->droppedmenu) {
	Cundrawwidget (button->droppedmenu->ident);
	button->droppedmenu = 0;
    }
}

void pull_down (CWidget * button) /* must create a new widget */
{
    CWidget *menu;
    CWidget *sib;
    int hl = TEXT_PIX_PER_LINE;
    int width, height, n;
    int x, y;
    char *s;

    if(button->droppedmenu)
	return;

    sib = button;
    while((sib = CNextFocus (sib)) != button) /* pull up any other sibling menus */
	pull_up (sib);

    n = button->numlines;
    s = button->menu[0].text;

    CGetWindowPosition (button->winid, CMain, &x, &y);

    height = WBI * 2 + n * hl + (n - 1) * SEP + SEPI * 2;
    width = max (WBI * 2 + SEPI * 2 + XTextWidth (CFontStruct, s, strlen (s)), XTextWidth (CFontStruct, "M", 1) * 20);

    menu = Csetupwidget (catstrs(button->ident, ".pull", 0), CMain, x,
	    y + button->height, width, height, CMENU_WIDGET, INPUT_MOTION | INPUT_BUTTON, C_FLAT, 0);

/* no destroy 'cos gets ->menu gets destroyed by other menu-button-widget */
    menu->numlines = n;
    menu->menu = button->menu;
    menu->eh = eh_menu;
    menu->droppedmenu = button;
    button->droppedmenu = menu;
}

void CPullDown (CWidget * button)
{
    pull_down (button);
}

void CPullUp (CWidget * button)
{
    pull_up (button);
}

int eh_menubutton (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    int c;
    CWidget *f;
    switch (xevent->type) {
    case FocusOut: {
	XEvent e;
	e.type = FocusOut;
	while(XCheckWindowEvent(CDisplay, cwevent->window, FocusChangeMask, &e));
	if(e.type != FocusOut)
	    break;
	pull_up (w);
	}
    case FocusIn:
	Crenderbutton (w);
	Cexposewindowarea (w->parentid, 0, w->x - FOCUS_RING, w->y - FOCUS_RING, w->width + FOCUS_RING * 2, w->height + FOCUS_RING * 2);
	break;
    case KeyPress:
	c = CKeySym (xevent);
	if (c == XK_Escape) {
	    CFocusWindow (w->menu_focus_return);
	    return 1;
	}
/*****************/
	if ((c == XK_Tab && xevent->xkey.state & ShiftMask) || c == XK_Left) {
	    f = CPreviousFocus (w);
	    while (f->kind != CMENU_BUTTON_WIDGET && (unsigned long) f != (unsigned long) w)
		f = CPreviousFocus (f);
	    if (f) {
		CFocus (f);
		if (w->droppedmenu)
		    pull_down (f);
	    }
	    return 1;
	}
	if (c == XK_Tab || c == XK_Right) {
	    f = CNextFocus (w);
	    while (f->kind != CMENU_BUTTON_WIDGET && (unsigned long) f != (unsigned long) w)
		f = CNextFocus (f);
	    if (f) {
		CFocus (f);
		if (w->droppedmenu)
		    pull_down (f);
	    }
	    return 1;
	}
	if (c == XK_Up && w->droppedmenu) {
	    if (w->droppedmenu->current == -1)
		w->droppedmenu->current = 0;
	    do {
		w->droppedmenu->current = (w->droppedmenu->current + w->droppedmenu->numlines - 1) % w->droppedmenu->numlines;
	    } while (w->droppedmenu->menu[w->droppedmenu->current].text[2] == 0);
	    Crender_menu (w->droppedmenu);
	    return 1;
	}
	if (c == XK_Down && w->droppedmenu) {
	    do {
		w->droppedmenu->current = (w->droppedmenu->current + 1) % w->droppedmenu->numlines;
	    } while (w->droppedmenu->menu[w->droppedmenu->current].text[2] == 0);
	    Crender_menu (w->droppedmenu);
	    return 1;
	}
	if (c != XK_Return && c != XK_space && c != XK_Down)
	    break;
	if (w->droppedmenu && (c == XK_Return || c == XK_space)) {
	    hold_the_menu = w;
	    if(w->droppedmenu->current >= 0)
		if (w->droppedmenu->menu[w->droppedmenu->current].call_back)
		    (*(w->droppedmenu->menu[w->droppedmenu->current].call_back)) ();
	    hold_the_menu = 0;
	    pull_up (w);
	    break;
	}
    case ButtonPress:
	if (xevent->type == ButtonPress)
	    w->options = CBUTTON_PRESSED;
	CFocus (w);
	Crenderbutton (w);
	if (w->droppedmenu)
	    pull_up (w);
	else
	    pull_down (w);
	cwevent->ident = w->ident;	/* return the event */
	break;
    case ButtonRelease:
	w->options = CBUTTON_HIGHLIGHT;
	Crenderbutton (w);
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



int eh_menu (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    static Window win = 0;
    static int current = -30000;
    switch (xevent->type) {
    case MotionNotify:
	w->current = whereis_pointer (xevent->xmotion.x, xevent->xmotion.y, w->width, w->height, w->numlines);
	if (w->current == current && w->winid == win)
	    break;
	current = w->current;
	win = w->winid;
	Crender_menu (w);
	break;
    case ButtonRelease:
	w->current = whereis_pointer (xevent->xmotion.x, xevent->xmotion.y, w->width, w->height, w->numlines);
	hold_the_menu = w->droppedmenu;
	if (w->current >= 0 && w->current < w->numlines)
	    if (w->menu[w->current].call_back)
		(*(w->menu[w->current].call_back)) ();
	hold_the_menu = 0;
	pull_up (w->droppedmenu);
	break;
/*    case EnterNotify: */
    case ButtonPress:
	w->current = whereis_pointer (xevent->xmotion.x, xevent->xmotion.y, w->width, w->height, w->numlines);
	Crender_menu (w);
	break;
    case Expose:
	if (xevent->xexpose.count)
	    break;
    case LeaveNotify:
	current = w->current = w->droppedmenu->current;
	Crender_menu (w);
	break;
    }
    return 0;
}

int find_hotkey (CWidget * w);

CWidget *Cdrawmenubutton (const char *ident, Window parent, Window focus_return,
	int x, int y, int width, int height, int num_items, const char *label,
	const char *text, int hot_key, callfn call_back,...)
{
    va_list ap;
    CWidget *wdt;
    struct menu_item *m;
    int i;
    int w, h;

    if (width == AUTO_WIDTH || height == AUTO_HEIGHT)
	Ctextsize (&w, &h, label);
    if (width == AUTO_WIDTH)
	width = w + 4 + BUTTON_RELIEF * 2;
    if (height == AUTO_HEIGHT)
	height = h + 4 + BUTTON_RELIEF * 2;


    wdt = Csetupwidget (ident, parent, x, y,
	    width, height, CMENU_BUTTON_WIDGET, INPUT_BUTTON, C_FLAT, 1);

    Csethintpos (x + width + WIDGET_SPACING, y + height + WIDGET_SPACING);

    wdt->label = strdup (label);
    wdt->hotkey = find_hotkey (wdt);

    m = Cmalloc (num_items * sizeof (struct menu_item));

    m[0].text = strdup (catstrs(" ", text, " ", 0));
    m[0].hot_key = hot_key;
    m[0].call_back = call_back;

    va_start (ap, call_back);
    if (num_items > 1)
	for (i = 1; i < num_items; i++) {
	    m[i].text = strdup (catstrs(" ", va_arg (ap, char *), " ", 0));
	    m[i].hot_key = va_arg (ap, KeySym);
	    m[i].call_back = va_arg (ap, callfn);
	}
    va_end (ap);

    wdt->destroy = destroy_menu;
    wdt->numlines = num_items;
    wdt->menu = m;
    wdt->eh = eh_menubutton;
    wdt->menu_focus_return = focus_return;

    return wdt;
}



void Caddmenuitem (const char *ident, const char *text, int hot_key, callfn call_back)
{
    struct menu_item *m;
    CWidget *w;

    w = Cwidget (ident);

    m = Cmalloc ((w->numlines + 1) * sizeof (struct menu_item));
    memcpy (m, w->menu, w->numlines * sizeof (struct menu_item));
    free (w->menu);
    w->menu = m;
    m[w->numlines].text = strdup (catstrs(" ", text, " ", 0));
    m[w->numlines].hot_key = hot_key;
    m[w->numlines].call_back = call_back;

    w->numlines ++;

    if (w->droppedmenu != 0) {
	w->droppedmenu->menu = m;
	w->droppedmenu->numlines = w->numlines;
	w->droppedmenu->current = w->current;
	Crender_menu (w->droppedmenu);
    }
}


static void remove_item (CWidget * w, int i)
{
    if (i >= w->numlines)
	return;
    if (w->menu[i].text)
	free (w->menu[i].text);
    w->numlines--;
    memmove (&w->menu[i], &w->menu[i + 1], (w->numlines - i) * sizeof (struct menu_item));
    if (w->current == i)
	w->current = -1;
    else if (w->current > i)
	w->current--;
    if (w->droppedmenu != 0) {
	w->droppedmenu->numlines = w->numlines;
	w->droppedmenu->current = w->current;
    }
}


/*
   Starts from the bottom of the menu and searches for the first
   menu item containing text (strstr != NULL), and deletes it.
   (this is untested).
 */
void Cremovemenuitem (const char *ident, const char *text)
{
    struct menu_item *m;
    CWidget *w;
    int i;

    w = Cwidget (ident);

    if (!w->numlines)
	return;

    m = w->menu;

    for (i = w->numlines - 1; i >= 0; i--) {
	if (!text)
	    goto remove;
	if (strstr (m[i].text, text) || !*text) {
	  remove:;
	    remove_item (w, i);
	    Crender_menu (w->droppedmenu);
	    return;
	}
    }
}




