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

/**
 * SECTION: gtkiconlist
 * @short_description: A file icon list widget for GTK.
 *
 * GtkIconList is a GtkFixed subclass that allows you to display a table of xpm icons with editable labels.
 * It's completely designed from scratch with some ideas borrowed from gnome-icon-list.
 * It contains almost all of its features and more. 
 * It can be used for file browsers and as a replacement of gnome-icon-list (without the need of using gnome libraries). 
 * Like GtkSheet, it uses the widget GtkItemEntry to edit the text fields.
 */



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>
#include "gtkextra-compat.h"
#include "gtkitementry.h"
#include "gtkiconlist.h"
#include "gtkextra-marshal.h"
#include <math.h>

#define DEFAULT_ROW_SPACING 	4
#define DEFAULT_COL_SPACING 	10	
#define DEFAULT_TEXT_SPACE 	60
#define DEFAULT_ICON_BORDER 	2
#define DEFAULT_WIDTH 150
#define DEFAULT_HEIGHT 120

#define EVENTS_MASK    (GDK_EXPOSURE_MASK |		\
                       GDK_POINTER_MOTION_MASK |	\
                       GDK_POINTER_MOTION_HINT_MASK |	\
                       GDK_BUTTON_PRESS_MASK |		\
                       GDK_BUTTON_RELEASE_MASK)

/* Signals */

extern void 
_gtkextra_signal_emit(GtkObject *object, guint signal_id, ...);

enum{
      SELECT_ICON,                       
      UNSELECT_ICON,                       
      TEXT_CHANGED,                       
      ACTIVATE_ICON,                       
      DEACTIVATE_ICON,                       
      CLICK_EVENT,                       
      LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL] = {0};

static void gtk_icon_list_class_init 		(GtkIconListClass *klass);
static void gtk_icon_list_init 			(GtkIconList *icon_list);
static void gtk_icon_list_destroy 		(GtkObject *object);
static void gtk_icon_list_finalize 		(GObject *object);

static void gtk_icon_list_size_allocate		(GtkWidget *widget,
                                                 GtkAllocation *allocation);

static void gtk_icon_list_realize		(GtkWidget *widget);
static gint gtk_icon_list_expose		(GtkWidget *widget, 
						 GdkEventExpose *event);
static gint gtk_icon_list_button_press		(GtkWidget *widget, 
						 GdkEventButton *event);
static gint deactivate_entry			(GtkIconList *iconlist);
static gint entry_in				(GtkWidget *widget,
						 GdkEventButton *event,
						 gpointer data);
static gint entry_changed 			(GtkWidget *widget, 
						 gpointer data);
static void select_icon				(GtkIconList *iconlist, 
						 GtkIconListItem *item,
						 GdkEvent *event);
static void unselect_icon			(GtkIconList *iconlist, 
						 GtkIconListItem *item,
						 GdkEvent *event);
static void unselect_all			(GtkIconList *iconlist); 
static void set_labels			        (GtkIconList *iconlist, 
					         GtkIconListItem *item, 
						 const gchar *label); 
static GtkIconListItem *get_icon_from_entry	(GtkIconList *iconlist, 
						 GtkWidget *widget);
static void reorder_icons			(GtkIconList *iconlist);
static void item_size_request			(GtkIconList *iconlist, 
                 	 			 GtkIconListItem *item,
                  				 GtkRequisition *requisition);
static void gtk_icon_list_move			(GtkIconList *iconlist, 
						 GtkIconListItem *icon, 
                   				 guint x, guint y);
static GtkIconListItem *gtk_icon_list_real_add	(GtkIconList *iconualist,
						 GdkPixmap *pixmap,
						 GdkBitmap *mask,
						 const gchar *label,
                                                 gpointer data);
static GtkIconListItem *gtk_icon_list_put 	(GtkIconList *iconlist, 
                   				 guint x, guint y, 
                   				 GdkPixmap *pixmap,
						 GdkBitmap *mask,
                   				 const gchar *label,
                                                 gpointer data);
static gint icon_key_press			(GtkWidget *widget, 
						 GdkEventKey *key, 
						 gpointer data);
static gint sort_list				(gpointer a, gpointer b);
static void pixmap_destroy                      ( GtkImage* pixmap);


static GtkFixedClass *parent_class = NULL;

static guint STRING_WIDTH(GtkWidget *widget,
                                 PangoFontDescription *font, const gchar *text)
{
  PangoRectangle rect;
  PangoLayout *layout;

  layout = gtk_widget_create_pango_layout (widget, text);
  pango_layout_set_font_description (layout, font);

  pango_layout_get_pixel_extents (layout, NULL, &rect);

  g_object_unref(G_OBJECT(layout));
  return rect.width;
}


GType
gtk_icon_list_get_type (void)
{
  static GType icon_list_type = 0;

  if (!icon_list_type)
    {
      icon_list_type = g_type_register_static_simple (
		gtk_fixed_get_type (),
		"GtkIconList",
		sizeof (GtkIconListClass),
		(GClassInitFunc) gtk_icon_list_class_init,
		sizeof (GtkIconList),
		(GInstanceInitFunc) gtk_icon_list_init,
		0);
    }
  return icon_list_type;
}

static GtkIconListItem*
gtk_icon_list_item_copy (const GtkIconListItem *item)
{
  GtkIconListItem *new_item;

  g_return_val_if_fail (item != NULL, NULL);

  new_item = g_new (GtkIconListItem, 1);

  *new_item = *item;

  return new_item;
}

static void
gtk_icon_list_item_free (GtkIconListItem *item)
{
  g_return_if_fail (item != NULL);

  g_free (item);
}


GType
gtk_icon_list_item_get_type (void)
{
  static GType icon_list_item_type;

  if(!icon_list_item_type)
  {
    icon_list_item_type = g_boxed_type_register_static("GtkIconListItem", (GBoxedCopyFunc)gtk_icon_list_item_copy, (GBoxedFreeFunc)gtk_icon_list_item_free);
  }
  return icon_list_item_type;
}
 

