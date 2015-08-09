/* gtkplotcanvas - gtkplot canvas widget for gtk+
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gtkextra-compat.h"
#include "gtkplotcairo.h"
#include "gtkplotcanvas.h"
#include "gtkplotgdk.h"
#include "gtkplotps.h"
#include "gtkextra-marshal.h"

/**
 * SECTION: gtkplotcanvas
 * @short_description: 
 *
 * FIXME:: need long description.
 */


#define P_(string) string

#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 150
#define DEFAULT_MARKER_SIZE 6
#define SHADOW_WIDTH 3 
#define GRAPH_MASK    (GDK_EXPOSURE_MASK |              \
                       GDK_POINTER_MOTION_MASK |        \
                       GDK_POINTER_MOTION_HINT_MASK |   \
                       GDK_BUTTON_PRESS_MASK |          \
                       GDK_BUTTON_RELEASE_MASK)

#if !GTK_CHECK_VERSION(2,22,0)
static GdkCursorType
gdk_cursor_get_cursor_type (GdkCursor *cursor)
{
  g_return_val_if_fail (cursor != NULL, GDK_BLANK_CURSOR);
  return cursor->type;
}
#endif

enum {
  ARG_CANVAS_0,
  ARG_CANVAS_FLAGS,
  ARG_CANVAS_WIDTH,
  ARG_CANVAS_HEIGHT,
  ARG_CANVAS_MAGNIFICATION,
  ARG_CANVAS_SHOW_GRID,
  ARG_CANVAS_GRID_STEP,
  ARG_CANVAS_LINE_GRID,
  ARG_CANVAS_COLOR_BG,
  ARG_CANVAS_TRANSPARENT,
};

enum {
  ARG_CHILD_0,
  ARG_CHILD_RX1,
  ARG_CHILD_RY1,
  ARG_CHILD_RX2,
  ARG_CHILD_RY2,
  ARG_CHILD_ALLOCATION,
  ARG_CHILD_MIN_WIDTH,
  ARG_CHILD_MIN_HEIGHT,
  ARG_CHILD_STATE,
  ARG_CHILD_FLAGS,
  ARG_CHILD_SELECTION,
  ARG_CHILD_SELECTION_MODE,
};

static void gtk_plot_canvas_class_init 		(GtkPlotCanvasClass *klass);
static void gtk_plot_canvas_init 		(GtkPlotCanvas *plot_canvas);
static void gtk_plot_canvas_set_property        (GObject *object,
                                                 guint            prop_id,
                                                 const GValue          *value,
                                                 GParamSpec      *pspec);
static void gtk_plot_canvas_get_property        (GObject *object,
                                                 guint            prop_id,
                                                 GValue          *value,
                                                 GParamSpec      *pspec);
static void gtk_plot_canvas_child_class_init 	(GtkPlotCanvasChildClass *klass);
static void gtk_plot_canvas_child_init 		(GtkPlotCanvasChild *child);
static void gtk_plot_canvas_child_set_property  (GObject *object,
                                                 guint            prop_id,
                                                 const GValue          *value,
                                                 GParamSpec      *pspec);
static void gtk_plot_canvas_child_get_property  (GObject *object,
                                                 guint            prop_id,
                                                 GValue          *value,
                                                 GParamSpec      *pspec);
static void gtk_plot_canvas_destroy 		(GtkObject *object);
static void gtk_plot_canvas_map                 (GtkWidget *widget);
static void gtk_plot_canvas_size_request        (GtkWidget *widget,
                                                 GtkRequisition *requisition);
static gint gtk_plot_canvas_motion 		(GtkWidget *widget, 
                                                 GdkEventMotion *event);
static gint gtk_plot_canvas_button_press	(GtkWidget *widget, 
                                                 GdkEventButton *event);
static gint gtk_plot_canvas_key_press		(GtkWidget *widget, 
                                                 GdkEventKey *event);
static gint gtk_plot_canvas_button_release	(GtkWidget *widget, 
                                                 GdkEventButton *event);
static gint gtk_plot_canvas_focus_in		(GtkWidget *widget, 
                                                 GdkEventFocus *event);
static gint gtk_plot_canvas_focus_out		(GtkWidget *widget, 
                                                 GdkEventFocus *event);
static void gtk_plot_canvas_child_size_allocate (GtkPlotCanvas *canvas,
						 GtkPlotCanvasChild *child);
static GtkPlotCanvasPos gtk_plot_canvas_child_button_press 
						(GtkPlotCanvas *canvas,
						 GtkPlotCanvasChild *child,
						 gint x, gint y);
static void gtk_plot_canvas_child_button_release(GtkPlotCanvas *canvas,
                                                 GtkPlotCanvasChild *child);
/* Drawing functions */
static gint gtk_plot_canvas_expose              (GtkWidget *widget, 
                                                 GdkEventExpose *event);
static void gtk_plot_canvas_create_pixmap       (GtkWidget *widget, 
                                                 gint width, gint height);
static void gtk_plot_canvas_child_draw		(GtkPlotCanvas *canvas,
						 GtkPlotCanvasChild *child);
static void gtk_plot_canvas_child_draw_selection(GtkPlotCanvas *canvas, 
						 GtkPlotCanvasChild *child,
                				 GtkAllocation area);
static void draw_selection 			(GtkPlotCanvas *canvas, 
						 GtkPlotCanvasChild *child,
                				 GtkAllocation area);
static void draw_marker				(GtkPlotCanvas *canvas, 
						 GdkGC *gc, gint x, gint y);

static void gtk_plot_canvas_draw_grid		(GtkPlotCanvas *canvas);
static void gtk_plot_canvas_child_draw		(GtkPlotCanvas *canvas,
                           			 GtkPlotCanvasChild *child);
/* Auxiliary functions */
GtkPlotCanvasPos 	possible_selection	(GtkAllocation area, 
						 gint x, gint y);
extern gint roundint                     (gdouble x);


/* Signals */

extern void
_gtkextra_signal_emit(GtkObject *object, guint signal_id, ...);
                                                                                
enum {
        SELECT_ITEM,
        MOVE_ITEM,
        RESIZE_ITEM,
        DELETE_ITEM,
        ADD_ITEM,
        SELECT_REGION,
        CHANGED,
        LAST_SIGNAL
};

typedef gboolean (*GtkPlotCanvasSignal1) (GtkObject *object,
                                          gpointer arg1, 
					  gdouble arg2,
					  gdouble arg3,
                                    	  gpointer user_data);

typedef gboolean (*GtkPlotCanvasSignal2) (GtkObject *object,
                                          gpointer arg1, 
					  gpointer arg2,
                                    	  gpointer user_data);

typedef gboolean (*GtkPlotCanvasSignal3) (GtkObject *object,
                                          gdouble arg1, 
                                          gdouble arg2, 
                                          gdouble arg3, 
                                          gdouble arg4, 
                                    	  gpointer user_data);

static GtkFixedClass *parent_class = NULL;
static guint canvas_signals[LAST_SIGNAL] = {0};

GType
gtk_plot_canvas_get_type (void)
{
  static GType plot_canvas_type = 0;

  if (!plot_canvas_type)
    {
      plot_canvas_type = g_type_register_static_simple (
		gtk_fixed_get_type(),
		"GtkPlotCanvas",
		sizeof (GtkPlotCanvasClass),
		(GClassInitFunc) gtk_plot_canvas_class_init,
		sizeof (GtkPlotCanvas),
		(GInstanceInitFunc) gtk_plot_canvas_init,
		0);
    }
  return plot_canvas_type;
}

GType
gtk_plot_canvas_child_get_type (void)
{
  static GType plot_canvas_child_type = 0;

  if (!plot_canvas_child_type)
    {

      plot_canvas_child_type = g_type_register_static_simple (
		gtk_object_get_type(),
		"GtkPlotCanvasChild",
		sizeof(GtkPlotCanvasChildClass),
		(GClassInitFunc) gtk_plot_canvas_child_class_init,
		sizeof(GtkPlotCanvasChild),
		(GInstanceInitFunc) gtk_plot_canvas_child_init,
		0);
    }
  return plot_canvas_child_type;
}

