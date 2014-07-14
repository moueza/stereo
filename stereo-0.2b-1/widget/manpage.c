/* manpage.c - draws an interactive man page browser
   Copyright (C) 1997 Paul Sheer

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


/* must be a power of 2 */
#define NUM_HISTORY 16
static char *history[NUM_HISTORY] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static unsigned int history_current = 0;

#define r_is_printable(x) ((x >= ' ' && x <= '~') || x >= 160)
#define my_is_letter(ch) ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '\b' || ch == '_' || ch == '-' || ch >= 160)

void check_prev_next (void)
{
    if (history[history_current & (NUM_HISTORY - 1)])
	Cwidget ("mandisplayfile.next")->disabled = 0;
    else
	Cwidget ("mandisplayfile.next")->disabled = 1;
    if (history[(history_current - 2) & (NUM_HISTORY - 1)])
	Cwidget ("mandisplayfile.prev")->disabled = 0;
    else
	Cwidget ("mandisplayfile.prev")->disabled = 1;
}

void add_to_history (char *t)
{
    char **h;
    history_current &= (NUM_HISTORY - 1);
    if (history[history_current])
	free (history[history_current]);
    h = &(history[(history_current + 1) & (NUM_HISTORY - 1)]);
    if (*h) {
	free (*h);
	*h = 0;
    }
    history[history_current++] = strdup (t);
}

int mansearch_callback (CWidget * w, XEvent * x, CEvent * c);
int mansearchagain_callback (CWidget * w, XEvent * x, CEvent * c);

int manpageclear_callback (CWidget * w, XEvent * x, CEvent * c)
{
    int i;
    Cundrawwidget ("mandisplayfile");
    for (i = 0; i < NUM_HISTORY; i++) {
	if (history[i]) {
	    free (history[i]);
	    history[i] = 0;
	}
    }
    history_current = 0;
    return 0;
}

int manpage_callback (CWidget * w, XEvent * x, CEvent * c)
{
    int p, q;
    unsigned char m[128] = "";
    char *t;
    if (c->key == XK_Escape || c->kind == CBITMAPBUTTON_WIDGET) {
	Cundrawwidget ("mandisplayfile");
	return 0;
    }
    if (c->command == CK_Find) {
	mansearch_callback (w, x, c);
    }
    if (c->command == CK_Find_Again) {
	mansearchagain_callback (w, x, c);
    }
    if (c->double_click) {
	unsigned char *text = (unsigned char *) w->text;
	q = strmovelines (text, w->current, c->yt - w->firstline, w->width);
	q = strfrombeginline (text, q, c->xt);
	p = q;
	if (my_is_letter (text[q])) {
	    while (--q >= 0)
		if (!my_is_letter (text[q]))
		    break;
	    q++;
	    while (text[++p])
		if (!my_is_letter (text[p]))
		    break;
	    strncpy (m, text + q, min (p - q, 127));
	    m[min (p - q, 127)] = 0;
	    t = str_strip_nroff ((char *) m, 0);
	    Cmanpagedialog (0, 0, 0, 0, 0, t);
	    free (t);
	}
    }
    check_prev_next ();
    return 0;
}


int manpageprev_callback (CWidget * w, XEvent * x, CEvent * c)
{
    history_current--;
    check_prev_next ();
    Credrawtextbox ("mandisplayfile.text", history[(history_current - 1) & (NUM_HISTORY - 1)], 1);
    return 0;
}

int manpagenext_callback (CWidget * w, XEvent * x, CEvent * c)
{
    history_current++;
    check_prev_next ();
    Credrawtextbox ("mandisplayfile.text", history[(history_current - 1) & (NUM_HISTORY - 1)], 1);
    return 0;
}

extern int replace_scanf;
extern int replace_regexp;
extern int replace_whole;
extern int replace_case;

int text_get_byte (unsigned char *text, long index)
{
    return text[index];
}

void Ctextboxsearch (CWidget * w, int again)
{
    static char *old = NULL;
    char *exp = "";
    int isfocussed = 0;

    exp = old ? old : exp;
    if (again) {
	if (!old)
	    return;
	exp = strdup (old);
    } else {
	isfocussed = (w->winid == CGetFocus ());
	edit_search_replace_dialog (w->parentid, 20, 20, &exp, 0, 0, " Search ", 0);
    }

    if (exp) {
	if (*exp) {
	    int len, l;
	    char *t;
	    long search_start;
	    if (old)
		free (old);
	    old = strdup (exp);
/* here we run strip on everything from here
   to the end of the file then search through
   the stripped text */
	    search_start = strmovelines (w->text, w->current, (isfocussed ? w->cursor - w->firstline : 0) + 1, 32000);
	    t = str_strip_nroff (w->text + search_start, &l);
	    search_start = edit_find (0, exp, &len, l, (int (*)(void *, long)) text_get_byte, (void *) t);
	    if (search_start == -3) {
		Cerrordialogue (w->parentid, 20, 20, " Error ", " Invalid regular expression ");
	    } else if (search_start >= 0) {
		int l;
		l = strcountlines (t, 0, search_start, 32000) + 1;
		l += isfocussed ? w->cursor : w->firstline;
		Csettextboxpos (w, TEXT_SETLINE, l);
		Csettextboxpos (w, TEXT_SET_CURSOR_LINE, l);
		Cexpose (w->ident);
	    } else {
		Cerrordialogue (w->parentid, 20, 20, " Search ", " Search search string not found. ");
	    }
	    free (t);
	}
	free (exp);
    }
}

