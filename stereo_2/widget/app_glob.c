/* app_glob.c: Declare the common global variables for X applications.

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


#ifndef APP_GLOB_C		/* To ensure that it is included once */
#define APP_GLOB_C

#include "my_string.h"

#define max(x,y)     (((x) > (y)) ? (x) : (y))
#define min(x,y)     (((x) < (y)) ? (x) : (y))

#ifdef DEF_APP_GLOB		/* Defined in the initapp.c file */

XWMHints *p_XWMH;		/* Hints for the window manager  */
XSizeHints *p_XSH;		/* Size hints for window manager */
XClassHint *p_CH;		/* Class hint for window manager */
XTextProperty WName;		/* Window name for title bar     */
XTextProperty IName;		/* Icon name for icon label      */
Display *CDisplay = NULL;	/* Connection to X display     */
GC CGC = 0;			/* The graphics context for main */
int AppDone = 0;		/* Flag to indicate when done   */
XFontStruct *CFontStruct;	/* Info on the default font */
unsigned long Cpixel[256];	/*for pixel */
unsigned long Cplane[256];	/*and plane values from alloccolor. */
char *CAppName;			/* Application's name    */
Window CMain = 0;		/* Application's main window */
Visual *Cvisual;
int Cdepth;
int CXimageLSBFirst;
int Cusinggreyscale;
int Cfont_is_proportional;
int CMean_font_width;
char *home_dir = 0;
char *temp_dir = 0;
char current_dir[MAX_PATH_LEN + 1];
XWindowAttributes MainXWA;	/* Attributes of main window */

#include "bitmap/cross.bitmap"
#include "bitmap/tick.bitmap"
#include "bitmap/switchon.bitmap"
#include "bitmap/switchoff.bitmap"
#include "bitmap/exclam.bitmap"

#else

extern XWMHints *p_XWMH;
extern XSizeHints *p_XSH;
extern XClassHint *p_CH;
extern XTextProperty WName;
extern XTextProperty IName;
extern Display *CDisplay;
extern GC CGC;
extern int AppDone;
extern XFontStruct *CFontStruct;
extern unsigned long Cpixel[256];	/*for pixel */
extern unsigned long Cplane[256];	/*and plane values from alloccolor. */
extern char *CAppName;
extern Window CMain;
extern Visual *Cvisual;
extern int Cdepth;
extern int CXimageLSBFirst;
extern int Cusinggreyscale;
extern int Cfont_is_proportional;
extern int CMean_font_width;
extern char *home_dir;
extern char *temp_dir;
extern char current_dir[MAX_PATH_LEN + 1];
extern XWindowAttributes MainXWA;

extern unsigned char cross_bits[];
extern unsigned char tick_bits[];
extern unsigned char switchon_bits[];
extern unsigned char switchoff_bits[];
extern unsigned char exclam_bits[];

#endif				/* #ifdef DEF_APP_GLOB */
#endif				/* #ifndef APP_GLOB_C  */
