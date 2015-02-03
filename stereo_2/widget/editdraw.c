/* editor text drawing.

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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <config.h>
#include "edit.h"

#define MAX_LINE_LEN 1024

#ifndef MIDNIGHT
#include "app_glob.c"
#include "dirtools.h"
#include "coollocal.h"
#else
#include "src/mad.h"
#endif


static void status_string (WEdit * edit, char *s, int w)
{
    int i;
    char t[160];		/* 160 just to be sure */
/* The field lengths just prevents the status line from shortening to much */
    sprintf (t, "[%c%c%c%c] %2ld:%3ld+%2ld=%3ld/%3ld - *%-4ld/%4ldb=%3d",
	     edit->mark1 != edit->mark2 ? 'B' : '-',
	     edit->modified ? 'M' : '-', edit->macro_i < 0 ? '-' : 'R',
	     edit->overwrite == 0 ? '-' : 'O',
	     edit->curs_col, edit->start_line + 1, edit->curs_row,
	     edit->curs_line + 1, edit->total_lines + 1, edit->curs1,
	     edit->last_byte, edit->curs1 < edit->last_byte
	     ? edit_get_byte (edit, edit->curs1) : -1);
    sprintf (s, "%.*s", w + 1, t);
    i = strlen (s);
    s[i] = ' ';
    i = w;
    do {
	if (strchr (" +-*=/:b", s[i]))	/* chop off the last word/number */
	    break;
	s[i] = ' ';
    } while (i--);
    s[i] = ' ';
    s[w] = 0;
}


#ifdef MIDNIGHT

/* how to get as much onto the status line as is numerically possible :) */
void edit_status (WEdit * edit)
{
    int w, i, t;
    char *s;
    w = edit->widget.cols - (edit->have_frame * 2);
    s = malloc (w + 15);
    if (w < 4)
	w = 4;
    memset (s, ' ', w);
    attrset (SELECTED_COLOR);
    if (w > 4) {
	widget_move (edit, edit->have_frame, edit->have_frame);
	i = w > 24 ? 18 : w - 6;
	i = i < 13 ? 13 : i;
	sprintf (s, "%s", name_trunc (edit->filename ? edit->filename : "", i));
	i += strlen (s);
	s[strlen (s)] = ' ';
	t = w - 20;
	if (t < 0)
	    t = 0;
	status_string (edit, s + 20, t);
    } else {
	s[w] = 0;
    }
    printw ("%.*s", w, s);

    attrset (NORMAL_COLOR);
    free (s);
}

#else

void edit_status (WEdit * edit)
{
    int w, i, t;
    char *s;
    w = edit->num_widget_columns - 1;
    if (w < 0)
	w = 0;
/*    if (w > 70)
   w = 70; */
    s = malloc (3 * w / 2 + 80);
    memset (s, ' ', w);
    if (w > 1) {
	i = w > 24 ? 18 : w - 6;
	i = i < 13 ? 13 : i;
	sprintf (s, "%s", name_trunc (edit->filename ? edit->filename : "", i));
	i = strlen (s);
	s[i] = ' ';
	t = w - i - 2;
	if (t < 0)
	    t = 0;
	status_string (edit, s + i + 2, t);
    }
    s[w] = 0;
    Credrawtext (catstrs (edit->widget->ident, ".text", 0), s);
    free (s);
}


#endif


/* boolean */
int cursor_in_screen (WEdit * edit, long row)
{
    if (row < 0 || row >= edit->num_widget_lines)
	return 0;
    else
	return 1;
}

/* returns rows from the first displayed line to the cursor */
int cursor_from_display_top (WEdit * edit)
{
    if (edit->curs1 < edit->start_display)
	return -edit_move_forward (edit, edit->curs1, 0, edit->start_display);
    else
	return edit_move_forward (edit, edit->start_display, 0, edit->curs1);
}

