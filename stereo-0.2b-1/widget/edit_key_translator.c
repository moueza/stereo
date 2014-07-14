/*
   This is #include'd into the function edit_handle_key in edit.c.

   This sequence of code takes the integer 'x_state' and the long
   integer 'x_key' and translates them into the integer 'command' or the
   integer 'char_for_insertion'. 'x_key' holds one of the keys in the
   system header file key_sym_def.h (/usr/include/X11/key_sym_def.h on
   my Linux machine) and 'x_state' holds a bitwise inclusive OR of
   Button1Mask, Button2Mask, ShiftMask, LockMask, ControlMask, Mod1Mask,
   Mod2Mask, Mod3Mask, Mod4Mask, or Mod5Mask as explained in the
   XKeyEvent man page. The integer 'command' is one of the editor
   commands in the header file editcmddef.h

   Normally you would only be interested in the ShiftMask and
   ControlMask modifiers. The Mod1Mask modifier refers to the Alt key
   on my system.

   If the particular 'x_key' is an ordinary character (say XK_a) then
   you must translate it into 'char_for_insertion', and leave 'command'
   untouched.

   So for example, to add the key binding Ctrl-@ for marking text,
   the following piece of code can be used:

   if ((x_state & ControlMask) && x_key == XK_2) {
       command = CK_Mark;
       goto fin:
   }

   For another example, suppose you want the exclamation mark key to
   insert a '1' then,

   if (x_key == XK_exclam) {
       char_for_insertion = '1';
       goto fin:
   }

   However you must not set both 'command' and 'char_for_insertion';
   one or the other only.

   Not every combination of key states and keys will work though,
   and some experimentation may be necessary.

   Almost any C code can go into this file. The code below is an
   example that may by appended or modified by the user. For brevity,
   it has a lookup table for basic key presses.

 */

#include "editcmddef.h"

    static long key_map[128] =
    {XK_BackSpace, CK_BackSpace, XK_Delete, CK_Delete, XK_Return, CK_Return, XK_Page_Up, CK_Page_Up,
     XK_Page_Down, CK_Page_Down, XK_Left, CK_Left, XK_Right, CK_Right, XK_Up, CK_Up, XK_Down, CK_Down,
     XK_Home, CK_Home, XK_End, CK_End, XK_Tab, CK_Tab, XK_Undo, CK_Undo, XK_Insert, CK_Toggle_Insert,
     XK_F3, CK_Mark, XK_F5, CK_Copy, XK_F6, CK_Move, XK_F8, CK_Remove, XK_F2, CK_Save, XK_F12, CK_Save_As,
     XK_F10, CK_Exit, /* XK_Escape, CK_Exit, this may be a bit rash */ XK_F19, CK_Pipe_Block (0),
     XK_F4, CK_Replace, XK_F17, CK_Find_Again, XK_F7, CK_Find, XK_F15, CK_Insert_File, 0, 0};

    static long key_pad_map[10] =
    {XK_Insert, XK_End, XK_Down, XK_Page_Down, XK_Left,
     XK_Down, XK_Right, XK_Home, XK_Up, XK_Page_Up};


