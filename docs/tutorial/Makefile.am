
EXTRA_DIST = \
	gtkextra-tutorial.sgml	\
	images/gtksheet_cell_text.jpg	\
	images/gtksheet_new_sheet.jpg	\
	images/gtksheet_pack_combo.jpg \
	images/gtksheet_pack_xpm.jpg	\
	images/gtksheet_range_bg.jpg

if HAVE_DOCBOOK
all: html
html:
	if test -w $(srcdir); then \
	  (cd $(srcdir); \
	   db2html gtkextra-tutorial.sgml; \
	   test -d html && rm -r html; \
	   mv gtkextra-tutorial html; \
	   mkdir html/images; \
	   cp images/*.* html/images); \
	fi

pdf:
	if test -w $(srcdir); then \
	  (cd $(srcdir); db2pdf gtkextra-tutorial.sgml); \
	fi

dist-hook: html
	cp -Rp $(srcdir)/html $(distdir)

else
dist-hook:
	echo "***"
	echo "*** Warning: Tutorial not built"
	echo "*** DISTRIBUTION IS INCOMPLETE"
	echo "***"

endif

