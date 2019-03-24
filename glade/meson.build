
private_h_sources = [
    'glade-gtksheet-editor.h',
]

private_c_sources = [
    'glade-gtksheet-editor.c',
]

c_sources = [ private_c_sources, ]

incdir = include_directories(['..', '../gtksheet'])

catalogdir = dep_glade.get_pkgconfig_variable('catalogdir')
moduledir = dep_glade.get_pkgconfig_variable('moduledir')
pixmapdir = dep_glade.get_pkgconfig_variable('pixmapdir')

libggladegtksheet = shared_library(
    'gladegtksheet',
    c_sources,
    soversion: lt_current,
    version: lt_so_version,
    dependencies: deps,
    include_directories: incdir,
    link_args: ['-Wl,--no-undefined'],
    link_with: libgtksheet,
    install: true,
    install_dir: moduledir,
    )

# beware - to be checked for backslash on Windows
so_basename = libggladegtksheet.full_path().split('/')[-1]

gladeconf = configuration_data()
gladeconf.set('libgladegtksheet_soname', so_basename)

configure_file(
    input: 'gtksheet-catalog.xml.in',
    output: 'gtksheet-catalog.xml',
    configuration: gladeconf,
    install_dir: catalogdir,
    )

pixdir = '../glade/pixmaps/'
pix16dir = 'hicolor/16x16/actions/'
pix22dir = 'hicolor/22x22/actions/'

pix16_sources = [
    pixdir + pix16dir + 'widget-gtksheet-gtkitementry.png',
    pixdir + pix16dir + 'widget-gtksheet-gtksheet.png',
    pixdir + pix16dir + 'widget-gtksheet-gtksheetcolumn.png',
]

install_data(
    pix16_sources,
    install_dir: pixmapdir + '/' + pix16dir,
    )

pix22_sources = [
    pixdir + pix22dir + 'widget-gtksheet-gtkitementry.png',
    pixdir + pix22dir + 'widget-gtksheet-gtksheet.png',
    pixdir + pix22dir + 'widget-gtksheet-gtksheetcolumn.png',
]

install_data(
    pix22_sources,
    install_dir: pixmapdir + '/' + pix22dir,
    )