#define DEFAULT_NUM_LOCK        1

    static int num_lock = DEFAULT_NUM_LOCK;
    static int raw = 0;
    int i = 0;
    int h;

    if (edit)
	if (edit->user_defined_key)
	    if ((h = (*(edit->user_defined_key)) (x_state, x_keycode))) {
		command = h;
		goto fin;
	    }

    if (x_key <= 0 || x_key == XK_Control_L || x_key == XK_Control_R || x_key == XK_Shift_L || x_key == XK_Shift_R || x_key == XK_Alt_L || x_key == XK_Alt_R)
	goto fin;

    if (raw) {
	char_for_insertion = x_key - XK_space + ' ';
	if (x_state & ControlMask)
	    char_for_insertion &= 31;
	if (x_state & (Mod1Mask))
	    char_for_insertion |= 128;
	raw = 0;
	goto fin;
    }

    if ((x_state & Mod1Mask)) {
	switch (x_key) {
	case XK_Left:
	case XK_KP_Left:
	    command = CK_Delete_Word_Left;
	    goto fin;
	case XK_Right:
	case XK_KP_Right:
	    command = CK_Delete_Word_Right;
	    goto fin;
	case XK_l:
	case XK_L:
	    command = CK_Goto;
	    goto fin;
	case XK_F5:
	    command = CK_Sort;
	    goto fin;
	case XK_F7:
	    command = CK_Run_Make;
	    goto fin;
	}
    }

    if (!(x_state & Mod1Mask)) {

	if ((x_key == XK_a || x_key == XK_A) && (x_state & ControlMask)) {
	    command = CK_Macro (Crawkeyquery (0, 0, 0, " Execute Macro ", " Press macro hotkey: "));
	    if (command == CK_Macro (0))
		command = -1;
	    goto fin;
	}
/* edit is a pointer to the widget */
	if (edit)
	    if ((x_key == XK_r || x_key == XK_R) && (x_state & ControlMask)) {
		command = edit->macro_i < 0 ? CK_Begin_Record_Macro : CK_End_Record_Macro;
		goto fin;
	    }
	if (x_key == XK_Num_Lock) {
	    num_lock = 1 - num_lock;
	    goto fin;
	}

	switch (x_key) {
	    case XK_KP_Home:
		x_key = XK_Home;
		break;
	    case XK_KP_End:
		x_key = XK_End;
		break;
	    case XK_KP_Page_Up:
		x_key = XK_Page_Up;
		break;
	    case XK_KP_Page_Down:
		x_key = XK_Page_Down;
		break;
	    case XK_KP_Up :
		x_key = XK_Up;
		break;
	    case XK_KP_Down :
		x_key = XK_Down;
		break;
	    case XK_KP_Left :
		x_key = XK_Left;
		break;
	    case XK_KP_Right :
		x_key = XK_Right;
		break;
	    case XK_KP_Insert :
		x_key = XK_Insert;
		break;
	    case XK_KP_Delete :
		x_key = XK_Delete;
		break;
	    case XK_KP_Enter :
		x_key = XK_Return;
		break;
	    case XK_KP_Add :
		x_key = XK_plus;
		break;
	    case XK_KP_Subtract :
		x_key = XK_minus;
		break;
	}

/* first translate the key-pad */
	if (num_lock) {
	    if (x_key >= XK_R1 && x_key <= XK_R9) {
		x_key = key_pad_map[x_key - XK_R1 + 1];
	    } else if (x_key >= XK_KP_0 && x_key <= XK_KP_9) {
		x_key = key_pad_map[x_key - XK_KP_0];
	    } else if (x_key == XK_KP_Decimal) {
		x_key = XK_Delete;
	    }
	} else {
	    if (x_key >= XK_KP_0 && x_key <= XK_KP_9) {
		x_key += XK_0 - XK_KP_0;
	    }
	    if (x_key == XK_KP_Decimal) {
		x_key = XK_period;
	    }
	}

	if ((x_state & ShiftMask) && (x_state & ControlMask)) {
	    switch (x_key) {
	    case XK_Page_Up:
		command = CK_Beginning_Of_Text_Highlight;
		goto fin;
	    case XK_Page_Down:
		command = CK_End_Of_Text_Highlight;
		goto fin;
	    case XK_Left:
		command = CK_Word_Left_Highlight;
		goto fin;
	    case XK_Right:
		command = CK_Word_Right_Highlight;
		goto fin;
	    case XK_Up:
		command = CK_Scroll_Up_Highlight;
		goto fin;
	    case XK_Down:
		command = CK_Scroll_Down_Highlight;
		goto fin;
	    case XK_Home:
		command = CK_Begin_Page_Highlight;
		goto fin;
	    case XK_End:
		command = CK_End_Page_Highlight;
		goto fin;
	    }
	}
	if ((x_state & ShiftMask) && !(x_state & ControlMask)) {
	    switch (x_key) {
	    case XK_Page_Up:
		command = CK_Page_Up_Highlight;
		goto fin;
	    case XK_Page_Down:
		command = CK_Page_Down_Highlight;
		goto fin;
	    case XK_Left:
		command = CK_Left_Highlight;
		goto fin;
	    case XK_Right:
		command = CK_Right_Highlight;
		goto fin;
	    case XK_Up:
		command = CK_Up_Highlight;
		goto fin;
	    case XK_Down:
		command = CK_Down_Highlight;
		goto fin;
	    case XK_Home:
		command = CK_Home_Highlight;
		goto fin;
	    case XK_End:
		command = CK_End_Highlight;
		goto fin;
	    case XK_Insert:
		command = CK_XPaste;
		goto fin;
	    case XK_Delete:
		command = CK_XCut;
		goto fin;
	    case XK_Return:
		command = CK_Enter;
		goto fin;
/* this parallel F12, F19, F15, and F17 for some systems */
	    case XK_F2:
		command = CK_Save_As;
		goto fin;
	    case XK_F9:
		command = CK_Pipe_Block (0);
		goto fin;
	    case XK_F5:
		command = CK_Insert_File;
		goto fin;
	    case XK_F7:
		command = CK_Find_Again;
		goto fin;
	    }
	}
/* things that need a control key */
	if (x_state & ControlMask) {
	    switch (x_key) {
	    case XK_F1:
		command = CK_Man_Page;
		goto fin;
	    case XK_U:
	    case XK_u:
	    case XK_BackSpace:
		command = CK_Undo;
		goto fin;
	    case XK_Page_Up:
		command = CK_Beginning_Of_Text;
		goto fin;
	    case XK_Page_Down:
		command = CK_End_Of_Text;
		goto fin;
	    case XK_Up:
		command = CK_Scroll_Up;
		goto fin;
	    case XK_Down:
		command = CK_Scroll_Down;
		goto fin;
	    case XK_Left:
		command = CK_Word_Left;
		goto fin;
	    case XK_Right:
		command = CK_Word_Right;
		goto fin;
	    case XK_Home:
		command = CK_Begin_Page;
		goto fin;
	    case XK_End:
		command = CK_End_Page;
		goto fin;
	    case XK_N:
	    case XK_n:
		command = CK_New;
		goto fin;
	    case XK_O:
	    case XK_o:
		command = CK_Load;
		goto fin;
	    case XK_P:
	    case XK_p:
		command = CK_Pipe_Block (1);
		goto fin;
	    case XK_D:
	    case XK_d:
		command = CK_Date;
		goto fin;
	    case XK_Q:
	    case XK_q:
		raw = 1;
		goto fin;
	    case XK_F:
	    case XK_f:
		command = CK_Save_Block;
		goto fin;
	    case XK_F5:
	    case XK_F15:
		command = CK_Insert_File;
		goto fin;
	    case XK_Insert:
		command = CK_XStore;
		goto fin;
	    case XK_y:
	    case XK_Y:
		command = CK_Delete_Line;
		goto fin;
	    case XK_Delete:
		command = CK_Remove;
		goto fin;
	    }
	}
/* an ordinary ascii character */
	if (!(x_state & ControlMask)) {
	    if (x_key >= XK_space && x_key <= XK_asciitilde) {
		char_for_insertion = x_key - XK_space + ' ';
		goto fin;
	    }
/* other commands */
	    i = 0;
	    while (key_map[i] != x_key && key_map[i])
		i += 2;
	    command = key_map[i + 1];
	    if (command)
		goto fin;
	}
    }

  fin:



