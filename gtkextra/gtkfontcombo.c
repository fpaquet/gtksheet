/* gtkfontcombo - font_combo widget for gtk+
 * Copyright 1999-2001 Adrian E. Feiguin <feiguin@ifir.edu.ar>
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
 * SECTION: gtkfontcombo
 * @short_description: A font combo widget for GTK.
 *
 * It is a GtkToolBar subclass with two combos to select among the 35 standard Adobe PostScript fonts with different sizes. 
 * It has also two buttons to select bold and italics.
 * When you select a new font, it returns the name of the corresponding Postscript font and the equivalent Xfont. 
 * This is what we all expect to have on the top of a GUI for a spreadsheet or word processor.
 */

/**
 * GtkFontCombo:
 *
 * The GtkFontCombo struct contains only private data.
 * It should only be accessed through the functions described below.
 */

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>
#include "gtkfontcombo.h"
#include "gtkextra-compat.h"
#include "gtkextra-marshal.h"




/* Signals */
enum {
      CHANGED,
      LAST_SIGNAL 
};

/* XPM */
static const char * bold_xpm[] = {
"16 16 2 1",
" 	c None",
".	c #000000000000",
"                ",
"  .........     ",
"   ...   ...    ",
"   ...    ...   ",
"   ...    ...   ",
"   ...    ...   ",
"   ...   ...    ",
"   ........     ",
"   ...    ...   ",
"   ...     ...  ",
"   ...     ...  ",
"   ...     ...  ",
"   ...     ...  ",
"   ...    ...   ",
"  .........     ",
"                "};


/* XPM */
static const char * italic_xpm[] = {
"16 16 2 1",
" 	c None",
".	c #000000000000",
"                ",
"        .....   ",
"         ...    ",
"         ...    ",
"        ...     ",
"        ...     ",
"       ...      ",
"       ...      ",
"      ...       ",
"      ...       ",
"     ...        ",
"     ...        ",
"    ...         ",
"    ...         ",
"   .....        ",
"                "};

#define NUM_SIZES 20

static gchar *default_sizes[] = {"8","9","10","12","13","14","16","18",
                                 "20","22","24","26","28","32","36","40",
                                 "48","56","64","72"};

static void         gtk_font_combo_class_init      (GtkFontComboClass *klass);
static void         gtk_font_combo_init            (GtkFontCombo      *font_combo);
static void         new_font			   (GtkWidget *widget, 
                                                    gpointer data);

G_DEFINE_TYPE(GtkFontCombo, gtk_font_combo, GTK_TYPE_TOOLBAR);

static guint font_combo_signals[LAST_SIGNAL] = {0};

static void
gtk_font_combo_class_init (GtkFontComboClass * klass)
{
  GObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  /**
   * GtkFontCombo::changed:
   * @bfont_combo: the #GtkFontCombo object that received the signal.
   *
   * Emmited whenever a different font is choosed.
   */ 
  font_combo_signals[CHANGED] =
    g_signal_new ("changed",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkFontComboClass, changed),
		    NULL, NULL,
                    gtkextra_VOID__VOID,
                    G_TYPE_NONE, 0);

}

