## Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps

glademoduledir = @GLADE_MODULE_DIR@
gladecatalogdir = @GLADE_CATALOG_DIR@
gladepixmapdir = @GLADE_PIXMAP_DIR@

libgladegtkextra = $(top_builddir)/glade/libgladegtkextra.la

GLADEUI_LIBS = `pkg-config --libs gladeui-1.0`

# activate this to install into glade-3 modules dir
glademodule_LTLIBRARIES = libgladegtkextra.la

# or activate this to install into gtkextra libdir
#lib_LTLIBRARIES = libgladegtkextra.la

libgladegtkextra_la_CPPLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(AM_CPPFLAGS)

libgladegtkextra_la_CFLAGS =  \
	-DG_LOG_DOMAIN=\"GladeUI-GTKExtra\" \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(CFLAGS) \
	@GLADE_CFLAGS@ \
	$(PLUGINS_WARN_CFLAGS) \
	$(AM_CFLAGS)

libgladegtkextra_la_SOURCES = glade-gtksheet-editor.c

libgladegtkextra_la_LDFLAGS = -module -avoid-version $(AM_LDFLAGS) $(GLADEUI_LIBS)
libgladegtkextra_la_LIBADD = ../gtkextra/@gtkextratargetlib@ 

if PLATFORM_WIN32
libgladegtkextra_la_LDFLAGS += -no-undefined
endif

# catalog data

gladecatalog_DATA = gtk+extra-catalog.xml

EXTRA_DIST = gtk+extra-catalog.xml