static void
gtk_icon_list_class_init (GtkIconListClass *klass)
{
  GtkObjectClass *object_class;
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  parent_class = g_type_class_ref (gtk_fixed_get_type ());

  object_class = (GtkObjectClass *) klass;
  gobject_class = (GObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  object_class->destroy = gtk_icon_list_destroy;
  gobject_class->finalize = gtk_icon_list_finalize;

  widget_class->realize = gtk_icon_list_realize;

  widget_class->size_allocate = gtk_icon_list_size_allocate;

  widget_class->expose_event = gtk_icon_list_expose;
  widget_class->button_press_event = gtk_icon_list_button_press;

  /**
   * GtkIconList::select_icon:
   * @iconlist: #GtkIconList widget created with gtk_icon_list_new()
   * @icon: a #GtkIconListItem
   * @event: a #GdkEvent
   *
   * Emmited  whenever the icon is selected.
   */ 
  signals[SELECT_ICON] =
      g_signal_new("select_icon",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(GtkIconListClass, select_icon),
		     NULL, NULL,
                     gtkextra_BOOLEAN__BOXED_BOXED,
		     G_TYPE_BOOLEAN, 2,
		     G_TYPE_ICON_LIST_ITEM, GDK_TYPE_EVENT); 

  /**
   * GtkIconList::unselect_icon:
   * @iconlist: #GtkIconList widget created with gtk_icon_list_new()
   * @icon: a #GtkIconListItem
   * @event: a #GdkEvent
   *
   * Emmited  whenever the icon is unselected.
   */
  signals[UNSELECT_ICON] =
      g_signal_new("unselect_icon",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_FIRST,
		     G_STRUCT_OFFSET(GtkIconListClass, unselect_icon),
		     NULL, NULL,
                     gtkextra_VOID__BOXED_BOXED,
		     G_TYPE_NONE, 2,
		     G_TYPE_ICON_LIST_ITEM, GDK_TYPE_EVENT); 

  /**
   * GtkIconList::text_changed:
   * @iconlist: #GtkIconList widget created with gtk_icon_list_new()
   * @icon: a #GtkIconListItem
   * @new_text: modified text
   *
   * Emmited  whenever text of the icon is changed.
   */
  signals[TEXT_CHANGED] =
      g_signal_new("text_changed",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(GtkIconListClass, text_changed),
		     NULL,NULL,
                     gtkextra_BOOLEAN__BOXED_STRING,
		     G_TYPE_BOOLEAN, 2,
		     G_TYPE_ICON_LIST_ITEM, G_TYPE_STRING); 

  /**
   * GtkIconList::activate_icon:
   * @iconlist: #GtkIconList widget created with gtk_icon_list_new()
   * @icon: a #GtkIconListItem
   *
   * Emmited  whenever whenever an icon is activated.
   */
  signals[ACTIVATE_ICON] =
      g_signal_new("activate_icon",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(GtkIconListClass, activate_icon),
		     NULL, NULL,
                     gtkextra_BOOLEAN__BOXED,
		     G_TYPE_BOOLEAN, 1,
		     G_TYPE_ICON_LIST_ITEM); 

  /**
   * GtkIconList::deactivate_icon:
   * @iconlist: #GtkIconList widget created with gtk_icon_list_new()
   * @icon: a #GtkIconListItem
   *
   * Emmited  whenever whenever an icon is deactivated.
   */
  signals[DEACTIVATE_ICON] =
      g_signal_new("deactivate_icon",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(GtkIconListClass, deactivate_icon),
		     NULL, NULL,
                     gtkextra_BOOLEAN__BOXED,
		     G_TYPE_BOOLEAN, 1,
		     G_TYPE_ICON_LIST_ITEM); 

  /**
   * GtkIconList::click_event:
   * @iconlist: #GtkIconList widget created with gtk_icon_list_new()
   * @event: a #GdkEvent
   *
   * Emmited whenever an event occures.
   */
  signals[CLICK_EVENT] =
      g_signal_new("click_event",
		     G_TYPE_FROM_CLASS(object_class),
		     G_SIGNAL_RUN_LAST,
		     G_STRUCT_OFFSET(GtkIconListClass, click_event),
		     NULL, NULL,
                     gtkextra_VOID__BOXED,
		     G_TYPE_NONE, 1,
		     GDK_TYPE_EVENT); 
 
}


/**
 * gtk_icon_list_freeze:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Freeze icon list. 
 */   
void
gtk_icon_list_freeze(GtkIconList *iconlist)
{
 iconlist->freeze_count++;
}

/**
 * gtk_icon_list_thaw:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Thaw(unfreeze) icon list. 
 */   
void
gtk_icon_list_thaw(GtkIconList *iconlist)
{
 if(iconlist->freeze_count == 0) return;
 iconlist->freeze_count--;
 if(iconlist->freeze_count == 0)
               reorder_icons(iconlist);
}

/**
 * gtk_icon_list_update:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Update the icon list widget.. 
 */ 
void
gtk_icon_list_update(GtkIconList *iconlist)
{
  reorder_icons(iconlist);
}

static void
reorder_icons(GtkIconList *iconlist)
{
  GtkWidget *widget;
  GtkIconListItem *item;
  GtkRequisition req;
  GtkAllocation allocation;
  GList *icons;
  gint hspace = 0;
  gint vspace = 0;
  gint x = 0;
  gint y = 0;
  gint old_width, old_height;

  widget = GTK_WIDGET(iconlist);

  if(iconlist->freeze_count > 0) return;

/*
  gdk_threads_enter();
*/

  gtk_widget_get_allocation(widget, &allocation);
  old_width = allocation.width;
  old_height = allocation.height;
  if(gtk_widget_get_realized(widget)){
    if(GTK_IS_VIEWPORT(gtk_widget_get_parent(widget)))
     gdk_window_get_size(
		GTK_VIEWPORT(gtk_widget_get_parent(widget))->view_window, 
		&old_width, &old_height);
  }

  y = iconlist->row_spacing;
  x = iconlist->col_spacing;

  icons = iconlist->icons;
  while(icons){
    item = (GtkIconListItem *) icons->data;

    gtk_icon_list_move(iconlist, item, x, y);

    item_size_request(iconlist, item, &req);
 
    vspace = req.height + iconlist->row_spacing;
    hspace = req.width + iconlist->col_spacing;

    switch(iconlist->mode){
      case GTK_ICON_LIST_TEXT_RIGHT:
        y += vspace;
        if(y + vspace >= old_height - DEFAULT_COL_SPACING){
                x += hspace;
                y = iconlist->row_spacing;
        }
        break;
      case GTK_ICON_LIST_TEXT_BELOW:
      case GTK_ICON_LIST_ICON:
      default:
        x += hspace;
        if(x + hspace >= old_width - DEFAULT_COL_SPACING){
                x = iconlist->col_spacing;
                y += vspace;
        }
        break;
    }

    icons = icons->next;

   }
/*
  gdk_threads_leave();
*/
}


static void
gtk_icon_list_move(GtkIconList *iconlist, GtkIconListItem *icon, 
                   guint x, guint y)
{
  GtkRequisition req1, req2;
  GtkRequisition req;
  GtkRequisition pixmap_req;
  GtkRequisition entry_req;
  GtkAllocation a;
  GtkAllocation pixmap_allocation;
  GtkAllocation entry_allocation;
  GtkAllocation icon_allocation;
  const gchar *text;
  gint old_width, old_height, width, height;
  gint old_x, old_y;
  gint size;

  old_x = icon->x;
  old_y = icon->y;

  icon->x = x;
  icon->y = y;

/*
  if(x == old_x && y == old_y) return;
*/

  item_size_request(iconlist, icon, &req);
  gtk_widget_get_requisition(icon->pixmap, &pixmap_req);
  gtk_widget_get_requisition(icon->entry, &entry_req);
  req1 = pixmap_req;
  req2 = entry_req;
  req2.width = iconlist->text_space;

  req1.width += 2*iconlist->icon_border;
  req1.height += 2*iconlist->icon_border;
  if(iconlist->mode == GTK_ICON_LIST_TEXT_BELOW){
     req1.width = MAX(req1.width, req.width);
  }

  if(iconlist->mode == GTK_ICON_LIST_ICON) 
          req2.width = req2.height = 0;

  gtk_widget_get_allocation(GTK_WIDGET(iconlist), &icon_allocation);
  old_width = width = icon_allocation.width;
  old_height = height = icon_allocation.height;

  gtk_fixed_move(GTK_FIXED(iconlist), icon->pixmap, 
                 x + req1.width/2 - pixmap_req.width/2, 
                 y + iconlist->icon_border);

  gtk_widget_get_allocation(icon->pixmap, &pixmap_allocation);
  gtk_widget_get_allocation(icon->entry, &entry_allocation);
  pixmap_allocation.x += (x - old_x);
  pixmap_allocation.y += (y - old_y);
  entry_allocation.x += (x - old_x);
  entry_allocation.y += (y - old_y);
  entry_allocation.width = req2.width;
  gtk_widget_set_allocation(icon->pixmap, &pixmap_allocation);
  gtk_widget_set_allocation(icon->entry, &entry_allocation);


  switch(iconlist->mode){
   case GTK_ICON_LIST_TEXT_BELOW:
        text = gtk_entry_get_text(GTK_ENTRY(icon->entry));
        size = STRING_WIDTH(icon->entry, 
			    gtk_widget_get_style(icon->entry)->font_desc, text);
        size = MIN(size, req2.width);

  	gtk_fixed_move(GTK_FIXED(iconlist), icon->entry, 
        	        x - req2.width/2 + req1.width/2, 
                        y + req1.height + iconlist->icon_border);

        if(y + req.height > height) 
           height += req.height;
        break;
   case GTK_ICON_LIST_TEXT_RIGHT:
  	gtk_fixed_move(GTK_FIXED(iconlist), icon->entry, 
        	        x + req1.width + iconlist->icon_border, 
                        y + req1.height/2 - req2.height/2); 

        if(x + req.width > width) 
            width += req.width;
	break;
   case GTK_ICON_LIST_ICON:
   default: ;
  }

  gtk_widget_get_allocation(icon->entry, &a);

  if(icon->entry){
    gtk_widget_size_allocate(icon->entry, &a);
    gtk_widget_draw(icon->entry, NULL);
  }

}


static void
gtk_icon_list_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
  GtkAllocation *old = gtk_object_get_data(GTK_OBJECT(widget),"viewport");
  GTK_WIDGET_CLASS(parent_class)->size_allocate(widget, allocation);
  if(gtk_widget_get_realized(widget) && old){
    gint new_width, new_height;
    gdk_window_get_size(GTK_VIEWPORT(gtk_widget_get_parent(widget))->view_window, &new_width, &new_height);
    if(old->width != new_width || old->height != new_height)
      reorder_icons(GTK_ICON_LIST(widget));
    old->width = new_width; 
    old->height = new_height; 
  }
}


