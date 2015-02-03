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
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/errno.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "dirtools.h"

#include "coolwidget.h"
#include "dialog.h"
#include "editcmddef.h"

#include "mad.h"

Window Cdrawfilebrowser (const char *identifier, Window parent, int x, int y,
		    const char *dir, const char *file, const char *label)
{
    char *filelist = 0;
    char *directorylist = 0;
    int y2, x2, x3, y3;
    Window win;

    win = Cdrawheadedwindow (identifier, parent, x, y, 10, 10, label);
    (Cwidget (identifier))->options |= CALWAYS_ON_TOP;
    if ((filelist = getfilelist (dir, 'f')) == NULL ||
	(directorylist = getfilelist (dir, '/')) == NULL) {
	Cerrordialog (parent, 20, 20, " File browser ", " Unable to read directory ");
	Cundrawwidget (identifier);
    }
    Cgethintpos (&x, &y);
    Cdrawtext (catstrs (identifier, ".dir", 0), win, x, y, dir);
    Cgethintpos (0, &y);
    y3 = y;
    Cdrawtextbox (catstrs (identifier, ".fbox", 0), win, x, y,
		  TEXT_M_WIDTH * 17 + 7, TEXT_PIX_PER_LINE * 14 + 6, 0, 0, filelist, TEXT_FILES);
    Cgethintpos (&x2, &y2);
    x3 = x2;
    Cdrawtextbox (catstrs (identifier, ".dbox", 0), win, x2, y + 40 + 8 + WIDGET_SPACING,
		  TEXT_M_WIDTH * 17 + 7, y2 - WIDGET_SPACING * 2 - y - 40 - 8, 0, 0, directorylist, TEXT_FILES);
    Cgethintpos (&x2, &y2);
    Cdrawtextinput (catstrs (identifier, ".finp", 0), win, x, y2,
		    x2 - WIDGET_SPACING * 2 - 2, AUTO_HEIGHT, 256, file);
    Cdrawbitmapbutton (catstrs (identifier, ".ok", 0), win, x3, y3,
		       40, 40, Ccolor (6), C_FLAT, tick_bits);
    Cdrawbitmapbutton (catstrs (identifier, ".cancel", 0), win, x2 - WIDGET_SPACING * 2 - 40 - 8 - 20, y3,
		       40, 40, Ccolor (18), C_FLAT, cross_bits);
    Csetsizehintpos (identifier);
    if (directorylist)
	free (directorylist);
    if (filelist)
	free (filelist);
    return win;
}

/* options */
#define GETFILE_GET_DIRECTORY		1
#define GETFILE_GET_EXISTING_FILE	2
#define GETFILE_BROWSER			4

/*
   Returns "" on no file entered and NULL on exit (i.e. Cancel button pushed)
   else returns the file or directory. Result must be immediately copied.
   Result must not be free'd.
 */
char *Chandlebrowser (const char *identifier, CEvent * cwevent, int options)
{
    struct stat st;
    char *q;
    char *idd = catstrs (identifier, ".dbox", 0);
    char *idf = catstrs (identifier, ".fbox", 0);
    static char estr[MAX_PATH_LEN];
    CWidget *input = Cwidget (catstrs (identifier, ".finp", 0));
    CWidget *directory = Cwidget (catstrs (identifier, ".dir", 0));
    CWidget *filelist = Cwidget (idf);
    CWidget *directorylist = Cwidget (idd);
    CWidget *textinput = Cwidget (catstrs (identifier, ".finp", 0));

    CSetDndDirectory (directory->text);
    if (!strcmp (cwevent->ident, idf) && !(options & (GETFILE_GET_DIRECTORY | GETFILE_BROWSER)) && (cwevent->button == Button1 || cwevent->key == XK_Return)) {
	q = strline (filelist->text, strmovelines (filelist->text, 0, filelist->cursor, 32000));
	Cdrawtextinput (textinput->ident, Cwidget (identifier)->winid, textinput->x, textinput->y,
			textinput->width, textinput->height, 256, q);
    }
    if (!strcmp (cwevent->ident, idd) && (cwevent->button == Button1 || cwevent->key == XK_Return)) {
	q = strline (directorylist->text, strmovelines (directorylist->text, 0, directorylist->cursor, 32000));
	if (*q != ' ')
	    Cdrawtextinput (catstrs (identifier, ".finp", 0), Cwidget (identifier)->winid, textinput->x, textinput->y,
			textinput->width, textinput->height, 256, q + 1);
    }
    if (!strcmp (cwevent->ident, input->ident)) {
	switch (cwevent->command) {
	case CK_Down:
	    CFocus (filelist);
	    Csettextboxpos (filelist, TEXT_SET_CURSOR_LINE, 0);
	    break;
	case CK_Up:
	    CFocus (filelist);
	    Csettextboxpos (filelist, TEXT_SET_CURSOR_LINE, 999999);
	    break;
	case CK_Page_Down:
	    CFocus (filelist);
	    Csettextboxpos (filelist, TEXT_SET_CURSOR_LINE, filelist->height / TEXT_PIX_PER_LINE - 1);
	    break;
	case CK_Page_Up:
	    CFocus (filelist);
	    Csettextboxpos (filelist, TEXT_SET_CURSOR_LINE, filelist->numlines - filelist->height / TEXT_PIX_PER_LINE + 1);
	    break;
	}
    }
    if (cwevent->key == XK_Escape || !strcmp (cwevent->ident, catstrs (identifier, ".cancel", 0)))
	return 0;

    if (options & GETFILE_GET_DIRECTORY) {
	if (!strcmp (cwevent->ident, catstrs (identifier, ".ok", 0))) {
	    strcpy (estr, path_compress (directory->text, ""));
	    return estr;
	}
    }
    if (!strcmp (cwevent->ident, catstrs (identifier, ".ok", 0))
	|| cwevent->key == XK_Return
	|| (cwevent->double_click && !strcmp (cwevent->ident, catstrs (identifier, ".finp", 0)))
	|| (cwevent->double_click && !strcmp (cwevent->ident, catstrs (identifier, ".dbox", 0)))
	|| (cwevent->double_click && !strcmp (cwevent->ident, catstrs (identifier, ".fbox", 0)))) {
	input->keypressed = 0;
	strcpy (estr, path_compress (directory->text, input->text));
	q = estr + strlen (estr) - 1;
	if (!estr[0])
	    return "";
	if (stat (estr, &st)) {
	    char *be = "";
	    switch (errno) {
	    case EACCES:
		be = " Search\n permission\n denied for\n this path.\n";
		break;
	    case ENOENT:
/* The user wanted a directory, but typed in one that doesn't exist */
		if (*q != '/' && !(options & GETFILE_GET_EXISTING_FILE) && !(options & (GETFILE_GET_DIRECTORY | GETFILE_BROWSER)))
		    return estr;	/* user wants a new file */
		be = " No such\n file/directory.\n";
		break;
	    case ENOTDIR:
		be = " No such\n directory.\n";
		break;
	    default:
		be = " Could not get\n info on this\n file/directory.\n";
		break;
	    }
	    Credrawtextbox (catstrs (identifier, ".fbox", 0), "\n", 0);
	    Credrawtextbox (catstrs (identifier, ".dbox", 0), be, 0);
	    return "";
	}
	if (S_ISDIR (st.st_mode)) {
	    char *f = getfilelist (estr, 'f');
	    if (strncmp (f, "Error: ", 7)) {
		Credrawtextbox (catstrs (identifier, ".fbox", 0), f, 0);
		if (f)
		    free (f);
		Credrawtextbox (catstrs (identifier, ".dbox", 0), f = getfilelist (estr, '/'), 0);
		if (*q == '/')
		    *q = 0;
		Credrawtext (catstrs (identifier, ".dir", 0), estr);
	    }
	    if (f)
		free (f);
	    return "";
	} else {
	    if (options & (GETFILE_GET_DIRECTORY | GETFILE_BROWSER)) {
		Credrawtextbox (catstrs (identifier, ".fbox", 0), "\n", 0);
		Credrawtextbox (catstrs (identifier, ".dbox", 0), " No such\n directory.\n", 0);
		return "";
	    }
	    return estr;	/* entry exists and is a file */
	}
    }
    return "";
}


