# Makefile.am for gtksheet

#AUTOMAKE_OPTIONS = 1.7

gtarget=@gdktarget@

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32
gtksheet_def = gtksheet.def
gtksheet_win32_symbols = -export-symbols $(gtksheet_def)

#Not yet supported
#gtksheet_win32res_lo = gtksheet-win32res.lo
gtksheet_win32res_lo =

gtksheet-win32res.lo : gtksheet-win32.rc
	$(LT_COMPILE_RESOURCE) gtksheet-win32.rc gtksheet-win32res.lo
endif

if MS_LIB_AVAILABLE
noinst_DATA = gtksheet-win32-4.0.lib

gtksheet-win32-4.0.lib: libgtksheet-win32-4.0.la gtksheet.def
	lib -name:libgtksheet-win32-4.0-@LT_CURRENT_MINUS_AGE@.dll -def:gtksheet.def -out:$@

install-ms-lib:
	$(INSTALL) gtksheet-win32-4.0.lib $(DESTDIR)$(libdir)

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/gtksheet-win32-4.0.lib
else
install-ms-lib:
uninstall-ms-lib:
endif

# libtool stuff: set version and export symbols for resolving
# since automake doesn't support conditionalized libsomething_la_LDFLAGS
# we use the general approach here
libgtksheetincludedir = $(includedir)/gtksheet-4.0/gtksheet
AM_LDFLAGS = \
	-version-info $(LIBGTKSHEET_SO_VERSION)    \
	-export-dynamic                                         \
	-rpath $(libdir)                                        \
	$(no_undefined)                                         \
	$(GTK_LIBS)

#
# setup source file variables
#

gtksheet_public_h_sources = \
	gtkdataentry.h \
	gtkdataformat.h	\
	pango-markup.h \
	gtkdatatextview.h \
	gtksheet.h \
	gtksheetcolumn.h

gtksheet_c_sources = \
	gtkdataentry.c \
	gtkdataformat.c	\
	pango-markup.c \
	gtkdatatextview.c \
        gtksheet.c \
	gtksheetcolumn.c

gtksheet_private_h_sources = \
	gtksheet-compat.h \
	icons.h \
	pixmaps.h

gtksheet_built_public_sources =	\
	gtksheet-marshal.h	\
	gtksheettypebuiltins.h\
	gtksheet-resources.h 

gtksheet_built_sources = \
	gtksheet-marshal.c	\
	gtksheettypebuiltins.c	\
	${gtksheet_built_public_sources} \
	gtksheet-resources.c

gtksheet_sheet_sources = \
	gtksheetfeatures.h.in	\
	gtksheet-marshal.list

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) $(GTK_CFLAGS)

# target platform:
lib_LTLIBRARIES = $(gtksheettargetlib)

gtksheetincludedir = $(includedir)/gtksheet-4.0/gtksheet
gtksheetinclude_HEADERS = $(gtksheet_public_h_sources)
nodist_gtksheetinclude_HEADERS = $(gtksheet_built_public_sources) gtksheetfeatures.h

libgtksheet_4_0_la_SOURCES = $(gtksheet_c_sources)
nodist_libgtksheet_4_0_la_SOURCES = $(gtksheet_built_sources)

if USE_WIN32
libgtksheet_4_0_la_LIBADD = $(gtksheet_win32res_lo)
libgtksheet_4_0_la_DEPENDENCIES = $(gtksheet_def) $(gtksheet_win32res_lo)

libgtksheet_target_ldflags = $(gtksheet_win32_symbols) -lwsock32
endif
EXTRA_LTLIBRARIES = libgtksheet-4.0.la

MAINTAINERCLEANFILES = $(gtksheet_built_sources)
CLEANFILES = $(gtksheet_built_sources)

EXTRA_DIST = $(gtksheet_private_h_sources) $(gtksheet_sheet_sources)
#EXTRA_DIST += $(gtksheet_built_sources)
EXTRA_DIST += gtksheet.def sat.xpm sg_small.xpm cloud.xpm suncloud.xpm testgtkplotdt.in

# built sources

$(libgtksheet_4_0_la_OBJECTS): ${gtksheet_built_public_sources} ${gtksheet_built_private_headers}

gtksheet-marshal.h: $(builddir)/gtksheet-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=gtksheet $(srcdir)/gtksheet-marshal.list --header > gtksheet-marshal.tmp \
        && mv gtksheet-marshal.tmp gtksheet-marshal.h ) \
        || ( rm -f gtksheet-marshal.tmp && exit 1 )

gtksheet-marshal.c: $(builddir)/gtksheet-marshal.h
	( @GLIB_GENMARSHAL@ --prefix=gtksheet $(srcdir)/gtksheet-marshal.list --body > gtksheet-marshal.tmp \
        && mv gtksheet-marshal.tmp gtksheet-marshal.c ) \
        || ( rm -f gtksheet-marshal.tmp && exit 1 )


