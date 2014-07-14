/* coolwidget.h - main header file
   Copyright (C) 1996, 1997 Paul Sheer

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

#ifndef COOL_WIDGET_H
#define COOL_WIDGET_H

#include <config.h>
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "global.h"
#include "vgagl.h"
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "drawings.h"
#include "3dkit.h"

#ifdef HAVE_MAD
#include "mad.h"
#endif

typedef struct initialisation {
    int x, y, width, height;
    int lines, columns, width_plus, height_plus;
    char *name;
    char *display, *geometry, *font, *bg;
    char *fg_red, *fg_green, *fg_blue;	/* string doubles */
#define CINIT_OPTION_USE_GREY 1
#define CINIT_OPTION_VERBOSE 2
    unsigned long options;
} CInitData;


#define CEdit WEdit

extern int errno;

/* edit this */
/* #define quad_t unsigned long */
#define quad_t unsigned int
#define word unsigned short
#define byte unsigned char

#define CMAGIC_BEGIN 0x9e065f4d
#define CMAGIC_END 0xd4f560e9

#define TEXT_SETCOL 1
#define TEXT_SETLINE 2
#define TEXT_SETPOS 3
#define TEXT_SET_CURSOR_LINE 4

#define FOCUS_RING 6

/* These are all the widget kinds (the kind member of the widget structure) */
enum { \
 CNOT_A_WIDGET, CBUTTON_WIDGET, CWINDOW_WIDGET, CBAR_WIDGET, CSUNKEN_WIDGET, \
 CVERTSCROLL_WIDGET, CHORSCROLL_WIDGET, CTEXTINPUT_WIDGET, CTEXTBOX_WIDGET, \
 CTEXT_WIDGET, CBWIMAGE_WIDGET, CSPREAD_WIDGET, CPROGRESS_WIDGET, \
 CBITMAP_WIDGET, CBITMAPBUTTON_WIDGET, CSWITCH_WIDGET, C8BITIMAGE_WIDGET, \
 CTHREED_WIDGET, CPICTURE_WIDGET, CEDITOR_WIDGET, CMENU_WIDGET, CMENU_BUTTON_WIDGET, \
 CALARM_WIDGET
};

/*
   Here are some addition events that you may recieve or send using
   CNextEvent instead of XNextEvent. (LASTEvent = 35)
 */
/* this comes every 1/7 of a second */
#define AlarmEvent (LASTEvent + 1000)
/* This you won't recieve ---  it is used for joining small expose regions together */
#define InternalExpose (LASTEvent + 1001)
/* This send this event to the editor to force the editor widget to execute a command */
#define EditorCommand (LASTEvent + 1002)
/* send to widgets when a window resizes */
#define ResizeNotify (LASTEvent + 1003)
/* comes every 1/100 of a second (see initapp.c for correct amount) */
#define TickEvent (LASTEvent + 1004)
/* used for generating repeats on keys */ /* NOT USED */
#define InternalKeyPress (LASTEvent + 1005)
/* used for sending internal FocusOut events */
#define InternalFocusOut (LASTEvent + 1006)

/* Library is limited to this number of widgets at once */
#define MAX_NUMBER_OF_WIDGETS 1024

/* one of the (long int) "CoolBlue" colors (0-15) that make up the windows, buttons etc */
#define Cwidgetcolor(i) Cpixel[(i)]
/* one of a 3x3x3 (RxGxB) color palette. eg R=2, G=1, B=0 is Ccolor(19). */
#define Ccolor(i) Cpixel[(i) + 16]
/* 0-64 grey levels (not supprted unless specified in config.h) */
#define Cgrey(i) Cpixel[(i) + 43]

/* draw a line in the window d */
#define Cline(d, x, y, w, h)  XDrawLine(CDisplay, d, CGC, x, y, w, h)
/* rectangle */
#define Crect(d, x, y, w, h)  XFillRectangle(CDisplay, d, CGC, x, y, w, h)
/* set the foreground color */
#define Csetcolor(c) XSetForeground(CDisplay, CGC, (c))
/* set the background color */
#define Csetbackcolor(c) XSetBackground(CDisplay, CGC, (c))
/* width of a string in pixels in the basic font */
#define Cstringwidth(s) XTextWidth (CFontStruct, s, strlen(s))

#define CSCREEN_ASPECT 1.333

