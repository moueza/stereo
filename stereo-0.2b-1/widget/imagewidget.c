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


/*general note: widget labels and identifiers are copied from
   data passed and free'd on widget undraw. */

#include <config.h>
#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "stringtools.h"
#include "app_glob.c"

#include "coolwidget.h"
#include "dialog.h"

#include "mad.h"


#define TPRINTF tiffprintf


void greyscaletopix (void *pixdata, unsigned char *data, int width, int height, int bytedepth)
{
    long p = width * height - 1;
    int i;
    static unsigned long c[256] =
    {1};

    quad_t *pixquad = pixdata;
    word *pixword = pixdata;
    byte *pixbyte = pixdata;

    if (*c == 1)
	for (i = 0; i < 256; i++)
	    c[i] = Cgrey (i >> 2);

    switch (bytedepth) {
    case 1:
	do {
	    *(pixbyte + p) = (byte) c[data[p]];
	} while (p--);
	break;
    case 2:
	do {
	    *(pixword + p) = (word) c[data[p]];
	} while (p--);
	break;
    case 3:{
	    long q = 0;
#if THIS_WAY_OR_THAT_WAY==THIS_WAY		/* this code is untested  ************ */
	    do {
		*(pixbyte++) = (quad_t) c[data[q]];
		*(pixbyte++) = (quad_t) c[data[q]] >> 8;
		*(pixbyte++) = (quad_t) c[data[q++]] >> 16;
	    } while (q <= p);
#else
	    do {
		*(pixbyte++) = (quad_t) c[data[q]] >> 16;
		*(pixbyte++) = (quad_t) c[data[q]] >> 8;
		*(pixbyte++) = (quad_t) c[data[q++]];
	    } while (q <= p);
#endif
	    break;
	}
    case 4:
	do {
	    *(pixquad + p) = (quad_t) c[data[p]];
	} while (p--);
	break;
    }
}



void color8bittopix (void *pixdata, unsigned char *data, int width, int height, int bytedepth)
{
    long p = width * height - 1;
    static unsigned long c[256] =
    {1};
    quad_t *pixquad = pixdata;
    word *pixword = pixdata;
    byte *pixbyte = pixdata;

    if (c[0] == 1)
	memcpy (c, Cpixel, 256 * sizeof (long));

    switch (bytedepth) {
    case 1:
	do {
	    *(pixbyte + p) = (byte) c[data[p]];
	} while (p--);
	break;
    case 2:
	do {
	    *(pixword + p) = (word) c[data[p]];
	} while (p--);
	break;
    case 3:{
	    long q = 0;
#if THIS_WAY_OR_THAT_WAY==THIS_WAY		/* this code is untested  ************ */
	    do {
		*(pixbyte++) = (quad_t) c[data[q]];
		*(pixbyte++) = (quad_t) c[data[q]] >> 8;
		*(pixbyte++) = (quad_t) c[data[q++]] >> 16;
	    } while (q <= p);
#else
	    do {
		*(pixbyte++) = (quad_t) c[data[q]] >> 16;
		*(pixbyte++) = (quad_t) c[data[q]] >> 8;
		*(pixbyte++) = (quad_t) c[data[q++]];
	    } while (q <= p);
#endif
	    break;
	}
    case 4:
	do {
	    *(pixquad + p) = (quad_t) c[data[p]];
	} while (p--);
	break;
    }
}



/* width and height refer to the image. 4 pixels will be added on to
   this for the border */
CWidget * Cdrawbwimage (const char *identifier, Window parent, int x, int y,
		       int width, int height, unsigned char *data)
{
    int bytespp = 16;
    CWidget *w;

    if (width & 1)
	bytespp = 8;
    if (!(width & 3))
	bytespp = 32;

    w = Csetupwidget (identifier, parent, x, y,
			  width + 4, height + 4, CBWIMAGE_WIDGET, INPUT_MOTION, C_WHITE, 0);

    w->ximage = XCreateImage (CDisplay, Cvisual, Cdepth, ZPixmap,
				   0, NULL, width, height, bytespp, 0);

    if (w->ximage == NULL) {
	Cundrawwidget(identifier);
	Cerrordialog (CMain, 20, 20, " Cdrawbwimage ", " Cannot create Ximage ");
	return NULL;
    }

    w->ximage->data = Cmalloc (width * height * w->ximage->bits_per_pixel / 8 + 4);

/*now format the one-byte-per-pixel-is-an-eight-bit-grey-level
   to a bytedepth-per-pixel-is-an-actual-pixel-value format */
/*We shift right by two to bring 0-255 grey-scale down to 0-63 for
   the coolwidget palette */

    if (w->ximage->bits_per_pixel % 8)
	Cerror ("Non-multiple of 8 bits_per_pixel in XImage not supported.\n");

    greyscaletopix (w->ximage->data, data, width, height, w->ximage->bits_per_pixel / 8);

    return w;
}


