/* gtkcheckitem - widget for gtk+
 * Copyright (C) 1999-2001 Adrian E. Feiguin 
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

/**
 * SECTION: gtkcheckitem
 * @short_description: Check item widget for Gtk
 *
 * It is a GtkCheckButton hack with the look and feel of the Redmond95 theme.
 */

/**
 * GtkCheckItem:
 *
 * The GtkCheckItem struct contains only private data.
 * It should only be accessed through the functions described below.
 */


#include "gtkextra-compat.h"
#include "gtkcheckitem.h"


#define INDICATOR_SIZE     14
#define INDICATOR_SPACING  2



static void gtk_check_item_class_init          (GtkCheckItemClass *klass);
static void gtk_check_item_init                (GtkCheckItem      *check_item);
static void gtk_check_item_size_request        (GtkWidget           *widget,
						GtkRequisition      *requisition);
static void gtk_check_item_size_allocate       (GtkWidget           *widget,
						GtkAllocation       *allocation);
static gboolean gtk_check_item_expose          (GtkWidget           *widget,
						GdkEventExpose      *event);
static void gtk_check_item_paint               (GtkWidget           *widget,
						GdkRectangle        *area);
static void gtk_check_item_draw_indicator      (GtkCheckItem      *check_item,
						GdkRectangle        *area);
static void gtk_real_check_item_draw_indicator (GtkCheckItem      *check_item,
						GdkRectangle        *area);

static GtkToggleButtonClass *parent_class = NULL;


GType
gtk_check_item_get_type (void)
{
  static GType check_item_type = 0;
  
  if (!check_item_type)
    {
      check_item_type = g_type_register_static_simple(
		gtk_toggle_button_get_type (),
		"GtkCheckItem",
		sizeof (GtkCheckItemClass),
	        (GClassInitFunc) gtk_check_item_class_init,
		sizeof (GtkCheckItem),
		(GInstanceInitFunc) gtk_check_item_init,
		0);
    }
  
  return check_item_type;
}

static void
gtk_check_item_class_init (GtkCheckItemClass *klass)
{
  GtkWidgetClass *widget_class;
  
  widget_class = (GtkWidgetClass*) klass;
  parent_class = g_type_class_ref (gtk_toggle_button_get_type ());
  
  widget_class->size_request = gtk_check_item_size_request;
  widget_class->size_allocate = gtk_check_item_size_allocate;
  widget_class->expose_event = gtk_check_item_expose;
  
  klass->indicator_size = INDICATOR_SIZE;
  klass->indicator_spacing = INDICATOR_SPACING;
   /**
   * GtkCheckItem::draw_indicator:
   * @check_item: the #GtkCheckItem object that received the signal
   * @area: the #GdkRectangle area of the check item.
   *
   * Emmited when the check item(check button) is pressed.
   */ 
  klass->draw_indicator = gtk_real_check_item_draw_indicator;
}

static void
gtk_check_item_init (GtkCheckItem *check_item)
{
  gtk_widget_set_has_window(GTK_WIDGET(check_item), FALSE);
  gtk_widget_set_receives_default(GTK_WIDGET(check_item), FALSE);
  GTK_TOGGLE_BUTTON (check_item)->draw_indicator = TRUE;
}

GtkWidget*
gtk_check_item_new (void)
{
  return gtk_widget_new (G_TYPE_CHECK_ITEM, NULL);
}

/**
 * gtk_check_item_new_with_label:
 * @label: text near the check item
 *
 * Create a #GtkCheckItem widget with a specified label.
 *  
 * Returns: the newly-created #GtkCheckItem.
 */
GtkWidget*
gtk_check_item_new_with_label (const gchar *label)
{
  GtkWidget *check_item;
  
  check_item = gtk_check_item_new ();
  gtk_check_item_construct_with_label(GTK_CHECK_ITEM(check_item), label);
  
  return check_item;
}

/**
 * gtk_check_item_construct_with_label:
 * @check_item: a #GtkCheckItem widget
 * @label: text near the check item
 *
 * Initialize a #GtkCheckItem widget with a specified label.
 */
void
gtk_check_item_construct_with_label (GtkCheckItem* check_item, const gchar *label)
{
  GtkWidget *label_widget;
  
  label_widget = gtk_label_new (label);
  gtk_misc_set_alignment (GTK_MISC (label_widget), 0.0, 0.5);
  
  gtk_container_add (GTK_CONTAINER (check_item), label_widget);
  gtk_widget_show (label_widget);
}

/* This should only be called when toggle_button->draw_indicator
 * is true.
 */