static void
gtk_plot_canvas_child_class_init (GtkPlotCanvasChildClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  klass->size_allocate = gtk_plot_canvas_child_size_allocate;
  klass->draw = NULL;
  klass->unselect = NULL;
  klass->move = NULL;
  klass->move_resize = NULL;
  klass->draw_selection = draw_selection;
  klass->button_press = gtk_plot_canvas_child_button_press;
  klass->button_release = gtk_plot_canvas_child_button_release;
  klass->set_magnification = NULL;

  gobject_class->get_property = gtk_plot_canvas_child_get_property;
  gobject_class->set_property = gtk_plot_canvas_child_set_property;

  /**
   * GtkPlotCanvasChild:rx1:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_RX1,
  g_param_spec_double ("rx1",
                           P_(""),
                           P_(""),
                           -G_MAXDOUBLE,G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:ry1:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_RY1,
  g_param_spec_double ("ry1",
                           P_(""),
                           P_(""),
                           -G_MAXDOUBLE,G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:rx2:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_RX2,
  g_param_spec_double ("rx2",
                           P_(""),
                           P_(""),
                           -G_MAXDOUBLE,G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:ry2:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_RY2,
  g_param_spec_double ("ry2",
                           P_(""),
                           P_(""),
                           -G_MAXDOUBLE,G_MAXDOUBLE,0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:allocation:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_ALLOCATION,
  g_param_spec_pointer ("allocation",
                           P_(""),
                           P_(""),
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:min_width:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_MIN_WIDTH,
  g_param_spec_int ("min_width",
                           P_(""),
                           P_(""),
                           -1,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:min_height:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_MIN_HEIGHT,
  g_param_spec_int ("min_height",
                           P_(""),
                           P_(""),
                           -1,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:state:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_STATE,
  g_param_spec_int ("state",
                           P_(""),
                           P_(""),
                           0,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:flags:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_FLAGS,
  g_param_spec_int ("flags",
                           P_(""),
                           P_(""),
                           0,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:selection:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_SELECTION,
  g_param_spec_int ("selection",
                           P_(""),
                           P_(""),
                           0,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  /**
   * GtkPlotCanvasChild:selection_mode:
   *
   *
   **/
  g_object_class_install_property (gobject_class,
                           ARG_CHILD_SELECTION_MODE,
  g_param_spec_int ("selection_mode",
                           P_(""),
                           P_(""),
                           0,G_MAXINT,0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

}

static void
gtk_plot_canvas_child_get_property (GObject      *object,
                                    guint            prop_id,
                                    GValue          *value,
                                    GParamSpec      *pspec) 
{
  GtkPlotCanvasChild *child = GTK_PLOT_CANVAS_CHILD(object);

  switch(prop_id){
    case ARG_CHILD_RX1:
      g_value_set_double(value, child->rx1);
      break;
    case ARG_CHILD_RY1:
      g_value_set_double(value, child->ry1);
      break;
    case ARG_CHILD_RX2:
      g_value_set_double(value, child->rx2);
      break;
    case ARG_CHILD_RY2:
      g_value_set_double(value, child->ry2);
      break;
    case ARG_CHILD_ALLOCATION:
      g_value_set_pointer(value, &child->allocation);
      break;
    case ARG_CHILD_MIN_WIDTH:
      g_value_set_int(value, child->min_width);
      break;
    case ARG_CHILD_MIN_HEIGHT:
      g_value_set_int(value, child->min_height);
      break;
    case ARG_CHILD_STATE:
      g_value_set_int(value, child->state);
      break;
    case ARG_CHILD_FLAGS:
      g_value_set_int(value, child->flags);
      break;
    case ARG_CHILD_SELECTION:
      g_value_set_int(value, child->selection);
      break;
    case ARG_CHILD_SELECTION_MODE:
      g_value_set_int(value, child->mode);
      break;
  }
}

static void
gtk_plot_canvas_child_set_property (GObject      *object,
                                    guint            prop_id,
                                    const GValue          *value,
                                    GParamSpec      *pspec) 
{
  GtkPlotCanvasChild *child = GTK_PLOT_CANVAS_CHILD(object);

  switch(prop_id){
    case ARG_CHILD_RX1:
      child->rx1 = g_value_get_double(value);
      break;
    case ARG_CHILD_RY1:
      child->ry1 = g_value_get_double(value); 
      break;
    case ARG_CHILD_RX2:
      child->rx2 = g_value_get_double(value);
      break;
    case ARG_CHILD_RY2:
      child->ry2 = g_value_get_double(value); 
      break;
    case ARG_CHILD_ALLOCATION:
      child->allocation = *((GtkAllocation *)g_value_get_pointer(value));
      break;
    case ARG_CHILD_MIN_WIDTH:
      child->min_width = g_value_get_int(value); 
      break;
    case ARG_CHILD_MIN_HEIGHT:
      child->min_height = g_value_get_int(value); 
      break;
    case ARG_CHILD_STATE:
      child->state = g_value_get_int(value); 
      break;
    case ARG_CHILD_FLAGS:
      child->flags = g_value_get_int(value); 
      break;
    case ARG_CHILD_SELECTION:
      child->selection = g_value_get_int(value); 
      break;
    case ARG_CHILD_SELECTION_MODE:
      child->mode = g_value_get_int(value); 
      break;
  }
}

static void
gtk_plot_canvas_child_init(GtkPlotCanvasChild *child)
{
  child->flags = GTK_PLOT_CANVAS_CHILD_CAN_MOVE | 
                 GTK_PLOT_CANVAS_CHILD_CAN_RESIZE;

  child->min_width = -1;
  child->min_height = -1;

  child->selection = GTK_PLOT_CANVAS_SELECT_MARKERS;
  child->mode = GTK_PLOT_CANVAS_SELECT_CLICK_2;
  child->parent = NULL;
}

static void
gtk_plot_canvas_class_init (GtkPlotCanvasClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkContainerClass *container_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  parent_class = g_type_class_ref (gtk_fixed_get_type ());

  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;
  container_class = (GtkContainerClass *) klass;

  
  /**
   * GtkPlotCanvas::select_item:
   * @canvas:
   * @event:
   * @item:
   * 
   *
   *
   * Return value: 
   */
  canvas_signals[SELECT_ITEM] =
    g_signal_new ("select_item",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkPlotCanvasClass, select_item),
		    NULL, NULL,
                    gtkextra_BOOLEAN__BOXED_BOXED,
                    G_TYPE_BOOLEAN, 2, GDK_TYPE_EVENT, 
                    G_TYPE_PLOT_CANVAS_CHILD);

  /**
   * GtkPlotCanvas::move_item:
   * @canvas:
   * @item:
   * @new_x:
   * @new_y
   * 
   *
   *
   * Return value: 
   */
  canvas_signals[MOVE_ITEM] =
    g_signal_new ("move_item",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkPlotCanvasClass, move_item),
		    NULL, NULL,
                    gtkextra_BOOLEAN__BOXED_DOUBLE_DOUBLE,
                    G_TYPE_BOOLEAN, 3, G_TYPE_PLOT_CANVAS_CHILD, 
                    G_TYPE_DOUBLE,
                    G_TYPE_DOUBLE);

  /**
   * GtkPlotCanvas::resize_item:
   * @canvas:
   * @item:
   * @new_width:
   * @new_height:
   * 
   *
   *
   * Return value: 
   */
  canvas_signals[RESIZE_ITEM] =
    g_signal_new ("resize_item",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkPlotCanvasClass, resize_item),
		    NULL, NULL,
                    gtkextra_BOOLEAN__BOXED_DOUBLE_DOUBLE,
                    G_TYPE_BOOLEAN, 3, G_TYPE_PLOT_CANVAS_CHILD,
                    G_TYPE_DOUBLE,
                    G_TYPE_DOUBLE);

  /**
   * GtkPlotCanvas::add_item:
   * @canvas:
   * @item:
   * 
   *
   */
  canvas_signals[ADD_ITEM] =
    g_signal_new ("add_item",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkPlotCanvasClass, add_item),
		    NULL, NULL,
                    gtkextra_VOID__POINTER,
                    G_TYPE_NONE, 1,  
                    G_TYPE_PLOT_CANVAS_CHILD);

  /**
   * GtkPlotCanvas::delete_item:
   * @canvas:	
   * @item:
   * 
   *
   *
   * Return value: 
   */
  canvas_signals[DELETE_ITEM] =
    g_signal_new ("delete_item",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkPlotCanvasClass, delete_item),
		    NULL, NULL,
                    gtkextra_BOOL__POINTER,
                    G_TYPE_BOOLEAN, 1,  
                    G_TYPE_PLOT_CANVAS_CHILD);

  /**
   * GtkPlotCanvas::select_region:
   * @canvas:
   * @xmin:
   * @ymin:
   * @xmax:
   * @ymax:
   * 
   *
   */
  canvas_signals[SELECT_REGION] =
    g_signal_new ("select_region",
                    G_TYPE_FROM_CLASS(object_class),
                    G_SIGNAL_RUN_LAST,
                    G_STRUCT_OFFSET (GtkPlotCanvasClass, select_region),
		    NULL, NULL,
                    gtkextra_VOID__DOUBLE_DOUBLE_DOUBLE_DOUBLE,
                    G_TYPE_NONE, 4, 
                    G_TYPE_DOUBLE, G_TYPE_DOUBLE,
                    G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  /**
   * GtkPlotCanvas::changed:
   * @canvas:
   * 
   *
   */
  canvas_signals[CHANGED] =
    g_signal_new("changed",
                   G_TYPE_FROM_CLASS(object_class),
                   G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET (GtkPlotCanvasClass, changed),
		   NULL, NULL,
                   gtkextra_VOID__VOID,
                   G_TYPE_NONE, 0);

  object_class->destroy = gtk_plot_canvas_destroy;

  gobject_class->get_property = gtk_plot_canvas_get_property;
  gobject_class->set_property = gtk_plot_canvas_set_property;

  widget_class->map = gtk_plot_canvas_map;
  widget_class->expose_event = gtk_plot_canvas_expose;
  widget_class->size_request = gtk_plot_canvas_size_request;
  widget_class->focus_in_event = gtk_plot_canvas_focus_in;
  widget_class->focus_out_event = gtk_plot_canvas_focus_out;
  widget_class->motion_notify_event = gtk_plot_canvas_motion;
  widget_class->button_press_event = gtk_plot_canvas_button_press;
  widget_class->button_release_event = gtk_plot_canvas_button_release;
  widget_class->key_press_event = gtk_plot_canvas_key_press;

  klass->move_item = NULL;
  klass->resize_item = NULL;
  klass->select_item = NULL;
  klass->delete_item = NULL;
  klass->select_region = NULL;

  GParamSpec *pspec;

  /**
   * GtkPlotCanvas:flags:
   *
   * PlotCanvas flags control selection and DND,
   * see #GtkPlotCanvasFlags.
   **/
  pspec = g_param_spec_int ("flags", P_("Flags"),
      P_("Canvas flags"),
      0,G_MAXINT,0,
      G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_FLAGS, pspec);

  /**
   * GtkPlotCanvas:magnification:
   *
   * PlotCanvas magnification factor
   **/
  pspec = g_param_spec_double ("magnification", P_("Magnification"),
      P_("Canvas magnification"),
      0,G_MAXDOUBLE,1.0,
      G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_MAGNIFICATION, pspec
  );

  /**
   * GtkPlotCanvas:width:
   *
   * PlotCanvas width in pixels.
   **/
  pspec = g_param_spec_int ("width", P_("Width"),
      P_("Canvas width"),
      0,G_MAXINT,1,
      G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_WIDTH, pspec);

  /**
   * GtkPlotCanvas:height:
   *
   * PlotCanvas height in pixels.
   **/
  pspec = g_param_spec_int ("height", P_("Height"),
      P_("Canvas height"),
      0,G_MAXINT,1,
      G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_HEIGHT, pspec);

  /**
   * GtkPlotCanvas:show_grid:
   *
   * Show grid lines.
   **/
  pspec = g_param_spec_boolean ("show_grid", P_("Show grid"),
                           P_("Show grid"),
                           FALSE,
                           G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_SHOW_GRID, pspec);

  /**
   * GtkPlotCanvas:grid_step:
   *
   * Distance between grid lines.
   **/
  pspec = g_param_spec_double ("grid_step", P_("Grid step"),
                           P_("Distance between grid lines"),
                           0,G_MAXDOUBLE,0.0,
                           G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_GRID_STEP, pspec);

  /**
   * GtkPlotCanvas:line_grid:
   *
   * Line grid pattern.
   **/
  pspec = g_param_spec_pointer ("line_grid", P_("Line Grid"),
                           P_("Canvas line grid"),
                           G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_LINE_GRID, pspec);

  /**
   * GtkPlotCanvas:color_bg:
   *
   * Background color.
   **/
  pspec = g_param_spec_pointer ("color_bg", P_("Background"),
                           P_("Background color"),
                           G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_COLOR_BG, pspec);

  /**
   * GtkPlotCanvas:transparent:
   *
   * Canvas transparency
   **/
  pspec = g_param_spec_boolean ("transparent", P_("Transparent"),
                           P_("Canvas transparency"),
                           TRUE,
                           G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, ARG_CANVAS_TRANSPARENT, pspec);
}

