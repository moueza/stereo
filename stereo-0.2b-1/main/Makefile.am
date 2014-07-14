bin_PROGRAMS = display

display_SOURCES = \
	calibrate.c calibrate.h camera.c camera.h \
	display.c display.h desktop.c desktop.h \
	fitline.c fitline.h hugeimage.c hugeimage.h \
	matrix.c matrix.h simplex.c simplex.h \
	loadcalfile.c callback.c picsetup.c imagehandler.c \
	displaycam.c marker.c marker.h output.c output.h \
	imagefit.c imagefit.h join.c join.h savewindow.c \
	../config.h

INCLUDES = -I$(top_srcdir)/widget -I$(top_srcdir)

LDADD = ../widget/libcw.a -lX11 -lm

$(PROGRAMS): ../widget/libcw.a

check: display
	echo 'No checks are available'
	echo ' '

