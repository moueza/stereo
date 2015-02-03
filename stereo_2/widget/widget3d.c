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
#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <math.h>
#include <my_string.h>

#include "stringtools.h"
#include "app_glob.c"

#include "coolwidget.h"
#include "widget3d.h"

#include "vgagl.h"
#include "3dkit.h"

#ifdef USING_MATRIXLIB
#include "matrix.h"
#endif

#include "imagewidget.h"

#include "mad.h"

static Pixmap widget3d_currentpixmap;
static long widget3d_thres;

void local_striangle (int x0, int y0,
		      int x1, int y1,
		      int x2, int y2, int z0, int bf)
{
    long n;
    XPoint p[3];
    p[0].x = x0;
    p[0].y = y0;
    p[1].x = x1;
    p[1].y = y1;
    p[2].x = x2;
    p[2].y = y2;

    if ((((n = (x0 - x1) * (y0 - y2) - (y0 - y1) * (x0 - x2)) > 0) + bf) & 1 || bf == 2) {
	if(abs(n) < widget3d_thres) {
	    if(abs(x0) + abs(x1) + abs(x2) + abs(y0) + abs(y1) + abs(y2) < 32000) {
	    if(Cdepth >= 8 && (ClassOfVisual(Cvisual) == PseudoColor || ClassOfVisual (Cvisual) == StaticGray))
		Csetcolor(z0);
	    else
		Csetcolor(Cpixel[z0]);
	    XFillPolygon (CDisplay, widget3d_currentpixmap, CGC, p, 3, Convex, CoordModeOrigin);
	    }
	}
    }
}



void local_drawline (int x1, int y1, int x2, int y2, int c)
{
    Csetcolor(c);
    if(abs(x1) + abs(x2) + abs(y1) + abs(y2) < 32000)
	    Cline(widget3d_currentpixmap, x1, y1, x2, y2);
}


void local_setpixel (int x, int y, int c)
{
    Csetcolor(c);
    XDrawPoint(CDisplay, widget3d_currentpixmap, CGC, x, y);
}



/* this redraws the object. If force is non-zero then the object WILL
   be redrawn. If not the routine checks if there are any events in the
   event queue and only redraws if the event queue is empty. */
CWidget * Credraw3dobject (const char *ident, int force)
{
    CWidget *w = Cwidget (ident);
    Window win = w->winid;
    TD_Solid *object = w->solid;

    if (force || !CMousePending(ident)) {
	gl_setcontext (w->gl_graphicscontext);
	gl_enableclipping ();

	widget3d_thres = w->width * w->height;
	widget3d_currentpixmap = w->pixmap;
	Csetcolor (C_FLAT);
	Crect (widget3d_currentpixmap, 0, 0, w->width - 4, w->height - 4);
	TD_draw_solid (object);
	XCopyArea (CDisplay, widget3d_currentpixmap, win, CGC, 0, 0, w->width - 4, w->height - 4, 2, 2);
    }
    return w;
}




/* Allocates memory for the TD_Solid structure. returns 0 on error */
TD_Solid *TD_allocate_solid (int max_num_surfaces)
{
    TD_Solid *object;

    if ((object = malloc (sizeof (TD_Solid))) == NULL)
	return 0;

    memset (object, 0, sizeof (TD_Solid));

    object->num_surfaces = max_num_surfaces;

    if ((object->surf = malloc (max_num_surfaces * sizeof (TD_Surface))) == NULL)
	return 0;

    memset (object->surf, 0, max_num_surfaces * sizeof (TD_Surface));

    return object;
}


/*returns NULL on error, else returns a pointer to the surface points */
TD_Surface *Calloc_surf (const char *ident, int surf_width, int surf_height, int *surf)
{
    int j = 0;
    CWidget *w = Cwidget (ident);

    for (; j < w->solid->num_surfaces; j++)
	if (!w->solid->surf[j].point) {		/* find the first unused surface */
	    if (surf)
		*surf = j;
	    w->solid->surf[j].point = Cmalloc (surf_width * surf_height * sizeof (TD_Point));
	    memset (w->solid->surf[j].point, 0, surf_width * surf_height * sizeof (TD_Point));
	    w->solid->surf[j].w = surf_width;
	    w->solid->surf[j].l = surf_height;
	    return &(w->solid->surf[j]);
	}
    Cerror ("Max number of surfaces exceeded for this solid.\n");
    return NULL;
}

/* free the i'th surface if widget ident */
void Cfree_surf (CWidget *w, int i)
{

    if(w->solid->surf[i].point) {
	free(w->solid->surf[i].point);
	w->solid->surf[i].point = NULL;
    }
}

/* free the last surface of widget ident, returns first available surface in surf */
/* returns 1 if there are any remaining unfree'd surfaces */
int Cfree_last_surf (CWidget *w, int *surf)
{
    int j = 0;
    for (; j < w->solid->num_surfaces; j++)
	if (!w->solid->surf[j].point)		/* find the first unused surface */
	    break;
    if (surf)
	*surf = j;
    if(j > 0)
	Cfree_surf(w, --j);
    if(j)
	return 1;
    return 0;
}

void Cclear_all_surfaces(const char *ident)
{
    CWidget *w = Cwidget(ident);
    while(Cfree_last_surf (w, NULL));
}

void Cinit_surf_points (const char *ident, int width, int height, TD_Point data[])
{
    TD_Surface *s = Calloc_surf (ident, width, height, NULL);
    memcpy (s->point, data, width * height * sizeof (TD_Point));
    TD_initcolor (s, -256);
}

