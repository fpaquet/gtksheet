dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(gtksheet, 0.2.1)

# Save this value here, since automake will set cflags later
cflags_set=${CFLAGS+set}

AC_DIVERT_PUSH(NOTICE)dnl
# Making releases:
#   GTK_SHEET_MICRO_VERSION += 1;
#   GTK_SHEET_INTERFACE_AGE += 1;
#   GTK_SHEET_BINARY_AGE += 1;
# if any functions have been added, set GTK_SHEET_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set GTK_BINARY_AGE and GTK_SHEET_INTERFACE_AGE to 0.

GTK_SHEET_MAJOR_VERSION=0
GTK_SHEET_MINOR_VERSION=2
GTK_SHEET_MICRO_VERSION=1
GTK_SHEET_INTERFACE_AGE=0
GTK_SHEET_BINARY_AGE=0
GTK_SHEET_VERSION=$GTK_SHEET_MAJOR_VERSION.$GTK_SHEET_MINOR_VERSION.$GTK_SHEET_MICRO_VERSION
dnl
AC_DIVERT_POP()dnl

AC_SUBST(GTK_SHEET_MAJOR_VERSION)
AC_SUBST(GTK_SHEET_MINOR_VERSION)
AC_SUBST(GTK_SHEET_MICRO_VERSION)
AC_SUBST(GTK_SHEET_INTERFACE_AGE)
AC_SUBST(GTK_SHEET_BINARY_AGE)
AC_SUBST(GTK_SHEET_VERSION)

# libtool versioning
#LT_CURRENT=`expr $GTK_SHEET_MICRO_VERSION - $GTK_SHEET_INTERFACE_AGE`
#LT_REVISION=$GTK_SHEET_INTERFACE_AGE
#LT_AGE=`expr $GTK_SHEET_BINARY_AGE - $GTK_SHEET_INTERFACE_AGE`
#LIBGTKSHEET_SO_VERSION=$LT_CURRENT:$LT_REVISION:$LT_AGE
LIBGTKSHEET_SO_VERSION=$GTK_SHEET_MICRO_VERSION:$GTK_SHEET_MINOR_VERSION:$GTK_SHEET_MICRO_VERSION
AC_SUBST(LIBGTKSHEET_SO_VERSION)


AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR(m4)
AC_DEFINE([ENABLE_NLS],1,undef)
AC_DEFINE([HAVE_GETTEXT],1,undef)

# Build time sanity check...
AM_SANITY_CHECK

AC_PROG_CC
AC_ISC_POSIX
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

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

if test "$platform_win32" = "yes"; then
  gdktarget=win32
else
  gdktarget=x11
fi

AC_ARG_WITH(gdktarget, [  --with-gdktarget=[[x11/linux-fb/win32]] select GDK target [default depends on host system]],
        gdktarget=$with_gdktarget)

AC_SUBST(gdktarget)
case $gdktarget in
  x11|linux-fb|win32) ;;
  *) AC_MSG_ERROR([Invalid target for GDK: use x11, linux-fb or win32.]);;
esac

gtksheettargetlib=libgtksheet-$gdktarget-2.0.la

AC_SUBST(gtksheettargetlib)

if test "x$gdktarget" = "xwin32"; then
  # We start off with the libraries from Pango

  ## be sure we also have Pango built with win32 support
  PANGO_PACKAGES="pangowin32"

  if test x$have_wintab = xyes; then
    GDK_WIN32_SHEET_CFLAGS="-I $with_wintab/include"
    AC_SUBST(GDK_WIN32_SHEET_CFLAGS)
  fi

  GDK_SHEET_LIBS="$GDK_SHEET_LIBS"
  AM_CONDITIONAL(USE_WIN32, true)
else
  AM_CONDITIONAL(USE_WIN32, false)
fi


changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

# Honor aclocal flags
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl header file checks
AC_CHECK_HEADERS(unistd.h, AC_DEFINE(HAVE_UNISTD_H))
AC_CHECK_HEADERS(dirent.h, AC_DEFINE(HAVE_DIRENT_H))
dnl Don't attempt to use fnmatch under win32 due to problems with creating DLLs.
if test "$platform_win32" = "no"; then
  AC_CHECK_HEADERS(fnmatch.h, AC_DEFINE(HAVE_FNMATCH_H))
fi

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

AC_ARG_ENABLE(man,
              [AC_HELP_STRING([--enable-man],
                              [regenerate man pages from Docbook [default=no]])],enable_man=yes,
              enable_man=no)

if test "${enable_man}" != no; then
  dnl
  dnl Check for xsltproc
  dnl
  AC_PATH_PROG([XSLTPROC], [xsltproc])
  if test -z "$XSLTPROC"; then
    enable_man=no
  fi

  dnl check for DocBook DTD and stylesheets in the local catalog.
  JH_CHECK_XML_CATALOG([-//OASIS//DTD DocBook XML V4.1.2//EN],
     [DocBook XML DTD V4.1.2],,enable_man=no)
  JH_CHECK_XML_CATALOG([http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl],
     [DocBook XSL Stylesheets],,enable_man=no)
fi

AM_CONDITIONAL(ENABLE_MAN, test x$enable_man != xno)


GTK_REQUIRED=2.12.0
GLIB_REQUIRED=2.0.0

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL


PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= $GTK_REQUIRED glib-2.0 gobject-2.0])

dnl -- check for glib; redundant at this point, but sets $GLIB_MKENUMS
AM_PATH_GLIB_2_0($GLIB_REQUIRED)

AC_OUTPUT(
Makefile
docs/Makefile
docs/reference/Makefile
docs/tutorial/Makefile
gtksheet/Makefile
gtksheet/gtksheetfeatures.h
gtksheet-2.0.spec
gtksheet-2.0.pc
)
