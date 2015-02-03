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


/* setup application */

static unsigned char cool_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x4f, 0xe4, 0x07,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x4a, 0xa4,
   0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xcf, 0x4a,
   0xa4, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x33,
   0x4b, 0xa4, 0x6f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe8,
   0x8c, 0x5c, 0x7c, 0xb2, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x16, 0x63, 0xe2, 0x47, 0xcc, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xb1, 0x10, 0x21, 0x84, 0x30, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x80, 0x4c, 0x8c, 0x20, 0x08, 0x41, 0xb4, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x40, 0xc2, 0x43, 0x20, 0x08, 0x81, 0xeb, 0x01, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x20, 0x31, 0x21, 0x10, 0x08, 0x02, 0xb3, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xb0, 0x08, 0x26, 0x10, 0x10, 0x84, 0xcc, 0x07,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x04, 0x7e, 0x10, 0x10, 0x64, 0xb1,
   0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x02, 0x81, 0x1f, 0x10, 0x3e,
   0x42, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x81, 0x00, 0xe1, 0xff,
   0x41, 0x84, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x92, 0x81, 0x80, 0x00,
   0x81, 0x80, 0x08, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x42, 0x80,
   0x00, 0x01, 0x81, 0x10, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x4e,
   0x40, 0x00, 0x01, 0x01, 0xa1, 0x7a, 0x00, 0x00, 0x00, 0x00, 0x80, 0x08,
   0x31, 0x40, 0x00, 0x01, 0x02, 0xe1, 0x74, 0x00, 0x00, 0x00, 0x00, 0x80,
   0x04, 0xc1, 0x40, 0x80, 0x00, 0x02, 0x22, 0xa9, 0x00, 0x00, 0x00, 0x00,
   0x80, 0x85, 0x00, 0x43, 0x80, 0x00, 0x02, 0x1a, 0xaa, 0x00, 0x00, 0x00,
   0x00, 0x40, 0x87, 0x00, 0x7d, 0x80, 0x00, 0x04, 0x26, 0xb2, 0x00, 0x00,
   0x00, 0x00, 0x40, 0x45, 0x00, 0xc1, 0x83, 0x00, 0xfc, 0x41, 0x54, 0x01,
   0x00, 0x00, 0x00, 0x40, 0x49, 0x80, 0x00, 0xfc, 0xff, 0x87, 0x40, 0x54,
   0x01, 0x00, 0x00, 0x00, 0xf8, 0x59, 0x80, 0x00, 0x08, 0x40, 0x00, 0x81,
   0x98, 0x01, 0x00, 0x00, 0x80, 0xdf, 0x6e, 0x80, 0x00, 0x08, 0x40, 0x00,
   0x81, 0xa8, 0x01, 0x00, 0x00, 0x78, 0x38, 0x98, 0x40, 0x00, 0x08, 0x40,
   0x00, 0x82, 0x26, 0x01, 0x00, 0x80, 0xbf, 0x0f, 0x20, 0x41, 0x00, 0x04,
   0x80, 0x00, 0x82, 0x45, 0x03, 0x00, 0x60, 0x70, 0x10, 0x78, 0x47, 0x00,
   0x04, 0x80, 0x00, 0x62, 0x48, 0x03, 0x00, 0xfc, 0x0f, 0xe0, 0x8f, 0x79,
   0x00, 0x04, 0x80, 0x00, 0x1c, 0x48, 0x03, 0x80, 0xe3, 0x10, 0x38, 0x10,
   0xc1, 0x03, 0x04, 0x80, 0x80, 0x27, 0x48, 0x03, 0x60, 0x1c, 0x20, 0x07,
   0x20, 0x02, 0xfc, 0x04, 0x80, 0x7c, 0x20, 0x90, 0x03, 0x90, 0x28, 0xe0,
   0x04, 0x40, 0x04, 0x10, 0xff, 0xff, 0x23, 0x20, 0x50, 0x03, 0x88, 0x48,
   0x1e, 0x04, 0x40, 0x04, 0x10, 0x00, 0x02, 0x20, 0x40, 0x50, 0x03, 0x94,
   0x08, 0x01, 0x08, 0x80, 0x04, 0x10, 0x00, 0x02, 0x20, 0x40, 0x70, 0x03,
   0x14, 0x09, 0x03, 0x08, 0x80, 0x0b, 0x08, 0x00, 0x02, 0x40, 0x40, 0x50,
   0x03, 0xa4, 0x8f, 0x01, 0x10, 0x70, 0x08, 0x08, 0x00, 0x02, 0x40, 0x40,
   0x48, 0x03, 0x42, 0x48, 0x04, 0x10, 0x0f, 0x08, 0x08, 0x00, 0x02, 0x40,
   0x40, 0x4c, 0x03, 0x42, 0x30, 0x00, 0xf0, 0x10, 0x10, 0x08, 0x00, 0x02,
   0x40, 0x40, 0x4b, 0x03, 0x26, 0x28, 0x08, 0x0e, 0x10, 0x10, 0x08, 0x00,
   0x02, 0x40, 0xc0, 0x48, 0x03, 0x2a, 0x47, 0xc0, 0x01, 0x20, 0x10, 0x08,
   0x00, 0x02, 0x40, 0x30, 0x48, 0x01, 0xf2, 0x40, 0x3c, 0x02, 0x20, 0xf0,
   0x09, 0x00, 0x02, 0x40, 0x0f, 0x48, 0x01, 0x22, 0xa0, 0x13, 0x02, 0x20,
   0x1c, 0x1e, 0x00, 0x02, 0xf0, 0x08, 0x48, 0x01, 0x22, 0xa0, 0x00, 0x04,
   0x20, 0x13, 0xe0, 0x0f, 0xc2, 0x0f, 0x08, 0x48, 0x01, 0x22, 0xa0, 0x20,
   0x04, 0xf0, 0x11, 0x00, 0xf2, 0x3f, 0x01, 0x08, 0x48, 0x01, 0x22, 0x20,
   0x01, 0x04, 0x0f, 0x11, 0x00, 0x02, 0x00, 0x01, 0x08, 0xc8, 0x00, 0x22,
   0xa0, 0x23, 0xf4, 0x00, 0x11, 0x00, 0x02, 0x00, 0x01, 0x08, 0xc8, 0x00,
   0x22, 0x60, 0x0d, 0x8f, 0x00, 0x11, 0x00, 0x02, 0x00, 0x01, 0x08, 0x48,
   0x00, 0x22, 0x18, 0xf1, 0x80, 0x00, 0x11, 0x00, 0x02, 0x00, 0x01, 0x08,
   0x28, 0x00, 0x2c, 0x07, 0x01, 0x80, 0x00, 0x11, 0x00, 0x02, 0x00, 0x01,
   0x08, 0x18, 0x00, 0xf0, 0x00, 0x21, 0x80, 0x00, 0x11, 0x00, 0x02, 0x00,
   0x01, 0x08, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x11, 0x00, 0x02,
   0x00, 0x01, 0x08, 0x03, 0x00, 0x00, 0x00, 0x21, 0x80, 0x00, 0x11, 0x00,
   0x02, 0x00, 0x01, 0xc8, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x39,
   0x00, 0x02, 0x00, 0x01, 0x38, 0x00, 0x00, 0x00, 0x00, 0x21, 0x80, 0x00,
   0xc7, 0x07, 0x02, 0x00, 0x81, 0x07, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
   0xe0, 0x00, 0xf8, 0x03, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21,
   0x80, 0x1c, 0x00, 0x00, 0xfc, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x2c, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00};


