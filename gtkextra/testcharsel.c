
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib.h>
#include "gtkcharsel.h"

GtkWidget *charsel;


void
quit ()
{
  gtk_main_quit();
}

void
ok_clicked(GtkWidget *widget, gpointer data)
{
  GtkCharSelection *charsel;

  charsel = GTK_CHAR_SELECTION(data);
}

int main(int argc, char *argv[]) 
{
  gtk_init(&argc, &argv);

  charsel=gtk_char_selection_new();


  g_signal_connect (GTK_OBJECT (charsel), "destroy",
		      (void *)quit, NULL);

/*
  g_signal_connect (GTK_OBJECT (GTK_CHAR_SELECTION(charsel)->ok_button), 
                     "clicked",
		      G_CALLBACK(ok_clicked), charsel);
*/
/*
  gtk_char_selection_set_selection(GTK_CHAR_SELECTION(charsel), 25);
*/

  gtk_widget_show(charsel);
  gtk_main();

  return(0);
}


