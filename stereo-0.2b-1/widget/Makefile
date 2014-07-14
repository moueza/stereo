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

noinst_LIBRARIES = cw

cw_SOURCES = 3dinit.c 3dinit.h 3dkit.c 3dkit.h 3dtext.c ClientWin.c \
	bitmapbutton.c cachedraw.c clearevents.c cmdlineopt.c \
	cmdlineopt.h compatable.c coollocal.h coolnext.c \
	coolwidget.c coolwidget.h copyright.h dialog.c dialog.h dirtools.c \
	dirtools.h disable.c dnd.c drawings.c drawings.h edit.c edit.h \
	editcmd.c editcmddef.h editdraw.c \
	editlearnkeys.c editmenu.c editoptions.c editoptions.h editwidget.c \
	filebrowser.c focus.c grlib.c hintpos.c imagewidget.c imagewidget.h \
	initapp.c lkeysym.h loadfile.c loadfile.h loadtiff.c loadtiff.h \
	mad.c mad.h manpage.c mathfuncs.c menu.c my_string.h options.c \
	options.h pipe.c quickmath.c quickmath.h regex.c regex.h striangle.c \
	stringtools.c stringtools.h swtriangle.c textwidget.c \
	triangle.c triangle.h vga.h vgagl.h widget3d.c \
	widget3d.h wrapsurf.c wtriangle.c
mkinstalldirs = $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
LIBRARIES = $(noinst_LIBRARIES)

noinst_LIBFILES = libcw.a

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
cw_LIBADD =
cw_OBJECTS = 3dinit.o 3dkit.o 3dtext.o ClientWin.o bitmapbutton.o \
cachedraw.o clearevents.o cmdlineopt.o compatable.o coolnext.o \
coolwidget.o dialog.o dirtools.o disable.o dnd.o drawings.o edit.o \
editcmd.o editdraw.o editlearnkeys.o editmenu.o editoptions.o \
editwidget.o filebrowser.o focus.o grlib.o hintpos.o imagewidget.o \
initapp.o loadfile.o loadtiff.o mad.o manpage.o mathfuncs.o menu.o \
options.o pipe.o quickmath.o regex.o striangle.o stringtools.o \
swtriangle.o textwidget.o triangle.o widget3d.o wrapsurf.o wtriangle.o
EXTRA_cw_SOURCES =
LIBFILES =  libcw.a
AR = ar
RANLIB = ranlib
DIST_COMMON = Makefile.am Makefile.in


PACKAGE = stereo
VERSION = r_VERSION_r

DISTFILES = $(DIST_COMMON) $(SOURCES) $(BUILT_SOURCES) $(HEADERS) \
	$(TEXINFOS) $(INFOS) $(MANS) $(EXTRA_DIST) $(DATA)
DEP_DISTFILES = $(DIST_COMMON) $(SOURCES) $(BUILT_SOURCES) $(HEADERS) \
	$(TEXINFOS) $(INFO_DEPS) $(MANS) $(EXTRA_DIST) $(DATA)

TAR = tar
SOURCES = $(cw_SOURCES)
OBJECTS = $(cw_OBJECTS)

default: all


$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in
	cd $(top_srcdir) && automake $(subdir)/Makefile

Makefile: $(top_builddir)/config.status Makefile.in
	cd $(top_builddir) && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= ./config.status

mostlyclean-noinstLIBRARIES:

clean-noinstLIBRARIES:
	rm -f $(noinst_LIBFILES)

distclean-noinstLIBRARIES:

maintainer-clean-noinstLIBRARIES:

.c.o:
	$(COMPILE) $<

mostlyclean-compile:
	rm -f *.o core

clean-compile:

distclean-compile:
	rm -f *.tab.c

maintainer-clean-compile:
$(cw_OBJECTS): ../config.h

libcw.a: $(cw_OBJECTS) $(cw_LIBADD)
	rm -f libcw.a
	$(AR) cru libcw.a $(cw_OBJECTS) $(cw_LIBADD)
	$(RANLIB) libcw.a

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

subdir = widget
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

install-exec: 

install-data: 

install: install-exec install-data all
	@:

uninstall: 

all: $(LIBFILES) Makefile

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install
installdirs:


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
mostlyclean:  mostlyclean-noinstLIBRARIES mostlyclean-compile \
		mostlyclean-tags mostlyclean-generic

clean:  clean-noinstLIBRARIES clean-compile clean-tags clean-generic \
		mostlyclean 

distclean:  distclean-noinstLIBRARIES distclean-compile distclean-tags \
		distclean-generic clean 
	rm -f config.status

maintainer-clean:  maintainer-clean-noinstLIBRARIES \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-generic distclean 
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

.PHONY: default mostlyclean-noinstLIBRARIES distclean-noinstLIBRARIES \
clean-noinstLIBRARIES maintainer-clean-noinstLIBRARIES \
mostlyclean-compile distclean-compile clean-compile \
maintainer-clean-compile tags mostlyclean-tags distclean-tags \
clean-tags maintainer-clean-tags distdir info dvi check installcheck \
install-exec install-data install uninstall all installdirs \
mostlyclean-generic distclean-generic clean-generic \
maintainer-clean-generic clean mostlyclean distclean maintainer-clean

.SUFFIXES:
.SUFFIXES: .c .o

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
