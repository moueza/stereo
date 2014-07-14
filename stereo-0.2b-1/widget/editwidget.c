/* editor initialisation and callback handler.

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

#ifndef MIDNIGHT
#include "app_glob.c"
#include "dirtools.h"
#include "coollocal.h"
#include "editcmddef.h"
#endif



#ifndef MIDNIGHT

extern int EditExposeRedraw;
CWidget *wedit = 0;

void Cedit_destroy (CWidget * w)
{
    if (w) {
	edit_clean (w->editor);
	if (w->editor)
	    free (w->editor);
	w->editor = NULL;
    } else
	Cerror ("Trying to destroy non-existing editor widget.\n");
}

extern int option_editor_bg_normal;

/* starting_directory is for the filebrowser */
CWidget *Cdraweditor (const char *identifier, Window parent, int x, int y,
	   int width, int height, const char *text, const char *filename,
		      const char *starting_directory)
{
    static made_directory = 0;
    char *scroll;
    CWidget *w;
    WEdit *e;
    wedit = w = Csetupwidget (identifier, parent, x, y,
			      width + 7, height + 6, CEDITOR_WIDGET, INPUT_KEY | KeyReleaseMask | ButtonMotionMask, Ccolor (option_editor_bg_normal), 1);

    w->destroy = Cedit_destroy;
    if (filename)
	w->label = strdup (filename);
    else
	w->label = strdup ("");

    if (!made_directory) {
	mkdir (catstrs (home_dir, EDIT_DIR, 0), 0777);
	made_directory = 1;
    }
    e = w->editor = Cmalloc (sizeof (WEdit));
    if (!w->editor) {
	Cerror ("Error initialising editor.\n");
	return 0;
    }
    w->editor->widget = w;
    e = w->editor = edit_init (e, height / TEXT_PIX_PER_LINE, width / TEXT_M_WIDTH, filename, text, starting_directory);
    if (!e) {
	Cundrawwidget (w->ident);
	return 0;
    }
    e->macro_i = -1;
    e->widget = w;

    Cdrawtext (catstrs (identifier, ".text", 0), parent, x, y + height + 6 + WIDGET_SPACING, "%s", e->filename);
    Cdrawvertscrollbar (scroll = catstrs (identifier, ".vsc", 0), parent,
		x + width + 7 + WIDGET_SPACING, y, height + 6, 20, 0, 0);
    w->scrollbar = Cwidget (scroll);
    w->scrollbar->editor = (void *) w;

    return w;
}

void update_scroll_bar (WEdit * e)
{
    int i, x1, x2;
    CWidget *scroll;
    scroll = e->widget->scrollbar;
    if (!scroll)
	return;

    i = e->total_lines - e->start_line + 1;
    if (i > e->num_widget_lines)
	i = e->num_widget_lines;
    if (e->total_lines) {
	x1 = (double) 65535.0 *e->start_line / (e->total_lines + 1);
	x2 = (double) 65535.0 *i / (e->total_lines + 1);
    } else {
	x1 = 0;
	x2 = 65535;
    }
    if (x1 != scroll->firstline || x2 != scroll->numlines) {
	scroll->firstline = x1;
	scroll->numlines = x2;
	EditExposeRedraw = 1;
	Crenderscrollbar (scroll);
	EditExposeRedraw = 0;
    }
}


void Cedit_update_screen (WEdit * e)
{
    XEvent ev;

    if (!e)
	return;

    edit_scroll_screen_over_cursor (e);
    update_curs_row (e);
    update_curs_col (e);
    update_scroll_bar (e);
    edit_status (e);

/* pop all events for this window for internal handling */
    {
	int b = 0;
	while (XCheckWindowEvent (CDisplay, e->widget->winid, KeyPressMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, &ev)) {
	    XSendEvent (CDisplay, e->widget->winid, 0, KeyPressMask, &ev);	/* don't discard the event, but resend it */
	    b = 1;
	}
	if (b && !(e->force & REDRAW_CHAR_ONLY) && !(e->force & REDRAW_COMPLETELY)) {
	    e->force |= REDRAW_PAGE;
	    return;
	}
    }

    if (e->force & REDRAW_COMPLETELY)
	e->force |= REDRAW_PAGE;
    edit_render_keypress (e);
}