/* returns how far the cursor is out of the screen */
int cursor_out_of_screen (WEdit * edit)
{
    int row = cursor_from_display_top (edit);
    if (row >= edit->num_widget_lines)
	return row - edit->num_widget_lines + 1;
    if (row < 0)
	return row;
    return 0;
}


/* this scrolls the text so that cursor is on the screen */
void edit_scroll_screen_over_cursor (WEdit * edit)
{
    int p = edit_get_col (edit);
    int outby;
    update_curs_row (edit);
    outby = p + edit->start_col - edit->num_widget_columns + 1 + EDIT_RIGHT_EXTREME;
    if (outby > 0)
	edit_scroll_right (edit, outby);
    outby = EDIT_LEFT_EXTREME - p - edit->start_col;
    if (outby > 0)
	edit_scroll_left (edit, outby);
    p = edit->curs_row;
    outby = p - edit->num_widget_lines + 1 + EDIT_BOTTOM_EXTREME;
    if (outby > 0)
	edit_scroll_downward (edit, outby);
    outby = EDIT_TOP_EXTREME - p;
    if (outby > 0)
	edit_scroll_upward (edit, outby);
    update_curs_row (edit);
}



#ifndef MIDNIGHT

#define CACHE_WIDTH 256
#define CACHE_HEIGHT 128

int EditExposeRedraw = 0;

/* background colors: marked is refers to mouse highlighting, highlighted refers to a found string. */
unsigned long edit_abnormal_color, edit_marked_abnormal_color;
unsigned long edit_highlighted_color, edit_marked_color;
unsigned long edit_normal_background_color;

/* foreground colors */
unsigned long edit_normal_foreground_color, edit_bold_color;
unsigned long edit_italic_color;

/* cursor color */
unsigned long edit_cursor_color;

void edit_set_foreground_colors (unsigned long normal, unsigned long bold, unsigned long italic)
{
    edit_normal_foreground_color = normal;
    edit_bold_color = bold;
    edit_italic_color = italic;
}

void edit_set_background_colors (unsigned long normal, unsigned long abnormal, unsigned long marked, unsigned long marked_abnormal, unsigned long highlighted)
{
    edit_abnormal_color = abnormal;
    edit_marked_abnormal_color = marked_abnormal;
    edit_marked_color = marked;
    edit_highlighted_color = highlighted;
    edit_normal_background_color = normal;
}

void edit_set_cursor_color (unsigned long c)
{
    edit_cursor_color = c;
}