static void
gtk_icon_list_realize(GtkWidget *widget)
{
  GList *icons;
  GtkIconList *iconlist;
  GtkIconListItem *item;
  GtkStyle *style;
  
  GTK_WIDGET_CLASS(parent_class)->realize (widget);

  iconlist = GTK_ICON_LIST(widget);

  style = gtk_style_copy(gtk_widget_get_style(widget));

  style->bg[0] = iconlist->background;
  gtk_widget_set_style(widget, style);
  gtk_style_set_background(style, gtk_widget_get_window(widget), 
			   GTK_STATE_NORMAL);
  gtk_style_set_background(style, gtk_widget_get_window(widget), 
			    GTK_STATE_ACTIVE);

  icons = iconlist->icons;
  while(icons){
    item = (GtkIconListItem *) icons->data;
    gtk_widget_draw(item->pixmap, NULL);
    if(iconlist->mode != GTK_ICON_LIST_ICON){
      GtkStyle *style;

      gtk_widget_realize(item->entry);

      style = gtk_style_copy(gtk_widget_get_style(item->entry));
      style->bg[GTK_STATE_ACTIVE] = iconlist->background;
      style->bg[GTK_STATE_NORMAL] = iconlist->background;
      gtk_widget_set_style(item->entry, style);
      gtk_widget_show(item->entry);
    }
    if(item->entry) gtk_widget_draw(item->entry, NULL);
    icons = icons->next;
  }

/*  
  if(GTK_IS_VIEWPORT(widget->parent) && gtk_widget_get_realized(widget->parent)){
    GtkAllocation *allocation = gtk_object_get_data(GTK_OBJECT(widget),"viewport");
    gdk_window_get_size(GTK_VIEWPORT(widget->parent)->view_window, &allocation->width, &allocation->height);
  }
  reorder_icons(iconlist);
*/
}


static void
gtk_icon_list_init (GtkIconList *icon_list)
{
  GtkWidget *widget;
  widget = GTK_WIDGET(icon_list);

  gtk_widget_ensure_style(widget);
  gdk_color_black(gtk_widget_get_colormap(widget), 
		    &(gtk_widget_get_style(widget)->black));
  gdk_color_white(gtk_widget_get_colormap(widget), 
		    &(gtk_widget_get_style(widget)->white));

  gtk_fixed_set_has_window(GTK_FIXED(widget), TRUE);

  gtk_widget_set_events (widget, gtk_widget_get_events(widget)|
                         EVENTS_MASK);

  icon_list->selection = NULL;
  icon_list->is_editable = TRUE;

  icon_list->num_icons = 0;
  icon_list->background = gtk_widget_get_style(widget)->white;

  icon_list->row_spacing = DEFAULT_ROW_SPACING;
  icon_list->col_spacing = DEFAULT_COL_SPACING;
  icon_list->text_space = DEFAULT_TEXT_SPACE;
  icon_list->icon_border = DEFAULT_ICON_BORDER;

  icon_list->active_icon = NULL;
  icon_list->compare_func = (GCompareFunc)sort_list;
}

static gint 
sort_list(gpointer a, gpointer b)
{
  GtkIconListItem *itema;
  GtkIconListItem *itemb;

  itema = (GtkIconListItem *)a;
  itemb = (GtkIconListItem *)b;

  return (strcmp(itema->label, itemb->label));
}

static gboolean
gtk_icon_list_expose (GtkWidget *widget, GdkEventExpose *event)
{
  GtkIconList *icon_list;

  icon_list = GTK_ICON_LIST(widget);

  if(!gtk_widget_is_drawable(widget)) return FALSE;

  gtk_paint_flat_box (gtk_widget_get_style(widget), 
		      gtk_widget_get_window(widget), 
		      GTK_STATE_NORMAL,
                      GTK_SHADOW_NONE, &event->area, widget, 
		      "base", 0, 0, -1, -1);

  GTK_WIDGET_CLASS(parent_class)->expose_event(widget, event);

  if(icon_list->active_icon && icon_list->active_icon->entry){
	GtkAllocation allocation;
	gtk_widget_get_allocation(icon_list->active_icon->entry, &allocation);
	gdk_draw_rectangle(gtk_widget_get_window(widget),
                           gtk_widget_get_style(widget)->black_gc,
                           FALSE,
                           allocation.x-2,
                           allocation.y-2,
                           allocation.width+4,
                           allocation.height+4);
  }

  return FALSE;
}

static gint
gtk_icon_list_button_press(GtkWidget *widget, GdkEventButton *event)
{
  GtkIconList *iconlist;
  GtkIconListItem *item;
  gint x, y;
  GtkAllocation allocation;

  if(!GTK_IS_ICON_LIST(widget)) return FALSE;

  iconlist = GTK_ICON_LIST(widget);


  gtk_widget_get_pointer(widget, &x, &y);
  item = gtk_icon_list_get_icon_at(iconlist, x , y );

  if(!item){ 
     g_signal_emit(GTK_OBJECT(iconlist), signals[CLICK_EVENT], 0,
                     event);
     return FALSE;
  }

  if(item->entry){
    gtk_widget_get_allocation(item->entry, &allocation);
    if(x >= allocation.x &&
       x <= allocation.x + allocation.width &&
       y >= allocation.y &&
       y <= allocation.y + allocation.height) return FALSE;
  }

  if(item)
   switch(iconlist->selection_mode){
     case GTK_SELECTION_SINGLE:
     case GTK_SELECTION_BROWSE:
        unselect_all(iconlist);
     case GTK_SELECTION_MULTIPLE:
        select_icon(iconlist, item, (GdkEvent *)event);
     case GTK_SELECTION_NONE:
        break;
   }

  return FALSE;
}


static gint 
deactivate_entry(GtkIconList *iconlist)
{
  GdkGC *gc;
  GtkEntry *entry;
  gboolean veto = TRUE;

  if(iconlist->active_icon) {
     _gtkextra_signal_emit(GTK_OBJECT(iconlist), signals[DEACTIVATE_ICON], 
                     iconlist->active_icon, &veto);
     if(!veto) return FALSE;

     entry = GTK_ENTRY(iconlist->active_icon->entry);

     if(!entry || !gtk_widget_get_realized(GTK_WIDGET(entry))) return TRUE;
     gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);    
     gtk_entry_select_region(entry, 0, 0);
     gtk_item_entry_set_cursor_visible(GTK_ITEM_ENTRY(entry), FALSE);

     switch(iconlist->mode){
       case GTK_ICON_LIST_TEXT_RIGHT:
         gtk_item_entry_set_text(GTK_ITEM_ENTRY(entry), 
                                 iconlist->active_icon->entry_label,
                                 GTK_JUSTIFY_LEFT);
         break;
       case GTK_ICON_LIST_TEXT_BELOW:
         gtk_item_entry_set_text(GTK_ITEM_ENTRY(entry), 
                                 iconlist->active_icon->entry_label,
                                 GTK_JUSTIFY_CENTER);
         break;
       case GTK_ICON_LIST_ICON:
       default:
         break;
     }

     if(gtk_widget_get_realized(iconlist->active_icon->entry)){
       GtkAllocation allocation;
       gtk_widget_get_allocation(GTK_WIDGET(entry), &allocation);

       gc = gdk_gc_new(gtk_widget_get_window(GTK_WIDGET(iconlist)));
       gdk_gc_set_foreground(gc, &iconlist->background);
       gdk_draw_rectangle(gtk_widget_get_window((GTK_WIDGET(iconlist))),
                          gc,
                          FALSE,
                          allocation.x-2,
                          allocation.y-2,
                          allocation.width+4,
                          allocation.height+4);
       gdk_gc_unref(gc);
     }

/*
     iconlist->active_icon->state = GTK_STATE_NORMAL;
*/
     iconlist->active_icon = NULL;
  }

  return TRUE;
}


