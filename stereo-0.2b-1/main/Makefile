# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.0 from Makefile.am

# Copyright (C) 1994, 1995, 1996 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

pkgdatadir = $(datadir)/stereo
pkglibdir = $(libdir)/stereo
pkgincludedir = $(includedir)/stereo

top_builddir = ..

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = @INSTALL_SCRIPT@
transform = s,x,x,

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
mkinstalldirs = $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
PROGRAMS = $(bin_PROGRAMS)


CC = gcc
LEX = @LEX@
YACC = @YACC@

DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I..
CPPFLAGS = 
CFLAGS = -g -O2  -I/usr/X11R6/include
LDFLAGS =   -L/usr/X11R6/lib
LIBS = 

COMPILE = $(CC) -c $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
LINK = $(CC) $(LDFLAGS) -o $@
X_CFLAGS =  -I/usr/X11R6/include
X_LIBS =  -L/usr/X11R6/lib
X_EXTRA_LIBS = 
X_PRE_LIBS =  -lSM -lICE
display_OBJECTS = calibrate.o camera.o display.o desktop.o fitline.o \
hugeimage.o matrix.o simplex.o loadcalfile.o callback.o picsetup.o \
imagehandler.o displaycam.o marker.o output.o imagefit.o join.o \
savewindow.o
EXTRA_display_SOURCES =
display_LDADD = $(LDADD)
DIST_COMMON = Makefile.am Makefile.in


PACKAGE = stereo
VERSION = r_VERSION_r

DISTFILES = $(DIST_COMMON) $(SOURCES) $(BUILT_SOURCES) $(HEADERS) \
	$(TEXINFOS) $(INFOS) $(MANS) $(EXTRA_DIST) $(DATA)
DEP_DISTFILES = $(DIST_COMMON) $(SOURCES) $(BUILT_SOURCES) $(HEADERS) \
	$(TEXINFOS) $(INFO_DEPS) $(MANS) $(EXTRA_DIST) $(DATA)

TAR = tar
SOURCES = $(display_SOURCES)
OBJECTS = $(display_OBJECTS)

default: all


$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in
	cd $(top_srcdir) && automake $(subdir)/Makefile

Makefile: $(top_builddir)/config.status Makefile.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= ./config.status

mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	$(mkinstalldirs) $(bindir)
	list="$(bin_PROGRAMS)"; for p in $$list; do \
	  if test -f $$p; then \
	    $(INSTALL_PROGRAM) $$p $(bindir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	list="$(bin_PROGRAMS)"; for p in $$list; do \
	  rm -f $(bindir)/`echo $$p|sed '$(transform)'`; \
	done

.c.o:
	$(COMPILE) $<

mostlyclean-compile:
	rm -f *.o core

clean-compile:

distclean-compile:
	rm -f *.tab.c

maintainer-clean-compile:
$(display_OBJECTS): ../config.h

display: $(display_OBJECTS) $(display_DEPENDENCIES)
	$(LINK) $(display_OBJECTS) $(display_LDADD) $(LIBS)

ID: $(HEADERS) $(SOURCES)
	here=`pwd` && cd $(srcdir) && mkid -f$$here/ID $(SOURCES) $(HEADERS)

tags: TAGS

TAGS: $(HEADERS) $(SOURCES) $(TAGS_DEPENDENCIES)
	here=`pwd` && cd $(srcdir) && etags $(ETAGS_ARGS) $(SOURCES) $(HEADERS) -o $$here/TAGS

mostlyclean-tags:

clean-tags:

distclean-tags:
	rm -f TAGS ID

maintainer-clean-tags:

subdir = main
distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)
distdir: $(DEP_DISTFILES)
	@for file in `cd $(srcdir) && echo $(DISTFILES)`; do \
	  test -f $(distdir)/$$file \
	  || ln $(srcdir)/$$file $(distdir)/$$file 2> /dev/null \
	  || cp -p $(srcdir)/$$file $(distdir)/$$file; \
	done
info:

dvi:

check: all

installcheck:

install-exec: install-binPROGRAMS

install-data: 

install: install-exec install-data all
	@:

uninstall: uninstall-binPROGRAMS

all: $(PROGRAMS) Makefile

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install
installdirs:
	$(mkinstalldirs)  $(bindir)


mostlyclean-generic:
	test -z "$(MOSTLYCLEANFILES)" || rm -f $(MOSTLYCLEANFILES)

clean-generic:
	test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	rm -f Makefile $(DISTCLEANFILES)
	rm -f config.cache config.log $(CONFIG_HEADER) stamp-h

maintainer-clean-generic:
	test -z "$(MAINTAINERCLEANFILES)" || rm -f $(MAINTAINERCLEANFILES)
	test -z "$(BUILT_SOURCES)" || rm -f $(BUILT_SOURCES)
mostlyclean:  mostlyclean-binPROGRAMS mostlyclean-compile \
		mostlyclean-tags mostlyclean-generic

clean:  clean-binPROGRAMS clean-compile clean-tags clean-generic \
		mostlyclean 

distclean:  distclean-binPROGRAMS distclean-compile distclean-tags \
		distclean-generic clean 
	rm -f config.status

maintainer-clean:  maintainer-clean-binPROGRAMS maintainer-clean-compile \
		maintainer-clean-tags maintainer-clean-generic \
		distclean 
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: default mostlyclean-binPROGRAMS distclean-binPROGRAMS \
clean-binPROGRAMS maintainer-clean-binPROGRAMS uninstall-binPROGRAMS \
install-binPROGRAMS mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile tags mostlyclean-tags distclean-tags \
clean-tags maintainer-clean-tags distdir info dvi check installcheck \
install-exec install-data install uninstall all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean


$(PROGRAMS): ../widget/libcw.a

check: display
	echo 'No checks are available'
	echo ' '
.SUFFIXES:
.SUFFIXES: .c .o

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
