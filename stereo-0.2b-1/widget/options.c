/* options - global options that may be set on the command line
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
#include "edit.h"
#include "cmdlineopt.h"

/* #define OMIT_NON_COOLEDIT_OPTIONS */

#ifdef OMIT_NON_COOLEDIT_OPTIONS

extern int option_save_setup_on_exit;
extern int option_suppress_load_files;

#endif

extern int option_text_fg_normal;
extern int option_text_fg_bold;
extern int option_text_fg_italic;

extern int option_text_bg_normal;
extern int option_text_bg_marked;
extern int option_text_bg_highlighted;

static struct {
    char *name;
    int *value;
} integer_options [] = {
    {"option_international_characters", &option_international_characters},
	{"option_tab_spacing", &option_tab_spacing},
	{"option_fill_tabs_with_spaces", &option_fill_tabs_with_spaces},
	{"option_return_does_auto_indent", &option_return_does_auto_indent},
	{"option_backspace_through_tabs", &option_backspace_through_tabs},
	{"option_fake_half_tabs", &option_fake_half_tabs},
	{"option_editor_fg_normal", &option_editor_fg_normal},
	{"option_editor_fg_bold", &option_editor_fg_bold},
	{"option_editor_fg_italic", &option_editor_fg_italic},
	{"option_editor_bg_normal", &option_editor_bg_normal},
	{"option_editor_bg_abnormal", &option_editor_bg_abnormal},
	{"option_editor_bg_marked", &option_editor_bg_marked},
	{"option_editor_bg_marked_abnormal", &option_editor_bg_marked_abnormal},
	{"option_editor_bg_highlighted", &option_editor_bg_highlighted},
	{"option_editor_fg_cursor", &option_editor_fg_cursor},
	{"option_text_fg_normal", &option_text_fg_normal},
	{"option_text_fg_bold", &option_text_fg_bold},
	{"option_text_fg_italic", &option_text_fg_italic},
	{"option_text_bg_normal", &option_text_bg_normal},
	{"option_text_bg_marked", &option_text_bg_marked},
	{"option_text_bg_highlighted", &option_text_bg_highlighted},
#ifdef OMIT_NON_COOLEDIT_OPTIONS
	{"option_save_setup_on_exit", &option_save_setup_on_exit},
	{"option_suppress_load_files", &option_suppress_load_files},
#endif
	{0, 0}
};

extern char *option_display;
extern char *option_geometry;
extern char *option_background_color;
extern char *option_foreground_red;
extern char *option_foreground_green;
extern char *option_foreground_blue;
extern char *option_font;

static struct {
    char *name;
    char **value;
} string_options [] = {
    {"option_whole_chars_search", &option_whole_chars_search},
	{"option_whole_chars_move", &option_whole_chars_move},
	{"option_display", &option_display},
	{"option_geometry", &option_geometry},
	{"option_background_color", &option_background_color},
	{"option_foreground_red", &option_foreground_red},
	{"option_foreground_green", &option_foreground_green},
	{"option_foreground_blue", &option_foreground_blue},
	{"option_font", &option_font},
	{0, 0}
};

int load_setup (const char *file)
{
    char *p, *q;
    static char *s = 0;
    int fin = 0;
    
    if (!file) {
	if (s) {
	    free (s);
	    s = 0;
	}
	return 0;
    }
    p = q = get_options_section (file, "[Options]");
    s = p;

    if (!s)
	return -1;

    for (fin = 0;!fin;) {
	if (*q == '\n' || !*q) {
	    int i;
	    if (!*q)
		fin = 1;
	    *q = 0;
	    for (i = 0; string_options[i].name; i++) {
		int l;
		l = strlen (string_options[i].name);
		l = strnlen (p, l);
		if (p[l] && strchr ("\t =", p[l])) {
		    if (!strncasecmp (p, string_options[i].name, l)) {
			*(string_options[i].value) = p + l + strspn (p + l, " =\t");
			break;
		    }
		}
	    }
	    for (i = 0; integer_options[i].name; i++) {
		int l;
		l = strlen (integer_options[i].name);
		l = strnlen (p, l);
		if (p[l] && strchr ("\t =", p[l])) {
		    if (!strncasecmp (p, integer_options[i].name, l)) {
			*(integer_options[i].value) = atoi (p + l + strspn (p + l, " =\t"));
			break;
		    }
		}
	    }
	    p = (++q);
	} else {
	    q++;
	}
    }
    return 0;
}

int save_setup (const char *file)
{
    char *p, *s;
    int r, i;

    p = s = Cmalloc (8192);

    for (i = 0; string_options[i].name; i++) {
	if (*string_options[i].value) {
	    sprintf (p, "%s = %s\n%n", string_options[i].name, *string_options[i].value, &r);
	    p += r;
	}
    }
    for (i = 0; integer_options[i].name; i++) {
	sprintf (p, "%s = %d\n%n", integer_options[i].name, *integer_options[i].value, &r);
	p += r;
    }
    *p = 0;

    r = save_options_section (file, "[Options]", s);
    free (s);
    return r;
}

int cb_save_options (CWidget *w, XEvent *xe, CEvent *cw)
{
    return 0;
}