/* some standard colors */
/* the color of the "flat" of a window */
#define C_FLAT Cwidgetcolor(9)
#define C_WHITE Ccolor(26)
#define C_BLACK Ccolor(0)

/* SelectInput for various types of widgets */
#define INPUT_EXPOSE (KeyPressMask | ExposureMask | StructureNotifyMask | VisibilityChangeMask)
#define INPUT_KEY (ExposureMask | ButtonPressMask | ButtonReleaseMask | \
			KeyPressMask | ButtonMotionMask | FocusChangeMask | StructureNotifyMask | PropertyChangeMask | EnterWindowMask)
#define INPUT_MOTION (KeyPressMask | KeyReleaseMask | ExposureMask | ButtonPressMask | \
			ButtonReleaseMask | PointerMotionMask | ButtonMotionMask | EnterWindowMask)
#define INPUT_BUTTON (INPUT_KEY | EnterWindowMask | LeaveWindowMask)

/* internal */
#define MAPPED 1
#define FOCUS_WHEN_MAPPED 2

/* menu callback function */
typedef void (*callfn) ();

struct menu_item {
    char *text;
    KeySym hot_key;
    callfn call_back;
};


#define ClassOfVisual(v) ((v)->class)

/* font dimensions */
#define TEXT_BASE_LINE (CFontStruct->per_char['H'].ascent + CFontStruct->descent)
#define TEXT_PIX_PER_LINE (TEXT_BASE_LINE + CFontStruct->descent)

#define TEXT_M_WIDTH CMean_font_width

/* spacing between widgets in pixels */
#define WIDGET_SPACING 6

/* spacing between the bevel and the text of the text widget */
#define TEXT_RELIEF 3
#define TEXT_INPUT_RELIEF 1
#define BUTTON_RELIEF 2

/* auto widget sizing (use instaed of width or height to work out the width
    of widgets that have text in them) */
#define AUTO_WIDTH		-32000
#define AUTO_HEIGHT		-32001
#define AUTO_SIZE		AUTO_WIDTH, AUTO_HEIGHT

/* font offsets for drawing */
#define FONT_OFFSET_X (0)
#define IN_FONT_OFFSET_Y (0)

/* if this gets changed, the cursor rendering (upside down "L") in editdraw and
    cooledit.c must be adjusted so that the cursor is erased properly */
#define FONT_OFFSET_Y (CFontStruct->per_char['H'].ascent + CFontStruct->descent + IN_FONT_OFFSET_Y)

/*
   A reduced event. This structure is returned by CNextEvent, and
   contains most of the things you may need. Anything more is contained
   in XEvent.
*/
typedef struct {
/* widget's identification string */
    char *ident;
    int i;

/* data */
    int x;
    int y;
    int xt;
    int yt;

    Window window;

/* enumerated above */
    int kind;
    int type;

/* if a key was pressed, this is the KeySym */
    int key;
    int xlat;
    Time time;
    unsigned int button;

/* 1 for a double click */
    int double_click;
    unsigned int state;

/* if text was returned by the event */
    char *text;

/* if the event was already handled by a callback routine */
    char handled;

/* if the event coused an editor command */
    long command;  /* editor commands */
} CEvent;


