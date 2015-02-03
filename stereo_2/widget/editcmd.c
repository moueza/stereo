/* editor high level editing commands.

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

/* #define PIPE_BLOCKS_SO_READ_BYTE_BY_BYTE */

#include <config.h>
#include "edit.h"

#ifndef MIDNIGHT
#include "loadfile.h"
#endif

/* globals: */

/* search and replace: */
int replace_scanf = 0;
int replace_regexp = 0;
int replace_all = 0;
int replace_prompt = 1;
int replace_whole = 0;
int replace_case = 0;

/* queries on a save */
#ifdef MIDNIGHT
int edit_confirm_save = 1;
#else
int edit_confirm_save = 0;
#endif

#define NUM_REPL_ARGS 16
#define MAX_REPL_LEN 1024

#ifdef MIDNIGHT

#define my_lower_case(x) ((x) < 'a' ? (x) - 'A' + 'a' : (x))

char *strcasechr (const char *s, int c)
{
    for (; my_lower_case (*s) != my_lower_case ((char) c); ++s)
	if (*s == '\0')
	    return 0;
    return (char *) s;
}


#include "src/mad.h"

#ifndef HAVE_MEMMOVE
/* for Christophe */
static void *memmove (void *dest, const void *src, size_t n)
{
    char *t, *s;

    if (dest <= src) {
	t = (char *) dest;
	s = (char *) src;
	while (n--)
	    *t++ = *s++;
    } else {
	t = (char *) dest + n;
	s = (char *) src + n;
	while (n--)
	    *--t = *--s;
    }
    return dest;
}
#endif

char *itoa (int i)
{
    static char t[14];
    char *s = t + 13;
    int j = i;
    *s-- = 0;
    do {
	*s-- = i % 10 + '0';
    } while ((i = i / 10));
    if (j < 0)
	*s-- = '-';
    return ++s;
}

/*
   This joins strings end on end and allocates memory for the result.
   The result is later automatically free'd and must not be free'd
   by the caller.
 */
char *catstrs (const char *first,...)
{
    static char *stacked[16] =
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static int i = 0;
    va_list ap;
    int len;
    char *data;

    if (!first)
	return 0;

    len = strlen (first);
    va_start (ap, first);

    while ((data = va_arg (ap, char *)) != 0)
	 len += strlen (data);

    len++;

    i = (i + 1) % 16;
    if (stacked[i])
	free (stacked[i]);

    stacked[i] = malloc (len);
    va_end (ap);
    va_start (ap, first);
    strcpy (stacked[i], first);
    while ((data = va_arg (ap, char *)) != 0)
	 strcat (stacked[i], data);
    va_end (ap);

    return stacked[i];
}
#endif

#ifdef MIDNIGHT

void edit_help_cmd (WEdit * edit)
{
    interactive_display (LIBDIR "mc.hlp", "[Internal File Editor]");
    edit->force |= REDRAW_COMPLETELY;
}

void edit_refresh_cmd (WEdit * edit)
{
    dlg_erase (edit->widget.parent);
    edit->force |= REDRAW_COMPLETELY;
}

#else

void edit_help_cmd (WEdit * edit)
{
}

void edit_refresh_cmd (WEdit * edit)
{
}

#endif


/*TODO: Create backfile */

