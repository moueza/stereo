/* coolwidget.c - routines for simple widgets. Widget setup and destruction
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

#define COOL_WIDGET_C

#include "coolwidget.h"
#include "coollocal.h"
#include "dialog.h"

#include "mad.h"

/* call this for fatal errors */
void Cerror (const char *fmt,...)
{
    va_list s;
    char *str;
    va_start (s, fmt);
    str = vsprintf_alloc (catstrs (" ", fmt, " ", 0), s);
    Cfatalerrordialog (20, 20, str);
    free (str);
}



/* an malloc with an error check */

#ifndef HAVE_MAD

#ifndef Cmalloc
void *Cmalloc (size_t size)
{
    void *p;
    if ((p = malloc (size)) == NULL)
	Cerror ("Unable to allocate memory.\n");
    return p;
}
#endif

void *CDebugMalloc (size_t x, int line, const char *file)
{
    void *p;
    if ((p = malloc (x)) == NULL)
	Cerror ("Unable to allocate memory: line %d, file %s.\n", line, file);
    return p;
}

#endif


struct cw_cursor {
    int x, y, h, w;
    Window window;
    int state;
    int type;
    char chr;
    unsigned long bg, fg;
};

struct cw_cursor CursorState =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void Cdrawcursor (Window window);

void Csetcursor (Window win, int x, int y, int w, int h, int type, int chr, unsigned long bg, unsigned long fg)
{
    if (win == CGetFocus ()) {
	CursorState.x = x;
	CursorState.y = y;
	CursorState.h = h;
	CursorState.w = w;
	CursorState.window = win;
	CursorState.type = type;
	CursorState.chr = chr;
	CursorState.bg = bg;
	CursorState.fg = fg;
	Cdrawcursor (win);
    } else {
	if (!(win | h | w))
	    CursorState.window = 0;
    }
}



void render_cursor (struct cw_cursor c, int clear_cursor)
{
    if (c.type == EDITOR_CURSOR) {
	if (!c.state || clear_cursor)
	    Csetcolor (c.bg);
	else
	    Csetcolor (Ccolor (19));
	Cline (c.window, c.x, c.y + CFontStruct->descent + IN_FONT_OFFSET_Y, c.x, c.y + c.h - 1 + IN_FONT_OFFSET_Y);
	Cline (c.window, c.x + 1, c.y + CFontStruct->descent + IN_FONT_OFFSET_Y, c.x + 1, c.y + c.h - 1 + IN_FONT_OFFSET_Y);
	Cline (c.window, c.x + 2, c.y + CFontStruct->descent + IN_FONT_OFFSET_Y, c.x + c.w - 1, c.y + CFontStruct->descent + IN_FONT_OFFSET_Y);
	Cline (c.window, c.x + 2, c.y + CFontStruct->descent + IN_FONT_OFFSET_Y + 1, c.x + c.w - 1, c.y + CFontStruct->descent + 1 + IN_FONT_OFFSET_Y);
	if (!c.state || clear_cursor) {
	    Csetbackcolor (c.bg);
	    Csetcolor (c.fg);
	    XDrawImageString (CDisplay, c.window, CGC, c.x + FONT_OFFSET_X, c.y + FONT_OFFSET_Y, &(c.chr), 1);
	}
    } else {
	if (CursorState.state) {
	    Csetcolor (C_FLAT);
	    Cline (c.window, c.x, c.y, c.x, c.y + c.h - 6);
	    Crenderbevel (c.window, c.x - 1, c.y - 1, c.x + 1, c.y + c.h - 5, 1, 0);
	} else {
	    Csetcolor (C_FLAT);
	    Cline (c.window, c.x, c.y, c.x, c.y + c.h - 6);	/*cursor bar flat */
	    Crenderbevel (c.window, c.x - 1, c.y - 1, c.x + 1, c.y + c.h - 5, 1, -1);	/*cursor bar bevel */
	}
    }
}


/* this draws a full cursor if its window is focussed and draws a thin line otherwise. */
void Cdrawcursor (Window window)
{
    if (CursorState.window == CGetFocus () && CursorState.window)
	render_cursor (CursorState, 0);
}


/* this is called from CNextEvent if an alarm event comes */
void Ctoggle_cursor ()
{
    CursorState.state = 1 - CursorState.state;
    Cdrawcursor (0);		/* Cdrawcursor above does nothing if no window is focussed */
}



/*
   These two routines are not much slower than doing the same
   thing with integers as identifiers instead of strings.
   It returns the index in the global array of widgets of
   the widget named ident. Returns 0 if not found.
 */
static inline int _CW_i (const char *ident)
{
    int i = CLastwidget + 1;
    quad_t p;

    memcpy (&p, ident, sizeof (quad_t));	/* we need four byte alignment for some machines */

    if (ident[2]) {		/* can compare first four bytes at once */
	while (--i)
	    if (CW (i))
		if (*((quad_t *) CW (i)->ident) == p)
		    if (!strcmp (CW (i)->ident, ident))
			return i;
	return 0;
    } else {
	word s = *((word *) (&p));
	while (--i)
	    if (CW (i))
		if (*((word *) CW (i)->ident) == s)
		    if (!strcmp (CW (i)->ident, ident))
			return i;

    }
    return 0;
}