/*static void
gtk_font_combo_destroy (GtkWidget* font_combo)
{
  if (GTK_WIDGET_CLASS (parent_class)->destroy)

    (*GTK_WIDGET_CLASS (parent_class)->destroy) (font_combo);
}

static void
gtk_font_combo_finalize (GObject * font_combo)
{
  gtk_psfont_unref();
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (font_combo);
}
*/
static void
gtk_font_combo_init (GtkFontCombo * font_combo)
{
  GtkWidget *widget;
  GtkToolbar *toolbar;
  GdkPixbuf *pixmap;
  GtkWidget *tpixmap;
  GtkWidget *label;
  GtkWidget *space;
  GtkRequisition req;
  GList *family = NULL;
  gint numf, i;
  GtkToolItem *toolitem;

  gtk_psfont_init();

  widget=GTK_WIDGET(font_combo);

  toolbar = GTK_TOOLBAR(font_combo);
  gtk_container_set_border_width(GTK_CONTAINER(toolbar), 0);

  label = gtk_label_new("Font:   ");
  font_combo->name_combo = gtk_combo_box_text_new(); 
  font_combo->size_combo = gtk_combo_box_text_new();
  space = gtk_label_new("  ");

  toolitem = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(toolitem), label);
  gtk_toolbar_insert(toolbar, toolitem, 0);
  gtk_widget_show_all(GTK_WIDGET(toolitem));

  toolitem = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(toolitem), font_combo->name_combo);
  gtk_toolbar_insert(toolbar, toolitem, 1);
  gtk_widget_show_all(GTK_WIDGET(toolitem));

  toolitem = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(toolitem), font_combo->size_combo);
  gtk_toolbar_insert(toolbar, toolitem, 2);
  gtk_widget_show_all(GTK_WIDGET(toolitem));

  toolitem = gtk_tool_item_new();
  gtk_container_add(GTK_CONTAINER(toolitem), space);
  gtk_toolbar_insert(toolbar, toolitem, 3);
  gtk_widget_show_all(GTK_WIDGET(toolitem));

  font_combo->bold_button =  GTK_WIDGET(gtk_toggle_tool_button_new());
  font_combo->italic_button =  GTK_WIDGET(gtk_toggle_tool_button_new());
  gtk_toolbar_insert(toolbar, GTK_TOOL_ITEM(font_combo->bold_button), 4);
  gtk_toolbar_insert(toolbar, GTK_TOOL_ITEM(font_combo->italic_button), 5);
  gtk_widget_set_size_request(font_combo->bold_button, 24, 24);
  gtk_widget_set_size_request(font_combo->italic_button, 24, 24);

  pixmap = gdk_pixbuf_new_from_xpm_data(bold_xpm);
  tpixmap = gtk_image_new_from_pixbuf(pixmap);
  gtk_container_add(GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(font_combo->bold_button))), tpixmap);
  gtk_widget_show(tpixmap);

  pixmap = gdk_pixbuf_new_from_xpm_data(italic_xpm);
  tpixmap = gtk_image_new_from_pixbuf(pixmap);
  gtk_container_add(GTK_CONTAINER(gtk_bin_get_child(GTK_BIN(font_combo->italic_button))), tpixmap);
  gtk_widget_show(tpixmap);

  gtk_widget_get_preferred_size(font_combo->size_combo, &req, NULL);
  req.width = 56;
  gtk_widget_set_size_request(font_combo->size_combo, req.width, req.height);

/* FIXME */
//  gtk_toolbar_set_space_size(toolbar, 20);
  for(i = 0; i < NUM_SIZES; i++)
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(font_combo->size_combo),
				default_sizes[i]);

   gtk_psfont_get_families(&family, &numf);
  GList *list;
  list = family;
  while(list)
  {
      gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(font_combo->name_combo), 
		(gchar *)list->data);
      list = list->next;
  }
  gtk_combo_box_set_active (GTK_COMBO_BOX(font_combo->name_combo), 0);
  gtk_combo_box_set_active (GTK_COMBO_BOX(font_combo->size_combo), 3);
  gtk_widget_show (font_combo->size_combo);
  gtk_widget_show (font_combo->bold_button);
  gtk_widget_show (font_combo->italic_button);

  gtk_widget_show (font_combo->name_combo);

  g_signal_connect(
	G_OBJECT(GTK_FONT_COMBO(font_combo)->name_combo),
                     "changed",
                     G_CALLBACK(new_font), font_combo);

  g_signal_connect(
	G_OBJECT(GTK_FONT_COMBO(font_combo)->size_combo),
                     	"changed",
                     G_CALLBACK(new_font), font_combo);

  g_signal_connect(
	G_OBJECT(GTK_FONT_COMBO(font_combo)->italic_button),
                     "toggled",
                     G_CALLBACK(new_font), font_combo);

  g_signal_connect(
	G_OBJECT(GTK_FONT_COMBO(font_combo)->bold_button),
                     "toggled",
                     G_CALLBACK(new_font), font_combo);
}

GtkWidget *
gtk_font_combo_new ()
{
  return GTK_WIDGET(g_object_new (gtk_font_combo_get_type (), NULL));
}

static void
new_font(GtkWidget *widget, gpointer data)
{
  GtkFontCombo *font_combo;
  gchar *text;

  font_combo = GTK_FONT_COMBO(data);

  text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(font_combo->name_combo));

  if(!text || strlen(text) == 0) return;
  g_free(text);

  text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(font_combo->size_combo));

  if(!text || strlen(text) == 0) return;
  g_free(text);

  g_signal_emit(G_OBJECT(font_combo), font_combo_signals[CHANGED], 0);
  //unfocus
  GtkWidget *parent_widget = widget;
  while (parent_widget && !GTK_IS_WINDOW (parent_widget)) {
	parent_widget = gtk_widget_get_parent (parent_widget);
  }
  if (parent_widget) {
	gtk_window_set_focus (GTK_WINDOW (parent_widget), NULL);
  }
}

