/*****************************************************************************************/
/* hugeimage.c - widget to draw the tiff file, zoom box and do caching of image data     */
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

#include <config.h>
#include "global.h"
#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "stringtools.h"
#include "app_glob.c"

#include "coolwidget.h"
#include "hugeimage.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "display.h"
#include "loadtiff.h"
#include "dialog.h"

#include "mad.h"

int hugegetpixel (HugeImage * image, unsigned long x, unsigned long y)
{
    if (y < 0 || y >= image->height || x < 0 || x >= image->width)
	return 0;
    else {
	int i = x >> COLUMNSHIFT;

/*check if the point has not already been read: */
	if (y < image->linestart[i] || y >= image->lineend[i])
	    hugeloadnextbuf (image, x, y, i);

	return (image->buffers[i])[((y - image->linestart[i]) << COLUMNSHIFT)
				   + (x & COLUMNMASK)];
    }
}


void hugeloadnextbuf (HugeImage * image, long x, long y, int i)
{
/*try not to seek if possible, rather just read the next
   adjacent LINESCACHED : */
    if (y >= image->lineend[i] + LINESCACHED || y < image->lineend[i]) {
	lseek (image->temp[i], y * image->columnwidth, SEEK_SET);
	image->linestart[i] = y;
	image->lineend[i] = y + LINESCACHED;
    } else {
	image->linestart[i] += LINESCACHED;
	image->lineend[i] += LINESCACHED;
    }
    read (image->temp[i], image->buffers[i],
	  image->columnwidth * LINESCACHED);
}


