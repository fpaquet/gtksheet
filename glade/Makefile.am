## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps

glademoduledir = @GLADE_MODULE_DIR@
gladecatalogdir = @GLADE_CATALOG_DIR@
gladepixmapdir = @GLADE_PIXMAP_DIR@

libgladegtksheet = $(top_builddir)/glade/libgladegtksheet.la

GLADEUI_LIBS = `pkg-config --libs gladeui-1.0`

# activate this to install into glade-3 modules dir
glademodule_LTLIBRARIES = libgladegtksheet.la

# or activate this to install into gtksheet libdir
#lib_LTLIBRARIES = libgladegtksheet.la

libgladegtksheet_la_CPPLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(AM_CPPFLAGS)

libgladegtksheet_la_CFLAGS =  \
	-DG_LOG_DOMAIN=\"GladeUI-GTKExtra\" \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(CFLAGS) \
	@GLADE_CFLAGS@ \
	$(PLUGINS_WARN_CFLAGS) \
	$(AM_CFLAGS)

libgladegtksheet_la_SOURCES = glade-gtksheet-editor.c

libgladegtksheet_la_LDFLAGS = -module -avoid-version $(AM_LDFLAGS) $(GLADEUI_LIBS)
libgladegtksheet_la_LIBADD = ../gtksheet/@gtksheettargetlib@ 

if PLATFORM_WIN32
libgladegtksheet_la_LDFLAGS += -no-undefined
endif

# catalog data

gladecatalog_DATA = gtksheet-catalog.xml

EXTRA_DIST = gtksheet-catalog.xml
