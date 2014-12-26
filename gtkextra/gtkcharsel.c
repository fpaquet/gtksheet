/* gtkcharselection - character selection dialog for gtk+
 * Copyright 1999-2001  Adrian E. Feiguin <feiguin@ifir.edu.ar>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION: gtkcharsel
 * @short_description: Character selection dialog
 *
 * Char selection is a widget which contains all the characters in a font.
 * Connecting a callback to the ok button of the widget you get the selected char.
 */

/**
 * GtkCharSelection:
 *
 * The GtkCharSelection struct contains only private data.
 * It should only be accessed through the functions described below.
 */


#include <gtk/gtk.h>
#include <glib.h>

#include <string.h>
#include "gtkextra-compat.h"
#include "gtkcharsel.h"

static void gtk_char_selection_realize 		   (GtkWidget *widget);
static void gtk_char_selection_map 		   (GtkWidget *widget);
static void new_font				   (GtkFontCombo *font_combo, 
						    gpointer data);
static void new_selection			   (GtkButton *button, 
                                                    gpointer data);

G_DEFINE_TYPE(GtkCharSelection, gtk_char_selection, GTK_TYPE_DIALOG);


GtkWidget*
gtk_char_selection_new (void)
{
  return GTK_WIDGET(g_object_new (gtk_char_selection_get_type (), NULL));
}

static void
gtk_char_selection_class_init (GtkCharSelectionClass *klass)
{
  GtkWidgetClass *widget_class;
  
  widget_class = (GtkWidgetClass*) klass;

  widget_class->realize = gtk_char_selection_realize;
  widget_class->map = gtk_char_selection_map;
}

