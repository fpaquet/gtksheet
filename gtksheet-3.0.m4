# Configure paths for GtkSheet
# created Owen Taylor 1997-2001
# modified F. Paquet 2018

dnl AM_PATH_GTK_SHEET_3_0([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GTK_SHEET, and define GTK_SHEET_CFLAGS and GTK_SHEET_LIBS, if gthread is specified in MODULES, 
dnl pass to pkg-config
dnl
AC_DEFUN([AM_PATH_GTK_SHEET_3_0],
[dnl 
dnl Get the cflags and libraries from pkg-config
dnl
AC_ARG_ENABLE(gtksheettest, [  --disable-gtksheettest       do not try to compile and run a test GTK_SHEET program],
		    , enable_gtksheettest=yes)

  pkg_config_args=gtksheet-3.0
  for module in . $4
  do
      case "$module" in
         gthread) 
             pkg_config_args="$pkg_config_args gthread-2.0"
         ;;
      esac
  done

  no_gtksheet=""

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)

  if test x$PKG_CONFIG != xno ; then
    if pkg-config --atleast-pkgconfig-version 0.7 ; then
      :
    else
      echo "*** pkg-config too old; version 0.7 or better required."
      no_gtksheet=yes
      PKG_CONFIG=no
    fi
  else
    no_gtksheet=yes
  fi

  min_gtksheet_version=ifelse([$1], ,3.0.0,$1)
  AC_MSG_CHECKING(for GTK_SHEET - version >= $min_gtksheet_version)

  if test x$PKG_CONFIG != xno ; then
    ## don't try to run the test against uninstalled libtool libs
    if $PKG_CONFIG --uninstalled $pkg_config_args; then
	  echo "Will use uninstalled version of GTK_SHEET found in PKG_CONFIG_PATH"
	  enable_gtksheettest=no
    fi

    if $PKG_CONFIG --atleast-version $min_gtksheet_version $pkg_config_args; then
	  :
    else
	  no_gtksheet=yes
    fi
  fi

  if test x"$no_gtksheet" = x ; then
    GTK_SHEET_CFLAGS=`$PKG_CONFIG $pkg_config_args --cflags`
    GTK_SHEET_LIBS=`$PKG_CONFIG $pkg_config_args --libs`
    gtksheet_config_major_version=`$PKG_CONFIG --modversion gtksheet-3.0 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtksheet_config_minor_version=`$PKG_CONFIG --modversion gtksheet-3.0 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtksheet_config_micro_version=`$PKG_CONFIG --modversion gtksheet-3.0 | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtksheettest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_SHEET_CFLAGS"
      LIBS="$GTK_SHEET_LIBS $LIBS"
dnl
dnl Now check if the installed GTK_SHEET is sufficiently new. (Also sanity
dnl checks the results of pkg-config to some extent)
dnl
      rm -f conf.gtksheettest
      AC_TRY_RUN([
#include <gtksheet/gtksheet.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtksheettest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtksheet_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtksheet_version");
     exit(1);
   }

  if ((gtksheet_major_version != $gtksheet_config_major_version) ||
      (gtksheet_minor_version != $gtksheet_config_minor_version) ||
      (gtksheet_micro_version != $gtksheet_config_micro_version))
    {
      printf("\n*** 'pkg-config --modversion gtksheet-3.0' returned %d.%d.%d, but GTK_SHEET+ (%d.%d.%d)\n", 
             $gtksheet_config_major_version, $gtksheet_config_minor_version, $gtksheet_config_micro_version,
             gtksheet_major_version, gtksheet_minor_version, gtksheet_micro_version);
      printf ("*** was found! If pkg-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK_SHEET. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If pkg-config was wrong, set the environment variable PKG_CONFIG_PATH\n");
      printf("*** to point to the correct configuration files\n");
    } 
  else if ((gtksheet_major_version != GTK_SHEET_MAJOR_VERSION) ||
	   (gtksheet_minor_version != GTK_SHEET_MINOR_VERSION) ||
           (gtksheet_micro_version != GTK_SHEET_MICRO_VERSION))
    {
      printf("*** GTK_SHEET header files (version %d.%d.%d) do not match\n",
	     GTK_SHEET_MAJOR_VERSION, GTK_SHEET_MINOR_VERSION, GTK_SHEET_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtksheet_major_version, gtksheet_minor_version, gtksheet_micro_version);
    }
  else
    {
      if ((gtksheet_major_version > major) ||
        ((gtksheet_major_version == major) && (gtksheet_minor_version > minor)) ||
        ((gtksheet_major_version == major) && (gtksheet_minor_version == minor) && (gtksheet_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK_SHEET (%d.%d.%d) was found.\n",
               gtksheet_major_version, gtksheet_minor_version, gtksheet_micro_version);
        printf("*** You need a version of GTK_SHEET newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK_SHEET is always available from GitHub.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the pkg-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK_SHEET, but you can also set the PKG_CONFIG environment to point to the\n");
        printf("*** correct copy of pkg-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtksheet=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtksheet" = x ; then
     AC_MSG_RESULT(yes (version $gtksheet_config_major_version.$gtksheet_config_minor_version.$gtksheet_config_micro_version))
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$PKG_CONFIG" = "no" ; then
       echo "*** A new enough version of pkg-config was not found."
       echo "*** See http://pkgconfig.sourceforge.net"
     else
       if test -f conf.gtksheettest ; then
        :
       else
          echo "*** Could not run GTK_SHEET test program, checking why..."
	  ac_save_CFLAGS="$CFLAGS"
	  ac_save_LIBS="$LIBS"
          CFLAGS="$CFLAGS $GTK_SHEET_CFLAGS"
          LIBS="$LIBS $GTK_SHEET_LIBS"
          AC_TRY_LINK([
#include <gtksheet/gtksheet.h>
#include <stdio.h>
],      [ return ((gtksheet_major_version) || (gtksheet_minor_version) || (gtksheet_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK_SHEET or finding the wrong"
          echo "*** version of GTK_SHEET. If it is not finding GTK_SHEET, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK_SHEET is incorrectly installed."])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_SHEET_CFLAGS=""
     GTK_SHEET_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_SHEET_CFLAGS)
  AC_SUBST(GTK_SHEET_LIBS)
  rm -f conf.gtksheettest
])
