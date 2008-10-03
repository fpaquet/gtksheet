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
 GtkStyle *style, *previous_style;
 const gchar *text;

 preview_entry = GTK_WIDGET(data);

 previous_style = preview_entry->style;
 style = gtk_style_copy (previous_style);

 pango_font_description_free(style->font_desc);
 style->font_desc = gtk_font_combo_get_font_description(font_combo);

 gtk_widget_set_style(GTK_WIDGET(preview_entry), style);
 gtk_style_unref(style);

 printf("NEW FONT %s\n",pango_font_description_to_string(style->font_desc));

 text = gtk_entry_get_text(GTK_ENTRY(preview_entry));
 if (strlen(text) == 0)
   gtk_entry_set_text(GTK_ENTRY(preview_entry), PREVIEW_TEXT);
 gtk_entry_set_position(GTK_ENTRY(preview_entry), 0);
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
 gtk_container_border_width(GTK_CONTAINER(window1),0);

 gtk_signal_connect (GTK_OBJECT (window1), "destroy",
		     GTK_SIGNAL_FUNC (quit), NULL);

 vbox1=gtk_vbox_new(FALSE,0);
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

 gtk_signal_connect(GTK_OBJECT(font_combo),
                    "changed",
                    GTK_SIGNAL_FUNC(new_font), preview_entry);

 gtk_main();

 return(0);
}


