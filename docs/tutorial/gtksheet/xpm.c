#include <gtk/gtk.h>
#include <gtkextra/gtksheet.h>
#include "pixmaps.h"


int main( int argc,
          char *argv[] )
{
	GtkWidget *window,*scrolled_window;
        GtkWidget *window_box;
	GtkWidget *sheet;  
	GtkWidget *bullet;
	GdkPixmap *pixmap;
	GdkColormap *colormap;
	GdkBitmap *mask;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        gtk_signal_connect (GTK_OBJECT (window), "destroy", 
                      GTK_SIGNAL_FUNC (gtk_main_quit), 
                      "WM destroy");
      	gtk_widget_set_usize (window,800, 600); 
	 
	window_box = gtk_vbox_new (FALSE, 1);

	scrolled_window=gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add (GTK_CONTAINER (window), window_box);
        gtk_box_pack_start (GTK_BOX (window_box), scrolled_window, 1,1,1);
 
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                             GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);         

	sheet=gtk_sheet_new(10,5,"Edit table");
	GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_AUTORESIZE);
	gtk_container_add(GTK_CONTAINER(scrolled_window), sheet);

	colormap = gdk_colormap_get_system();
	pixmap=gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask, NULL,
			                                   bullet_xpm);

	bullet = gtk_pixmap_new(pixmap, mask);
	gtk_sheet_attach(GTK_SHEET(sheet), bullet, 1, 1, .5, .5);	

	gtk_widget_show_all (bullet);
	gtk_widget_show_all (window);


        gtk_main ();
  
}

