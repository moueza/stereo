/*****************************************************************************************/
/* imagehandler.c - handles events from the view windows                                 */
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
#include "marker.h"
#include "app_glob.c"
#include "hugeimage.h"
#include "widget3d.h"
#include "main/marker.h"
#include "main/displaycam.h"
#include "picsetup.h"
#include "dialog.h"
#include "stringtools.h"
#include "callback.h"


/* check zoom box motion */
static int check_motion (Picture * image, CEvent * cwevent)
{
    if (!CPending (CDisplay)) {
	Csetdrawingtarget (image->zoom_rect->ident);
	Cremovepp (0);
	Cdrawpicrectangle ((cwevent->x - 1) - (cwevent->x - 1) % image->mag - 1, \
		(cwevent->y - 1) - (cwevent->y - 1) % image->mag - 1, image->mag + 1, \
		image->mag + 1, Ccolor (19));
	return 1;
    } else
	return 0;
}


static int check_button_release (Picture * image, CEvent * cwevent)
{
    double x, y;
    int xzoom, yzoom;
    long p, q;
    float oldmag = image->mag;
    xzoom = cwevent->x / image->mag;
    yzoom = cwevent->y / image->mag;

    x = (double) (image->xzoom + xzoom) * image->width / image->real_width;
    y = (double) (image->yzoom + yzoom) * image->width / image->real_width;

    image->mag <<= 1;		/*cycle zooming with right mouse button */
    if (image->mag > 16)
	image->mag = 1;

    q = ZOOMSIZE / (2 * image->mag);
    p = q * image->width / image->real_width;

    image->xzoom = (double) x *image->real_width / image->width - q;
    image->yzoom = (double) y *image->real_width / image->width - q;

    Csetdrawingtarget (image->main_rect->ident);
    Cremovepp (0);
    Cdrawpicrectangle (x - p - 2, y - p - 2, p * 2 + 1, p * 2 + 1, Ccolor (19));

    Csetdrawingtarget (image->zoom_rect->ident);
    Cremovepp (0);

    Csetdrawingtarget (image->zoom_markers->ident);
    Cscalepicture ((float) image->mag / oldmag);
    Csetwidgetposition (image->zoom_markers->ident, \
		-image->xzoom * image->mag + 2, -image->yzoom * image->mag + 2);
    Cupdatezoombox (image->zoom_image->ident, image->xzoom, image->yzoom, \
								    image->mag);
    return 1;
}

static int check_button_drag (Picture * image, CEvent * cwevent)
{
    int xzoom, yzoom;
    long p, q;
    xzoom = cwevent->x;
    yzoom = cwevent->y;
    q = ZOOMSIZE / (2 * image->mag);
    p = q * image->width / image->real_width;

    image->xzoom = xzoom * image->real_width / image->width - q;
    image->yzoom = yzoom * image->real_width / image->width - q;

    if (!CPending (CDisplay) || !(cwevent->type == MotionNotify)) {
	Csetdrawingtarget (image->zoom_markers->ident);
	Csetwidgetposition (image->zoom_markers->ident, \
	    -image->xzoom * image->mag + 2, -image->yzoom * image->mag + 2);

	Csetdrawingtarget (image->main_rect->ident);
	Cremovepp (0);
	Cdrawpicrectangle (xzoom - p - 2, yzoom - p - 2, p * 2 + 1, p * 2 + 1, \
							    Ccolor (19));

	Csetdrawingtarget (image->zoom_rect->ident);
	Cremovepp (0);

	Cupdatezoombox (image->zoom_image->ident, image->xzoom, image->yzoom, \
							    image->mag);
    }
    return 1;
}


int handle_zoom_box (Desktop * d, CEvent * cwevent)
{
    Picture *p;

    set_current_from_pointer (d, cwevent);
    p = &(d->view[d->current_view].pic);

    if (cwevent->state & ControlMask) {
	if ((cwevent->button == Button1
	     && (cwevent->type == ButtonRelease
		 || cwevent->type == ButtonPress))
	    || (cwevent->state & Button1Mask && cwevent->type == MotionNotify))
	    move_marker (p, cwevent);

	if (cwevent->button == Button2 && cwevent->type == ButtonRelease)
	    remove_marker (p, cwevent);
    } else {
	if (cwevent->button == Button1 && cwevent->type == ButtonRelease)
	    insert_marker (p, cwevent);

	if (cwevent->button == Button2 && cwevent->type == ButtonRelease)
	    check_button_release (p, cwevent);
    }

    if (cwevent->type == MotionNotify || cwevent->type == ButtonRelease)
	return check_motion (p, cwevent);

    return 0;
}


int handle_main_box (Desktop * d, CEvent * cwevent)
{
    Picture *p;

    set_current_from_pointer (d, cwevent);
    p = &(d->view[d->current_view].pic);

    if (cwevent->button == Button1 || (cwevent->state & Button1Mask))
	return check_button_drag (p, cwevent);

    return 0;
}