int Ci (const char *ident)
{
    return _CW_i (ident);
}

CWidget *Cwidget (const char *ident)
{
    return CW (_CW_i (ident));
}

int Csystem (const char *string)
{
    int r;
    CDisableAlarm ();
    r = system (string);
    CEnableAlarm ();
    return r;
}

extern int (*global_alarm_callback) (CWidget *, XEvent *, CEvent *);

void Caddcallback (const char *ident, int (*callback) (CWidget *, XEvent *, CEvent *))
{
    CWidget *w = Cwidget (ident);
    if (w)
	w->callback = callback;
    else {
	if (!strcmp (ident, "AlarmCallback"))
	    global_alarm_callback = callback;
    }
}

/* checks the magic numbers */
int Ccheck ()
{
    int i = 0;

    while (CLastwidget > i++)
	if (CW (i) != NULL)
	    if (CW (i)->magic_begin != CMAGIC_BEGIN || CW (i)->magic_end != CMAGIC_END)
		Cerror ("Cool widget internal error - magic number overwritten overwritten.\n");
    return 0;
}

/* sends a full expose event to the widget */
void Cexpose (const char *ident)
{
    CWidget *w = Cwidget (ident);
    if (w)
	CSendExpose (w->winid, 0, 0, w->width, w->height);
}

/* Returns the widgets window or 0 if not found */
Window Cwin (const char *ident)
{
    CWidget *w = Cwidget (ident);
    if (w)
	return w->winid;
    else
	return 0;
}

/* send an expose event to the internel queue */
void Cexposewindowarea (Window win, int count, int x, int y, int w, int h)
{
    if (x < 0) {
	w = x + w;
	x = 0;
    }
    if (y < 0) {
	h = y + h;
	y = 0;
    }
    if (w <= 0 || h <= 0)
	return;

    CSendExpose (win, x, y, w, h);
}


/* Returns the first NULL list entry. Exits if list full. */
CWidget **Cfindemptywidgetentry ()
{
    int i = 0;

/* widget can be added to an empty point in the list (created from an
   undraw command, or to the end of the list. */
    while (CLastwidget > i++) {
	if (CW (i) == NULL)
	    break;
    }

    if (i == MAX_NUMBER_OF_WIDGETS - 2)
	Cerror ("No more space in widget list\nIncrease MAX_NUMBER_OF_WIDGETS in coolwidget.h\n");

    if (i == CLastwidget)
	CLastwidget++;		/* increase list length if an entry was added to the end */

    return &(CW (i));
}


/* fills in the widget structure */
CWidget *Callocatewidget (Window newwin, const char *ident, Window parent, int x, int y,
			  int width, int height, int kindofwidget)
{
    CWidget *w = Cmalloc (sizeof (CWidget));
    memset (w, 0, sizeof (CWidget));	/*: important, 'cos free's check if NULL before freeing many parems */

    w->magic_begin = CMAGIC_BEGIN;
    w->winid = newwin;
    w->parentid = parent;
    w->width = width;
    w->height = height;
    w->x = x;
    w->y = y;
    strncpy (w->ident, ident, 32);

    w->kind = kindofwidget;
    w->magic_end = CMAGIC_END;
    return w;
}


/*
   Sets up the widget's window and calls Callocatewidget()
   to allocate space and set up the data structures.
   What is set up here is common to all widgets, so
   it will always be the first routine called by a Cdrawthis()
   function.
 */
CWidget *Csetupwidget (const char *identifier, Window parent, int x, int y,
	    int width, int height, int kindofwidget, unsigned long input,
		       unsigned long bgcolor, int takes_focus)
{
    Window newwin;
    CWidget **w;

    if (Cwidget (identifier) && kindofwidget == CBUTTON_WIDGET)
	Cerror ("Trying to create a button with the same identifier as an existing widget.\n");

    newwin = XCreateSimpleWindow (CDisplay, parent, x, y, width,
				  height, 0, C_BLACK, bgcolor);

    XSelectInput (CDisplay, newwin, input);

    XMapWindow (CDisplay, newwin);	/* shows the window */

    w = Cfindemptywidgetentry ();	/* find first unused list entry in list of widgets */
    *w = Callocatewidget (newwin, identifier, parent, x, y,
			  width, height, kindofwidget);
    (*w)->eh = Cdefaulthandler (kindofwidget);
    (*w)->takes_focus = takes_focus;

    return (*w);
}

Window Cdrawwindow (const char *identifier, Window parent, int x, int y,
		    int width, int height, const char *label)
{
    Window w;
    w = (Csetupwidget (identifier, parent, x, y,
	 width, height, CWINDOW_WIDGET, INPUT_MOTION, C_FLAT, 0))->winid;
    Cresethintpos (WIDGET_SPACING + 2, WIDGET_SPACING + 2);
    return w;
}

