#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <pango/pango.h>
#include "gtkfontcombo.h"

#define PREVIEW_TEXT "ABCDEFGHI abcdefghi 0123456789"

void
quit ()
{
  gtk_main_quit();
}

void
new_font(GtkFontCombo *font_combo, gpointer data)
{
 GtkWidget *preview_entry;
 const gchar *text;
 PangoFontDescription *font_desc;

 preview_entry = GTK_WIDGET(data);

 font_desc = gtk_font_combo_get_font_description(font_combo);

 gtk_widget_override_font(preview_entry, font_desc);

 printf("NEW FONT %s\n",pango_font_description_to_string(font_desc));

 pango_font_description_free(font_desc);

 text = gtk_entry_get_text(GTK_ENTRY(preview_entry));
 if (strlen(text) == 0)
   gtk_entry_set_text(GTK_ENTRY(preview_entry), PREVIEW_TEXT);
 gtk_editable_set_position(GTK_EDITABLE(preview_entry), 0);
}

int main(int argc, char *argv[])
{
 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *font_combo;
 GtkWidget *preview_entry;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkFontCombo Demo");
 gtk_container_set_border_width(GTK_CONTAINER(window1),0);

 g_signal_connect (G_OBJECT (window1), "destroy",
		     G_CALLBACK (quit), NULL);

 vbox1=gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
 gtk_container_add(GTK_CONTAINER(window1),vbox1);
 gtk_widget_show(vbox1);

 font_combo = gtk_font_combo_new();
 gtk_box_pack_start(GTK_BOX(vbox1),font_combo, FALSE, FALSE, 0);
 gtk_widget_show(font_combo);

 preview_entry = gtk_entry_new();
 gtk_box_pack_start(GTK_BOX(vbox1),preview_entry, TRUE, TRUE,0);
 gtk_widget_show(preview_entry);
 gtk_entry_set_text(GTK_ENTRY(preview_entry), PREVIEW_TEXT);
 gtk_widget_show(window1);

 new_font(GTK_FONT_COMBO(font_combo), preview_entry); 
/*********** SIGNALS ************/

 g_signal_connect(G_OBJECT(font_combo),
                    "changed",
                    G_CALLBACK(new_font), preview_entry);

 gtk_main();

 return(0);
}