/*
Colormap allocation:

A colormap is allocated as follows into the array Cpixel:
('i' refers to Cpixel[i])

These are allocated in sequential palette cells, hence
(for pseudocolor only) Cpixel[j + i] = Cpixel[j] + i,
for j = 0,16,43. Obviously in TrueColor this is not true.

,----------+------------------------------------------.
|    i     |   colors                                 |
+----------+------------------------------------------+
|  0-15    | 16 levels of the widget colors that      |
|          | make up button bevels etc. Starting from |
|          | (i=0) black (for shadowed bevels), up    |
|          | to (i=15) bright highlighted bevels      |
|          | those facing up-to-the-left. These       |
|          | are sequential (see next block).         |
+----------+------------------------------------------+
|  16-42   | 3^3 combinations of RGB, vis. (0,0,0),   |
|          | (0,0,127), (0,0,255), (0,127,0), ...     |
|          | ... (255,255,255).                       |
+----------+------------------------------------------+
|  43-106  | 64 levels of grey. (optional)            |
+----------+------------------------------------------+
|  107->   | For other colors. (Not used at present)  |
`----------+------------------------------------------'
*/

/* Thence macros are defined in coolwidgets.h for color lookup */


#include <config.h>
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#define  DEF_APP_GLOB		/* so that globals get defined not externed */

