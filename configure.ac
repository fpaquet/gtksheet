# Process this file with autoconf to produce a configure script.
#
# Help for autoconf:
# - http://www.linuxselfhelp.com/gnu/autoconf/html_chapter/autoconf_toc.html
# Option Syntax:
# - http://www.linuxselfhelp.com/gnu/autoconf/html_chapter/autoconf_12.html
# Printing Messages:
# - http://www.linuxselfhelp.com/gnu/autoconf/html_chapter/autoconf_7.html
#

AC_INIT([gtkextra], [3.2.0])
AC_LANG([C])
AC_CONFIG_MACRO_DIR([m4])

# Save this value here, since automake will set cflags later
cflags_set=${CFLAGS:+set}
CFLAGS=${CFLAGS:-""}

AC_DIVERT_PUSH(NOTICE)dnl

# GtkExtra versioning: before making releases, be sure to read
# http://gtkextra.sourceforge.net/cms/index.php?option=com_content&view=article&id=44&Itemid=22

GTK_EXTRA_MAJOR_VERSION=3
GTK_EXTRA_MINOR_VERSION=2
GTK_EXTRA_MICRO_VERSION=0

GTK_EXTRA_VERSION=$GTK_EXTRA_MAJOR_VERSION.$GTK_EXTRA_MINOR_VERSION.$GTK_EXTRA_MICRO_VERSION

GTK_EXTRA_VERSION_SHORT=$GTK_EXTRA_MAJOR_VERSION.$GTK_EXTRA_MINOR_VERSION

AC_DIVERT_POP()dnl

AC_SUBST(GTK_EXTRA_MAJOR_VERSION)
AC_SUBST(GTK_EXTRA_MINOR_VERSION)
AC_SUBST(GTK_EXTRA_MICRO_VERSION)
AC_SUBST(GTK_EXTRA_VERSION)
AC_SUBST(GTK_EXTRA_VERSION_SHORT)

# libtool versioning

LT_CURRENT=8
LT_REVISION=0
LT_AGE=0

AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

LIBGTKEXTRA_SO_VERSION=$LT_CURRENT:$LT_REVISION:$LT_AGE
AC_SUBST(LIBGTKEXTRA_SO_VERSION)

# For automake
VERSION=$GTK_EXTRA_VERSION
PACKAGE=gtk+extra

AC_CONFIG_AUX_DIR([build-aux])
# Initialize automake stuff
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
#AM_SILENT_RULES([yes])

# Specify a configuration file
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_DEFINE([ENABLE_NLS],1, [NLS enabled])
AC_DEFINE([HAVE_GETTEXT],1, [have gettext])

# Build time sanity check...
AM_SANITY_CHECK

# Checks for programs.
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Initialize libtool
AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL
AM_MAINTAINER_MODE

AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, test "$os_win32" = "yes")

if test "$os_win32" = "yes"; then
  AC_CHECK_PROG(ms_librarian, lib.exe, yes, no)
fi
AM_CONDITIONAL(MS_LIB_AVAILABLE, test x$ms_librarian = xyes)

# --enable-debug option

AC_MSG_CHECKING([whether to enable debugging])

AC_ARG_ENABLE(debug, 
	AC_HELP_STRING(
		[--enable-debug=[no/yes]], 
		[turn on debugging [default=no]]),
	,
	enable_debug=no)

if test "x$enable_debug" = "xyes"; then
    test "$cflags_set" = set || CFLAGS="$CFLAGS -g -DDEBUG"
    AC_MSG_RESULT(yes)
else
    test "$cflags_set" = set || CFLAGS="$CFLAGS -O2"
    AC_MSG_RESULT(no)
fi

# --enable-tests option

AC_MSG_CHECKING([whether to enable tests])

AC_ARG_ENABLE(tests, 
	AC_HELP_STRING(
		[--enable-tests=[no/yes]], 
		[turn on tests [default=yes]]),
	,
	enable_tests=yes)

if test "x$enable_tests" = "xyes"; then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(ENABLE_TESTS, test x$enable_tests = xyes)



changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

# Compiler and Lib Stuff
# AC_PROG_CXX

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl header file checks
AC_CHECK_HEADERS(unistd.h, AC_DEFINE(HAVE_UNISTD_H, 1, [unistd.h found]))
AC_CHECK_HEADERS(dirent.h, AC_DEFINE(HAVE_DIRENT_H, 1, [dirent.h found]))
AC_CHECK_HEADERS([locale.h])
AC_CHECK_HEADERS([sys/time.h])

dnl Don't attempt to use fnmatch under win32 due to problems with creating DLLs.
if test "$platform_win32" = "no"; then
  AC_CHECK_HEADERS(fnmatch.h, AC_DEFINE(HAVE_FNMATCH_H, 1, [fnmatch.h found]))
fi

# Checks for nonportable Functions
AC_CHECK_FUNCS([floor getcwd gethostname gettimeofday])
AC_CHECK_FUNCS([pow setlocale strstr strtol sin])
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD

