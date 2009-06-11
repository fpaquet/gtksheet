import sys
sys.path += ['/usr/local/lib/python2.6/dist-packages/gtk-2.0']
import gtk
from gtk import gdk
import pango
import sheet
from bordercombo import BorderCombo
#from gtkextra import BorderCombo
#import gtkextra

class TestSheet(sheet.Sheet):
    def __init__(self):
        sheet.Sheet.__init__(self, 20, 20, "Test")
        colormap = gdk.colormap_get_system()

        self.default_bg_color = colormap.alloc_color("light yellow")
        self.default_fg_color = colormap.alloc_color("black")

        self.set_background(self.default_bg_color)
        self.set_grid(colormap.alloc_color("light blue"))


        for column in xrange(self.get_columns_count()):
            name = chr(ord("A") + column)
            self.column_button_add_label(column, name)
            self.set_column_title(column, name)

        self.default_font = self.style.font_desc
            


class TestWindow(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)

        status_box = gtk.HBox(spacing=1)
        status_box.set_border_width(0)

        self.location = gtk.Label("")
        (width, height) = self.location.size_request()
        self.location.set_size_request(160, height)
        status_box.pack_start(self.location, False)

        self.entry = gtk.Entry()
        self.entry.connect("changed", self._show_sheet_entry_cb)
        status_box.pack_start(self.entry)
        
        t = gtk.Toolbar()
        ttips = gtk.Tooltips()
        def add_widget_to_toolbar(widget, separator=True, tooltip=None):
            ti = gtk.ToolItem()
            ti.add(widget)
            if tooltip is not None:                
                ti.set_tooltip(ttips, tooltip)
            t.insert(ti, -1)
            if separator:
                t.insert(gtk.SeparatorToolItem(), -1)       
         
        fontbutton = gtk.FontButton()
        fontbutton.connect("font-set", self._font_changed_cb)
        add_widget_to_toolbar(fontbutton,
            tooltip="Change the font of the selected cells");
        self.fontbutton = fontbutton

        items = \
            (("justleft",  None,                  
              "Justify selected cells to the left",
              gtk.STOCK_JUSTIFY_LEFT, self._justification_cb, 
              gtk.JUSTIFY_LEFT), 
             ("justcenter", None,                  
              "Justify selected cells to the center",
              gtk.STOCK_JUSTIFY_CENTER, self._justification_cb, 
              gtk.JUSTIFY_CENTER),
             ("justright", None,                  
              "Justify selected cells to the right",
              gtk.STOCK_JUSTIFY_RIGHT, self._justification_cb, 
              gtk.JUSTIFY_RIGHT))
        for name, label, tooltip, stock_id, cb, cb_params in items:
            ti = gtk.Action(name, label, tooltip, stock_id)
            ti.connect("activate", cb, cb_params)
            t.insert(ti.create_tool_item(), -1)
        
        bordercombo = BorderCombo()
        bordercombo.connect("changed", self._border_changed_cb)
        add_widget_to_toolbar(bordercombo,
            tooltip="Change the border of the selected cells")
        
        colormap = gdk.colormap_get_system()
        colorbtn = gtk.ColorButton(colormap.alloc_color("black"))
        colorbtn.connect("color-set", self._color_changed_cb, "f")
        add_widget_to_toolbar(colorbtn, separator=False,
            tooltip="Change the foreground color of the selected cells")
        self.fgcolorbtn = colorbtn

        colorbtn = gtk.ColorButton(colormap.alloc_color("light yellow"))
        colorbtn.connect("color-set", self._color_changed_cb, "b")
        add_widget_to_toolbar(colorbtn, 
            tooltip="Change the background color of the selected cells");
        self.bgcolorbtn = colorbtn

        self.sheet = TestSheet()
        self.sheet.connect("activate", self._activate_sheet_cell_cb)
        self.sheet.get_entry().connect("changed", self._show_entry_cb)
        self.sheet.connect("changed", self._sheet_changed_cb)
        ws = gtk.ScrolledWindow()
        ws.add(self.sheet)

        fd = self.sheet.default_font
        fontbutton.set_font_name(fd.to_string())

        vbox = gtk.VBox()
        vbox.pack_start(t, False, False, 0)
        vbox.pack_start(status_box, False, False, 0)
        vbox.pack_start(ws, True, True, 0)

        self.add(vbox)
        self.set_size_request(500,400)
        self.show_all()

    def _sheet_changed_cb(self, sheet, row, column):
        print "Sheet change at row: %d, column: %d" % (row, column)

    def _show_sheet_entry_cb(self, entry):
        if not entry.flags() & gtk.HAS_FOCUS:
            return
        sheet_entry = self.sheet.get_entry()
        text = entry.get_text()
        sheet_entry.set_text(text)

    def _show_entry_cb(self, sheet_entry, *args):
        if not sheet_entry.flags() & gtk.HAS_FOCUS:
            return
        text = sheet_entry.get_text()
        self.entry.set_text(text)

    def _activate_sheet_cell_cb(self, sheet, row, column):
        title = sheet.get_column_title(column)
        if title:
            cell = "  %s:%d  " % (title, row)
        else:
            cell = " ROW: %d COLUMN: %d " % (row, column)
        self.location.set_text(cell)

        # Set attributes
        attributes = sheet.get_attributes(row, column)
        if attributes:
            fd = attributes.font_desc if attributes.font_desc else self.sheet.default_font
            fgcolor = attributes.foreground
            bgcolor = attributes.background
        else:
            fd = self.sheet.default_font
            fgcolor = self.sheet.default_fg_color
            bgcolor = self.sheet.default_bg_color

        self.fontbutton.set_font_name(fd.to_string())
        self.fgcolorbtn.set_color(fgcolor)
        self.bgcolorbtn.set_color(bgcolor)

        # Set entry text
        sheet_entry = sheet.get_entry()
        self.entry.props.max_length = sheet_entry.props.max_length
        text = sheet.cell_get_text(row, column)
        if text:
            self.entry.set_text(text)
        else:
            self.entry.set_text("")

        print self.sheet.props.active_cell


    def _font_changed_cb(self, widget):
        r = self.sheet.props.selected_range
        fd = pango.FontDescription(widget.get_font_name())
        self.sheet.range_set_font(r, fd)
        

    def _justification_cb(self, widget, data=None):
        if data is None:
            return
        r = self.sheet.props.selected_range
        if r:
            self.sheet.range_set_justification(r, data)

    def _border_changed_cb(self, widget):
        border = widget.get_active()
        range = self.sheet.props.selected_range
        border_width = 3
        self.sheet.range_set_border(range, 0, 0)
        if border == 1:
            border_mask = sheet.SHEET_TOP_BORDER
            range.rowi = range.row0
            self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 2:
            border_mask = sheet.SHEET_BOTTOM_BORDER
            range.row0 = range.rowi
            self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 3:
            border_mask = sheet.SHEET_RIGHT_BORDER
            range.col0 = range.coli
            self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 4:
            border_mask = sheet.SHEET_LEFT_BORDER
            range.coli = range.col0
            self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 5:
            if range.col0 == range.coli:
                border_mask = sheet.SHEET_LEFT_BORDER | sheet.SHEET_RIGHT_BORDER
                self.sheet.range_set_border(range, border_mask, border_width)
            else:
                border_mask = sheet.SHEET_LEFT_BORDER
                auxcol = range.coli
                range.coli = range.col0
                self.sheet.range_set_border(range, border_mask, border_width)
                border_mask = sheet.SHEET_RIGHT_BORDER
                range.col0 = range.coli = auxcol
                self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 6:
            if range.row0 == range.rowi:
                border_mask = sheet.SHEET_TOP_BORDER | sheet.SHEET_BOTTOM_BORDER
                self.sheet.range_set_border(range, border_mask, border_width)
            else:
                border_mask = sheet.SHEET_TOP_BORDER
                auxrow = range.rowi
                range.rowi = range.row0
                self.sheet.range_set_border(range, border_mask, border_width)
                border_mask = sheet.SHEET_BOTTOM_BORDER
                range.row0 = range.rowi = auxrow
                self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 7:
            border_mask = sheet.SHEET_RIGHT_BORDER | sheet.SHEET_LEFT_BORDER
            self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 8:
            border_mask = sheet.SHEET_BOTTOM_BORDER | sheet.SHEET_TOP_BORDER
            self.sheet.range_set_border(range, border_mask, border_width)
        elif border == 9:
            self.sheet.range_set_border(range, 15, border_width)
            for i in xrange(range.row0, range.rowi + 1):
                for j in xrange(range.col0, range.coli + 1):
                    border_mask = 15
                    auxrange = sheet.SheetRange(i, j, i, j)
                    if i == range.rowi:
                        border_mask = border_mask ^ sheet.SHEET_BOTTOM_BORDER
                    if i == range.row0:
                        border_mask = border_mask ^ sheet.SHEET_TOP_BORDER
                    if j == range.coli:
                        border_mask = border_mask ^ sheet.SHEET_RIGHT_BORDER
                    if j == range.col0:
                        border_mask = border_mask ^ sheet.SHEET_LEFT_BORDER
                    if border_mask != 15:
                        self.sheet.range_set_border(auxrange, border_mask,
                                               border_width)
        elif border == 10:
            for i in xrange(range.row0, range.rowi + 1):
                for j in xrange(range.col0, range.coli + 1):
                    border_mask = 0
                    auxrange = sheet.SheetRange(i, j, i, j)
                    if i == range.rowi:
                        border_mask = border_mask | sheet.SHEET_BOTTOM_BORDER
                    if i == range.row0:
                        border_mask = border_mask | sheet.SHEET_TOP_BORDER
                    if j == range.coli:
                        border_mask = border_mask | sheet.SHEET_RIGHT_BORDER
                    if j == range.col0:
                        border_mask = border_mask | sheet.SHEET_LEFT_BORDER
                    if border_mask != 0:
                        self.sheet.range_set_border(auxrange, border_mask,
                                               border_width)
        elif border == 11:
            border_mask = 15
            self.sheet.range_set_border(range, border_mask, border_width)

    def _color_changed_cb(self, widget, data=None):
        # Bug in GtkSheet?: the color must be allocated with the system's 
        # colormap, else it is ignored
        if data is None:
            return
        color = widget.get_color()
        _range = self.sheet.props.selected_range
        if data == "f":
            self.sheet.range_set_foreground(_range, color)
        else:
            self.sheet.range_set_background(_range, color)


def main():
    w = TestWindow()
    w.connect("delete-event", lambda x,y: gtk.main_quit())
    gtk.main()
        
if __name__=='__main__':
    main()