gtksheettypebuiltins.h: $(gtksheet_public_h_sources)
	( cd $(srcdir) && glib-mkenums \
		--fhead "#ifndef __GTKSHEET_TYPE_BUILTINS_H__\n#define __GTKSHEET_TYPE_BUILTINS_H__\n\n#include <glib-object.h>\n\nG_BEGIN_DECLS\n" \
		--fprod "/* enumerations from \"@filename@\" */\n" \
		--vhead "GType @enum_name@_get_type (void);\n#define GTK_TYPE_@ENUMSHORT@ (@enum_name@_get_type())\n" \
		--ftail "G_END_DECLS\n\n#endif /* __GTKSHEET_TYPE_BUILTINS_H__ */" \
		$(gtksheet_public_h_sources) ) > $(builddir)/gtksheettypebuiltins.h


gtksheettypebuiltins.c: $(gtksheet_public_h_sources)
	( cd $(srcdir) && glib-mkenums \
		--fhead "#define GTKSHEET_ENABLE_BROKEN\n#include \"gtksheet.h\"" \
		--fprod "\n/* enumerations from \"@filename@\" */" \
		--vhead "GType\n@enum_name@_get_type (void)\n{\n  static GType etype = 0;\n  if (etype == 0) {\n    static const G@Type@Value values[] = {" \
		--vprod "      { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
		--vtail "      { 0, NULL, NULL }\n    };\n    etype = g_@type@_register_static (\"@EnumName@\", values);\n  }\n  return etype;\n}\n" \
		$(gtksheet_public_h_sources) ) > $(builddir)/gtksheettypebuiltins.c

gtksheet-resources.c: \
		gtksheet-resources.xml \
		gtksheet.css
	glib-compile-resources \
		--sourcedir=$(srcdir) \
                --target=$@ \
                --generate-source \
                --c-name gtk_sheet \
		gtksheet-resources.xml

gtksheet-resources.h: \
		gtksheet-resources.xml \
		gtksheet.css
	glib-compile-resources \
		--sourcedir=$(srcdir) \
                --target=$@ \
                --generate-header \
                --c-name gtk_sheet \
		gtksheet-resources.xml

# test programs, not to be installed

if ENABLE_TESTS
noinst_PROGRAMS = testgtksheet
endif

DEPS = $(gtksheettargetlib) testgtksheet.css
LDADDS = $(gtksheettargetlib) $(GTK_LIBS)

testgtksheet_DEPENDENCIES = $(DEPS)
testgtksheet_LDADD = $(LDADDS)

# gobject-introspection (begin)
if ENABLE_INTROSPECTION
-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS = GtkSheet-$(GTK_SHEET_VERSION).gir

INTROSPECTION_SCANNER_ARGS = --accept-unprefixed --identifier-prefix="Gtk" --symbol-prefix="gtk" --warn-all --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = $(gtksheet_c_sources) $(gtksheet_public_h_sources)

GtkSheet-$(GTK_SHEET_VERSION).gir: $(gtksheettargetlib)
GtkSheet_@GTK_SHEET_MAJOR_VERSION@_@GTK_SHEET_MINOR_VERSION@_@GTK_SHEET_MICRO_VERSION@_gir_NAMESPACE = GtkSheet
GtkSheet_@GTK_SHEET_MAJOR_VERSION@_@GTK_SHEET_MINOR_VERSION@_@GTK_SHEET_MICRO_VERSION@_gir_VERSION = $(GTK_SHEET_VERSION)
GtkSheet_@GTK_SHEET_MAJOR_VERSION@_@GTK_SHEET_MINOR_VERSION@_@GTK_SHEET_MICRO_VERSION@_gir_INCLUDES = GLib-2.0 GObject-2.0 Gtk-3.0
GtkSheet_@GTK_SHEET_MAJOR_VERSION@_@GTK_SHEET_MINOR_VERSION@_@GTK_SHEET_MICRO_VERSION@_gir_CFLAGS = $(AM_CPPFLAGS)
GtkSheet_@GTK_SHEET_MAJOR_VERSION@_@GTK_SHEET_MINOR_VERSION@_@GTK_SHEET_MICRO_VERSION@_gir_LIBS = $(gtksheettargetlib)
GtkSheet_@GTK_SHEET_MAJOR_VERSION@_@GTK_SHEET_MINOR_VERSION@_@GTK_SHEET_MICRO_VERSION@_gir_FILES = $(introspection_sources)

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

test: testgtksheet
	builddir=`pwd`; cd $(top_builddir); top_builddir=`pwd`; export HAVE_PROGRESSBAR=1; \
	cd $$builddir; cd $(srcdir); \
	$(SHELL) $$top_builddir/libtool --mode=execute gdb $$builddir/testgtksheet;

clean-local:

