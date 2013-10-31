# Makefile.am for gtk+extra

#AUTOMAKE_OPTIONS = 1.7


gtarget=@gdktarget@

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32
gtkextra_def = gtkextra.def 
gtkextra_win32_symbols = -export-symbols $(gtkextra_def)

#Not yet supported
#gtkextra_win32res_lo = gtkextra-win32res.lo
gtkextra_win32res_lo =

gtkextra-win32res.lo : gtkextra-win32.rc
	$(LT_COMPILE_RESOURCE) gtkextra-win32.rc gtkextra-win32res.lo

install-libtool-import-lib:
	$(INSTALL) .libs/libgtkextra-win32-3.0.dll.a $(DESTDIR)$(libdir)
uninstall-libtool-import-lib:
	-rm $(DESTDIR)$(libdir)/libgtkextra-win32-3.0.dll.a
install-exec-local: install-libtool-import-lib
else
install-libtool-import-lib:
uninstall-libtool-import-lib:
endif

if MS_LIB_AVAILABLE
noinst_DATA = gtkextra-win32-3.0.lib

gtkextra-win32-3.0.lib: libgtkextra-win32-3.0.la gtkextra.def
	lib -name:libgtkextra-win32-3.0-@LT_CURRENT_MINUS_AGE@.dll -def:gtkextra.def -out:$@

install-ms-lib:
	$(INSTALL) gtkextra-win32-3.0.lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/gtkextra-win32-3.0.lib
else
install-ms-lib:
uninstall-ms-lib:
endif

# libtool stuff: set version and export symbols for resolving
# since automake doesn't support conditionalized libsomething_la_LDFLAGS
# we use the general approach here
libgtkextraincludedir = $(includedir)/gtkextra-3.0/gtkextra
AM_LDFLAGS = \
	-version-info $(LIBGTKEXTRA_SO_VERSION)    \
	-export-dynamic                                         \
	-rpath $(libdir)                                        \
	$(no_undefined)                                         \
	$(GTK_LIBS)                                             

#
# setup source file variables
#


gtkextra_public_h_sources = \
        gtkbordercombo.h 	\
        gtkcharsel.h	\
        gtkcheckitem.h	\
        gtkcolorcombo.h	\
        gtkcombobutton.h	\
        gtkdataentry.h	\
        gtkdataformat.h	\
        gtkdatatextview.h	\
        gtkdirtree.h	\
        gtkextra.h	\
        gtkextraicons.h	\
        gtkfileicons.h	\
        gtkfilelist.h	\
        gtkfontcombo.h	\
        gtkiconfilesel.h	\
        gtkiconlist.h	\
        gtkitementry.h	\
        gtkplot.h	\
        gtkplot3d.h	\
        gtkplotarray.h	\
        gtkplotbar.h	\
        gtkplotbox.h	\
        gtkplotbubble.h	\
        gtkplotcairo.h	\
        gtkplotcandle.h	\
        gtkplotcanvas.h	\
        gtkplotcanvasellipse.h \
        gtkplotcanvasline.h \
        gtkplotcanvaspixmap.h \
        gtkplotcanvasplot.h \
        gtkplotcanvasrectangle.h \
        gtkplotcanvastext.h \
        gtkplotcsurface.h	\
        gtkplotdata.h	\
        gtkplotdt.h	\
        gtkplotflux.h	\
        gtkplotgdk.h	\
        gtkplotpc.h	\
        gtkplotpixmap.h	\
        gtkplotpolar.h	\
        gtkplotprint.h	\
        gtkplotps.h	\
        gtkplotsegment.h	\
        gtkplotsurface.h	\
        gtkpsfont.h	\
        gtksheet.h	\
        gtksheetcolumn.h	\
        gtktogglecombo.h

gtkextra_c_sources = 	\
        gtkbordercombo.c	\
        gtkcharsel.c	\
        gtkcheckitem.c	\
        gtkcolorcombo.c	\
        gtkcombobutton.c	\
        gtkdataentry.c	\
        gtkdataformat.c	\
        gtkdatatextview.c	\
        gtkdirtree.c	\
        gtkextra.c	\
        gtkfilelist.c	\
        gtkfontcombo.c	\
        gtkiconfilesel.c	\
        gtkiconlist.c	\
        gtkitementry.c	\
        gtkplot.c	\
        gtkplot3d.c	\
        gtkplotarray.c	\
        gtkplotbar.c	\
        gtkplotbox.c	\
        gtkplotbubble.c	\
        gtkplotcairo.c	\
        gtkplotcandle.c	\
        gtkplotcanvas.c	\
        gtkplotcanvasellipse.c \
        gtkplotcanvasline.c \
        gtkplotcanvaspixmap.c \
        gtkplotcanvasplot.c \
        gtkplotcanvasrectangle.c \
        gtkplotcanvastext.c \
        gtkplotcsurface.c	\
        gtkplotdata.c	\
        gtkplotdt.c	\
        gtkplotflux.c	\
        gtkplotgdk.c	\
        gtkplotpc.c	\
        gtkplotpixmap.c \
        gtkplotpolar.c	\
        gtkplotprint.c	\
        gtkplotps.c	\
        gtkplotsegment.c	\
        gtkplotsurface.c	\
        gtkpsfont.c	\
        gtksheet.c	\
        gtksheetcolumn.c	\
        gtktogglecombo.c 