void xprint_to_widget (Window win, long row, int start_col, float start_col_real, long end_col, unsigned short line[])
{
    static unsigned short cache[CACHE_WIDTH][CACHE_HEIGHT];
    static Window last = 0;
    int M_width = TEXT_M_WIDTH;
    int M_height = TEXT_PIX_PER_LINE;
    int x, y = row * M_height + EDIT_TEXT_VERTICAL_OFFSET;
    unsigned short *p;
    static char text[MAX_LINE_LEN];
    unsigned int style;
    int i = 0;
    unsigned long bg, fg;
    int cursor_found = 0;

    if (row >= CACHE_HEIGHT || end_col >= CACHE_WIDTH)	/* this will look ugly to users who want gian-normou-huge edit windows */
	return;

/* find the end of the line and, */
    while (line[i++]);

/* discard the part of the line we don't need to draw */
    line += (start_col - (int) start_col_real);
    i -= start_col - (int) start_col_real + 1;
    if (i <= 0)
	i = 0;

/* if its not the same window, reset the screen rememberer */
    if (last != win) {
	last = win;
	memset (cache, -1, CACHE_WIDTH * CACHE_HEIGHT * sizeof (short));
    }
/* fill from the end of the line with spaces up to the required width */
    while (i <= end_col - start_col && i < CACHE_WIDTH)
	line[i++] = ' ';
    line[i] = 0;

    if (!EditExposeRedraw) {	/* except for exposes (when we don't know what was erased) */
/* discard part of the line that is the same on the screen, where the cache remembers it: */
	for (i = 0, x = start_col; x <= end_col; x++, i++)
	    if (cache[x][row] != line[i] || cache[x][row] & MOD_CURSOR * 256)
		break;

	for (x = end_col; x >= start_col && x >= i; line[(x--) - start_col] = 0)
	    if (cache[x][row] != line[x - start_col] || cache[x][row] & MOD_CURSOR * 256)
		break;
	start_col += i;
	line += i;
    }
    x = start_col * M_width + EDIT_TEXT_HORIZONTAL_OFFSET;	/* pixel pos of line start */
    p = line;

    while (*p) {
	i = 0;
	style = (*p) & 0xFF00;
	if (style & (MOD_ABNORMAL * 256)) {
	    bg = edit_abnormal_color;
	    if (style & (MOD_MARKED * 256))
		bg = edit_marked_abnormal_color;
	} else if (style & (MOD_HIGHLIGHTED * 256))
	    bg = edit_highlighted_color;
	else if (style & (MOD_MARKED * 256))
	    bg = edit_marked_color;
	else
	    bg = edit_normal_background_color;
	fg = edit_normal_foreground_color;
	if (style & (MOD_BOLD * 256))
	    fg = edit_bold_color;
	if (style & (MOD_ITALIC * 256))
	    fg = edit_italic_color;
/* print all characters with the same style (i.e. same foreground and background color) at once: */
	while (style == ((*p) & 0xFF00) && *p) {
	    text[i++] = (char) *p;
	    cache[start_col++][row] = *(p++);
	}
	Csetbackcolor (bg);
	Csetcolor (fg);
	XDrawImageString (CDisplay, win, CGC, x + FONT_OFFSET_X, y + FONT_OFFSET_Y, text, i);
/* if we printed a cursor: */
	if (style & (MOD_CURSOR * 256)) {
	    Csetcolor (edit_cursor_color);
	    Cline (win, x, y + CFontStruct->descent + IN_FONT_OFFSET_Y,
		   x, y + M_height - 1 + IN_FONT_OFFSET_Y);	/* non focussed cursor form */
	    Cline (win, x + 1, y + CFontStruct->descent + IN_FONT_OFFSET_Y,
	    x + M_width - 1, y + CFontStruct->descent + IN_FONT_OFFSET_Y);
	    cursor_found = 1;
	    Csetcursor (win, x, y, M_width, M_height, EDITOR_CURSOR, *text, bg, fg);	/* widget library's flashing cursor */
	}
	x += M_width * i;
    }
}


#else

#define BOLD_COLOR        MARKED_COLOR
#define UNDERLINE_COLOR   VIEW_UNDERLINED_COLOR
#define MARK_COLOR        SELECTED_COLOR
#define DEF_COLOR         NORMAL_COLOR

static void set_color (int font)
{
    attrset (font);
}

#define edit_move(x,y) widget_move(edit, y, x);

static void print_to_widget (WEdit * edit, long row, int start_col, float start_col_real, long end_col, unsigned short line[])
{
    int x = (float) start_col_real + EDIT_TEXT_HORIZONTAL_OFFSET;
    int x1 = start_col + EDIT_TEXT_HORIZONTAL_OFFSET;
    int y = row + EDIT_TEXT_VERTICAL_OFFSET;

    set_color (DEF_COLOR);
    edit_move (x1, y);
    hline (' ', end_col + 1 - EDIT_TEXT_HORIZONTAL_OFFSET - x1);

    edit_move (x + FONT_OFFSET_X, y + FONT_OFFSET_Y);
    {
	unsigned short *p = line;
	int textchar = ' ';
	long style;

	while (*p) {
	    style = (*p) >> 8;
	    textchar = (*p) & 255;
	    if (!style || style & MOD_ABNORMAL || style & MOD_CURSOR)
		set_color (DEF_COLOR);
	    if (style & MOD_ABNORMAL)
		textchar = '.';
	    if (style & MOD_HIGHLIGHTED) {
		set_color (BOLD_COLOR);
	    } else if (style & MOD_MARKED) {
		set_color (MARK_COLOR);
	    }
	    if (style & MOD_UNDERLINED) {
		set_color (UNDERLINE_COLOR);
	    }
	    if (style & MOD_BOLD) {
		set_color (BOLD_COLOR);
	    }
	    addch (textchar);
	    p++;
	}
    }
}