/* returns the position in the edit buffer of a window click */
long edit_get_click_pos (WEdit * edit, int x, int y)
{
    long click;
/* (1) goto to left margin */
    click = edit_bol (edit, edit->curs1);

/* (1) move up or down */
    if (y > (edit->curs_row + 1))
	click = edit_move_forward (edit, click, y - (edit->curs_row + 1), 0);
    if (y < (edit->curs_row + 1))
	click = edit_move_backward (edit, click, (edit->curs_row + 1) - y);

/* (3) move right to x pos */
    click = edit_move_forward3 (edit, click, x - edit->start_col - 1, 0);
    return click;
}

void edit_translate_xy (int xs, int ys, int *x, int *y)
{
    *x = (xs - EDIT_TEXT_HORIZONTAL_OFFSET) / TEXT_M_WIDTH + 1;
    *y = (ys - EDIT_TEXT_VERTICAL_OFFSET) / TEXT_PIX_PER_LINE + 1;
}

static int just_dropped_something = 0;

static void mouse_redraw (WEdit * edit, long click)
{
    edit->force |= REDRAW_PAGE;
    update_curs_row (edit);
    update_curs_col (edit);
    edit->prev_col = edit_get_col (edit);
    Cedit_update_screen (edit);
    edit->search_start = click;
}

static void edit_mouse_mark (WEdit * edit, XEvent * event, CEvent * ce)
{
    static Window win_press = 0;
    static int state = 0;	/* 0 = button up, 1 = button pressed, 2 = button pressed and dragging */
    static int x_last, y_last;
    long click;

    update_curs_row (edit);
    update_curs_col (edit);

    if (event->type != MotionNotify) {
	push_action (edit, KEY_PRESS + edit->start_display);
	if (edit->mark2 == -1)
	    push_action (edit, MARK_1 + edit->mark1);	/* mark1 must be following the cursor */
    }
    if (event->type == ButtonPress) {
	long start_mark, end_mark;
	state = 1;
	win_press = edit->widget->winid;
	edit->highlight = 0;
	edit_translate_xy (event->xbutton.x, event->xbutton.y, &x_last, &y_last);
	click = edit_get_click_pos (edit, x_last, y_last);
	if (!eval_marks (edit, &start_mark, &end_mark)) {
	    if (start_mark <= click && click < end_mark) {	/* if clicked on highlighted text */
		unsigned char *t;
		if (edit->mark2 < 0)
		    edit_mark_cmd (edit, 0);
		t = edit_get_block (edit, start_mark, end_mark);
		just_dropped_something = 1;
		CDrag (event->xbutton.window, DndText, t, end_mark - start_mark, event->xbutton.button == Button1 ? Button1Mask : 0);
		free (t);
		return;
	    }
	}
	just_dropped_something = 0;
	if (edit->mark2 < 0)
	    edit_mark_cmd (edit, 0);
	edit_cursor_move (edit, click - edit->curs1);
	if (ce->double_click) {
	    edit_mark_cmd (edit, 1);
	    edit_right_word_move (edit);
	    edit_mark_cmd (edit, 0);
	    edit_left_word_move (edit);
	    edit_mark_cmd (edit, 0);
	}
	mouse_redraw (edit, click);
    } else if (event->type == ButtonRelease && state > 0 && win_press == edit->widget->winid) {
	int x, y;
	edit_translate_xy (event->xbutton.x, event->xbutton.y, &x, &y);
	click = edit_get_click_pos (edit, x, y);
	state = 0;
	edit_cursor_move (edit, click - edit->curs1);
	if (edit->mark2 < 0)
	    edit_mark_cmd (edit, 0);
	mouse_redraw (edit, click);
    } else if (event->type == MotionNotify && state > 0 && win_press == edit->widget->winid) {
	int x, y;
	edit_translate_xy (event->xbutton.x, event->xbutton.y, &x, &y);
	if (x == x_last && y == y_last && state == 1)
	    return;
	click = edit_get_click_pos (edit, x, y);
	if (state == 1) {
	    state = 2;
	    edit_mark_cmd (edit, 1);
	    edit_mark_cmd (edit, 0);
	}
	edit_cursor_move (edit, click - edit->curs1);
	mouse_redraw (edit, click);
    }
}
    