/* This is the structure of a widget: */
struct cool_widget {
    char ident[33];		/*unique identifying string given by user */
/* for debugging */
    quad_t magic_begin;

/* essentials */
    Window winid;		/*X integer window id of widget */
    Window parentid;		/*parent window of window */
    int (*eh) (struct cool_widget *, XEvent *, CEvent *);	/* internal event handler */
    int (*callback) (struct cool_widget *, XEvent *, CEvent *);	/* user event handler */
    void (*destroy) (struct cool_widget *);	/*only a few widgets need special distruction */

/* basics */
    int width, height;		/*of window --- just to save looking it up */
    int x, y;			/*position in parent --- top left corner */
    int kind;			/*one of C??????_WIDGET above */
    char disabled;		/*displayed, but not functioning */
    char takes_focus;		/*can this widget take input focus? */
    char mapped;

/* data */
    char *label;		/*text that gets drawn into button */
    char *graphic;		/*Possibly a bitmap to go onto the button */
    int *tab;			/*columns for spreadsheat widget */
    char *text;			/*text goes into textbox textinput and text widgets */
    char *headings;		/*headings of spreadsheet columns */
    GraphicsContext *gl_graphicscontext;    /*for svgalib image widgets */
    XImage *ximage;		/*for X images picture widgets */
    Pixmap pixmap;		/*for pixmaps */
    CPicture *pic;		/*for lines, circles, rects and arcs.*/
    TD_Solid *solid;
    struct editor_widget *editor;
    struct menu_item *menu;

/* Positions. What they are used for depends on the kind of widget. See coolwidget.c for an explanation */
    long cursor;
    long column;
    long numlines;
    long firstline;
    long current;
    long firstcolumn;
    long textlength;
    long mark1, mark2;
    long search_start;
    int search_len;
    Window menu_focus_return;

/* settings */
    long options;
#define CBUTTON_HIGHLIGHT 1
#define CBUTTON_PRESSED 2
#define MAN_PAGE 4
#define TEXT_WRAP 8
#define TEXT_NO_KEYS 16
#define TEXT_BOX_NO_CURSOR 32
#define TEXT_FILES 64
#define BAR_WINDOW_WIDTH 128
#define TEXT_CENTRED 256
    char position;		/*one of three: */
#define CALWAYS_ON_TOP			1	/*always raised */
#define CALWAYS_ON_BOTTOM		2	/*always lowered */
#define CFIXED_POSITION			4	/*cannot be moved */
#define CRESIZABLE			8	/*can be resized */

    struct cool_widget * scrollbar;		/*links to other widgets */
    struct cool_widget * textbox;
    struct cool_widget * textinput;
    struct cool_widget * droppedmenu;
    char keypressed;
    char hotkey;
    unsigned long fg;		/*colors */
    unsigned long bg;

/* user structure. you can put addition data that you might need in here */
    void *user;

/* for debugging */
    quad_t magic_end;
};

typedef struct cool_widget CWidget;

/* you may want to use these */
#define CTextOf(w) ((w)->text)
#define CLabelOf(w) ((w)->label)
#define CUserOf(w) ((w)->user)
#define CHeightOf(w) ((w)->height)
#define CWidthOf(w) ((w)->width)
#define CXof(w) ((w)->x)
#define CYof(w) ((w)->y)
#define CWindowOf(w) ((w)->winid)
#define CParentOf(w) ((w)->parentid)
#define CIdentOf(w) ((w)->ident)

/* internal */
typedef struct disabled_state {
    quad_t state[(MAX_NUMBER_OF_WIDGETS + 31) / 32];
    quad_t mask[(MAX_NUMBER_OF_WIDGETS + 31) / 32];
} CState;

/*
   The only global variables for the widgets. This is the actual array
   of pointers that holds the malloced widget structures
 */

#ifdef COOL_WIDGET_C
int CLastwidget;		/* gives length of widget list */
CWidget *widget[MAX_NUMBER_OF_WIDGETS];   /* first widget is at 1*/
#else
extern int CLastwidget;
extern CWidget *widget[MAX_NUMBER_OF_WIDGETS];
#endif

/* CW(i) used to return a pointer to the widget i */
#define CW(i) widget[i]

#define Cgettext(i) CW(Ci(i))->text

/* returns a pointer (of type CWidget) to the widget called ident */
CWidget * Cwidget(const char *ident);

/*Ci'ndex, returns the index of this widget */
int Ci (const char *ident);


/* Initialise. Opens connection to the X display, processing -display -font, and -geom args
   sets up GC's, visual's and so on */
void Cinit (CInitData *config);

/* Call when app is done. This undraws all widgets, free's all data
   and closes the connection to the X display */
void CShutdown (void);

/* Prints an error to stderr, or to a window if one can be created, then exits */
void Cerror (const char *fmt,...);

/* Normal malloc with check for 0 return */
void *Cmalloc (size_t size);
void *CDebugMalloc(size_t x, int line, const char *file);

/* Draw a panel onto which widgets will be drawn */
Window Cdrawwindow (const char *identifier, Window parent, int x, int y,
		      int width, int height, const char *label);

/* Draw a panel with a heading and a seperator line. The position below the
seperator line is recorded in h, start drawing in the window from there. */
Window Cdrawheadedwindow (const char *identifier, Window parent, int x, int y,
		      int width, int height, const char *label);

