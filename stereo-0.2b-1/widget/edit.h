#ifndef __EDIT_H
#define __EDIT_H

#ifdef MIDNIGHT

#include <stdio.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include "src/tty.h"
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <malloc.h>

#else

#include "global.h"
#include <stdio.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <my_string.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdlib.h>
#include <stdarg.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "regex.h"

#endif

#ifndef MIDNIGHT

#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"
#include "coolwidget.h"
#include "app_glob.c"
#include "coollocal.h"
#include "dialog.h"
#include "stringtools.h"
#include "editoptions.h"

#else

#include "src/main.h"		/* for char *shell */
#include "src/mad.h"
#include "src/dlg.h"
#include "src/widget.h"
#include "src/color.h"
#include "src/dialog.h"
#include "src/mouse.h"
#include "src/global.h"
#include "src/help.h"
#include "src/key.h"
#include "src/wtools.h"		/* for QuickWidgets */
#include "src/win.h"
#include "vfs/vfs.h"
#include "src/menu.h"
#include "src/regex.h"
#define WANT_WIDGETS

#define WIDGET_COMMAND (WIDGET_USER + 10)
#define N_menus 5

#endif

#define SEARCH_DIALOG_OPTION_NO_SCANF	1
#define SEARCH_DIALOG_OPTION_NO_REGEX	2
#define SEARCH_DIALOG_OPTION_NO_CASE	4

#define CLIP_FILE "/cedit/cooledit.clip"
#define MACRO_FILE "/cedit/cooledit.macros"
#define BLOCK_FILE "/cedit/cooledit.block"
#define ERROR_FILE "/cedit/cooledit.error"
#define TEMP_FILE "/cedit/cooledit.temp"
#define EDIT_DIR "/cedit"

#define EDIT_KEY_EMULATION_NORMAL 0
#define EDIT_KEY_EMULATION_EMACS  1

#define REDRAW_LINE_ABOVE    (1 << 0)
#define REDRAW_LINE_BELOW    (1 << 1)
#define REDRAW_AFTER_CURSOR  (1 << 2)
#define REDRAW_BEFORE_CURSOR (1 << 3)
#define REDRAW_PAGE          (1 << 4)
#define REDRAW_IN_BOUNDS     (1 << 5)
#define REDRAW_CHAR_ONLY     (1 << 6)
#define REDRAW_COMPLETELY    (1 << 7)

#define MOD_ABNORMAL 1
#define MOD_UNDERLINED 2
#define MOD_BOLD 4
#define MOD_HIGHLIGHTED 8
#define MOD_MARKED 16
#define MOD_ITALIC 32
#define MOD_CURSOR 64

#ifndef MIDNIGHT
#define EDIT_TEXT_HORIZONTAL_OFFSET 4
#define EDIT_TEXT_VERTICAL_OFFSET 3
#else
#define EDIT_TEXT_HORIZONTAL_OFFSET 0
#define EDIT_TEXT_VERTICAL_OFFSET 1
#define FONT_OFFSET_X 0
#define FONT_OFFSET_Y 0

#endif

#define EDIT_RIGHT_EXTREME 0
#define EDIT_LEFT_EXTREME 0
#define EDIT_TOP_EXTREME 0
#define EDIT_BOTTOM_EXTREME 0

#define MAX_MACRO_LENGTH 256

/*there are a maximum of ... */
#define MAXBUFF 1024
/*... edit buffers, each of which is ... */
#define EDIT_BUF_SIZE 16384
/* ...bytes in size. */

/*x / EDIT_BUF_SIZE equals x >> ... */
#define S_EDIT_BUF_SIZE 14

/* x % EDIT_BUF_SIZE is equal to x && ... */
#define M_EDIT_BUF_SIZE 16383

#define SIZE_LIMIT (EDIT_BUF_SIZE * (MAXBUFF - 2))
/* Note a 16k stack is 64k of data and enough to hold (usually) around 10
   pages of undo info. */