static void
gtk_plot_canvas_get_property (GObject      *object,
                              guint            prop_id,
                              GValue          *value,
                              GParamSpec      *pspec) 
{
  GtkPlotCanvas *canvas = GTK_PLOT_CANVAS(object);

  switch(prop_id){
    case ARG_CANVAS_FLAGS:
      g_value_set_int(value, canvas->flags);
      break;
    case ARG_CANVAS_WIDTH:
      g_value_set_int(value, canvas->width);
      break;
    case ARG_CANVAS_HEIGHT:
      g_value_set_int(value, canvas->height);
      break;
    case ARG_CANVAS_MAGNIFICATION:
      g_value_set_double(value, canvas->magnification);
      break;
    case ARG_CANVAS_SHOW_GRID:
      g_value_set_boolean(value, canvas->show_grid);
      break;
    case ARG_CANVAS_GRID_STEP:
      g_value_set_double(value, canvas->grid_step);
      break;
    case ARG_CANVAS_LINE_GRID:
      g_value_set_pointer(value, &canvas->grid);
      break;
    case ARG_CANVAS_COLOR_BG:
      g_value_set_pointer(value, &canvas->background);
      break;
    case ARG_CANVAS_TRANSPARENT:
      g_value_set_boolean(value, canvas->transparent);
      break;
  }
}

static void
gtk_plot_canvas_set_property (GObject      *object,
                              guint            prop_id,
                              const GValue          *value,
                              GParamSpec      *pspec) 
{
  GtkPlotCanvas *canvas;
                                                                                
  canvas = GTK_PLOT_CANVAS (object);
                                                                                
  switch(prop_id){
    case ARG_CANVAS_FLAGS:
      canvas->flags = g_value_get_int(value); 
      break;
    case ARG_CANVAS_WIDTH:
      canvas->width = g_value_get_int(value);
      break;
    case ARG_CANVAS_HEIGHT:
      canvas->height = g_value_get_int(value); 
      break;
    case ARG_CANVAS_MAGNIFICATION:
      canvas->magnification = g_value_get_double(value);
      break;
    case ARG_CANVAS_SHOW_GRID:
      canvas->show_grid = g_value_get_boolean(value); 
      break;
    case ARG_CANVAS_GRID_STEP:
       canvas->grid_step = g_value_get_double(value);
      break;
    case ARG_CANVAS_LINE_GRID:
      canvas->grid = *((GtkPlotLine *)g_value_get_pointer(value));
      break;
    case ARG_CANVAS_COLOR_BG:
      canvas->background = *((GdkColor *)g_value_get_pointer(value)); 
      break;
    case ARG_CANVAS_TRANSPARENT:
      canvas->transparent = g_value_get_boolean(value);
      break;
  }
}

static void
gtk_plot_canvas_init (GtkPlotCanvas *plot_canvas)
{
  GtkWidget *widget;
  GdkColor color;

  widget = GTK_WIDGET(plot_canvas);

  gtk_widget_set_can_focus(GTK_WIDGET(widget), TRUE);

  gdk_color_black(gtk_widget_get_colormap(widget), 
		  &gtk_widget_get_style(widget)->black);
  gdk_color_white(gtk_widget_get_colormap(widget), 
		  &gtk_widget_get_style(widget)->white);

  gtk_widget_set_events (widget, gtk_widget_get_events(widget)|
                         GRAPH_MASK);

  plot_canvas->freeze_count = 0;
  plot_canvas->cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);

  plot_canvas->background = gtk_widget_get_style(widget)->white;
  plot_canvas->transparent = TRUE;

  plot_canvas->flags = 0;
  plot_canvas->state = GTK_STATE_NORMAL;
  plot_canvas->action = GTK_PLOT_CANVAS_ACTION_INACTIVE;
  plot_canvas->magnification = 1.;

  plot_canvas->show_grid = FALSE;
  plot_canvas->grid_step = 20.;
  plot_canvas->grid.line_style = GTK_PLOT_LINE_SOLID;
  plot_canvas->grid.line_width = 0;

  gdk_color_parse("grey90", &color);
  gdk_color_alloc(gdk_colormap_get_system(), &color);
  plot_canvas->grid.color = color;

  plot_canvas->drag_x = plot_canvas->drag_y = 0;
  plot_canvas->pointer_x = plot_canvas->pointer_y = 0;

  plot_canvas->childs = NULL;

  plot_canvas->width = DEFAULT_WIDTH;
  plot_canvas->height = DEFAULT_HEIGHT;
  plot_canvas->pixmap_width = DEFAULT_WIDTH;
  plot_canvas->pixmap_height = DEFAULT_HEIGHT;

  gtk_psfont_init();

  plot_canvas->pc = NULL;
  gtk_plot_canvas_set_pc(plot_canvas, NULL);

  plot_canvas->pixmap = NULL;
}

GtkPlotCanvasFlags gtk_plot_canvas_flags(GtkPlotCanvas *canvas)
{
  g_return_val_if_fail(canvas != NULL, 0);
  return(canvas->flags);
}

void gtk_plot_canvas_set_flags(GtkPlotCanvas *canvas, GtkPlotCanvasFlags flags)
{
  g_return_if_fail(canvas != NULL);
  canvas->flags |= flags;
}

void gtk_plot_canvas_unset_flags(GtkPlotCanvas *canvas, GtkPlotCanvasFlags flags)
{
  g_return_if_fail(canvas != NULL);
  canvas->flags &= ~flags;
}


void
gtk_plot_canvas_set_pc(GtkPlotCanvas *canvas, GtkPlotPC *pc)
{
  if(canvas->pc)
    g_object_unref(GTK_OBJECT(canvas->pc));

  if(!pc){
    canvas->pc = (GtkPlotPC *)gtk_plot_cairo_new(NULL);
    g_object_ref(GTK_OBJECT(canvas->pc));
    gtk_object_sink(GTK_OBJECT(canvas->pc));
  } else {
    canvas->pc = pc;
    g_object_ref(GTK_OBJECT(pc));
    gtk_object_sink(GTK_OBJECT(pc));
  }

  if(canvas->pc && GTK_IS_PLOT_GDK(canvas->pc)){
       GTK_PLOT_GDK(canvas->pc)->drawable = canvas->pixmap;
  }
  gtk_plot_pc_set_viewport(canvas->pc, canvas->pixmap_width, canvas->pixmap_height);
}

/**
 * gtk_plot_canvas_new:
 * @width:
 * @height:
 * @magnification:
 *
 *
 *
 * Return value:
 */
GtkWidget*
gtk_plot_canvas_new (gint width, gint height, gdouble magnification)
{
  GtkPlotCanvas *plot_canvas;

  plot_canvas = g_object_new (gtk_plot_canvas_get_type (), NULL);

  gtk_plot_canvas_construct(GTK_PLOT_CANVAS(plot_canvas),
			    width, height, magnification);

  return GTK_WIDGET (plot_canvas);
}

/**
 * gtk_plot_canvas_construct:
 * @canvas: a #GtkPlotCanvas.
 * @width:
 * @height:
 * @magnification:
 *
 *
 */
void
gtk_plot_canvas_construct(GtkPlotCanvas *canvas,
			  gint width, gint height, gdouble magnification)
{
  gdouble m = magnification;

  canvas->width = width;
  canvas->height = height;
  canvas->pixmap_width = roundint(width * m);
  canvas->pixmap_height = roundint(height * m);
  gtk_plot_canvas_set_magnification(canvas, m);

  gtk_fixed_set_has_window (GTK_FIXED(canvas), TRUE);
}

/**
 * gtk_plot_canvas_freeze:
 * @canvas: a #GtkPlotCanvas.
 *
 *
 */
void
gtk_plot_canvas_freeze(GtkPlotCanvas *canvas)
{
  canvas->freeze_count++;
}

/**
 * gtk_plot_canvas_thaw:
 * @canvas: a #GtkPlotCanvas.
 *
 *
 */
void
gtk_plot_canvas_thaw(GtkPlotCanvas *canvas)
{
  if(canvas->freeze_count == 0) return;
  canvas->freeze_count--;
}

static void
gtk_plot_canvas_destroy (GtkObject *object)
{
  GtkPlotCanvas *plot_canvas;
  GList *list;
  gboolean veto = TRUE;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_PLOT_CANVAS (object));

  plot_canvas = GTK_PLOT_CANVAS (object);
 
  list = plot_canvas->childs;
  while(list){
    g_signal_emit(GTK_OBJECT(plot_canvas), canvas_signals[DELETE_ITEM], 0, 
                    GTK_PLOT_CANVAS_CHILD(list->data), &veto);

    g_object_unref(GTK_OBJECT(list->data));

    plot_canvas->childs = g_list_remove_link(plot_canvas->childs, list);
    g_list_free_1(list);

    list = plot_canvas->childs;
  }

  plot_canvas->childs = NULL;

  if( plot_canvas->cursor ){
     gdk_cursor_destroy(plot_canvas->cursor);
     plot_canvas->cursor = NULL;
  }
  if( plot_canvas->pc ){
     g_object_unref(GTK_OBJECT(plot_canvas->pc));
     plot_canvas->pc = NULL;
  }