/* Draw a button */
CWidget * Cdrawbutton (const char *identifier, Window parent, int x, int y,
		      int width, int height, const char *label);

/* Draw a button with a bitmap on it, (see dialog.c for example) */
CWidget * Cdrawbitmapbutton (const char *identifier, Window parent, int x, int y,
		      int width, int height, unsigned long fg, unsigned long bg, const unsigned char data[]);

/* Draws a toggle switch, pass on as the default setting */
CWidget * Cdrawswitch (const char *identifier, Window parent, int x, int y,
	      unsigned long fg, unsigned long bg, int on);

/* Draw a text input widget */
CWidget * Cdrawtextinput (const char *identifier, Window parent, int x, int y,
		  int width, int height, int maxlen, const char *string);

/* Draws a scrollable textbox, with its scrollbar. text is newline seperate */
CWidget * Cdrawtextbox (const char *identifier, Window parent, int x, int y,
	  int width, int height, int line, int column, const char *text, long options);
CWidget *Cdrawmanpage (const char *identifier, Window parent, int x, int y,
	   int width, int height, int line, int column, const char *text);
/* Change the text of the textbox. If preserve is 1, then the position in the text is not altered */
CWidget * Credrawtextbox (const char *identifier, const char *text, int preserve);

/* Draws a thin horizontal raised ridge */
CWidget *Cdrawbar (Window parent, int x, int y, int w, int options);

/* Vertical scroll bar */
CWidget * Cdrawvertscrollbar (const char *identifier, Window parent, int x, int y,
			     int length, int width, int pos, int prop);

/* Draws one or more lines of text (seperated by newlines) in a sunken panel. Use like printf() */
CWidget *Cdrawtext (const char *identifier, Window parent, int x, int y, const char *fmt,...);
/* Will replace the text of an existing text widget. Unlike other widgets, multiple text widgets can have the same ident */
CWidget *Credrawtext (const char *identifier, const char *fmt,...);

/* Draws a file browser and returns a filename, file is the default file name */
char *Cgetfile (Window parent, int x, int y,
		const char *dir, const char *file, const char *label);
char *Cgetdirectory (Window parent, int x, int y,
		     const char *dir, const char *file, const char *label);
char *Cgetsavefile (Window parent, int x, int y,
		    const char *dir, const char *file, const char *label);
char *Cgetloadfile (Window parent, int x, int y,
		    const char *dir, const char *file, const char *label);

/* Draws a directory browser and returns immediately */
void Cdrawbrowser (const char *ident, Window parent, int x, int y,
		   const char *dir, const char *file, const char *label);


/* Draws a simple spreadsheat widget (not supprted) */
CWidget * Cdrawspreadsheet (const char *ident, Window parent, int x, int y, int w, int h, const char *spreadtext, const char *heading, int *columns);

/* Draws a full blown text editor, scrollbar and status line */
CWidget * Cdraweditor (const char *identifier, Window parent, int x, int y,
	   int width, int height, const char *text, const char *filename,
		      const char *starting_directory);

/* Draws a menu button that may be pulled down if clicked on */
CWidget *Cdrawmenubutton (const char *ident, Window parent, Window focus_return,
	int x, int y, int width, int height, int num_items, const char *label,
/* this is a menu item: */
	const char *text, int hot_key, callfn call_back,...);

/* Draws menu buttons for the editor. focus_return is where focus goes to if you escape from a menu */
void CDrawEditMenuButtons (const char *ident, Window parent, Window focus_return, int x, int y);
void Caddmenuitem (const char *ident, const char *text, int hot_key, callfn call_back);
void Cremovemenuitem (const char *ident, const char *text); /**** UNTESTED ****/

/* Draws a bitmap inside a sunken window */
CWidget * Cdrawbitmap (const char *identifier, Window parent, int x, int y,
		      int width, int height, unsigned long fg, unsigned long bg, const unsigned char *data);

/* Draws a black and white picture 1 byte per pixel contiguous data */
CWidget * Cdrawbwimage (const char *identifier, Window parent, int x, int y,
		       int width, int height, unsigned char *data);

/* A window with inward bevels */
CWidget * Cdrawsunkenpanel (const char *identifier, Window parent, int x, int y,
			   int width, int height, const char *label);

/* Draw a progress bar */
CWidget * Cdrawprogress (const char *identifier, Window parent, int x, int y,
			int width, int height, int p);

