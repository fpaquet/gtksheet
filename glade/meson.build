
private_h_sources = [
    'glade-gtksheet-editor.h',
    ]

private_c_sources = [
    'glade-gtksheet-editor.c',
    ]

c_sources = [ private_c_sources, ]

incdir = include_directories(['..', '../gtksheet'])

libgtksheet = shared_library(
    'gladegtksheet', 
    c_sources, 
    soversion : lt_current,
    version : lt_so_version,
    dependencies : deps,
    include_directories : incdir,
    link_args : ['-Wl,--no-undefined'],
    link_with: libgtksheet,
    install : true)

