/* textwidget.c - for drawing a scrollable text window widget
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "edit.h"
#include "editcmddef.h"

#include "coolwidget.h"
#include "coollocal.h"
#include "dialog.h"

#include "mad.h"

int option_text_fg_normal = 0;
int option_text_fg_bold = 17;
int option_text_fg_italic = 16;

int option_text_bg_normal = 22;
int option_text_bg_marked = 25;
int option_text_bg_highlighted = 12;

CWidget *Cdrawtextbox (const char *identifier, Window parent, int x, int y,
		       int width, int height, int line, int column, const char *text, long options)
{
    char *scroll;
    int numlines;
    CWidget *wdt;

    int w, h;
    if (width == AUTO_WIDTH || height == AUTO_HEIGHT)
	Ctextsize (&w, &h, text);
    if (width == AUTO_WIDTH)
	width = w + 6;
    if (height == AUTO_HEIGHT)
        height = h + 6;

    wdt = Csetupwidget (identifier, parent, x, y,
	   width, height, CTEXTBOX_WIDGET, INPUT_KEY,
	   Ccolor (option_text_bg_normal), 1);

    wdt->options = options;
    wdt->text = strdup (text);
    numlines = strcountlines (text, 0, 1000000000, options & TEXT_WRAP ? wdt->width / TEXT_M_WIDTH : 32000) + 1;

    wdt->firstline = 0;
    wdt->firstcolumn = 0;
    wdt->cursor = 0;
    wdt->current = 0;
    wdt->numlines = numlines;
    wdt->textlength = strlen (wdt->text);

    Csettextboxpos (wdt, TEXT_SETLINE, line);
    Csettextboxpos (wdt, TEXT_SETCOL, column);

/* this will also set the hint position, Csethintpos() */
    Cdrawvertscrollbar (scroll = catstrs (identifier, ".vsc", 0), parent,
			x + width + WIDGET_SPACING, y, height, AUTO_WIDTH, 0, 0);

    wdt->scrollbar = Cwidget (scroll);
    wdt->scrollbar->textbox = wdt;

    return wdt;
}


long Csettextboxpos (CWidget * w, int which, long p);
void Cinputtobox (CWidget * wdt, CWidget * box);

/* redraws the text box. If preserve is 0 then view position is reset to 0 */
CWidget *Credrawtextbox (const char *identifier, const char *text, int preserve)
{
    CWidget *w = Cwidget (identifier);
    CWidget *cell;
    int numlines, firstline, firstcolumn, cursor;

    char *input = catstrs ("sprinp", w->ident, 0);

    if ((cell = Cwidget (input)))
	Cundrawwidget (input);

    free (w->text);
    w->text = strdup (text);
    w->textlength = strlen (w->text);
    numlines = strcountlines (text, 0, 1000000000, w->options & TEXT_WRAP ? w->width / TEXT_M_WIDTH : 32000) + 1;
    w->numlines = numlines;
    firstline = w->firstline;
    firstcolumn = w->firstcolumn;
    cursor = w->cursor;

    w->firstline = 0;
    w->current = 0;
    w->firstcolumn = 0;
    w->column = 0;
    w->cursor = 0;

    if (preserve) {
	Csettextboxpos (w, TEXT_SETLINE, firstline);
	Csettextboxpos (w, TEXT_SETCOL, firstcolumn);
	Csettextboxpos (w, TEXT_SET_CURSOR_LINE, cursor);
    }
    Cexpose (identifier);

    return w;
}


CWidget *Cdrawmanpage (const char *identifier, Window parent, int x, int y,
	   int width, int height, int line, int column, const char *text)
{
    CWidget *w;
    w = Cdrawtextbox (identifier, parent, x, y, width, height, line, column, text, MAN_PAGE);
    return w;
}



/*
   If which is TEXT_SETPOS the current offset of the top right
   corner is set to p and the top line number is returned;
   returns new ofset if which is TEXT_SETLINE (set the top line number);
   returns new column if which is TEXT_SETCOL;
   return top line number if which is TEXT_SET_CURSOR_LINE.
 */