static void
gtk_check_item_paint (GtkWidget    *widget,
			GdkRectangle *area)
{
  GtkCheckItem *check_item;
  GtkAllocation allocation;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_CHECK_ITEM (widget));
  
  check_item = GTK_CHECK_ITEM (widget);
  
  gtk_widget_get_allocation(widget, &allocation);
  if (gtk_widget_is_drawable (widget))
    {
      gint border_width;
	  
      gtk_check_item_draw_indicator (check_item, area);
      
      border_width = gtk_container_get_border_width(GTK_CONTAINER (widget));
      if (gtk_widget_has_focus (widget))
	gtk_paint_focus (gtk_widget_get_style(widget), 
			gtk_widget_get_window(widget),
			gtk_widget_get_state(widget), area, widget, "checkitem",
			border_width + allocation.x,
			border_width + allocation.y,
			allocation.width - 2 * border_width - 1,
			allocation.height - 2 * border_width - 1);
    }
}

static void
gtk_check_item_size_request (GtkWidget      *widget,
			       GtkRequisition *requisition)
{
  GtkToggleButton *toggle_button;
  gint temp;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_CHECK_ITEM (widget));
  g_return_if_fail (requisition != NULL);
  
  toggle_button = GTK_TOGGLE_BUTTON (widget);
  
  if (GTK_WIDGET_CLASS (parent_class)->size_request)
    (* GTK_WIDGET_CLASS (parent_class)->size_request) (widget, requisition);
  
  if (toggle_button->draw_indicator)
    {
      requisition->width += (GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size +
			     GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_spacing * 3 + 2);
      
      temp = (GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size +
	      GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_spacing * 2);
      requisition->height = MAX (requisition->height, temp) + 2;
    }
}

static void
gtk_check_item_size_allocate (GtkWidget     *widget,
				GtkAllocation *allocation)
{
  GtkCheckItem *check_item;
  GtkToggleButton *toggle_button;
  GtkButton *button;
  GtkAllocation child_allocation;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_CHECK_ITEM (widget));
  g_return_if_fail (allocation != NULL);
  
  check_item = GTK_CHECK_ITEM (widget);
  toggle_button = GTK_TOGGLE_BUTTON (widget);
  button = GTK_BUTTON (widget);

  if (toggle_button->draw_indicator)
    {
      gtk_widget_set_allocation(widget, allocation);
      if (gtk_widget_get_realized (widget))
	gdk_window_move_resize (button->event_window,
				allocation->x, allocation->y,
				allocation->width, allocation->height);
      
      
      if (gtk_bin_get_child(GTK_BIN (button))&& 
		gtk_widget_get_visible (gtk_bin_get_child(GTK_BIN (button))))
	{
	  guint border_width = gtk_container_get_border_width(GTK_CONTAINER (widget));

	  child_allocation.x = (border_width +
				GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size +
				GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_spacing * 3 + 1 +
				allocation->x);
	  child_allocation.y = border_width + 1 + allocation->y;
	  child_allocation.width = MAX (1, allocation->width - 
				(border_width +
				 GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size +
				 GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_spacing * 3 + 1)  -
				border_width - 1);
	  child_allocation.height = MAX (1, allocation->height - (border_width + 1) * 2);
	  
	  gtk_widget_size_allocate (gtk_bin_get_child(GTK_BIN (button)), 
				&child_allocation);
	}
    }
  else
    {
      if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
	(* GTK_WIDGET_CLASS (parent_class)->size_allocate) (widget, allocation);
    }
}

