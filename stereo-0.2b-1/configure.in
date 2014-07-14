dnl Process this file with autoconf to produce a configure script.
AC_INIT(main/display.c)
AC_CONFIG_HEADER(config.h)

dnl Define package and version.
PACKAGE=stereo
AC_SUBST(PACKAGE)
AC_DEFINE_UNQUOTED(PACKAGE, "${PACKAGE}")
VERSION=r_VERSION_r
AC_SUBST(VERSION)
AC_DEFINE_UNQUOTED(VERSION, "${VERSION}")
echo "Now configuring $PACKAGE-$VERSION"

AC_DEFINE(HAVE_PICTURE)
AC_DEFINE(HAVE_THREE_D)
AC_DEFINE(HAVE_BWIMAGE)

dnl Identify target.
AC_CANONICAL_SYSTEM
AC_DEFINE_UNQUOTED(TARGET_MACHINE, "${target_cpu}-${target_vendor}-${target_os}")

dnl Send target name.
AC_ARG_ENABLE(mail-author,
	[--disable-mail-author     Don't automatically mail me on first run],[
	AC_DEFINE(DISABLE_AUTO_MAIL,1)
])

dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_RANLIB

dnl Checks for C compiler flags that X needs and the X linker flags
AC_PATH_XTRA
CFLAGS="$CFLAGS $X_CFLAGS"
LDFLAGS="$LDFLAGS $X_LIBS"
AC_SUBST(LDFLAGS)

dnl Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_PROG_INSTALL
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h sys/select.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_PID_T

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
dnl
dnl autoscan wants to check for a select function. How can the piping work without select?
dnl
AC_CHECK_FUNCS(strerror getwd re_comp regcomp strdup strspn strstr strcspn)
AC_CHECK_FUNCS(strchr strrchr memmove memcpy memset memchr memcmp)
AC_CHECK_FUNCS(strcasecmp strncasecmp)

AC_ARG_PROGRAM
AC_OUTPUT(widget/Makefile main/Makefile Makefile)

echo ""
echo "Configuration done."
echo "Type 'make' to compile $PACKAGE-$VERSION"
echo ""