void linkscrollbartoeditor (CWidget * w, CWidget * editor, XEvent * xevent, CEvent * cwevent, int whichscrbutton)
{
    int redrawtext = 0, i;
    WEdit *e;
    e = editor->editor;
    if (!e)
	return;
    if (w->firstline > 65535)
	w->firstline = 65535;
    if (xevent->type == MotionNotify && whichscrbutton == 3) {
	edit_move_display (e, (double) w->firstline * e->total_lines / 65535.0 + 1);
	redrawtext = 1;
    } else if (xevent->type == ButtonPress && (cwevent->button == Button1 || cwevent->button == Button2)) {
	switch (whichscrbutton) {
	case 1:
	    edit_move_display (e, e->start_line - e->num_widget_lines + 1);
	    redrawtext = 1;
	    break;
	case 2:
	    edit_move_display (e, e->start_line - 1);
	    redrawtext = 1;
	    break;
	case 5:
	    edit_move_display (e, e->start_line + 1);
	    redrawtext = 1;
	    break;
	case 4:
	    edit_move_display (e, e->start_line + e->num_widget_lines - 1);
	    redrawtext = 1;
	    break;
	}
    }
    if (e->total_lines)
	w->firstline = (double) 65535.0 *e->start_line / (e->total_lines + 1);
    else
	w->firstline = 0;
    i = e->total_lines - e->start_line + 1;
    if (i > e->num_widget_lines)
	i = e->num_widget_lines;
    if (e->total_lines)
	w->numlines = (double) 65535.0 *i / (e->total_lines + 1);
    else
	w->numlines = 65535;
    if (redrawtext) {
	e->force |= REDRAW_PAGE;
	Csetcursor (0, 0, 0, 0, 0, 0, 0, 0, 0);
	{
	    int b = 0;
	    XEvent ev;
	    while (XCheckWindowEvent (CDisplay, xevent->xany.window, ButtonMotionMask, &ev)) {
		e->force |= REDRAW_PAGE;
		XSendEvent (CDisplay, e->widget->winid, 0, KeyPressMask, &ev);	/* don't discard the event, but resend it */
		b = 1;
	    }
	    if (b)
		return;
	}
	edit_render_keypress (e);
	edit_status (e);
    }
}


#define dnd_null_term_type(d) \
	((d) == DndFile || (d) == DndText || (d) == DndDir || \
	(d) == DndLink || (d) == DndExe || (d) == DndURL)