Window Cdrawheadedwindow (const char *identifier, Window parent, int x, int y,
			  int width, int height, const char *label)
{
    int w, h;
    Window win;
    CWidget *wdt;

    Ctextsize (&w, &h, label);
    win = Cdrawwindow (identifier, parent, x, y,
		       width, height + h + WIDGET_SPACING * 3 + 3, label);
    wdt = Cdrawtext (catstrs (identifier, "header", 0), win, (width - w) / 2 - 1 - TEXT_RELIEF, WIDGET_SPACING, label);
    wdt->options = TEXT_CENTRED;
    Cgethintpos (&x, &y);
    Cdrawbar (win, WIDGET_SPACING, y, width - WIDGET_SPACING * 2, BAR_WINDOW_WIDTH);
    Cgethintpos (&x, &y);
    Cresethintpos (WIDGET_SPACING + 2, y);
    return win;
}

#define r_lcase(x) (((x) >= 'A' && (x) <= 'Z') ? (x) + 'a' - 'A' : (x))

int find_hotkey (CWidget * w)
{
    char used_keys[32], *label;
    int j, n = -1;
    CWidget *p = w;
    int c;

    if (!*(label = w->label))
	return 0;

    do {
	w = CNextFocus (w);
	if (!w || n == 32)
	    return 0;
	used_keys[++n] = r_lcase (w->hotkey);
    } while ((unsigned long) w != (unsigned long) p);

    c = r_lcase (label[0]);
    if (c >= 'a' && c <= 'z')
	if (!memchr (used_keys, c, n))	/* check if first letter has not already been used */
	    return label[0];

    for (j = 1; label[j]; j++) {	/* check for letters at start of words that have not already been used */
	c = r_lcase (label[j]);
	if (label[j - 1] == ' ' && c >= 'a' && c <= 'z')
	    if (!memchr (used_keys, c, n))
		return label[j];
    }

    for (j = 1; label[j]; j++) {	/* check for any letters that have not already been used */
	c = r_lcase (label[j]);
	if (c >= 'a' && c <= 'z')
	    if (!memchr (used_keys, c, n))
		return label[j];
    }
    return 0;
}


CWidget *Cdrawbutton (const char *identifier, Window parent, int x, int y,
		      int width, int height, const char *label)
{
    CWidget *wdt;
    int w, h;
    if (width == AUTO_WIDTH || height == AUTO_HEIGHT)
	Ctextsize (&w, &h, label);
    if (width == AUTO_WIDTH)
	width = w + 4 + BUTTON_RELIEF * 2;
    if (height == AUTO_HEIGHT)
	height = h + 4 + BUTTON_RELIEF * 2;
    wdt = Csetupwidget (identifier, parent, x, y,
		 width, height, CBUTTON_WIDGET, INPUT_BUTTON, C_FLAT, 1);
    wdt->label = strdup (label);
    wdt->hotkey = find_hotkey (wdt);
    Csethintpos (x + width + WIDGET_SPACING, y + height + WIDGET_SPACING);
    return wdt;
}

CWidget *Cdrawprogress (const char *identifier, Window parent, int x, int y,
			int width, int height, int p)
{
    CWidget *w;
    if ((w = Cwidget (identifier))) {
	w->cursor = p;
	Csetwidgetposition (identifier, x, y);
	Csetwidgetsize (identifier, width, height);
	Cexpose (identifier);
    } else {
	w = Csetupwidget (identifier, parent, x, y,
	       width, height, CPROGRESS_WIDGET, INPUT_EXPOSE, C_FLAT, 0);
	w->cursor = p;
	Csethintpos (x + width + WIDGET_SPACING, y + height + WIDGET_SPACING);
    }
    return w;
}


void Crendertextinput (CWidget * w);

/*
   This will reallocate a previous draw of the same identifier.
   so you can draw the same widget over and over without flicker
 */
CWidget *Cdrawtextinput (const char *identifier, Window parent, int x, int y,
		     int width, int height, int maxlen, const char *text)
{
    CWidget *wdt;

    if (!(wdt = Cwidget (identifier))) {

	int w, h;
	if (width == AUTO_WIDTH || height == AUTO_HEIGHT)
	    Ctextsize (&w, &h, text);
	if (width == AUTO_WIDTH)
	    width = w + 6 + TEXT_INPUT_RELIEF * 2;
	if (height == AUTO_HEIGHT)
	    height = h + 6 + TEXT_INPUT_RELIEF * 2;

	Csethintpos (x + width + WIDGET_SPACING, y + height + WIDGET_SPACING);

	wdt = Csetupwidget (identifier, parent, x, y,
		 width, height, CTEXTINPUT_WIDGET, INPUT_KEY, C_FLAT, 1);

/* For the text input widget we need enough memory allocated to the label
   for it to grow to maxlen, so reallocate it */

	wdt->text = Cmalloc (maxlen + 16);
	strcpy (wdt->text, text);
	wdt->cursor = strlen (text);
	wdt->firstcolumn = 0;
	wdt->textlength = maxlen;
    } else {			/*redraw the thing so it doesn't flicker if its redrawn in the same place.
				   Also, this doesn't need an undraw */
	Csetwidgetsize (identifier, width, height);
	wdt->x = x;
	wdt->y = y;
	XMoveWindow (CDisplay, wdt->winid, x, y);
	free (wdt->text);
	wdt->text = Cmalloc (maxlen + 16);
	strcpy (wdt->text, text);
	wdt->cursor = strlen (text);
	wdt->firstcolumn = 0;
	wdt->textlength = maxlen;
	wdt->keypressed = 0;
	Crendertextinput (wdt);
    }

    return wdt;
}