#endif



#ifndef MIDNIGHT

/* feel free to add your own translation table (XWindows version only) */
static char basic_translation[256] =
{"0ABCDEFGHIJKL\005NOPQRSTUVWXYZ[\\]^_"
 " !\"#$%&'()*+,-./0123456789:;<=>?"
 "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
 "`abcdefghijklmnopqrstuvwxyz{|}~ "
 "\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024\024"
 "\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023\023"
 "\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"
 "\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021\021"
 "\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020\020"};

char *translation_table = 0;

#endif


/* b pointer to begining of line */
static void edit_draw_this_line (WEdit * edit, long b, long row, long start_col, long end_col)
{
    static unsigned short line[MAX_LINE_LEN];
    unsigned short *p = line;
    long m1 = 0, m2 = 0, q;
    int col, start_col_real;
    unsigned int c;
    int i;

#ifndef MIDNIGHT
    if (!translation_table)
	translation_table = basic_translation;
#endif

    q = edit_move_forward3 (edit, b, start_col - edit->start_col, 0);
    start_col_real = (col = (int) edit_move_forward3 (edit, b, 0, q)) + edit->start_col;

    if (col + 16 > -edit->start_col) {
	eval_marks (edit, &m1, &m2);

	if (row <= edit->total_lines - edit->start_line) {
	    while (col <= end_col - edit->start_col) {
		*p = 0;
		if (q == edit->curs1)
		    *p |= MOD_CURSOR * 256;
		if (q >= m1 && q < m2)
		    *p |= MOD_MARKED * 256;
		if (q >= edit->found_start && q < edit->found_start + edit->found_len)
		    *p |= MOD_HIGHLIGHTED * 256;
		switch (c = edit_get_byte (edit, q++)) {
		case '\n':
		    col = end_col - edit->start_col + 1;	/* quit */
		    *(p++) |= ' ';
		    break;
		case '\t':
		    i = TAB_SIZE - ((int) col % TAB_SIZE);
		    *p |= ' ';
		    c = *(p++) & (0xFFFF - MOD_CURSOR * 256);
		    col += i;
		    while (--i)
			*(p++) = c;
		    break;
#ifdef MIDNIGHT
		case '\r':
		    break;
#endif
		default:
#ifdef MIDNIGHT
		    if (is_printable (c)) {
			*(p++) |= c;
		    } else {
			*(p++) = '.';
			*p |= (256 * MOD_ABNORMAL);
		    }
#else
		    if (option_international_characters) {
			if (c < ' ' || (c > '~' && c < 160))
			    *p |= (256 * MOD_ABNORMAL);
			if (c < 160)
			    *(p++) |= translation_table[c];
			else
			    *(p++) |= c;
		    } else {
			if (c < ' ' || c > '~')
			    *p |= (256 * MOD_ABNORMAL);
			*(p++) |= translation_table[c];
		    }
#endif
		    col++;
		    break;
		}
	    }
	}
    } else {
	start_col_real = start_col = 0;
    }
    *p = 0;

#ifdef MIDNIGHT
    print_to_widget (edit, row, start_col, start_col_real, end_col, line);
#else
    xprint_to_widget (edit->widget->winid, row, start_col, start_col_real, end_col, line);
#endif
}

#ifdef MIDNIGHT
#define key_pending(x) (!is_idle())
#else
static int key_pending (WEdit * edit)
{
    int b = 0;
    if (!(edit->force & REDRAW_COMPLETELY)) {
	XEvent ev;
	while (XCheckWindowEvent (CDisplay, edit->widget->winid, KeyPressMask, &ev)) {
	    XSendEvent (CDisplay, edit->widget->winid, 0, KeyPressMask, &ev);	/* don't discard the event, but resend it */
	    b = 1;
	}
    }
    return b;
}
#endif

