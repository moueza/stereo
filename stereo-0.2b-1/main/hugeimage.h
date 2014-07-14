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
#ifndef HUG_IMAGE_H
#define HUG_IMAGE_H

typedef struct {
    long width;
    long height;
    int *temp;
    long *linestart;	/* first line cached in buffer (inclusive of this line) */
    long *lineend;	/* last line cached in buffer (exclusive of this line) */
    unsigned char **buffers;	/* buffer */
    int columnwidth;
    long numcolumns;
    unsigned long has_been_allocated;
} HugeImage;

#define BLOCKHEIGHT 64
/*the number of rows we are going to read from the file
   at a time. Small to reduce memory requirements, large to be
   faster */
#define COLUMNSHIFT 7
#define COLUMNWIDTH 128
#define COLUMNMASK 127

/*This must be about the size as the maximum mask you
   might want to use on the image. i.e. if you are going to
   readily access pixel(i,j) followed by pixel(i, j+4) then
   choose 5. If you are only going to readily access
   pixel(i+1,j) after pixel(i,j) the choose 1.
   In any even, mask must also be accessed from left to right. */
#define LINESCACHED 128

void hugeloadnextbuf (HugeImage * image, long x, long y, int i);

/*getpixel gets a pixel grey value (0-255) from a huge image.
   It exploits a caching mechanism to improve speed. This is a little
   faster than the caching mechanism used by the FILE type.
   The caching mechanism works best when pixels are accessed from left to
   right in the image. See LINESCACHED above. */

CWidget *Cdrawhugebwimage (const char *identifier, Window parent, int x, int y,
		       long *width, long *height, const char *imagefile);
void Crenderzoombox (CWidget * w, int x, int y, int rendw, int rendh);
void extractfromhugeimage (HugeImage * image, unsigned char *data,
			long x, long y, int width, int height, int zoom);
CWidget *Cdrawzoombox (const char *identifier, const char *main, \
			    Window parent, int x, int y, \
		long width, long height, long posx, long posy, int zoom);
CWidget *Cupdatezoombox (const char *identifier, long posx, long posy, int zoom);
long CHugeImageRealHeight (const char *ident);
long CHugeImageRealWidth (const char *ident);

#endif