CWidget *Cdrawvertscrollbar (const char *identifier, Window parent, int x, int y,
			     int length, int width, int pos, int prop)
{
    CWidget *w;
    if (width == AUTO_WIDTH)
	width = 20;
    w = Csetupwidget (identifier, parent, x, y,
		      width, length, CVERTSCROLL_WIDGET,
		      ExposureMask | ButtonPressMask |
	       ButtonReleaseMask | ButtonMotionMask | PointerMotionMask |
		      EnterWindowMask | LeaveWindowMask, C_FLAT, 0);
    w->firstline = pos;
    w->numlines = prop;
    Csethintpos (x + width + WIDGET_SPACING, y + length + WIDGET_SPACING);
    return w;
}


CWidget *Cdrawbar (Window parent, int x, int y, int w, int options)
{
    CWidget *wdt;
    wdt = Csetupwidget ("hbar", parent, x, y,
			 w, 3, CBAR_WIDGET, INPUT_EXPOSE, C_FLAT, 0);
    wdt->options = options;
    Csethintpos (x + w + WIDGET_SPACING, y + 3 + WIDGET_SPACING);
    return wdt;
}

/* returns the text size. The result is one descent greater than the actual size */
void Ctextsize (int *w, int *h, const char *str)
{
    char *r, *p, *q;
    int w1, h1;

    if (!w)
	w = &w1;
    if (!h)
	h = &h1;

    *w = *h = 0;

    r = q = catstrs (str, "\n", NULL);

    while ((p = strchr (q, '\n'))) {
	*h += TEXT_PIX_PER_LINE;
	*w = max (XTextWidth (CFontStruct, q, (unsigned long) p - (unsigned long) q), *w);
	q = p + 1;
    }
}


CWidget *Cdrawtext (const char *identifier, Window parent, int x, int y, const char *fmt,...)
{
    va_list pa;
    char *str;
    int w, h;
    CWidget *wdt;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    Ctextsize (&w, &h, str);
    w += TEXT_RELIEF * 2 + 2;
    h += TEXT_RELIEF * 2 + 2;
    wdt = Csetupwidget (identifier, parent, x, y,
			w, h, CTEXT_WIDGET, INPUT_EXPOSE, C_FLAT, 0);
    wdt->text = strdup (str);
    free (str);
    Csethintpos (x + w + WIDGET_SPACING, y + h + WIDGET_SPACING);
    return wdt;
}


void Crendertext (CWidget * w);
void Crerendertext (CWidget * wdt);

CWidget *Credrawtext (const char *identifier, const char *fmt,...)
{
    va_list pa;
    char *str;
    CWidget *wdt;
    int w, h;

    wdt = Cwidget (identifier);
    if (!wdt)
	return 0;

    va_start (pa, fmt);
    str = vsprintf_alloc (fmt, pa);
    va_end (pa);

    free (wdt->text);
    wdt->text = strdup (str);

    Ctextsize (&w, &h, str);
    w += TEXT_RELIEF * 2 + 2;
    h += TEXT_RELIEF * 2 + 2;

    Csetwidgetsize (identifier, w, h);
    Crerendertext (wdt);
    free (str);
    return wdt;
}

void focus_stack_remove_window (Window w);

/*
   Unmaps and destroys widget and frees memory.
   Only for a widget that has no children.
 */
int Cfreesinglewidget (int i)
{
    if (i && CW (i)) {
	if (CW (i)->winid) {
	    if (CursorState.window == CW (i)->winid)
		Csetcursor (0, 0, 0, 0, 0, 0, 0, 0, 0);
	    XUnmapWindow (CDisplay, CW (i)->winid);
	    XDestroyWindow (CDisplay, CW (i)->winid);
	    focus_stack_remove_window (CW (i)->winid);	/* removes the window from the focus history stack */
	}
	if (CW (i)->label)
	    free (CW (i)->label);
	if (CW (i)->headings)
	    free (CW (i)->headings);
	if (CW (i)->gl_graphicscontext)
	    free (CW (i)->gl_graphicscontext);

#ifndef HAVE_MAD
	if (CW (i)->ximage) {
	    if ((long) CW (i)->ximage->data == (long) CW (i)->graphic)
		CW (i)->graphic = NULL;
	    XDestroyImage (CW (i)->ximage);
	}
#else
	if (CW (i)->ximage) {
	    if ((long) CW (i)->ximage->data == (long) CW (i)->graphic)
		CW (i)->graphic = NULL;
	    free (CW (i)->ximage->data);
	}
#endif
	if (CW (i)->graphic)
	    free (CW (i)->graphic);

	if (CW (i)->tab)
	    free (CW (i)->tab);

	if (CW (i)->text)
	    free (CW (i)->text);

	if (CW (i)->destroy)
	    (*(CW (i)->destroy)) (CW (i));

	free (CW (i));
	CW (i) = NULL;
	if (i == CLastwidget - 1)
	    CLastwidget--;

	return 1;
    } else
	return 0;
}