#include "coolwidget.h"
#include "stringtools.h"

#include <signal.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "mad.h"

static int verbose_operation = 0;
CInitData *given = 0;


/* defaults */
#define DEFAULT_DISPLAY			NULL
#define DEFAULT_GEOM			NULL
#define DEFAULT_FONT			"8x13bold"
#define DEFAULT_BG_COLOR		"grey"
#define DEFAULT_WIDGET_COLOR_R		"0.9"
#define DEFAULT_WIDGET_COLOR_G		"1.1"
#define DEFAULT_WIDGET_COLOR_B		"1.4"

#define DEFAULT_BDWIDTH         1

struct resource_param {
    char *name;
    char **value;
};

static char *init_display = DEFAULT_DISPLAY;
static char *init_geometry = DEFAULT_GEOM;
static char *init_font = DEFAULT_FONT;
static char *init_bg_color = DEFAULT_BG_COLOR;
static char *init_fg_color_red = DEFAULT_WIDGET_COLOR_R;
static char *init_fg_color_green = DEFAULT_WIDGET_COLOR_G;
static char *init_fg_color_blue = DEFAULT_WIDGET_COLOR_B;

/* Resources */

struct resource_param resources[] =
{
    {"display", &init_display},
    {"geometry", &init_geometry},
    {"font", &init_font},
    {"background", &init_bg_color},
    {"fg_red", &init_fg_color_red},
    {"fg_blue", &init_fg_color_blue},
    {"fg_green", &init_fg_color_green},
    {0, 0}
};

static void alloccolorerror (void)
{
    Cerror ("Cannot allocate colors. Could be to many applications\ntrying to use the colormap. If closing other apps doesn't\nhelp, then your graphics hardware may be inadequite.\n");
}

static void init_widgets ()
{
    int i;
    CLastwidget = 1;		/*widget[0] is never used since index 0 is used
				   to indicate an error message */
    for (i = 0; i < MAX_NUMBER_OF_WIDGETS; i++)
	CW (i) = NULL;		/*initialise */
}

static void open_display (char *app_name)
{
    if ((CDisplay = XOpenDisplay (init_display)) == NULL) {
	fprintf (stderr, "%s: can't open display named \"%s\"\n",
		 app_name, XDisplayName (init_display));
	exit (1);
    }
    if(verbose_operation)
        printf ("Opened display \"%s\"\n", XDisplayName (init_display));
}


static void get_resources ()
{
    int i;
    char *type;
    XrmValue value;
    XrmDatabase rdb;
    XrmInitialize ();
    rdb = XrmGetFileDatabase (catstrs (getenv ("HOME"), "/.Xdefaults", 0));
    if (rdb != NULL) {
	for (i = 0; resources[i].name; i++) {
	    char *rname = catstrs (CAppName, "*", resources[i].name, 0);
	    if (XrmGetResource (rdb, rname, rname,
				&type, &value)) {
		*resources[i].value = value.addr;
	    }
	}
    }
}

static void load_font ()
{
    if ((CFontStruct = XLoadQueryFont (CDisplay, init_font)) == NULL) {
	fprintf (stderr, "%s: display %s cannot load font %s\n",
		 CAppName, DisplayString (CDisplay), init_font);
	exit (1);
    }
#if 0
    if (CFontStruct->per_char['I'].width == CFontStruct->per_char['M'].width ) {
	Cfont_is_proportional = 0;
	CMean_font_width = CFontStruct->max_bounds.width;
    } else {
	Cfont_is_proportional = 1;
	CMean_font_width = XTextWidth (CFontStruct, "The quick brown fox Jumps over the lazy dog.", 45) / 45;
    }
#else
    Cfont_is_proportional = 0;
    CMean_font_width = CFontStruct->max_bounds.width;
#endif
}

static void visual_comments (long class)
{
    switch (class) {
    case PseudoColor:
	printf ("PseudoColor");
	if (Cdepth >= 7)
	    printf (" - depth ok, this will work.\n");
	else
	    printf (" - depth low, this may not work.\n");
	break;
    case GrayScale:
	printf ("Grayscale -\n");
	printf ("Mmmmh, haven't tried this visual class, let's see what happens.\n");
	break;
    case DirectColor:
	printf ("DirectColor -\n");
	printf ("Mmmmh, haven't tried this visual class, let's see what happens.\n");
	break;
    case StaticColor:
	printf ("StaticColor - lets give it a try.\n");
	break;
    case StaticGray:
	printf ("StaticGray - lets give it a try.\n");
	break;
    case TrueColor:
	printf ("TrueColor - fine.\n");
	break;
    default:
	Cerror ("?\nVisual class unknown.\n");
	break;
    }
}