#define STACK_BITS 14
#define MAX_STACK (1<<(STACK_BITS-1))	/* the most that one command can occupy without giving a warning */
#define STACK_SIZE (1<<STACK_BITS)
#define M_STACK_SIZE (STACK_SIZE-1)
/*.. used same as M_EDIT_BUF_SIZE */

/*some codes that may be pushed onto or returned from the undo stack: */
#define CURS_LEFT 601
#define CURS_RIGHT 602
#define DELETE 603
#define BACKSPACE 604
#define STACK_BOTTOM 605
#define CURS_LEFT_LOTS 606
#define CURS_RIGHT_LOTS 607
#define MARK_1 1000
#define MARK_2 700000000
#define KEY_PRESS 1400000000

/*Tabs spaces: (sofar only HALF_TAB_SIZE is used: */
#define TAB_SIZE		option_tab_spacing
#define HALF_TAB_SIZE		((int) option_tab_spacing / 2)

struct macro {
    short command;
    short ch;
};

struct editor_widget {
#ifdef MIDNIGHT
    Widget widget;
#else
    struct cool_widget *widget;
#endif
#define from_here num_widget_lines
    int num_widget_lines;
    int num_widget_columns;

#ifdef MIDNIGHT
    int have_frame;
#else
    int stopped;
#endif

    char *filename;		/* Name of the file */
    char *dir;			/* current directory */

/* dynamic buffers and curser position for editor: */
    long curs1;			/*position of the cursor from the beginning of the file. */
    long curs2;			/*position from the end of the file */
    unsigned char *buffers1[MAXBUFF + 1];	/*all data up to curs1 */
    unsigned char *buffers2[MAXBUFF + 1];	/*all data from end of file down to curs2 */

/* search variables */
    long search_start;		/* First character to start searching from */
    int found_len;		/* Length of found string or 0 if none was found */
    long found_start;		/* the found word from a search - start position */

/* display information */
    long last_byte;		/* Last byte of file */
    long start_display;		/* First char displayed */
    long start_col;		/* First displayed column, negative */
    long curs_row;		/*row position of curser on the screen */
    long curs_col;		/*column position on screen */
    unsigned char force;	/* how much of the screen do we redraw? */
    unsigned char overwrite;
    unsigned char modified;	/*has the file been changed?: 1 if char inserted or
				   deleted at all since last load or save */
    unsigned char highlight;
    long prev_col;		/*recent column position of the curser - used when moving
				   up or down past lines that are shorter than the current line */
    long curs_line;		/*line number of the cursor. */
    long start_line;		/*line nummber of the top of the page */

/* file info */
    long total_lines;		/*total lines in the file */
    long mark1;			/*position of highlight start */
    long mark2;			/*position of highlight end */
/* user options */
    int wrap_len;

/* undo stack and pointers */
    unsigned long stack_pointer;
    long *undo_stack;
    unsigned long stack_bottom;
    int (*user_defined_key) (unsigned int state, unsigned int keycode);

    int to_here;		/* dummy marker */

/* macro stuff */
    int macro_i;		/* -1 if not recording index to macro[] otherwise */
    struct macro macro[MAX_MACRO_LENGTH];
};

typedef struct editor_widget WEdit;

#ifndef MIDNIGHT

void edit_render_expose (WEdit * edit, XExposeEvent * xexpose);
void edit_render_tidbits (struct cool_widget *widget);
int eh_editor (CWidget * w, XEvent * xevent, CEvent * cwevent);
void edit_draw_menus (Window parent, int x, int y);
void edit_run_make (void);
void edit_change_directory (void);
void xprint_to_widget (Window win, long row, int start_col, float start_col_real, long end_col, unsigned short line[]);
int edit_man_page_cmd (WEdit * edit);
void edit_search_replace_dialog (Window parent, int x, int y, char **search_text, char **replace_text, char **arg_order, char *heading, int option);
void edit_search_dialog (WEdit * edit, char **search_text);
long edit_find (long search_start, char *exp, int *len, long last_byte, int (*get_byte) (void *, long index), void *data);
void edit_set_foreground_colors (unsigned long normal, unsigned long bold, unsigned long italic);
void edit_set_background_colors (unsigned long normal, unsigned long abnormal, unsigned long marked, unsigned long marked_abnormal, unsigned long highlighted);
void edit_set_cursor_color (unsigned long c);
void draw_options_dialog (Window parent, int x, int y);

