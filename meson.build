# GtkSheet versioning: before making releases, be sure to read
# https://github.com/fpaquet/gtksheet/wiki/GtkSheet-Version-Numbering

project('GtkSheet', 'c', version: '4.3.4')

as_version = meson.project_version()
ver_arr = as_version.split('.')
GTK_SHEET_VERSION = '.'.join([ver_arr[0], ver_arr[1], ver_arr[2]])
GTK_SHEET_VERSION_SHORT = '.'.join([ver_arr[0], ver_arr[1]])

GTK_SHEET_MAJOR_VERSION = ver_arr[0]
GTK_SHEET_MINOR_VERSION = ver_arr[1]
GTK_SHEET_MICRO_VERSION = ver_arr[2]

GTK_SHEET_VERSION_BASE = '.'.join([ver_arr[0], '0'])

# libtool versioning

lt_current = '1'
lt_revision = '4'
lt_age = '3'
lt_so_version = '.'.join([lt_current, lt_revision, lt_age])

conf = configuration_data()

# Surround the version in quotes to make it a C string
conf.set_quoted('VERSION', as_version)
configure_file(output: 'config.h', configuration: conf)

dep_gtk = dependency('gtk+-3.0', version: '>= 3.10.0')
dep_glib = dependency('glib-2.0', version: '>= 2.52.3')
dep_gobject = dependency('gobject-2.0', version: '>= 2.52.3')

deps = [dep_gtk, dep_glib, dep_gobject]

if get_option('enable-glade')
    dep_glade = dependency('gladeui-2.0', version: '>= 3.10')
    i18n = import('i18n')

    add_project_arguments(
        '-DGETTEXT_PACKAGE="GtkSheet"', language: 'c')

    deps += dep_glade
endif

if get_option('enable-introspection')
    dep_gobjintro = dependency(
        'gobject-introspection-1.0', version: '>= 1.46')
    deps += dep_gobjintro
endif

gnome = import('gnome')

if get_option('enable-gtk-doc')
    gtkdoc = find_program('gtkdoc-scan', required: false)

    if gtkdoc.found()
        gtkdoc_result = run_command(gtkdoc, ['--version'])
        gtkdoc_ver = gtkdoc_result.stdout().strip()
        if gtkdoc_ver == ''
            gtkdoc_ver = gtkdoc_result.stderr().strip()
            endif
        if gtkdoc_ver.version_compare('<1.26')
            error('gtk-doc test requires gtkdoc >= 1.26.')
        endif
    else
        error('gtkdoc not found.')
    endif
endif

if get_option('buildtype').startswith('debug')
    add_project_arguments('-DDEBUG', language: ['c'])
endif

subdir('gtksheet')

pkg = import('pkgconfig')
pkg.generate(
    libraries: [ libgtksheet, deps ],
    name: 'GtkSheet',
    filebase: 'gtksheet-' + GTK_SHEET_VERSION_BASE,
    description: 'A spreadsheet like matrix widget for Gtk+'
    )

if get_option('enable-glade')
    subdir('glade')
    #subdir('po')
endif

if get_option('enable-gtk-doc')
    subdir('docs/reference')
    subdir('docs/tutorial')
endif