/* Added by Jacek Sieka jsieka at memphite.se , from Memphite AB */
  if( plot_canvas->pixmap ){
     gdk_pixmap_unref(plot_canvas->pixmap);
     plot_canvas->pixmap = NULL;
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);

  gtk_psfont_unref();
}

/**
 * gtk_plot_canvas_remove_child:
 * @canvas: a #GtkPlotCanvas.
 * @child: a #GtkPlotCanvasChild
 *
 *
 */
void
gtk_plot_canvas_remove_child(GtkPlotCanvas *canvas, GtkPlotCanvasChild *child)
{
  GList *list;

  gtk_plot_canvas_cancel_action(canvas);

  list = canvas->childs;
  while(list){
   if(list->data == child){
      gboolean veto = TRUE;

      _gtkextra_signal_emit(GTK_OBJECT(canvas), canvas_signals[DELETE_ITEM],
                      child, &veto);

      if(veto){
        child->parent = NULL;
        g_object_unref(GTK_OBJECT(child));
        canvas->childs = g_list_remove_link(canvas->childs, list);
        g_list_free_1(list);
      }
      break;
   }
   list = list->next;
  }
}

/*
static void
gtk_plot_canvas_draw (GtkWidget *widget, GdkRectangle *area)
{
  GtkPlotCanvas *canvas;

  canvas = GTK_PLOT_CANVAS(widget);

  if(!gtk_widget_get_realized(widget)) return;
  if(!canvas->pixmap) return;

  GTK_WIDGET_CLASS(parent_class)->draw(widget, area);

  gtk_plot_canvas_paint(canvas);
  gtk_plot_canvas_refresh(canvas);
}
*/

/**
 * gtk_plot_canvas_paint:
 * @canvas: a #GtkPlotCanvas.
 *
 *
 */
void
gtk_plot_canvas_paint (GtkPlotCanvas *canvas)
{
  GtkWidget *widget;
  GList *childs;

  widget = GTK_WIDGET(canvas);

  if(gtk_widget_get_realized(widget) && !canvas->pixmap) return;
  if(canvas->freeze_count > 0) return;

  if(!gtk_plot_pc_init(canvas->pc)) return;

  gtk_plot_pc_gsave(canvas->pc);

  if(!GTK_IS_PLOT_PS(canvas->pc) || !canvas->transparent){
    if(canvas->transparent){
      GdkColor white;
      gdk_color_white(gtk_widget_get_colormap(GTK_WIDGET(canvas)), &white);
      gtk_plot_pc_set_color(canvas->pc, &white);
    } else
      gtk_plot_pc_set_color(canvas->pc, &canvas->background);
    gtk_plot_pc_draw_rectangle(canvas->pc,
                              TRUE,
                              0,0,canvas->pixmap_width, canvas->pixmap_height);
  }

  gtk_plot_canvas_draw_grid(canvas);

  childs = canvas->childs;
  while(childs)
   {
     GtkPlotCanvasChild *child;

     child = GTK_PLOT_CANVAS_CHILD(childs->data);
     gtk_plot_canvas_child_draw(canvas, child);
     childs = childs->next;
   }

  gtk_plot_pc_grestore(canvas->pc);
  gtk_plot_pc_leave(canvas->pc);
}

/**
 * gtk_plot_canvas_refresh:
 * @canvas: a #GtkPlotCanvas.
 *
 *
 */
void
gtk_plot_canvas_refresh(GtkPlotCanvas *canvas)
{
  GList *children = NULL;
  GtkFixed *fixed = GTK_FIXED(canvas);

  if(!gtk_widget_get_realized(GTK_WIDGET(canvas))) return;
  if(!canvas->pixmap) return;
  if(canvas->freeze_count > 0) return;

  gdk_draw_drawable(gtk_widget_get_window(GTK_WIDGET(canvas)),
              gtk_widget_get_style(GTK_WIDGET(canvas))->fg_gc[GTK_STATE_NORMAL],
              canvas->pixmap,
              0, 0,
              0, 0,
              -1, -1);

  children = fixed->children;
  while (children)
    {
      GtkFixedChild *child;
      child = children->data;
      gtk_widget_queue_draw(child->widget);
      children = children->next;
    }

}

static void
gtk_plot_canvas_draw_grid(GtkPlotCanvas *canvas)
{
  gdouble x, y;

  if(!canvas->pixmap) return;
  if(!canvas->show_grid) return;

  if(!canvas->pc) return;

  gtk_plot_canvas_set_line_attributes(canvas, canvas->grid);

  for(x = 0; x < canvas->pixmap_width; x += canvas->grid_step)
      gtk_plot_pc_draw_line(canvas->pc, 
                            roundint(x), 0, roundint(x), canvas->pixmap_height);

  for(y = 0; y < canvas->pixmap_height; y += canvas->grid_step)
      gtk_plot_pc_draw_line(canvas->pc, 
                            0, roundint(y), canvas->pixmap_width, roundint(y));
}

static void
gtk_plot_canvas_map(GtkWidget *widget)
{
  GtkPlotCanvas *plot_canvas;

  plot_canvas=GTK_PLOT_CANVAS(widget);

  GTK_WIDGET_CLASS(parent_class)->map(widget);

  if(!plot_canvas->pixmap){
      gtk_plot_canvas_create_pixmap(widget,
                                    plot_canvas->pixmap_width,
                                    plot_canvas->pixmap_height);
      if (GTK_IS_PLOT_CAIRO(plot_canvas->pc)) {
          gtk_plot_canvas_set_pc(plot_canvas, GTK_PLOT_PC(gtk_plot_cairo_new_with_drawable(plot_canvas->pixmap)));
      }
  }

  gtk_plot_canvas_paint(plot_canvas);
  gdk_window_set_cursor(gtk_widget_get_window(widget), plot_canvas->cursor);
}

static gint
gtk_plot_canvas_key_press(GtkWidget *widget, GdkEventKey *key)
{
	GtkPlotCanvas *canvas = GTK_PLOT_CANVAS(widget);

	switch (key->keyval)
	{
		case GDK_KEY_Escape:
			gtk_plot_canvas_cancel_action(canvas);
			break;

		default: break;
	}
	return TRUE;
}

static gint 
gtk_plot_canvas_motion (GtkWidget *widget, GdkEventMotion *event)
{
  GtkPlotCanvas *canvas;
  GtkAllocation area;
  gint x, y;
  gint new_x = 0, new_y = 0;
  gint new_width = 0, new_height = 0;
  gint cursor = GDK_TOP_LEFT_ARROW;
  gint pivot_x, pivot_y;

  canvas = GTK_PLOT_CANVAS(widget);
  gtk_widget_get_pointer(widget, &x, &y);

  if(canvas->active_item && canvas->active_item->flags == GTK_PLOT_CANVAS_CHILD_FROZEN) return TRUE;

  if(canvas->active_item){
    area = canvas->active_item->drag_area;
    new_x = area.x;
    new_y = area.y;
    new_width = area.width;
    new_height = area.height;
  }
  pivot_x = x;
  pivot_y = y;

  if(canvas->action == GTK_PLOT_CANVAS_ACTION_INACTIVE) 
       cursor = GDK_TOP_LEFT_ARROW;
  else if(canvas->action == GTK_PLOT_CANVAS_ACTION_DRAG) 
       cursor = GDK_FLEUR;
  else
       switch(canvas->drag_point){
            case GTK_PLOT_CANVAS_TOP_LEFT: 
                 cursor = GDK_UL_ANGLE; 
                 pivot_x = area.x + area.width;
                 pivot_y = area.y + area.height;
                 break;
            case GTK_PLOT_CANVAS_TOP_RIGHT:
                 cursor = GDK_UR_ANGLE; 
                 pivot_x = area.x;
                 pivot_y = area.y + area.height;
                 break;
            case GTK_PLOT_CANVAS_TOP:
                 cursor = GDK_TOP_SIDE;
                 pivot_y = area.y + area.height;
                 break;
            case GTK_PLOT_CANVAS_BOTTOM_LEFT:
                 cursor = GDK_LL_ANGLE;
                 pivot_x = area.x + area.width;
                 pivot_y = area.y;
                 break;
            case GTK_PLOT_CANVAS_BOTTOM_RIGHT:
                 cursor = GDK_LR_ANGLE;
                 pivot_x = area.x;
                 pivot_y = area.y;
                 break;
            case GTK_PLOT_CANVAS_BOTTOM:
                 cursor = GDK_BOTTOM_SIDE;
                 pivot_y = area.y;
                 break;
            case GTK_PLOT_CANVAS_LEFT:
                 cursor = GDK_LEFT_SIDE;
                 pivot_x = area.x + area.width;
                 break;
            case GTK_PLOT_CANVAS_RIGHT:
                 cursor = GDK_RIGHT_SIDE;
                 pivot_x = area.x;
                 break;
            default:
                 cursor = GDK_TOP_LEFT_ARROW;
       }

  if(cursor != gdk_cursor_get_cursor_type(canvas->cursor)){
         gdk_cursor_destroy(canvas->cursor);
         canvas->cursor = gdk_cursor_new(cursor);
         gdk_window_set_cursor(gtk_widget_get_window(widget), canvas->cursor);
  }    
    

  if(canvas->action == GTK_PLOT_CANVAS_ACTION_INACTIVE) return TRUE;

  switch(canvas->action){
     case GTK_PLOT_CANVAS_ACTION_DRAG:
       if(canvas->active_item && canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_MOVE){
         gint dx, dy;

         gtk_plot_canvas_child_draw_selection(canvas, canvas->active_item, canvas->drag_area);
         canvas->pointer_x = x;
         canvas->pointer_y = y;
         dx = x - canvas->drag_x;
         dy = y - canvas->drag_y;
         area.x = canvas->active_item->drag_area.x + dx; 
         area.y = canvas->active_item->drag_area.y + dy;
         gtk_plot_canvas_child_draw_selection(canvas, canvas->active_item, area);
         canvas->drag_area = area;
       }
       break;
     case GTK_PLOT_CANVAS_ACTION_RESIZE:
       switch(canvas->drag_point){
            case GTK_PLOT_CANVAS_TOP_LEFT: 
            case GTK_PLOT_CANVAS_TOP_RIGHT:
               if(canvas->active_item && canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                    new_x = MIN(x, pivot_x);
                    new_width = abs(x - pivot_x);
               }
            case GTK_PLOT_CANVAS_TOP:
               if(canvas->active_item && canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                    new_y = MIN(y, pivot_y);
                    new_height = abs(y - pivot_y);
               }
               gdk_cursor_destroy(canvas->cursor);
               canvas->cursor = gdk_cursor_new(cursor);
               gdk_window_set_cursor(gtk_widget_get_window(widget), canvas->cursor);
               break;
            case GTK_PLOT_CANVAS_BOTTOM_LEFT:
            case GTK_PLOT_CANVAS_BOTTOM_RIGHT:
               if(canvas->active_item && canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                    new_x = MIN(x, pivot_x);
                    new_width = abs(x - pivot_x);
               }
            case GTK_PLOT_CANVAS_BOTTOM:
               if(canvas->active_item && canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                    new_y = MIN(y, pivot_y);
                    new_height = abs(y - pivot_y);
               }
               break;
            case GTK_PLOT_CANVAS_LEFT:
            case GTK_PLOT_CANVAS_RIGHT:
               if(canvas->active_item && canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                    new_x = MIN(x, pivot_x);
                    new_width = abs(x - pivot_x);
               }
               break;
            case GTK_PLOT_CANVAS_IN:
            case GTK_PLOT_CANVAS_OUT:
            default:
               break;
       }


       if(canvas->active_item && new_width >= canvas->active_item->min_width &&
          new_height >= canvas->active_item->min_height){ 

                gtk_plot_canvas_child_draw_selection(canvas, canvas->active_item, canvas->drag_area);
                canvas->pointer_x = x;
                canvas->pointer_y = y;
                if(canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_MOVE || canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                  area.x = new_x;
                  area.y = new_y;
                }
                if(canvas->active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE){
                  area.width = new_width;
                  area.height = new_height;
                }
                gtk_plot_canvas_child_draw_selection(canvas, canvas->active_item, area);
                canvas->drag_area = area;

       }

       break;
     case GTK_PLOT_CANVAS_ACTION_SELECTION:
         draw_selection(canvas, NULL, canvas->drag_area);
         canvas->pointer_x = x;
         canvas->pointer_y = y;
         area.x = MIN(canvas->pointer_x, canvas->drag_x); 
         area.y = MIN(canvas->pointer_y, canvas->drag_y); 
         area.width = abs(x - canvas->drag_x);
         area.height = abs(y - canvas->drag_y);
         canvas->drag_area = area;
         draw_selection(canvas, NULL, canvas->drag_area);
 	 break;
     case GTK_PLOT_CANVAS_ACTION_INACTIVE:
     default:
         break;
  }

  return TRUE;

}