/* must be free'd */
XColor * get_cells(Colormap cmap, int *size)
{
    int i;
    XColor *c;
    *size = DisplayCells (CDisplay, DefaultScreen (CDisplay));
    c = Cmalloc (*size * sizeof (XColor));
    for (i = 0; i < *size; i++)
	c[i].pixel = i;
    XQueryColors (CDisplay, cmap, c, *size);
    return c;
}

int CBitsPerRGB = 0;


#define BitsPerRGBofVisual(v) (v->bits_per_rgb)


/* find the closest color without allocating it */
int CGetCloseColor (XColor * cells, int ncells, XColor color, long *error)
{
    unsigned long merror = (unsigned long) -1;
    unsigned long e;
    int min = 0, i;
    unsigned long mask = 0xFFFF0000;

    mask >>= BitsPerRGBofVisual(Cvisual);
    for (i = 0; i < ncells; i++) {
	e = 8 * abs ((int) (color.red & mask) - (cells[i].red & mask)) + 10 * abs ((int) (color.green & mask) - (cells[i].green & mask)) + 5 * abs ((int) (color.blue & mask) - (cells[i].blue & mask));
	if (e < merror) {
	    merror = e;
	    min = i;
	}
    }
    merror = 8 * abs ((int) (color.red & mask) - (cells[min].red & mask)) + 10 * abs ((int) (color.green & mask) - (cells[min].green & mask)) + 5 * abs ((int) (color.blue & mask) - (cells[min].blue & mask));
    if(error)
	*error = (long) merror;
    return min;
}

#define grey_intense(i) (i * 65535 / 63)

/* return -1 if not found. Meaning that another coolwidget app is not running */
int find_coolwidget_grey_scale (XColor * c, int ncells)
{
    unsigned long mask = 0xFFFF0000;
    int i, j;
    mask >>= BitsPerRGBofVisual (Cvisual);

    for (i = 0; i < ncells; i++) {
	for (j = 0; j < 64; j++)
	    if (!((c[i + j].green & mask) == (grey_intense (j) & mask)
		  && c[i + j].red == c[i + j].green && c[i + j].green == c[i + j].blue))
		goto cont;
	return i;
      cont:;
    }
    return -1;
}


void CAllocColorCells (Colormap colormap, Bool contig,
	unsigned long plane_masks[], unsigned int nplanes,
	unsigned long pixels[], unsigned int npixels)
{
    if(!XAllocColorCells (CDisplay, colormap, contig,
		plane_masks, nplanes, pixels, npixels))
	alloccolorerror ();
}

void CAllocColor (Colormap cmap, XColor *c)
{
    if (!XAllocColor (CDisplay, cmap, c))
	alloccolorerror ();
}

/* the 16 coolwidget widget colors: (provided the visual is sufficient) */
static void get_button_color (XColor * color, int i)
{
    double r, g, b, min_wc;

    r = 1 / atof (init_fg_color_red);
    g = 1 / atof (init_fg_color_green);
    b = 1 / atof (init_fg_color_blue);

    min_wc = min (r, min (g, b));

    color->red = (float) 65535 *my_pow ((float) i / 20, r) * my_pow (0.75, -min_wc);
    color->green = (float) 65535 *my_pow ((float) i / 20, g) * my_pow (0.75, -min_wc);
    color->blue = (float) 65535 *my_pow ((float) i / 20, b) * my_pow (0.75, -min_wc);
    color->flags = DoRed | DoBlue | DoGreen;
}

/* takes 0-26 and converts it to RGB */
static void get_general_colors (XColor *color, int i)
{
    color->red = (long) (i / 9) * 65535 / 2;
    color->green = (long) ((i % 9) / 3) * 65535 / 2;
    color->blue = (long) (i % 3) * 65535 / 2;
    color->flags = DoRed | DoBlue | DoGreen;
}

static void get_grey_colors (XColor *color, int i)
{
    color->red = color->green = grey_intense(i);
    color->blue = grey_intense(i);
    color->flags = DoRed | DoBlue | DoGreen;
}


