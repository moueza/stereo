/*
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
#ifndef _JOIN_H
#define _JOIN_H

void join_up_surface_edges (TD_Solid * o, double t);
void join_up_normals (TD_Solid * o, double t, double a);
void join_up_normals_and_surface_edges (TD_Solid * o, double t, double a);
void calc_all_normals (TD_Solid * o);

#endif

