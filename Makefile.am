## Makefile.am for gtk+extra

ACLOCAL_AMFLAGS = -I m4

if ENABLE_GLADE
    GLADE_SUBDIR = glade
endif

SRC_SUBDIRS = gtkextra  $(GLADE_SUBDIR)
SUBDIRS = $(SRC_SUBDIRS) docs

# require automake 1.7
#AUTOMAKE_OPTIONS = 1.7

EXTRA_DIST = \
	TODO \
	gtkextra-3.0.m4 \
	makecopyright \
	gtkextra-3.0.spec 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA= gtkextra-3.0.pc

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

ChangeLog.cvs:
	cvs log | perl mkchlog > ChangeLog.cvs

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-man --disable-rebuilds
#DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --enable-man --disable-rebuilds