#else

int edit_drop_hotkey_menu (WEdit * e, int key);
void edit_menu_cmd (WEdit * e);
void edit_init_menu_emacs (void);
void edit_init_menu_normal (void);
void edit_done_menu (void);
int edit_raw_key_query (char *heading, char *query, int cancel);

#endif

int edit_get_byte (WEdit * edit, long byte_index);
int load_edit_file (WEdit * edit, const char *filename, const char *text);
int edit_count_lines (WEdit * edit, long current, int upto);
long edit_move_forward (WEdit * edit, long current, int lines, long upto);
float edit_move_forward3 (WEdit * edit, long current, int cols, long upto);
long edit_move_backward (WEdit * edit, long current, int lines);
int edit_translate_key (WEdit * edit, unsigned int x_keycode, long x_key, int x_state, int *cmd, int *ch);
void edit_scroll_screen_over_cursor (WEdit * edit);
void edit_render_keypress (WEdit * edit);
void edit_scroll_upward (WEdit * edit, unsigned long i);
void edit_scroll_downward (WEdit * edit, int i);
void edit_scroll_right (WEdit * edit, int i);
void edit_scroll_left (WEdit * edit, int i);
int edit_get_col (WEdit * edit);
long edit_bol (WEdit * edit, long current);
long edit_eol (WEdit * edit, long current);
void update_curs_row (WEdit * edit);
void update_curs_col (WEdit * edit);

void edit_block_copy_cmd (WEdit * edit);
void edit_block_move_cmd (WEdit * edit);
int edit_block_delete_cmd (WEdit * edit);

int edit_delete (WEdit * edit);
void edit_insert (WEdit * edit, int c);
int edit_cursor_move (WEdit * edit, long increment);
void push_action (WEdit * edit, long c,...);
void push_key_press (WEdit * edit);
void edit_insert_ahead (WEdit * edit, int c);
int edit_save_file (WEdit * edit, const char *filename);
int edit_save_cmd (WEdit * edit);
int edit_save_confirm_cmd (WEdit * edit);
int edit_save_as_cmd (WEdit * edit);
WEdit *edit_init (WEdit * e, int lines, int columns, const char *filename, const char *text, const char *dir);
int edit_clean (WEdit * edit);
int edit_renew (WEdit * edit);
int edit_new_cmd (WEdit * edit);
int edit_reload (WEdit * edit, const char *filename, const char *text, const char *dir);
int edit_load_cmd (WEdit * edit);
void edit_mark_cmd (WEdit * edit, int unmark);
void edit_set_markers (WEdit * edit, long m1, long m2);
void edit_push_markers (WEdit * edit);
void edit_quit_cmd (WEdit * edit);
void edit_replace_cmd (WEdit * edit, int again);
void edit_search_cmd (WEdit * edit, int again);
int edit_save_block_cmd (WEdit * edit);
int edit_insert_file_cmd (WEdit * edit);
int edit_insert_file (WEdit * edit, const char *filename);
void edit_block_process_cmd (WEdit * edit, const char *shell_cmd, int block);
char *catstrs (const char *first,...);
void edit_refresh_cmd (WEdit * edit);
void edit_date_cmd (WEdit * edit);
void edit_goto_cmd (WEdit * edit);
int eval_marks (WEdit * edit, long *start_mark, long *end_mark);
void edit_status (WEdit * edit);
int Cedit_execute_command (WEdit * edit, int command, int char_for_insertion);
int edit_execute_key_command (WEdit * edit, int command, int char_for_insertion);
void Cedit_update_screen (WEdit * edit);
int edit_printf (WEdit * e, const char *fmt,...);
int edit_print_string (WEdit * e, const char *s);
void edit_move_to_line (WEdit * e, long line);
void edit_move_display (WEdit * e, long line);
void edit_word_wrap (WEdit * edit);
unsigned char *edit_get_block (WEdit * edit, long start, long finish);
int edit_sort_cmd (WEdit * edit);
void edit_help_cmd (WEdit * edit);
void edit_left_word_move (WEdit * edit);
void edit_right_word_move (WEdit * edit);

