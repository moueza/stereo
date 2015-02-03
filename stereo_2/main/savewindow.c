/*****************************************************************************************/
/* savewindow.c - saves the rendered 3D scene as a targa file                            */
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
#include "imagewidget.h"
#include "../global.h"
#include "app_glob.c"
#include "dialog.h"

/* \end{verbatim} \begin{verbatim} */

/* returns 1 on error */
int save_window (Window win, int x, int y, int width, int height, const char *file)
{
    XImage *image;
    byte *p_byte;
    byte *p, *q, *pix;
    word *p_word;
    quad_t *p_quad;

    Colormap cmap;

    int i, j, n;

    image = XGetImage (CDisplay, win, x, y, width, height, \
					    (unsigned long) -1, ZPixmap);
    if (!image) {
	Cerrordialog (0, 0, 0, " Save Window ", " Unable to create XImage ");
	return 1;
    }
    p = image->data;
    p_byte = p;
    p_word = (word *) p;
    p_quad = (quad_t *) p;

    pix = Cmalloc (width * height * 3);
    q = pix;

    switch (image->bits_per_pixel) {
    case 8:{
	    XColor *c;
	    n = DisplayCells (CDisplay, DefaultScreen (CDisplay));
	    c = Cmalloc (n * sizeof (XColor));
	    for (i = 0; i < n; i++)
		c[i].pixel = i;
	    cmap = DefaultColormap (CDisplay, DefaultScreen (CDisplay));
	    XQueryColors (CDisplay, cmap, c, n);

	    for (i = 0; i < height; i++) {
		p_byte = p;
		for (j = 0; j < width; j++) {
		    *q++ = c[*p_byte].blue >> 8;
		    *q++ = c[*p_byte].green >> 8;
		    *q++ = c[*p_byte].red >> 8;
		    p_byte++;
		}
		p += image->bytes_per_line;
	    }
	    free (c);
	    break;
	}
    case 16:{
	    XColor c;
	    c.flags = DoRed | DoGreen | DoBlue;
	    cmap = DefaultColormap (CDisplay, DefaultScreen (CDisplay));
	    for (i = 0; i < height; i++) {
		p_word = (word *) p;
		for (j = 0; j < width; j++) {
		    c.pixel = *p_word;
		    XQueryColor (CDisplay, cmap, &c);
		    *q++ = c.blue >> 8;
		    *q++ = c.green >> 8;
		    *q++ = c.red >> 8;
		    p_word++;
		}
		p += image->bytes_per_line;
	    }
	    break;
	}
    case 24:
	free (pix);
	Cerrordialog (0, 0, 0, " Save Window ", \
	    " %s:%d 24bpp images not supported, use 8, 16 or 32bpp. \n" \
	    " This may mean changing your display. ");
	return 1;
    case 32:{
	    XColor c;
	    c.flags = DoRed | DoGreen | DoBlue;
	    cmap = DefaultColormap (CDisplay, DefaultScreen (CDisplay));
	    for (i = 0; i < height; i++) {
		p_quad = (quad_t *) p;
		for (j = 0; j < width; j++) {
		    c.pixel = *p_quad;
		    XQueryColor (CDisplay, cmap, &c);
		    *q++ = c.blue >> 8;
		    *q++ = c.green >> 8;
		    *q++ = c.red >> 8;
		    p_quad++;
		}
		p += image->bytes_per_line;
	    }
	    break;
	}
    }
    writetarga (pix, file, width, height, 0);
    free (pix);
    return 0;
}


int save_window_to_file (Window win, int x, int y, int width, int height)
{
    char *filename;
    filename = Cgetfile (0, 0, 0, home_dir, "", " Save Window ");
    if (filename)
	if (*filename) {
	    save_window (win, x, y, width, height, filename);
	    free (filename);
	    return 0;
	}
    return 1;
}