long Csettextboxpos (CWidget * wdt, int which, long p)
{
    long q;
    int width;
    if (p < 0)
	p = 0;

    width = wdt->options & TEXT_WRAP ? wdt->width / TEXT_M_WIDTH : 32000;

    switch (which) {
    case TEXT_SETCOL:
	wdt->firstcolumn = p;
	return p;
    case TEXT_SETLINE:
	if (p >= wdt->numlines)
	    p = wdt->numlines - 1;
	if (p < 0)
	    p = 0;
	q = strmovelines (wdt->text, wdt->current, p - wdt->firstline, width);
	wdt->firstline += strcountlines (wdt->text, wdt->current, q - wdt->current, width);
	wdt->current = q;
	return wdt->current;
    case TEXT_SETPOS:
	wdt->firstline += strcountlines (wdt->text, wdt->current, p - wdt->current, width);
	wdt->current = p;
	return wdt->firstline;
    case TEXT_SET_CURSOR_LINE:
	if (p < 0)
	    p = 0;
	if (p >= wdt->numlines)
	    p = wdt->numlines - 1;
	wdt->cursor = p;
	if (p < wdt->firstline)
	    Csettextboxpos (wdt, TEXT_SETLINE, p);
	else if (p > wdt->firstline + wdt->height / TEXT_PIX_PER_LINE - 1)
	    Csettextboxpos (wdt, TEXT_SETLINE, p - wdt->height / TEXT_PIX_PER_LINE + 1);
	return wdt->firstline;
    }
    Cerror ("settextpos: command not found.\n");
    return 0;
}


/*
   ->firstline   is line number of the top line in the window.
   ->firstcolumn is column shift (positive).
   ->current     is actual char position of first line in display.
   ->numlines    is the total number of lines.
   ->cursor      is the number of the highlighted line.
   ->textlength  is the length of text excluding trailing NULL.
   First three must be initialised to proper values (e.g. 0, 0 and 0).
 */

#define r_is_printable(x) ((x >= ' ' && x <= '~') || x >= 160)

extern int EditExposeRedraw;

/*
   Renders the text box, doing nroff formating if necesary.
   wrapped text with nroff is not supported.
   Returns number of text lines printed.
 */