/* b for pointer to begining of line */
static void drawthischar (WEdit * edit, long curs, long row)
{
    int b = edit_bol (edit, curs);
    long start_col = edit_move_forward3 (edit, b, 0, curs) + edit->start_col;
    edit_draw_this_line (edit, b, row, start_col, start_col);
}


/* cursor must be in screen for other than REDRAW_PAGE passed in force */
void Crenderedittext (WEdit * edit, long start_row, long start_column, long end_row, long end_column)
{
    long row = 0, curs_row;
    static int prev_curs_row = 0;
    static long prev_start_display = 0;
    static int prev_start_col = 0;
    static long prev_curs = 0;

#ifndef MIDNIGHT
    static Window prev_win = 0;
#endif

    int force = edit->force;
    long b;

/*
   if the position of the page has not moved then we can draw the cursor character only.
   This will prevent line flicker when using arrow keys.
 */
    if ((!(force & REDRAW_CHAR_ONLY)) || (force & REDRAW_PAGE)
#ifndef MIDNIGHT
	|| prev_win != edit->widget->winid
#endif
	) {
	if (!(force & REDRAW_IN_BOUNDS)) {	/* !REDRAW_IN_BOUNDS means to ignore bounds and redraw whole rows */
	    start_row = 0;
	    start_column = 0;
	    end_row = edit->num_widget_lines - 1;
	    end_column = edit->num_widget_columns - 1;
	}
	if (force & REDRAW_PAGE) {
	    row = start_row;
	    b = edit_move_forward (edit, edit->start_display, start_row, 0);
	    while (row <= end_row) {
		if (key_pending (edit))
		    return;
		edit_draw_this_line (edit, b, row, start_column, end_column);
		b = edit_move_forward (edit, b, 1, 0);
		row++;
	    }
	} else {
	    curs_row = edit->curs_row;

	    if (force & REDRAW_BEFORE_CURSOR) {
		if (start_row < curs_row) {
		    long upto = curs_row - 1 <= end_row ? curs_row - 1 : end_row;
		    row = start_row;
		    b = edit->start_display;
		    while (row <= upto) {
			if (key_pending (edit))
			    return;
			edit_draw_this_line (edit, b, row, start_column, end_column);
			b = edit_move_forward (edit, b, 1, 0);
		    }
		}
	    }
	    b = edit_bol (edit, edit->curs1);
	    if (curs_row >= start_row && curs_row <= end_row) {
		if (key_pending (edit))
		    return;
		edit_draw_this_line (edit, b, curs_row, start_column, end_column);
	    }
	    if (force & REDRAW_AFTER_CURSOR) {
		if (end_row > curs_row) {
		    row = curs_row + 1 < start_row ? start_row : curs_row + 1;
		    b = edit_move_forward (edit, b, 1, 0);
		    while (row <= end_row) {
			if (key_pending (edit))
			    return;
			edit_draw_this_line (edit, b, row, start_column, end_column);
			b = edit_move_forward (edit, b, 1, 0);
			row++;
		    }
		}
	    }
	    if (force & REDRAW_LINE_ABOVE && curs_row >= 1) {
		row = curs_row - 1;
		b = edit_move_backward (edit, edit_bol (edit, edit->curs1), 1);
		if (row >= start_row && row <= end_row) {
		    if (key_pending (edit))
			return;
		    edit_draw_this_line (edit, b, row, start_column, end_column);
		}
	    }
	    if (force & REDRAW_LINE_BELOW && row < edit->num_widget_lines - 1) {
		row = curs_row + 1;
		b = edit_bol (edit, edit->curs1);
		b = edit_move_forward (edit, b, 1, 0);
		if (row >= start_row && row <= end_row) {
		    if (key_pending (edit))
			return;
		    edit_draw_this_line (edit, b, row, start_column, end_column);
		}
	    }
	}
    } else {
	drawthischar (edit, prev_curs, prev_curs_row);
	drawthischar (edit, edit->curs1, edit->curs_row);
    }

    edit->force = 0;

    prev_curs_row = edit->curs_row;
    prev_curs = edit->curs1;
    prev_start_display = edit->start_display;
    prev_start_col = edit->start_col;
#ifndef MIDNIGHT
    prev_win = edit->widget->winid;
#endif

}