/**
 * gtk_font_combo_select: 
 * @font_combo: a #GtkFontCombo 
 * @family: font family
 * @bold: TRUE or FALSE
 * @italic: TRUE or FALSE
 * @height: height of the font
 * 
 * Select from the combo a font which satisfies the arguments.
 */
void
gtk_font_combo_select (GtkFontCombo *combo, 
		       const gchar *family,
                       gboolean bold,
		       gboolean italic,
		       gint height)
{
  gchar *text;
  gint n = 0;
  GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo->name_combo));
  gboolean valid;
  GtkTreeIter iter;
  gint column;

  if (gtk_tree_model_get_column_type(model, 0) != G_TYPE_STRING) {
  	fprintf(stderr, "column 0 is not of type string\n");
	exit(1);
  }

  valid =  gtk_tree_model_get_iter_first(model, &iter);

  /* I would be very surprised if column would ever be different from zero but let's check it anyways */
  column = gtk_combo_box_get_entry_text_column(GTK_COMBO_BOX(combo->name_combo));

  while (valid) {
  	gtk_tree_model_get(model, &iter, column, &text, -1);
	if (strcmp(family, text) == 0) {
		/* match */
		g_free(text);
		break;
	}
	g_free(text);
	valid = gtk_tree_model_iter_next(model, &iter);
	n++;
  }

  if (!valid) {
  	g_warning("gtk_font_combo_select: font family %s not found in model", family);
	return;
  }

  gtk_font_combo_select_nth(combo, n, bold, italic, height);
}

/**
 * gtk_font_combo_select_nth:
 * @font_combo: a #GtkFontCombo 
 * @n: nth font
 * @bold: TRUE or FALSE
 * @italic: TRUE or FALSE
 * @height: height of the font
 * 
 * Select from the combo the nth font which satisfies the arguments.
 */
void
gtk_font_combo_select_nth (GtkFontCombo *combo, 
		           gint n,
                           gboolean bold,
		           gboolean italic,
		           gint height)
{
  gint i;

  GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo->name_combo));
  if (n >= gtk_tree_model_iter_n_children(model, NULL)) {
  	g_warning("gtk_font_combo_select: font number %i not found in model", n);
	return;
  }
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo->name_combo), n);

  for(i = 0; i < NUM_SIZES; i++){
     if(atoi(default_sizes[i]) >= height) break;
  }

  if(i < NUM_SIZES)
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo->size_combo), i);

  if(GTK_IS_TOGGLE_TOOL_BUTTON(combo->bold_button))
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(combo->bold_button), bold);
  if(GTK_IS_TOGGLE_BUTTON(combo->italic_button))
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(combo->italic_button), italic);
}

gint 
gtk_font_combo_get_font_height (GtkFontCombo *combo)
{
  gchar *size;
  int isize;

  size = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo->size_combo));

  if (size)
  {
	isize = atoi(size);
	g_free(size);
  	return isize;
  }
  else
  {
	printf("gtk_font_combo_get_font_height no size\n");
	return(10);
  }
}

/**
 * gtk_font_combo_get_psfont:
 * @font_combo:  the #GtkFontCombo
 * 
 * Returns: (transfer none) the #GtkPSFont
 */
GtkPSFont * 
gtk_font_combo_get_psfont (GtkFontCombo *font_combo)
{
  const gchar *text;
  gboolean italic = FALSE, bold = FALSE;

  text=gtk_entry_get_text(GTK_ENTRY(GTK_COMBO_BOX(font_combo->name_combo)));

  if(GTK_IS_TOGGLE_TOOL_BUTTON(GTK_FONT_COMBO(font_combo)->italic_button))
    italic = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(GTK_FONT_COMBO(font_combo)->italic_button));
  if(GTK_IS_TOGGLE_BUTTON(GTK_FONT_COMBO(font_combo)->bold_button))
    bold = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(GTK_FONT_COMBO(font_combo)->bold_button));

  return (gtk_psfont_get_by_family(text, italic, bold));
}

PangoFontDescription * 
gtk_font_combo_get_font_description (GtkFontCombo *combo)
{
  gchar *text;
  GtkPSFont *psfont;
  gboolean italic, bold;
  gint height;

  text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo->name_combo));
  if (text == NULL)
	return NULL;

  italic = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(GTK_FONT_COMBO(combo)->italic_button));
  bold = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(GTK_FONT_COMBO(combo)->bold_button));
  height = gtk_font_combo_get_font_height(combo);

  psfont = gtk_psfont_get_by_family(text, italic, bold);
  g_free(text);
  return (gtk_psfont_get_font_description(psfont, height));
}
