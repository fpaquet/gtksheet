/* gtkplotprint - gtkplot printing functions
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
 * SECTION: gtkplotprint
 * @short_description: 
 *
 * FIXME:: Need long description.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <gtk/gtk.h>

#include "gtkplotpc.h"
#include "gtkplot.h"
#include "gtkplot3d.h"
#include "gtkplotdata.h"
#include "gtkpsfont.h"
#include "gtkplotpc.h"
#include "gtkplotps.h"
#include "gtkplotdt.h"
#include "gtkplotsurface.h"
#include "gtkplotcairo.h"
#include "gtkplotcanvas.h"

static void recalc_pixels(GtkPlot *plot);



/**
 * gtk_plot_export_ps:
 * @plot: a #GtkPlot
 * @file_name:
 * @orient:
 * @epsflag:
 * @page_size:
 *
 * 
 *
 * Return value: 
 */
gboolean
gtk_plot_export_ps                              (GtkPlot *plot,
                                                 char *file_name,
                                                 GtkPlotPageOrientation orient,
                                                 gboolean epsflag,
                                                 GtkPlotPageSize page_size)
{
  GtkPlotPC *pc;
  GtkPlotPS *ps;
  gdouble scalex, scaley;
  gdouble m;
  GtkAllocation allocation;


  m = plot->magnification;

  ps = GTK_PLOT_PS(gtk_plot_ps_new(file_name, orient, epsflag, page_size, 1.0, 1.0));

  gtk_widget_get_allocation(GTK_WIDGET(plot), &allocation);

  if(orient == GTK_PLOT_PORTRAIT){
    scalex = (gfloat)ps->page_width /
                                (gfloat)allocation.width;
    scaley = (gfloat)ps->page_height /
                                (gfloat)allocation.height;
  }else{
    scalex = (gfloat)ps->page_width /
                                (gfloat)allocation.height;
    scaley = (gfloat)ps->page_height /
                                (gfloat)allocation.width;
  }

  gtk_plot_ps_set_scale(ps, scalex, scaley);

  pc = plot->pc;

  plot->pc = GTK_PLOT_PC(ps);
  plot->magnification = 1.0;
  recalc_pixels(plot); 

  gtk_plot_paint(plot);

  plot->pc = pc;
  plot->magnification = m;
  gtk_object_destroy(GTK_OBJECT(ps));
  recalc_pixels(plot); 

  return TRUE;
}

/**
 * gtk_plot_export_ps_with_size:
 * @plot: a #GtkPlot
 * @file_name:
 * @orient:
 * @epsflag:
 * @units:
 * @width:
 * @height:
 *
 * 
 *
 * Return value: 
 */
gboolean
gtk_plot_export_ps_with_size                    (GtkPlot *plot,
                                                 char *file_name,
                                                 GtkPlotPageOrientation orient,
                                                 gboolean epsflag,
                                                 GtkPlotUnits units,
                                                 gint width,
                                                 gint height)
{
  GtkPlotPC *pc;
  GtkPlotPS *ps;
  gdouble scalex, scaley;
  gdouble m;
  GtkAllocation allocation;

  m = plot->magnification;

  ps = GTK_PLOT_PS(gtk_plot_ps_new_with_size(file_name, orient, epsflag, 
                                             units, 
                                             width, height, 
                                             1.0 , 1.0));

  gtk_widget_get_allocation(GTK_WIDGET(plot), &allocation);
  if(orient == GTK_PLOT_PORTRAIT){
    scalex = (gfloat)ps->page_width /
                                (gfloat)allocation.width;
    scaley = (gfloat)ps->page_height /
                                (gfloat)allocation.height;
  }else{
    scalex = (gfloat)ps->page_width /
                                (gfloat)allocation.height;
    scaley = (gfloat)ps->page_height /
                                (gfloat)allocation.width;
  }

  gtk_plot_ps_set_scale(ps, scalex, scaley);

  pc = plot->pc;
 
  plot->pc = GTK_PLOT_PC(ps);
  plot->magnification = 1.0;
  recalc_pixels(plot); 

  gtk_plot_paint(plot);

  plot->pc = pc;
  plot->magnification = m;
  recalc_pixels(plot); 
  gtk_object_destroy(GTK_OBJECT(ps));

  return TRUE;
}

/**
 * gtk_plot_canvas_export_ps:
 * @canvas: a #GtkPlotCanvas .
 * @file_name:
 * @orient:
 * @epsflag:
 * @page_size:
 *
 * 
 *
 * Return value: 
 */