static GtkPlotCanvasPos 
gtk_plot_canvas_child_button_press(GtkPlotCanvas *canvas,
				   GtkPlotCanvasChild *child,
				   gint x, gint y)
{
  GtkPlotCanvasPos pos;
  GtkAllocation area;

  area = child->allocation;

  if((pos = possible_selection(area, x, y)) != GTK_PLOT_CANVAS_OUT){ 
      child->state = GTK_STATE_SELECTED;
      child->drag_area = area;
  }
  return pos;
}

static gint
gtk_plot_canvas_button_press(GtkWidget *widget, GdkEventButton *event)
{
  GtkPlotCanvas *canvas = NULL;
  GtkPlotCanvasChild *active_item = NULL;
  GList *childs = NULL;
  GdkModifierType mods;
  gint x = 0, y = 0;
  gboolean veto;
  gboolean double_click = FALSE;
  gdouble m;
  gboolean new_item = FALSE;
  GtkPlotCanvasPos pos = GTK_PLOT_CANVAS_OUT;

  gdk_window_get_pointer(gtk_widget_get_window(widget), NULL, NULL, &mods);
  if(!(mods & GDK_BUTTON1_MASK)) return FALSE;
  double_click = (event->button == GDK_2BUTTON_PRESS);
 
  canvas = GTK_PLOT_CANVAS(widget);
  m = canvas->magnification;

/*
  if(double_click && canvas->state == GTK_STATE_SELECTED) return TRUE;
*/
/*
  gdk_pointer_ungrab(event->time);
*/

  if(!gtk_widget_has_focus(widget)) gtk_widget_grab_focus(widget);

  gtk_widget_get_pointer(widget, &x, &y);

/**********************************************************************/
  
  if(GTK_PLOT_CANVAS_CAN_SELECT_ITEM(canvas)){
    childs = g_list_last(canvas->childs);
    while(childs)
      {
        GtkPlotCanvasChild *child = GTK_PLOT_CANVAS_CHILD(childs->data);
  
        pos = GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->button_press(canvas, child, x, y);
        if(pos != GTK_PLOT_CANVAS_OUT && child->state == GTK_STATE_SELECTED){
          active_item = child;
          break;
        }

        childs = childs->prev;
      }

/**********************************************************************/

    new_item = active_item && ((canvas->state != GTK_STATE_SELECTED ||
                active_item != canvas->active_item));

    veto = TRUE;
    if(active_item && active_item->state == GTK_STATE_SELECTED)
      _gtkextra_signal_emit(GTK_OBJECT(canvas), canvas_signals[SELECT_ITEM],
                      event, active_item, &veto);
 
    if(new_item){
  
      if(veto){
        gtk_plot_canvas_unselect(canvas);
        canvas->active_item = active_item;
        canvas->drag_area = active_item->drag_area;
        canvas->state = GTK_STATE_SELECTED; 
        canvas->action = GTK_PLOT_CANVAS_ACTION_INACTIVE;
   
        canvas->drag_point = pos;
     
        canvas->drag_x = x;
        canvas->drag_y = y;
        canvas->pointer_x = x;
        canvas->pointer_y = y;
   
        gtk_plot_canvas_child_draw_selection(canvas, active_item, active_item->drag_area);
   
        if(active_item->mode == GTK_PLOT_CANVAS_SELECT_CLICK_2)
          return TRUE;
      }
    }
    if(active_item && veto){
      if((!new_item && active_item->mode == GTK_PLOT_CANVAS_SELECT_CLICK_2) ||  active_item->mode == GTK_PLOT_CANVAS_SELECT_CLICK_1) {
      
        if(GTK_PLOT_CANVAS_CAN_DND(canvas)) {
          switch(pos){
           case GTK_PLOT_CANVAS_IN:
               canvas->action = GTK_PLOT_CANVAS_ACTION_DRAG;
               break;
           default:
               if(active_item->flags & GTK_PLOT_CANVAS_CHILD_CAN_RESIZE)
                  canvas->action = GTK_PLOT_CANVAS_ACTION_RESIZE;
               else
                  canvas->action = GTK_PLOT_CANVAS_ACTION_DRAG;
          }
/*
          gdk_pointer_grab (gtk_widget_get_window(widget), FALSE,
                            GDK_POINTER_MOTION_HINT_MASK |
                            GDK_BUTTON1_MOTION_MASK |
                            GDK_BUTTON_RELEASE_MASK,
                            NULL, NULL, event->time);
*/ 
          canvas->drag_point = pos;
 
          canvas->drag_x = x;
          canvas->drag_y = y;
          canvas->pointer_x = x;
          canvas->pointer_y = y;

          return TRUE;
        }
      }
    }
  } 
  gtk_plot_canvas_unselect(canvas);

  if(GTK_PLOT_CANVAS_CAN_SELECT(canvas)){
    veto = TRUE;

    _gtkextra_signal_emit(GTK_OBJECT(canvas), canvas_signals[SELECT_ITEM],
                  event, NULL, &veto);
    if(veto){
      canvas->active_item = NULL;
      canvas->state = GTK_STATE_SELECTED;
      canvas->action = GTK_PLOT_CANVAS_ACTION_SELECTION;
      canvas->drag_point = pos;
      canvas->drag_x = x;
      canvas->drag_y = y;
      canvas->pointer_x = x;
      canvas->pointer_y = y;
      canvas->drag_area.x = x;
      canvas->drag_area.y = y;
      canvas->drag_area.width = 0;
      canvas->drag_area.height = 0;
/*
      gdk_pointer_grab (gtk_widget_get_window(widget), FALSE,
                        GDK_POINTER_MOTION_HINT_MASK |
                        GDK_BUTTON1_MOTION_MASK |
                        GDK_BUTTON_RELEASE_MASK,
                        NULL, NULL, event->time);
*/
      draw_selection(canvas, NULL, canvas->drag_area);
    }
  }

  return TRUE;  
}

static void
gtk_plot_canvas_child_button_release(GtkPlotCanvas *canvas, GtkPlotCanvasChild *child)
{
  gdouble new_x, new_y, new_width, new_height;
  double dx, dy;
  gdouble x1, x2, y1, y2;
                                                                                
  gtk_plot_canvas_get_position(canvas,
                               canvas->drag_area.width,
                               canvas->drag_area.height,
                               &new_width, &new_height);
                                                                                
  gtk_plot_canvas_get_position(canvas,
                               canvas->drag_area.x,
                               canvas->drag_area.y,
                               &new_x, &new_y);
                                                                                
  gtk_plot_canvas_get_position(canvas,
                               canvas->drag_area.x - child->drag_area.x,
                               canvas->drag_area.y - child->drag_area.y,
                               &dx, &dy);
                                                                                
  x1 = MIN(child->rx1, child->rx2);
  y1 = MIN(child->ry1, child->ry2);
  x2 = MAX(child->rx1, child->rx2);
  y2 = MAX(child->ry1, child->ry2);
  x1 += dx;
  y1 += dy;
  x2 = x1 + new_width;
  y2 = y1 + new_height;
                                                                                
  gtk_plot_canvas_child_move_resize(canvas, child, x1, y1, x2, y2);
  child->drag_area = canvas->drag_area;
}
 
