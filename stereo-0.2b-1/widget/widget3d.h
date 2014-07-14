
#ifndef WIDGET_3D_H
#define WIDGET_3D_H

#ifdef USING_MATRIXLIB
#include "matrix.h"
void Cmatrix_to_surf(const char *ident, int surf_width, int surf_height, Matrix *x, Matrix *offset, double scale);
#endif

CWidget * Credraw3dobject (const char *ident, int force);

CWidget * Cdraw3dobject (const char *identifier, Window parent, int x, int y,
	       int width, int height, int defaults, int max_num_surfaces);


void Cinit_surf_points (const char *ident, int width, int height, TD_Point data[]);

void Cclear_all_surfaces(const char *ident);

void Crender3dwidget (CWidget *wdt, int x, int y, int rendw, int rendh);

#endif