gboolean
gtk_plot_canvas_export_ps                       (GtkPlotCanvas *canvas,
                                                 char *file_name,
                                                 GtkPlotPageOrientation orient,
                                                 gboolean epsflag,
                                                 GtkPlotPageSize page_size)
{
  GtkPlotPC *pc;
  GtkPlotPS *ps;
  gdouble scalex, scaley;
  gdouble m;
  GdkPixmap *pixmap;

  m = canvas->magnification;

  ps = GTK_PLOT_PS(gtk_plot_ps_new(file_name, orient, epsflag, page_size, 1.0, 1.0));

  if(orient == GTK_PLOT_PORTRAIT){
    scalex = (gfloat)ps->page_width / (gfloat)canvas->width;
    scaley = (gfloat)ps->page_height / (gfloat)canvas->height;
  }else{
    scalex = (gfloat)ps->page_width / (gfloat)canvas->height;
    scaley = (gfloat)ps->page_height / (gfloat)canvas->width;
  }

  gtk_plot_ps_set_scale(ps, scalex, scaley);

  pc = canvas->pc;

  pixmap = canvas->pixmap;
  canvas->pixmap = NULL;
  canvas->pc = NULL;
  gtk_plot_canvas_set_magnification(canvas, 1.0);
  canvas->pc = GTK_PLOT_PC(ps);
  canvas->pixmap = pixmap;

  gtk_plot_canvas_paint(canvas);

  canvas->pixmap = NULL;
  canvas->pc = NULL;
  gtk_plot_canvas_set_magnification(canvas, m);
  canvas->pixmap = pixmap;
  canvas->pc = pc;

  gtk_object_destroy(GTK_OBJECT(ps));

  return TRUE;
}

/**
 * gtk_plot_canvas_export_ps_with_size:
 * @canvas: a #GtkPlotCanvas .
 * @file_name:
 * @orient:
 * @epsflag:
 * @units:
 * @width:
 * @height:
 *
 * 
 *
 * Return value: 
 */
gboolean
gtk_plot_canvas_export_ps_with_size             (GtkPlotCanvas *canvas,
                                                 char *file_name,
                                                 GtkPlotPageOrientation orient,
                                                 gboolean epsflag,
                                                 GtkPlotUnits units,
                                                 gint width,
                                                 gint height)
{
  GtkPlotPC *pc;
  GtkPlotPS *ps;
  gdouble scalex, scaley;
  gdouble m;
  GdkPixmap *pixmap;

  m = canvas->magnification;

  ps = GTK_PLOT_PS(gtk_plot_ps_new_with_size(file_name, orient, epsflag, 
                                             units, 
                                             width, height, 
                                             1.0 , 1.0));

  if(orient == GTK_PLOT_PORTRAIT){
    scalex = (gfloat)ps->page_width / (gfloat)canvas->width;
    scaley = (gfloat)ps->page_height / (gfloat)canvas->height;
  }else{
    scalex = (gfloat)ps->page_width / (gfloat)canvas->height;
    scaley = (gfloat)ps->page_height / (gfloat)canvas->width;
  }

  gtk_plot_ps_set_scale(ps, scalex, scaley);

  pc = canvas->pc;

  pixmap = canvas->pixmap;
  canvas->pixmap = NULL;
  canvas->pc = NULL;
  gtk_plot_canvas_set_magnification(canvas, 1.0);
  canvas->pc = GTK_PLOT_PC(ps);
  canvas->pixmap = pixmap;

  gtk_plot_canvas_paint(canvas);

  canvas->pixmap = NULL;
  canvas->pc = NULL;
  gtk_plot_canvas_set_magnification(canvas, m);
  canvas->pixmap = pixmap;
  canvas->pc = pc;

  gtk_object_destroy(GTK_OBJECT(ps));

  return TRUE;
}

static void
recalc_pixels(GtkPlot *plot)
{
  GList *list;
  
  list = plot->data_sets;
  while(list){
    GtkPlotData *data;
    data = GTK_PLOT_DATA(list->data);
    if(GTK_IS_PLOT_SURFACE(data)){
      GtkPlotSurface *surface = GTK_PLOT_SURFACE(data);
      gint i;

      for(i = surface->dt->node_0; i < surface->dt->node_cnt; i++){
        GtkPlotDTnode *node;
        node = gtk_plot_dt_get_node(surface->dt,i);
        if(GTK_IS_PLOT3D(plot)){
          gtk_plot3d_get_pixel(GTK_PLOT3D(plot),
                               node->x, node->y, node->z,
                               &node->px, &node->py, &node->pz);
        } else {
          gtk_plot_get_pixel(plot,
                             node->x, node->y,
                             &node->px, &node->py);
          node->pz = 0.0;
        }
      }
    }
    list = list->next;
  }
}

/**
 * gtk_plot_canvas_export_cairo:
 * @canvas: a #GtkPlotCanvas .
 * @cairo:
 *
 * 
 *
 * Return value: 
 */

gboolean
gtk_plot_canvas_export_cairo                    (GtkPlotCanvas *canvas,
						 cairo_t *cairo)
{
  GdkPixmap *pixmap;
  GtkPlotPC *pc, *new_pc;
  gdouble m;

  m = canvas->magnification;
  pc = canvas->pc;
  new_pc = GTK_PLOT_PC(gtk_plot_cairo_new(cairo));

  pixmap = canvas->pixmap;
  canvas->pixmap = NULL;
  canvas->pc = NULL;
  gtk_plot_canvas_set_magnification(canvas, 1.);
  canvas->pc = new_pc;
  canvas->pixmap = pixmap;
  gtk_plot_canvas_paint(canvas);
  canvas->pc = pc;
  gtk_plot_canvas_set_magnification(canvas, m);
  g_object_unref(new_pc);

  return TRUE;
}


