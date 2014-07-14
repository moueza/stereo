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


#define DRAWINGS_C

#include <config.h>
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"

#include "coolwidget.h"
#include "coollocal.h"

#include "mad.h"

#include "drawings.h"

/*
    the bounds are the maximum extents of the elements.
    only events withing the bounds are evaluated.
*/
void extendbounds (CWidget *wdt, int x1, int y1, int x2, int y2)
{
    int xb = max (x1, x2);
    int xa = min (x1, x2);
    int yb = max (y1, y2);
    int ya = min (y1, y2);

    if (wdt->pic->x1 > xa)
	wdt->pic->x1 = xa;
    if (wdt->pic->x2 < xb)
	wdt->pic->x2 = xb;

    if (wdt->pic->y1 > ya)
	wdt->pic->y1 = ya;
    if (wdt->pic->y2 < yb)
	wdt->pic->y2 = yb;
}

/* returns 1 on error */
int Csetdrawingtarget (const char *picture_ident)
{
    return (!(CWdrawtarget = Cwidget(picture_ident)));
}


void Cexposepic(CWidget *wdt)
{
    Cexposewindowarea (wdt->parentid, 0, wdt->pic->x1 + wdt->x,
	wdt->pic->y1 + wdt->y, wdt->pic->x2 - wdt->pic->x1 + 1,
	wdt->pic->y2 - wdt->pic->y1 + 1);
}


void cw_destroypicture (CWidget *wdt)
{
    if (wdt->pic->pp) {
	free (wdt->pic->pp);
	wdt->pic->pp = 0;
    }
    if (wdt->pic) {
	free (wdt->pic);
	wdt->pic->pp = 0;
    }
}


void cw_reboundpic(CWidget *wdt)
{
    int j;
    wdt->pic->x1 = 30000;
    wdt->pic->x2 = -30000;
    wdt->pic->y1 = 30000;
    wdt->pic->y2 = -30000;

    if (wdt->pic->numelements) {
	for (j = 0; j < wdt->pic->numelements; j++) {
	    CPicturePrimative *pp = &(wdt->pic->pp[j]);
	    switch (pp->type) {
	    case CRECTANGLE:
	    case CFILLED_RECTANGLE:
		extendbounds (wdt, pp->x, pp->y, pp->x + pp->a, pp->y + pp->b);
		break;
	    case CLINE:
		extendbounds (wdt, pp->x, pp->y, pp->a, pp->b);
		break;
	    }
	}
    }
}


CWidget *Cdrawpicture (const char *identifier, Window parent, int x, int y,
		int max_num_elements)
{
    CWidget **w;

    if(Cwidget(identifier))
	Cerror("Trying to create a picture with the same identifier as an existing widget.\n");

    w = Cfindemptywidgetentry (); /*find first unused list entry in list of widgets*/
    *w = Callocatewidget (0, identifier, parent, x, y,
		       0, 0, CPICTURE_WIDGET);
    (*w)->pic = Cmalloc(sizeof(CPicture));
    memset((*w)->pic, 0, sizeof(CPicture));
    (*w)->pic->pp = Cmalloc(sizeof(CPicturePrimative) * max_num_elements);
    cw_reboundpic(*w);

    (*w)->eh = Cdefaulthandler(CPICTURE_WIDGET);
    (*w)->destroy = cw_destroypicture;
    Csetdrawingtarget (identifier);
    return (*w);
}

void Cclearpic(void)
{
    CWidget *wdt = CWdrawtarget;
    wdt->pic->numelements = 0;

    Cexposepic (wdt);
    cw_reboundpic(wdt);
}

int Cdrawline (float x1, float y1, float x2, float y2, unsigned long c)
{
    CWidget *wdt = CWdrawtarget;
    int last = wdt->pic->numelements;

    wdt->pic->pp[last].x = x1;
    wdt->pic->pp[last].y = y1;
    wdt->pic->pp[last].a = x2;
    wdt->pic->pp[last].b = y2;
    wdt->pic->pp[last].color = c;
    wdt->pic->pp[last].type = CLINE;
    wdt->pic->numelements++;

    extendbounds (wdt, x1, y1, x2, y2);

    Cexposepic (wdt);

    return last;
}

