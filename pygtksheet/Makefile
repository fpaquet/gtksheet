
DEFS=`pkg-config --variable=defsdir pygtk-2.0`

sheet.c: sheet.defs sheet.override
	pygobject-codegen-2.0 --prefix sheet \
	--register $(DEFS)/gdk-types.defs \
	--register $(DEFS)/gtk-types.defs \
	--override sheet.override \
	sheet.defs > $@

CFLAGS = `pkg-config --cflags gtk+-2.0 pygtk-2.0` -I/usr/include/python2.6/ -I. -I../gtksheet -I..
LDFLAGS = `pkg-config --libs gtk+-2.0 pygtk-2.0`

GTKSHEET_OBJECTS = ../gtksheet/gtksheetwidget.o \
                   ../gtksheet/gtkitementry.o \
                   ../gtksheet/gtksheet-marshal.o \
                   ../gtksheet/gtksheettypebuiltins.o \
                   ../gtksheet/gtksheet.o

sheet.so: sheet.o  $(GTKSHEET_OBJECTS) sheetmodule.o
	$(CC) $(LDFLAGS) -shared $^ -o $@

clean:
	-rm -f *.so *.o sheet.c
    