int loadhugeimage (HugeImage * image, const char *fname)
{
    Window progresswin;
    unsigned char *blockofrows;	/*holds the block of hieght BLOCKHEIGHT */
    long width, height;		/*of the whole image */
    long numblocks, numcolumns, i, j, k, bheight;
    char tempstr[256];
    int fileformat = 0;
    unsigned char *cache;
    int createtempfiles = 0;
    char *imagefile = strdup (fname);

    memset (image, 0, sizeof (HugeImage));
    image->has_been_allocated = 314159265;
	    /* this is just so we don't free anything that hasn't been allocated */

/*The first thing is to do create a whole lot of temp files,
   hence setting up the HugeImage structure.
   Each temp file is a columnwidth-pixels-wide column of the image.
   By dividing the image up into columns like this, we can speed
   up access to the image. The data in the temp file is contigous
   scan lines with no header.
 */

/*first get the image width and height and find the file format */

    if ((blockofrows = loadgreytiff (imagefile, &width, &height, \
		    0, 1, MONITOR_GAMMA)))
	fileformat = 1;
    else if ((blockofrows = loadtarga2grey (imagefile, &width, &height, 0, 1)))
	fileformat = 2;
    else {
	Cerrordialog (CMain, 20, 20, " Load Image ", \
		    " Unsupported image file format ");
	goto error;
    }
    free (blockofrows);

    image->width = width;
    image->height = height;

    numcolumns = (width + COLUMNWIDTH - 1) / COLUMNWIDTH;
    numblocks = (height + BLOCKHEIGHT - 1) / BLOCKHEIGHT;

    image->columnwidth = COLUMNWIDTH;
    image->numcolumns = numcolumns;

    image->temp = Cmalloc (numcolumns * sizeof (int));
    image->buffers = Cmalloc (numcolumns * sizeof (unsigned char *));
    image->linestart = Cmalloc (numcolumns * sizeof (long));
    image->lineend = Cmalloc (numcolumns * sizeof (long));

    printf ("loading image: res=(%ldx%ld)\n", width, height);

/*open files and set up caches */
    for (i = 0; i < numcolumns; i++) {
	for (j = strlen (imagefile); j > 0; j--)
	    if (imagefile[j] == '/')
		break;
	if (imagefile[j] != '/') {
	    memmove (imagefile + 1, imagefile + j, strlen (imagefile + j) + 1);
	    imagefile[0] = '/';
	    j = 0;
	}
	sprintf (tempstr, "%s%s%ld.temp", TEMPDIR, imagefile + j, i);
	if ((image->temp[i] = open (tempstr, O_RDWR)) == -1) {
	    createtempfiles = 1;
	    if ((image->temp[i] = open (tempstr, O_RDWR | O_CREAT | O_TRUNC,
			 S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1) {
		printf ("%s\n", tempstr);
		Cerror (" cannot open temporary file %s ");
	    }
	} else {
/********** ask user if they want to use temp files, or create new ones.*/
	    /*tempfiles exist this flag says don't need to write them out */
	}
/*********/
	image->buffers[i] = Cmalloc (LINESCACHED * COLUMNWIDTH);
	image->linestart[i] = 0;
			/* linestart = lineend = 0 will force a cache */
	image->lineend[i] = 0;	/*load on the first call to hugegetpixel */
    }

    if (createtempfiles)
	printf ("Creating new temp column files.\n");
    else
	printf ("Using existing temp column files.\n");

    progresswin = Cdrawwindow ("loadprogw", CMain, 50, 50, 300, 110, "");

    if (createtempfiles) {
	cache = Cmalloc (BLOCKHEIGHT * COLUMNWIDTH);
	for (i = 0; i < numblocks; i++) {
	    bheight = height - i * BLOCKHEIGHT;
	    if (bheight > BLOCKHEIGHT)
		bheight = BLOCKHEIGHT;
	    blockofrows = NULL;


	    Cdrawprogress ("loadprogbar", progresswin, 20, 20, 260, 25,
			   i * 65535 / numblocks);
	    Ccheckifevent (NULL, NULL);

	    switch (fileformat) {
	    case 1:
		blockofrows = loadgreytiff (imagefile, &width, &height,
					    i * BLOCKHEIGHT, \
				i * BLOCKHEIGHT + bheight, MONITOR_GAMMA);
		break;
	    case 2:
		blockofrows = loadtarga2grey (imagefile, &width, &height,
			     i * BLOCKHEIGHT, i * BLOCKHEIGHT + bheight);
		break;
	    }

	    for (k = 0; k < numcolumns; k++) {
		for (j = 0; j < bheight; j++)
		    memcpy (cache + j * COLUMNWIDTH,
			    blockofrows + j * width + k * COLUMNWIDTH, COLUMNWIDTH);
		write (image->temp[k], cache, COLUMNWIDTH * bheight);
	    }
	    if (blockofrows)
		free (blockofrows);
	}
	free (cache);
    }
    Cundrawwidget ("loadprogw");

    free (imagefile);
    return 0;
  error:;
    free (imagefile);
    return 1;
}

/* This just frees all the stuff allocated in loadhugeimage */
void freehugeimage (HugeImage * image)
{
    int i;

    if (image->has_been_allocated != 314159265)
	return;

    if (image->temp)
	for (i = 0; i < image->numcolumns; i++)
	    if (image->temp[i] >= 0)	/*do you get zero file handles? */
		close (image->temp[i]);

    if (image->buffers)
	for (i = 0; i < image->numcolumns; i++)
	    if (image->buffers[i])
		free (image->buffers[i]);

    if (image->temp)
	free (image->temp);
    if (image->buffers)
	free (image->buffers);
    if (image->linestart)
	free (image->linestart);
    if (image->lineend)
	free (image->lineend);
    memset (image, 0, sizeof (HugeImage));
}


void destroy_huge (CWidget * w)
{
    if (CUserOf (w)) {
	freehugeimage (CUserOf (w));
	free (CUserOf (w));
	CUserOf (w) = 0;
    }
}



/*
   Draw a huge image. The image is scaled (i.e. scaled by proper interpolation
   NOT by just leaving out scanlines) to the size specified (width, height).
   if width is zero image is scaled to height and width set adjusted to
   preserve aspect ratio. Similarly if height is zero.

   At the moment this handles only tiff files as discribed in imagewidget.c
   loadgreytiff(...) ********** 
 */
/*, HugeImage * returnimage */

CWidget *Cdrawhugebwimage (const char *identifier, Window parent, int x, int y,
			long *width, long *height, const char *imagefile)
{
    Window progresswin;
    CWidget *wdt;
    unsigned char *data;
/* check what happens to 'data' memory for mallocing and freeing***** */
    HugeImage *image;
    long i, j;
    long pixel;
    unsigned long xfrac, yfrac;
    unsigned long xs1, ys1, xs2, ys2, xp, yp, area;
    char fstr[256];
    long w, h;

    image = Cmalloc (sizeof (HugeImage));

    if (loadhugeimage (image, imagefile)) {
	free (image);
	return NULL;
    }
    if (!(*width | *height)) {
	*width = image->width;
	*height = image->height;

	data = Cmalloc (*width * *height);

	for (j = 0; j < *height; j++)
	    for (i = 0; i < *width; i++)
		data[i + j * *width] = hugegetpixel (image, i, j);

    } else {
	sprintf (fstr, "%s.thumb.tga", imagefile);
	data = loadtarga2grey (fstr, &w, &h, 0, 1 << 30);

	if (!data) {	/*a previously created thumbnail image does not exist, so... */
	    printf ( \
	"No thumbnail image exists, creating thumbnail image from original.\n");
	    if (!*width)
		*width = image->width * (*height) / image->height;
	    if (!*height)
		*height = image->height * (*width) / image->width;

	    data = Cmalloc ((*width + 20) * *height);

#define PFRAC 256
#define PSH 8

	    area = (double) image->width * image->height * PFRAC * PFRAC / \
								(*height * *width);

	    progresswin = Cdrawwindow ("loadprogw", CMain, 50, 50, 300, 110, "");

	    for (j = 0; j < *height; j++) {

		Cdrawprogress ("loadprogbar", progresswin, 20, 20, 260, 25,
			       j * 65535 / (*height));
		Ccheckifevent (NULL, NULL);
		for (i = 0; i < *width; i++) {
		    pixel = 0;	/*pixel holds the accumulated sum */

/*the following rectangle must be averaged; it bounds an unscaled pixel: */
		    xs1 = ((i * image->width) << PSH) / (*width);
		    xs2 = (((i + 1) * image->width) << PSH) / (*width);
		    ys1 = ((j * image->height) << PSH) / (*height);
		    ys2 = (((j + 1) * image->height) << PSH) / (*height);

/*
   yfrac is the proportion of the pixel that can be seen if
   the pixel crosses the rectangle border. It is out of 1 << PSH = PFRAC
   and is therefore only less than PFRAC for border pixels.
 */

		    yfrac = PFRAC - (ys1 & (PFRAC - 1));

		    for (yp = (ys1 >> PSH); yp < (ys2 >> PSH); yp++) {
			xfrac = PFRAC - (xs1 & (PFRAC - 1));
			for (xp = (xs1 >> PSH); xp < (xs2 >> PSH); xp++) {
			    pixel += xfrac * yfrac * hugegetpixel (image, xp, yp);
			    xfrac = PFRAC;
			    /* inner pixels can all be seen entirely */
			}
/*do last pixel in row */
			if ((xfrac = (xs2 & (PFRAC - 1))))
			    pixel += xfrac * yfrac * hugegetpixel (image, xp, yp);
			yfrac = PFRAC;
		    }

/*now do the last row of the rectangle */
		    if ((yfrac = (ys2 & (PFRAC - 1)))) {
			xfrac = PFRAC - (xs1 & (PFRAC - 1));
			for (xp = (xs1 >> PSH); xp < (xs2 >> PSH); xp++) {
			    pixel += xfrac * yfrac * hugegetpixel (image, xp, yp);
			    xfrac = PFRAC;
			}
/*do last pixel in row */
			xfrac = (xs2 & (PFRAC - 1));
			pixel += xfrac * yfrac * hugegetpixel (image, xp, yp);
		    }
		    data[i + j * *width] = (long) pixel / area;
		}
	    }

	    Cundrawwidget ("loadprogw");
	    writetarga (data, fstr, *width, *height, 1);
				/* save the thumbnail image for later */
	} else {   /* thumbnail image exists and is stored in data so do nothing */
	    printf ("Thumbnail image exists, loading.\n");
	    *width = w;
	    *height = h;
	}
    }

    wdt = Cdrawbwimage (identifier, parent, x, y,
			*width, *height, data);

    CUserOf (wdt) = (void *) image;
    wdt->destroy = destroy_huge;
    return wdt;
}


void Crenderzoombox (CWidget * wdt, int x, int y, int rendw, int rendh)
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



/*
   x, y is position in huge image. w, h is data size, zoom is
   pixel width of a hugeimage pixel i.e. the enlargement factor.
 */
void extractfromhugeimage (HugeImage * image, unsigned char *data,
			 long x, long y, int width, int height, int zoom)
{				/*this must be tested */
    long i, j;
    long c, l, m, k;
    unsigned char *q = data;

/*wanna optimise more than this? I'm sure there's room. */
    if (zoom)
	if (zoom == 1) {
	    for (j = y; j < height + y; j++)
		for (i = x; i < width + x; i++) {
		    *(q++) = hugegetpixel (image, i, j);
		}
	} else if (zoom == 2) {
	    for (j = 0; j < height; j += 2)
		for (i = 0; i < width; i += 2) {
		    data[k = (i + j * width)] = c = \
				    hugegetpixel (image, (i >> 1) + x, (j >> 1) + y);
		    data[k + 1] = c;
		    data[(k++) + width] = c;
		    data[k + width] = c;
		}
	} else if (zoom == 3) {
	    for (j = 0; j < height; j += 3)
		for (i = 0; i < width; i += 3) {
		    data[k = (i + j * width)] = c = \
				    hugegetpixel (image, i / 3 + x, j / 3 + y);
		    data[k + 1] = c;
		    data[k + 2] = c;
		    k += width;
		    data[k] = c;
		    data[k + 1] = c;
		    data[k + 2] = c;
		    k += width;
		    data[k] = c;
		    data[k + 1] = c;
		    data[k + 2] = c;
		}
	} else if (zoom == 4) {
	    for (j = 0; j < height; j += 4)
		for (i = 0; i < width; i += 4) {
		    data[k = (i + j * width)] = c = \
				hugegetpixel (image, (i >> 2) + x, (j >> 2) + y);
		    data[k + 1] = c;
		    data[k + 2] = c;
		    data[k + 3] = c;
		    k += width;
		    data[k] = c;
		    data[k + 1] = c;
		    data[k + 2] = c;
		    data[k + 3] = c;
		    k += width;
		    data[k] = c;
		    data[k + 1] = c;
		    data[k + 2] = c;
		    data[k + 3] = c;
		    k += width;
		    data[k] = c;
		    data[k + 1] = c;
		    data[k + 2] = c;
		    data[k + 3] = c;
		}
	} else {
	    for (j = 0; j < height; j += zoom)
		for (i = 0; i < width; i += zoom) {
		    c = hugegetpixel (image, i / zoom + x, j / zoom + y);
		    k = i + j * width;
		    for (m = 0; m < zoom; m++, k += width)
			for (l = 0; l < zoom; l++)
			    data[k + l] = c;
		}
	}
}

CWidget *Cdrawzoombox (const char *identifier, const char *main, \
	    Window parent, int x, int y, long width, long height, \
	    long posx, long posy, int zoom)
{
    CWidget *w;
    unsigned char *data = Cmalloc (width * height);
    HugeImage *hi = (HugeImage *) CUserOf (Cwidget (main));
    extractfromhugeimage (hi, data, posx, posy, width, height, zoom);

    w = Cdrawbwimage (identifier, parent, x, y,
		      width, height, data);
    CUserOf (w) = hi;
/*    w->destroy = destroy_huge; */
    free (data);
    return w;
}


CWidget *Cupdatezoombox (const char *identifier, long posx, long posy, int zoom)
{
    Window win;
    unsigned char *data;
    long width, height;
    HugeImage *hi;

    CWidget *w = Cwidget (identifier);
    hi = (HugeImage *) CUserOf (w);

    win = w->winid;
    width = w->width;
    height = w->height;

    data = Cmalloc (width * height);

    extractfromhugeimage (hi, data, posx, posy, width - 4, height - 4, zoom);

/* as with Cdrawbwimage: */
    greyscaletopix (w->ximage->data, data, width - 4, height - 4, \
						w->ximage->bits_per_pixel / 8);

    Cexpose (identifier);

    free (data);
    return w;
}



long CHugeImageRealWidth (const char *ident)
{
    return ((HugeImage *) CUserOf (Cwidget (ident)))->width;
}


long CHugeImageRealHeight (const char *ident)
{
    return ((HugeImage *) CUserOf (Cwidget (ident)))->height;
}
