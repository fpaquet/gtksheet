
public_h_sources = [
    'gtkdataentry.h',
	'gtkdataformat.h', 
	'pango-markup.h',
	'gtkdatatextview.h',
	'gtksheet.h',
	'gtksheetcolumn.h',
    ]

private_h_sources = [
    'gtksheet-compat.h',
    'icons.h',
    'pixmaps.h',
    ]

private_c_sources = [
	'gtkdataentry.c',
	'gtkdataformat.c', 
	'pango-markup.c',
	'gtkdatatextview.c',
    'gtksheet.c',
	'gtksheetcolumn.c',
    ]

built_public_h_sources =	[
	#'gtksheet-marshal.h',
	'gtksheettypebuiltins.h',
	#'gtksheet-resources.h',
    ]

built_c_sources =	[
	#'gtksheet-marshal.c',
	'gtksheettypebuiltins.c',
	#'gtksheet-resources.c',
    ]

built_sources = [ built_public_h_sources, built_c_sources, ]

sheet_sources = [
	'gtksheetfeatures.h.in',
	'gtksheet-marshal.list',
    ]

c_sources = [ private_c_sources, ]

enums = gnome.mkenums_simple(
    'gtksheettypebuiltins', 
    sources : public_h_sources)

resources = gnome.compile_resources(
  'gtksheet-resources', 'gtksheet-resources.xml', 'gtksheet.css',
  source_dir : '.',
  c_name : 'gtk_sheet')

marshallers = gnome.genmarshal(
    'gtksheet-marshal', 
    sources : 'gtksheet-marshal.list',
    prefix: 'gtksheet',
    valist_marshallers: true,
    )

libgtksheet = shared_library(
    'gtksheet', 
    c_sources, resources, marshallers, enums, 
    soversion : lt_current,
    version : lt_so_version,
    dependencies : deps,
    include_directories : include_directories('..'),
    link_args : ['-Wl,--no-undefined'],
    install : true)

install_headers(
    public_h_sources, subdir : 'gtksheet-4.0/gtksheet')

if get_option('enable-introspection')
    gnome.generate_gir(
        libgtksheet,
        sources : c_sources + public_h_sources,
        nsversion : GTK_SHEET_VERSION,
        namespace : 'GtkSheet',
        symbol_prefix : 'gtk',
        identifier_prefix : 'Gtk',
        includes : ['GLib-2.0', 'GObject-2.0', 'Gtk-3.0'],
        install : true)
endif


if get_option('enable-tests')
    testgtksheet_c_sources = 'testgtksheet.c'

    testgtksheet = executable(
        'testgtksheet', testgtksheet_c_sources,
        include_directories : include_directories('..'),
        dependencies : deps,
        link_with : libgtksheet)

    test('testgtksheet', testgtksheet,
         workdir : meson.current_source_dir())
endif

