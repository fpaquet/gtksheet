/* gtkplotpc - gtkplot print context - a renderer for printing functions
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

#ifndef __GTK_PLOT_CAIRO_H__
#define __GTK_PLOT_CAIRO_H__

#include <stdio.h>
#include <pango/pango.h>
#include "gtkplotpc.h"
#include "gtkpsfont.h"
#include "cairo.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_PLOT_CAIRO(obj)        G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_plot_cairo_get_type (), GtkPlotCairo)
#define G_TYPE_PLOT_CAIRO   (gtk_plot_cairo_get_type ())

#define GTK_PLOT_CAIRO_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, gtk_plot_cairo_get_type(), GtkPlotCairoClass)
#define GTK_IS_PLOT_CAIRO(obj)     G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_plot_cairo_get_type ())


    typedef struct _GtkPlotCairo GtkPlotCairo;
    typedef struct _GtkPlotCairoClass GtkPlotCairoClass;

   
/**
 * GtkPlotCairo:
 *
 * The GtkPlotCairo struct contains only private data.
 * It should only be accessed through the functions described below.
 */
    struct _GtkPlotCairo
    {
        GtkPlotPC pc;

        cairo_t *cairo;
        PangoContext *context;
        PangoLayout *layout;
        gboolean destroy_cairo;

        gint ref_count;
    };


    struct _GtkPlotCairoClass
    {
        GtkPlotPCClass parent_class;

    };

    GType    gtk_plot_cairo_get_type			(void);
    GtkObject *gtk_plot_cairo_new (cairo_t *cairo);
    GtkObject *gtk_plot_cairo_new_with_drawable (GdkDrawable *drawable);
    void gtk_plot_cairo_construct(GtkPlotCairo *pc,
                                  cairo_t *cairo,
                                  PangoContext *context);
    void gtk_plot_cairo_set_cairo(GtkPlotCairo *pc,
                                  cairo_t *cairo);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_PLOT_CAIRO_H__ */