int eh_editor (CWidget * w, XEvent * xevent, CEvent * cwevent)
{
    WEdit *e = w->editor;
    char xlat;
    KeySym key;
    long k = 1;
    int cmd, ch;
    static int old_tab_spacing = -1;
    
    if (!e)
	return 0;
    if (!w->scrollbar)
	return 0;

    if (old_tab_spacing != option_tab_spacing)
	e->force |= REDRAW_COMPLETELY;
    old_tab_spacing = option_tab_spacing;

    switch (xevent->type) {
    case ClientMessage:{
	    int data_type;
	    unsigned char *data;
	    unsigned long size;
	    int xs, ys;
	    int x, y, r;
	    long click;
	    unsigned int state;
	    long start_mark = 0, end_mark = 0;

/* see just below for a comment on what this is for: */
	    if (CIsDropAcknowledge (xevent, &state) != DndNotDnd) {
		if (!(state & Button1Mask) && just_dropped_something) {
		    push_action (e, KEY_PRESS + e->start_display);
		    edit_block_delete_cmd (e);
		    Cedit_update_screen (e);
		}
		break;
	    }
	    data_type = CGetDrop (xevent, &data, &size, &xs, &ys);
	    if (data_type == DndNotDnd || xs < 0 || ys < 0 || xs >= w->width || ys >= w->height)
		break;
	    edit_translate_xy (xs, ys, &x, &y);
	    click = edit_get_click_pos (e, x, y);

	    r = eval_marks (e, &start_mark, &end_mark);
	    if (r || click < start_mark || click >= end_mark) {		/* musn't be able to drop into a block,
									   otherwise a single click will copy a block: */
		push_action (e, KEY_PRESS + e->start_display);
		edit_cursor_move (e, click - e->curs1);
		if (data_type == DndFile) {
		    edit_insert_file (e, data);
		} else if (data_type != DndFiles) {
		    if (dnd_null_term_type (data_type)) {
			int len;
			len = strlen (data);
			size = min (len, size);
		    }
		    while (size--)
			edit_insert_ahead (e, data[size]);
		} else {
		    while (size--)
			edit_insert_ahead (e, data[size] ? data[size] : '\n');
		}
/* The drop has now been successfully recieved. We can now send an acknowledge
   event back to the window that send the data. When this window recieves
   the acknowledge event, the app can decide whether or not to delete the data.
   This allows text to be safely moved betweem text windows without the
   risk of data being lost. In our case, drag with button1 is a copy
   drag, while drag with any other button is a move drag (i.e. the sending
   application must delete its selection after recieving an acknowledge
   event). We must not, however, send an acknowledge signal if a filelist
   (for example) was passed to us, since the sender might take this to
   mean that all those file can be deleted! The two types we can acknowledge
   are: */
		if (data_type == DndText || data_type == DndRawData)
		    CDropAcknowledge (xevent);
		e->force |= REDRAW_PAGE;
		Cedit_update_screen (e);
		break;
	    } else {
		push_action (e, KEY_PRESS + e->start_display);
		edit_push_markers (e);
		edit_set_markers (e, 0, 0);
		e->force |= REDRAW_PAGE;
		Cedit_update_screen (e);
	    }
	}
	break;
    case ResizeNotify:
	Csetwidgetsize (w->ident, w->width + xevent->xconfigure.x, w->height + xevent->xconfigure.y);
	e->num_widget_lines = (w->height - 6) / TEXT_PIX_PER_LINE;
	e->num_widget_columns = (w->width - 7) / TEXT_M_WIDTH;
	Csetwidgetsize (catstrs (w->ident, ".vsc", 0), 20, w->height);
	Csetwidgetposition (catstrs (w->ident, ".vsc", 0), (Cwidget (catstrs (w->ident, ".vsc", 0)))->x + xevent->xconfigure.x, WIDGET_SPACING + 2);
	Csetwidgetposition (catstrs (w->ident, ".text", 0), WIDGET_SPACING + 2, (Cwidget (catstrs (w->ident, ".text", 0)))->y + xevent->xconfigure.y);
	edit_status (e);
	break;
    case ButtonPress:
	Cresolvebutton (xevent, cwevent);
	CFocusWindow (w->winid);
	edit_render_tidbits (w);
    case ButtonRelease:
    case MotionNotify:
	edit_mouse_mark (e, xevent, cwevent);
	break;
    case Expose:
	EditExposeRedraw = 1;
	edit_render_expose (e, &(xevent->xexpose));
	EditExposeRedraw = 0;
	return 0;
    case FocusIn:
    case FocusOut:
	e->force |= REDRAW_CHAR_ONLY;
	Cedit_update_screen (e);
	break;
    case KeyRelease:
	break;
    case KeyPress:
	xlat = 0;
	key = CKeySym (xevent);
	if (!key)
	    return 0;
	k = (long) k *key;
	cwevent->ident = w->ident;
	cwevent->key = k;
	cwevent->xlat = xlat;
	cwevent->state = xevent->xkey.state;
	if (!edit_translate_key (e, xevent->xkey.keycode, k, xevent->xkey.state, &cmd, &ch)) {
	    KeySym c;
	    c = CKeySym (xevent);
	    cmd = CKeySymMod (xevent);
	    if (cmd > 0)
		cmd = CK_Macro (cmd);
	    else
		return 0;
	}
	cwevent->command = cmd;
	k = edit_execute_key_command (e, cmd, ch);
	Cedit_update_screen (e);	/* command changed display, so update display */
	return k;
    case EditorCommand:
	edit_execute_key_command (e, xevent->xkey.keycode, -1);
	Cedit_update_screen (e);
	return 1;
    }
    return 0;
}



#else

WEdit *wedit;
WButtonBar *edit_bar;
Dlg_head *edit_dlg;
WMenu *edit_menubar;