void alloc_grey_scale (Colormap cmap)
{
    XColor color;
    int i;

    if (Cusinggreyscale) {
	for (i = 0; i < 64; i++) {
	    get_grey_colors (&color, i);
	    CAllocColor (cmap, &color);
	    Cpixel[i + 43] = color.pixel;
	}
    }
}

/* this sets up static color, but tries to be more intelligent about the
   way it handles grey scales */
static void setup_staticcolor ()
{
    XColor *c;
    unsigned short *grey_levels;
    XColor color;
    int size, i, j, k, n, m = 0, num_greys, grey;
    Colormap default_cmap;

    default_cmap = DefaultColormap (CDisplay, DefaultScreen (CDisplay));

    c = get_cells(default_cmap, &size);
    grey_levels = Cmalloc ((size + 2) * sizeof (unsigned short));
    num_greys = 0;

/* we are probably not going to find our coolwwidget colors here,
   so use greyscale for the buttons. first count how many greys,
   and sort them: */

    grey = 0;
    for (i = 0; i < size; i++) {
	if (c[i].red == c[i].green && c[i].green == c[i].blue) {
	    if (grey) {
		for (n = 0; n < grey; n++)
		    if (c[i].green == grey_levels[n])
			goto cont;
		for (n = grey - 1; n >= 0; n--)
		    if (grey_levels[n] > c[i].green) {
			memmove (&(grey_levels[n + 1]), &(grey_levels[n]), (grey - n) * sizeof (unsigned short));
			grey_levels[n] = c[i].green;
			grey++;
			goto cont;
		    }
		grey_levels[grey] = c[i].green;
	    } else
		grey_levels[grey] = c[i].green;
	    grey++;
	  cont:;
	}
    }
    num_greys = grey;

    if (num_greys <= 2) {	/* there's just no hope  :(   */
	if(verbose_operation)
	    printf ("This will work, but it may look terrible.\n");
	for (grey = 0; grey < 16; grey++) {
	    color.flags = DoRed | DoGreen | DoBlue;
	    color.red = grey * 65535 / 15;
	    color.green = grey * 65535 / 15;
	    color.blue = grey * 65535 / 15;
	    if (!XAllocColor (CDisplay, default_cmap, &color))
		alloccolorerror ();
	    Cpixel[grey] = color.pixel;
	}
	alloc_grey_scale (default_cmap);
    } else {
	j = 0;
	k = 0;
	for (grey = 0; grey < num_greys; grey++) {
/* button colors */
	    color.red = color.green = grey_levels[grey];
	    color.blue = grey_levels[grey];
	    color.flags = DoRed | DoGreen | DoBlue;

	    for (; j < (grey + 1) * 16 / num_greys; j++) {
		CAllocColor (default_cmap, &color);
		Cpixel[j] = color.pixel;
	    }
/* grey scale */
	    if (Cusinggreyscale) {
		for (; k < (grey + 1) * 64 / num_greys; k++) {
		    CAllocColor (default_cmap, &color);
		    Cpixel[k + 43] = color.pixel;
		}
	    }
	}
    }

    for(i=0;i<27;i++) {
	get_general_colors (&color, i);
	m = CGetCloseColor (c, size, color, 0);
	CAllocColor (default_cmap, &(c[m]));
	Cpixel[16 + i] = c[m].pixel;
    }

    free (grey_levels);
    free (c);
}


static void setup_alloc_colors()
{
    int i;
    XColor color;
    Colormap default_cmap;

    default_cmap = DefaultColormap (CDisplay, DefaultScreen (CDisplay));

    color.flags = DoRed | DoGreen | DoBlue;

    for (i = 0; i < 16; i++) {
	get_button_color(&color, i);
	CAllocColor (default_cmap, &color);
	Cpixel[i] = color.pixel;
    }

    for (i = 0; i < 27; i++) {
	get_general_colors (&color, i);
	CAllocColor (default_cmap, &color);
	Cpixel[16 + i] = color.pixel;
    }

    alloc_grey_scale (default_cmap);
}


void store_grey_scale (Colormap cmap)
{
    XColor color;
    int i;
    if(verbose_operation)
	printf ("Storing grey scale.\n");
    if (!XAllocColorCells (CDisplay, cmap, 1, Cplane, 6, Cpixel + 43, 1))
	alloccolorerror ();
    for (i = 0; i < 64; i++) {
	Cpixel[43 + i] = Cpixel[43] + i;
	color.pixel = Cpixel[43 + i];
	get_grey_colors (&color, i);
	XStoreColor (CDisplay, cmap, &color);
    }
}


