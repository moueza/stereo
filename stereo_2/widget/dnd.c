/* dnd.c - drag and drop support

   Copyright (C) 1996 the Free Software Foundation

   Authors: 1996 Paul Sheer

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

/* 
   The cursor bitmaps and the cursor initialisation routines are from
   the DND package which can be found with the OffiX package at
   sunsite.unc.edu. These are Copyright (C) 1996 César Crusius.
   I also took a look at his code to get a better idea how get my Dnd
   working the same, and small parts of the code are modifications
   from there.
 */

#include <config.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include "cursor/file.xbm"
#include "cursor/file_mask.xbm"
#include "cursor/files.xbm"
#include "cursor/files_mask.xbm"
#include "cursor/dir.xbm"
#include "cursor/dir_mask.xbm"
#include "cursor/text.xbm"
#include "cursor/text_mask.xbm"
#include "cursor/grey.xbm"
#include "cursor/grey_mask.xbm"
#include "cursor/link.xbm"
#include "cursor/link_mask.xbm"
#include "cursor/app.xbm"
#include "cursor/app_mask.xbm"
#include "cursor/url.xbm"
#include "cursor/url_mask.xbm"
#include "cursor/mime.xbm"
#include "cursor/mime_mask.xbm"

#include "app_glob.c"
#include "coolwidget.h"
#include "mad.h"

/* #define DND_DEBUG */

Atom DndProtocol, DndSelection, DndAcknowledge;

typedef struct
{	
    int	Width,Height;
    char	*ImageData,*MaskData;
    int	HotSpotX,HotSpotY;
    Pixmap	ImagePixmap,MaskPixmap;
    Cursor	CursorID;
} CursorData;

static CursorData DndCursor[DndEND]={
  { 0,0,NULL,NULL,0,0,0 },
  { grey_width,	grey_height,grey_bits,grey_mask_bits,
    grey_x_hot,grey_y_hot},
  { file_width,file_height,file_bits,file_mask_bits,
    file_x_hot,file_y_hot},
  { files_width,files_height,files_bits,files_mask_bits,
    files_x_hot,files_y_hot},
  { text_width,text_height,text_bits,text_mask_bits,
    text_x_hot,text_y_hot },
  { dir_width,dir_height,dir_bits,dir_mask_bits,
    dir_x_hot,dir_y_hot },
  { link_width,link_height,link_bits,link_mask_bits,
    link_x_hot,link_y_hot},
  { app_width,app_height,app_bits,app_mask_bits,
    app_x_hot,app_y_hot },
  { url_width,url_height,url_bits,url_mask_bits,
    url_x_hot,url_y_hot },
  { mime_width,mime_height,mime_bits,mime_mask_bits,
    mime_x_hot,mime_y_hot }  
};

static char dnd_directory[MAX_PATH_LEN] = "";

/* return the prepending directory (see CDndFileList() below) */
char *CDndDirectory (void)
{
    return dnd_directory;
}

char *striptrailing (char *s, int c)
{
    int i;
    i = strlen (s) - 1;

    while (i >= 0) {
	if (s[i] == c) {
	    s[i--] = 0;
	    continue;
	}
	break;
    }
    return s;
}

/*
   Sets the directory, must be a null terminated complete path.
   Strips trailing slashes.
 */
void CSetDndDirectory (char *d)
{
    if (!d)
	return;
    strcpy (dnd_directory, d);
    striptrailing (dnd_directory, '/');
    if (*dnd_directory)
	return;
    *dnd_directory = '/';
}

/*
   Takes a newline seperated list of files,
   returns a null seperated list of complete path names
   by prepending dnd_directory to each file name.
   returns 0 if no files in list.
   result must always be free'd.
   returns l as the total length of data.
   returns num_files as the number of files in the list.
   Alters t
 */
