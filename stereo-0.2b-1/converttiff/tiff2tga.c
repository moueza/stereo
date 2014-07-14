#include <math.h>
#include <stdio.h>
#include <stdlib.h>


char *loadgreytiff (char *fname, long *width, long *height,
	long rowstart, long rowend, float gamma);


void main (int argc, char *argv[])
{
    unsigned char *data;
    FILE *fp;
    int i, j;
    long w, h;
    float gamma = 1.8;
    long rowstart, rowend;

    if (argc != 3 && argc != 4) {
	printf ("\nConverts TIFF Version 5 uncompressed topleft orientated\n");
	printf ("\"Class G\" files to TARGA 8-bits-per-pixel grey-scale files.\n");
	printf ("These TIFF files are typical scanner-software output files.\n");
	printf ("The resulting .tga file can then be compressed with \"cjpeg -grayscale\".\n");
	printf ("They can be uncompressed with \"djpeg -targa -grayscale\".\n");
	printf ("Usage: tiff2tga <infile> <outfile> [GAMMA]\n");
	printf ("GAMMA is a decimal number being the expected luminance\n");
	printf ("responce of the output device. Default value is 1.8.\n");
	printf ("Higher values of gamma will brighten the image.\n\n");
	return;
    }

    if(argc == 4) gamma = atof(argv[3]);

    data = loadgreytiff (argv[1], &w, &h, 0, 2, gamma);

    if ((fp = fopen (argv[2], "w+")) == NULL) {
	fprintf(stderr, "Cannot open/create/overwrite targa image file.\n");
	return;
    }

    /* write header information */

    putc (0, fp);
    putc (0, fp);

    putc (3, fp);

    for (i = 0; i < 9; i++)
	putc (0, fp);

    putc (w & 0xFF, fp);
    putc (w >> 8, fp);

    putc (h & 0xFF, fp);
    putc (h >> 8, fp);

    putc (8, fp);
    putc (32, fp);		/*top left displayed */

    for (j = 0; j < h; j += 262144 / w) {
	rowstart = j;
	rowend = j + 262144 / w;
	if (rowend > h)
	    rowend = h;
	data = loadgreytiff (argv[1], &w, &h, rowstart, rowend, gamma);
	if(!data) {
	    fprintf(stderr, "Error: tiff file.\n");
	    return;
	}
	fwrite (data, (rowend - rowstart) * w, 1, fp);
	if(data) free (data);
    }

    fclose (fp);
    return;
}