static int edit_callback (Dlg_head * h, WEdit * edit, int msg, int par);

static int edit_mode_callback (struct Dlg_head *h, int id, int msg)
{
    return 0;
}

int edit_event (WEdit * edit, Gpm_Event * event, int *result)
{
    *result = MOU_NORMAL;
    update_curs_row (edit);
    update_curs_col (edit);
    if (event->type & (GPM_DOWN | GPM_DRAG | GPM_UP)) {
	if (event->y > 1 && event->x > 0
	    && event->x <= edit->num_widget_columns
	    && event->y <= edit->num_widget_lines + 1) {
	    if (edit->mark2 != -1 && event->type & (GPM_UP | GPM_DRAG))
		return 1;	/* a lone up mustn't do anything */
	    if (event->type & (GPM_DOWN | GPM_UP))
		push_key_press (edit);
	    edit_cursor_move (edit, edit_bol (edit, edit->curs1) - edit->curs1);
	    if (--event->y > (edit->curs_row + 1))
		edit_cursor_move (edit,
				  edit_move_forward (edit, edit->curs1, event->y - (edit->curs_row + 1), 0)
				  - edit->curs1);
	    if (event->y < (edit->curs_row + 1))
		edit_cursor_move (edit,
				  +edit_move_backward (edit, edit->curs1, (edit->curs_row + 1) - event->y)
				  - edit->curs1);
	    edit_cursor_move (edit, (int) edit_move_forward3 (edit, edit->curs1,
		       event->x - edit->start_col - 1, 0) - edit->curs1);
	    edit->prev_col = edit_get_col (edit);
	    if (event->type & GPM_DOWN) {
		edit_mark_cmd (edit, 1);	/* reset */
		edit->highlight = 0;
	    }
	    if (!(event->type & GPM_DRAG))
		edit_mark_cmd (edit, 0);
	    edit->force |= REDRAW_COMPLETELY;
	    update_curs_row (edit);
	    update_curs_col (edit);
	    Cedit_update_screen (edit);
	    return 1;
	}
    }
    return 0;
}



int menubar_event (Gpm_Event * event, WMenu * menubar);		/* menu.c */

int edit_mouse_event (Gpm_Event * event, void *x)
{
    int result;
    if (edit_event ((WEdit *) x, event, &result))
	return result;
    else
	return menubar_event (event, edit_menubar);
}

extern Menu EditMenuBar[5];

int edit (char *_file)
{
    static int made_directory = 0;
    int framed = 0;
    int midnight_colors[4];
    char *text = 0;

    if (!made_directory) {
	mkdir (catstrs (home_dir, EDIT_DIR, 0), 0777);
	made_directory = 1;
    }
    if (_file) {
	if (!(*_file)) {
	    _file = 0;
	    text = "";
	}
    } else
	text = "";

    if (!(wedit = edit_init (NULL, LINES - 2, COLS, _file, text, ""))) {
	message (1, " Error ", get_error_msg (""));
	return 0;
    }
    wedit->macro_i = -1;

    /* Create a new dialog and add it widgets to it */
    edit_dlg = create_dlg (0, 0, LINES, COLS, midnight_colors,
			   edit_mode_callback, "[Internal File Editor]",
			   "edit",
			   DLG_NONE);

    edit_dlg->raw = 1;		/*so that tab = '\t' key works */

    init_widget (&(wedit->widget), 0, 0, LINES - 1, COLS,
		 (callback_fn) edit_callback,
		 (destroy_fn) edit_clean,
		 (mouse_h) edit_mouse_event, 0);

    widget_want_cursor (wedit->widget, 1);

    edit_bar = buttonbar_new (1);

    if (!framed) {
	switch (edit_key_emulation) {
	case EDIT_KEY_EMULATION_NORMAL:
	    edit_init_menu_normal ();	/* editmenu.c */
	    break;
	case EDIT_KEY_EMULATION_EMACS:
	    edit_init_menu_emacs ();	/* editmenu.c */
	    break;
	}
	edit_menubar = menubar_new (0, 0, COLS, EditMenuBar, N_menus);
    }
    add_widget (edit_dlg, wedit);

    if (!framed)
	add_widget (edit_dlg, edit_menubar);

    add_widget (edit_dlg, edit_bar);

    run_dlg (edit_dlg);

    if (!framed)
	edit_done_menu ();	/* editmenu.c */

    destroy_dlg (edit_dlg);

    return 1;
}