/*data[] must be 1 byte per pixel. Each pixel is a value between 0
   and 16+27+64 = 107: 0-16 is widget-cool-colors, 16-42 is 3^3 colors,
   43-63 is greyscale-colors. */

/*width and height refer to the image. 4 pixels will be added on to
   this for the border */
CWidget * Cdraw8bitimage (const char *identifier, Window parent, int x, int y,
			 int width, int height, unsigned char *data)
{
    int bytespp = 16;
    CWidget *w;

    if (width & 1)
	bytespp = 8;
    if (!(width & 3))
	bytespp = 32;

    w = Csetupwidget (identifier, parent, x, y,
			  width + 4, height + 4, C8BITIMAGE_WIDGET, INPUT_MOTION, C_WHITE, 0);

    w->ximage = XCreateImage (CDisplay, Cvisual, Cdepth, ZPixmap,
				   0, NULL, width, height, bytespp, 0);

    if (w->ximage == NULL) {
	Cundrawwidget(identifier);
	Cerrordialog (CMain, 20, 20, " Cdrawbwimage ", " Cannot create Ximage ");
	return NULL;
    }

    w->ximage->data = Cmalloc (width * height * w->ximage->bits_per_pixel / 8);

    if (w->ximage->bits_per_pixel % 8)
	Cerror ("Non-multiple of 8 bits_per_pixel in XImage not supported.\n");

    color8bittopix (w->ximage->data, data, width, height, w->ximage->bits_per_pixel / 8);

    return w;
}


void Crenderbwimage (CWidget *wdt, int x, int y, int rendw, int rendh)
{
    int w = wdt->width;
    int h = wdt->height;
    Window win = wdt->winid;
    int xim, yim, xwin, ywin;

    xim = x - 2;
    yim = y - 2;
    xwin = x;
    ywin = y;
    if (xim < 0) {
	rendw += xim;
	xim = 0;
	xwin = 2;
    }
    if (yim < 0) {
	rendh += yim;
	yim = 0;
	ywin = 2;
    }
    XPutImage (CDisplay, win, CGC, wdt->ximage,
	       xim, yim, xwin, ywin, rendw, rendh);

    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 1);
}


/*xvtarga.c : */


/*This file originally comes from XVIEW version 3.1 image viewing utility.
   I modified it to my needs. I suppose it isn't really a copyright violation
   because the targa image file format is really simple --- I just got
   lazy --- and I've changed the code considerably. Here are the original
   comments for the file just to acknowledge the help:  */

/*
 * xvtarga.c - load routine for 'targa' format pictures
 *
 * written and submitted by:
 *     Derek Dongray    (dongray@genrad.com)
 *
 * The format read/written is actually Targa type 2 uncompressed as
 * produced by POVray 1.0
 *
 * LoadTarga(fname, pinfo)
 * WriteTarga(fp, pic, ptype, w,h, rmap,gmap,bmap,numcols, cstyle)
 */


/*
 * Targa Format (near as I can tell)
 *   0:
 *   1: colormap type
 *   2: image type  (1=colmap RGB, 2=uncomp RGB, 3=uncomp gray)
 *   3: 
 *   4: 
 *   5: colormap_length, low byte
 *   6: colormap_length, high byte
 *   7: bits per cmap entry     (8, 24, 32)
 *
 *  12: width, low byte
 *  13: width, high byte
 *  14: height, low byte
 *  15: height, high byte
 *  16: bits per pixel (8, 24)
 *  17: flags  
 */


void tgaerror (const char *errmessage)
{
    fprintf (stderr, errmessage);	/*OR for the application: */
/*    Cerror (errmessage); *//********/
}


/*This loads a targa file and converts it to grey scale if it is color */

/*it returns a pointer to the data, which consists of contigous
   scanlines from the top left. It also returns the width and height.
   The returned data are 8bpp greyscale.
   Since loadtarga2grey mallocs, the returned pointer must be free'd */


