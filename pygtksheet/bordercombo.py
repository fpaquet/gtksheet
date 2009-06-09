import gtk
from gtk import gdk
import gobject

xpm_border = [\
"15 15 2 1",
"      c None",
"X     c #000000000000",
"               ",
" X X X X X X X ",
"               ",
" X     X     X ",
"               ",
" X     X     X ",
"               ", 
" X X X X X X X ",
"               ",
" X     X     X ",
"               ",
" X     X     X ",
"               ",
" X X X X X X X ",
"               "]

full   =" XXXXXXXXXXXXX "
dotted =" X X X X X X X "
side111=" X     X     X "
side000="               "
side101=" X           X "
side010="       X       "
side100=" X             "
side001="             X "

class BorderCombo(gtk.ComboBox):
    __gtype_name__= "BorderCombo"
    def __init__(self):
        gtk.ComboBox.__init__(self)

        self._nrows = 3
        self._ncols = 4
        self._row = -1
        self._column = -1               

        self._create_model()
        self.set_model(self._model)
        cell = gtk.CellRendererPixbuf()        
        self.pack_start(cell, True)
        self.add_attribute(cell, "pixbuf", 0)
        self.set_wrap_width(self._ncols)
        self.set_active(0)

    def _create_model(self):
        model = gtk.ListStore(gdk.Pixbuf)

        # EMPTY
        border = list(xpm_border)
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        #TOP
        border[4] = full
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        #BOTTOM
        border[4] = dotted
        border[16] = full
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        #RIGHT
        border[16]=dotted;
        for i in xrange(5, 16, 2):
            border[i] = side001
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # LEFT
        for i in xrange(5, 16, 2):
            border[i] = side100
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # V101
        for i in xrange(5, 16, 2):
            border[i] = side101
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # H101
        for i in xrange(5, 16, 2):
            border[i] = side000
        border[4] = full
        border[16] = full
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # V111
        border[4] = dotted
        border[16] = dotted
        for i in xrange(5, 16, 2):
            border[i] = side111
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # H111
        for i in xrange(5, 16, 2):
            border[i] = side000;
        border[4] = full
        border[16] = full
        border[10] = full
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # CROSS
        border[4] = dotted
        border[16] = dotted
        for i in xrange(5, 16, 2):
            border[i] = side010
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # SIDES
        for i in xrange(5, 16, 2):
            border[i] = side101
        border[4] = full
        border[16] = full
        border[10] = dotted
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        # FULL
        for i in xrange(5, 16, 2):
            border[i] = side111
        border[4]=full
        border[10]=full
        border[16]=full
        model.append((gdk.pixbuf_new_from_xpm_data(border),))

        self._model = model

if __name__=="__main__":
    w = gtk.Window()
    w.show()        
    w.add(BorderCombo())
    
    w.show_all()
    w.connect("delete_event", lambda win,event: gtk.main_quit())
    gtk.main()

