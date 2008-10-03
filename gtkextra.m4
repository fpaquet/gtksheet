# Configure paths for GTK+EXTRA
# Owen Taylor     97-11-3
# Adrian Feiguin  01-04-03 

dnl AM_PATH_GTK_EXTRA([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
dnl Test for GTK_EXTRA, and define GTK_EXTRA_CFLAGS and GTK_EXTRA_LIBS
dnl
AC_DEFUN(AM_PATH_GTK_EXTRA,
[dnl 
dnl Get the cflags and libraries from the gtkextra-config script
dnl
AC_ARG_WITH(gtkextra-prefix,[  --with-gtkextra-prefix=PFX   Prefix where GTK_EXTRA is installed (optional)],
            gtkextra_config_prefix="$withval", gtkextra_config_prefix="")
AC_ARG_WITH(gtkextra-exec-prefix,[  --with-gtkextra-exec-prefix=PFX Exec prefix where GTK_EXTRA is installed (optional)],
            gtkextra_config_exec_prefix="$withval", gtkextra_config_exec_prefix="")
AC_ARG_ENABLE(gtkextratest, [  --disable-gtkextratest       Do not try to compile and run a test GTK_EXTRA program],
		    , enable_gtkextratest=yes)

  for module in . $4
  do
      case "$module" in
         gthread) 
             gtkextra_config_args="$gtkextra_config_args gthread"
         ;;
      esac
  done

  if test x$gtkextra_config_exec_prefix != x ; then
     gtkextra_config_args="$gtkextra_config_args --exec-prefix=$gtkextra_config_exec_prefix"
     if test x${GTK_EXTRA_CONFIG+set} != xset ; then
        GTK_EXTRA_CONFIG=$gtkextra_config_exec_prefix/bin/gtkextra-config
     fi
  fi
  if test x$gtkextra_config_prefix != x ; then
     gtkextra_config_args="$gtkextra_config_args --prefix=$gtkextra_config_prefix"
     if test x${GTK_EXTRA_CONFIG+set} != xset ; then
        GTK_EXTRA_CONFIG=$gtkextra_config_prefix/bin/gtkextra-config
     fi
  fi

  AC_PATH_PROG(GTK_EXTRA_CONFIG, gtkextra-config, no)
  min_gtkextra_version=ifelse([$1], ,0.99.13,$1)
  AC_MSG_CHECKING(for GTK_EXTRA - version >= $min_gtkextra_version)
  no_gtkextra=""
  if test "$GTK_EXTRA_CONFIG" = "no" ; then
    no_gtkextra=yes
  else
    GTK_EXTRA_CFLAGS=`$GTK_EXTRA_CONFIG $gtkextra_config_args --cflags`
    GTK_EXTRA_LIBS=`$GTK_EXTRA_CONFIG $gtkextra_config_args --libs`
    gtkextra_config_major_version=`$GTK_EXTRA_CONFIG $gtkextra_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    gtkextra_config_minor_version=`$GTK_EXTRA_CONFIG $gtkextra_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    gtkextra_config_micro_version=`$GTK_EXTRA_CONFIG $gtkextra_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_gtkextratest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $GTK_EXTRA_CFLAGS"
      LIBS="$GTK_EXTRA_LIBS $LIBS"
dnl
dnl Now check if the installed GTK_EXTRA is sufficiently new. (Also sanity
dnl checks the results of gtkextra-config to some extent
dnl
      rm -f conf.gtkextratest
      AC_TRY_RUN([
#include <gtkextra/gtkextra.h>
#include <stdio.h>
#include <stdlib.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.gtkextratest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = g_strdup("$min_gtkextra_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_gtkextra_version");
     exit(1);
   }

  if ((gtkextra_major_version != $gtkextra_config_major_version) ||
      (gtkextra_minor_version != $gtkextra_config_minor_version) ||
      (gtkextra_micro_version != $gtkextra_config_micro_version))
    {
      printf("\n*** 'gtkextra-config --version' returned %d.%d.%d, but GTK_EXTRA+ (%d.%d.%d)\n", 
             $gtkextra_config_major_version, $gtkextra_config_minor_version, $gtkextra_config_micro_version,
             gtkextra_major_version, gtkextra_minor_version, gtkextra_micro_version);
      printf ("*** was found! If gtkextra-config was correct, then it is best\n");
      printf ("*** to remove the old version of GTK_EXTRA+. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If gtkextra-config was wrong, set the environment variable GTK_EXTRA_CONFIG\n");
      printf("*** to point to the correct copy of gtkextra-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
#if defined (GTK_EXTRA_MAJOR_VERSION) && defined (GTK_EXTRA_MINOR_VERSION) && defined (GTK_EXTRA_MICRO_VERSION)
  else if ((gtkextra_major_version != GTK_EXTRA_MAJOR_VERSION) ||
	   (gtkextra_minor_version != GTK_EXTRA_MINOR_VERSION) ||
           (gtkextra_micro_version != GTK_EXTRA_MICRO_VERSION))
    {
      printf("*** GTK_EXTRA+ header files (version %d.%d.%d) do not match\n",
	     GTK_EXTRA_MAJOR_VERSION, GTK_EXTRA_MINOR_VERSION, GTK_EXTRA_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     gtkextra_major_version, gtkextra_minor_version, gtkextra_micro_version);
    }
#endif /* defined (GTK_EXTRA_MAJOR_VERSION) ... */
  else
    {
      if ((gtkextra_major_version > major) ||
        ((gtkextra_major_version == major) && (gtkextra_minor_version > minor)) ||
        ((gtkextra_major_version == major) && (gtkextra_minor_version == minor) && (gtkextra_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of GTK_EXTRA+ (%d.%d.%d) was found.\n",
               gtkextra_major_version, gtkextra_minor_version, gtkextra_micro_version);
        printf("*** You need a version of GTK_EXTRA+ newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("*** GTK_EXTRA+ is always available from ftp://ftp.gtkextra.org.\n");
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the gtkextra-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of GTK_EXTRA+, but you can also set the GTK_EXTRA_CONFIG environment to point to the\n");
        printf("*** correct copy of gtkextra-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_gtkextra=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_gtkextra" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$GTK_EXTRA_CONFIG" = "no" ; then
       echo "*** The gtkextra-config script installed by GTK_EXTRA could not be found"
       echo "*** If GTK_EXTRA was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the GTK_EXTRA_CONFIG environment variable to the"
       echo "*** full path to gtkextra-config."
     else
       if test -f conf.gtkextratest ; then
        :
       else
          echo "*** Could not run GTK_EXTRA test program, checking why..."
          CFLAGS="$CFLAGS $GTK_EXTRA_CFLAGS"
          LIBS="$LIBS $GTK_EXTRA_LIBS"
          AC_TRY_LINK([
#include <gtkextra/gtkextra.h>
#include <stdio.h>
],      [ return ((gtkextra_major_version) || (gtkextra_minor_version) || (gtkextra_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding GTK_EXTRA or finding the wrong"
          echo "*** version of GTK_EXTRA. If it is not finding GTK_EXTRA, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***"
          echo "*** If you have a RedHat 5.0 system, you should remove the GTK_EXTRA package that"
          echo "*** came with the system with the command"
          echo "***"
          echo "***    rpm --erase --nodeps gtkextra gtkextra-devel" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means GTK_EXTRA was incorrectly installed"
          echo "*** or that you have moved GTK_EXTRA since it was installed. In the latter case, you"
          echo "*** may want to edit the gtkextra-config script: $GTK_EXTRA_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     GTK_EXTRA_CFLAGS=""
     GTK_EXTRA_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(GTK_EXTRA_CFLAGS)
  AC_SUBST(GTK_EXTRA_LIBS)
  rm -f conf.gtkextratest
])