long Crendertextbox (CWidget * wdt, int redrawall)
{
    int w = wdt->width, h = wdt->height;
    int nroff, col = 0, row = 0, height, width;
    long from;
    unsigned char c;
    int wrap_mode, style = 0, draw = 1, index;
    Window win;
    unsigned short line[256];
    static Window lastwin;
    static long lasttexttop, lastcursorline, lastcol;
    static int lastfocussed;
    static long lastm1, lastm2;
    int isfocussed;
    long result, m1, m2;
    unsigned char *text;
    int curs = 1;

    win = wdt->winid;
    isfocussed = (win == CGetFocus ());

    nroff = (wdt->options & MAN_PAGE);
    wrap_mode = (wdt->options & TEXT_WRAP);
    if (nroff)
	wrap_mode = 0;

    if (!redrawall) {
	if (lastwin == win && lasttexttop == wdt->firstline && lastfocussed == isfocussed
	  && lastcol == wdt->firstcolumn && lastcursorline == wdt->cursor
	    && lastm1 == wdt->mark1 && lastm2 == wdt->mark2) {
	    draw = 0;
	}
    } else {
	EditExposeRedraw = 1;
    }

    m1 = min (wdt->mark1, wdt->mark2);
    m2 = max (wdt->mark1, wdt->mark2);

/* when text is highlighted, the cursor must be off */
    if (wdt->options & TEXT_BOX_NO_CURSOR || m1 != m2)	/* don't draw the cursor line */
	curs = 0;

    edit_set_foreground_colors (Ccolor (option_text_fg_normal), Ccolor (option_text_fg_bold), Ccolor (option_text_fg_italic));
    edit_set_background_colors (Ccolor (option_text_bg_normal), Ccolor (0), Ccolor (option_text_bg_marked), Ccolor (9), Ccolor (option_text_bg_highlighted));

    text = (unsigned char *) wdt->text;
    height = wdt->height / TEXT_PIX_PER_LINE;
    width = wdt->width / TEXT_M_WIDTH;
    from = wdt->current;

    if (row + wdt->firstline == wdt->cursor && isfocussed && curs)
	shortset ((short *) line, ' ' + 256 * MOD_HIGHLIGHTED, width);
    else
	shortset ((short *) line, ' ', width);
    line[width] = 0;

    for (; row < height; from++) {
	c = text[from];
	style = 0;
	if (!c)
	    break;
	if (c == '\n' || (col == width && wrap_mode)) {
	    if (from >= m1 && from < m2)
		line[col - wdt->firstcolumn] = ' ' | (MOD_MARKED * 256);
	    col = 0;
	    if (draw)
		xprint_to_widget (win, row, 0, 0, width - 1, line);
	    row++;
	    if (row + wdt->firstline == wdt->cursor && isfocussed && curs)
		shortset ((short *) line, ' ' + 256 * MOD_HIGHLIGHTED, width);
	    else
		shortset ((short *) line, ' ', width);
	    line[width] = 0;
	    if (c == '\n' || row >= height)
		continue;
	}
	if (from >= m1 && from < m2)
	    style |= MOD_MARKED;
	if (c == '\r')
	    continue;
	if (c == '\t') {
	    int col2;
	    col2 = col + 8 - (col % 8);
	    do {
		line[col - wdt->firstcolumn] = ' ' + ((style * 256) | (line[col - wdt->firstcolumn] & 0xFF00));
	    } while (++col < col2);
	    continue;
	}
	if (nroff && text[from + 1] == '\b') {
	    if (r_is_printable (text[from + 2]) && r_is_printable (c)) {
		from += 2;
		while (text[from + 1] == '\b' && r_is_printable (text[from + 2]) && r_is_printable (text[from]))
		    from += 2;
		c = text[from];
		if (text[from - 2] == '_')
		    style |= MOD_ITALIC;
		else
		    style |= MOD_BOLD;
	    }
	}
#if 0
	if (wdt->found_len && from >= wdt->search_start
	    && from < wdt->search_start + wdt->found_len) {
	    style = MOD_HIGHLIGHTED;
	}
#endif
	if (col >= wdt->firstcolumn && col < width + wdt->firstcolumn) {
	    index = col - wdt->firstcolumn;
	    if (!r_is_printable (c))
		c = '.';
	    line[index] = c + ((style * 256) | (line[index] & 0xFF00));
	}
	col++;
    }

    result = row + 1;

    do {
	if (draw) {
	    xprint_to_widget (win, row, 0, 0, width - 1, line);
	    shortset ((short *) line, ' ', width);
	    line[width] = 0;
	}
    } while (row++ < height);

    lasttexttop = wdt->firstline;
    lastcol = wdt->firstcolumn;
    lastwin = win;
    lastcursorline = wdt->cursor;
    lastfocussed = isfocussed;
    lastm1 = wdt->mark1;
    lastm2 = wdt->mark2;

    if (isfocussed) {
	Crenderbevel (win, 0, 0, w - 1, h - 1, 3, 1);	/*most outer border bevel */
    } else {
	Crenderbevel (win, 2, 2, w - 3, h - 3, 1, 1);	/*border bevel */
	Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 0);	/*most outer border bevel */
    }

    EditExposeRedraw = 0;
    return result;
}



/*
   Count the number of lines that would be printed
   by the above routine, but don't print anything.
   If all is non-zero then count all the lines.
 */