gtkextra_private_h_sources = \
	gtkextra-compat.h \
	icons.h \
	pixmaps.h 

gtkextra_built_public_sources =	\
	gtkextra-marshal.h	\
	gtkextratypebuiltins.h

gtkextra_built_sources = \
	gtkextra-marshal.c	\
	gtkextratypebuiltins.c	\
	${gtkextra_built_public_sources}            

gtkextra_extra_sources = \
	gtkextrafeatures.h.in	\
	gtkextra-marshal.list 	

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) $(GTK_CFLAGS) 

# target platform:
lib_LTLIBRARIES = $(gtkextratargetlib)

gtkextraincludedir = $(includedir)/gtkextra-3.0/gtkextra
gtkextrainclude_HEADERS = $(gtkextra_public_h_sources)
nodist_gtkextrainclude_HEADERS = $(gtkextra_built_public_sources) gtkextrafeatures.h

libgtkextra_x11_3_0_la_SOURCES = $(gtkextra_c_sources)
nodist_libgtkextra_x11_3_0_la_SOURCES = $(gtkextra_built_sources)
libgtkextra_linux_fb_3_0_la_SOURCES = $(gtkextra_c_sources)
nodist_libgtkextra_linux_fb_3_0_la_SOURCES = $(gtkextra_built_sources)
libgtkextra_win32_3_0_la_SOURCES = $(gtkextra_c_sources)
nodist_libgtkextra_win32_3_0_la_SOURCES = $(gtkextra_built_sources)
libgtkextra_quartz_3_0_la_SOURCES = $(gtkextra_c_sources)
nodist_libgtkextra_quartz_3_0_la_SOURCES = $(gtkextra_built_sources)

libgtkextra_win32_3_0_la_LIBADD = $(gtkextra_win32res_lo)
libgtkextra_win32_3_0_la_DEPENDENCIES = $(gtkextra_def) $(gtkextra_win32res_lo)

if USE_WIN32
libgtkextra_target_ldflags = $(gtkextra_win32_symbols) -lwsock32
endif
EXTRA_LTLIBRARIES = libgtkextra-x11-3.0.la libgtkextra-linux-fb-3.0.la libgtkextra-win32-3.0.la libgtkextra-quartz-3.0.la

MAINTAINERCLEANFILES = $(gtkextra_built_sources) 
CLEANFILES = $(gtkextra_built_sources) 

EXTRA_DIST = $(gtkextra_private_h_sources) $(gtkextra_extra_sources)
#EXTRA_DIST += $(gtkextra_built_sources)
EXTRA_DIST += gtkextra.def sat.xpm sg_small.xpm cloud.xpm suncloud.xpm testgtkplotdt.in


$(libgtkextra_x11_3_0_la_OBJECTS) $(libgtkextra_linux_fb_3_0_la_OBJECTS) $(libgtkextra_win32_3_0_la_OBJECTS) $(libgtkextra_quartz_3_0_la_OBJECTS): ${gtkextra_built_public_sources} ${gtkextra_built_private_headers}

gtkextra-marshal.h: $(builddir)/gtkextra-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=gtkextra $(srcdir)/gtkextra-marshal.list --header > gtkextra-marshal.tmp \
        && mv gtkextra-marshal.tmp gtkextra-marshal.h ) \
        || ( rm -f gtkextra-marshal.tmp && exit 1 )

gtkextra-marshal.c: $(builddir)/gtkextra-marshal.h
	( @GLIB_GENMARSHAL@ --prefix=gtkextra $(srcdir)/gtkextra-marshal.list --body > gtkextra-marshal.tmp \
        && mv gtkextra-marshal.tmp gtkextra-marshal.c ) \
        || ( rm -f gtkextra-marshal.tmp && exit 1 )


gtkextratypebuiltins.h: $(gtkextra_public_h_sources)
	( cd $(srcdir) && glib-mkenums \
		--fhead "#ifndef __GTKEXTRA_TYPE_BUILTINS_H__\n#define __GTKEXTRA_TYPE_BUILTINS_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void);\n#define GTK_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n#endif /* __GTKEXTRA_TYPE_BUILTINS_H__ */" \
		$(gtkextra_public_h_sources) ) > $(builddir)/gtkextratypebuiltins.h


