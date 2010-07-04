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

#ifndef __GTK_PLOT_CANVAS_RECTANGLE_H__
#define __GTK_PLOT_CANVAS_RECTANGLE_H__

#define GTK_PLOT_CANVAS_RECTANGLE(obj)        G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_plot_canvas_rectangle_get_type (), GtkPlotCanvasRectangle)
#define GTK_PLOT_CANVAS_RECTANGLE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, gtk_plot_canvas_rectangle_get_type(), GtkPlotCanvasRectangleClass)
#define GTK_IS_PLOT_CANVAS_RECTANGLE(obj)     G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_plot_canvas_rectangle_get_type ())
#define G_TYPE_PLOT_CANVAS_RECTANGLE (gtk_plot_canvas_rectangle_get_type ())


#include <gdk/gdk.h>
#include "gtkplotpc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct _GtkPlotCanvasRectangle	GtkPlotCanvasRectangle;
typedef struct _GtkPlotCanvasRectangleClass	GtkPlotCanvasRectangleClass;

/**
 * GtkPlotCanvasRectangle:
 *
 * The GtkPlotCanvasRectangle struct contains only private data.
 * It should only be accessed through the functions described below.
 */
struct _GtkPlotCanvasRectangle
{
  GtkPlotCanvasChild parent;

  GtkPlotLine line;

  gboolean filled;
  GtkPlotBorderStyle border;
                                                                                
  gint shadow_width;
  GdkColor bg;
};

struct _GtkPlotCanvasRectangleClass
{
  GtkPlotCanvasChildClass parent_class;
};

GType 		gtk_plot_canvas_rectangle_get_type	(void);
GtkPlotCanvasChild * 
		gtk_plot_canvas_rectangle_new	(GtkPlotLineStyle style,
                              			 gfloat width,
                     			         const GdkColor *fg,
                       			         const GdkColor *bg,
                       			         GtkPlotBorderStyle border,
                             			 gboolean fill);
void 		gtk_plot_canvas_rectangle_set_attributes(
					  	 GtkPlotCanvasRectangle *rectangle,
                                         	 GtkPlotLineStyle style,
                                         	 gfloat width,
                                         	 const GdkColor *fg,
                                         	 const GdkColor *bg,
                                         	 GtkPlotBorderStyle border,
                                         	 gboolean fill);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_PLOT_CANVAS_RECTANGLE_H__ */