unsigned char *loadtarga2grey (const char *fname, long *width, long *height, long rowstart, long rowend)
{
    FILE *fp;
    int i, j, k, row, c, c1, w, h, flags, intlace, topleft, trunc, bytesperpixel;
    unsigned char *pic8 = NULL, *pp;
    long filesize;

    if ((fp = fopen (fname, "r")) == NULL) {
	tgaerror ("Cannot open targa image file.\n");
	return NULL;
    }
    /* compute file length */
    fseek (fp, 0L, 2);
    filesize = ftell (fp);
    fseek (fp, 0L, 0);

    if (filesize < 18) {
	fclose (fp);
	tgaerror ("Targa file is too short.\n");
	return NULL;
    }
    /* Discard the first few bytes of the file. */

    for (i = 0; i < 12; i++) {
	c = getc (fp);
    }


    /* read in header information */
    c = getc (fp);
    c1 = getc (fp);
    w = c1 * 256 + c;
    *width = w;

    c = getc (fp);
    c1 = getc (fp);
    h = c1 * 256 + c;
    *height = h;

    if (rowstart > rowend) {
	tgaerror ("loadtga2grey called with rowstart > rowend.\n");
	return NULL;
    }
    if (rowstart > h)
	rowstart = h;

    if (rowstart < 0)
	rowstart = 0;

    if (rowend > h)
	rowend = h;

    if (w < 2 || h < 2) {
	fclose (fp);
	tgaerror ("Error in Targa header (bad image size).\n");
	return NULL;
    }
    c = getc (fp);
    if (c != 24 && c != 8) {
	fclose (fp);
	tgaerror ("Unsupported type (not 24-bit or 8-bit)\n");
	return NULL;
    }
    bytesperpixel = c / 8;

    flags = getc (fp);
    topleft = (flags & 0x20) >> 5;
    intlace = (flags & 0xc0) >> 6;

    if (intlace && (rowstart || rowend < h || !topleft)) {
	tgaerror ("Cannot load only a part of an interlaced or inverted tga.\n");
	return NULL;
    }
    if ((pic8 = malloc ((rowend - rowstart) * w + 1)) == NULL || (pp = malloc (w * 3)) == NULL) {
	tgaerror ("Cannot allocate memory for tga file.\n");
	return NULL;
    }
    if (rowstart)
	fseek (fp, rowstart * w * bytesperpixel, SEEK_CUR);

    trunc = 0;

    /* read the data */
    for (i = 0; i < rowend - rowstart; i++) {
	if (intlace == 2) {	/* four pass interlace */
	    if (i < (1 * h) / 4)
		row = 4 * i;
	    else if (i < (2 * h) / 4)
		row = 4 * (i - ((1 * h) / 4)) + 1;
	    else if (i < (3 * h) / 4)
		row = 4 * (i - ((2 * h) / 4)) + 2;
	    else
		row = 4 * (i - ((3 * h) / 4)) + 3;
	} else if (intlace == 1) {	/* two pass interlace */
	    if (i < h / 2)
		row = 2 * i;
	    else
		row = 2 * (i - h / 2) + 1;
	} else
	    row = i;		/* no interlace */

	if (!topleft)
	    row = (h - row - 1);	/* bottom-left origin: invert y */

	if (bytesperpixel == 3) {
	    c = fread (pp, (size_t) 1, (size_t) w * 3, fp);
	    if (c != w * 3)
		trunc = 1;
	    for (j = 0, k = 0; j < w; j++, k += 3)
/*luminance transformation: */
		pic8[j + row * w] = (int) (11 * pp[k + 2] + 16 * pp[k + 1] + 5 * pp[k]) >> 5;
	} else {
	    c = fread (pp, (size_t) 1, (size_t) w, fp);
	    if (c != w)
		trunc = 1;
	    for (j = 0; j < w; j++)
		pic8[j + row * w] = pp[j];
	}
    }

    free (pp);

    if (trunc) {
	tgaerror ("File appears to be truncated.\n");
    }
    fclose (fp);

    return pic8;
}



/*returns 1 on error */
int writetarga (unsigned char *pic8, const char *fname, long w, long h, int grey)
{
    FILE *fp;
    int i, j;
    long index = 0;

    if ((fp = fopen (fname, "w+")) == NULL) {
	tgaerror ("Cannot open create/overwrite targa image file.\n");
	return 1;
    }
    /* write header information */

    putc (0, fp);
    putc (0, fp);

    if (grey)
	putc (3, fp);
    else
	putc (2, fp);

    for (i = 0; i < 9; i++) {
	putc (0, fp);
    }

    putc (w & 0xFF, fp);
    putc (w >> 8, fp);

    putc (h & 0xFF, fp);
    putc (h >> 8, fp);

    if (grey)
	putc (8, fp);
    else
	putc (24, fp);

    putc (32, fp);		/*top left displayed */

    /* write the data */

    for (i = 0; i < h; i++) {
	if (grey)
	    fwrite (pic8 + i * w, w, 1, fp);
	else
	    for (j = 0; j < w; j++) {
		fputc (pic8[index], fp);
		fputc (pic8[index], fp);
		fputc (pic8[index++], fp);
	    }
    }

    fclose (fp);
    return 0;
}