/* Draws a picture, containing nothing. Allows lines, rectangles etc to
be drawn into the picture */
CWidget *Cdrawpicture (const char *identifier, Window parent, int x, int y,
		int max_num_elements);


/* Destroy a widget. This will destroy all descendent widgets recursively */
int Cundrawwidget (const char *identifier);

/* Returns the widget's window */
Window Cgetwidgetwinfromident (const char *identifier);

/* Used internally, or for creating you own widgets, see coolwidget.c */
CWidget * Csetupwidget (const char *identifier, Window parent, int x, int y,
	     int width, int height, int kindofwidget, unsigned long input,
	     unsigned long bgcolor, int takes_focus);

/* For resizing and reposition a widget */
void Csetwidgetsize (const char *ident, int w, int h);
void Csetwidgetposition (const char *ident, int x, int y);

/* Forces the a widget to be entirely redrawn */
void Cexpose (const char *ident);

/* Sends an expose event to a window */
void Cexposewindowarea (Window win, int count, int x, int y, int w, int h);

/* Sends an event to the coolwidget queue. Use instead of XSendEvent */
int CSendEvent (XEvent *e);


/* add a callback to a widget. Will be called if anything relevent happens
to the widget. callback must return 1 if they handles a key press */
void Caddcallback(const char *ident, int (*callback) (CWidget *, XEvent *, CEvent *));

/* send the text box a command (such as XK_Left or XK_Down to scroll) */
int Ctextboxcursormove (CWidget *w, KeySym key);

/* forces all windows set to CALWAYS_NO_TOP (see above) to be raised */
void Craisewindows (void);
/* same for ALWAYS_ON_BOTTOM, call these after raising or lowering
   a window, to keep the "underneath" windows where they should be
   (eg the coolwidget logo in the top left of the screen */
void Clowerwindows (void);

/* Various rendering routines called internally */
void Crenderbevel (Window win, int x1, int y1, int x2, int y2, int thick, int sunken);
long Crendertextbox (CWidget *w, int redrawall);
void Crenderbitmapbutton (CWidget *w, int state);
void Crenderbwimage (CWidget *w, int x, int y, int rendw, int rendh);
int render_focus_ring (CWidget * wdt);

/* used internally to process filebrowser events */
char *Chandlebrowser (const char *identifier, CEvent * cwevent, int options);

/* internal */
void Credrawspreadinput (CWidget *w, int dump);

/*
   Processes all events remaining in the queue. If any event matches
   cwevent.ident then it copies it into xevent and cwevent.
   It does NOT wait if there are no events in the queue.
 */
int Ccheckifevent (XEvent * xevent, CEvent * cwevent);

/*
   The hinge of this whole library. This handles widget events and calls
   the callback routines. It must go in the main loop of a program.
*/
void CNextEvent (XEvent * xevent, CEvent * cwevent);

/* Process all events in the event queue before returning. Call this periodically
during functions that take a long time (several seconds) to keep the display clean */
void Cclearevents ();

/* returns 1 if there is a mouse event on the queue, but does nothing */
int CMousePending(const char *ident);

/* Any events left? */
int CPending ();

/* Any events left on coolwidgets own event queue? */
int CQueueSize ();

/* Do not use the libc sleep command. This sleeps for t seconds,
   resolution is 1/50 of a second */
void CSleep (double t);

/* Do not use the libc system command */
int Csystem (const char * string);

/* Destroy all widgets */
void Cundrawall ();

/* All widgets may be either enabled or disabled, meaning they either
   recieve input from the mouse and keyboard, or not. This backs up the
   state of all widgets into the structure CState. See dialog.c for an
   example */
void CBackupState (CState * s);
/* This restore the state from the structure */
void CRestoreState (CState * s);

/* Disable a widget. ident may be a regular expression. */
void CDisable (const char *ident);
/* Enable */
void CEnable (const char *ident);

/* set the focus to a widget */
void CFocus (CWidget *w);

/* set the focus to a window */
void CFocusWindow (Window win);

/* get the current focus */
Window CGetFocus(void);

/* pull up or down a menu */
void CPullDown (CWidget * button);
void CPullUp (CWidget * button);

