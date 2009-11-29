## Makefile.am for gtk+sheet

SRC_SUBDIRS = gtksheet 
SUBDIRS = $(SRC_SUBDIRS) docs

# require automake 1.7
AUTOMAKE_OPTIONS = 1.7

EXTRA_DIST = \
	TODO \
	autogen.sh \
	config.h.win32 \
	gtksheet-2.0.spec \
	ChangeLog.gtkextra \
	gitlog2changelog.py

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gtksheet-2.0.pc

.PHONY: files release sanity snapshot

files:
	@files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
          echo $$p; \
        done
	@for subdir in $(SUBDIRS); do \
          files=`cd $$subdir; $(MAKE) files | grep -v "make\[[1-9]\]"`; \
          for file in $$files; do \
            echo $$subdir/$$file; \
          done; \
        done

release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

sanity:
	./sanity_check $(VERSION)

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%y%m%d"`

ChangeLog.git:
	./gitlog2changelog.py ChangeLog.git

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-man --disable-rebuilds
