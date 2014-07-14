#define get_byte(x,i) 


{
    for (; row < height && from < view->last_byte; from++) {
	c = get_byte (view, from);
	if ((c == '\n') || (col == width && wrap_mode)) {
	    col = frame_shift;
	    row++;
	    if (c == '\n' || row >= height)
		continue;
	}
	if (c == '\r')
	    continue;
	if (c == '\t') {
	    col = ((col - frame_shift) / 8) * 8 + 8 + frame_shift;
	    continue;
	}
	if (view->viewer_nroff_flag && c == '\b') {
	    if (from + 1 < view->last_byte
		&& is_printable (get_byte (view, from + 1)) &&
		from > view->first
		&& is_printable (get_byte (view, from - 1))) {
		if (col <= frame_shift) {
		    /* So it has to be wrap_mode - do not need to check for it */
		    if (row == 1 + frame_shift) {
			from++;
			continue;	/* There had to be a bold character on the rightmost position
					   of the previous undisplayed line */
		    }
		    row--;
		    col = width;
		}
		col--;
		boldflag = 1;
		if (get_byte (view, from - 1) == '_' && get_byte (view, from + 1) != '_')
		    set_color (UNDERLINE_COLOR);
		else
		    set_color (BOLD_COLOR);
		continue;
	    }
	}
	if (view->found_len && from >= view->search_start
	    && from < view->search_start + view->found_len) {
	    boldflag = 1;
	    set_color (MARK_COLOR);
	}
	if (col >= frame_shift - view->start_col
	    && col < width - view->start_col) {
	    view_gotoyx (view, row, col + view->start_col);
	    add_character (view, c);
	}
	col++;
	if (boldflag) {
	    boldflag = 0;
	    set_color (DEF_COLOR);
	}
    }
}