/*searches for the first widget in the list that has win as its parent
   and returns index */
int Cfindfirstchildof (Window win)
{
    int i = 0;
    while (CLastwidget > i++)
	if (CW (i) != NULL)
	    if (CW (i)->parentid == win)
		return i;
    return 0;
}

int Cfindlastchildof (Window win)
{
    int i = CLastwidget;
    while (--i > 0)
	if (CW (i) != NULL)
	    if (CW (i)->parentid == win)
		return i;
    return 0;
}

int CWidgetOf (Window win)
{
    int i = 0;
    while (CLastwidget > i++)
	if (CW (i) != NULL)
	    if (CW (i)->winid == win)
		return i;
    return 0;
}

int Cfindnextchildof (Window win, Window child)
{
    int i = CWidgetOf (child);
    if (i)
	while (CLastwidget > i++)
	    if (CW (i) != NULL)
		if (CW (i)->parentid == win)
		    return i;
    return 0;
}

int Cfindpreviouschildof (Window win, Window child)
{
    int i = CWidgetOf (child);
    if (i)
	while (--i > 0)
	    if (CW (i) != NULL)
		if (CW (i)->parentid == win)
		    return i;
    return 0;
}

/*recursively destroys a widget and all its descendants */
void Crecursivedestroy (int i)
{
    int j;
    while ((j = Cfindfirstchildof (CW (i)->winid)))
	Crecursivedestroy (j);
    Cfreesinglewidget (i);
}

void CFocusLast (void);

/*returns 1 on error --- not found. Destroys a widget by name and all its
   descendents */
int Cundrawwidget (const char *identifier)
{
    int i = Ci (identifier);

    if (i) {
	Crecursivedestroy (i);
	CFocusLast ();
	return 0;
    } else
	return 1;
}


void Cundrawall ()
{
    int j;
    while ((j = Cfindfirstchildof (CMain)))
	Crecursivedestroy (j);
    while ((j = Cfindfirstchildof (DefaultRootWindow (CDisplay))))
	Crecursivedestroy (j);
}

void free_last_query_buttons (void);

void CShutdown (void)
{
    Cundrawall ();
    free (home_dir);
    free (temp_dir);
    home_dir = 0;
    temp_dir = 0;
    free_last_query_buttons ();
    XFreeGC (CDisplay, CGC);
    XUnloadFont (CDisplay, CFontStruct->fid);
    XDestroyWindow (CDisplay, CMain);
    XCloseDisplay (CDisplay);
}

void Cdrawstringxy (Window win, int x, int y, const char *text)
{
    XDrawString (CDisplay, win,
		 CGC,
		 FONT_OFFSET_X + x,
		 FONT_OFFSET_Y + y,
		 text,
		 strlen (text));
}

void Cdrawstring (Window win, const char *text)
{
    Cdrawstringxy (win, 0, 0, text);
}

/* return 1 if focussed */
int render_focus_ring (CWidget * wdt)
{
    unsigned int wd, hd, d;
    int xd, yd;
    Window p;
    Window win = wdt->winid;

    XGetGeometry (CDisplay, win, &p, &xd, &yd, &wd, &hd, &d, &d);

    if (win == CGetFocus ()) {
	if (wd == wdt->width) {
	    XMoveWindow (CDisplay, win, wdt->x - FOCUS_RING, wdt->y - FOCUS_RING);
	    XResizeWindow (CDisplay, win, wd + 2 * FOCUS_RING, hd + 2 * FOCUS_RING);
	}
	Crenderbevel (win, 0, 0, wdt->width + 2 * FOCUS_RING - 1, wdt->height + 2 * FOCUS_RING - 1, 1, 0);
	Crenderbevel (win, 2, 2, wdt->width + 2 * FOCUS_RING - 3, wdt->height + 2 * FOCUS_RING - 3, 2, 1);
	return 1;
    } else {
	if (wd != wdt->width) {
	    XMoveWindow (CDisplay, win, wdt->x, wdt->y);
	    XResizeWindow (CDisplay, win, wd - 2 * FOCUS_RING, hd - 2 * FOCUS_RING);
	}
    }
    return 0;
}

