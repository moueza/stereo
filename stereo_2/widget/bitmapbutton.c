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


void Crenderroundedbevel (Window win, int x1, int y1, int x2, int y2, int radius, int thick, int sunken);

CWidget *Cdrawbitmapbutton (const char *identifier, Window parent, int x, int y,
			    int width, int height, unsigned long fg, unsigned long bg, const unsigned char data[])
{
    CWidget *w = Csetupwidget (identifier, parent, x, y,
       width + 8, height + 8, CBITMAPBUTTON_WIDGET, INPUT_BUTTON, bg, 1);
    Pixmap pixmap;

    pixmap = XCreateBitmapFromData (CDisplay, w->winid, (char *) data, width, height);
    w->pixmap = pixmap;
    w->fg = fg;
    w->bg = bg;

    Csethintpos (x + width + 8 + WIDGET_SPACING, y + height + 8 + WIDGET_SPACING);

    return w;
}

CWidget *Cdrawbitmap (const char *identifier, Window parent, int x, int y,
		      int width, int height, unsigned long fg, unsigned long bg, const unsigned char data[])
{
    CWidget *w = Csetupwidget (identifier, parent, x, y,
	     width + 8, height + 8, CBITMAP_WIDGET, INPUT_EXPOSE, bg, 0);
    Pixmap pixmap;

    pixmap = XCreateBitmapFromData (CDisplay, w->winid, (char *) data, width, height);
    w->pixmap = pixmap;
    w->fg = fg;
    w->bg = bg;

    Csethintpos (x + width + 8 + WIDGET_SPACING, height + 8 + WIDGET_SPACING);

    return w;
}

Pixmap Cswitchon = 0;
Pixmap Cswitchoff = 0;

CWidget *Cdrawswitch (const char *identifier, Window parent, int x, int y,
		      unsigned long fg, unsigned long bg, int on)
{
    CWidget *w = Csetupwidget (identifier, parent, x, y,
			    32, 32, CSWITCH_WIDGET, INPUT_BUTTON, bg, 1);

    if (!Cswitchon) {
	Cswitchon = XCreateBitmapFromData (CDisplay, w->winid, (char *) switchon_bits, 32, 32);
	Cswitchoff = XCreateBitmapFromData (CDisplay, w->winid, (char *) switchoff_bits, 32, 32);
    }
    w->fg = fg;
    w->bg = bg;
    w->keypressed = on;

    Csethintpos (x + 32 + WIDGET_SPACING, y + 32 + WIDGET_SPACING);

    return w;
}

void Crenderbitmapbutton (CWidget * wdt, int state)
{
    int w = wdt->width, h = wdt->height;
    int x = 0, y = 0;
    Window win = wdt->winid;

    if (render_focus_ring (wdt))
	x = y = FOCUS_RING;

    if (state == 3)
	Csetcolor (wdt->bg);
    else
	Csetcolor (C_FLAT);
    Crect (win, x + 1, y + 1, w - 2, h - 2);


    switch (state) {
    case 0:
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 2, 0);
	break;
    case 1:
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 1, 0);
	break;
    case 2:
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 2, 1);
	break;
    case 3:
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 1, 1);
	break;
    }

    Csetcolor (wdt->fg);
    Csetbackcolor (wdt->bg);
    XCopyPlane (CDisplay, wdt->pixmap, win, CGC, 0, 0,
		w - 8, h - 8, 4 + x, 4 + y, 1);
}

void Crenderswitch (CWidget * wdt, int state)
{
    int w = wdt->width, h = wdt->height;
    Window win = wdt->winid;
    int x = 0, y = 0;

    if (render_focus_ring (wdt))
	x = y = FOCUS_RING;

    Csetcolor (C_FLAT);
    Crect (win, x, y, w - 1, h - 1);


    Csetcolor (wdt->fg);
    Csetbackcolor (wdt->bg);
    if (wdt->keypressed)
	XCopyPlane (CDisplay, Cswitchon, win, CGC, 0, 0,
		    w, h, x, y, 1);
    else
	XCopyPlane (CDisplay, Cswitchoff, win, CGC, 0, 0,
		    w, h, x, y, 1);

    switch (state) {
    case 0:
	Crenderroundedbevel (win, x, y, x + w - 1, y + h - 1, 7, 1, 0);
	break;
    case 1:
	Crenderroundedbevel (win, x, y, x + w - 1, y + h - 1, 7, 1, 1);
	break;
    case 2:
	Crenderroundedbevel (win, x, y, x + w - 1, y + h - 1, 7, 1, 1);
	break;
    }
}


