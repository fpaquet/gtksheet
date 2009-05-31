#!/usr/bin/env python

import pygtk
pygtk.require("2.0")
import gtk
import sheet


def run_test():
    s = sheet.Sheet(20,20,"Test PyGtkSheet")

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