int mansearch_callback (CWidget * w, XEvent * x, CEvent * c)
{
    Ctextboxsearch (Cwidget ("mandisplayfile.text"), 0);
    return 0;
}

int mansearchagain_callback (CWidget * w, XEvent * x, CEvent * c)
{
    Ctextboxsearch (Cwidget ("mandisplayfile.text"), 1);
    return 0;
}

char *read_pipe (int fd, int *len);
int my_popen (int *in, int *out, int *err, int mix, const char *file, char *const argv[]);

CWidget *Cmanpagedialog (Window in, int x, int y, int columns, int lines, const char *manpage)
{
    char *t;
    char *argv[] =
    {MAN_COMMAND, MAN_ARGS, 0, 0};
    int i = 0, man_pipe, fre = 1;

    if (manpage) {
	while (argv[i])
	    i++;
	argv[i] = (char *) manpage;

	if (my_popen (0, &man_pipe, 0, 1, MAN_COMMAND, argv) <= 0) {	/* "1" is to pipe both stderr AND stdout into man_pipe */
	    Cerrordialog (CMain, 20, 20, " Manual page ", " Fail trying to run man, check your man command in global.h ");
	    return 0;
	}
	t = read_pipe (man_pipe, 0);
	close (man_pipe);
    } else {
	if (!(t = history[(history_current - 1) & (NUM_HISTORY - 1)])) {
	    Cerrordialog (CMain, 20, 20, " Manual Page ", " No man pages open ");
	    return 0;
	}
	fre = 0;
    }

    if (!t) {
	Cerrordialog (CMain, 20, 20, " Manual Page ", get_sys_error (" Error reading from pipe, check your man command in the file global.h "));
	return 0;
    } else if (*t) {
	if (fre)
	    add_to_history (t);
	if (Cwidget ("mandisplayfile")) {
	    Credrawtextbox ("mandisplayfile.text", t, 0);
	} else {
	    Window win;
	    CWidget *w;
	    win = Cdrawheadedwindow ("mandisplayfile", in, x, y, 10, 10, " Manual Page ");
	    Cgethintpos (&x, &y);
	    w = Cdrawtextbox ("mandisplayfile.text", win,
			x, y, columns * TEXT_M_WIDTH + 7,
			lines * TEXT_PIX_PER_LINE, 0, 0, t,
			MAN_PAGE | TEXT_BOX_NO_CURSOR);
	    Cgethintpos (0, &y);
	    Cdrawbutton ("mandisplayfile.prev", win, x, y, AUTO_WIDTH, AUTO_HEIGHT, " Previous ");
	    Cgethintpos (&x, 0);
	    Cdrawbutton ("mandisplayfile.next", win, x, y, AUTO_WIDTH, AUTO_HEIGHT, " Next ");
	    Cgethintpos (&x, 0);
	    Cdrawbutton ("mandisplayfile.search", win, x, y, AUTO_WIDTH, AUTO_HEIGHT, " Search ");
	    Cgethintpos (&x, 0);
	    Cdrawbutton ("mandisplayfile.again", win, x, y, AUTO_WIDTH, AUTO_HEIGHT, " Again ");
	    Cgethintpos (&x, 0);
	    Cdrawbitmapbutton ("mandisplayfile.done", win, x, y, 40, 40, Ccolor (6), C_FLAT, tick_bits);
	    Cgethintpos (&x, 0);
	    Cdrawbitmapbutton ("mandisplayfile.clear", win, x, y, 40, 40, Ccolor (18), C_FLAT, cross_bits);
	    Csetsizehintpos ("mandisplayfile");
	    Caddcallback ("mandisplayfile.done", manpage_callback);
	    Caddcallback ("mandisplayfile.clear", manpageclear_callback);
	    Caddcallback ("mandisplayfile.text", manpage_callback);
	    Caddcallback ("mandisplayfile.next", manpagenext_callback);
	    Caddcallback ("mandisplayfile.prev", manpageprev_callback);
	    Caddcallback ("mandisplayfile.search", mansearch_callback);
	    Caddcallback ("mandisplayfile.again", mansearchagain_callback);
	}
	check_prev_next ();
	if (t && fre)
	    free (t);
    } else {
	Cerrordialog (CMain, 20, 20, " Manual page ", " Fail trying to popen man, check your man command in the file global.h ");
	return 0;
    }
    return Cwidget ("mandisplayfile");
}