static void
select_icon(GtkIconList *iconlist, GtkIconListItem *item, GdkEvent *event)
{
  gboolean veto = TRUE;

  if(item == NULL) return;

  _gtkextra_signal_emit(GTK_OBJECT(iconlist), signals[SELECT_ICON], item, 
                  event, &veto);                        

  if(!veto) return;

  if(iconlist->mode != GTK_ICON_LIST_ICON){ 
    if(!deactivate_entry(iconlist)) return;

    if(item->state != GTK_STATE_NORMAL && iconlist->selection_mode == GTK_SELECTION_MULTIPLE) {
       unselect_icon(iconlist, item, event);
       return;
    }
    if(item->state == GTK_STATE_SELECTED) return;

    if(item->entry && gtk_widget_get_realized(item->entry)){
      GtkStyle *style = gtk_style_copy(gtk_widget_get_style(item->entry));
      style->bg[GTK_STATE_ACTIVE] = style->base[GTK_STATE_SELECTED];
      style->bg[GTK_STATE_NORMAL] = style->base[GTK_STATE_SELECTED];
      style->text[GTK_STATE_ACTIVE] = style->text[GTK_STATE_SELECTED];
      style->text[GTK_STATE_NORMAL] = style->text[GTK_STATE_SELECTED];
      gtk_widget_set_style(item->entry, style);
      gtk_style_unref(style);

      switch(iconlist->mode){
        case GTK_ICON_LIST_TEXT_RIGHT:
          gtk_item_entry_set_text(GTK_ITEM_ENTRY(item->entry), 
                                  item->label,
                                  GTK_JUSTIFY_LEFT);
          break;
        case GTK_ICON_LIST_TEXT_BELOW:
          gtk_item_entry_set_text(GTK_ITEM_ENTRY(item->entry), 
                                  item->label,
                                  GTK_JUSTIFY_CENTER);
          break;
        case GTK_ICON_LIST_ICON:
        default:
          break;
      }
    }
  }
  iconlist->selection = g_list_append(iconlist->selection, item);

/*
{
  GList *aux = iconlist->selection;
  while(aux){
  GtkIconListItem *kk = (GtkIconListItem *)aux->data;
  printf("SELECTION %s\n",kk->entry_label);
  aux = aux->next;
  }
}
*/


  item->state = GTK_STATE_SELECTED;  
  if(item->entry) gtk_widget_grab_focus(item->entry);
}

static void
unselect_icon(GtkIconList *iconlist, GtkIconListItem *item, GdkEvent *event)
{
  GList *selection;
  GtkIconListItem *icon;

  if(!item) return;

  if(item->state == GTK_STATE_NORMAL) return;

  selection = iconlist->selection;
  while(selection){
    icon = (GtkIconListItem *)selection->data; 
    if(item == icon) break;
    selection = selection->next;
  }

  if(selection){
       iconlist->selection = g_list_remove_link(iconlist->selection, selection);
  }

  item->state = GTK_STATE_NORMAL;

  if(iconlist->mode != GTK_ICON_LIST_ICON){
   if(item->entry && gtk_widget_get_realized(item->entry)){
     GtkStyle *style = gtk_style_copy(gtk_widget_get_style(item->entry));

     style->bg[GTK_STATE_ACTIVE] = iconlist->background;
     style->bg[GTK_STATE_NORMAL] = iconlist->background;
     style->text[GTK_STATE_ACTIVE] = 
	gtk_widget_get_style(GTK_WIDGET(iconlist))->text[GTK_STATE_ACTIVE];
     style->text[GTK_STATE_NORMAL] = 
	gtk_widget_get_style(GTK_WIDGET(iconlist))->text[GTK_STATE_NORMAL];
     gtk_widget_set_style(item->entry, style);
     gtk_style_unref(style);

     gtk_entry_select_region(GTK_ENTRY(item->entry), 0, 0);
     gtk_entry_set_text(GTK_ENTRY(item->entry), item->entry_label);
     gtk_editable_set_editable(GTK_EDITABLE(item->entry), FALSE);
     gtk_widget_draw(item->entry, NULL);

   }
  }

  g_signal_emit(GTK_OBJECT(iconlist), signals[UNSELECT_ICON], 0, item, event);                        
}

static void
unselect_all(GtkIconList *iconlist)
{
  GList *selection;
  GtkIconListItem *item;

  selection = iconlist->selection;
  while(selection){
    item = (GtkIconListItem *)selection->data;
    unselect_icon(iconlist, item, NULL);
    selection = iconlist->selection;
  }

  g_list_free(iconlist->selection);
  iconlist->selection = NULL;
}

/**
 * gtk_icon_list_new:
 * @icon_width: the width of the icon 
 * @mode: GTK_ICON_LIST_ICON,GTK_ICON_LIST_TEXT_RIGHT,GTK_ICON_LIST_TEXT_BELOW
 * 
 * Creates a new #GtkIconList widget.
 *  
 * Returns: the newly-created #GtkIconList widget.
 */
GtkWidget*
gtk_icon_list_new (guint icon_width, GtkIconListMode mode)
{
  GtkIconList *icon_list;
  GtkAllocation *allocation;

  icon_list = g_object_new (gtk_icon_list_get_type (),NULL);

  gtk_icon_list_construct(icon_list, icon_width, mode);
  allocation = g_new0(GtkAllocation, 1);
  gtk_object_set_data(GTK_OBJECT(icon_list), "viewport", allocation);

  return GTK_WIDGET (icon_list);
}

/**
 * gtk_icon_list_construct:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @icon_width: the width of the icon 
 * @mode: GTK_ICON_LIST_ICON,GTK_ICON_LIST_TEXT_RIGHT,GTK_ICON_LIST_TEXT_BELOW
 *
 * Initialize iconlist structure. 
 */
void
gtk_icon_list_construct (GtkIconList *icon_list, guint icon_width, GtkIconListMode mode)
{
  icon_list->icon_width = icon_width;
  icon_list->mode = mode;
  icon_list->icons = NULL;
  icon_list->selection = NULL;
  icon_list->selection_mode = GTK_SELECTION_SINGLE;
}

/**
 * gtk_icon_list_set_mode:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @mode: GTK_ICON_LIST_ICON,GTK_ICON_LIST_TEXT_RIGHT,GTK_ICON_LIST_TEXT_BELOW
 *
 * Set the icons display mode . 
 */
void
gtk_icon_list_set_mode (GtkIconList *iconlist, GtkIconListMode mode)
{
  GList *icons;

  iconlist->mode = mode;

  icons = iconlist->icons;
  while(icons){
    GtkIconListItem *item;

    item = (GtkIconListItem *)icons->data;

    switch(mode){
      case GTK_ICON_LIST_TEXT_RIGHT:
        gtk_item_entry_set_justification(GTK_ITEM_ENTRY(item->entry), 
                                         GTK_JUSTIFY_LEFT);
        break;
      case GTK_ICON_LIST_TEXT_BELOW:
        gtk_item_entry_set_justification(GTK_ITEM_ENTRY(item->entry), 
                                         GTK_JUSTIFY_CENTER);
        break;
      case GTK_ICON_LIST_ICON:
      default:
        break;
    }
    
    icons = icons->next;
  }
 
  reorder_icons(iconlist); 
}

/**
 * gtk_icon_list_get_mode:
 * @iconlist: a #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the icons display mode . 
 *  
 * Returns: 
 * GTK_ICON_LIST_ICON,GTK_ICON_LIST_TEXT_RIGHT,GTK_ICON_LIST_TEXT_BELOW 
 */
GtkIconListMode
gtk_icon_list_get_mode (GtkIconList *iconlist)
{
  return(iconlist->mode);
}

/**
 * gtk_icon_list_set_text_space:
 * @iconlist: a #GtkIconList widget created with gtk_icon_list_new().
 * @text_space: distance in pixels.
 *
 * Set the text max size in pixels.
 */
void
gtk_icon_list_set_text_space (GtkIconList *iconlist, guint text_space)
{
  GList *icons;

  iconlist->text_space = text_space;

  icons = iconlist->icons;
  while(icons){
    GtkIconListItem *item;

    item = (GtkIconListItem *)icons->data;
    
    if(item->entry) GTK_ITEM_ENTRY(item->entry)->text_max_size = text_space;
    
    icons = icons->next;
  }
 
  reorder_icons(iconlist); 
}