#ifndef MIDNIGHT
void Cedit_expose_to_area (XExposeEvent * xexpose, int *row1, int *col1, int *row2, int *col2)
{
    int M_width = TEXT_M_WIDTH;
    int M_height = TEXT_PIX_PER_LINE;

    *col1 = (xexpose->x - EDIT_TEXT_HORIZONTAL_OFFSET) / M_width;
    *row1 = (xexpose->y - EDIT_TEXT_VERTICAL_OFFSET) / M_height;
    *col2 = (xexpose->x + xexpose->width - EDIT_TEXT_HORIZONTAL_OFFSET) / M_width;
    *row2 = (xexpose->y + xexpose->height - EDIT_TEXT_VERTICAL_OFFSET) / M_height;
}

void edit_render_tidbits (CWidget * widget)
{
    int isfocussed;
    int w = widget->width, h = widget->height;
    Window win;

    win = widget->winid;
    isfocussed = (win == CGetFocus ());

    Csetcolor (C_FLAT);

    if (isfocussed) {
	Crenderbevel (win, 0, 0, w - 1, h - 1, 3, 1);	/*most outer border bevel */
    } else {
	Crenderbevel (win, 2, 2, w - 3, h - 3, 1, 1);	/*border bevel */
	Crenderbevel (win, 0, 0, w - 1, h - 1, 2, 0);	/*most outer border bevel */
    }
}

#endif

void edit_render (WEdit * edit, int page, int row_start, int col_start, int row_end, int col_end)
{
    if (page)			/* if it was an expose event, 'page' would be set */
	edit->force |= REDRAW_PAGE | REDRAW_IN_BOUNDS;

#ifndef MIDNIGHT
    edit_set_foreground_colors (
				   Ccolor (option_editor_fg_normal),
				   Ccolor (option_editor_fg_bold),
				   Ccolor (option_editor_fg_italic)
	);
    edit_set_background_colors (
				   Ccolor (option_editor_bg_normal),
				   Ccolor (option_editor_bg_abnormal),
				   Ccolor (option_editor_bg_marked),
			       Ccolor (option_editor_bg_marked_abnormal),
				   Ccolor (option_editor_bg_highlighted)
	);
    edit_set_cursor_color (
			      Ccolor (option_editor_fg_cursor)
	);

    if (!EditExposeRedraw)
	Csetcursor (0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

    Crenderedittext (edit, row_start, col_start, row_end, col_end);
    if (edit->force)		/* edit->force != 0 means a key was pending and the redraw 
				   was halted, so next time we must redraw everything in case stuff
				   was left undrawn from a previous key press */
	edit->force |= REDRAW_PAGE;
#ifndef MIDNIGHT
    edit_render_tidbits (edit->widget);
#endif
}

#ifndef MIDNIGHT
void edit_render_expose (WEdit * edit, XExposeEvent * xexpose)
{
    int row_start, col_start, row_end, col_end;
    Cedit_expose_to_area (xexpose, &row_start, &col_start, &row_end, &col_end);
    edit_render (edit, 1, row_start, col_start, row_end, col_end);
}

void edit_render_keypress (WEdit * edit)
{
    edit_render (edit, 0, 0, 0, 0, 0);
}

#else

void edit_render_keypress (WEdit * edit)
{
    edit_render (edit, 0, 0, 0, 0, 0);
}




#endif