/* check getdirectory's mem allocation*********** */

/* result must be free'd */
char *get_file_or_dir (Window parent, int x, int y,
       const char *dir, const char *file, const char *label, int options)
{
    CEvent cwevent;
    XEvent xevent;
    CState s;

    CBackupState (&s);
    CDisable ("*");
    CEnable ("_cfileBr*");

    if (!(parent | x | y)) {
	parent = CMain;
	x = 20;
	y = 20;
    }
    Cdrawfilebrowser ("Cgetfile", parent, x, y, dir, file, label);

    CFocus (Cwidget ("Cgetfile.finp"));

    do {
	CNextEvent (&xevent, &cwevent);
	if (xevent.type == Expose || !xevent.type || xevent.type == AlarmEvent
	  || xevent.type == InternalExpose || xevent.type == TickEvent) {
	    file = "";
	    continue;
	}
	file = Chandlebrowser ("Cgetfile", &cwevent, options);
	if (!file)
	    break;
    } while (!(*file));

    Cundrawwidget ("Cgetfile");

    CRestoreState (&s);

    if (file)
	return strdup (file);
    else
	return 0;
}

int cb_browser (CWidget *w, XEvent *x, CEvent *c)
{
    char id[32], *s;
    strcpy (id, w->ident);
    s = strchr (id, '.');
    if (s)
	*s = 0;
    if(!Chandlebrowser (id, c, GETFILE_BROWSER))
	Cundrawwidget (id);
    return 0;
}

void Cdrawbrowser (const char *ident, Window parent, int x, int y,
		   const char *dir, const char *file, const char *label)
{
    if (!(parent | x | y)) {
	parent = CMain;
	x = 20;
	y = 20;
    }

    Cdrawfilebrowser (ident, parent, x, y, dir, file, label);

    Caddcallback (catstrs (ident, ".dbox", 0), cb_browser);
    Caddcallback (catstrs (ident, ".fbox", 0), cb_browser);
    Caddcallback (catstrs (ident, ".finp", 0), cb_browser);
    Caddcallback (catstrs (ident, ".ok", 0), cb_browser);
    Caddcallback (catstrs (ident, ".cancel", 0), cb_browser);

    CFocus (Cwidget (catstrs (ident, ".finp", 0)));
}

char *Cgetfile (Window parent, int x, int y,
		const char *dir, const char *file, const char *label)
{
    return get_file_or_dir (parent, x, y, dir, file, label, 0);
}

char *Cgetdirectory (Window parent, int x, int y,
		     const char *dir, const char *file, const char *label)
{
    return get_file_or_dir (parent, x, y, dir, file, label, GETFILE_GET_DIRECTORY);
}

char *Cgetsavefile (Window parent, int x, int y,
		    const char *dir, const char *file, const char *label)
{
    return get_file_or_dir (parent, x, y, dir, file, label, 0);
}

char *Cgetloadfile (Window parent, int x, int y,
		    const char *dir, const char *file, const char *label)
{
    return get_file_or_dir (parent, x, y, dir, file, label, GETFILE_GET_EXISTING_FILE);
}