static void
gtk_icon_list_destroy (GtkObject *object)
{
  GtkIconList *icon_list;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_ICON_LIST (object));

  icon_list = GTK_ICON_LIST (object);

  gtk_icon_list_clear(icon_list);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_icon_list_finalize (GObject *object)
{
  GtkIconList *icon_list;
  GtkAllocation *allocation;

  icon_list = GTK_ICON_LIST (object);

  allocation = gtk_object_get_data(GTK_OBJECT(icon_list), "viewport");
  if(allocation) g_free(allocation);
  gtk_object_set_data(GTK_OBJECT(icon_list), "viewport", NULL);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (object);
}

/**
 * gtk_icon_list_set_background:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @color: a #GdkColor
 *
 * Set the iconlist background color. 
 */
void
gtk_icon_list_set_background (GtkIconList *iconlist, GdkColor *color)
{
  GtkWidget *widget;
  GtkStyle *style;
  
  g_return_if_fail (iconlist != NULL);
  g_return_if_fail (GTK_IS_ICON_LIST (iconlist));

  widget = GTK_WIDGET(iconlist);

  iconlist->background = *color;

  style = gtk_style_copy(gtk_widget_get_style(widget));
  style->bg[0] = iconlist->background;

  gtk_widget_set_style(widget, style);
  if(gtk_widget_get_window(widget)) 
	gdk_window_set_background(gtk_widget_get_window(widget), color);
  gtk_style_unref(style);

}

static gint
entry_changed (GtkWidget *widget, gpointer data)
{
  GtkIconList *iconlist;
  GtkIconListItem *item;
  gboolean veto = TRUE;
  const gchar *text;

  iconlist = GTK_ICON_LIST(data);
  item = get_icon_from_entry(iconlist, widget);
  text = gtk_entry_get_text(GTK_ENTRY(widget));

  _gtkextra_signal_emit(GTK_OBJECT(data), signals[TEXT_CHANGED],
                  item, text, &veto);

  if(!veto) return veto; 

  if(item->entry && gtk_editable_get_editable(GTK_EDITABLE(item->entry))){
    if(item->label) g_free(item->label);
    if(text) item->label = g_strdup(text); 
    if(item->entry_label) g_free(item->entry_label);
    set_labels(iconlist, item, text); 
  }

  return veto;
} 
                  
static gint
entry_in (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GtkIconList *iconlist;
  GtkIconListItem *item;
  gboolean veto = TRUE;

  if(!GTK_IS_ENTRY(widget)) return FALSE;
  iconlist = GTK_ICON_LIST(data);

  item = get_icon_from_entry(iconlist, widget);
  if(iconlist->active_icon && iconlist->active_icon->entry == widget) 
          return FALSE;

  _gtkextra_signal_emit(GTK_OBJECT(iconlist), signals[ACTIVATE_ICON], &item, &veto);

  if(!veto) return FALSE; 
  if(!deactivate_entry(iconlist)) return FALSE;

  if(item->state == GTK_STATE_SELECTED){
   if(iconlist->is_editable && !gtk_editable_get_editable(GTK_EDITABLE(widget))){
     unselect_all(iconlist);

     gtk_editable_set_editable(GTK_EDITABLE(widget), TRUE);
     gtk_item_entry_set_cursor_visible(GTK_ITEM_ENTRY(widget), TRUE);
     if(item->label) gtk_entry_set_text(GTK_ENTRY(widget), item->label);
     iconlist->active_icon = item;
     item->state = GTK_STATE_NORMAL;

     if(gtk_widget_is_drawable(widget)) 
     {
	GtkAllocation allocation;
  	gtk_widget_get_allocation(iconlist->active_icon->entry, &allocation);
       gdk_draw_rectangle(gtk_widget_get_window(GTK_WIDGET(iconlist)),
                          gtk_widget_get_style(widget)->black_gc,
                          FALSE,
                          allocation.x-2,
                          allocation.y-2,
                          allocation.width+4,
                          allocation.height+4);
     }
   }else{
     g_signal_stop_emission_by_name (GTK_OBJECT(widget), "button_press_event"); 
     if(iconlist->selection_mode == GTK_SELECTION_SINGLE ||
        iconlist->selection_mode == GTK_SELECTION_BROWSE) 
          unselect_all(iconlist);
     select_icon(iconlist, item, (GdkEvent *)event);
   }
  }else{
     if(iconlist->selection_mode == GTK_SELECTION_SINGLE ||
        iconlist->selection_mode == GTK_SELECTION_BROWSE) 
          unselect_all(iconlist);
     select_icon(iconlist, item, (GdkEvent *)event);
  }

  return FALSE;
}

/**
 * gtk_icon_list_get_active_icon:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Set the iconlist background color. 
 *  
 * Returns: the icon pointer
 */
GtkIconListItem *
gtk_icon_list_get_active_icon(GtkIconList *iconlist)
{
  return iconlist->active_icon;
}

static GtkIconListItem *
get_icon_from_entry(GtkIconList *iconlist, GtkWidget *widget)
{
  GList *icons;
  GtkIconListItem *item;

  icons = iconlist->icons;
  while(icons){
    item = (GtkIconListItem *) icons->data;
    if(widget == item->entry) return item;
    icons = icons->next;
  }

  return NULL;
}

/**
 * gtk_icon_list_get_icon_at:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @x: x coordindate 
 * @y: y coordinate
 *
 * Get the icon situated at x,y coordinates in icon list.
 *  
 * Returns: the icon pointer
 */
GtkIconListItem *
gtk_icon_list_get_icon_at(GtkIconList *iconlist, gint x, gint y)
{
  GList *icons;
  GtkIconListItem *item;
  GtkRequisition req;

  icons = iconlist->icons;
  while(icons){
    item = (GtkIconListItem *) icons->data;
    item_size_request(iconlist, item, &req);
    if(x >= item->x && x <= item->x + req.width &&
       y >= item->y && y <= item->y + req.height) return item;
    icons = icons->next;
  }

  return NULL;
}

/**
 * gtk_icon_list_add:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @pixmap_file:  from #include "file.xpm"
 * @label: label of the icon
 * @link: link
 *
 * Add a icon to the icon list.
 *  
 * Returns: the icon pointer
 */
GtkIconListItem *
gtk_icon_list_add (GtkIconList *iconlist, 
                   const gchar *pixmap_file,
                   const gchar *label,
                   gpointer link)
{
  GtkIconListItem *item;
  GdkColormap *colormap;
  GdkPixmap *pixmap;
  GdkBitmap *mask;

  colormap = gdk_colormap_get_system();
  pixmap = gdk_pixmap_colormap_create_from_xpm(NULL, colormap, &mask, NULL, 
                                               pixmap_file);
  item = gtk_icon_list_real_add(iconlist, pixmap, mask, label, link);
  return item;
}

/**
 * gtk_icon_list_add_from_data:
 * @iconlist: a #GtkIconList widget created with gtk_icon_list_new().
 * @data:  pointer to the xpm data string
 * @label: label of the icon
 * @link: a data pointer
 *
 * Add a icon to the icon list.
 *  
 * Returns: the icon pointer
 */
GtkIconListItem *
gtk_icon_list_add_from_data (GtkIconList *iconlist, 
                             gchar **data,
                             const gchar *label,
                             gpointer link)
{
  GtkIconListItem *item;
  GdkColormap *colormap;
  GdkPixmap *pixmap;
  GdkBitmap *mask;

  colormap = gdk_colormap_get_system();
  pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask, NULL, 
                                                 data);
  item = gtk_icon_list_real_add(iconlist, pixmap, mask, label, link);
  return item;
}

/**
 * gtk_icon_list_add_from_pixmap:
 * @icon_list: a #GtkIconList widget created with gtk_icon_list_new().
 * @pixmap: a #GdkPixmap
 * @bitmap_mask: a #GdkBitmap 
 * @label: label of the icon
 * @link: a gpointer link to some data
 *
 * Add a icon to the icon list.
 *  
 * Returns: the icon pointer
 */
GtkIconListItem *
gtk_icon_list_add_from_pixmap (GtkIconList *iconlist, 
                               GdkPixmap *pixmap,
                               GdkBitmap *bitmap_mask,
                               const gchar *label,
                               gpointer link)
{
  GtkIconListItem *item;

  gdk_pixmap_ref(pixmap);
  if(bitmap_mask) gdk_bitmap_ref(bitmap_mask);
  item = gtk_icon_list_real_add(iconlist, pixmap, bitmap_mask, label, link);
  return item;
}