void try_color (Colormap cmap, XColor * c, int size, XColor color, int i)
{
    int x;
    long error;
    XColor close;

    x = CGetCloseColor (c, size, color, &error);
    close = c[x];

    if (error) {
	if (XAllocColorCells (CDisplay, cmap, 0, Cplane, 0, Cpixel + i, 1)) {
	    color.pixel = Cpixel[i];
	    XStoreColor (CDisplay, cmap, &color);
	    if(verbose_operation)
		printf ("S,");
	    return;
	}
    }
    CAllocColor (cmap, &close);
    Cpixel[i] = close.pixel;
    if(verbose_operation)
	printf ("%ld,", error >> 11);
}




/*
   for PseudoColor displays. This tries to be conservative in the number
   of entries its going to store, while still keeping the colors exact.
   first it looks for an entry in the default colormap and only stores
   in the map if no match is found. Multiple coolwidget applications can
   therefore share the same map. At worst 16 + 27 of the palette are used
   plus another 64 if you are using greyscale.
 */
static void setup_store_colors ()
{
    int i, size;
    XColor *c;
    XColor color;
    Colormap default_cmap;

    default_cmap = DefaultColormap (CDisplay, DefaultScreen (CDisplay));
    c = get_cells (default_cmap, &size);

    color.flags = DoRed | DoGreen | DoBlue;

/* grey scale has to be contigous to be fast so store a 64 colors */
    if (Cusinggreyscale) {
/*
   i = find_coolwidget_grey_scale (c, size);
   if (i >= 0) {
   if(verbose_operation)
   printf ("found grey scale\n");
   alloc_grey_scale (default_cmap);
   } else {
 */
	store_grey_scale (default_cmap);
/*    } */
    }
    if (verbose_operation)
	printf ("Trying colors...\n");

    for (i = 0; i < 16; i++) {
	get_button_color (&color, i);
	try_color (default_cmap, c, size, color, i);
    }

    for (i = 0; i < 27; i++) {
	get_general_colors (&color, i);
	try_color (default_cmap, c, size, color, i + 16);
    }
    if (verbose_operation)
	printf ("\n");
    free (c);
}




static void setup_colormap (long class)
{
    switch(class) {
	case StaticColor:
	case StaticGray:
	    setup_staticcolor ();
	    break;
	case GrayScale:
	case DirectColor:
	case TrueColor:
	    setup_alloc_colors ();
	    return;
	case PseudoColor:
	    setup_store_colors ();
	    break;
    }
}



static void hints (int x, int y, int width, int height)
{
    int bitmask;
    char default_geometry[80];

/* Fill out a XsizeHints structure to inform the window manager
 * of desired size and location of main window.
 */
    if ((p_XSH = XAllocSizeHints ()) == NULL) {
	fprintf (stderr, "Error allocating size hints!\n");
	exit (1);
    }
    p_XSH->flags = (PPosition | PSize | PMinSize);
    p_XSH->height = height;
    p_XSH->min_height = 80;
    p_XSH->width = width;
    p_XSH->min_width = 128;
    p_XSH->x = x;
    p_XSH->y = y;

/* Construct a default geometry string */
    if (!init_geometry) {
	sprintf (default_geometry, "%dx%d+%d+%d", p_XSH->width,
		p_XSH->height, p_XSH->x, p_XSH->y);
	init_geometry = default_geometry;
    }

/* Process the geometry specification */
    bitmask = XGeometry (CDisplay, DefaultScreen (CDisplay),
			 init_geometry, default_geometry, DEFAULT_BDWIDTH,
			 1, 1, 0, 0, &(p_XSH->x), &(p_XSH->y),
			 &(p_XSH->width), &(p_XSH->height));

/* Check bitmask and set flags in XSizeHints structure */
    if (bitmask & (XValue | YValue))
	p_XSH->flags |= USPosition;
    if (bitmask & (WidthValue | HeightValue))
	p_XSH->flags |= USSize;
}