gtkextratypebuiltins.c: $(gtkextra_public_h_sources)
	( cd $(srcdir) && glib-mkenums \
		--fhead "#define GTKEXTRA_ENABLE_BROKEN\n#include \"gtkextra.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(gtkextra_public_h_sources) ) > $(builddir)/gtkextratypebuiltins.c


# test programs, not to be installed

if ENABLE_TESTS
noinst_PROGRAMS = testgtksheet testgtkiconlist testgtkfilesel testgtkplot testgtkfont testgtkplotcanvas testrealtime testiterator testpolar testgtkplot3d testcontour testbubbles testflux testboxes testcandles testimage testpixmap testcharsel testdt testgtkplotdt testsegment testprintexport
endif
DEPS = $(gtkextratargetlib)
LDADDS = $(gtkextratargetlib) $(GTK_LIBS)

testprintexport_DEPENDENCIES = $(DEPS)
testgtksheet_DEPENDENCIES = $(DEPS)
testgtkplot_DEPENDENCIES = $(DEPS)
testdt_DEPENDENCIES = $(DEPS)
testgtkplotdt_DEPENDENCIES = $(DEPS)
testpolar_DEPENDENCIES = $(DEPS)
testgtkplot3d_DEPENDENCIES = $(DEPS)
testcontour_DEPENDENCIES = $(DEPS)
testrealtime_DEPENDENCIES = $(DEPS)
testgtkfilesel_DEPENDENCIES = $(DEPS)
testgtkiconlist_DEPENDENCIES = $(DEPS)
testgtkfont_DEPENDENCIES = $(DEPS)
testbubbles_DEPENDENCIES = $(DEPS)
testflux_DEPENDENCIES = $(DEPS)
testboxes_DEPENDENCIES = $(DEPS)
testcandles_DEPENDENCIES = $(DEPS)
testpixmap_DEPENDENCIES = $(DEPS)
testimage_DEPENDENCIES = $(DEPS)
testcharsel_DEPENDENCIES = $(DEPS)
testiterator_DEPENDENCIES = $(DEPS)
testsegment_DEPENDENCIES = $(DEPS)
testgtkplotcanvas_DEPENDENCIES = $(DEPS)
testgtksheet_LDADD = $(LDADDS)
testgtkplot_LDADD = $(LDADDS)
testdt_LDADD = $(LDADDS)
testgtkplotdt_LDADD = $(LDADDS)
testimage_LDADD = $(LDADDS)
testcharsel_LDADD = $(LDADDS)
testpixmap_LDADD = $(LDADDS)
testpolar_LDADD = $(LDADDS)
testgtkplot3d_LDADD = $(LDADDS)
testcontour_LDADD = $(LDADDS)
testrealtime_LDADD = $(LDADDS)
testgtkfilesel_LDADD = $(LDADDS)
testgtkiconlist_LDADD = $(LDADDS)
testgtkfont_LDADD = $(LDADDS)
testbubbles_LDADD = $(LDADDS)
testflux_LDADD = $(LDADDS)
testboxes_LDADD = $(LDADDS)
testcandles_LDADD = $(LDADDS)
testiterator_LDADD = $(LDADDS)
testsegment_LDADD = $(LDADDS)
testgtkplotcanvas_LDADD = $(LDADDS)
testprintexport_LDADD = $(LDADDS)

# gobject-introspection (begin)
if ENABLE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS = GtkExtra-3.0.gir

INTROSPECTION_SCANNER_ARGS = --accept-unprefixed --identifier-prefix="Gtk" --symbol-prefix="gtk" --warn-all --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = $(gtkextra_c_sources) $(gtkextra_public_h_sources)

GtkExtra-3.0.gir: $(gtkextratargetlib)
GtkExtra_3_0_gir_NAMESPACE = GtkExtra
GtkExtra_3_0_gir_VERSION = $(GTK_EXTRA_VERSION)
GtkExtra_3_0_gir_INCLUDES = GLib-2.0 GObject-2.0 Gtk-2.0
GtkExtra_3_0_gir_CFLAGS = $(AM_CPPFLAGS)
GtkExtra_3_0_gir_LIBS = $(gtkextratargetlib)
GtkExtra_3_0_gir_FILES = $(introspection_sources)

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) # $(typelib_DATA)
endif
# gobject-introspection (end)


files:
	@files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
		echo $$p; \
	done

#test: testgtksheet testgtkiconlist testgtkfilesel testgtkplot testgtkfont testrealtime testpolar testgtkplot3d testcontour testbubbles testflux testboxes testcandles testimage testpixmap testcharsel testdt testgtkplotdt testiterator
#        builddir=`pwd`; cd $(top_builddir); top_builddir=`pwd`; export HAVE_PROGRESSBAR=1; \
#	cd $$builddir; cd $(srcdir); \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtksheet;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkplot;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkpolar;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkplot3d;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkbubble;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkflux;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testcontour;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkiconlist;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkfilesel;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtkfont;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testrealtime;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testiterator;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testimage;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testcandles;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testpixmap;  \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testcharsel; \
#	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testdt \
#	$$builddir/testgtkplotdt.in > $$builddir/testgtkplotdt.out; echo

clean-local:
	rm -rf gtkextrafeatures.h