static GtkIconListItem*
gtk_icon_list_real_add (GtkIconList *iconlist, 
                        GdkPixmap *pixmap,
                        GdkBitmap *mask,
                        const gchar *label,
                        gpointer data)
{
  GtkIconListItem *item;
  GtkRequisition requisition;
  GtkAllocation allocation;
  gint hspace = 0;
  gint vspace = 0;
  gint x = 0;
  gint y = 0;
  gint width, height;

  gtk_widget_get_allocation(GTK_WIDGET(iconlist), &allocation);
  width = allocation.width;
  height = allocation.height;

  if(iconlist->num_icons > 0){
    item = gtk_icon_list_get_nth(iconlist, iconlist->num_icons-1);
    x = item->x;
    y = item->y;
    item_size_request(iconlist, item, &requisition);
 
    vspace = requisition.height + iconlist->row_spacing;
    hspace = requisition.width + iconlist->col_spacing;

    switch(iconlist->mode){
      case GTK_ICON_LIST_TEXT_RIGHT:
        y += vspace;
        if(y >= height){
                x += hspace;
                y = iconlist->row_spacing;
        }
        break;
      case GTK_ICON_LIST_TEXT_BELOW:
      case GTK_ICON_LIST_ICON:
      default:
        x += hspace;
        if(x >= width){
                x = iconlist->col_spacing;
                y += vspace;
        }
        break;
    }
  } else {
    y = iconlist->row_spacing;
    x = iconlist->col_spacing;
  }

  item = gtk_icon_list_put(iconlist, x, y, pixmap, mask, label, data);
  return item;
}

static GtkIconListItem *
gtk_icon_list_put (GtkIconList *iconlist, 
                   guint x, guint y, 
                   GdkPixmap *pixmap,
                   GdkBitmap *mask,
                   const gchar *label,
                   gpointer data)
{
  GtkIconListItem *icon;
  GtkIconListItem *active_icon;
  GtkWidget *widget;
  GtkRequisition req, req1, req2;
  gint text_width;
  gint width, height, old_width, old_height;
  GtkAllocation alloc;
  GtkAllocation allocation;
  GtkRequisition pixmap_requisition;
  GtkRequisition entry_requisition;


  widget = GTK_WIDGET(iconlist);

  gtk_widget_get_allocation(GTK_WIDGET(iconlist), &allocation);
  old_width = width = allocation.width;
  old_height = height = allocation.height;

  active_icon = iconlist->active_icon;
  gtk_icon_list_set_active_icon(iconlist, NULL);

  icon = g_new(GtkIconListItem, 1);
  icon->x = x;
  icon->y = y;
  icon->state = GTK_STATE_NORMAL;
  icon->label = NULL;
  icon->entry_label = NULL;
  if(label) icon->label = g_strdup(label);
  icon->entry = gtk_item_entry_new();
  icon->pixmap = gtk_image_new_from_pixmap(pixmap, mask);
  icon->link = data;

  GTK_ITEM_ENTRY(icon->entry)->text_max_size = iconlist->text_space; 
  item_size_request(iconlist, icon, &req);
  gtk_widget_get_requisition(icon->pixmap, &pixmap_requisition);
  req1 = pixmap_requisition;
  gtk_widget_get_requisition(icon->entry, &entry_requisition);
  req2 = entry_requisition;
  req2.width = iconlist->text_space; 

  req1.width += 2*iconlist->icon_border;
  req1.height += 2*iconlist->icon_border;
  if(iconlist->mode == GTK_ICON_LIST_TEXT_BELOW){
     req1.width = MAX(req1.width, req.width);
  }

  if(iconlist->mode == GTK_ICON_LIST_ICON) 
          req2.width = req2.height = 0;
  else
          set_labels(iconlist, icon, label);

  text_width = 0;
  if(label) 
  {
	text_width = STRING_WIDTH(icon->entry, 
			gtk_widget_get_style(icon->entry)->font_desc, label);
  }

  gtk_fixed_put(GTK_FIXED(iconlist), icon->pixmap, 
                 x + req1.width/2 - pixmap_requisition.width/2, 
                 y + iconlist->icon_border);

  alloc.x = x + req1.width/2 - pixmap_requisition.width/2; 
  alloc.y = y + iconlist->icon_border; 
  alloc.width =  req1.width;
  alloc.height =  req1.height;
  gtk_widget_size_allocate(icon->pixmap, &alloc);

  switch(iconlist->mode){
   case GTK_ICON_LIST_TEXT_BELOW:
        gtk_item_entry_set_text(GTK_ITEM_ENTRY(icon->entry), icon->entry_label, 
                                GTK_JUSTIFY_CENTER);
  	gtk_fixed_put(GTK_FIXED(iconlist), icon->entry, 
        	       x - req2.width/2 + req1.width/2, 
                       y + req1.height + iconlist->icon_border);
        alloc.x = x - req2.width/2 + req1.width/2; 
        alloc.y = y + req1.height + iconlist->icon_border;
        alloc.width = req2.width;
        alloc.height = req2.height;
        gtk_widget_size_allocate(icon->entry, &alloc);

        if(y + req1.height + iconlist->icon_border + req2.height > height) 
           height += req1.height + iconlist->icon_border + req2.height;
        break;
   case GTK_ICON_LIST_TEXT_RIGHT:
        gtk_item_entry_set_text(GTK_ITEM_ENTRY(icon->entry), icon->entry_label, 
                                GTK_JUSTIFY_LEFT);
  	gtk_fixed_put(GTK_FIXED(iconlist), icon->entry, 
        	       x + req1.width + iconlist->icon_border, 
                       y + req1.height/2 - req2.height/2); 
        alloc.x = x + req1.width + iconlist->icon_border; 
        alloc.y = y + req1.height/2 - req2.height/2; 
        alloc.width = req2.width;
        alloc.height = req2.height;
        gtk_widget_size_allocate(icon->entry, &alloc);

        if(x + req1.width + iconlist->icon_border + text_width > width) 
            width += req1.width + iconlist->icon_border + text_width;
	break;
   case GTK_ICON_LIST_ICON:
   default: ;
  }

  if(gtk_widget_get_realized(GTK_WIDGET(iconlist)))
    if(iconlist->mode != GTK_ICON_LIST_ICON){
      GtkStyle *style = gtk_style_copy(gtk_widget_get_style(icon->entry));
      style->bg[GTK_STATE_ACTIVE] = iconlist->background;
      style->bg[GTK_STATE_NORMAL] = iconlist->background;
      gtk_widget_set_style(icon->entry, style);
      gtk_style_unref(style);
      gtk_widget_show(icon->entry);
    }

  gtk_widget_show(icon->pixmap);

  if(iconlist->compare_func)
    iconlist->icons = g_list_insert_sorted(iconlist->icons, icon, iconlist->compare_func);
  else
    iconlist->icons = g_list_append(iconlist->icons, icon);

  iconlist->num_icons++;

  if(gtk_widget_get_realized(GTK_WIDGET(iconlist)))
                reorder_icons(iconlist);

  gtk_editable_set_editable(GTK_EDITABLE(icon->entry), FALSE);

  g_signal_connect(GTK_OBJECT(icon->entry), "key_press_event",
                    (void *)icon_key_press, iconlist);
  g_signal_connect(GTK_OBJECT(icon->entry), "button_press_event", 
                     (void *)entry_in, iconlist);
  g_signal_connect(GTK_OBJECT(icon->entry), "changed", 
                     (void *)entry_changed, iconlist);

  gtk_icon_list_set_active_icon(iconlist, active_icon);
  return icon;
}

static void 
set_labels(GtkIconList *iconlist, GtkIconListItem *icon, const gchar *label)
{
  gint text_width;
  gint point_width;
  gint max_width;
  gchar *entry_label = NULL;
  gint n, space;

  if(!label) return;

  entry_label = (gchar *)g_malloc(strlen(label) + 5);
  entry_label[0] = label[0];
  entry_label[1] = '\0';

  text_width = STRING_WIDTH(icon->entry, 
			    gtk_widget_get_style(icon->entry)->font_desc, 
			    label);
  point_width = STRING_WIDTH(icon->entry, 
			    gtk_widget_get_style(icon->entry)->font_desc, 
			    "X");

  max_width = iconlist->text_space;

  for(n = 0; n < strlen(label); n++){
     space = strlen(label) - n + 1;
     if(space > 3 && 
        STRING_WIDTH(icon->entry, gtk_widget_get_style(icon->entry)->font_desc,
						 entry_label) +
        					   3 * point_width > max_width) 
               break;
     entry_label[n] = label[n];
     entry_label[n + 1] = '\0';
  }


  if(strlen(entry_label) < strlen(label))
      sprintf(entry_label,"%s...", entry_label);

  icon->entry_label = g_strdup(entry_label);

  g_free(entry_label);
}

