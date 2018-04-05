/* gtkiconlist - gtkiconlist widget for gtk+
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

#ifndef __GTK_ICON_LIST_H__
#define __GTK_ICON_LIST_H__


#include <gdk/gdk.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
  GTK_ICON_LIST_ICON,
  GTK_ICON_LIST_TEXT_RIGHT,
  GTK_ICON_LIST_TEXT_BELOW,
} GtkIconListMode;

#define GTK_ICON_LIST(obj)        G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_icon_list_get_type (), GtkIconList)
#define GTK_ICON_LIST_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, gtk_icon_list_get_type, GtkIconListClass)
#define GTK_IS_ICON_LIST(obj)     G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_icon_list_get_type ())
#define G_TYPE_ICON_LIST (gtk_icon_list_get_type ())
#define G_TYPE_ICON_LIST_ITEM (gtk_icon_list_item_get_type ())


typedef struct _GtkIconList	GtkIconList;
typedef struct _GtkIconListClass	GtkIconListClass;
typedef struct _GtkIconListItem	GtkIconListItem;

/**
 * GtkIconListItem:
 *
 * The GtkIconListItem structure contains only private data.
 * It should only be accessed through the functions described below.
 */

struct _GtkIconListItem
{
  /*< private >*/
  guint x, y;

  gint state;

  gchar *entry_label;
  gchar *label;
 
  GtkWidget *pixmap;
  GtkWidget *entry;

  gpointer link;
};

/**
 * GtkIconList:
 *
 * The GtkIconList structure contains only private data.
 * It should only be accessed through the functions described below.
 */
struct _GtkIconList
{
  GtkFixed fixed;

  GtkIconListMode mode;
  GtkSelectionMode selection_mode;

  guint freeze_count;

  guint icon_width;

  guint text_space;   /* entry width */
  guint row_spacing;  /* space between rows */ 
  guint col_spacing;  /* space between columns */
  guint icon_border;  /* space between icon and entry */
  
  gboolean is_editable;

  GtkIconListItem *active_icon;

  GdkColor background;

  gint num_icons;
  GList *icons;

  GList *selection;

  GCompareFunc compare_func;
};

struct _GtkIconListClass
{
  GtkFixedClass parent_class;

  gboolean (*select_icon)	(GtkIconList *iconlist,
                         	 GtkIconListItem *icon,
                         	 GdkEvent *event);
  void (*unselect_icon)		(GtkIconList *iconlist,
                        	 GtkIconListItem *icon,
                         	 GdkEvent *event);
  gboolean (*text_changed)	(GtkIconList *iconlist,
                         	 GtkIconListItem *icon,
                         	 gchar *new_text);
  gboolean (*activate_icon)	(GtkIconList *iconlist,
                                 GtkIconListItem *icon);
  gboolean (*deactivate_icon)	(GtkIconList *iconlist,
                                 GtkIconListItem *icon);
  void (*click_event)		(GtkIconList *iconlist,
                         	 GdkEvent *event);
};


GType		gtk_icon_list_get_type	(void);
GType		gtk_icon_list_item_get_type	(void);
GtkWidget*	gtk_icon_list_new		(guint icon_width,
						 GtkIconListMode mode);
void		gtk_icon_list_construct		(GtkIconList *iconlist,
       						 guint icon_width,
						 GtkIconListMode mode);
void		gtk_icon_list_set_mode		(GtkIconList *iconlist,
						 GtkIconListMode mode);
GtkIconListMode	gtk_icon_list_get_mode		(GtkIconList *iconlist);
void		gtk_icon_list_set_editable	(GtkIconList *iconlist,
						 gboolean editable);
gboolean	gtk_icon_list_is_editable	(GtkIconList *iconlist);
void		gtk_icon_list_set_row_spacing	(GtkIconList *iconlist,
						 guint spacing);
guint		gtk_icon_list_get_row_spacing	(GtkIconList *iconlist);
void		gtk_icon_list_set_col_spacing	(GtkIconList *iconlist,
						 guint spacing);
guint		gtk_icon_list_get_col_spacing	(GtkIconList *iconlist);
void		gtk_icon_list_set_text_space	(GtkIconList *iconlist,
						 guint text_space);
guint		gtk_icon_list_get_text_space	(GtkIconList *iconlist);
void		gtk_icon_list_set_icon_border	(GtkIconList *iconlist,
						 guint border);
guint		gtk_icon_list_get_icon_border	(GtkIconList *iconlist);
void		gtk_icon_list_set_icon_width	(GtkIconList *iconlist,
						 guint width);
guint		gtk_icon_list_get_icon_width	(GtkIconList *iconlist);
void		gtk_icon_list_freeze		(GtkIconList *iconlist);
void		gtk_icon_list_thaw		(GtkIconList *iconlist);
void		gtk_icon_list_set_background	(GtkIconList *iconlist,
						 GdkColor *color);
GtkIconListItem *gtk_icon_list_add_from_pixmap	(GtkIconList *icon_list,
						 GdkPixmap *pixmap,
						 GdkBitmap *bitmap_mask,
						 const gchar *label,
                                                 gpointer link);
GtkIconListItem *gtk_icon_list_add_from_data	(GtkIconList *iconlist,
						 gchar **data,
						 const gchar *label,
                                                 gpointer link);
GtkIconListItem *gtk_icon_list_add		(GtkIconList *iconlist,
						 const gchar *pixmap_file,
						 const gchar *label,
                                                 gpointer link);
GtkIconListItem	*gtk_icon_list_get_nth		(GtkIconList *iconlist,
						 guint n);
gint		gtk_icon_list_get_index		(GtkIconList *iconlist,
						 GtkIconListItem *item);
void		gtk_icon_list_remove		(GtkIconList *iconlist,
						 GtkIconListItem *item);
void		gtk_icon_list_set_active_icon	(GtkIconList *iconlist,
						 GtkIconListItem *icon);
void		gtk_icon_list_remove_nth	(GtkIconList *iconlist,
						 guint n);
void		gtk_icon_list_update		(GtkIconList *iconlist);
void		gtk_icon_list_clear		(GtkIconList *iconlist);
void		gtk_icon_list_link		(GtkIconListItem *item,
						 gpointer data);
gpointer	gtk_icon_list_get_link		(GtkIconListItem *item);
GtkIconListItem *gtk_icon_list_get_icon_from_link(GtkIconList *iconlist,
   						 gpointer data);
GtkIconListItem *gtk_icon_list_get_icon_at  	(GtkIconList *iconlist,
						 gint x, gint y);
GtkIconListItem *gtk_icon_list_get_active_icon  (GtkIconList *iconlist);
GtkWidget 	*gtk_icon_list_get_entry	(GtkIconListItem *item);
GtkWidget 	*gtk_icon_list_get_pixmap	(GtkIconListItem *item);
void		gtk_icon_list_set_pixmap	(GtkIconListItem *item,
						 GdkPixmap *pixmap,
						 GdkBitmap *bitmap_mask);
void 		gtk_icon_list_set_label		(GtkIconList *iconlist,
                                                 GtkIconListItem *item, 
						 const gchar *label);
void 		gtk_icon_list_set_selection_mode(GtkIconList *iconlist, 
						 gint mode);
void		gtk_icon_list_select_icon	(GtkIconList *iconlist, 
						 GtkIconListItem *item);
void		gtk_icon_list_unselect_icon	(GtkIconList *iconlist, 
						 GtkIconListItem *item);
void		gtk_icon_list_unselect_all	(GtkIconList *iconlist);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_ICON_LIST_H__ */
