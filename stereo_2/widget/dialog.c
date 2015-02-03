/* dialog.c - draws various useful dialog boxes
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/errno.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "dirtools.h"

#include "coolwidget.h"
#include "coollocal.h"
#include "loadfile.h"

#include "mad.h"
#include "dialog.h"
#include "edit.h"
#include "editcmddef.h"

#define MID_X 20
#define MID_Y 20


/* Yuriy Elkin: (from mc/src/util.c) */
char *get_sys_error (const char *s)
{
    char *error_msg;
    if (errno) {
#ifdef HAVE_STRERROR
	error_msg = strerror (errno);
#else
	extern int sys_nerr;
	extern char *sys_errlist[];
	if ((0 <= errno) && (errno < sys_nerr))
	    error_msg = sys_errlist[errno];
	else
	    error_msg = "strange errno";
#endif
	return catstrs (s, "\n [", error_msg, "] ", 0);
	errno = 0;
    } else {
	return (char *) s;
    }
}

void Cerrordialog (Window in, int x, int y, const char *heading, const char *fmt,...)
{
    va_list pa;
    char *str;
    Window win;
    CEvent cwevent;
    CState s;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    if (!in) {
	in = CMain;
	x = MID_X;
	y = MID_Y;
    }
    CBackupState (&s);
    CDisable ("*");
    win = Cdrawheadedwindow ("_error", in, x, y, 10, 10, heading);
    Cgethintpos (&x, &y);
    Cdrawtext ("", win, x, y, str);
    free (str);
    Cgethintpos (0, &y);
    (Cdrawbitmapbutton ("_clickhere", win, -50, y, 40, 40, Ccolor (22), C_FLAT, exclam_bits))->options = TEXT_CENTRED;
    Cwidget ("_error")->position = CFIXED_POSITION | CALWAYS_ON_TOP;
    Csetsizehintpos ("_error");
    CFocus (Cwidget ("_clickhere"));
    do {
	CNextEvent (NULL, &cwevent);
    } while (strcmp (cwevent.ident, "_clickhere") && cwevent.key != XK_Escape);
    Cundrawwidget ("_error");
    CRestoreState (&s);
}

void Cmessagedialog (Window in, int x, int y, const char *heading, const char *fmt,...)
{
    va_list pa;
    char *str;
    Window win;
    CEvent cwevent;
    CState s;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    if (!in)
	in = CMain;

    CBackupState (&s);
    CDisable ("*");
    win = Cdrawheadedwindow ("_error", in, x, y, 10, 10, heading);
    Cgethintpos (&x, &y);
    Cdrawtext ("", win, x, y, str);
    free (str);
    Cgethintpos (0, &y);
    (Cdrawbitmapbutton ("_clickhere", win, -50, y, 40, 40, Ccolor (6), C_FLAT, tick_bits))->options = TEXT_CENTRED;
    Cwidget ("_error")->position = CFIXED_POSITION | CALWAYS_ON_TOP;
    Csetsizehintpos ("_error");
    CFocus (Cwidget ("_clickhere"));
    do {
	CNextEvent (NULL, &cwevent);
    } while (strcmp (cwevent.ident, "_clickhere") && cwevent.key != XK_Escape);
    Cundrawwidget ("_error");
    CRestoreState (&s);
}

/* draws a scrollable text box with a button to clear. Can be used to give long help messages */
void Ctextboxmessagedialog (Window in, int x, int y, int columns, int lines, const char *heading, const char *text, int line)
{
    Window win;
    CEvent cwevent;
    CState s;
    int width, height;

    width = columns * TEXT_M_WIDTH + 1 + 6;
    height = lines * TEXT_PIX_PER_LINE + 1 + 6;

    if (!in) {
	in = CMain;
	x = 20;
	y = 20;
    }
    CBackupState (&s);
    CDisable ("*");
    win = Cdrawheadedwindow ("_error", in, x, y, 10, 10, heading);
    Cgethintpos (&x, &y);
    Cdrawtextbox ("_textmessbox", win, x, y, width, height, line, 0, text, 0);
    Cgethintpos (0, &y);
    (Cdrawbitmapbutton ("_clickhere", win, -50, y, 40, 40, Ccolor (6), C_FLAT, tick_bits))->options = TEXT_CENTRED;
    Cwidget ("_error")->position = CFIXED_POSITION | CALWAYS_ON_TOP;
    Csetsizehintpos ("_error");
    CFocus (Cwidget ("_clickhere"));
    do {
	CNextEvent (NULL, &cwevent);
    } while (strcmp (cwevent.ident, "_clickhere") && cwevent.key != XK_Escape);
    Cundrawwidget ("_error");
    CRestoreState (&s);
}

void Cfatalerrordialog (int x, int y, const char *fmt,...)
{
    va_list pa;
    char *str;
    Window win;
    CEvent cwevent;
    CState s;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    fprintf (stderr, "%s\n", str);

    if (CDisplay) {
	CBackupState (&s);
	CDisable ("*");
	win = Cdrawheadedwindow ("fatalerror", CMain, x, y,
				 10, 10, " Fatal Error ");
	Cgethintpos (&x, &y);
	Cdrawtext ("", win, x, y, str);
	Cgethintpos (0, &y);
	(Cdrawbitmapbutton ("clickhere", win, -50, y, 40, 40, Ccolor (19), C_FLAT, cross_bits))->options = TEXT_CENTRED;
	Cwidget ("fatalerror")->position = CFIXED_POSITION | CALWAYS_ON_TOP;
	Csetsizehintpos ("fatalerror");
	CFocus (Cwidget ("clickhere"));
	do {
	    CNextEvent (NULL, &cwevent);
	} while (strcmp (cwevent.ident, "clickhere"));
    }
    abort ();
}