static gint
gtk_plot_canvas_button_release(GtkWidget *widget, GdkEventButton *event)
{
  GtkPlotCanvas *canvas;
  gdouble new_x, new_y;
  gdouble new_width, new_height;
  gdouble x1 = 0., y1 = 0., x2 = 0., y2 = 0.;
  gboolean veto = TRUE;
  gdouble dx = 0., dy = 0.;

  canvas = GTK_PLOT_CANVAS(widget); 

/*
  gdk_pointer_ungrab(event->time);
*/

  if(gtk_widget_get_mapped(widget)){
      gdk_cursor_destroy(canvas->cursor);
      canvas->cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
      gdk_window_set_cursor(gtk_widget_get_window(widget), 
                            canvas->cursor);
  }

  if(canvas->action == GTK_PLOT_CANVAS_ACTION_INACTIVE) return TRUE;

  gtk_plot_canvas_get_position(canvas,
                               canvas->drag_area.width, 
                               canvas->drag_area.height, 
                               &new_width, &new_height);

  gtk_plot_canvas_get_position(canvas,
                               canvas->drag_area.x, 
                               canvas->drag_area.y, 
                               &new_x, &new_y);

  if(canvas->action != GTK_PLOT_CANVAS_ACTION_SELECTION && canvas->active_item){
    GtkPlotCanvasChild *child = canvas->active_item;

    gtk_plot_canvas_get_position(canvas,
                                 canvas->drag_area.x - 
                                 canvas->active_item->drag_area.x, 
                                 canvas->drag_area.y - 
                                 canvas->active_item->drag_area.y, 
                                 &dx, &dy);

    x1 = MIN(child->rx1, child->rx2);
    y1 = MIN(child->ry1, child->ry2);
    x2 = MAX(child->rx1, child->rx2);
    y2 = MAX(child->ry1, child->ry2);
    x1 += dx;
    y1 += dy;
    x2 = x1 + new_width;
    y2 = y1 + new_height;

    if(canvas->action == GTK_PLOT_CANVAS_ACTION_DRAG){
      _gtkextra_signal_emit(GTK_OBJECT(canvas), canvas_signals[MOVE_ITEM],
                      child,
                      x1, y1, &veto);
    }
    if(canvas->action == GTK_PLOT_CANVAS_ACTION_RESIZE){
       _gtkextra_signal_emit(GTK_OBJECT(canvas),
                       canvas_signals[RESIZE_ITEM],
                       child, new_width, new_height, &veto);
    }
    if(canvas->action != GTK_PLOT_CANVAS_ACTION_INACTIVE && veto) {
      if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(canvas->active_item)))->button_release)
        GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(canvas->active_item)))->button_release(canvas, canvas->active_item);

      g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
      canvas->pointer_x = canvas->pointer_y = canvas->drag_x = canvas->drag_y = 0;
      gtk_plot_canvas_child_draw_selection(canvas, canvas->active_item, canvas->active_item->drag_area);
      canvas->action = GTK_PLOT_CANVAS_ACTION_INACTIVE;
      return TRUE;
    } else {
      canvas->state = GTK_STATE_NORMAL;
    }

  } else {
    gtk_plot_canvas_get_position(canvas, 
                                 canvas->drag_x, canvas->drag_y,
				 &x1, &y1);
    gtk_plot_canvas_get_position(canvas, 
                                 canvas->pointer_x, canvas->pointer_y,
				 &x2, &y2);
    new_width = abs(canvas->pointer_x - canvas->drag_x);
    new_height = abs(canvas->pointer_y - canvas->drag_y);
    draw_selection(canvas, NULL, canvas->drag_area); 
    g_signal_emit(GTK_OBJECT(canvas), canvas_signals[SELECT_REGION], 0,
                    x1, y1, x2, y2);
    canvas->state = GTK_STATE_NORMAL;
    canvas->action = GTK_PLOT_CANVAS_ACTION_INACTIVE;
    return TRUE;
  }

  canvas->drag_x = canvas->pointer_x; 
  canvas->drag_y = canvas->pointer_y; 

  if(canvas->action != GTK_PLOT_CANVAS_ACTION_SELECTION)
             canvas->state = GTK_STATE_NORMAL;
  canvas->action = GTK_PLOT_CANVAS_ACTION_INACTIVE;
 
  return TRUE;

}

static gint
gtk_plot_canvas_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
  GTK_WIDGET_SET_FLAGS(widget, GTK_HAS_FOCUS);
  return FALSE;
}


static gint
gtk_plot_canvas_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
  GTK_WIDGET_UNSET_FLAGS(widget, GTK_HAS_FOCUS);
  gtk_plot_canvas_unselect(GTK_PLOT_CANVAS(widget));
  return FALSE;
}

/**
 * gtk_plot_canvas_set_transparent:
 * @canvas: a #GtkPlotCanvas.
 * @transparent:
 *
 *
 */
void
gtk_plot_canvas_set_transparent (GtkPlotCanvas *canvas, gboolean transparent)
{
  g_return_if_fail (canvas != NULL);
  g_return_if_fail (GTK_IS_PLOT_CANVAS (canvas));

  canvas->transparent = transparent;
}

/**
 * gtk_plot_canvas_trasnparent:
 * @canvas: a #GtkPlotCanvas.
 *
 *
 *
 * Return value:
 */
gboolean
gtk_plot_canvas_transparent (GtkPlotCanvas *canvas)
{
  g_return_val_if_fail (canvas != NULL, TRUE);
  g_return_val_if_fail (GTK_IS_PLOT_CANVAS (canvas), TRUE);

  return(canvas->transparent);
}

/**
 * gtk_plot_canvas_set_background:
 * @canvas: a #GtkPlotCanvas.
 * @bg_color: a GdkColor.
 *
 *
 */
void
gtk_plot_canvas_set_background (GtkPlotCanvas *canvas, const GdkColor *bg_color)
{

  g_return_if_fail (canvas != NULL);
  g_return_if_fail (GTK_IS_PLOT_CANVAS (canvas));

  if(!bg_color) {
    canvas->transparent = TRUE;
    return;
  } else {
    canvas->background = *bg_color;
    canvas->transparent = FALSE;
  }

  if(gtk_widget_get_realized(GTK_WIDGET(canvas)))
       gtk_plot_canvas_paint(canvas);

  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

/**
 * gtk_plot_canvas_get_pixel:
 * @canvas: a #GtkPlotCanvas.
 * @px:
 * @py:
 * @x:
 * @y:
 *
 *
 */
void
gtk_plot_canvas_get_pixel(GtkPlotCanvas *canvas, gdouble px, gdouble py,
                          gint *x, gint *y)
{
  *x = roundint(canvas->pixmap_width * px);
  *y = roundint(canvas->pixmap_height * py);
}

/**
 * gtk_plot_canvas_get_position:
 * @plot_canvas: a #GtkPlotCanvas.
 * @x:
 * @y:
 * @px:
 * @py:
 *
 *
 */
void
gtk_plot_canvas_get_position(GtkPlotCanvas *plot_canvas, gint x, gint y,
                             gdouble *px, gdouble *py)
{
  *px = (gdouble) x / (gdouble) plot_canvas->pixmap_width;
  *py = (gdouble) y / (gdouble) plot_canvas->pixmap_height;
}

/**
 * gtk_plot_canvas_unselect:
 * @plot_canvas: a #GtkPlotCanvas.
 *
 *
 */
void
gtk_plot_canvas_unselect (GtkPlotCanvas *plot_canvas)
{

  if(plot_canvas->state == GTK_STATE_SELECTED){
    if(plot_canvas->active_item)
      gtk_plot_canvas_child_draw_selection(plot_canvas, plot_canvas->active_item, plot_canvas->drag_area); 
    else
      draw_selection(plot_canvas, NULL, plot_canvas->drag_area); 
  }

  plot_canvas->action = GTK_PLOT_CANVAS_ACTION_INACTIVE;
  plot_canvas->state = GTK_STATE_NORMAL;
  if(plot_canvas->active_item){ 
    plot_canvas->active_item->state = GTK_STATE_NORMAL;
    if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(plot_canvas->active_item)))->unselect)
      GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(plot_canvas->active_item)))->unselect(plot_canvas, plot_canvas->active_item);
  }
  plot_canvas->active_item = NULL;

  if(gtk_widget_get_mapped(GTK_WIDGET(plot_canvas))){
      gdk_cursor_destroy(plot_canvas->cursor);
      plot_canvas->cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
      gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(plot_canvas)), 
                            plot_canvas->cursor);
  }
}

/**
 * gtk_plot_canvas_cancel_action:
 * @plot_canvas: a #GtkPlotCanvas.
 *
 *
 */

void
gtk_plot_canvas_cancel_action (GtkPlotCanvas *plot_canvas)
{
  gtk_plot_canvas_unselect(plot_canvas);
/*
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
*/
}


/**
 * gtk_plot_canvas_get_active_item:
 * @canvas: a #GtkPlotCanvas. 
 *
 * Return value: (transfer none): the active #GtkPlotCanvasChild
 */
GtkPlotCanvasChild *
gtk_plot_canvas_get_active_item(GtkPlotCanvas *canvas)
{
  return (canvas->active_item);
}

/**
 * gtk_plot_canvas_child_get_position:
 * @canvas: a #GtkPlotCanvas.
 * @child: a #GtkPlotCanvasChild.
 * @x1:
 * @y1:
 * @x2:
 * @y2:
 *
 *
 *
 * Return value:
 */
gboolean
gtk_plot_canvas_child_get_position(GtkPlotCanvas *canvas,
                                   GtkPlotCanvasChild *child,
                                   double *x1, double *y1,
                                   double *x2, double *y2)
{
  GList *list;
  gpointer data;

  *x1 = child->rx1;
  *x2 = child->rx2;
  *y1 = child->ry1;
  *y2 = child->ry2;

  list = canvas->childs;

  /* Return TRUE if child is actually contained w/in canvas, and
     coordinates of the childs bounding box, else FALSE */
  while(list)
   {
     data = list->data;
     if(GTK_PLOT_CANVAS_CHILD(data) == child) return TRUE;
     list = list->next;
   }

  return FALSE;

}

