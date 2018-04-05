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
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>
#include "gtkfontcombo.h"
#include "gtkextra-marshal.h"

/* Signals */
enum {
      CHANGED,
      LAST_SIGNAL 
};

/* XPM */
static char * bold_xpm[] = {
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
static char * italic_xpm[] = {
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
static void         gtk_font_combo_destroy         (GtkObject     *font_combo);
static void         gtk_font_combo_finalize        (GObject     *font_combo);
static void         new_font			   (GtkWidget *widget, 
                                                    gpointer data);

static GtkToolbarClass *parent_class = NULL;
static guint font_combo_signals[LAST_SIGNAL] = {0};

static void
gtk_font_combo_class_init (GtkFontComboClass * klass)
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  parent_class = g_type_class_ref (gtk_toolbar_get_type ());
  object_class = (GtkObjectClass *) klass;
  gobject_class = (GObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  object_class->destroy = gtk_font_combo_destroy;
  gobject_class->finalize = gtk_font_combo_finalize;
  
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

static void
gtk_font_combo_destroy (GtkObject * font_combo)
{
  if (GTK_OBJECT_CLASS (parent_class)->destroy)

    (*GTK_OBJECT_CLASS (parent_class)->destroy) (font_combo);
}

static void
gtk_font_combo_finalize (GObject * font_combo)
{
  gtk_psfont_unref();
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (font_combo);
}

static void
gtk_font_combo_init (GtkFontCombo * font_combo)
{
  GtkWidget *widget;
  GtkToolbar *toolbar;
  GdkColormap *colormap;
  GdkPixmap *pixmap;
  GtkWidget *tpixmap;
  GtkWidget *label;
  GtkWidget *space;
  GdkBitmap *mask;
  GtkRequisition req;
  GList *family = NULL;
  gint numf, i;

  gtk_psfont_init();

  widget=GTK_WIDGET(font_combo);

  toolbar = GTK_TOOLBAR(font_combo);
  gtk_container_set_border_width(GTK_CONTAINER(toolbar), 0);

  colormap = gdk_colormap_get_system();
  label = gtk_label_new("Font:   ");
  font_combo->name_combo = gtk_combo_box_new_text(); 
  font_combo->size_combo = gtk_combo_box_new_text();
  space = gtk_label_new("  ");

  gtk_container_add(GTK_CONTAINER(toolbar), label);
  gtk_container_add(GTK_CONTAINER(toolbar), font_combo->name_combo);
  gtk_container_add(GTK_CONTAINER(toolbar), font_combo->size_combo);
  gtk_container_add(GTK_CONTAINER(toolbar), space);
  gtk_widget_show(label);
  gtk_widget_show(space);
  font_combo->bold_button =  GTK_WIDGET(gtk_toggle_button_new());
  font_combo->italic_button =  GTK_WIDGET(gtk_toggle_button_new());
  gtk_container_add(GTK_CONTAINER(toolbar), font_combo->bold_button);
  gtk_container_add(GTK_CONTAINER(toolbar), font_combo->italic_button);
  gtk_widget_set_size_request(font_combo->bold_button, 24, 24);
  gtk_widget_set_size_request(font_combo->italic_button, 24, 24);

  pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask, NULL,
                                                 bold_xpm);
  tpixmap = gtk_image_new_from_pixmap(pixmap, mask);
  gtk_container_add(GTK_CONTAINER(font_combo->bold_button), tpixmap);
  gtk_widget_show(tpixmap);

  pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask, NULL,
                                                 italic_xpm);
  tpixmap = gtk_image_new_from_pixmap(pixmap, mask);
  gtk_container_add(GTK_CONTAINER(font_combo->italic_button), tpixmap);
  gtk_widget_show(tpixmap);

  gtk_widget_size_request(font_combo->size_combo, &req);
  req.width = 56;
  gtk_widget_set_size_request(font_combo->size_combo, req.width, req.height);

/* FIXME */
//  gtk_toolbar_set_space_size(toolbar, 20);
  for(i = 0; i < NUM_SIZES; i++)
    gtk_combo_box_append_text(GTK_COMBO_BOX(font_combo->size_combo), 
				default_sizes[i]);

   gtk_psfont_get_families(&family, &numf);
  GList *list;
  list = family;
  while(list)
  {
      gtk_combo_box_append_text(GTK_COMBO_BOX(font_combo->name_combo), 
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
	GTK_OBJECT(GTK_COMBO_BOX(font_combo->name_combo)),
                     "changed",
                     (void *)new_font, font_combo);

  g_signal_connect(
	GTK_OBJECT(GTK_FONT_COMBO(font_combo)->size_combo),
                     	"changed",
                     (void *)new_font, font_combo);

  g_signal_connect(
	GTK_OBJECT(GTK_FONT_COMBO(font_combo)->italic_button),
                     "clicked",
                     (void *)new_font, font_combo);

  g_signal_connect(
	GTK_OBJECT(GTK_FONT_COMBO(font_combo)->bold_button),
                     "clicked",
                     (void *)new_font, font_combo);

}

GType
gtk_font_combo_get_type ()
{
  static GType font_combo_type = 0;

  if (!font_combo_type)
    {
      font_combo_type = g_type_register_static_simple (
		gtk_toolbar_get_type (),
		"GtkFontCombo",
		sizeof (GtkFontComboClass),
		(GClassInitFunc) gtk_font_combo_class_init,
		sizeof (GtkFontCombo),
		(GInstanceInitFunc) gtk_font_combo_init,
		0);
    }
  return font_combo_type;
}

GtkWidget *
gtk_font_combo_new ()
{
  GtkFontCombo *font_combo;

  font_combo = g_object_new (gtk_font_combo_get_type (), NULL);

  return(GTK_WIDGET(font_combo));
}

static void
new_font(GtkWidget *widget, gpointer data)
{
  GtkFontCombo *font_combo;
  gchar *text;

  font_combo = GTK_FONT_COMBO(data);

  text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(font_combo->name_combo));

  if(!text || strlen(text) == 0) return;
  g_free(text);

  text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(font_combo->size_combo));

  if(!text || strlen(text) == 0) return;
  g_free(text);

  g_signal_emit(GTK_OBJECT(font_combo), font_combo_signals[CHANGED], 0);
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
  GtkItem *item;
  GList *children;
  gchar *text;
  gint n = 0;

  children = GTK_LIST(GTK_COMBO_BOX(combo->name_combo))->children;

  while(children){
    item = GTK_ITEM(children->data);
    text = GTK_LABEL(gtk_bin_get_child(GTK_BIN(item)))->label;
    if(strcmp(text, family) == 0) break;
    n++;
    children = children->next;
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

  gtk_list_select_item(GTK_LIST(GTK_COMBO_BOX(combo->name_combo)), n);

  for(i = 0; i < NUM_SIZES; i++){
     if(atoi(default_sizes[i]) >= height) break;
  }

  if(i < NUM_SIZES)
    gtk_list_select_item(GTK_LIST(GTK_COMBO_BOX(combo->size_combo)), i);

  if(GTK_IS_TOGGLE_BUTTON(combo->bold_button))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(combo->bold_button), bold);
  if(GTK_IS_TOGGLE_BUTTON(combo->italic_button))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(combo->italic_button), italic);
}

