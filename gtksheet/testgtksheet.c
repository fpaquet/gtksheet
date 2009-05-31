#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <string.h>
#include "gtksheet.h"


#define DEFAULT_PRECISION 3
#define DEFAULT_SPACE 8
#define NUM_SHEETS 3 

GtkWidget *window;
GtkWidget *main_vbox;
GtkWidget *notebook;
GtkWidget **sheets;
GtkWidget **scrolled_windows;
GtkWidget *show_hide_box;
GtkWidget *status_box;
GtkWidget *location;
GtkWidget *entry;
GtkWidget *fgcolorcombo;
GtkWidget *bgcolorcombo;
GtkWidget *bordercombo;
GdkPixmap *pixmap;
GdkBitmap *mask;
GtkWidget *bg_pixmap;
GtkWidget *fg_pixmap;
GtkWidget *toolbar;
GtkWidget *left_button;
GtkWidget *center_button;
GtkWidget *right_button;
GtkWidget *tpixmap;
GtkWidget *bullet[10];
GtkWidget *smile;
GtkWidget *curve;
GtkWidget *popup;

void
quit ()
{
  gtk_main_quit();
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);
    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GtkSheet Demo");
    gtk_widget_set_usize(GTK_WIDGET(window), 900, 600);

    gtk_signal_connect (GTK_OBJECT (window), "destroy",
	          GTK_SIGNAL_FUNC (quit), NULL);
    main_vbox = gtk_vbox_new(FALSE,1);

    GtkWidget *sheet = gtk_sheet_new_with_custom_entry(1000,26, "Sheet 1", gtk_entry_get_type());
    GtkWidget *scrwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrwin), sheet);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrwin, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    gtk_widget_show_all(  window );

    gtk_main();

}


