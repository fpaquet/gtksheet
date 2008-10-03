/* gtkplotsegment - 3d scientific plots widget for gtk+
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

#ifndef __GTK_PLOT_SEGMENT_H__
#define __GTK_PLOT_SEGMENT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gtkplot.h"

#define GTK_PLOT_SEGMENT(obj)        GTK_CHECK_CAST (obj, gtk_plot_segment_get_type (), GtkPlotSegment)
#define GTK_TYPE_PLOT_SEGMENT        (gtk_plot_segment_get_type ())
#define GTK_PLOT_SEGMENT_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_plot_segment_get_type(), GtkPlotSegmentClass)
#define GTK_IS_PLOT_SEGMENT(obj)     GTK_CHECK_TYPE (obj, gtk_plot_segment_get_type ())

typedef struct _GtkPlotSegment             GtkPlotSegment;
typedef struct _GtkPlotSegmentClass        GtkPlotSegmentClass;

typedef enum
{
      GTK_PLOT_ARROW_NONE            = 0,
      GTK_PLOT_ARROW_ORIGIN          = 1 << 0,
      GTK_PLOT_ARROW_END             = 1 << 1
} GtkPlotArrow;

struct _GtkPlotSegment
{
  GtkPlotData data;

  gboolean relative;
  gboolean centered;

  GtkPlotArrow arrow_mask;
  gint arrow_length;
  gint arrow_width;
  GtkPlotSymbolStyle arrow_style;
};

struct _GtkPlotSegmentClass
{
  GtkPlotDataClass parent_class;
};


GtkType		gtk_plot_segment_get_type		(void);
GtkWidget*	gtk_plot_segment_new		(void);
void		gtk_plot_segment_set_relative   (GtkPlotSegment *segment,
						 gboolean set);
gboolean	gtk_plot_segment_relative       (GtkPlotSegment *segment);
void            gtk_plot_segment_set_arrow         (GtkPlotSegment *segment,
                                                 gint arrow_length,
                                                 gint arrow_width,
                                                 GtkPlotSymbolStyle style);
void            gtk_plot_segment_get_arrow      (GtkPlotSegment *segment,
                                                 gint *arrow_length,
                                                 gint *arrow_width,
                                                 GtkPlotSymbolStyle *style);
void            gtk_plot_segment_set_arrow_mask (GtkPlotSegment *segment,
                                                 GtkPlotArrow mask);
guint   	gtk_plot_segment_get_arrow_mask	(GtkPlotSegment *segment);
void            gtk_plot_segment_center         (GtkPlotSegment *segment,
                                                 gboolean center);
gboolean        gtk_plot_segment_is_centered    (GtkPlotSegment *segment);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_PLOT_SEGMENT_H__ */