void draw_options_dialog (Window parent, int x, int y)
{
    Window win;
    XEvent xev;
    CEvent cev;
    int o, x2, y2;
    CState s;
    CWidget *m;
    
    CBackupState (&s);
    CDisable ("*");

    win = Cdrawheadedwindow ("options", parent, x, y, 10, 10, " Options ");
    Cgethintpos (&x, &y);
    
    o = (32 - (TEXT_PIX_PER_LINE + 2 + TEXT_RELIEF * 2)) / 2;

    Cdrawswitch ("options.int_char", win, x, y, C_BLACK, C_FLAT, option_international_characters);
    Cgethintpos (0, &y2);
    Cdrawtext ("options.tint_char", win, 32 + WIDGET_SPACING + x, y + o, " Display international characters ");

    Cdrawswitch ("options.tab_space", win, x, y2, C_BLACK, C_FLAT, option_fill_tabs_with_spaces);
    Cgethintpos (0, &y);
    Cdrawtext ("options.ttab_space", win, 32 + WIDGET_SPACING + x, y2 + o, " Fill tabs with spaces ");

    Cdrawswitch ("options.auto_ind", win, x, y, C_BLACK, C_FLAT, option_return_does_auto_indent);
    Cgethintpos (0, &y2);
    Cdrawtext ("options.tauto_ind", win, 32 + WIDGET_SPACING + x, y + o, " Return does auto indent ");
    
    Cdrawswitch ("options.back_thru", win, x, y2, C_BLACK, C_FLAT, option_backspace_through_tabs);
    Cgethintpos (0, &y);
    Cdrawtext ("options.tback_thru", win, 32 + WIDGET_SPACING + x, y2 + o, " Backspace through all tabs ");

    Cdrawswitch ("options.half_tab", win, x, y, C_BLACK, C_FLAT, option_fake_half_tabs);
    Cgethintpos (0, &y2);
    Cdrawtext ("options.thalf_tab", win, 32 + WIDGET_SPACING + x, y + o, " Emulate half tabs with spaces ");

#ifdef OMIT_NON_COOLEDIT_OPTIONS
    Cdrawswitch ("options.save_exit", win, x, y2, C_BLACK, C_FLAT, option_save_setup_on_exit);
    Cgethintpos (0, &y);
    Cdrawtext ("options.tsave_exit", win, 32 + WIDGET_SPACING + x, y2 + o, " Save setup on exit ");

    Cdrawswitch ("options.no_load", win, x, y, C_BLACK, C_FLAT, !option_suppress_load_files);
    Cgethintpos (0, &y2);
    Cdrawtext ("options.tno_load", win, 32 + WIDGET_SPACING + x, y + o, " Load back desktop on startup ");
#endif

    Cdrawtext ("options.ttab_spacing", win, x, y2, " Tab spacing: ");
    Cgethintpos (&x2, 0);
    Cdrawtextinput ("options.tab_spacing", win, x2, y2, TEXT_M_WIDTH * 4, AUTO_HEIGHT, 8, itoa (option_tab_spacing));

    Cgethintpos (0, &y);
    Cdrawtext ("options.twc_search", win, x, y, " Whole chars search: ");
    Cgethintpos (&x2, 0);
    Cdrawtextinput ("options.wc_search", win, x2, y, TEXT_M_WIDTH * 16, AUTO_HEIGHT, 258, option_whole_chars_search);
    Cgethintpos (0, &y);
    Cdrawtext ("options.twc_move", win, x, y, " Whole chars move: ");
    Cgethintpos (&x2, 0);
    Cdrawtextinput ("options.wc_move", win, x2, y, TEXT_M_WIDTH * 16, AUTO_HEIGHT, 258, option_whole_chars_move);

/*
extern int option_international_characters;
extern int option_fill_tabs_with_spaces;
extern int option_return_does_auto_indent;
extern int option_backspace_through_tabs;
extern int option_fake_half_tabs;

extern int option_tab_spacing;
extern int option_tab_spacing_spaces;
*/
    
    Cgethintpos (0, &y);
    Csetsizehintpos ("options");
    m = Cwidget ("options");
    Cdrawbitmapbutton ("options.ok", win, x, y, 40, 40, Ccolor (6), C_FLAT, tick_bits);
    Cgethintpos (&x2, 0);
    Cdrawbitmapbutton ("options.cancel", win, x2, y, 40, 40, Ccolor (18), C_FLAT, cross_bits);
    Cgethintpos (&x2, 0);
    Cdrawbutton ("options.save", win, x2, y, AUTO_WIDTH, AUTO_HEIGHT, " Save ");
    Caddcallback ("options.save", cb_save_options);
    Csetsizehintpos ("options");

    CFocus (Cwidget ("options.ok"));

    for (;;) {
	CNextEvent (&xev, &cev);
	if (!strcmp (cev.ident, "options.cancel") || (CKeySym (&xev) == XK_Escape && xev.type == KeyPress))
	    break;
	if (!strcmp (cev.ident, "options.ok") || (CKeySym (&xev) == XK_Return && xev.type == KeyPress)) {
	    option_international_characters = (Cwidget ("options.int_char"))->keypressed;
	    option_fill_tabs_with_spaces = (Cwidget ("options.tab_space"))->keypressed;
	    option_return_does_auto_indent = (Cwidget ("options.auto_ind"))->keypressed;
	    option_backspace_through_tabs = (Cwidget ("options.back_thru"))->keypressed;
	    option_fake_half_tabs = (Cwidget ("options.half_tab"))->keypressed;
	    option_tab_spacing = atoi ((Cwidget ("options.tab_spacing"))->text);
	    option_whole_chars_search = strdup ((Cwidget ("options.wc_search"))->text);
	    option_whole_chars_move = strdup ((Cwidget ("options.wc_move"))->text);
#ifdef OMIT_NON_COOLEDIT_OPTIONS
	    option_save_setup_on_exit = (Cwidget ("options.save_exit"))->keypressed;
	    option_suppress_load_files = !(Cwidget ("options.no_load"))->keypressed;
#endif
	    break;
	}
    }
    Cundrawwidget ("options");
    CRestoreState (&s);
}



