static gboolean
gtk_check_item_expose (GtkWidget      *widget,
			 GdkEventExpose *event)
{
  GtkCheckItem *check_item;
  GtkToggleButton *toggle_button;
  GtkBin *bin;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_CHECK_ITEM (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  
  check_item = GTK_CHECK_ITEM (widget);
  toggle_button = GTK_TOGGLE_BUTTON (widget);
  bin = GTK_BIN (widget);
  
  if (gtk_widget_is_drawable (widget))
    {
      if (toggle_button->draw_indicator)
        {
          gtk_check_item_paint (widget, &event->area);
                                                                                
          if (gtk_bin_get_child(bin))
            gtk_container_propagate_expose (GTK_CONTAINER (widget),
                                            gtk_bin_get_child(bin),
                                            event);
        }
      else if (GTK_WIDGET_CLASS (parent_class)->expose_event)
        (* GTK_WIDGET_CLASS (parent_class)->expose_event) (widget, event);
    }
 
  return FALSE;
}



static void
gtk_check_item_draw_indicator (GtkCheckItem *check_item,
				 GdkRectangle   *area)
{
  GtkCheckItemClass *klass;
  
  g_return_if_fail (check_item != NULL);
  g_return_if_fail (GTK_IS_CHECK_ITEM (check_item));
  
  klass = GTK_CHECK_ITEM_GET_CLASS (check_item);
  
  if (klass->draw_indicator)
    (* klass->draw_indicator) (check_item, area);
}

static void
gtk_real_check_item_draw_indicator (GtkCheckItem *check_item,
				      GdkRectangle   *area)
{
  GtkWidget *widget;
  GtkToggleButton *toggle_button;
  GtkStateType state_type;
  GdkRectangle restrict_area;
  GdkRectangle new_area;
  GdkGC *fg_gc = NULL;
  gint width, height;
  gint x, y;
  gint border;
  GdkWindow *window;
  GtkAllocation allocation;
  
  g_return_if_fail (check_item != NULL);
  g_return_if_fail (GTK_IS_CHECK_ITEM (check_item));
  
  widget = GTK_WIDGET (check_item);
  toggle_button = GTK_TOGGLE_BUTTON (check_item);
  gtk_widget_get_allocation(widget, &allocation);

  if (gtk_widget_is_drawable (widget))
    {
      window = gtk_widget_get_window(widget);
      
      state_type = gtk_widget_get_state (widget);
      if (state_type != GTK_STATE_NORMAL &&
	  state_type != GTK_STATE_PRELIGHT)
	state_type = GTK_STATE_NORMAL;
      
      guint border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));

      restrict_area.x = allocation.x + border_width;
      restrict_area.y = allocation.y + border_width;
      restrict_area.width = allocation.width - ( 2 * border_width);
      restrict_area.height = allocation.height - ( 2 * border_width);
      
      if (gdk_rectangle_intersect (area, &restrict_area, &new_area))
	{
	  if (state_type != GTK_STATE_NORMAL)
	    gtk_paint_flat_box (gtk_widget_get_style(widget), 
				window, state_type, 
				GTK_SHADOW_ETCHED_OUT, 
				area, widget, "checkitem",
				new_area.x, new_area.y,
				new_area.width, new_area.height);
	}
      
      x = allocation.x + GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_spacing + border_width;
      y = allocation.y + (allocation.height - GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size) / 2;
      width = GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size;
      height = GTK_CHECK_ITEM_GET_CLASS (widget)->indicator_size;
    
      if(!gtk_bin_get_child(GTK_BIN(widget))){
       x = allocation.x + allocation.width/2 - width/2;
       y = allocation.y + allocation.height/2 - height/2;
      }
 
      if (GTK_TOGGLE_BUTTON (widget)->active)
        {
          state_type = GTK_STATE_ACTIVE;
        }
      else
        {
          state_type = GTK_STATE_NORMAL;
        }
 
      fg_gc = gdk_gc_new(window);
      gdk_gc_set_foreground(fg_gc, &gtk_widget_get_style(widget)->white);

      gdk_draw_rectangle(window,
                         fg_gc,
                         TRUE,
                         x, y, width, height);

      gtk_paint_shadow (gtk_widget_get_style(widget), window, GTK_STATE_NORMAL,
                       GTK_SHADOW_IN, 
			NULL, NULL, NULL,
			x, y, width, height);

      if(state_type == GTK_STATE_ACTIVE){
         GdkPoint points[3];
         border = gtk_widget_get_style(widget)->xthickness;
         gdk_gc_set_foreground(fg_gc, &gtk_widget_get_style(widget)->black);
         x += border;
         y += border;

         points[0].x = x+1; 
         points[0].y = y+6; 
         points[1].x = x+3; 
         points[1].y = y+height-2*border-2; 
         points[2].x = x+width-2*border-2; 
         points[2].y = y+3; 

         gdk_draw_lines(window, fg_gc, points, 3);

         points[0].x = x+1; 
         points[0].y = y+5; 
         points[1].x = x+3; 
         points[1].y = y+height-2*border-3; 
         points[2].x = x+width-2*border-2; 
         points[2].y = y+2; 

         gdk_draw_lines(window, fg_gc, points, 3);

         points[0].x = x+1; 
         points[0].y = y+4; 
         points[1].x = x+3; 
         points[1].y = y+height-2*border-4; 
         points[2].x = x+width-2*border-2; 
         points[2].y = y+1; 

         gdk_draw_lines(window, fg_gc, points, 3);

      }

    }

    gdk_gc_unref(fg_gc);
}