/* returns a raw XK_key sym. See 'Note well' below. */
short Crawkeyquery (Window in, int x, int y, const char *heading, const char *fmt,...)
{
    va_list pa;
    char *str;
    int p = 0;
    Window win;
    CEvent cwevent;
    XEvent xevent;
    CState s;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    if (!in) {
	in = CMain;
	x = MID_X;
	y = MID_Y;
    }
    CBackupState (&s);
    CDisable ("*");
    win = Cdrawheadedwindow ("_inputdialog", in, x, y, 10, 10, heading);
    Cgethintpos (&x, &y);
    Cdrawtext ("", win, x, y, str);
    Cgethintpos (&x, 0);
    free (str);
    Cdrawtextinput ("_inputdialog.input", win, x, y, (TEXT_M_WIDTH) * 3, AUTO_HEIGHT, 256, "");
    Cgethintpos (0, &y);
    (Cdrawbitmapbutton ("_inputdialog.crosshere", win, -50, y, 40, 40, Ccolor (18), C_FLAT, cross_bits))->options = TEXT_CENTRED;
    Csetsizehintpos ("_inputdialog");
    CFocus (Cwidget ("_inputdialog.input"));
    Cwidget ("_inputdialog")->position = CALWAYS_ON_TOP;
/* handler : */
    do {
	CNextEvent (&xevent, &cwevent);
	if (cwevent.key == XK_Escape || !strcmp (cwevent.ident, "_inputdialog.crosshere"))
	    break;
	if (xevent.type == KeyPress)
	    p = CKeySymMod (&xevent);
    } while (!p);

    Cundrawwidget ("_inputdialog");
    CRestoreState (&s);
    return p;
}

/*
   def is the default string in the textinput widget.
   Result must be free'd. Returns 0 on cancel.
 */
char *Cinputdialog (Window in, int x, int y, int min_width, const char *def, const char *heading, const char *fmt,...)
{
    va_list pa;
    char *str, *p = 0;
    int w, h;
    Window win;
    CEvent cwevent;
    CState s;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    if (!in) {
	in = CMain;
	x = MID_X;
	y = MID_Y;
    }
    Ctextsize (&w, &h, str);
    w = max (max (w, min_width), 130);
    CBackupState (&s);
    CDisable ("*");
    win = Cdrawheadedwindow ("_inputdialog", in, x, y, 10, 10, heading);
    Cgethintpos (&x, &y);
    Cdrawtext ("", win, x, y, str);
    Cgethintpos (0, &y);
    free (str);
    Cdrawtextinput ("_inputdialog.input", win, x, y, w, AUTO_HEIGHT, 256, def);
    Cgethintpos (0, &y);
    Cdrawbitmapbutton ("_inputdialog.clickhere", win, (w + 16) / 4 - 22, y, 40, 40, Ccolor (6), C_FLAT, tick_bits);
    Cdrawbitmapbutton ("_inputdialog.crosshere", win, 3 * (w + 16) / 4 - 22, y, 40, 40, Ccolor (18), C_FLAT, cross_bits);
    Csetsizehintpos ("_inputdialog");
    CFocus (Cwidget ("_inputdialog.input"));
    Cwidget ("_inputdialog")->position = CALWAYS_ON_TOP;
/* handler : */
    do {
	CNextEvent (NULL, &cwevent);
	if (cwevent.key == XK_Escape || !strcmp (cwevent.ident, "_inputdialog.crosshere"))
	    goto fin;
	if (cwevent.key == XK_Return)
	    break;
    } while (strcmp (cwevent.ident, "_inputdialog.clickhere"));
    p = strdup (Cwidget ("_inputdialog.input")->text);

  fin:
    Cundrawwidget ("_inputdialog");
    CRestoreState (&s);
    return p;
}

static char *id[32] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void free_last_query_buttons (void)
{
    int i;
    for (i = 0; i < 32; i++)
	if (id[i]) {
	    free (id[i]);
	    id[i] = 0;
	}
}

int Cquerydialog (Window in, int x, int y, const char *heading, const char *first,...)
{
    va_list pa;
    int i, buttons = 0;
    Window win;
    CEvent cwevent;
    CState s;
    char *b[32];

    free_last_query_buttons ();
    va_start (pa, first);
    while ((b[buttons] = va_arg (pa, char *)))
	buttons++;
    va_end (pa);
    if (!buttons)
	return -1;

    if (!in) {
	in = CMain;
	x = MID_X;
	y = MID_Y;
    }
    CBackupState (&s);
    CDisable ("*");
    win = Cdrawheadedwindow ("_querydialog", in, x, y, 10, 10, heading);
    Cgethintpos (&x, &y);
    (Cdrawtext ("_querydialog.text", win, x, y, first))->options = TEXT_CENTRED;
    Cgethintpos (0, &y);

    for (i = 0; i < buttons; i++) {
	Cdrawbutton (id[i] = sprintf_alloc ("_query.%.20s", b[i]),
		     win, x, y, AUTO_WIDTH, AUTO_HEIGHT, b[i]);
	Cgethintpos (&x, 0);
    }

    Csetsizehintpos ("_querydialog");
    CFocus (Cwidget (catstrs ("_query.", b[0], 0)));
    Cwidget ("_querydialog")->position = CALWAYS_ON_TOP;
    for (;;) {
	CNextEvent (NULL, &cwevent);
	for (i = 0; i < buttons; i++) {
	    if (!strcmp (cwevent.ident, id[i])) {
		Cundrawwidget ("_querydialog");
		CRestoreState (&s);
		return i;
	    }
	}
    }
    CRestoreState (&s);
}