/**
 * gtk_plot_canvas_get_size:
 * @canvas: a #GtkPlotCanvas.
 * @width:
 * @height:
 * 
 */
void
gtk_plot_canvas_set_size(GtkPlotCanvas *canvas, gint width, gint height)
{
  GList *list = NULL;
  gdouble m = canvas->magnification;
  GtkAllocation allocation;

  gtk_widget_get_allocation(GTK_WIDGET(canvas), &allocation);

  gtk_plot_canvas_cancel_action(canvas);

  canvas->width = width;
  canvas->height = height;
  canvas->pixmap_width = roundint(m * width);
  canvas->pixmap_height = roundint(m * height);

  if(gtk_widget_get_mapped(GTK_WIDGET(canvas)) && canvas->pixmap){ 
    gtk_plot_canvas_create_pixmap(GTK_WIDGET(canvas), 
                                  canvas->pixmap_width, 
                                  canvas->pixmap_height);
  }

/*
  gtk_widget_set_size_request(GTK_WIDGET(canvas), 
                       canvas->pixmap_width, canvas->pixmap_height);
*/

  allocation.width = canvas->pixmap_width;
  allocation.height = canvas->pixmap_height;

  gtk_widget_size_allocate(GTK_WIDGET(canvas), &allocation);

  list = canvas->childs;
  while(list){
    GtkPlotCanvasChild *child = GTK_PLOT_CANVAS_CHILD(list->data);
    if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->size_allocate)
      GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->size_allocate(canvas, child);
    list = list->next;
  }


  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

static void
gtk_plot_canvas_create_pixmap(GtkWidget *widget, gint width, gint height)
{
  GtkPlotCanvas *canvas;
  gint pixmap_width, pixmap_height;

  canvas = GTK_PLOT_CANVAS(widget);
  if (!canvas->pixmap){
    canvas->pixmap = gdk_pixmap_new (gtk_widget_get_window(widget),
                                     width,
                                     height, -1);
  }else{
    gdk_window_get_size(canvas->pixmap, &pixmap_width, &pixmap_height);
    if(width != pixmap_width || height != pixmap_height){
        gdk_pixmap_unref(canvas->pixmap);
        canvas->pixmap = gdk_pixmap_new (gtk_widget_get_window(widget),
                                         width,
                                         height, -1);
    }
  }

  if (canvas->pc && GTK_IS_PLOT_CAIRO(canvas->pc)) {
      gtk_plot_canvas_set_pc(canvas, GTK_PLOT_PC(gtk_plot_cairo_new_with_drawable(canvas->pixmap)));
  }

  if(canvas->pc && GTK_IS_PLOT_GDK(canvas->pc)){
       GTK_PLOT_GDK(canvas->pc)->drawable = canvas->pixmap;
  }
  gtk_plot_pc_set_viewport(canvas->pc, width, height);
}

static gint
gtk_plot_canvas_expose(GtkWidget *widget, GdkEventExpose *event)
{
  GtkPlotCanvas *canvas;
  GdkPixmap *pixmap;

  if(!gtk_widget_is_drawable(widget)) return FALSE;

  canvas = GTK_PLOT_CANVAS(widget);

  if(!canvas->pixmap){
      gtk_plot_canvas_create_pixmap(widget, 
                                    canvas->pixmap_width, 
                                    canvas->pixmap_height);
      gtk_plot_canvas_paint(canvas);
      return FALSE;
  }

  pixmap = canvas->pixmap;
  gdk_draw_pixmap(gtk_widget_get_window(GTK_WIDGET(canvas)),
                  gtk_widget_get_style(widget)->fg_gc[GTK_STATE_NORMAL],
                  pixmap, 
                  event->area.x, 
                  event->area.y,
                  event->area.x, 
                  event->area.y,
                  event->area.width, event->area.height);

  GTK_WIDGET_CLASS(parent_class)->expose_event(widget, event);

  return FALSE;

}


static void
gtk_plot_canvas_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  GtkPlotCanvas *canvas;

  canvas = GTK_PLOT_CANVAS(widget);

  GTK_WIDGET_CLASS(parent_class)->size_request(widget, requisition);

  gtk_widget_set_size_request(widget, 
  			MAX(canvas->pixmap_width, requisition->width),
  			MAX(canvas->pixmap_height, requisition->height));
}


/**
 * gtk_plot_canvas_set_magnification:
 * @canvas: a #GtkPlotCanvas.
 * @magnification:
 *
 *
 */
void
gtk_plot_canvas_set_magnification(GtkPlotCanvas *canvas, 
                                  gdouble magnification)
{
  GList *list;

  canvas->magnification = magnification;

  list = canvas->childs;
  while(list){
    GtkPlotCanvasChild *child = GTK_PLOT_CANVAS_CHILD(list->data);

    if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->set_magnification)
      GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->set_magnification(canvas, child, magnification);

    list = list->next;
  }

  gtk_plot_canvas_set_size(canvas, 
                           canvas->width, 
                           canvas->height);
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

static void
gtk_plot_canvas_child_draw_selection(GtkPlotCanvas *canvas, GtkPlotCanvasChild *child, GtkAllocation area)
{
   if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->draw_selection) 
     GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->draw_selection(canvas, child, area);
}

static void
draw_selection(GtkPlotCanvas *canvas, GtkPlotCanvasChild *child, GtkAllocation area)
{
  GdkGC *xor_gc = NULL;
  GdkGCValues values;

  gdk_gc_get_values(gtk_widget_get_style(GTK_WIDGET(canvas))->fg_gc[0], &values);
  values.function = GDK_INVERT;
  values.foreground = gtk_widget_get_style(GTK_WIDGET(canvas))->white;
  values.subwindow_mode = GDK_INCLUDE_INFERIORS;
  xor_gc = gdk_gc_new_with_values(gtk_widget_get_window(GTK_WIDGET(canvas)),
                                  &values,
                                  GDK_GC_FOREGROUND |
                                  GDK_GC_FUNCTION |
                                  GDK_GC_SUBWINDOW);

  if(canvas->active_item){
    if(canvas->active_item->selection == GTK_PLOT_CANVAS_SELECT_TARGET){
      area.x += area.width / 2 - 10;
      area.y += area.height / 2 - 10;
      area.width = 20;
      area.height = 20;
      gdk_draw_rectangle (gtk_widget_get_window(GTK_WIDGET(canvas)),
                          xor_gc,
                          FALSE,
                          area.x, area.y,
                          area.width, area.height);         
  
      gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(canvas)), xor_gc,
                    area.x + 1, area.y + area.height/2, 
                    area.x + 6, area.y + area.height/2);
      gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(canvas)), xor_gc,
                    area.x + area.width - 1, area.y + area.height / 2, 
                    area.x + area.width - 6, area.y + area.height / 2);
      gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(canvas)), xor_gc,
                    area.x + area.width/2, area.y + 1, 
                    area.x + area.width/2, area.y + 6);
      gdk_draw_line(gtk_widget_get_window(GTK_WIDGET(canvas)), xor_gc,
                    area.x + area.width/2, area.y + area.height - 1, 
                    area.x + area.width/2, area.y + area.height - 6);
  
      if(xor_gc) gdk_gc_destroy(xor_gc);
      return;
    }
  
    
    if(canvas->active_item->selection == GTK_PLOT_CANVAS_SELECT_MARKERS){
       gdk_draw_rectangle (gtk_widget_get_window(GTK_WIDGET(canvas)),
                           xor_gc,
                           FALSE,
                           area.x, area.y,
                           area.width, area.height);         
       draw_marker(canvas, xor_gc, area.x, area.y); 
       draw_marker(canvas, xor_gc, area.x, area.y + area.height); 
       draw_marker(canvas, xor_gc, area.x + area.width, area.y); 
       draw_marker(canvas, xor_gc, area.x + area.width, area.y + area.height); 
       if(area.height > DEFAULT_MARKER_SIZE * 2){
         draw_marker(canvas, xor_gc, area.x, area.y + area.height / 2); 
         draw_marker(canvas, xor_gc, area.x + area.width, 
                                     area.y + area.height / 2); 
       }
       if(area.width > DEFAULT_MARKER_SIZE * 2){
         draw_marker(canvas, xor_gc, area.x + area.width / 2, area.y); 
         draw_marker(canvas, xor_gc, area.x + area.width / 2, 
                                     area.y + area.height); 
       }
    }
  } else {
    gdk_gc_set_line_attributes(xor_gc, 1, 1, 0 ,0 );
  
    gdk_draw_rectangle (gtk_widget_get_window(GTK_WIDGET(canvas)),
                        xor_gc,
                        FALSE,
                        area.x, area.y,
                        area.width, area.height);         
  }
  if(xor_gc) gdk_gc_unref(xor_gc);
}

static void
draw_marker(GtkPlotCanvas *canvas, GdkGC *gc, gint x, gint y)
{
  GdkDrawable *darea;

  darea = gtk_widget_get_window(GTK_WIDGET(canvas));

  gdk_draw_rectangle(darea, gc, TRUE,
                     x - DEFAULT_MARKER_SIZE / 2, y - DEFAULT_MARKER_SIZE / 2,
                     DEFAULT_MARKER_SIZE + 1, DEFAULT_MARKER_SIZE + 1);
}