int Cdrawpicrectangle (float x, float y, float w, float h, unsigned long c)
{
    CWidget *wdt = CWdrawtarget;
    int last = wdt->pic->numelements;

    wdt->pic->pp[last].x = x;
    wdt->pic->pp[last].y = y;
    wdt->pic->pp[last].a = w;
    wdt->pic->pp[last].b = h;
    wdt->pic->pp[last].color = c;
    wdt->pic->pp[last].type = CRECTANGLE;
    wdt->pic->numelements++;

    extendbounds (wdt, x, y, x +  w, y + h);

    Cexposepic (wdt);

    return last;
}


void Cremovepp (int j)
{
    if (CWdrawtarget->pic->numelements <= j || j < 0)
	return;

    if (CWdrawtarget->pic->numelements - 1 == j)
	CWdrawtarget->pic->numelements--;
    else
	CWdrawtarget->pic->pp[j].type = 0;

    Cexposepic (CWdrawtarget);
    cw_reboundpic(CWdrawtarget);
}


void Cscalepicture (float s)
{
    CWidget *wdt = CWdrawtarget;
    int j = 0;

    if (s == 1)
	return;

    if (s < 1)
	Cexposepic (wdt);
    for (; j < wdt->pic->numelements; j++) {
	CPicturePrimative *pp = &(wdt->pic->pp[j]);
	pp->x *= s;
	pp->y *= s;
	pp->a *= s;
	pp->b *= s;
    }
    wdt->pic->x1 *= s;
    wdt->pic->x2 *= s;
    wdt->pic->y1 *= s;
    wdt->pic->y2 *= s;
    cw_reboundpic (wdt);

    if (s > 1)
	Cexposepic (wdt);
}


#define is_between(a,b,c) \
		    ((a) >= (b) && (a) <= (c))

int eh_picture (CWidget *wdt, XEvent * xevent, CEvent * cwevent)
{
    int last = wdt->pic->numelements;
    CPicturePrimative *pp = wdt->pic->pp;
    Window win = wdt->parentid;
    int x = wdt->x;
    int y = wdt->y;

    switch (xevent->type) {
    case Expose:
	if (last) {
/* check if the expose covers the region of the picture */
	    int w = max (max (xevent->xexpose.x, xevent->xexpose.x + xevent->xexpose.width),
			 wdt->pic->x2 + x)
	    - min (min (xevent->xexpose.x, xevent->xexpose.x + xevent->xexpose.width),
		   wdt->pic->x1 + x);

	    int h = max (max (xevent->xexpose.y, xevent->xexpose.y + xevent->xexpose.height),
			 wdt->pic->y2 + y)
	    - min (min (xevent->xexpose.y, xevent->xexpose.y + xevent->xexpose.height),
		   wdt->pic->y1 + y);

	    if (h < wdt->pic->y2 - wdt->pic->y1 + abs (xevent->xexpose.height)
		&& w < wdt->pic->x2 - wdt->pic->x1 + abs (xevent->xexpose.width)) {
		int j = 0;
		for (; j < last; j++)
		    switch (pp[j].type) {
		    case CLINE:
			Csetcolor (pp[j].color);
			Cline (win, pp[j].x + x, pp[j].y + y, pp[j].a + x, pp[j].b + y);
			break;
		    case CFILLED_RECTANGLE:
			Csetcolor (pp[j].color);
			Crect (win, pp[j].x + x, pp[j].y + y, pp[j].a, pp[j].b);
			break;
		    case CRECTANGLE:
			Csetcolor (pp[j].color);
			Cline (win, pp[j].x + x, pp[j].y + y, pp[j].x + x + pp[j].a, pp[j].y + y);
			Cline (win, pp[j].x + x, pp[j].y + y, pp[j].x + x, pp[j].y + y + pp[j].b);
			Cline (win, pp[j].x + x + pp[j].a, pp[j].y + y + pp[j].b, pp[j].x + x + pp[j].a, pp[j].y + y);
			Cline (win, pp[j].x + x + pp[j].a, pp[j].y + y + pp[j].b, pp[j].x + x, pp[j].y + y + pp[j].b);
			break;
		    }
	    }
	}
	break;
    case ButtonPress:
	break;
    }
    return 0;
}