static gint
icon_key_press(GtkWidget *widget, GdkEventKey *key, gpointer data)
{
  GtkIconList *iconlist;

  iconlist = GTK_ICON_LIST(data);
  if(key->keyval != GDK_KEY_Return) return FALSE;

  if(iconlist->active_icon)
          select_icon(iconlist, iconlist->active_icon, NULL);

  return FALSE;
}

static void
item_size_request(GtkIconList *iconlist, 
                  GtkIconListItem *item,
                  GtkRequisition *requisition)
{
  GtkRequisition req2;

  gtk_widget_size_request(item->entry, &req2);
  req2.width = iconlist->text_space;

  gtk_widget_size_request(item->pixmap, requisition);
  requisition->width = MAX(iconlist->icon_width, requisition->width);
  requisition->width += 2*iconlist->icon_border;
  requisition->height += 2*iconlist->icon_border;

  switch(iconlist->mode){
   case GTK_ICON_LIST_TEXT_BELOW:
        requisition->height += req2.height;
        requisition->width = MAX(requisition->width, req2.width);
        break;
   case GTK_ICON_LIST_TEXT_RIGHT:
        requisition->width += req2.width;
        break;
   case GTK_ICON_LIST_ICON:
   default: ;
  }

}

/**
 * gtk_icon_list_set_editable:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @editable: TUE or FALSE
 *
 * Set if the user can edit the text in the editable widget or not.
 */ 		  
void
gtk_icon_list_set_editable (GtkIconList *iconlist, gboolean editable)
{
  GList *icons;
  GtkIconListItem *item;
  
  icons = iconlist->icons;
  while(icons){
    item = (GtkIconListItem *) icons->data;
    gtk_editable_set_editable(GTK_EDITABLE(item->entry), editable);
    icons = icons->next;
  }

  iconlist->is_editable = editable;
} 

/**
 * gtk_icon_list_get_nth:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @n: icon index number.
 *
 * Get nth icon from the icon list.
 *  
 * Returns: a #GtkIconListItem
 */ 	
GtkIconListItem *
gtk_icon_list_get_nth(GtkIconList *iconlist, guint n)
{
  return (GtkIconListItem *)g_list_nth_data(iconlist->icons, n);
}

/**
 * gtk_icon_list_get_index:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @item: a #GtkIconListItem
 *
 * Get the index number of the icon specified by its #GtkIconListItem pointer.
 *  
 * Returns: icon index number
 */ 	
gint
gtk_icon_list_get_index(GtkIconList *iconlist, GtkIconListItem *item)
{
  GList *icons;
  GtkIconListItem *icon;
  gint n = 0;

  if(item == NULL) return -1;
 
  icons = iconlist->icons;
  while(icons){
    n++;
    icon = (GtkIconListItem *) icons->data;
    if(item == icon) break;
    icons = icons->next;
  }

  if(icons) return n;

  return -1;
}

static void
remove_from_fixed(GtkIconList *iconlist, GtkWidget *widget)
{
  GtkFixed *fixed;
  GList *children;

  fixed = GTK_FIXED(iconlist);
  children = fixed->children;
  while(children){
    GtkFixedChild *child;

    child = children->data;

    if(child->widget == widget){
      gtk_widget_unparent(widget);
      fixed->children = g_list_remove_link (fixed->children, children);
      g_list_free (children);
      g_free (child);

      break;
    }

    children = children->next;
  }
}

static void
pixmap_destroy( GtkImage* pixmap)
{

  /* release pixmap */
  if (pixmap){
    GdkPixmap* pm = NULL;
    GdkBitmap* bm = NULL;

    gtk_image_get_pixmap(pixmap, &pm, &bm);

    /* HB: i don't know enough about Gtk+ to call this a design flaw, but it
     * appears the pixmaps need to be destroyed by hand ...
     */
    if (pm) gdk_pixmap_unref(pm);
    if (bm) gdk_pixmap_unref(bm);
  }
}
  
/**
 * gtk_icon_list_remove:
 * @iconlist: a #GtkIconList widget created with gtk_icon_list_new().
 * @item: a #GtkIconListItem
 *
 * Remove the icon from the iconlist. 
 */ 	
void
gtk_icon_list_remove (GtkIconList *iconlist, GtkIconListItem *item)
{
  GList *icons;
  GtkIconListItem *icon = 0;

  if(item == NULL) return;
 
  icons = iconlist->icons;
  while(icons){
    icon = (GtkIconListItem *) icons->data;
    if(item == icon) break;
    icons = icons->next;
  }

  if(icons){
     if(icon->state == GTK_STATE_SELECTED) unselect_icon(iconlist, icon, NULL);
     if(icon == iconlist->active_icon) deactivate_entry(iconlist);
     pixmap_destroy((GtkImage*)icon->pixmap);
     if(icon->entry && iconlist->mode != GTK_ICON_LIST_ICON){
       remove_from_fixed(iconlist, icon->entry);
       icon->entry = NULL;
     }
     if(icon->pixmap){
       remove_from_fixed(iconlist, icon->pixmap);
       icon->pixmap = NULL;
     }
     if(icon->label){
        g_free(icon->label);
        icon->label = NULL;
     }
     if(icon->entry_label){
        g_free(icon->entry_label);
        icon->entry_label = NULL;
     }

     g_free(icon);
     iconlist->icons = g_list_remove_link(iconlist->icons, icons);
     g_list_free_1(icons);
     iconlist->num_icons--;
  }

  if(iconlist->num_icons == 0){
      iconlist->icons = NULL;
      iconlist->selection = NULL;
  }
}

/**
 * gtk_icon_list_remove_nth:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @n: icon index number
 *
 * Remove nth icon from the iconlist. 
 */ 	
void
gtk_icon_list_remove_nth (GtkIconList *iconlist, guint n)
{
  GtkIconListItem *item;

  item = gtk_icon_list_get_nth(iconlist, n);
  gtk_icon_list_remove(iconlist, item);
}

/**
 * gtk_icon_list_clear:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Clear the icon list widget. 
 */ 	
void
gtk_icon_list_clear(GtkIconList *iconlist)
{
  GList *icons;
  GtkIconListItem *icon;

  if(iconlist->num_icons == 0) return;
  if(!deactivate_entry(iconlist)) return;

  unselect_all(iconlist);

  icons = iconlist->icons;

  while(icons){
    icon = (GtkIconListItem *) icons->data;
    pixmap_destroy((GtkImage*)icon->pixmap);
    if(icon->entry && iconlist->mode != GTK_ICON_LIST_ICON){
      remove_from_fixed(iconlist, icon->entry);
      icon->entry = NULL;
    }
    if(icon->pixmap){
      gtk_widget_hide(icon->pixmap);
      remove_from_fixed(iconlist, icon->pixmap);
      icon->pixmap = NULL;
    }
    if(icon->label){
        g_free(icon->label);
        icon->label = NULL;
    }
    if(icon->entry_label){
        g_free(icon->entry_label);
        icon->entry_label = NULL;
    }

    g_free(icon);
    icon = NULL;

    iconlist->icons = g_list_remove_link(iconlist->icons, icons);
    g_list_free_1(icons);
    icons = iconlist->icons;
  }

  iconlist->icons = NULL;
  iconlist->selection = NULL;
  iconlist->active_icon = NULL;
  iconlist->num_icons = 0;
}    

/**
 * gtk_icon_list_link:
 * @item: #GtkIconListItem
 * @data: a gpointer to some data
 *
 * Add a gpointer link to a icon from the iconlist.
 */ 	
void
gtk_icon_list_link(GtkIconListItem *item, gpointer data)
{
  item->link = data;
} 

/**
 * gtk_icon_list_get_link:
 * @item: a #GtkIconListItem 
 *
 * Add a gpointer link to a icon from the iconlist.
 *  
 * Returns: (transfer none) a gpointer to some data sed with 
 * gtk_icon_list_link(). 
 */ 	
gpointer
gtk_icon_list_get_link(GtkIconListItem *item)
{
  return item->link;
}