void Crenderbutton (CWidget * wdt)
{
    int w = wdt->width, h = wdt->height;
    int x = 0, y = 0;

    Window win = wdt->winid;

    if (render_focus_ring (wdt))
	x = y = FOCUS_RING;

    Csetcolor (C_FLAT);

    switch (wdt->options) {
    case CBUTTON_PRESSED:
	Crect (win, x, y, w - 4, h - 4);
	Csetcolor (C_BLACK);
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 2, 1);
	break;
    case CBUTTON_HIGHLIGHT:
	Crect (win, x + 1, y + 1, w - 2, h - 2);
	Csetcolor (C_BLACK);
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 1, 0);
	break;
    default:
	Crect (win, x, y, w - 4, h - 4);
	Csetcolor (C_BLACK);
	Crenderbevel (win, x, y, x + w - 1, y + h - 1, 2, 0);
    }
    Cdrawstringxy (win, x + 2 + BUTTON_RELIEF, y + 2 + BUTTON_RELIEF, wdt->label);
    if (wdt->hotkey) {
	char *p;
	int i;
	for (i = 1; wdt->label[i]; i++)
	    if (wdt->label[i - 1] == ' ' && wdt->label[i] == wdt->hotkey) {
		p = wdt->label + i;
		goto done;
	    }
	p = strchr (wdt->label, wdt->hotkey);
      done:;
	if (!p)
	    return;
	x += XTextWidth (CFontStruct, wdt->label, (unsigned long) p - (unsigned long) wdt->label);
	x += 2 + BUTTON_RELIEF + CFontStruct->per_char[wdt->hotkey].lbearing - 1;
	y += TEXT_BASE_LINE + CFontStruct->per_char[wdt->hotkey].descent;
	y += 2 + BUTTON_RELIEF + 1;
	Csetcolor (C_BLACK);
	Cline (win, x, y, x + CFontStruct->per_char[wdt->hotkey].rbearing, y);
    }
}

void Crendertextinput (CWidget * wdt)
{
    int wc, isfocussed = 0;
    int f;
    int w = wdt->width, h = wdt->height;
    Window win;

    win = wdt->winid;
    isfocussed = (win == CGetFocus ());

    Csetcolor (C_WHITE);
    Crect (win, 3, 3, w - 6, h - 6);

    Csetcolor (C_BLACK);

/*This is a little untidy, but it will account for uneven font widths
   without having to think to hard */

    do {
	f = 0;
/*wc is the position of the cursor from the left of the input window */
	wc = CFontStruct->max_bounds.width / 2 + 1 +
	    XTextWidth (CFontStruct, wdt->text +
			wdt->firstcolumn,
			wdt->cursor - wdt->firstcolumn);

	/*now lets make sure the cursor is well within the view */

/*except for when the cursor is at the end of the line */
	if (wdt->cursor == strlen (wdt->text)) {
	    if (wc > w - 3) {
		wdt->firstcolumn++;
		f = 1;
	    }
	} else if (wc > max (w - 20, w * 3 / 4)) {
	    wdt->firstcolumn++;
	    f = 1;
	}
	if (wc < min (20, w / 4)) {
	    wdt->firstcolumn--;
	    f = 1;
	    /*Unless of course we are at the beginning of the string */
	    if (wdt->firstcolumn < 0) {
		wdt->firstcolumn = 0;
		f = 0;
	    }
	}
    } while (f);		/*recalculate if firstcolumn has changed */

/*now draw the visible part of the string */
    Cdrawstringxy (win, 3 + TEXT_INPUT_RELIEF, 3 + TEXT_INPUT_RELIEF, wdt->text + wdt->firstcolumn);

    if (isfocussed) {
	Crenderbevel (win, 0, 0, w - 1, h - 1, 3, 1);	/*most outer border bevel */
    } else {
	Crenderbevel (win, 2, 2, w - 3, h - 3, 1, 1);	/*border bevel */
	Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 0);	/*most outer border bevel */
    }

    Csetcursor (win, wc, 5, w, h - 5, TEXT_INPUT_CURSOR, 0, 0, 0);
}

void Crenderbar (CWidget * wdt)
{
    int w = wdt->width, h = wdt->height;

    Window win = wdt->winid;

    Csetcolor (C_FLAT);
    Cline (win, 1, 1, w - 2, 1);
    Crenderbevel (win, 0, 0, w - 1, h - 1, 1, 1);
}

void Crerendertext (CWidget * wdt)
{
    static Window lastwin = 0;
    static char lasttext[1024] = "";
    Window win = wdt->winid;
    char *q;
    int h = wdt->height;
    int w = wdt->width;
    int i, l = 32000, n, x;

    if (strchr (wdt->text, '\n')) {
	Crendertext (wdt);
	return;
    }
    q = wdt->text;

    n = strlen (q);
    x = TEXT_RELIEF + 1;	/* bevel is 1 */
    if (lastwin == win) {
	int last_width;
	for (i = 0; i < n; i++)
	    if (lasttext[i] != q[i])
		break;
	q += i;
	x += XTextWidth (CFontStruct, wdt->text, i);
	n = strlen (q);
	l = x + XTextWidth (CFontStruct, q, n);
	last_width = XTextWidth (CFontStruct, lasttext, strlen (lasttext)) + TEXT_RELIEF + 1;
	if (l < last_width && l < w - 1) {
	    Csetcolor (C_FLAT);
	    Crect (win, l, 1, w - 1 - l, h - 2);
	}
    }
    Csetcolor (C_BLACK);
    Csetbackcolor (C_FLAT);

    XDrawImageString (CDisplay, win, CGC, FONT_OFFSET_X + x, FONT_OFFSET_Y + TEXT_RELIEF + 1, q, n);

    if (l > w - 1)
	Crenderbevel (win, 0, 0, w - 1, h - 1, 1, 1);
    lastwin = win;
    strcpy (lasttext, wdt->text);
}

