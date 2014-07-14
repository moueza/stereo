/* hintpos.c - routines for easy positioning of widgets
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

#include <config.h>
#include <stdio.h>
#include <my_string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include "lkeysym.h"

#include "stringtools.h"
#include "app_glob.c"
#include "coolwidget.h"
#include "coollocal.h"

#include "mad.h"

static int hint_pos_x = 0;
static int hint_pos_y = 0;
static int hint_pos_max_x = 0;
static int hint_pos_max_y = 0;

void Cresethintpos (int x, int y)
{
    hint_pos_x = x;
    hint_pos_y = y;
    hint_pos_max_x = x;
    hint_pos_max_y = y;
}

void  Csethintpos (int x, int y)
{
    hint_pos_x = x;
    hint_pos_y = y;
    hint_pos_max_x = max(x, hint_pos_max_x);
    hint_pos_max_y = max(y, hint_pos_max_y);
}

void Cgethintpos (int *x, int *y)
{
    if (x)
	*x = hint_pos_x;
    if (y)
	*y = hint_pos_y;
}

void Cgethintlimits (int *max_x, int *max_y)
{
    if (max_x)
	*max_x = hint_pos_max_x;
    if (max_y)
	*max_y = hint_pos_max_y;
}