int edit_save_macro_cmd (WEdit * edit, struct macro macro[], int n);
int edit_load_macro_cmd (WEdit * edit, struct macro macro[], int *n, int *k);

int edit_copy_to_X_buf_cmd (WEdit * edit);
int edit_cut_to_X_buf_cmd (WEdit * edit);
void paste_from_X_buf_cmd (WEdit * edit);

void split_filename (WEdit * edit, char *name);

#ifdef MIDNIGHT

#define TEXT_PIX_PER_LINE 1
#define TEXT_M_WIDTH 1

#define get_sys_error(s) (s)
#define open(f,p) mc_open(f,p)
#define close(f) mc_close(f)
#define read(f,b,c) mc_read(f,b,c)
#define write(f,b,c) mc_write(f,b,c)
#define stat(f,s) mc_stat(f,s)

#define Cgetloadfile(w,x,y,d,f,h) input_dialog (h, " Enter file name: ", f)
#define Cgetsavefile(w,x,y,d,f,h) input_dialog (h, " Enter file name: ", f)
#define Cmalloc(x) malloc(x)

#define set_error_msg(s) edit_init_error_msg = strdup(s)

#ifdef _EDIT_C

#define Cerrordialogue(w,x,y,h,s) set_error_msg(s)
char *edit_init_error_msg = NULL;
#else				/* ! _EDIT_C */

#define Cerrordialogue(w,x,y,h,s) query_dialog (h, s, 0, 1, " Cancel ")
#define Cmessagedialogue(w,x,y,h,s) query_dialog (h, s, 0, 1, " Ok ")
extern char *edit_init_error_msg;

#endif				/* ! _EDIT_C */

#define get_error_msg(s) edit_init_error_msg

#else				/* ! MIDNIGHT */

#define Cerrordialogue(w,x,y,h,s) Cerrordialog(w,x,y,h,s)
#define Cmessagedialogue(w,x,y,h,s) Cmessagedialog(w,x,y,h,s)
#define Cmessagedialogue(w,x,y,h,s) Cmessagedialog(w,x,y,h,s)
#define Cquerydialogue Cquerydialog

#endif				/* ! MIDNIGHT */

extern char *home_dir;

#ifdef _EDIT_C

int option_international_characters = 0;
int option_tab_spacing = 8;
int option_fill_tabs_with_spaces = 0;
int option_return_does_auto_indent = 1;
int option_backspace_through_tabs = 0;
int option_fake_half_tabs = 1;

int option_editor_fg_normal = 26;
int option_editor_fg_bold = 8;
int option_editor_fg_italic = 10;

int option_editor_bg_normal = 1;
int option_editor_bg_abnormal = 0;
int option_editor_bg_marked = 2;
int option_editor_bg_marked_abnormal = 9;
int option_editor_bg_highlighted = 12;
int option_editor_fg_cursor = 18;

char *option_whole_chars_search = "0123456789abcdefghijklmnopqrstuvwxyz_";
char *option_whole_chars_move = "0123456789abcdefghijklmnopqrstuvwxyz_; ,[](){}";

#else				/* ! _EDIT_C */

extern int option_international_characters;
extern int option_tab_spacing;
extern int option_fill_tabs_with_spaces;
extern int option_return_does_auto_indent;
extern int option_backspace_through_tabs;
extern int option_fake_half_tabs;

extern int option_editor_fg_normal;
extern int option_editor_fg_bold;
extern int option_editor_fg_italic;

extern int option_editor_bg_normal;
extern int option_editor_bg_abnormal;
extern int option_editor_bg_marked;
extern int option_editor_bg_marked_abnormal;
extern int option_editor_bg_highlighted;
extern int option_editor_fg_cursor;

extern char *option_whole_chars_search;
extern char *option_whole_chars_move;

#endif				/* ! _EDIT_C */

#endif