gint 
gtk_font_combo_get_font_height (GtkFontCombo *combo)
{
  gchar *size;
  int isize;

  size = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo->size_combo));

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

  if(GTK_IS_TOGGLE_BUTTON(GTK_FONT_COMBO(font_combo)->italic_button))
    italic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_FONT_COMBO(font_combo)->italic_button));
  if(GTK_IS_TOGGLE_BUTTON(GTK_FONT_COMBO(font_combo)->bold_button))
    bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_FONT_COMBO(font_combo)->bold_button));

  return (gtk_psfont_get_by_family(text, italic, bold));
}

PangoFontDescription * 
gtk_font_combo_get_font_description (GtkFontCombo *combo)
{
  gchar *text;
  GtkPSFont *psfont;
  gboolean italic, bold;
  gint height;

  text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo->name_combo));
  if (text == NULL)
	return NULL;

  italic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_FONT_COMBO(combo)->italic_button));
  bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_FONT_COMBO(combo)->bold_button));
  height = gtk_font_combo_get_font_height(combo);

  psfont = gtk_psfont_get_by_family(text, italic, bold);
  g_free(text);
  return (gtk_psfont_get_font_description(psfont, height));
}

GdkFont * 
gtk_font_combo_get_gdkfont (GtkFontCombo *combo)
{
  const gchar *text;
  GtkPSFont *psfont;
  gboolean italic, bold;
  gint height;

  text=gtk_entry_get_text(GTK_ENTRY(GTK_COMBO_BOX(combo->name_combo)));

  italic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_FONT_COMBO(combo)->italic_button));
  bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(GTK_FONT_COMBO(combo)->bold_button));
  height = gtk_font_combo_get_font_height(combo);

  psfont = gtk_psfont_get_by_family(text, italic, bold);
  return (gtk_psfont_get_gdkfont(psfont, height));
}