void Crendertext (CWidget * wdt)
{
    Window win = wdt->winid;
    char *p, *q = catstrs (wdt->text, "\n", NULL);
    int h = wdt->height, w = wdt->width;

    Csetcolor (C_FLAT);
    Crect (win, 1, 1, w - 2, h - 2);
    Csetcolor (C_BLACK);

    h = 1;	/* bevel */
    while ((p = strchr (q, '\n'))) {
	XDrawString (CDisplay, win,
		     CGC,
		     FONT_OFFSET_X + TEXT_RELIEF + 1,
		     FONT_OFFSET_Y + TEXT_RELIEF + h,
		     q,
		     (long) p - (long) q);
	h += TEXT_PIX_PER_LINE;
	q = p + 1;
    }

    h = wdt->height;
    Crenderbevel (win, 0, 0, w - 1, h - 1, 1, 1);
}

void Crenderwindow (CWidget * wdt)
{
    int w = wdt->width, h = wdt->height;

    Window win = wdt->winid;

    if (wdt->position & CRESIZABLE) {
	Csetcolor (Cwidgetcolor (13));
	Cline (win, w - 4, h - 38, w - 38, h - 4);
	Cline (win, w - 4, h - 27, w - 27, h - 4);
	Cline (win, w - 4, h - 16, w - 16, h - 4);
	Cline (win, w - 4, h - 39, w - 39, h - 4);
	Cline (win, w - 4, h - 28, w - 28, h - 4);
	Cline (win, w - 4, h - 17, w - 17, h - 4);

	Csetcolor (Cwidgetcolor (3));
	Cline (win, w - 4, h - 34, w - 34, h - 4);
	Cline (win, w - 4, h - 23, w - 23, h - 4);
	Cline (win, w - 4, h - 12, w - 12, h - 4);
	Cline (win, w - 4, h - 35, w - 35, h - 4);
	Cline (win, w - 4, h - 24, w - 24, h - 4);
	Cline (win, w - 4, h - 13, w - 13, h - 4);
    }
    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 0);
    if (DefaultRootWindow (CDisplay) != wdt->parentid)
	if (win == CGetFocus ())
	    Crenderbevel (win, 4, 4, w - 5, h - 5, 3, 1);
}

void Crenderprogress (CWidget * wdt)
{
    int w = wdt->width, h = wdt->height;
    int p = wdt->cursor;

    Window win = wdt->winid;

    if (p > 65535)
	p = 65535;
    if (p < 0)
	p = 0;
    Csetcolor (C_FLAT);
    Crect (win, 4 + p * (w - 5) / 65535, 2, (65535 - p) * (w - 5) / 65535, h - 4);
    Csetcolor (Ccolor (3));
    Crect (win, 4, 4, p * (w - 9) / 65535, h - 8);
    Crenderbevel (win, 2, 2, 4 + p * (w - 9) / 65535, h - 3, 2, 0);
    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 1);
}


void Crendersunken (CWidget * wdt)
{
    int w = wdt->width, h = wdt->height;
    Window win = wdt->winid;
    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 1);
}


void Crendervertscrollbar (Window win, int x, int y, int w, int h, int pos, int prop, int flags)
{
    int l = h - 10 * w / 3 - 5;

    Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 1);
    Csetcolor (C_FLAT);
    Crect (win, 2, w + 2 * w / 3 + 2, w - 4, (l - 5) * pos / 65535);
    Crect (win, 2, w + 2 * w / 3 + 3 + l * (prop + pos) / 65535, w - 4, h - 1 - w - 2 * w / 3 - (w + 2 * w / 3 + 4 + l * (prop + pos) / 65535));

    if (flags & 32) {
	Crenderbevel (win, 2, 2, w - 3, w + 1, 2 - ((flags & 15) == 1), 2);
	Crenderbevel (win, 2, w + 2, w - 3, w + 2 * w / 3 + 1, 2 - ((flags & 15) == 2), 2);
	Crenderbevel (win, 2, h - 2 - w, w - 3, h - 3, 2 - ((flags & 15) == 4), 2);
	Crenderbevel (win, 2, h - 2 - w - 2 * w / 3, w - 3, h - 3 - w, 2 - ((flags & 15) == 5), 2);
	Crenderbevel (win, 2, w + 2 * w / 3 + 2 + (l - 5) * pos / 65535, w - 3, w + 2 * w / 3 + 7 + (l - 5) * (prop + pos) / 65535, 2 - ((flags & 15) == 3), 2);
    } else {
	Crenderbevel (win, 2, 2, w - 3, w + 1, 2, 2 | ((flags & 15) == 1));
	Crenderbevel (win, 2, w + 2, w - 3, w + 2 * w / 3 + 1, 2, 2 | ((flags & 15) == 2));
	Crenderbevel (win, 2, h - 2 - w, w - 3, h - 3, 2, 2 | ((flags & 15) == 4));
	Crenderbevel (win, 2, h - 2 - w - 2 * w / 3, w - 3, h - 3 - w, 2, 2 | ((flags & 15) == 5));
	Crenderbevel (win, 2, w + 2 * w / 3 + 2 + (l - 5) * pos / 65535, w - 3, w + 2 * w / 3 + 7 + (l - 5) * (prop + pos) / 65535, 2, 2 | ((flags & 15) == 3));
    }
}

