
#include <gtk/gtk.h>
#include <glib.h>
#include "gtkcharsel.h"


int main(int argc, char *argv[]) 
{
  GtkWidget *charsel;
  gtk_init(&argc, &argv);

  charsel = gtk_char_selection_new();


  /*g_signal_connect (G_OBJECT (charsel), "destroy",
		      (void *)quit, NULL);
  */
/*
  g_signal_connect (G_OBJECT (GTK_CHAR_SELECTION(charsel)->ok_button), 
                     "clicked",
		      G_CALLBACK(ok_clicked), charsel);
*/
/*
  gtk_char_selection_set_selection(GTK_CHAR_SELECTION(charsel), 25);
*/

  int result = gtk_dialog_run(GTK_DIALOG(charsel));
  //gtk_main();
  
  if (result == GTK_RESPONSE_ACCEPT)
  	printf("selected character: %c\n", gtk_char_selection_get_selection(GTK_CHAR_SELECTION(charsel)));
  else 
  	printf("dialog cancel button clicked or dialog was deleted\n");

  gtk_widget_destroy(charsel);

  return 0;
}