long Ccounttextboxlines (CWidget * wdt, int all)
{
    int nroff, col = 0, row = 0, height, width;
    long from;
    unsigned char c;
    int wrap_mode;
    unsigned char *text;

    nroff = (wdt->options & MAN_PAGE);
    wrap_mode = (wdt->options & TEXT_WRAP);
    if (nroff)
	wrap_mode = 0;

    text = (unsigned char *) wdt->text;
    height = wdt->height / TEXT_PIX_PER_LINE;
    width = wdt->width / TEXT_M_WIDTH;
    if (all)
	from = 0;
    else
	from = wdt->current;

    for (; row < height || all; from++) {
	c = text[from];
	if (!c)
	    break;
	if ((c == '\n') || (col == width && wrap_mode)) {
	    col = 0;
	    row++;
	    if (c == '\n' || row >= height)
		continue;
	}
	if (c == '\r')
	    continue;
	if (c == '\t') {
	    col = (col / 8) * 8 + 8;
	    continue;
	}
	col++;
    }
    return row + 1;
}

/* move the text box cursor or the text window if there isn't one */
int Ctextboxcursormove (CWidget * wdt, KeySym key)
{
    int handled = 0;
/* when text is highlighted, the cursor must be off */
    if (wdt->options & TEXT_BOX_NO_CURSOR || wdt->mark1 != wdt->mark2) {
	int move = 0;
	switch (key) {
	case CK_Up:
	    handled = 1;
	    move = -1;
	    break;
	case CK_Down:
	    handled = 1;
	    move = 1;
	    break;
	case CK_Page_Up:
	    handled = 1;
	    move = 1 - wdt->height / TEXT_PIX_PER_LINE;
	    break;
	case CK_Page_Down:
	    handled = 1;
	    move = wdt->height / TEXT_PIX_PER_LINE - 1;
	    break;
	case CK_Home:
	    handled = 1;
	    move = -32000;
	    break;
	case CK_End:
	    handled = 1;
	    move = 32000;
	    break;
	case CK_Left:
	    handled = 1;
	    if (wdt->firstcolumn > 0)
		wdt->firstcolumn--;
	    break;
	case CK_Right:
	    handled = 1;
	    wdt->firstcolumn++;
	    break;
	}
	Csettextboxpos (wdt, TEXT_SETLINE, wdt->firstline + move);
    } else {
	switch (key) {
	case CK_Up:
	    handled = 1;
	    wdt->cursor--;
	    break;
	case CK_Down:
	    handled = 1;
	    wdt->cursor++;
	    break;
	case CK_Page_Up:
	    handled = 1;
	    wdt->cursor -= (wdt->height / TEXT_PIX_PER_LINE - 1);
	    break;
	case CK_Page_Down:
	    handled = 1;
	    wdt->cursor += (wdt->height / TEXT_PIX_PER_LINE - 1);
	    break;
	case CK_Home:
	    handled = 1;
	    wdt->cursor = 0;
	    break;
	case CK_End:
	    handled = 1;
	    wdt->cursor = wdt->numlines;
	    break;
	case CK_Left:
	    handled = 1;
	    if (wdt->firstcolumn > 0) {
		wdt->firstcolumn--;
	    }
	    break;
	case CK_Right:
	    handled = 1;
	    wdt->firstcolumn++;
	    break;
	}
	Csettextboxpos (wdt, TEXT_SET_CURSOR_LINE, wdt->cursor);	/* just does some checks */
    }
    return handled;
}


void edit_translate_xy (int xs, int ys, int *x, int *y);

/* returns the position in the edit buffer of a window click */
long text_get_click_pos (CWidget * w, int x, int y)
{
    long click;
    int width;
    width = w->options & TEXT_WRAP ? w->width / TEXT_M_WIDTH : 32000;
    click = strmovelines (w->text, w->current, y, width);
    click = strcolmove (w->text, click, x);
    return click;
}