static void edit_my_define (Dlg_head * h, int idx, char *text,
			    void (*fn) (WEdit *), WEdit * edit)
{
    define_label_data (h, (Widget *) edit, idx, text, (buttonbarfn) fn, edit);
}


void cmd_F1 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (1));
}

void cmd_F2 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (2));
}

void cmd_F3 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (3));
}

void cmd_F4 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (4));
}

void cmd_F5 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (5));
}

void cmd_F6 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (6));
}

void cmd_F7 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (7));
}

void cmd_F8 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (8));
}

void cmd_F9 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (9));
}

void cmd_F10 (WEdit * edit)
{
    send_message (edit->widget.parent, (Widget *) edit, WIDGET_KEY, KEY_F (10));
}

void edit_labels (WEdit * edit)
{
    Dlg_head *h = edit->widget.parent;

    edit_my_define (h, 1, "Help", cmd_F1, edit);
    edit_my_define (h, 2, "Save", cmd_F2, edit);
    edit_my_define (h, 3, "Mark", cmd_F3, edit);
    edit_my_define (h, 4, "Replac", cmd_F4, edit);
    edit_my_define (h, 5, "Copy", cmd_F5, edit);
    edit_my_define (h, 6, "Move", cmd_F6, edit);
    edit_my_define (h, 7, "Search", cmd_F7, edit);
    edit_my_define (h, 8, "Delete", cmd_F8, edit);
    if (!edit->have_frame)
	edit_my_define (h, 9, "PullDn", edit_menu_cmd, edit);
    edit_my_define (h, 10, "Quit", cmd_F10, edit);

    redraw_labels (h, (Widget *) edit);
}


long get_key_state ()
{
    return (long) get_modifier ();
}

void edit_adjust_size (Dlg_head * h)
{
    WEdit *edit;
    WButtonBar *edit_bar;

    edit = (WEdit *) find_widget_type (h, (void *) &edit_callback);	/* <---- ansi standard violation */
    edit_bar = (WButtonBar *) edit->widget.parent->current->next->widget;
    widget_set_size (&edit->widget, 0, 0, LINES - 1, COLS);
    widget_set_size (&edit_bar->widget, LINES - 1, 0, 1, COLS);
}

void Cedit_update_screen (WEdit * e)
{
    edit_scroll_screen_over_cursor (e);

    update_curs_col (e);
    edit_status (e);

/* pop all events for this window for internal handling */

    if (!is_idle ()) {
	e->force |= REDRAW_PAGE;
	return;
    }
    if (e->force & REDRAW_COMPLETELY)
	e->force |= REDRAW_PAGE;
    edit_render_keypress (e);
}

static int edit_callback (Dlg_head * h, WEdit * e, int msg, int par)
{
    switch (msg) {
    case WIDGET_INIT:
	e->force |= REDRAW_COMPLETELY;
	edit_labels (e);
	break;
    case WIDGET_DRAW:
	e->force |= REDRAW_COMPLETELY;
	e->num_widget_lines = LINES - 2;
	e->num_widget_columns = COLS;
    case WIDGET_FOCUS:
	Cedit_update_screen (e);
	return 1;
    case WIDGET_KEY:{
	    int cmd, ch;
	    if (edit_drop_hotkey_menu (e, par))		/* first check alt-f, alt-e, alt-s, etc for drop menus */
		return 1;
	    if (!edit_translate_key (e, 0, par, get_key_state (), &cmd, &ch))
		return 0;
	    edit_execute_key_command (e, cmd, ch);
	    Cedit_update_screen (e);
	}
	return 1;
    case WIDGET_COMMAND:
	edit_execute_key_command (e, par, -1);
	Cedit_update_screen (e);
	return 1;
    case WIDGET_CURSOR:
	widget_move (&e->widget, e->curs_row + EDIT_TEXT_VERTICAL_OFFSET, e->curs_col + e->start_col);
	return 1;
    }
    return default_proc (h, msg, par);
}

#endif