/*
   void Crenderroundedbevel (Window win, int x1, int y1, int x2, int y2, int radius, int thick, int sunken)
   {
   unsigned long cn, cs, cnw, cne, cse;
   int i;

   if((sunken & 2)) {
   Csetcolor(C_FLAT);
   Crect(win,x1+thick,y1+thick,x2-x1-2*thick+1, y2-y1-2*thick+1);
   }

   sunken &= 1;

   cn = sunken ? Cwidgetcolor(4) : Cwidgetcolor(11);
   cs = sunken ? Cwidgetcolor(11) : Cwidgetcolor(4);
   cnw = Cwidgetcolor(14);
   cse = Cwidgetcolor(2);

   if(sunken) {cne = cnw; cnw = cse; cse = cne;}

   cne = Cwidgetcolor(8);

   #define Carc(win,x,y,w,h,a,b) XDrawArc(CDisplay,win,CGC,x,y,w,h,a,b)

   for (i = 0; i < thick; i++) {
   Csetcolor (cnw);
   Carc(win, x1 + i, y1 + i, (radius - i) * 2, (radius - i) * 2, 90*64, 90*64);
   Csetcolor (cse);
   Carc(win, x2 + i - radius*2, y2 + i - radius*2, (radius - i) * 2, (radius - i) * 2, 270*64, 90*64);
   Csetcolor (cne);
   Carc(win, x1 + i, y2 + i - radius*2, (radius - i) * 2, (radius - i) * 2, 180*64, 90*64);
   Carc(win, x2 + i - radius*2, y1 + i, (radius - i) * 2, (radius - i) * 2, 0, 90*64);

   Csetcolor (cn);
   Cline (win, x1 + i, y1 + radius, x1 + i, y2  - radius);
   Cline (win, x1 + radius, y1 + i, x2 - radius, y1 + i);
   Csetcolor (cs);
   Cline (win, x2 - radius, y2 - i, x1 + radius, y2 - i);
   Cline (win, x2 - i, y1 + radius, x2 - i, y2 - radius);
   }
   Csetcolor (C_BLACK);
   }
 */



void Crenderroundedbevel (Window win, int xs1, int ys1, int xs2, int ys2, int radius, int thick, int sunken)
{
    unsigned long cn, cs, cnw, cne, cse;
    int i;
    int x1, y1, x2, y2;

    if ((sunken & 2)) {
	Csetcolor (C_FLAT);
	Crect (win, xs1, ys1, xs2 - xs1 + 1, ys2 - ys1 + 1);
    }
    sunken &= 1;

    cn = sunken ? Cwidgetcolor (4) : Cwidgetcolor (11);
    cs = sunken ? Cwidgetcolor (11) : Cwidgetcolor (4);
    cnw = Cwidgetcolor (14);
    cse = Cwidgetcolor (2);

    if (sunken) {
	cne = cnw;
	cnw = cse;
	cse = cne;
    }
    cne = Cwidgetcolor (8);

#define Carc(win,x,y,w,h,a,b) XDrawArc(CDisplay,win,CGC,x,y,w,h,a,b)

    for (x1 = xs1; x1 < xs1 + thick; x1++)
	for (y1 = ys1; y1 < ys1 + thick; y1++) {
	    x2 = x1 + (xs2 - xs1 - thick + 1);
	    y2 = y1 + (ys2 - ys1 - thick + 1);
	    Csetcolor (cnw);
	    Carc (win, x1, y1, (radius) * 2, (radius) * 2, 90 * 64, 90 * 64);
	    Csetcolor (cse);
	    Carc (win, x2 - radius * 2, y2 - radius * 2, (radius) * 2, (radius) * 2, 270 * 64, 90 * 64);
	    Csetcolor (cne);
	    Carc (win, x1, y2 - radius * 2, (radius) * 2, (radius) * 2, 180 * 64, 90 * 64);
	    Carc (win, x2 - radius * 2, y1, (radius) * 2, (radius) * 2, 0, 90 * 64);
	}


    if (radius)
	radius--;
    for (i = 0; i < thick; i++) {
	Csetcolor (cn);
	Cline (win, xs1 + i, ys1 + radius, xs1 + i, ys2 - radius);
	Cline (win, xs1 + radius, ys1 + i, xs2 - radius, ys1 + i);
	Csetcolor (cs);
	Cline (win, xs2 - radius, ys2 - i, xs1 + radius, ys2 - i);
	Cline (win, xs2 - i, ys1 + radius, xs2 - i, ys2 - radius);
	Csetcolor (C_WHITE);
	if (sunken)
	    XDrawPoint (CDisplay, win, CGC, xs2 - i - (radius + 1) * 300 / 1024, ys2 - i - (radius + 1) * 300 / 1024);
	else
	    XDrawPoint (CDisplay, win, CGC, xs1 + i + (radius + 1) * 300 / 1024, ys1 + i + (radius + 1) * 300 / 1024);
    }


    Csetcolor (C_BLACK);
}