static void create_main_win (int argc, char **argv)
{
    XSetWindowAttributes xswa;
    XGCValues gcv;
    XColor color;
    unsigned long bg_pixel;

    if (XParseColor(CDisplay, DefaultColormap (CDisplay, DefaultScreen (CDisplay)), init_bg_color, &color) == 0 ||
        XAllocColor(CDisplay, DefaultColormap (CDisplay, DefaultScreen (CDisplay)), &color) == 0)
/* Use white background in case of failure */
        bg_pixel = WhitePixel(CDisplay, DefaultScreen(CDisplay));
    else
        bg_pixel = color.pixel;

    CMain = XCreateSimpleWindow (CDisplay,
				   DefaultRootWindow (CDisplay),
			 p_XSH->x, p_XSH->y, p_XSH->width, p_XSH->height,
				   DEFAULT_BDWIDTH, C_BLACK, bg_pixel);

/* Set up class hint */
    if ((p_CH = XAllocClassHint ()) == NULL) {
	fprintf (stderr, "Error allocating class hint!\n");
	exit (1);
    }
    p_CH->res_name = CAppName;
    p_CH->res_class = CAppName;

/* Set up XTextProperty for window name and icon name */
    if (XStringListToTextProperty (&CAppName, 1, &WName) == 0) {
	fprintf (stderr, "Error creating XTextProperty!\n");
	exit (1);
    }
    if (XStringListToTextProperty (&CAppName, 1, &IName) == 0) {
	fprintf (stderr, "Error creating XTextProperty!\n");
	exit (1);
    }
    if ((p_XWMH = XAllocWMHints ()) == NULL) {
	fprintf (stderr, "Error allocating Window Manager hints!\n");
	exit (1);
    }
    p_XWMH->flags = (InputHint | StateHint);
    p_XWMH->input = True;
    p_XWMH->initial_state = NormalState;
    XSetWMProperties (CDisplay, CMain, &WName, &IName, argv, argc,
		      p_XSH, p_XWMH, p_CH);

/* Finally, create a graphics context for the main window */

    gcv.font = CFontStruct->fid;
    gcv.foreground = C_BLACK;
    gcv.background = bg_pixel;
    CGC = XCreateGC (CDisplay, CMain,
		       (GCFont | GCForeground | GCBackground), &gcv);

/* Set main window's attributes (colormap, bit_gravity) */

    xswa.colormap = DefaultColormap (CDisplay,
				     DefaultScreen (CDisplay));
    xswa.bit_gravity = NorthWestGravity;
    XChangeWindowAttributes (CDisplay, CMain, (CWColormap |
						   CWBitGravity), &xswa);

/* Select Exposure events for the main window */

    XSelectInput (CDisplay, CMain, ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask);

/* Map the main window */

    XMapWindow (CDisplay, CMain);
    XFlush (CDisplay);
    if (XGetWindowAttributes (CDisplay, CMain, &MainXWA) == 0) {
	fprintf (stderr, "Error getting attributes of Main.\n");
	exit (2);
    }
}


int CSendEvent (XEvent * e);
static XEvent xevent;


#define ALRM_PER_SECOND 50
static int cursor_blink_rate;

/*
   Aim1: Get the cursor to flash all the time:

   Aim2: Have coolwidgets send an alarm event, just like
		any other event. For the application to use.

   Requirements: XNextEvent must still be the blocker
		so that the process doesn't hog when idle.

   Problems: If the alarm handler sends an event using
		XSendEvent it may hang the program. It
		also does not necessarily cause
		XNextEvent to unblock.

   To solve, we put a pause() before XNextEvent so that it waits for 
   an alarm, and also define our own CSendEvent routine with
   its own queue. So that things don't slow down, we pause() only
   if no events are pending. Also make the alarm rate high (100 X per sec).
   (I hope this is the easiest way to do this  :|   )
*/

void CSetCursorBlinkRate(int b)
{
    cursor_blink_rate = b;
}

/* does nothing and calls nothing for t seconds, resolution is ALRM_PER_SECOND */
void CSleep (double t)
{
    float i;
    for (i = 0; i < t; i += 1.0 / ALRM_PER_SECOND)
	pause ();
}


static struct itimerval alarm_every =
{
    {
	0, 0
    },
    {
	0, 1000000 / ALRM_PER_SECOND
    }
};

static struct itimerval alarm_off =
{
    {
	0, 0
    },
    {
	0, 0
    }
};