char *CDndFileList (char *t, int *l, int *num_files)
{
    char *p, *q, *r, *result;
    int i;
    int path_len, len;

/* strip leading newlines */
    while (*t == '\n')
	t++;

/* strip trailing newlines */
    striptrailing (t, '\n');

    if (!*t)
	return 0;

/* count files */
    for (i = 1, p = t; *p; p++)
	if (*p == '\n')
	    i++;

    *num_files = i;

    len = (unsigned long) p - (unsigned long) t;
    path_len = strlen (dnd_directory);
    result = Cmalloc ((path_len + 2) * i + len + 2);

    r = result;
    p = t;
    for (;;) {
	q = strchr (p, '\n');
	if (!q)
	    q = t + len;
	*q = 0;
	strcpy (r, dnd_directory);
	r += path_len;
	*r++ = '/';
	strcpy (r, p);
	r += (unsigned long) q - (unsigned long) p;
	*r++ = 0;
	if ((unsigned long) q == (unsigned long) t + len)
	    break;
	p = ++q;
    }
    *r = 0;
    *l = (unsigned long) r - (unsigned long) result;
    return result;
}


/* Dnd initialisation */
void CDndInit (void)
{
    int screen, i;
    Colormap colormap;
    Window root;
    XColor Black,White;

    DndAcknowledge = XInternAtom (CDisplay, "_DND_ACKNOWLEDGE", False);
    DndProtocol = XInternAtom (CDisplay, "_DND_PROTOCOL", False);
    DndSelection = XInternAtom (CDisplay, "_DND_SELECTION", False);

    screen = DefaultScreen (CDisplay);
    colormap = DefaultColormap (CDisplay, screen);
    root = DefaultRootWindow (CDisplay);

    Black.pixel = BlackPixel (CDisplay, screen);
    White.pixel = WhitePixel (CDisplay, screen);
    XQueryColor (CDisplay, colormap, &Black);
    XQueryColor (CDisplay, colormap, &White);

    for (i = 1; i != DndEND; i++) {
	DndCursor[i].ImagePixmap =
	    XCreateBitmapFromData (CDisplay, root,
				   DndCursor[i].ImageData,
				   DndCursor[i].Width,
				   DndCursor[i].Height);
	DndCursor[i].MaskPixmap =
	    XCreateBitmapFromData (CDisplay, root,
				   DndCursor[i].MaskData,
				   DndCursor[i].Width,
				   DndCursor[i].Height);
	DndCursor[i].CursorID =
	    XCreatePixmapCursor (CDisplay, DndCursor[i].ImagePixmap,
				 DndCursor[i].MaskPixmap,
				 &Black, &White,
				 DndCursor[i].HotSpotX,
				 DndCursor[i].HotSpotY);
    }
    DndCursor[0].CursorID = XCreateFontCursor (CDisplay, XC_question_arrow);
}

/*
   This does a drag.
   This is copied somewhat from the dnd Xt code by César Crusius 
   so it should behave almost exactly the way his does.
   From is the sending window. data_type is a dnd type in coolwidget.h,
   data is the actual data, and length is its length.
   pointer state is the state of the pointer.
   I set the pointer state to that *during* the drag, not before.
   See eh_editor in editwidget.c for details.
 */
void CDrag (Window from, int data_type, unsigned char *data, int length, unsigned long pointer_state)
{
    XEvent e;
    long x, y;
    Window root;
    Window target, dispatch;

#ifdef DND_DEBUG
    printf ("Drag start\n");
#endif

    root = DefaultRootWindow (CDisplay);

    XChangeProperty (CDisplay, root, DndSelection, XA_STRING, 8,
		     PropModeReplace, data, length);

    XGrabPointer (CDisplay, root, False,
		  ButtonMotionMask | ButtonPressMask | ButtonReleaseMask,
		  GrabModeSync, GrabModeAsync, root,
		  DndCursor[data_type].CursorID, CurrentTime);

    do {
	XAllowEvents (CDisplay, SyncPointer, CurrentTime);
	XNextEvent (CDisplay, &e);
    } while (e.type != ButtonRelease);

    XUngrabPointer (CDisplay, CurrentTime);

    if (!e.xbutton.subwindow) {
#ifdef DND_DEBUG
	printf ("Pointer on root window, Drag end\n");
#endif
	return;
    }
    target = my_XmuClientWindow (CDisplay, e.xbutton.subwindow);
    if (target == e.xbutton.subwindow)
	dispatch = target;
    else
	dispatch = PointerWindow;

    x = e.xbutton.x_root;
    y = e.xbutton.y_root;

    e.xclient.type = ClientMessage;
    e.xclient.display = CDisplay;
    e.xclient.message_type = DndProtocol;
    e.xclient.format = 32;
    e.xclient.window = target;
    e.xclient.data.l[0] = data_type;
    e.xclient.data.l[1] = pointer_state;
    e.xclient.data.l[2] = from;
    e.xclient.data.l[3] = x + y * 65536L;
    e.xclient.data.l[4] = 1;

    /* Send the drop message */
    XSendEvent (CDisplay, dispatch, True, NoEventMask, &e);

#ifdef DND_DEBUG
    printf ("Drop send to window %ld from %ld, Drag end\n", target, from);
#endif
}