void Crenderscrollbar (CWidget * wdt)
{
    int flags = wdt->options;
    if (wdt->kind == CVERTSCROLL_WIDGET)
	Crendervertscrollbar (wdt->winid,
			      wdt->x, wdt->y,
			      wdt->width, wdt->height,
			      wdt->firstline, wdt->numlines, flags);
}

void Crenderbevel (Window win, int x1, int y1, int x2, int y2, int thick, int sunken)
{
    long cn, cs;
    int i;

    if ((sunken & 2)) {
	Csetcolor (C_FLAT);
	Crect (win, x1 + thick, y1 + thick, x2 - x1 - 2 * thick + 1, y2 - y1 - 2 * thick + 1);
    }
    sunken &= 1;

    cn = sunken ? Cwidgetcolor (4) : Cwidgetcolor (11);
    cs = sunken ? Cwidgetcolor (11) : Cwidgetcolor (4);

    for (i = 0; i < thick; i++) {
	if (sunken || i != thick - 1) {
	    Csetcolor (cn);
	    Cline (win, x1 + i, y1 + i, x1 + i, y2 - 1 - i);
	    Cline (win, x1 + i, y1 + i, x2 - 1 - i, y1 + i);
	}
	if (!sunken || i != 0) {
	    Csetcolor (cs);
	    Cline (win, x2 - i, y2 - i, x1 + i, y2 - i);
	    Cline (win, x2 - i, y1 + i, x2 - i, y2 - i - 1);
	}
	if (sunken) {
	    Csetcolor (Cwidgetcolor (15));
	    XDrawPoint (CDisplay, win, CGC, x2 - i, y2 - i);
	} else {
	    Csetcolor (C_WHITE);
	    XDrawPoint (CDisplay, win, CGC, x1 + i, y1 + i);
	}
    }
    Csetcolor (Cwidgetcolor (13));
/* the edge of the bevel doth shine more brightly: */
    if (sunken) {
	Cline (win, x2 - 1, y2, x1, y2);
	Cline (win, x2, y1, x2, y2 - 1);
    } else {
	i--;
	Cline (win, x1 + i, y1 + i + 1, x1 + i, y2 - 1 - i);
	Cline (win, x1 + i + 1, y1 + i, x2 - 1 - i, y1 + i);
    }
    Csetcolor (C_BLACK);
}

void Cexposepic (CWidget * w);

void Csetwidgetposition (const char *ident, int x, int y)
{
    CWidget *w = Cwidget (ident);

    if (w->winid) {		/*some widgets have no window of there own */
	w->x = x;
	w->y = y;
	XMoveWindow (CDisplay, w->winid, x, y);
    } else {
#ifdef HAVE_PICTURE
	Cexposepic (w);
	w->x = x;
	w->y = y;
	Cexposepic (w);
#endif
    }
}

void Csetwidgetsize (const char *ident, int w, int h)
{
    CWidget *wt = Cwidget (ident);
    if (!wt)
	return;

    if (wt->winid)
	XResizeWindow (CDisplay, wt->winid, w, h);

    if (wt->kind == CWINDOW_WIDGET) {
	CWidget *wdt;
	XEvent e;
	int i;
	memset (&e, 0, sizeof (XEvent));
	e.type = ResizeNotify;
	e.xconfigure.x = w - wt->width;
	e.xconfigure.y = h - wt->height;
	e.xconfigure.width = w;
	e.xconfigure.height = h;
	e.xconfigure.display = CDisplay;
	i = Cfindfirstchildof (wt->winid);
	while (i) {
	    wdt = CW (i);
	    e.xany.window = wdt->winid;
	    CSendEvent (&e);
	    i = Cfindnextchildof (wdt->parentid, wdt->winid);
	}
    }
    wt->width = w;
    wt->height = h;
}

void Csetsizehintpos (const char *ident)
{
    int x, y;
    Cgethintlimits (&x, &y);
    Csetwidgetsize (ident, x + 2, y + 2);
}

static inline int inbounds (x, y, x1, y1, x2, y2)
{
    if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
	return 1;
    else
	return 0;
}

/*
   Which scrollbar button was pressed?
 */
int Cscrollwhereis (int bx, int by, CWidget * wdt)
{
    int w = wdt->width;
    int h = wdt->height;
    int pos = wdt->firstline;
    int prop = wdt->numlines;
    int l = h - 10 * w / 3 - 5;

    if (inbounds (bx, by, 2, 2, w - 3, w + 1))
	return 1;
    if (inbounds (bx, by, 2, w + 2, w - 3, w + 2 * w / 3 + 1))
	return 2;
    if (inbounds (bx, by, 2, h - 2 - w, w - 3, h - 3))
	return 4;
    if (inbounds (bx, by, 2, h - 2 - w - 2 * w / 3, w - 3, h - 3 - w))
	return 5;
    if (inbounds (bx, by, 2, w + 2 * w / 3 + 2 + (l - 5) * pos / 65535, w - 3, w + 2 * w / 3 + 7 + (l - 5) * (prop + pos) / 65535))
	return 3;
    return 0;
}