void text_mouse_mark (CWidget * w, XEvent * event, CEvent * ce)
{
    static Window win_press = 0;
    static drag = 0;
    static char *last_text;
    long click;
    int x, y;

    edit_translate_xy (event->xbutton.x, event->xbutton.y, &x, &y);
    click = text_get_click_pos (w, --x, --y);

    if (event->type == ButtonPress) {
	if (click >= min (w->mark1, w->mark2) && click < max (w->mark1, w->mark2)) {
	    char *t, *t2;
	    int l, type;
	    l = abs (w->mark2 - w->mark1);
	    t = Cmalloc (l + 1);
	    memcpy (t, w->text + min (w->mark1, w->mark2), l);
	    t[l] = 0;
	    t2 = str_strip_nroff ((char *) t, &l);
	    free (t);
	    t2[l] = 0;
	    if (w->options & TEXT_FILES) {
		char *p;
		int i;
		p = CDndFileList (t2, &l, &i);
		if (!p) {
		    free (t2);
		    return;
		}
		free (t2);
		t2 = p;
		if (i == 1)
		    type = DndFile;
		else
		    type = DndFiles;
	    } else {
		type = DndText;
	    }
	    CDrag (event->xbutton.window, type, t2, l, 0);
	    free (t2);
	    return;
	}
	w->mark1 = w->mark2 = click;
	win_press = w->winid;
	last_text = w->text;
	drag = 1;
	return;
    }
    if (win_press != w->winid || !drag || (unsigned long) last_text != (unsigned long) w->text) {
	w->mark1 = w->mark2 = 0;
	return;
    }
    if (event->type == ButtonRelease) {
	win_press = 0;
	drag = 0;
	w->mark2 = click;
	return;
    }
    if (event->type == MotionNotify) {
	int h;
	h = (w->height - 6) / TEXT_PIX_PER_LINE;
	if (y > h && w->firstline < w->numlines - h - 2)
	    Csettextboxpos (w, TEXT_SETLINE, w->firstline + y - h);
	if (y < 0)
	    Csettextboxpos (w, TEXT_SETLINE, w->firstline + y);
	w->mark2 = click;
    }
}


int eh_textbox (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    int cmd, ch;
    int handled = 0;
    int curpos;
    char *intext;
    char xlat;
    KeySym key;
    int redrawall;
    int count;

    redrawall = 0;
    switch (xevent->type) {
    case Expose:
	if (!xevent->xexpose.count)
	    redrawall = 1;
	break;
    case ClientMessage:
	w->mark1 = w->mark2 = 0;
	break;
    case ButtonPress:
	CFocusWindow (w->winid);
	if (xevent->xbutton.button == Button1)
	    w->cursor = (xevent->xbutton.y - 6) / TEXT_PIX_PER_LINE + w->firstline;
	if (w->cursor > w->numlines - 1)
	    w->cursor = w->numlines - 1;
	if (w->cursor < 0)
	    w->cursor = 0;
	cwevent->ident = w->ident;
	cwevent->xt = (xevent->xbutton.x - 7) / TEXT_M_WIDTH + w->firstcolumn;
	cwevent->yt = w->cursor;
    case ButtonRelease:
    case MotionNotify:
	Cresolvebutton (xevent, cwevent);
	text_mouse_mark (w, xevent, cwevent);
	break;
    case FocusIn:
    case FocusOut:
	break;
    case KeyPress:
	xlat = 0;
	XLookupString (&(xevent->xkey), &xlat, 1, &key, NULL);

	curpos = w->firstline;
	intext = w->text;

	cwevent->ident = w->ident;
	cwevent->key = key;
	cwevent->xlat = xlat;
	cwevent->state = xevent->xkey.state;

	if (!(TEXT_NO_KEYS & w->options))
	    if (edit_translate_key (0, xevent->xkey.keycode, key, xevent->xkey.state, &cmd, &ch)) {
		cwevent->command = cmd;
		handled = Ctextboxcursormove (w, cmd);
	    }
	break;
    default:
	return 0;
    }

/* Now draw the changed text box, count will contain
   the number of textlines displayed */
    count = Crendertextbox (w, redrawall);

/* now update the scrollbar position */
    w->scrollbar->firstline = (double) 65535.0 *w->firstline / w->numlines;
    w->scrollbar->numlines = (double) 65535.0 *count / w->numlines;
    w->scrollbar->options = 0;
    Crenderscrollbar (w->scrollbar);

    return handled;
}
