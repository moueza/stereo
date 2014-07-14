#if 1
void push_action (WEdit * edit, long c)
{
    int i;
    printf ("%d\n", c);
    l_push_action (edit, c);
    if (edit->stack_pointer)
    for (i=0;i<edit->stack_pointer;i++) {
	if(edit->undo_stack[i] == CURS_RIGHT)
	    printf ("> ");
	if(edit->undo_stack[i] == CURS_LEFT)
	    printf ("< ");
	if(edit->undo_stack[i] == DELETE)
	    printf ("\\ ");
	if(edit->undo_stack[i] == BACKSPACE)
	    printf ("^ ");
	if(edit->undo_stack[i] >= KEY_PRESS) {
	    printf ("KK");
	    continue;
	}
	if(edit->undo_stack[i] <= '~' && edit->undo_stack[i] >= ' ')
	    printf ("%c+", edit->undo_stack[i]);
	if(edit->undo_stack[i] <= '~' + 256 && edit->undo_stack[i] >= ' ' + 256)
	    printf ("%c-", edit->undo_stack[i] - 256);
	if(edit->undo_stack[i] < 0)
	    printf ("%d", edit->undo_stack[i]);
    }
    printf ("\n");
}
#endif











/* this must be appended onto edit.c */
/*
   This formats the region of text about the cursor.
   A "soft return" occurs at the end of a line of text and is
   represented by a " \n". A "hard return" is represented by
   a "\n" only. The following routine removes all soft returns
   from the paragraph, then inserts soft returns for correct
   spacing.
*/

#if WORD_WRAP

/* returns 0 if normal, 1 if the next is a " \n" or 2 if the next is a "\n" */
int forward_word (WEdit * edit, long *p)
{
    int c;

    for (;; *p++) {
	c = edit_get_byte (edit, *p);
	if (c == '\n')
	    return 2;
	if (c == ' ') {
	    if (edit_get_byte (edit, ++*p) == '\n')
		return 1;
	    else
		return 0;
	}
    }
}


long edit_trunc_line (WEdit *edit, long p)
{
    int current = p;
    int line = 0;
    int q = p;
    int r;

    for (;;) {
	while (!(r = forward_word (edit, &p)));
	if (r == 2)
	    return p + 1;
	line += (p - q);
	q = p;
	if (r == 1) {
	    p++;
	    forward_word (edit, &p)
	}

	if (edit_get_byte (edit, q) == ' ' && edit_get_byte (edit, q + 1) == '\n')
	    
	line++;
    }
}

void edit_word_wrap (WEdit * edit)
{
    int line;
    int c, b, i = 0;
    long current;

    edit->force |= REDRAW_PAGE;
    current = edit->curs1;

    b = edit_get_byte (edit, edit->curs1);
    for (;;) {
	if (i > 1024) {		/* if there are very long paragraphs, this could be slow, so
				   stop if we are trying to format to much at once */
	    edit_cursor_to_bol (edit);
	    break;
	}
	c = b;
	if ((b = edit_cursor_move (edit, -1)) < 0)	/* beginning of text */
	    break;
	if (b != ' ' && c == '\n') {
	    while (edit_cursor_move (edit, 1) != '\n')
		break;
	}
	i++;
    }

/*
   we are now at the beginning of the paragraph: move to end, deleting
   all soft returns, and inserting new soft returns.
*/
    i = 0;
    line = 1;
    c = edit_cursor_move (edit, 1);
    for (;;) {
	if (i > 2048)
	    break;
	b = c;
	if ((c = edit_cursor_move (edit, 1)) < 0)
	    break;			/* end of text */
	line++;
	if (c == '\n') {
	    if (b == ' ') {
		edit_backspace (edit);
		line--;
		if (edit->curs1 <= current)
		    current--;
	    } else {
		break;
	    }
	}
	if (line >= word_wrap_line_length) {
	    int lcurrent;
	    int t, u;
	    lcurrent = edit->curs1;
	    edit_cursor_move (edit, -1);
	    for (;;) {
		t = edit_cursor_move (edit, -1);
		if (t == ' ') {
		    edit_cursor_move (edit, 1);
		    edit_insert (edit, '\n');
		    if (edit->curs1 <= current)
			current++;
		    line = 0;
		    break;
		}
		if (t == '\n' || t < 0) {	/* a loooong word */
		    edit_cursor_move (edit, lcurrent - edit->curs1); /* return to our position */
		    u = edit_cursor_move (edit, 1);
		    for (;;) {	/* move forward until we see a space */
			t = u;
			u = edit_cursor_move (edit, 1);
			if (t == ' ') {
			    if (u != '\n') {	/* insert a newline unless there already is a newline at the end of the looong word */
				edit_cursor_move (edit, -1);
				edit_insert (edit, '\n');
				if (edit->curs1 <= current)
				    current++;
			    }
			    line = 0;
			    goto done;
			}
			if (t == '\n' || t < 0) {
			    line = 0;
			    goto done;
			}
		    }
		}
	    }
	    done:;
	    c = '\n';
	}
	i++;
    }
/* return to the original position */
    edit_cursor_move (edit, current - edit->curs1);
}

#endif