/*
   Returns data_type on success, DndNotDnd if not a Dnd drop.
   Unneeded values can be passed as null.
 */
int CGetDrop (XEvent * xe, unsigned char **data, unsigned long *size, int *x, int *y)
{
    Atom ActualType;
    int format;
    unsigned long dsize;
    unsigned char *ddata;
    unsigned long remaining;

    if (!data)
	data = &ddata;
    if (!size)
	size = &dsize;

    if (xe->type != ClientMessage) {
#ifdef DND_DEBUG
	printf ("None ClientMessage event to window %ld\n", xe->xany.window);
#endif
	return DndNotDnd;
    }
#ifdef DND_DEBUG
    printf ("ClientMessage event to window %ld\n", xe->xany.window);
#endif
    if (xe->xclient.message_type != DndProtocol || xe->xclient.data.l[4] != 1)
	return DndNotDnd;

#ifdef DND_DEBUG
    printf ("Drop recieved at window %ld from window %ld\n", xe->xclient.window, xe->xclient.data.l[2]);
#endif

    XGetWindowProperty (CDisplay, DefaultRootWindow (CDisplay), DndSelection,
			0L, 1000000L,
			False, AnyPropertyType,
			&ActualType, &format,
			size, &remaining,
			data);

    if (x)
	*x = xe->xclient.data.l[3] & 0xFFFF;
    if (y)
	*y = xe->xclient.data.l[3] >> 16;

    return xe->xclient.data.l[0];
}

/* 
   Sends an acknowledge event that the drop was recieved. This is my
   extension to the protocol, but cooledit doesn't depend on
   an acknowledge event being recieved, so everything will
   work as usual when communicating with apps that don't 
   support this. xe is the drop event that you recived and
   is not be modified.
 */
void CDropAcknowledge (XEvent * xe)
{
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.display = CDisplay;
    e.xclient.message_type = DndAcknowledge;
    e.xclient.format = 32;
    e.xclient.window = xe->xclient.data.l[2];	/* to window that send the drop */
    e.xclient.data.l[0] = xe->xclient.data.l[0];	/* data type */
    e.xclient.data.l[1] = xe->xclient.data.l[1];	/* pointer state of the pointer when the drop was send */
    e.xclient.data.l[2] = xe->xclient.window;	/* from this window */
    e.xclient.data.l[3] = 0;	/* not used */
    e.xclient.data.l[4] = 1;	/* same dnd version, since this extension can be 
				   ignored, there is no need to have a new version */

    XSendEvent (CDisplay, e.xclient.window, True, NoEventMask, &e);

#ifdef DND_DEBUG
    printf ("acknowledge send to window %ld from %ld, return\n", xe->xclient.data.l[2], xe->xclient.window);
#endif
}

/*
   Checks if xe is an acknowledge event.
   Returns data_type if so and the state that the pointer was in
   when the drag was initiated.
   Returns DndNotDnd if not an acknowledge event.
 */
int CIsDropAcknowledge (XEvent * xe, unsigned int *state)
{
    if (xe->type != ClientMessage)
	return DndNotDnd;
    if (xe->xclient.message_type != DndAcknowledge || xe->xclient.data.l[4] != 1)
	return DndNotDnd;
    if (state)
	*state = xe->xclient.data.l[1];
    return xe->xclient.data.l[0];
}
