#ifndef IMAGE_WIDGET_H
#define IMAGE_WIDGET_H

#include "3dkit.h"

void greyscaletopix (void *pixdata, unsigned char *data, int width, int height, int bytedepth);

unsigned char *loadtarga2grey (const char *fname, long *width, long *height, long rowstart, long rowend);

int writetarga (unsigned char *pic8, const char *fname, long w, long h, int grey);

void color8bittopix (void *pixdata, unsigned char *data, int width, int height, int bytedepth);

#endif