/*Fast save file: Does NOT create a backup file. */
/*returns 0 on error */
int edit_save_file (WEdit * edit, const char *filename)
{
    long buf;
    long filelen = 0;
    int file;

#ifdef MIDNIGHT
    if ((file = mc_open ((char *) filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	return 0;
#else
    if ((file = open ((char *) filename, O_WRONLY | O_TRUNC)) == -1)
	if ((file = creat ((char *) filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	    return 0;
#endif

    buf = 0;
    while (buf <= (edit->curs1 >> S_EDIT_BUF_SIZE) - 1) {
	filelen += write (file, (char *) edit->buffers1[buf], EDIT_BUF_SIZE);
	buf++;
    }
    filelen += write (file, (char *) edit->buffers1[buf], edit->curs1 & M_EDIT_BUF_SIZE);

    if (edit->curs2) {
	edit->curs2--;
	buf = (edit->curs2 >> S_EDIT_BUF_SIZE);
	filelen += write (file, (char *) edit->buffers2[buf] + EDIT_BUF_SIZE - (edit->curs2 & M_EDIT_BUF_SIZE) - 1, 1 + (edit->curs2 & M_EDIT_BUF_SIZE));
	buf--;
	while (buf >= 0) {
	    filelen += write (file, (char *) edit->buffers2[buf], EDIT_BUF_SIZE);
	    buf--;
	}
	edit->curs2++;
    }
    close (file);

    if (filelen == edit->last_byte)
	return 1;
    else
	return 0;
}

#ifdef MIDNIGHT

void split_filename (WEdit * edit, char *f)
{
    if (edit->filename)
	free (edit->filename);
    edit->filename = strdup (f);
    if (edit->dir)
	free (edit->dir);
    edit->dir = strdup ("");
}

#else

void split_filename (WEdit * edit, char *longname)
{
    char *exp, *p;
    exp = path_compress (0, longname);
    p = strrchr (exp, '/');
    if (edit->filename)
	free (edit->filename);
    if (edit->dir)
	free (edit->dir);
    if (p) {
	edit->filename = strdup (++p);
	*p = 0;
	edit->dir = strdup (exp);
    } else {
	edit->filename = strdup (exp);
	edit->dir = strdup ("/");
    }
}

#endif

/*  here we want to warn the user of overwriting an existing file, but only if they
   have made a change to the filename */
/* returns 1 on success */
int edit_save_as_cmd (WEdit * edit)
{
    char *exp = Cgetsavefile (edit->widget->winid, 40, 40, edit->dir, edit->filename, " Save As ");
    push_action (edit, KEY_PRESS + edit->start_display);
    edit->force |= REDRAW_COMPLETELY;

    if (exp) {
	if (!*exp) {
	    free (exp);
	    return 0;
	} else {
	    if (strcmp(catstrs (edit->dir, edit->filename, 0), exp)) {
		int file;
		if ((file = open ((char *) exp, O_RDONLY)) != -1) {	/* the file exists */
		    close (file);
#ifdef MIDNIGHT
		    if (query_dialog (" Warning ", " A file already exists with this name. ", 0, 2, " Overwrite ", " Cancel "))
#else
		    if (Cquerydialogue (edit->widget->winid, 20, 20, " Warning ", " A file already exists with this name. ", " Overwrite ", " Cancel ", NULL))
#endif
			return 0;
		}
	    }
	    if (edit_save_file (edit, exp)) {
		split_filename (edit, exp);
		free (exp);
		edit->modified = 0;
		return 1;
	    } else {
		free (exp);
		Cerrordialogue (edit->widget->winid, 20, 20, " Save as ", get_sys_error (" Error trying to save file. "));
		return 0;
	    }
	}
    } else
	return 0;
}



#ifdef MIDNIGHT
int raw_callback (struct Dlg_head *h, int key, int Msg)
{
    switch (Msg) {
    case DLG_DRAW:
	attrset (REVERSE_COLOR);
	dlg_erase (h);
	draw_box (h, 1, 1, h->lines - 2, h->cols - 2);

	attrset (COLOR_HOT_NORMAL);
	dlg_move (h, 1, 2);
	printw (h->title);
	break;

    case DLG_KEY:
	h->running = 0;
	h->ret_value = key;
	return 1;
    }
    return 0;
}

/* gets a raw key from the keyboard. Passing cancel = 1 draws
   a cancel button thus allowing c-c etc.. Alternatively, cancel = 0 
   will return the next key pressed */
int edit_raw_key_query (char *heading, char *query, int cancel)
{
    int w = strlen (query) + 7;
    struct Dlg_head *raw_dlg = create_dlg (0, 0, 7, w, dialog_colors,
					 raw_callback, "[Raw Key Query]",
					   "raw_key_input",
					   DLG_CENTER | DLG_TRYUP);
    x_set_dialog_title (raw_dlg, heading);
    raw_dlg->raw = 1;		/* to return even a tab key */
    if (cancel)
	add_widget (raw_dlg, button_new (4, w / 2 - 5, B_CANCEL, "[ Cancel ]", 'c', 2, 0, 0, 0));
    add_widget (raw_dlg, label_new (3 - cancel, 2, query, 0));
    add_widget (raw_dlg, input_new (3 - cancel, w - 5, INPUT_COLOR, 2, "", 0));
    run_dlg (raw_dlg);
    w = raw_dlg->ret_value;
    destroy_dlg (raw_dlg);
    if (cancel)
	if (w == XCTRL ('g') || w == XCTRL ('c') || w == ESC_CHAR || w == B_CANCEL)
	    return 0;
/* hence ctrl-a (=B_CANCEL), ctrl-g, ctrl-c, and Esc are cannot returned */
    return w;
}

#else

int edit_raw_key_query (char *heading, char *query, int cancel)
{
    return Crawkeyquery (0, 0, 0, heading, query);
}

#endif

/* creates a macro file if it doesn't exist */
static FILE *edit_open_macro_file (const char *r)
{
    char *filename;
    int file;
    filename = catstrs (home_dir, MACRO_FILE, 0);
#ifdef MIDNIGHT
    if ((file = mc_open (filename, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	return 0;
#else
    if ((file = open (filename, O_RDWR)) == -1)
	if ((file = creat ((char *) filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	    return 0;
#endif
    close (file);
    return fopen (filename, r);
}

/* returns 0 on error */
int edit_save_macro_cmd (WEdit * edit, struct macro macro[], int n)
{
    int i;
    FILE *f;
    int s;

    edit->force |= REDRAW_COMPLETELY;
    push_action (edit, KEY_PRESS + edit->start_display);
    s = edit_raw_key_query (" Macro ", " Press the macro's new hotkey: ", 1);
    if (s) {
	f = edit_open_macro_file ("a+");
	if (f) {
	    fprintf (f, "key '%d %d': ", s, /* s[1] */ (int) 0);	/* later we might want key combinations, but not yet */
	    for (i = 0; i < n; i++)
		fprintf (f, "%hd %hd, ", macro[i].command, macro[i].ch);
	    fprintf (f, ";\n");
	    fclose (f);
	    return 1;
	} else
	    Cerrordialogue (edit->widget->winid, 20, 20, " Save macro ", get_sys_error (" Error trying to open macro file. "));
    }
    return 0;
}




/* return 0 on error */
int edit_load_macro_cmd (WEdit * edit, struct macro macro[], int *n, int *k)
{
    FILE *f;
    int s[4];

    if ((f = edit_open_macro_file ("r"))) {
	do {
	    *n = fscanf (f, "key '%d %d': ", &(s[0]), &(s[1]));
	    if (!(*n) || *n == EOF) {
		fclose (f);
		return 0;
	    }
	    *n = 0;
	    while (fscanf (f, "%hd %hd, ", &macro[*n].command, &macro[*n].ch))
		(*n)++;
	    fscanf (f, ";\n");
	} while (s[0] != k[0] || s[1] != k[1]);
	fclose (f);
	return 1;
    } else
	Cerrordialogue (edit->widget->winid, 20, 20, " Load macro ", get_sys_error (" Error trying to open macro file. "));
    return 0;
}


/* returns 1 on success */
int edit_save_confirm_cmd (WEdit * edit)
{
    char *f;

#ifdef MIDNIGHT
    if (edit_confirm_save) {
	f = catstrs (" Confirm save file: ", edit->filename, " ? ", 0);
	if (query_dialog (" Save file ", f, 0, 2, " Save ", " Cancel "))
#else
    if (edit_confirm_save) {
	f = catstrs (" Confirm save file: ", edit->dir, edit->filename, " ? ", 0);
	if (Cquerydialogue (edit->widget->winid, 20, 20, " Save file ", f, " Save ", " Cancel ", NULL))
#endif
	    return 0;
    }
    return edit_save_cmd (edit);
}


/* returns 1 on success */
int edit_save_cmd (WEdit * edit)
{
    edit->force |= REDRAW_COMPLETELY;
    if (!edit_save_file (edit, catstrs (edit->dir, edit->filename, 0)))
	return edit_save_as_cmd (edit);
    edit->modified = 0;
    return 1;
}


/* returns 1 on success */
int edit_new_cmd (WEdit * edit)
{
    edit->force |= REDRAW_COMPLETELY;
    if (edit->modified)
#ifdef MIDNIGHT
	if (query_dialog (" Warning ", " Continue discards unsaved changes. ", 0, 2, " Continue ", " Cancel "))
#else
	if (Cquerydialogue (edit->widget->winid, 20, 20, " Warning ", " Current text was modified without a file save. \n Continue discards these changes. ", " Continue ", " Cancel ", NULL))
#endif
	    return 0;
    edit->modified = 0;
    return edit_renew (edit);	/* if this gives an error, something has really screwed up */
}

int edit_load_cmd (WEdit * edit)
{
    char *exp;
    int mod = 0;
    edit->force |= REDRAW_COMPLETELY;
    if (edit->modified) {
	mod = 1;
#ifdef MIDNIGHT
	if (query_dialog (" Warning ", " Continue discards unsaved changes. ", 0, 2, " Continue ", " Cancel "))
#else
	if (Cquerydialogue (edit->widget->winid, 20, 20, "Warning", " Current text was modified without a file save. \n Continue discards these changes. ", " Continue ", " Cancel ", NULL))
#endif
	    return 0;
    }
    edit->modified = 0;

    exp = Cgetloadfile (edit->widget->winid, 40, 40, edit->dir, edit->filename, " Load ");

    if (exp) {
	if (!*exp) {
	    free (exp);
	} else {
	    int file;
	    if ((file = open ((char *) exp, O_RDONLY)) != -1) {
		close (file);
		edit_reload (edit, exp, 0, "");
		split_filename (edit, exp);
		free (exp);
		edit->modified = 0;
		edit->force |= REDRAW_COMPLETELY;
		return 1;
	    } else {
		free (exp);
		Cerrordialogue (edit->widget->winid, 20, 20, " Load ", get_sys_error (" Error trying to open file for reading. "));
	    }
	}
    }
    edit->modified = mod;	/* on fail, restore the modidifed state */
    return 0;
}

/*
   if mark2 is -1 then marking is from mark1 to the cursor.
   Otherwise its between the markers. This handles this.
   Returns 1 if no text is marked.
 */
int eval_marks (WEdit * edit, long *start_mark, long *end_mark)
{
    if (edit->mark1 != edit->mark2) {
	if (edit->mark2 >= 0) {
	    *start_mark = min (edit->mark1, edit->mark2);
	    *end_mark = max (edit->mark1, edit->mark2);
	} else {
	    *start_mark = min (edit->mark1, edit->curs1);
	    *end_mark = max (edit->mark1, edit->curs1);
	}
	return 0;
    } else {
	*start_mark = *end_mark = 0;
	return 1;
    }
}

/*Block copy, move and delete commands */

void edit_block_copy_cmd (WEdit * edit)
{
    long start_mark, end_mark, current = edit->curs1;
    long count;
    char *copy_buf;

    if (eval_marks (edit, &start_mark, &end_mark))
	return;


    copy_buf = malloc (end_mark - start_mark);

/* all that gets pushed are deletes hence little space is used on the stack */

    edit_push_markers (edit);

    count = start_mark;
    while (count < end_mark) {
	copy_buf[end_mark - count - 1] = edit_get_byte (edit, count);
	count++;
    }
    while (count-- > start_mark) {
	edit_insert_ahead (edit, copy_buf[end_mark - count - 1]);
    }
    free (copy_buf);
    edit_scroll_screen_over_cursor (edit);

    if (start_mark < current && end_mark > current)
	edit_set_markers (edit, start_mark, end_mark + end_mark - start_mark);

    edit->force |= REDRAW_PAGE;
}


void edit_block_move_cmd (WEdit * edit)
{
    long count;
    long current;
    char *copy_buf;
    long start_mark, end_mark;

    if (eval_marks (edit, &start_mark, &end_mark))
	return;

    if (start_mark <= edit->curs1 && end_mark >= edit->curs1)
	return;

    if ((end_mark - start_mark) > MAX_STACK)
#ifdef MIDNIGHT
	if (query_dialog (" Warning ", " Block is large, you may not be able to undo this action. ", 0, 2, " Continue ", " Cancel "))
#else
	if (Cquerydialogue (edit->widget->winid, 20, 20, " Warning ", " Block is large, you may not be able to undo this action. ", " Continue ", " Cancel ", NULL))
#endif
	    return;

    copy_buf = malloc (end_mark - start_mark);

    edit_push_markers (edit);

    current = edit->curs1;
    edit_cursor_move (edit, start_mark - edit->curs1);
    edit_scroll_screen_over_cursor (edit);

    count = start_mark;
    while (count < end_mark) {
	copy_buf[end_mark - count - 1] = edit_delete (edit);
	count++;
    }
    edit_scroll_screen_over_cursor (edit);

    if (edit_cursor_move (edit, current - edit->curs1 -
	   (((current - edit->curs1) > 0) ? end_mark - start_mark : 0)));
    edit_scroll_screen_over_cursor (edit);

    while (count-- > start_mark) {
	edit_insert_ahead (edit, copy_buf[end_mark - count - 1]);
	edit_set_markers (edit, edit->curs1, edit->curs1 + end_mark - start_mark);
    }
    edit_scroll_screen_over_cursor (edit);

    free (copy_buf);
    edit->force |= REDRAW_PAGE;
}


/* returns 1 if canceelled by user */
int edit_block_delete_cmd (WEdit * edit)
{
    long count;
    long start_mark, end_mark;

    if (eval_marks (edit, &start_mark, &end_mark)) {
	start_mark = edit_bol (edit, edit->curs1);
	end_mark = edit_eol (edit, edit->curs1) + 1;
    }
    if ((end_mark - start_mark) > MAX_STACK)
#ifdef MIDNIGHT
	if (query_dialog (" Warning ", " Block is large, you may not be able to undo this action. ", 0, 2, " Continue ", " Cancel "))
#else
	if (Cquerydialogue (edit->widget->winid, 20, 20, " Warning ", " Block is large, you may not be able to undo this action. ", " Continue ", " Cancel ", NULL))
#endif
	    return 1;

    edit_push_markers (edit);

    edit_cursor_move (edit, start_mark - edit->curs1);
    edit_scroll_screen_over_cursor (edit);

    count = start_mark;
    while (count < end_mark) {
	edit_delete (edit);
	count++;
    }

    edit_set_markers (edit, 0, 0);
    edit->force |= REDRAW_PAGE;

    return 0;
}


#ifdef MIDNIGHT

#define INPUT_INDEX 8
#define SEARCH_DLG_HEIGHT 9
#define REPLACE_DLG_HEIGHT 14
#define B_REPLACE_ALL B_USER+1
#define B_SKIP_REPLACE B_USER+2

int edit_replace_prompt (WEdit * edit, char *replace_text, int xpos, int ypos)
{
    if (replace_prompt) {
	QuickWidget quick_widgets[] =
	{
	    {quick_button, 8, 11, 3, 6, "[ Cancel ]", 'c', 2, B_CANCEL, 0,
	     0, XV_WLAY_DONTCARE, NULL},
	    {quick_button, 5, 11, 3, 6, "[ Replace all ]", 'a', 10, B_REPLACE_ALL, 0,
	     0, XV_WLAY_DONTCARE, NULL},
	{quick_button, 3, 11, 3, 6, "[ Skip ]", 's', 2, B_SKIP_REPLACE, 0,
	 0, XV_WLAY_DONTCARE, NULL},
	    {quick_button, 1, 11, 3, 6, "[ Replace ]", 'r', 2, B_ENTER, 0,
	     0, XV_WLAY_DONTCARE, NULL},
	    {quick_label, 2, 50, 2, 6, 0,
	     0, 0, 0, 0, 0, XV_WLAY_DONTCARE, 0},
	    {0}};

	quick_widgets[4].text = catstrs (" Replace with: ", replace_text, 0);

	{
	    QuickDialog Quick_input =
	    {66, 6, 0, 0, " Replace ",
	     "[Input Line Keys]", "quick_input", 0 /*quick_widgets */ };

	    Quick_input.widgets = quick_widgets;

	    Quick_input.xpos = xpos;
	    Quick_input.ypos = ypos;
	    return quick_dialog (&Quick_input);
	}
    } else
	return 0;
}



void edit_replace_dialog (WEdit * edit, char **search_text, char **replace_text, char **arg_order)
{
    int treplace_scanf = replace_scanf;
    int treplace_regexp = replace_regexp;
    int treplace_all = replace_all;
    int treplace_prompt = replace_prompt;
    int treplace_whole = replace_whole;
    int treplace_case = replace_case;

    char *tsearch_text;
    char *treplace_text;
    char *targ_order;
    QuickWidget quick_widgets[] =
    {
	{quick_button, 6, 10, 11, REPLACE_DLG_HEIGHT, "[ Cancel ]", 'c', 2, B_CANCEL, 0,
	 0, XV_WLAY_DONTCARE, NULL},
	{quick_button, 2, 10, 11, REPLACE_DLG_HEIGHT, "[ Ok ]", 'o', 2, B_ENTER, 0,
	 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 25, 50, 10, REPLACE_DLG_HEIGHT, "Scanf expression", 'n', 8, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 25, 50, 9, REPLACE_DLG_HEIGHT, "Replace all", 'a', 8, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 25, 50, 8, REPLACE_DLG_HEIGHT, "Prompt on replace", 'p', 0, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 4, 50, 10, REPLACE_DLG_HEIGHT, "Regular exprssn", 'r', 0, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 4, 50, 9, REPLACE_DLG_HEIGHT, "Whole words only", 'w', 0, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 4, 50, 8, REPLACE_DLG_HEIGHT, "Case sensitive", 's', 5, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_input, 3, 50, 7, REPLACE_DLG_HEIGHT, 0, 0, 44, 0, 0,
	 0, XV_WLAY_BELOWCLOSE, 0},
	{quick_label, 2, 50, 6, REPLACE_DLG_HEIGHT, " Enter replacement argument order eg. 3,2,1,4 ", 0, 0, 0, 0,
	 0, XV_WLAY_DONTCARE, 0},
	{quick_input, 3, 50, 5, REPLACE_DLG_HEIGHT, 0, 0, 44, 0, 0,
	 0, XV_WLAY_BELOWCLOSE, 0},
	{quick_label, 2, 50, 4, REPLACE_DLG_HEIGHT, " Enter replacement string", 0, 0, 0, 0,
	 0, XV_WLAY_DONTCARE, 0},
	{quick_input, 3, 50, 3, REPLACE_DLG_HEIGHT, 0, 0, 44, 0, 0,
	 0, XV_WLAY_BELOWCLOSE, 0},
	{quick_label, 2, 50, 2, REPLACE_DLG_HEIGHT, " Enter search string", 0, 0, 0, 0,
	 0, XV_WLAY_DONTCARE, 0},
	{0}};

    quick_widgets[2].result = &treplace_scanf;
    quick_widgets[3].result = &treplace_all;
    quick_widgets[4].result = &treplace_prompt;
    quick_widgets[5].result = &treplace_regexp;
    quick_widgets[6].result = &treplace_whole;
    quick_widgets[7].result = &treplace_case;
    quick_widgets[8].str_result = &targ_order;
    quick_widgets[8].text = *arg_order;
    quick_widgets[10].str_result = &treplace_text;
    quick_widgets[10].text = *replace_text;
    quick_widgets[12].str_result = &tsearch_text;
    quick_widgets[12].text = *search_text;
    {
	QuickDialog Quick_input =
	{50, REPLACE_DLG_HEIGHT, -1, 0, " Replace ",
	 "[Input Line Keys]", "quick_input", 0 /*quick_widgets */ };

	Quick_input.widgets = quick_widgets;

	if (quick_dialog (&Quick_input) != B_CANCEL) {
	    *arg_order = *(quick_widgets[INPUT_INDEX].str_result);
	    *replace_text = *(quick_widgets[INPUT_INDEX + 2].str_result);
	    *search_text = *(quick_widgets[INPUT_INDEX + 4].str_result);
	    replace_scanf = treplace_scanf;
	    replace_regexp = treplace_regexp;
	    replace_all = treplace_all;
	    replace_prompt = treplace_prompt;
	    replace_whole = treplace_whole;
	    replace_case = treplace_case;
	    return;
	} else {
	    *arg_order = NULL;
	    *replace_text = NULL;
	    *search_text = NULL;
	    return;
	}
    }
}


void edit_search_dialog (WEdit * edit, char **search_text)
{
    int treplace_scanf = replace_scanf;
    int treplace_regexp = replace_regexp;
    int treplace_whole = replace_whole;
    int treplace_case = replace_case;

    char *tsearch_text;
    QuickWidget quick_widgets[] =
    {
	{quick_button, 6, 10, 6, SEARCH_DLG_HEIGHT, "[ Cancel ]", 'c', 2, B_CANCEL, 0,
	 0, XV_WLAY_DONTCARE, NULL},
	{quick_button, 2, 10, 6, SEARCH_DLG_HEIGHT, "[ Ok ]", 'o', 2, B_ENTER, 0,
	 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 25, 50, 5, SEARCH_DLG_HEIGHT, "Scanf expression", 'n', 8, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 25, 50, 4, SEARCH_DLG_HEIGHT, "Regular exprssn", 'r', 0, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 4, 50, 5, SEARCH_DLG_HEIGHT, "Whole words only", 'w', 0, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_checkbox, 4, 50, 4, SEARCH_DLG_HEIGHT, "Case sensitive", 's', 5, 0,
	 0, 0, XV_WLAY_DONTCARE, NULL},
	{quick_input, 3, 50, 3, SEARCH_DLG_HEIGHT, 0, 0, 44, 0, 0,
	 0, XV_WLAY_BELOWCLOSE, 0},
	{quick_label, 2, 50, 2, SEARCH_DLG_HEIGHT, " Enter search string", 0, 0, 0, 0,
	 0, XV_WLAY_DONTCARE, 0},
	{0}};

    quick_widgets[2].result = &treplace_scanf;
    quick_widgets[3].result = &treplace_regexp;
    quick_widgets[4].result = &treplace_whole;
    quick_widgets[5].result = &treplace_case;
    quick_widgets[6].str_result = &tsearch_text;
    quick_widgets[6].text = *search_text;

    {
	QuickDialog Quick_input =
	{50, SEARCH_DLG_HEIGHT, -1, 0, " Search ",
	 "[Input Line Keys]", "quick_input", 0 /*quick_widgets */ };

	Quick_input.widgets = quick_widgets;

	if (quick_dialog (&Quick_input) != B_CANCEL) {
	    *search_text = *(quick_widgets[6].str_result);
	    replace_scanf = treplace_scanf;
	    replace_regexp = treplace_regexp;
	    replace_whole = treplace_whole;
	    replace_case = treplace_case;
	    return;
	} else {
	    *search_text = NULL;
	    return;
	}
    }
}


#else

#define B_ENTER 0
#define B_SKIP_REPLACE 1
#define B_REPLACE_ALL 2
#define B_CANCEL 3

#define R_W 450

extern CWidget *wedit;

void edit_search_replace_dialog (Window parent, int x, int y, char **search_text, char **replace_text, char **arg_order, char *heading, int option)
{
    Window win;
    XEvent xev;
    CEvent cev;
    int h, height = 300;
    CState s;
    int xh, yh, yh2, yh3, xs, ys, o;
    CWidget *m;

    if (replace_text == 0)	/* must be a search dialog */
	height -= 110;

    CBackupState (&s);
    CDisable ("*");

    win = Cdrawheadedwindow ("replace", parent, x, y, R_W, height, heading);
    Cgethintpos (&xh, &h);

    Cwidget ("replace")->position = CALWAYS_ON_TOP;
    o = (Cdrawtext ("replace.t1", win, xh, h, " Enter search text : "))->height;

    o = (32 - o) / 2;
    Cgethintpos (0, &yh);
    Cdrawtextinput ("replace.sinp", win, xh, yh, 10, AUTO_HEIGHT, 256, *search_text);

    if (replace_text) {
	Cgethintpos (0, &yh);
	Cdrawtext ("replace.t2", win, xh, yh, " Enter replace text : ");
	Cgethintpos (0, &yh);
	Cdrawtextinput ("replace.rinp", win, xh, yh, 10, AUTO_HEIGHT, 256, *replace_text);
	Cgethintpos (0, &yh);
	Cdrawtext ("replace.t3", win, xh, yh, " Enter argument order : ");
	Cgethintpos (0, &yh);
	Cdrawtextinput ("replace.ainp", win, xh, yh, 10, AUTO_HEIGHT, 256, *arg_order);
    }
    Cgethintpos (0, &yh);
    yh3 = yh;
    Cdrawswitch ("replace.ww", win, xh, yh, C_BLACK, C_FLAT, replace_whole);
    Cgethintpos (0, &yh2);
    Cdrawtext ("replace.t4", win, 32 + WIDGET_SPACING + xh, yh + o, " Whole words only ");
    Cdrawswitch ("replace.case", win, xh, yh2, C_BLACK, C_FLAT, replace_case);
    Cgethintpos (0, &yh);
    Cdrawtext ("replace.t5", win, 32 + WIDGET_SPACING + xh, yh2 + o, " Case sensitive ");
    Cdrawswitch ("replace.reg", win, xh, yh, C_BLACK, C_FLAT, replace_regexp);
    Cdrawtext ("replace.t6", win, 32 + WIDGET_SPACING + xh, yh + o, " Regular expression ");
    Cgethintpos (&xh, 0);
    xs = xh;
    ys = yh;

    if (replace_text) {
	Cdrawswitch ("replace.pr", win, xh, yh3, C_BLACK, C_FLAT, replace_prompt);
	Cgethintpos (0, &yh);
	Cdrawtext ("replace.t7", win, xh + 32 + WIDGET_SPACING, yh3 + o, " Prompt on replace ");
	Cdrawswitch ("replace.all", win, xh, yh, C_BLACK, C_FLAT, replace_all);
	Cdrawtext ("replace.t8", win, xh + 32 + WIDGET_SPACING, yh + o, " Replace all ");
    }
    Cdrawswitch ("replace.scanf", win, xs, ys, C_BLACK, C_FLAT, replace_scanf);
    Cdrawtext ("replace.t9", win, xs + 32 + WIDGET_SPACING, ys + o, " Scanf expression ");

    Csetsizehintpos ("replace");
    m = Cwidget ("replace");
    Cdrawbitmapbutton ("replace.ok", win, m->width - WIDGET_SPACING - 48 - 2, h, 40, 40, Ccolor (6), C_FLAT, tick_bits);
    Cdrawbitmapbutton ("replace.cancel", win, m->width - WIDGET_SPACING - 48 - 2, h + WIDGET_SPACING + 48, 40, 40, Ccolor (18), C_FLAT, cross_bits);
    Csetwidgetsize ("replace.sinp", m->width - WIDGET_SPACING * 3 - 4 - 48, (Cwidget ("replace.sinp"))->height);
    if (replace_text) {
	Csetwidgetsize ("replace.rinp", m->width - WIDGET_SPACING * 3 - 4 - 48, (Cwidget ("replace.rinp"))->height);
	Csetwidgetsize ("replace.ainp", m->width - WIDGET_SPACING * 3 - 4 - 48, (Cwidget ("replace.ainp"))->height);
    }

    CFocus (Cwidget ("replace.sinp"));

    for (;;) {
	CNextEvent (&xev, &cev);
	if (!strcmp (cev.ident, "replace.cancel") || (CKeySym (&xev) == XK_Escape && xev.type == KeyPress)) {
	    *search_text = 0;
	    break;
	}
	if (!strcmp (cev.ident, "replace.ok") || (CKeySym (&xev) == XK_Return && xev.type == KeyPress)) {
	    if (replace_text) {
		replace_all = Cwidget ("replace.all")->keypressed;
		replace_prompt = Cwidget ("replace.pr")->keypressed;
		*replace_text = strdup (Cwidget ("replace.rinp")->text);
		*arg_order = strdup (Cwidget ("replace.ainp")->text);
	    }
	    *search_text = strdup (Cwidget ("replace.sinp")->text);
	    replace_whole = Cwidget ("replace.ww")->keypressed;
	    replace_case = Cwidget ("replace.case")->keypressed;
	    replace_scanf = Cwidget ("replace.scanf")->keypressed;
	    replace_regexp = Cwidget ("replace.reg")->keypressed;
	    break;
	}
    }
    Cundrawwidget ("replace");
    CRestoreState (&s);
}



void edit_search_dialog (WEdit * edit, char **search_text)
{
    edit_search_replace_dialog (CMain, 20, 20, search_text, 0, 0, " Search ", 0);
}

void edit_replace_dialog (WEdit * edit, char **search_text, char **replace_text, char **arg_order)
{
    edit_search_replace_dialog (CMain, 20, 20, search_text, replace_text, arg_order, " Replace ", 0);
}

int edit_replace_prompt (WEdit * edit, char *replace_text, int xpos, int ypos)
{
    if (replace_prompt) {
	int q;
	char *p, *r = 0;
	r = p = malloc (strlen (replace_text) + NUM_REPL_ARGS * 2);
	strcpy (p, replace_text);
	while ((p = strchr (p, '%'))) {		/* convert "%" to "%%" so no convertion is attempted */
	    memmove (p + 2, p + 1, strlen (p) + 1);
	    *(++p) = '%';
	    p++;
	}
	q = Cquerydialogue (CMain, 20, 20, " Replace ", catstrs (" Replace with: ", r, 0), " Replace ", " Skip ", " Replace all ", " Cancel ", NULL);
	if (r)
	    free (r);
	switch (q) {
	case 0:
	    return B_ENTER;
	case 1:
	    return B_SKIP_REPLACE;
	case 2:
	    return B_REPLACE_ALL;
	case 3:
	    return B_CANCEL;
	}
    }
    return 0;
}



#endif

#define lcase(c) (((c)>='A'&&(c)<='Z')?(c)-'A'+'a':(c))


long sargs[NUM_REPL_ARGS][256 / sizeof (long)];

#define SCANF_ARGS sargs[0], sargs[1], sargs[2], sargs[3], \
		     sargs[4], sargs[5], sargs[6], sargs[7], \
		     sargs[8], sargs[9], sargs[10], sargs[11], \
		     sargs[12], sargs[13], sargs[14], sargs[15]

#define PRINTF_ARGS sargs[argord[0]], sargs[argord[1]], sargs[argord[2]], sargs[argord[3]], \
		     sargs[argord[4]], sargs[argord[5]], sargs[argord[6]], sargs[argord[7]], \
		     sargs[argord[8]], sargs[argord[9]], sargs[argord[10]], sargs[argord[11]], \
		     sargs[argord[12]], sargs[argord[13]], sargs[argord[14]], sargs[argord[15]]

/* This function is a modification of mc-3.2.10/src/view.c:regexp_view_search() */
/* returns -3 on error in pattern, -1 on not found, found_len = 0 if either */
int string_regexp_search (char *pattern, char *string, int len, int match_type, int *found_len)
{
    static regex_t r;
    static char *old_pattern = NULL;
    static int old_type;
    int startpos;

    if (!old_pattern || strcmp (old_pattern, pattern) || old_type != match_type) {
	if (old_pattern) {
	    regfree (&r);
	    free (old_pattern);
	}
	if (regcomp (&r, pattern, REG_EXTENDED | REG_NOSUB)) {
	    *found_len = 0;
	    return -3;
	}
	old_pattern = strdup (pattern);
	old_type = match_type;
    }
    startpos = re_search (&r, string, len, 0, len, NULL);
    if (startpos < 0) {
	*found_len = 0;
	return -1;
    }
    *found_len = re_match (&r, string, len, startpos, NULL);
    return startpos;
}



long edit_find_string (long start, char *exp, int *len, long last_byte, int (*get_byte) (void *, long), void *data)
{
    long p, q;
    long l = strlen (exp), f = 0;
    int n = 0;

    *len = l;

    for (p = 0; p < l; p++)	/* count conversions... */
	if (exp[p] == '%')
	    if (exp[++p] != '%')	/* ...except for "%%" */
		n++;

    if ((n && replace_scanf) || replace_regexp) {
	int c;
	char *buf, *mbuf;

	replace_scanf = (!replace_regexp);	/* can't have both */

	if (!replace_regexp)
	    if (n > NUM_REPL_ARGS - 1)
		return -2;
	buf = malloc (MAX_REPL_LEN * 2 + 2);
	mbuf = buf;
	if (!replace_case)
	    for (p = 0; exp[p] != 0; p++)
		exp[p] = lcase (exp[p]);

	if (replace_case) {
	    for (p = start; p < last_byte && p < start + MAX_REPL_LEN; p++)
		buf[p - start] = (*get_byte) (data, p);
	} else {
	    for (p = start; p < last_byte && p < start + MAX_REPL_LEN; p++) {
		c = (*get_byte) (data, p);
		buf[p - start] = lcase (c);
	    }
	}

	buf[(q = p - start)] = 0;

	if (!replace_regexp)
	    exp = catstrs (exp, "%n", 0);

	while (*buf) {
	    if (replace_regexp) {
		if (buf == mbuf || buf == mbuf + MAX_REPL_LEN / 2) {	/* search the window after it moves half its length */
		    int found_start =
		    string_regexp_search (exp, buf, q, match_normal, len);
		    if (found_start == -3)
			return -3;
		    if (*len)
			return found_start + start;
		}
	    } else {
		if (n == sscanf (buf, exp, SCANF_ARGS)) {
		    *len = *((int *) sargs[n]);
		    return start;
		}
	    }
	    if (q + start < last_byte) {
		if (replace_case) {
		    buf[q] = (*get_byte) (data, q + start);
		} else {
		    c = (*get_byte) (data, q + start);
		    buf[q] = lcase (c);
		}
		q++;
	    }
	    buf[q] = 0;
	    start++;
	    buf++;		/* move the window along */
	    if (buf == mbuf + MAX_REPL_LEN) {	/* the window is about to go past the end of allocated memory, so... */
		memmove (mbuf, buf, strlen (buf) + 1);	/* reset it */
		buf = mbuf;
	    }
	    q--;
	}
	free (mbuf);
    } else {
	if (replace_case) {
	    for (p = start; p <= last_byte - l; p++) {
		if ((*get_byte) (data, p) == exp[0]) {	/* check if first char matches */
		    for (f = 0, q = 0; q < l && f < 1; q++)
			if ((*get_byte) (data, q + p) != exp[q])
			    f = 1;
		    if (f == 0)
			return p;
		}
	    }
	} else {
	    for (p = 0; exp[p] != 0; p++)
		exp[p] = lcase (exp[p]);

	    for (p = start; p <= last_byte - l; p++) {
		if (lcase ((*get_byte) (data, p)) == exp[0]) {
		    for (f = 0, q = 0; q < l && f < 1; q++)
			if (lcase ((*get_byte) (data, q + p)) != exp[q])
			    f = 1;
		    if (f == 0)
			return p;
		}
	    }
	}
    }
    return -2;
}


long edit_find (long search_start, char *exp, int *len, long last_byte, int (*get_byte) (void *, long), void *data)
{				/*front end to find_string to check for
				   whole words */
    long p;
    p = search_start;

    while ((p = edit_find_string (p, exp, len, last_byte, get_byte, data)) >= 0) {
	if (replace_whole) {
/*If the bordering chars are not in option_whole_chars_search then word is whole */
	    if (!strcasechr (option_whole_chars_search, (*get_byte) (data, p - 1))
		&& !strcasechr (option_whole_chars_search, (*get_byte) (data, p + *len)))
		return p;
	} else
	    return p;
	p++;			/*not a whole word so continue search. */
    }
    return p;
}

#define is_digit(x) ((x) >= '0' && (x) <= '9')

#define snprintf(v) { \
		*p1++ = *p++; \
		*p1++ = '%'; \
		*p1++ = 'n'; \
		*p1 = '\0'; \
		sprintf(s,q1,v,&n); \
		s += n; \
	    }

/* this function uses the sprintf command to do a vprintf */
/* it takes pointers to arguments instead of the arguments themselves */
int sprintf_p (char *str, const char *fmt,...)
{
    va_list ap;
    int n;
    char *q, *p, *s = str;
    char q1[32];
    char *p1;

    va_start (ap, fmt);
    p = q = (char *) fmt;

    while ((p = strchr (p, '%'))) {
	n = (int) ((unsigned long) p - (unsigned long) q);
	strncpy (s, q, n);	/* copy stuff between format specifiers */
	s += n;
	*s = 0;
	q = p;
	p1 = q1;
	*p1++ = *p++;
	if (*p == '%') {
	    p++;
	    *s++ = '%';
	    q = p;
	    continue;
	}
	if (*p == 'n') {
	    p++;
/* do nothing */
	    q = p;
	    continue;
	}
	if (*p == '#')
	    *p1++ = *p++;
	if (*p == '0')
	    *p1++ = *p++;
	if (*p == '-')
	    *p1++ = *p++;
	if (*p == '+')
	    *p1++ = *p++;
	if (*p == '*') {
	    p++;
	    strcpy (p1, itoa (*va_arg (ap, int *)));	/* replace field width with a number */
	    p1 += strlen (p1);
	} else {
	    while (is_digit (*p))
		*p1++ = *p++;
	}
	if (*p == '.')
	    *p1++ = *p++;
	if (*p == '*') {
	    p++;
	    strcpy (p1, itoa (*va_arg (ap, int *)));	/* replace precision with a number */
	    p1 += strlen (p1);
	} else {
	    while (is_digit (*p))
		*p1++ = *p++;
	}
/* flags done, now get argument */
	if (*p == 's') {
	    snprintf (va_arg (ap, char *));
	} else if (*p == 'h') {
	    if (strchr ("diouxX", *p))
		snprintf (*va_arg (ap, short *));
	} else if (*p == 'l') {
	    *p1++ = *p++;
	    if (strchr ("diouxX", *p))
		snprintf (*va_arg (ap, long *));
	} else if (strchr ("cdiouxX", *p)) {
	    snprintf (*va_arg (ap, int *));
	} else if (*p == 'L') {
	    *p1++ = *p++;
	    if (strchr ("EefgG", *p))
		snprintf (*va_arg (ap, long double *));
	} else if (strchr ("EefgG", *p)) {
	    snprintf (*va_arg (ap, double *));
	} else if (strchr ("DOU", *p)) {
	    snprintf (*va_arg (ap, long *));
	} else if (*p == 'p') {
	    snprintf (*va_arg (ap, void **));
	}
	q = p;
    }
    va_end (ap);
    sprintf (s, q);		/* print trailing leftover */
    return (unsigned long) s - (unsigned long) str + strlen (s);
}

static void regexp_error (WEdit *edit)
{
    Cerrordialogue (edit->widget->winid, 20, 20, " Error ", " Invalid regular expression ");
}

void edit_replace_cmd (WEdit * edit, int again)
{
    static char *old1 = NULL;
    static char *old2 = NULL;
    static char *old3 = NULL;
    char *exp1 = "";
    char *exp2 = "";
    char *exp3 = "";
    int replace_yes;
    int replace_continue;
    int i = 0;
    long times_replaced = 0;
    char fin_string[32];
    int argord[NUM_REPL_ARGS];

    edit->force |= REDRAW_COMPLETELY;

    exp1 = old1 ? old1 : exp1;
    exp2 = old2 ? old2 : exp2;
    exp3 = old3 ? old3 : exp3;

    if (again) {
	if (!old1 || !old2)
	    return;
	exp1 = strdup (old1);
	exp2 = strdup (old2);
	exp3 = strdup (old3);
    } else {
	push_action (edit, KEY_PRESS + edit->start_display);
	edit_replace_dialog (edit, &exp1, &exp2, &exp3);
    }

    if (!exp1 || !*exp1) {
	edit->force = REDRAW_COMPLETELY;
	if (exp1) {
	    free (exp1);
	    free (exp2);
	    free (exp3);
	}
	return;
    }
    if (old1)
	free (old1);
    if (old2)
	free (old2);
    if (old3)
	free (old3);
    old1 = strdup (exp1);
    old2 = strdup (exp2);
    old3 = strdup (exp3);

    {
	char *s;
	int ord, i;
	while ((s = strchr (exp3, ' ')))
	    memmove (s, s + 1, strlen (s));
	s = exp3;
	for (i = 0; i < NUM_REPL_ARGS; i++) {
	    if ((unsigned long) s != 1 && s < exp3 + strlen (exp3)) {
		if ((ord = atoi (s)))
		    argord[i] = ord - 1;
		else
		    argord[i] = i;
		s = strchr (s, ',') + 1;
	    } else
		argord[i] = i;
	}
    }

    replace_continue = replace_all;

    do {
	int len = 0;
	long new_start;
	new_start = edit_find (edit->search_start, exp1, &len, edit->last_byte,
			(int (*) (void *, long)) edit_get_byte, (void *) edit);
/*
long edit_find (long search_start, char *exp, int *len, long last_byte, int (*get_byte) (void *, long index), void *data)
*/
	if (new_start == -3) {
	    regexp_error (edit);
	    break;
	}
	edit->search_start = new_start;
	/*returns negative on not found or error in pattern */

	if (edit->search_start >= 0) {
	    edit->found_start = edit->search_start;
	    i = edit->found_len = len;

	    edit_cursor_move (edit, edit->search_start - edit->curs1);
	    edit_scroll_screen_over_cursor (edit);

	    replace_yes = 1;

	    if (replace_prompt) {
		int l;
		l = edit->curs_row - edit->num_widget_lines / 3;
		if (l > 0)
		    edit_scroll_downward (edit, l);
		if (l < 0)
		    edit_scroll_upward (edit, -l);

		edit_scroll_screen_over_cursor (edit);
		edit->force |= REDRAW_PAGE;
		edit_render_keypress (edit);

		/*so that undo stops at each query */
		push_key_press (edit);

		switch (edit_replace_prompt (edit, exp2,	/*and prompt 2/3 down */
					     edit->num_widget_columns / 2 - 33, edit->num_widget_lines * 2 / 3)) {
		case B_ENTER:
		    break;
		case B_SKIP_REPLACE:
		    replace_yes = 0;
		    break;
		case B_REPLACE_ALL:
		    replace_prompt = 0;
		    replace_continue = 1;
		    break;
		case B_CANCEL:
		    replace_yes = 0;
		    replace_continue = 0;
		    break;
		}
	    }
	    if (replace_yes) {	/* delete then insert new */
		if (replace_scanf) {
		    char repl_str[MAX_REPL_LEN + 2];
		    if (sprintf_p (repl_str, exp2, PRINTF_ARGS) >= 0) {
			times_replaced++;
			while (i--)
			    edit_delete (edit);
			while (repl_str[++i])
			    edit_insert (edit, repl_str[i]);
		    } else {
			Cerrordialogue (edit->widget->winid, 20, 20, " Replace ", " Error in replacement format string. ");
			replace_continue = 0;
		    }
		} else {
		    times_replaced++;
		    while (i--)
			edit_delete (edit);
		    while (exp2[++i])
			edit_insert (edit, exp2[i]);
		}
		edit->found_len = i;
	    }
	    edit->search_start += i;	/*so that we don't find the same string again */
	    edit_scroll_screen_over_cursor (edit);
	} else {
	    edit->search_start = edit->curs1;	/* try and find from right here for next search */
	    update_curs_col (edit);

	    edit->force |= REDRAW_PAGE;
	    edit_render_keypress (edit);
	    if (times_replaced) {
		sprintf (fin_string, " %ld replacements made. ", times_replaced);
		Cmessagedialogue (edit->widget->winid, 20, 20, " Replace ", fin_string);
	    } else
		Cmessagedialogue (edit->widget->winid, 20, 20, " Replace ", " Search string not found. ");
	    replace_continue = 0;
	}
    } while (replace_continue);

    free (exp1);
    free (exp2);
    free (exp3);
    edit->force = REDRAW_COMPLETELY;
    edit_scroll_screen_over_cursor (edit);
}




void edit_search_cmd (WEdit * edit, int again)
{
    static char *old = NULL;
    char *exp = "";

    exp = old ? old : exp;
    if (again) {		/*ctrl-hotkey for search again. */
	if (!old)
	    return;
	exp = strdup (old);
    } else {
	edit_search_dialog (edit, &exp);
	push_action (edit, KEY_PRESS + edit->start_display);
    }

    if (exp) {
	if (*exp) {
	    int len = 0;
	    if (old)
		free (old);
	    old = strdup (exp);

	    edit->search_start = edit_find (edit->search_start, exp, &len, edit->last_byte,
			(int (*) (void *, long)) edit_get_byte, (void *) edit);

	    if (edit->search_start >= 0) {
		edit->found_start = edit->search_start;
		edit->found_len = len;

		edit_cursor_move (edit, edit->search_start - edit->curs1);
		edit_scroll_screen_over_cursor (edit);

		edit->search_start++;
	    } else if (edit->search_start == -3) {
		edit->search_start = edit->curs1;
		regexp_error (edit);
	    } else {
		edit->search_start = edit->curs1;
		Cerrordialogue (edit->widget->winid, 20, 20, " Search ", " Search search string not found. ");
	    }
	}
	free (exp);
    }
    edit->force |= REDRAW_COMPLETELY;
    edit_scroll_screen_over_cursor (edit);
}


/* Real edit only */
void edit_quit_cmd (WEdit * edit)
{
    push_action (edit, KEY_PRESS + edit->start_display);

    edit->force |= REDRAW_COMPLETELY;
    if (edit->modified) {
#ifdef MIDNIGHT
	switch (query_dialog (" Quit ", " File was modified, Save with exit? ", 0, 3, " Cancel quit ", " Yes ", " No ")) {
#else
	switch (Cquerydialogue (edit->widget->winid, 20, 20, " Quit ", " Current text was modified without a file save. \n Save with exit? ", " Cancel quit ", " Yes ", " No ", NULL)) {
#endif
	case 1:
	    edit_push_markers (edit);
	    edit_set_markers (edit, 0, 0);
	    if (!edit_save_cmd (edit))
		return;
	case 2:
#ifdef MIDNIGHT
	    edit->widget.parent->running = 0;
#else
	    edit->stopped = 1;
#endif
	    return;
	case 0:
	    return;
	}
    } else
#ifdef MIDNIGHT
	edit->widget.parent->running = 0;
#else
	edit->stopped = 1;
#endif
}

#define TEMP_BUF_LEN 1024

/* returns a null terminated length of text. Result must be free'd */
unsigned char *edit_get_block (WEdit * edit, long start, long finish)
{
    unsigned char *s, *r;
    r = s = malloc (finish - start + 1);
    while (start<finish)
	*s++ = edit_get_byte(edit, start++);
    *s = 0;
    return r;
}

/* save block, returns 1 on success */
int edit_save_block (WEdit * edit, const char *filename, long start, long finish)
{
    long i = start, end, filelen = finish - start;
    int file;
    unsigned char *buf;

#ifdef MIDNIGHT
    if ((file = mc_open ((char *) filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	return 0;
#else
    if ((file = open ((char *) filename, O_WRONLY | O_TRUNC)) == -1)
	if ((file = creat ((char *) filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
	    return 0;
#endif

    buf = malloc (TEMP_BUF_LEN);
    while (start != finish) {
	end = min (finish, start + TEMP_BUF_LEN);
	for (; i < end; i++)
	    buf[i - start] = edit_get_byte (edit, i);
	filelen -= write (file, (char *) buf, end - start);
	start = end;
    }
    free (buf);
    close (file);
    if (filelen)
	return 0;
    return 1;
}

#ifndef MIDNIGHT

/* copies a block to the XWindows buffer */
static int edit_XStore_block (WEdit * edit, long start, long finish)
{
    unsigned long len = finish - start, i;
    char *buf;

    buf = malloc (len);
    if (buf) {
	for (i = 0; i < len; i++, start++)
	    buf[i] = edit_get_byte (edit, start);
	XStoreBytes (CDisplay, buf, (int) len);		/* note that XStoreBytes takes an integer length */
	free (buf);
	return 0;
    } else
	return 1;
}

int edit_copy_to_X_buf_cmd (WEdit * edit)
{
    long start_mark, end_mark;
    if (eval_marks (edit, &start_mark, &end_mark))
	return 0;
    if (edit_XStore_block (edit, start_mark, end_mark)) {
	Cerrordialogue (edit->widget->winid, 20, 20, " Copy to X clipboard ", " Block to big, cannot allocate memory. ");
	return 1;
    }
    edit_mark_cmd (edit, 1);
    return 0;
}

int edit_cut_to_X_buf_cmd (WEdit * edit)
{
    long start_mark, end_mark;
    if (eval_marks (edit, &start_mark, &end_mark))
	return 0;
    if (edit_XStore_block (edit, start_mark, end_mark)) {
	Cerrordialogue (edit->widget->winid, 20, 20, " Copy to X clipboard ", " Block to big, cannot allocate memory. ");
	return 1;
    }
    edit_block_delete_cmd (edit);
    edit_mark_cmd (edit, 1);
    return 0;
}

void paste_from_X_buf_cmd (WEdit * edit)
{
    char *s;
    int nbytes;

    s = XFetchBytes (CDisplay, &nbytes);
    if (s && nbytes) {
	int i;
	for (i = nbytes - 1; i >= 0; i--)	/* last byte first or else we'll write everything backwards */
	    edit_insert_ahead (edit, s[i]);
	XFree (s);
    }
    edit->force |= REDRAW_PAGE;
}

#else				/* MIDNIGHT */

/* copies a block to clipboard file */
static int edit_XStore_block (WEdit * edit, long start, long finish)
{
    return edit_save_block (edit, catstrs (home_dir, CLIP_FILE, 0), start, finish);
}

int edit_copy_to_X_buf_cmd (WEdit * edit)
{
    long start_mark, end_mark;
    if (eval_marks (edit, &start_mark, &end_mark))
	return 0;
    if (!edit_XStore_block (edit, start_mark, end_mark)) {
	Cerrordialogue (edit->widget->winid, 20, 20, " Copy to clipboard ", get_sys_error (" Unable to save to file. "));
	return 1;
    }
    edit_mark_cmd (edit, 1);
    return 0;
}

int edit_cut_to_X_buf_cmd (WEdit * edit)
{
    long start_mark, end_mark;
    if (eval_marks (edit, &start_mark, &end_mark))
	return 0;
    if (!edit_XStore_block (edit, start_mark, end_mark)) {
	Cerrordialogue (edit->widget->winid, 20, 20, " Cut to clipboard ", " Unable to save to file. ");
	return 1;
    }
    edit_block_delete_cmd (edit);
    edit_mark_cmd (edit, 1);
    return 0;
}

void paste_from_X_buf_cmd (WEdit * edit)
{
    edit_insert_file (edit, catstrs (home_dir, CLIP_FILE, 0));
}

#endif				/* MIDMIGHT */

void edit_goto_cmd (WEdit *edit)
{
    char *f;
    static int l = 0;
#ifdef MIDNIGHT
    char s[12];
    sprintf (s, "%d", l);
    f = input_dialog (" Goto line ", " Enter line: ", l ? s : "");
#else
    f = Cinputdialog (edit->widget->winid, 20, 20, 150, l ? itoa (l) : "", " Goto line ", " Enter line: ");
#endif
    if (f) {
	if (*f) {
	    l = atoi (f);
	    edit_move_display (edit, l - edit->num_widget_lines / 2 - 1);
	    edit_move_to_line (edit, l - 1);
	    edit->force |= REDRAW_COMPLETELY;
	    free (f);
	}
    }
}

/*returns 1 on success */
int edit_save_block_cmd (WEdit * edit) {
    long start_mark, end_mark;
    char *exp;
    if (eval_marks (edit, &start_mark, &end_mark))
	return 1;

    exp = Cgetsavefile (edit->widget->winid, 40, 40, edit->dir, catstrs (home_dir, CLIP_FILE, 0), " Save Block ");

    edit->force |= REDRAW_COMPLETELY;
    push_action (edit, KEY_PRESS + edit->start_display);

    if (exp) {
	if (!*exp) {
	    free (exp);
	    return 0;
	} else {
	    if (edit_save_block (edit, exp, start_mark, end_mark)) {
		free (exp);
		edit->force |= REDRAW_COMPLETELY;
		return 1;
	    } else {
		free (exp);
		edit->force |= REDRAW_COMPLETELY;
		Cerrordialogue (edit->widget->winid, 20, 20, " Save Block ", get_sys_error (" Error trying to save file. "));
		return 0;
	    }
	}
    } else
	return 0;
}


/* inserts a file at the cursor, returns 1 on success */
int edit_insert_file (WEdit * edit, const char *filename) {
    int i, file, blocklen;
    long current = edit->curs1;
    unsigned char *buf;

    if ((file = open ((char *) filename, O_RDONLY)) == -1)
	 return 0;
     buf = malloc (TEMP_BUF_LEN);
    while ((blocklen = read (file, (char *) buf, TEMP_BUF_LEN)) > 0) {
	for (i = 0; i < blocklen; i++)
	    edit_insert (edit, buf[i]);
    } edit_cursor_move (edit, current - edit->curs1);
    free (buf);
    close (file);
    if (blocklen)
	return 0;
    return 1;
}


/* returns 1 on success */
int edit_insert_file_cmd (WEdit * edit) {
    char *exp = Cgetloadfile (edit->widget->winid, 40, 40, edit->dir, catstrs (home_dir, CLIP_FILE, 0), " Insert File ");
    edit->force |= REDRAW_COMPLETELY;

    push_action (edit, KEY_PRESS + edit->start_display);

    if (exp) {
	if (!*exp) {
	    free (exp);
	    return 0;
	} else {
	    if (edit_insert_file (edit, exp)) {
		free (exp);
		return 1;
	    } else {
		free (exp);
		Cerrordialogue (edit->widget->winid, 20, 20, " Insert file ", get_sys_error (" Error trying to insert file. "));
		return 0;
	    }
	}
    } else
	return 0;
}

/* sorts a block, returns -1 on system fail, 1 on cancel and 0 on success */
int edit_sort_cmd (WEdit * edit)
{
    static char *old = 0;
    char *exp;
    long start_mark, end_mark;
    int e;

    if (eval_marks (edit, &start_mark, &end_mark)) {
	Cerrordialogue (edit->widget->winid, 20, 20, " Process block ", " You must first highlight a block of text. ");
	return 0;
    }
    edit_save_block (edit, catstrs (home_dir, BLOCK_FILE, 0), start_mark, end_mark);

    exp = old ? old : "";

#ifdef MIDNIGHT
    exp = input_dialog (" Run Sort ", " Enter sort options (see manpage) separated by whitespace: ", "");
#else
    exp = Cinputdialog (0, 0, 0, 200, exp, " Run Sort ", " Enter sort options (see manpage) separated by whitespace: ");
#endif

    if (!exp)
	return 1;
    if (old)
	free (old);
    old = exp;

#ifdef MIDNIGHT
    e = system (catstrs (" sort ", exp, " ", home_dir, BLOCK_FILE, " > ", home_dir, TEMP_FILE, 0));
#else
    e = Csystem (catstrs (" sort ", exp, " ", home_dir, BLOCK_FILE, " > ", home_dir, TEMP_FILE, 0));
#endif
    if (e) {
	if (e == -1 || e == 127) {
	    Cerrordialogue (edit->widget->winid, 20, 20, " Sort ", get_sys_error (" Error trying to execute sort command "));
	} else {
	    char q[8];
	    sprintf (q, "%d ", e);
	    Cerrordialogue (edit->widget->winid, 20, 20, " Sort ", catstrs (" Sort returned non-zero: ", q, 0));
	}
	return -1;
    }

    edit->force |= REDRAW_COMPLETELY;

    if (edit_block_delete_cmd (edit))
	return 1;
    edit_insert_file (edit, catstrs (home_dir, TEMP_FILE, 0));
    return 0;
}


/* if block is 1, a block must be highlighted and the shell command
   processes it. If block is 0 the shell command is a straight system
   command, that just produces some output which is to be inserted */
void edit_block_process_cmd (WEdit * edit, const char *shell_cmd, int block)
{
    long start_mark, end_mark;
    struct stat s;
    char *f = NULL, *b = NULL;

    if (block) {
	if (eval_marks (edit, &start_mark, &end_mark)) {
	    Cerrordialogue (edit->widget->winid, 20, 20, " Process block ", " You must first highlight a block of text. ");
	    return;
	}
	edit_save_block (edit, b = catstrs (home_dir, BLOCK_FILE, 0), start_mark, end_mark);
#ifdef MIDNIGHT
	my_system (0, shell, catstrs (home_dir, shell_cmd, 0));
#else
	Csystem (catstrs ("/bin/sh ", home_dir, shell_cmd, 0));
#endif
    } else {
#ifdef MIDNIGHT
	my_system (0, shell, shell_cmd);
#else
	Csystem (catstrs ("/bin/sh ", home_dir, shell_cmd, 0));
#endif
    }

    edit->force |= REDRAW_COMPLETELY;

    f = catstrs (home_dir, ERROR_FILE, 0);

    if (block) {
	if (stat (f, &s) == 0) {
	    if (!s.st_size) {	/* no error messages */
		if (edit_block_delete_cmd (edit))
		    return;
		edit_insert_file (edit, b);
		return;
	    } else {
#ifndef MIDNIGHT
		f = loadfile(f, 0);
		if (f) {	/* if the stat did not fail, niether should load file */
		    Ctextboxmessagedialog (CMain, 20, 20, 80, 25, catstrs (" Error messages of  ", shell_cmd, " ", 0), f, 0);
		    free (f);
		}
		return;
#else
		edit_insert_file (edit, f);
		return;
#endif
	    }
	} else {
	    Cerrordialogue (edit->widget->winid, 20, 20, " Process block ", get_sys_error (" Error trying to stat file "));
	    return;
	}
    }
}

int edit_execute_command (WEdit * edit, int command, int char_for_insertion);

/* prints at the cursor */
/* returns the number of chars printed */
int edit_print_string (WEdit * e, const char *s)
{
    int i = 0;
    while (s[i])
	edit_execute_command (e, -1, s[i++]);
    e->force |= REDRAW_COMPLETELY;
    Cedit_update_screen (e);
    return i;
}

int edit_printf (WEdit * e, const char *fmt,...)
{
    int i;
    va_list pa;
    char s[1024];
    va_start (pa, fmt);
    sprintf (s, fmt, pa);
    i = edit_print_string (e, s);
    va_end (pa);
    return i;
}