#ifdef USING_MATRIXLIB

void Cmatrix_to_surf (const char *ident, int surf_width, int surf_height, Matrix * x, Matrix * offset, double scale)
{
    TD_Point *p = Cmalloc (x->columns * sizeof (TD_Point));
    int i;

    for (i = 0; i < x->columns; i++) {
	p[i].x = (double) (Mard (*x, 0, i) + Mard (*offset, 0, 0)) * scale;
	p[i].y = (double) (Mard (*x, 1, i) + Mard (*offset, 1, 0)) * scale;
	p[i].z = (double) -(Mard (*x, 2, i) + Mard (*offset, 2, 0)) * scale;
	p[i].dirx = 0;
	p[i].diry = 0;
	p[i].dirz = 0;
    }

    Cinit_surf_points (ident, surf_width, surf_height, p);
    free (p);
}

#endif

CWidget * Cdraw3dobject (const char *identifier, Window parent, int x, int y,
	       int width, int height, int defaults, int max_num_surfaces)
{
    int i, j, n = 0;
    CWidget *w;

    TD_Solid *object;

    if (!(object = TD_allocate_solid (max_num_surfaces)))
	Cerror ("Error allocating memeory in call to Cdraw3dobject.\n");

    width &= 0xFFFFFFFC;	/* width must be a multiple of 4 */

    if (defaults) {

	n = object->num_surfaces;

	for (j = 0; j < n; j++) {
	    object->surf[j].backfacing = 1;	/*don't draw any of surface that faces away */
	    object->surf[j].depth_per_color = 6;	/*2^6 = 64 colors in the grey scale */
	    object->surf[j].bitmap1 = NULL;
	    object->surf[j].bitmap2 = NULL;
	    object->surf[j].mesh_color = Ccolor (6);
	}

	object->alpha = 0;	/* begin all at zero (flight dynamics */
	object->beta = 0;	/* says plane is level */
	object->gamma = 0;

	object->xlight = -147;	/* lighting out of the screen,... */
	object->ylight = -147;	/* ...to the right,... */
	object->zlight = 147;	/* ...and from the top. */

	object->distance = 85000;	/* distance of the camera from the */
	/* origin */

	object->x_cam = 85000;
	object->y_cam = 0;
	object->z_cam = 0;

/* These two are scale factors for the screen: */
/* xscale is now calculated so that the maximum volume (-2^15 to 2^15 or
   -2^31 to 2^31) will just fit inside the screen width at this distance: */
	object->yscale = object->xscale = (long) object->distance * (width + height) / (32768 * 4);
/*to get display aspect square */

/*The above gives an average (not to telescopic, and not to wide angle) view */

/*use any triangle or linedrawing routine: */
	object->draw_triangle = gl_triangle;
	object->draw_striangle = local_striangle;
	object->draw_wtriangle = gl_wtriangle;
	object->draw_swtriangle = gl_swtriangle;
	object->draw_line = local_drawline;
	object->draw_point = local_setpixel;

/* very important to set TDOPTION_INIT_ROTATION_MATRIX if you don't
   calculate the rotation matrix yourself. */

	object->option_flags = TDOPTION_INIT_ROTATION_MATRIX
	    | TDOPTION_ALL_SAME_RENDER | TDOPTION_SORT_SURFACES
	    | TDOPTION_ROTATE_OBJECT | TDOPTION_LIGHT_SOURCE_CAM
	    | TDOPTION_FLAT_TRIANGLE;

	object->render = TD_MESH_AND_SOLID;	/*how we want to render it */

	object->posx = width / 2;
	object->posy = height / 2;
    }

    w = Csetupwidget (identifier, parent, x, y,
			  width + 4, height + 4, CTHREED_WIDGET, INPUT_MOTION, C_FLAT, 1);

    w->solid = object;

    w->pixmap = XCreatePixmap (CDisplay, CMain, width, height, Cdepth);

    if (defaults) {
	for (j = 0; j < n; j++) {
	    if (Cdepth >= 8 && (ClassOfVisual (Cvisual) == PseudoColor || ClassOfVisual (Cvisual) == StaticGray)) {
/*In this case only we can use the actual pixel values because the
   triangle routines can write the calculated pixel values to the buffer
   which will be the same as the actual (hardware interpreted) pixel values */
		object->surf[j].shadow = Cgrey (10);
		object->surf[j].maxcolor = Cgrey (61);
	    } else {
/*In all other cases, the triangle routine must calculate the color
   and then convert it into a actual pixel value from the lookup
   table. The triangle routine itself checks if
   bytesperpixel is non-unity and performs lookup if so. */
		object->surf[j].shadow = 43 + 10;
		object->surf[j].maxcolor = 43 + 61;
	    }
	}
    }

/*now use vgagl graphics contexts */
    gl_setcontextvirtual (width, height, (Cdepth + 7) / 8,
			  Cdepth, 0);
    w->gl_graphicscontext = Cmalloc (sizeof (GraphicsContext));
    gl_getcontext (w->gl_graphicscontext);

    for(i=0;i<256;i++)
	gl_trisetcolorlookup(i, Cpixel[i]);

    Credraw3dobject (identifier, 2);

    return w;
}



void Crender3dwidget (CWidget *wdt, int x, int y, int rendw, int rendh)
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

    XCopyArea(CDisplay, widget3d_currentpixmap, win, CGC, xim, yim, rendw, rendh, xwin, ywin);

    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 1);
}