dnl NeXTStep cc seems to need this
AC_MSG_CHECKING([for extra flags for POSIX compliance])
AC_TRY_COMPILE([#include <dirent.h>], [DIR *dir;],
  AC_MSG_RESULT(none needed),
  gtk_save_CFLAGS=$CFLAGS
  CFLAGS="$CFLAGS -posix"
  AC_TRY_COMPILE([#include <dirent.h>], [DIR *dir;],
    AC_MSG_RESULT(-posix),
    AC_MSG_RESULT()
    CFLAGS=$gtk_save_CFLAGS
    AC_MSG_WARN([Could not determine POSIX flag. (-posix didn't work.)])))


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE


##################################################
# Checks for gtk-doc and docbook-tools
##################################################

# check for gtk-doc
GTK_DOC_CHECK(1.8)

AC_CHECK_PROG(DB2HTML, db2html, true, false)
AM_CONDITIONAL(HAVE_DOCBOOK, $DB2HTML)

# --enable-man option

AC_MSG_CHECKING([whether to enable man pages])

AC_ARG_ENABLE(man,
	AC_HELP_STRING(
		[--enable-man],
		[regenerate man pages from Docbook [default=no]])],
	,
	enable_man=no)

if test "${enable_man}" != no; then
    AC_MSG_RESULT(yes)

    AC_PATH_PROG([XSLTPROC], [xsltproc])
    if test -z "$XSLTPROC"; then
	enable_man=no
    fi

    if test "$enable_man" = "no" ; then
	AC_MSG_WARN([xsltproc is missing - disabling man pages])
    fi

    #check for DocBook DTD and stylesheets in the local catalog

    JH_CHECK_XML_CATALOG(
	[-//OASIS//DTD DocBook XML V4.1.2//EN],
	[DocBook XML DTD V4.1.2],
	,
	enable_man=no)

    if test "$enable_man" = "no" ; then
	AC_MSG_WARN([missing DocBook DTD - disabling man pages])
    fi

    JH_CHECK_XML_CATALOG(
	[http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl],
	[DocBook XSL Stylesheets],
	,
	enable_man=no)

    if test "$enable_man" = "no" ; then
	AC_MSG_WARN([missing DocBook XML stylesheets - disabling man pages])
    fi
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(ENABLE_MAN, test x$enable_man != xno)


##################################################
# Start of pkg-config checks
##################################################

PKG_PROG_PKG_CONFIG

# Find the GTK+ include and library directories.

# 4.5.10/fp - fontconfig should be implicit to pango  
#PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.12.0 fontconfig])
PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.12.0])

AC_SUBST(GLIB_GENMARSHAL, `$PKG_CONFIG --variable=glib_genmarshal glib-2.0`)

# --enable-glade option

AC_MSG_CHECKING([whether to enable glade integration])

AC_ARG_ENABLE(glade,
    AC_HELP_STRING(
	[--enable-glade],
	[build glade integration files [default=no]]),
    ,
    enable_glade=no)

if test "x$enable_glade" != xno; then
    AC_MSG_RESULT(yes)

    PKG_CHECK_MODULES([GLADE], [gladeui-1.0 >= 3.6])

    AC_SUBST(GLADE_CATALOG_DIR, `$PKG_CONFIG --variable=catalogdir gladeui-1.0`)
    AC_SUBST(GLADE_MODULE_DIR, `$PKG_CONFIG --variable=moduledir gladeui-1.0`)
    AC_SUBST(GLADE_PIXMAP_DIR, `$PKG_CONFIG --variable=pixmapdir gladeui-1.0`)

    #You must define GETTEXT_PACKAGE before including gi18n-lib.h
    GETTEXT_PACKAGE=AC_PACKAGE_NAME
    AC_SUBST(GETTEXT_PACKAGE)
    AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", 
	[Name of the gettext package.])
    AM_GLIB_GNU_GETTEXT
else
    AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(ENABLE_GLADE, test x$enable_glade != xno)


# --enable-introspection

AC_MSG_CHECKING([whether to enable gobject-introspection])

AC_ARG_ENABLE(introspection,
    AC_HELP_STRING(
	[--enable-introspection],
	[build gobject-introspection [default=no]]),
    ,
    enable_introspection=no)

if test "x$enable_introspection" != xno; then
    AC_MSG_RESULT(yes)
    GOBJECT_INTROSPECTION_CHECK([0.6.14])
else
    AC_MSG_RESULT(no)
    AM_CONDITIONAL(HAVE_INTROSPECTION, test 1=0)
fi
AM_CONDITIONAL(ENABLE_INTROSPECTION, test x$enable_introspection != xno)


if test "$platform_win32" = "yes"; then
  AC_SUBST(libgladegtkextra_soname, [libgladegtkextra.dll])
else
  AC_SUBST(libgladegtkextra_soname, [libgladegtkextra.so])
fi



AC_SUBST(gdktarget, `$PKG_CONFIG --variable=target gtk+-2.0`)

gtkextratargetlib=libgtkextra-$gdktarget-3.0.la

AC_SUBST(gtkextratargetlib)
AC_CHECK_LIB(m, sqrt)

if test "x$gdktarget" = "xwin32"; then
  AM_CONDITIONAL(USE_WIN32, true)
else
  AM_CONDITIONAL(USE_WIN32, false)
fi


# configure output definition

AC_CONFIG_FILES(
Makefile
glade/Makefile
glade/pixmaps/Makefile
glade/gtk+extra-catalog.xml
gtkextra/Makefile
gtkextra/gtkextrafeatures.h
docs/Makefile
docs/reference/Makefile
docs/tutorial/Makefile
gtkextra-3.0.spec
gtkextra-3.0.pc
)

AC_OUTPUT