static void
gtk_char_selection_init (GtkCharSelection *charsel)
{
  GtkWidget *main_vbox;
  GtkWidget *frame;
  GtkWidget *separator;
  gint i;

  charsel->selection = -1;

  gtk_window_set_resizable(GTK_WINDOW(charsel), FALSE);
  gtk_window_set_title(GTK_WINDOW(charsel), "Select Character");
  gtk_container_set_border_width (GTK_CONTAINER (charsel), 10);

  main_vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 0);
  gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(charsel))), main_vbox);
  gtk_widget_show(main_vbox);

  charsel->font_combo = GTK_FONT_COMBO(gtk_font_combo_new());
  gtk_box_pack_start(GTK_BOX(main_vbox), GTK_WIDGET(charsel->font_combo), FALSE, FALSE, 0);
  gtk_widget_show(GTK_WIDGET(charsel->font_combo));

  frame = gtk_frame_new(NULL);
  gtk_widget_set_hexpand(frame, TRUE);
  gtk_widget_set_vexpand(frame, TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
  gtk_box_pack_start(GTK_BOX(main_vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show(frame);


  GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  charsel->grid= GTK_GRID(gtk_grid_new());
  gtk_container_add(GTK_CONTAINER(sw), GTK_WIDGET(charsel->grid));
  gtk_container_add(GTK_CONTAINER(frame), sw);
  gtk_widget_show(GTK_WIDGET(charsel->grid));
  gtk_widget_show(sw);

  for(i = 0; i < 256; i++){
    gint x, y;
    y = i / 32;
    x = i % 32;

    gchar *s2;
    gunichar s[2];
    s[0] = i;
    s[1] = '\0';

    s2 = g_ucs4_to_utf8(s,1,NULL,NULL,NULL);
    
    charsel->button[i] = GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label(s2));
    g_free(s2);
    gtk_container_set_border_width(GTK_CONTAINER(charsel->button[i]), 0);
    gtk_grid_attach(charsel->grid, 
                              GTK_WIDGET(charsel->button[i]),
                              x, y, 1, 1);
/*
    gtk_button_set_relief(GTK_BUTTON(charsel->button[i]), GTK_RELIEF_NONE);
*/
    gtk_widget_set_size_request(GTK_WIDGET(charsel->button[i]), 18, 18);

    gtk_widget_show(GTK_WIDGET(charsel->button[i]));

    g_signal_connect(G_OBJECT(charsel->button[i]), "clicked",
                       G_CALLBACK(new_selection),
                       charsel);
  }


  /* Action Area */

  separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start(GTK_BOX(main_vbox), separator, FALSE, FALSE, 3);
  gtk_widget_show(separator);

  gtk_dialog_add_button(GTK_DIALOG(charsel), "_Ok", GTK_RESPONSE_ACCEPT);
  gtk_dialog_add_button(GTK_DIALOG(charsel), "_Cancel", GTK_RESPONSE_REJECT);

  /* Signals */

  g_signal_connect(G_OBJECT(charsel->font_combo), "changed",
                     G_CALLBACK(new_font), charsel);

  new_font(charsel->font_combo, charsel); 
  gtk_window_set_resizable(GTK_WINDOW(charsel), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(charsel), 500, 400);
  //gtk_window_set_position(GTK_WINDOW(charsel), GTK_WIN_POS_CENTER);
}

static void
gtk_char_selection_realize (GtkWidget *widget)
{
  GtkCharSelection *charsel;

  charsel = GTK_CHAR_SELECTION(widget);

  GTK_WIDGET_CLASS(gtk_char_selection_parent_class)->realize(widget);
}

static void
gtk_char_selection_map (GtkWidget *widget)
{
  GtkCharSelection *charsel;

  charsel = GTK_CHAR_SELECTION(widget);

  GTK_WIDGET_CLASS(gtk_char_selection_parent_class)->map(widget);

  new_font(charsel->font_combo, charsel); 
}


static void
new_font(GtkFontCombo *font_combo, gpointer data)
{
  GtkCharSelection *charsel;
  PangoFontDescription *font;
  gint i;
 
  charsel = GTK_CHAR_SELECTION(data);
 

  font = gtk_font_combo_get_font_description(font_combo);

  for(i = 0; i < 256; i++) {
    //let's try it simple
    gtk_widget_override_font(GTK_WIDGET(charsel->button[i]), font);

    if(charsel->selection == i)
      gtk_toggle_button_set_active(charsel->button[i], TRUE);
    else
      gtk_toggle_button_set_active(charsel->button[i], FALSE);
  }

  pango_font_description_free(font);
}

static void 
new_selection(GtkButton *button, gpointer data)
{
  GtkCharSelection *charsel;
  gint i;
  gint new_selection = -1;

  charsel = GTK_CHAR_SELECTION(data);

  for(i = 0; i < 256; i++){
    if(button == GTK_BUTTON(charsel->button[i])){
          new_selection = i;
          break;
    }
  }

  if(new_selection == charsel->selection){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(charsel->button[new_selection]), TRUE);

      return;
  }

  if(new_selection != -1){
    gtk_char_selection_set_selection(charsel, new_selection);
  } 
}

/**
 * gtk_char_selection_get_selection:
 * @charsel: Char Selection widget.
 *
 * Gets the current selection
 *  
 * Returns: The current selection(a character from the list).0 
 * is left,upper corner;256 is right, down corner. 
 */
gint
gtk_char_selection_get_selection(GtkCharSelection *charsel)
{
  return (charsel->selection);
}

/**
 * gtk_char_selection_set_selection:
 * @charsel: Char Selection widget.
 * @selection: a character index from the list. 0 is left,upper
 *           corner;256 is right, down corner.
 *
 * Sets the selection for the #GtkCharSelection widget.
 */
void
gtk_char_selection_set_selection(GtkCharSelection *charsel, gint selection)
{
  if(selection >= 256) return;

  if(charsel->selection >= 0){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(charsel->button[charsel->selection]), FALSE);
      if(gtk_widget_get_mapped(GTK_WIDGET(charsel)))
         gtk_widget_queue_draw(GTK_WIDGET(charsel->button[charsel->selection]));
  }

  charsel->selection = selection;

  if(charsel->selection >= 0){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(charsel->button[selection]), TRUE);

      if(gtk_widget_get_mapped(GTK_WIDGET(charsel)))
         gtk_widget_queue_draw(GTK_WIDGET(charsel->button[selection]));
  }
}
