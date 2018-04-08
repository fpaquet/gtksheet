## Process this file with automake to produce Makefile.in

gladepixmapdir = @GLADE_PIXMAP_DIR@

P16DIR = hicolor/16x16/actions
P22DIR = hicolor/22x22/actions

gladepixmap16dir = @GLADE_PIXMAP_DIR@/$(P16DIR)
gladepixmap22dir = @GLADE_PIXMAP_DIR@/$(P22DIR)


PIXMAPS_16 = \
    $(P16DIR)/widget-gtksheet-gtkitementry.png \
    $(P16DIR)/widget-gtksheet-gtksheet.png \
    $(P16DIR)/widget-gtksheet-gtksheetcolumn.png

PIXMAPS_22 = \
    $(P22DIR)/widget-gtksheet-gtkitementry.png \
    $(P22DIR)/widget-gtksheet-gtksheet.png \
    $(P22DIR)/widget-gtksheet-gtksheetcolumn.png

gladepixmap16_DATA = $(PIXMAPS_16)
gladepixmap22_DATA = $(PIXMAPS_22)

EXTRA_DIST = $(PIXMAPS_16) $(PIXMAPS_22)