static RETSIGTYPE alarmhandler (int x)
{
    static int count = ALRM_PER_SECOND;
    if (count) {
	count--;
	if (!CQueueSize ()) {
	    CSendEvent (&xevent);
	}
    } else {
	xevent.type = AlarmEvent;
	if (CQueueSize () < 512) {	/* say */
	    CSendEvent (&xevent);
	}
	xevent.type = TickEvent;
	count = ALRM_PER_SECOND / cursor_blink_rate;
    }
    signal (SIGALRM, alarmhandler);
    setitimer (ITIMER_REAL, &alarm_every, NULL);
#if (RETSIGTYPE==void)
    return;
#else
    return 1;			/* :guess --- I don't know what to return here */
#endif
}



static void set_alarm (void)
{
    memset (&xevent, 0, sizeof (XEvent));
    xevent.type = 0;
    xevent.xany.display = CDisplay;
    xevent.xany.send_event = 1;

    CSetCursorBlinkRate(7); /* theta rhythms ? */

    signal (SIGALRM, alarmhandler);
    setitimer (ITIMER_REAL, &alarm_every, NULL);
}

void CEnableAlarm()
{
    set_alarm();
}

void CDisableAlarm()
{
    setitimer (ITIMER_REAL, &alarm_off, NULL);
    signal (SIGALRM, 0);
}


static void draw_logo ()
{
    Cdrawbitmap ("c00ls0ftware", CMain, 10, 10,
		   100, 62, Ccolor (6), Ccolor (12), cool_bits);
    Cwidget ("c00ls0ftware")->position = CALWAYS_ON_BOTTOM;
}

void get_temp_dir (void)
{
    if (temp_dir)
	free (temp_dir);
    temp_dir = getenv ("TEMP");
    if (temp_dir)
	if(*temp_dir) {
	    temp_dir = strdup (temp_dir);
	    return;
	}
    temp_dir = getenv ("TMP");
    if (temp_dir)
	if(*temp_dir) {
	    temp_dir = strdup (temp_dir);
	    return;
	}
    temp_dir = strdup ("/tmp");
}

void get_home_dir ()
{
    if (home_dir)
	free (home_dir);
    home_dir = getenv ("HOME");
    if (home_dir)
	if (*home_dir) {
	    home_dir = strdup (home_dir);
	    return;
	}
    home_dir = strdup (current_dir);
}

void get_dir (void)
{
    if (!get_current_wd (current_dir, MAX_PATH_LEN))
	*current_dir = 0;
    get_temp_dir ();
    get_home_dir ();
}


/*-------------------------------------------------------------*/
void Cinit (CInitData *config_start)
{
    given = config_start;
    verbose_operation = (given->options & CINIT_OPTION_VERBOSE);

    CAppName = given->name;
    AppDone = 0;

    Cusinggreyscale = (given->options & CINIT_OPTION_USE_GREY);

/* Initialise the widget library */
    init_widgets ();

/* get home dir directory into home_dir and current directory into current_dir */
    get_dir ();

/* Get resources from the resource file */
    get_resources ();
    if (given->display)
	init_display = given->display;
    if (given->geometry)
	init_geometry = given->geometry;
    if (given->font)
	init_font = given->font;
    if (given->bg)
	init_bg_color = given->bg;
    if (given->fg_red)
	init_fg_color_red = given->fg_red;
    if (given->fg_green)
	init_fg_color_green = given->fg_green;
    if (given->fg_blue)
	init_fg_color_blue = given->fg_blue;

/*  Open connection to display selected by user */
    open_display (CAppName);

/*  Initialise drag and drop capabilities */
    CDndInit ();

/* Set up font */
    load_font ();

/* Now set up the visual and colors */

    Cdepth = DefaultDepth (CDisplay, DefaultScreen (CDisplay));

    Cvisual = DefaultVisual (CDisplay, DefaultScreen (CDisplay));

    if(verbose_operation) {
	printf ("Found default visual, depth = %d,\n       visual class = ", Cdepth);
	visual_comments (ClassOfVisual (Cvisual));
    }

/* Now setup that color map discribed above */
    setup_colormap (ClassOfVisual (Cvisual));

    hints (given->x, given->y, given->width_plus + TEXT_M_WIDTH * given->columns, given->height_plus + TEXT_PIX_PER_LINE * given->lines);

/* Create the main window using the position and size hints. */
    create_main_win (1, &(given->name));

    draw_logo ();

/* an alarm handler generates xevent of tyoe AlarmEvent every 1/4 second to flash the cursor */
    set_alarm ();
}
