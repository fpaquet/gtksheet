# GtkSheet versioning: before making releases, be sure to read
# https://github.com/fpaquet/gtksheet/wiki/GtkSheet-Version-Numbering

project('GtkSheet', 'c', version : '4.2.0')

as_version = meson.project_version()
ver_arr = as_version.split('.')
GTK_SHEET_VERSION = '.'.join([ver_arr[0], ver_arr[1], ver_arr[2]])
GTK_SHEET_VERSION_SHORT = '.'.join([ver_arr[0], ver_arr[1]])

GTK_SHEET_MAJOR_VERSION = ver_arr[0]
GTK_SHEET_MINOR_VERSION = ver_arr[1]
GTK_SHEET_MICRO_VERSION = ver_arr[2]

GTK_SHEET_INTERFACE_AGE = 0
GTK_SHEET_BINARY_AGE = 0

# libtool versioning

lt_current = '3'
lt_revision = '0'
lt_age = '2'
lt_so_version = '.'.join([lt_current, lt_revision, lt_age])

conf = configuration_data()

# Surround the version in quotes to make it a C string
conf.set_quoted('VERSION', as_version)
configure_file(output : 'config.h', configuration : conf)

gtkdep = dependency('gtk+-3.0', version : '>= 3.10.0')

if get_option('enable-glade')
  gladedep = dependency('gladeui-2.0', version : '>= 3.10')
endif

subdir('gtksheet')
