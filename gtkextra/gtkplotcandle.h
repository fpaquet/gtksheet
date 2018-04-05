/* gtkplotcandle - 3d scientific plots widget for gtk+
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

#ifndef __GTK_PLOT_CANDLE_H__
#define __GTK_PLOT_CANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gtkplot.h"

#define GTK_PLOT_CANDLE(obj)        G_TYPE_CHECK_INSTANCE_CAST (obj, gtk_plot_candle_get_type (), GtkPlotCandle)
#define G_TYPE_PLOT_CANDLE        (gtk_plot_candle_get_type ())
#define GTK_PLOT_CANDLE_CLASS(klass) G_TYPE_CHECK_CLASS_CAST (klass, gtk_plot_candle_get_type(), GtkPlotCandleClass)
#define GTK_IS_PLOT_CANDLE(obj)     G_TYPE_CHECK_INSTANCE_TYPE (obj, gtk_plot_candle_get_type ())

typedef struct _GtkPlotCandle             GtkPlotCandle;
typedef struct _GtkPlotCandleClass        GtkPlotCandleClass;

/**
 * GtkPlotCandle:
 *
 * The GtkPlotCandle struct contains only private data.
 * It should only be accessed through the functions described below.
 */
struct _GtkPlotCandle
{
  GtkPlotData data;
};

struct _GtkPlotCandleClass
{
  GtkPlotDataClass parent_class;
};


GType		gtk_plot_candle_get_type	(void);
GtkWidget*	gtk_plot_candle_new		(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_PLOT_CANDLE_H__ */
