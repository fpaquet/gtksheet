/* gtkiconfileselection - gtkiconfileselection dialog widget for gtk+
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


#ifndef __GTK_ICON_FILESEL_H__
#define __GTK_ICON_FILESEL_H__


#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "gtkdirtree.h"
#include "gtkiconlist.h"
#include "gtkfilelist.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_ICON_FILESEL                  (gtk_icon_file_selection_get_type ())
#define GTK_ICON_FILESEL(obj)                  (GTK_CHECK_CAST ((obj), GTK_TYPE_ICON_FILESEL, GtkIconFileSel))
#define GTK_ICON_FILESEL_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_ICON_FILESEL, GtkIconFileSelClass))
#define GTK_IS_ICON_FILESEL(obj)               (GTK_CHECK_TYPE ((obj), GTK_TYPE_ICON_FILESEL))
#define GTK_IS_ICON_FILESEL_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_ICON_FILESEL))

typedef struct _GtkIconFileSel       GtkIconFileSel;
typedef struct _GtkIconFileSelClass  GtkIconFileSelClass;



struct _GtkIconFileSel
{
  GtkWindow window;

  gchar *title;

  gboolean show_tree;

  GtkWidget *path_label;
  
  GtkWidget *tree_window;
  GtkWidget *dir_tree;
  GtkWidget *list_window;
  GtkWidget *file_list;
  GtkWidget *history_combo;
  GtkWidget *up_button;
  GtkWidget *refresh_button;
  GtkWidget *home_button;
  guint tree_signal_id;

  GtkWidget *file_entry;
  GtkWidget *filter_entry;

  GtkWidget *ok_button;
  GtkWidget *cancel_button;

  GtkWidget *action_area;     /* It's a GtkTable with the entries */

  gchar *selection;
};

struct _GtkIconFileSelClass
{
  GtkWindowClass parent_class;

};


GtkType    gtk_icon_file_selection_get_type       (void);
GtkWidget* gtk_icon_file_selection_new            (const gchar *title);
void 	   gtk_icon_file_selection_construct      (GtkIconFileSel *filesel,
						   const gchar *title);
void 	   gtk_icon_file_selection_show_tree	  (GtkIconFileSel *filesel, 
						   gboolean show);
gint 	   gtk_icon_file_selection_open_dir       (GtkIconFileSel *filesel, 
                                                   const gchar *path);
void       gtk_icon_file_selection_show_hidden    (GtkIconFileSel *filesel,
						   gboolean visible);
void       gtk_icon_file_selection_set_filter     (GtkIconFileSel *filesel, 
						   const gchar *filter);
const gchar *gtk_icon_file_selection_get_selection(GtkIconFileSel *filesel);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_ICON_FILESEL_H__ */
