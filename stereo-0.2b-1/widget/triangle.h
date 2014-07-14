/*

    3DKIT   version   1.2
    High speed 3D graphics and rendering library for Linux.

    Copyright (C) 1996  Paul Sheer   psheer@hertz.mech.wits.ac.za

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
    MA 02111-1307, USA

*/

/*

File: triangle.h

*/

#define max(x,y)     (((x) > (y)) ? (x) : (y))
#define min(x,y)     (((x) < (y)) ? (x) : (y))

typedef struct {

unsigned char *bitmap1;
unsigned char *bitmap2;

int bf;

} TD_tridata;



void gl_triangle (int x0, int y0, int z0, int x1, int y1, int z1, int x2, int y2, int z2, int bf);
void gl_wtriangle (int x0, int y0, int xd0, int yd0, int z0, \
		   int x1, int y1, int xd1, int yd1, int z1, \
		   int x2, int y2, int xd2, int yd2, int z2, \
	TD_tridata * tri);  /*This does not alter tri structure*/
void gl_swtriangle (int x0, int y0, int xd0, int yd0, \
		   int x1, int y1, int xd1, int yd1, \
		   int x2, int y2, int xd2, int yd2, int c, \
	TD_tridata * tri);  /*This does not alter tri structure*/

void gl_striangle (int x0, int y0, int x1, int y1, int x2, int y2, int color, int bf);


#define TRIANGLE_COLOR_LOOKUP_TABLE_SIZE 4096

void gl_trisetcolorlookup(int i, long c);
void gl_trisetdrawpoint(void (setpixelfunc) (int, int, int));