/**
 * gtk_icon_list_get_icon_from_link:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @data: a gpointer to some data
 *
 * Gets the icon pointer from the link attached to it. 
 *  
 * Returns: a #GtkIconListItem
 */ 	
GtkIconListItem *
gtk_icon_list_get_icon_from_link(GtkIconList *iconlist, gpointer data)
{
  GList *icons;
  GtkIconListItem *item;

  icons = iconlist->icons;
  while(icons){
    item = (GtkIconListItem *) icons->data;
    if(data == item->link) return item;
    icons = icons->next;
  }

  return NULL;
}

/**
 * gtk_icon_list_get_entry:
 * @item: a #GtkIconListItem
 *
 * Get the item->entry.
 * 
 * Returns: (transfer none) a #GtkWidget
 */ 	
GtkWidget *
gtk_icon_list_get_entry(GtkIconListItem *item)
{
  return item->entry;
}

/**
 * gtk_icon_list_get_pixmap:
 * @item: a #GtkIconListItem
 *
 * Get the icon pixmap.
 *  
 * Returns: (transfer none) a #GtkWidget
 */ 	
GtkWidget *
gtk_icon_list_get_pixmap(GtkIconListItem *item)
{
  return item->pixmap;
}

/**
 * gtk_icon_list_set_pixmap:
 * @item: a #GtkIconListItem
 * @pixmap: a #GdkPixmap
 * @bitmap_mask:  a #GdkBitmap
 *
 * Set the icon pixmap.
 */ 	
void
gtk_icon_list_set_pixmap(GtkIconListItem *item, 
			 GdkPixmap *pixmap, 
			 GdkBitmap *bitmap_mask)
{
  gint x, y;
  GtkAllocation allocation;

  GtkWidget *widget = gtk_widget_get_parent(item->pixmap);

  x = item->x;
  y = item->y;
  
  gtk_container_remove(GTK_CONTAINER(widget), item->pixmap);
  gtk_widget_get_allocation(item->pixmap, &allocation);
  x = allocation.x;
  y = allocation.y;

  item->pixmap = gtk_image_new_from_pixmap(pixmap, bitmap_mask);
  gtk_widget_show(item->pixmap);
  gtk_fixed_put(GTK_FIXED(widget), item->pixmap, x, y);
}

/**
 * gtk_icon_list_set_label:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @item: a #GtkIconListItem
 * @label: icon label
 *
 * Set the icon label.
 */ 	
void 
gtk_icon_list_set_label(GtkIconList *iconlist, GtkIconListItem *item, const gchar *label)
{
  if(item->label){
      g_free(item->label);
      item->label = NULL;
  }
  if(item->entry_label){
      g_free(item->entry_label);
      item->entry_label = NULL;
  }
  if(label) item->label = g_strdup(label);
  gtk_entry_set_text(GTK_ENTRY(item->entry), label);
  set_labels(iconlist, item, label);
}
 
/**********************************
 * gtk_icon_list_set_icon_width
 * gtk_icon_list_set_row_spacing
 * gtk_icon_list_set_col_spacing
 * gtk_icon_list_set_text_space
 * gtk_icon_list_icon_border
 **********************************/

/**
 * gtk_icon_list_set_icon_width:
 * @iconlist: a #GtkIconList widget created with gtk_icon_list_new().
 * @width: icon width in pixels.
 *
 * Set the icon width.
 */ 	
void
gtk_icon_list_set_icon_width(GtkIconList *iconlist, guint width)
{
  iconlist->icon_width = width;
  reorder_icons(iconlist);
}

/**
 * gtk_icon_list_set_icon_border:
 * @iconlist: a #GtkIconList widget created with gtk_icon_list_new().
 * @border: icon border in pixels.
 *
 * Set the icon border.
 */ 	
void
gtk_icon_list_set_icon_border(GtkIconList *iconlist, guint border)
{
  iconlist->icon_border = border;
  reorder_icons(iconlist);
}

/**
 * gtk_icon_list_set_row_spacing:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @spacing: spacing between rows in pixels.
 *
 * Set the icon spacing between rows.
 */ 	
void
gtk_icon_list_set_row_spacing(GtkIconList *iconlist, guint spacing)
{
  iconlist->row_spacing = spacing;
  reorder_icons(iconlist);
}

/**
 * gtk_icon_list_set_col_spacing:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @spacing: spacing  between columns in pixels.
 *
 * Set the icon spacing between columns.
 */ 	
void
gtk_icon_list_set_col_spacing(GtkIconList *iconlist, guint spacing)
{
  iconlist->col_spacing = spacing;
  reorder_icons(iconlist);
}


/**********************************
 * gtk_icon_list_set_selection_mode
 * gtk_icon_list_select_icon
 * gtk_icon_list_unselect_icon
 * gtk_icon_list_unselect_all
 **********************************/

/**
 * gtk_icon_list_set_selection_mode:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @mode: GTK_ICON_LIST_ICON, GTK_ICON_LIST_TEXT_RIGHT, GTK_ICON_LIST_TEXT_BELOW
 *
 * Set the icon selection mode.
 */ 	
void
gtk_icon_list_set_selection_mode(GtkIconList *iconlist, gint mode)
{
  iconlist->selection_mode = mode;
}

/**
 * gtk_icon_list_select_icon:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @item: a #GtkIconListItem
 *
 * Select the icon specified by pointer.
 */ 	
void
gtk_icon_list_select_icon(GtkIconList *iconlist, GtkIconListItem *item)
{
  select_icon(iconlist, item, NULL);
}

/**
 * gtk_icon_list_unselect_icon:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @item: a #GtkIconListItem
 *
 * Unselect the icon specified by pointer.
 */ 	
void
gtk_icon_list_unselect_icon(GtkIconList *iconlist, GtkIconListItem *item)
{
  unselect_icon(iconlist, item, NULL);
}

/**
 * gtk_icon_list_unselect_all:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Unselect all icons.
 */ 	
void
gtk_icon_list_unselect_all(GtkIconList *iconlist)
{
  unselect_all(iconlist);
}

/**
 * gtk_icon_list_set_active_icon:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 * @icon: a #GtkIconListItem
 *
 * Set an active icon in the icon list.
 */ 	
void
gtk_icon_list_set_active_icon(GtkIconList *iconlist, GtkIconListItem *icon)
{
  if(!icon){
    deactivate_entry(iconlist);
    unselect_all(iconlist);
    return;
  }

  if(icon->entry){
    icon->state = GTK_STATE_SELECTED;
    entry_in(icon->entry, NULL, iconlist);
    gtk_widget_grab_focus(icon->entry);
  }
}

/**
 * gtk_icon_list_is_editable:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the editable status of icon list.
 *  
 * Returns: TRUE or FALSE
 */ 	
gboolean
gtk_icon_list_is_editable       (GtkIconList *iconlist)
{
  return (iconlist->is_editable);
}

/**
 * gtk_icon_list_get_row_spacing:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the spacing between rows.
 *  
 * Returns: spacing between rows in pixels.
 */ 	
guint
gtk_icon_list_get_row_spacing       (GtkIconList *iconlist)
{
  return(iconlist->row_spacing);
}

/**
 * gtk_icon_list_get_col_spacing:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the spacing between columns.
 *  
 * Returns: spacing between columns in pixels.
 */ 	
guint
gtk_icon_list_get_col_spacing       (GtkIconList *iconlist)
{
  return(iconlist->col_spacing);
}

/**
 * gtk_icon_list_get_text_space:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the text maximum size in pixels.
 *  
 * Returns: text maximum size distance in pixels.
 */ 	
guint
gtk_icon_list_get_text_space       (GtkIconList *iconlist)
{
  return(iconlist->text_space);
}

/**
 * gtk_icon_list_get_icon_border:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the icon border width.
 *  
 * Returns: icon border width in pixels.
 */ 	
guint
gtk_icon_list_get_icon_border       (GtkIconList *iconlist)
{
  return(iconlist->icon_border);
}

/**
 * gtk_icon_list_get_icon_width:
 * @iconlist: #GtkIconList widget created with gtk_icon_list_new().
 *
 * Get the icon width.
 *  
 * Returns: icon width in pixels.
 */ 	
guint
gtk_icon_list_get_icon_width       (GtkIconList *iconlist)
{
  return(iconlist->icon_width);
}

