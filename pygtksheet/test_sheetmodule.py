#!/usr/bin/env python

import pygtk
pygtk.require("2.0")
import gtk
import sheet

def _selection_toggled_cb(button, sheet):
    print "The sheet has %d rows and %d columns" % (sheet.props.n_rows, 
                                                    sheet.props.n_columns)
    if button.props.active:
        button.props.label = "Selection single"
        sheet.props.selection_mode = gtk.SELECTION_SINGLE
    else:
        button.props.label = "Selection browse"
        sheet.props.selection_mode = gtk.SELECTION_BROWSE

def _increase_xpadding_cb(button, sheet):
    padding = sheet.child_get_property(button, "x-padding")
    sheet.child_set_property(button, "x-padding", padding+5)

def _change_yoptions_cb(button, sheet):  
    yoptions = sheet.child_get_property(button, "y-options")
    if yoptions & gtk.EXPAND:
        yoptions = gtk.SHRINK
        button.set_label("Shrink")
    else:
        yoptions = gtk.EXPAND
        button.set_label("Expand")
    sheet.child_set_property(button, "y-options", yoptions)
            

def run_test():
    s = sheet.Sheet(20,20,"Test PyGtkSheet")

    b = gtk.ToggleButton("Selection browse")
    b.connect("toggled", _selection_toggled_cb, s)
    s.attach_default(b, 1, 1)

    b2 = gtk.Button("Increase x-padding")
    b2.connect("clicked", _increase_xpadding_cb, s)
    s.attach_default(b2, 2, 1)

    b3 = gtk.Button("Expand")
    b3.connect("clicked", _change_yoptions_cb, s)   
    s.attach(b3, 3, 1, gtk.EXPAND|gtk.FILL, gtk.EXPAND, 0, 10)
    s.set_row_height(3, 60)

    ws = gtk.ScrolledWindow()
    ws.add(s)

    w = gtk.Window()
    w.set_title("Test PyGtkSheet")
    w.set_size_request(400,400)
    w.add(ws)    
    w.show_all()
    w.connect("delete_event", lambda x,y: gtk.main_quit())

    gtk.main()
    

if __name__=="__main__":
    run_test()