/* FIXME:: Is this private or public */
GtkPlotCanvasPos
possible_selection(GtkAllocation area, gint x, gint y)
{
  GtkPlotCanvasPos return_value = GTK_PLOT_CANVAS_OUT;

  if(x >= area.x - DEFAULT_MARKER_SIZE / 2 &&
     x <= area.x + DEFAULT_MARKER_SIZE / 2){
       if(y >= area.y - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + DEFAULT_MARKER_SIZE / 2.)
                      return_value = GTK_PLOT_CANVAS_TOP_LEFT;
       if(y >= area.y + area.height - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + area.height + DEFAULT_MARKER_SIZE / 2.)
                      return_value = GTK_PLOT_CANVAS_BOTTOM_LEFT;
       if(y >= area.y + area.height / 2 - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + area.height / 2 + DEFAULT_MARKER_SIZE / 2. &&
          area.height > DEFAULT_MARKER_SIZE * 2)
                      return_value = GTK_PLOT_CANVAS_LEFT;
  }

  if(x >= area.x + area.width - DEFAULT_MARKER_SIZE / 2 &&
     x <= area.x + area.width + DEFAULT_MARKER_SIZE / 2){
       if(y >= area.y - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + DEFAULT_MARKER_SIZE / 2.)
                      return_value = GTK_PLOT_CANVAS_TOP_RIGHT;
       if(y >= area.y + area.height - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + area.height + DEFAULT_MARKER_SIZE / 2.)
                      return_value = GTK_PLOT_CANVAS_BOTTOM_RIGHT;
       if(y >= area.y + area.height / 2 - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + area.height / 2 + DEFAULT_MARKER_SIZE / 2. &&
          area.height > DEFAULT_MARKER_SIZE * 2)
                      return_value = GTK_PLOT_CANVAS_RIGHT;
  }

  if(x >= area.x + area.width / 2 - DEFAULT_MARKER_SIZE / 2 &&
     x <= area.x + area.width / 2 + DEFAULT_MARKER_SIZE / 2 &&
     area.width > DEFAULT_MARKER_SIZE * 2){
       if(y >= area.y - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + DEFAULT_MARKER_SIZE / 2.)
                      return_value = GTK_PLOT_CANVAS_TOP;
       if(y >= area.y + area.height - DEFAULT_MARKER_SIZE / 2. &&
          y <= area.y + area.height + DEFAULT_MARKER_SIZE / 2.)
                      return_value = GTK_PLOT_CANVAS_BOTTOM;
  }

  if(return_value == GTK_PLOT_CANVAS_OUT){
     if (x >= area.x && x <= area.x + area.width &&
         y >= area.y && y <= area.y + area.height)
                      return_value = GTK_PLOT_CANVAS_IN;
  }

  return (return_value);
}    

/**********************************************************************/

/**
 * gtk_plot_canvas_grid_set_visible:
 * @canvas: a #GtkPlotCanvas.
 * @visible:
 *
 *
 */
void
gtk_plot_canvas_grid_set_visible(GtkPlotCanvas *canvas, gboolean visible)
{
  canvas->show_grid= visible;
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

/**
 * gtk_plot_canvas_grid_set_step:
 * @canvas: a #GtkPlotCanvas.
 * @step:
 *
 *
 */
void
gtk_plot_canvas_grid_set_step(GtkPlotCanvas *canvas, gdouble step)
{
  canvas->grid_step = step;
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

/**
 * gtk_plot_canvas_grid_set_attributes:
 * @canvas: a #GtkPlotCanvas.
 * @style:
 * @width:
 * @color:
 *
 *
 */
void
gtk_plot_canvas_grid_set_attributes(GtkPlotCanvas *canvas,
		 	            GtkPlotLineStyle style,
			            gint width,
			            const GdkColor *color)
{
  if(color)
      canvas->grid.color = *color;
  canvas->grid.line_width = width;
  canvas->grid.line_style = style;
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

/**********************************************************************/

/**
 * gtk_plot_canvas_put_child:
 * @canvas: a #GtkPlotCanvas.
 * @x1:
 * @y1:
 * @x2:
 * @y2:
 *
 *
 */
void
gtk_plot_canvas_put_child(GtkPlotCanvas *canvas, 
                          GtkPlotCanvasChild *child,
			  gdouble x1, gdouble y1, 
                          gdouble x2, gdouble y2)
{
  child->rx1 = x1;
  child->ry1 = y1;
  child->rx2 = x2;
  child->ry2 = y2;

  child->parent = canvas;
  canvas->childs = g_list_append(canvas->childs, child);
  g_object_ref(GTK_OBJECT(child));
  gtk_object_sink(GTK_OBJECT(child));

  if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->size_allocate)
    GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->size_allocate(canvas, child);
  if(gtk_widget_get_realized(GTK_WIDGET(canvas)) && 
		gtk_widget_get_visible(GTK_WIDGET(canvas))) 
    gtk_plot_canvas_child_draw(canvas, child);
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
  g_signal_emit(GTK_OBJECT(canvas), canvas_signals[ADD_ITEM], 0, child);
}

/**
 * gtk_plot_canvas_child_move:
 * @canvas: a #GtkPlotCanvas.
 * @child: a #GtkPlotCanvasChild
 * @x1:
 * @y1:
 *
 *
 */
void
gtk_plot_canvas_child_move(GtkPlotCanvas *canvas, 
                           GtkPlotCanvasChild *child,
			   gdouble x1, gdouble y1) 
{
  child->rx2 += (x1 - child->rx1);
  child->ry2 += (y1 - child->ry1);
  child->rx1 = x1;
  child->ry1 = y1;

  if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->move)
    GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->move(canvas, child, x1, y1);
  GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->size_allocate(canvas, child);
  gtk_plot_canvas_paint(canvas);
  gtk_plot_canvas_refresh(canvas);
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

/**
 * gtk_plot_canvas_child_move_resize:
 * @canvas: a #GtkPlotCanvas.
 * @x1:
 * @y1:
 * @x2:
 * @y2:
 *
 *
 */
void
gtk_plot_canvas_child_move_resize(GtkPlotCanvas *canvas, 
                                  GtkPlotCanvasChild *child,
			          gdouble x1, gdouble y1,
				  gdouble x2, gdouble y2) 
{
  child->rx1 = x1;
  child->ry1 = y1;
  child->rx2 = x2;
  child->ry2 = y2;

  if(GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->move_resize)
    GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->move_resize(canvas, child, x1, y1, x2, y2);
  GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->size_allocate(canvas, child);
  gtk_plot_canvas_paint(canvas);
  gtk_plot_canvas_refresh(canvas);
  g_signal_emit (GTK_OBJECT(canvas), canvas_signals[CHANGED], 0);
}

static void
gtk_plot_canvas_child_size_allocate(GtkPlotCanvas *canvas, GtkPlotCanvasChild *child)
{
  gint x1, x2, y1, y2;
  
  gtk_plot_canvas_get_pixel(canvas, child->rx1, child->ry1, &x1, &y1);
  gtk_plot_canvas_get_pixel(canvas, child->rx2, child->ry2, &x2, &y2);
  child->allocation.x = MIN(x1, x2);
  child->allocation.y = MIN(y1, y2);
  child->allocation.width = abs(x1 - x2);
  child->allocation.height = abs(y1 - y2);
}

/**
 * gtk_plot_canvas_set_selection:
 * @child: a #GtkPlotCanvasChild.
 * @selection: a #GtkPlotCanvasSelection
 *
 *
 */
void
gtk_plot_canvas_child_set_selection (GtkPlotCanvasChild *child,
				     GtkPlotCanvasSelection selection)
{
  if(!child) return;
  child->selection = selection;
}

/**
 * gtk_plot_canvas_child_set_selection_mode:
 * @child: a #GtkPlotCanvasChild.
 * @mode:
 *
 *
 */
void
gtk_plot_canvas_child_set_selection_mode (GtkPlotCanvasChild *child,
				          GtkPlotCanvasSelectionMode mode)
{
  if(!child) return;
  child->mode = mode;
}

/**
 * gtk_plot_canvas_child_draw:
 * @canvas: a #GtkPlotCanvas.
 * @child: a #GtkPlotCanvasChild.
 *
 *
 */
void
gtk_plot_canvas_child_draw(GtkPlotCanvas *canvas,
                           GtkPlotCanvasChild *child)
{
  gtk_plot_pc_gsave(canvas->pc);

  GTK_PLOT_CANVAS_CHILD_CLASS(GTK_OBJECT_GET_CLASS(GTK_OBJECT(child)))->draw(canvas, child);

  gtk_plot_pc_grestore(canvas->pc);
}

/**
 * gtk_plot_canvas_set_line_attributes:
 * @canvas: a #GtkPlotCanvas.
 * @line:
 *
 *
 */
void
gtk_plot_canvas_set_line_attributes(GtkPlotCanvas *canvas, GtkPlotLine line)
{
  gdouble dot[] = {2., 3.};
  gdouble dash[] = {6., 4.};
  gdouble dot_dash[] = {6., 4., 2., 4.};
  gdouble dot_dot_dash[] = {6., 4., 2., 4., 2., 4.};
  gdouble dot_dash_dash[] = {6., 4., 6., 4., 2., 4.};
                                                                                
  gtk_plot_pc_set_color(canvas->pc, &line.color);
                                                                                
  switch(line.line_style){
   case GTK_PLOT_LINE_SOLID:
        gtk_plot_pc_set_lineattr(canvas->pc, line.line_width, 0, 0, 0);
        break;
   case GTK_PLOT_LINE_DOTTED:
        gtk_plot_pc_set_lineattr(canvas->pc, line.line_width,
                                GDK_LINE_ON_OFF_DASH, 0, 0);
        gtk_plot_pc_set_dash(canvas->pc, 0, dot, 2);
        break;
   case GTK_PLOT_LINE_DASHED:
        gtk_plot_pc_set_lineattr(canvas->pc, line.line_width,
                                GDK_LINE_ON_OFF_DASH, 0, 0);
        gtk_plot_pc_set_dash(canvas->pc, 0, dash, 2);
   case GTK_PLOT_LINE_DOT_DASH:
        gtk_plot_pc_set_lineattr(canvas->pc, line.line_width,
                                GDK_LINE_ON_OFF_DASH, 0, 0);
        gtk_plot_pc_set_dash(canvas->pc, 0, dot_dash, 4);
        break;
   case GTK_PLOT_LINE_DOT_DOT_DASH:
        gtk_plot_pc_set_lineattr(canvas->pc, line.line_width,
                                GDK_LINE_ON_OFF_DASH, 0, 0);
        gtk_plot_pc_set_dash(canvas->pc, 0, dot_dot_dash, 6);
        break;
   case GTK_PLOT_LINE_DOT_DASH_DASH:
        gtk_plot_pc_set_lineattr(canvas->pc, line.line_width,
                                GDK_LINE_ON_OFF_DASH, 0, 0);
        gtk_plot_pc_set_dash(canvas->pc, 0, dot_dash_dash, 6);
        break;
   case GTK_PLOT_LINE_NONE:
   default:
        break;
  }
}