/* set the editor that editmenu will send commands to */
void CSetEditMenu(const char *ident);
void CEditMenuCommand (int i);
CWidget * CGetEditMenu(void);
void CEditMenuKey (KeySym i, int state);

/* internal */
int CSendExpose (Window win, int x, int y, int w, int h);

/* Set the position of the text in the text-box, see coolwidget.c */
long Csettextboxpos (CWidget *wdt, int which, long p);

/************* the rest is not properly documented **************/

int Ccheck ();

int Cistimerevent (XEvent * xevent);

void Cresolvebutton (XEvent * xevent, CEvent * cwevent);


void Ctextsize (int *w, int *h, const char *str);


#ifdef DRAWINGS_C
CWidget * CWdrawtarget;
#else
extern CWidget * CWdrawtarget;
#endif

void cw_destroypicture (CWidget * w);


Window Cdrawfilebrowser (const char *identifier, Window parent, int x, int y,
		   const char *dir, const char *file, const char *label);


/* internal */
void Ctoggle_cursor ();




int regexp_match (char *pattern, char *string, int match_type);


CWidget *CFindFirstDescendent (Window win);
int Cfindnextchildof (Window win, Window child);
int Cfindpreviouschildof (Window win, Window child);
int Cfindfirstchildof (Window win);
int Cfindlastchildof (Window win);
int CWidgetOf (Window win);

/* there are two cursor types */
#define TEXT_INPUT_CURSOR 1
#define EDITOR_CURSOR 2

/* set the cursor position (internal) */
void Csetcursor(Window win, int x, int y, int w, int h, int type, int chr, unsigned long fg, unsigned long bg);

/* translates a key press to a keysym */
KeySym CKeySym (XEvent *e);
/* some by converts to a short with upper bits representing the state */
short CKeySymMod (XEvent * e);

/* gets a widgets position relative to some ancestor widget */ 
void CGetWindowPosition (Window win, Window ancestor, int *x_return, int *y_return);

CWidget *CNextFocus (CWidget * w);
CWidget *CPreviousFocus (CWidget * w);
CWidget *CChildFocus (CWidget * w);

void Cresethintpos (int x, int y);
void Csethintpos (int x, int y);
void Cgethintpos (int *x, int *y);
void Cgethintlimits (int *max_x, int *max_y);

void CEnableAlarm();
void CDisableAlarm();

int CWindowPending (Window w);

double my_log (double x);
double my_sqrt (double x);
double my_pow (double x, double y);

#ifdef HAVE_MAD
#define Cmalloc(x) malloc(x)
#endif

#ifndef HAVE_MAD
#define Cmalloc(x) CDebugMalloc(x, __LINE__, __FILE__)
#endif

/* #define FOCUS_DEBUG */

#ifdef FOCUS_DEBUG
#	define CFocus(x) CFocusDebug(x,__LINE__,__FILE__)
#	define CFocusWindow(x) CFocusWindowDebug(x,__LINE__,__FILE__)
#else
#	define CFocus(x) CFocusNormal(x)
#	define CFocusWindow(x) CFocusWindowNormal(x)
#endif

void CFocusDebug (CWidget *w, int line, char *file);
void CFocusNormal (CWidget *w);
void CFocusWindowDebug (Window w, int line, char *file);
void CFocusWindowNormal (Window w);

void CDndInit (void);
void CDrag (Window from, int data_type, unsigned char *data, int length, unsigned long pointer_state);
int CGetDrop (XEvent * xe, unsigned char **data, unsigned long *size, int *x, int *y);
void CDropAcknowledge (XEvent *xe);
int CIsDropAcknowledge (XEvent *xe, unsigned int *state);
Window CQueryPointer (int *x, int *y, unsigned int *mask);
char *CDndDirectory (void);
void CSetDndDirectory (char *d);
char *CDndFileList (char *t, int *l, int *num_files);

void Csetsizehintpos (const char *ident);

Window my_XmuClientWindow (Display *dpy, Window win);

#define DndNotDnd	-1
#define DndUnknown	0
#define DndRawData	1
#define DndFile		2
#define	DndFiles	3
#define	DndText		4
#define DndDir		5
#define DndLink		6
#define DndExe		7
#define DndURL          8
#define DndMIME         9

#define DndEND		10

#include "dirtools.h"
#include "edit.h"
#include "editcmddef.h"
#include "imagewidget.h"
#include "dialog.h"

#endif

