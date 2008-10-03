/* gtkplotbubble - 3d scientific plots widget for gtk+
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

#ifndef __GTK_PLOT_BUBBLE_H__
#define __GTK_PLOT_BUBBLE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gtkplot.h"

#define GTK_PLOT_BUBBLE(obj)        GTK_CHECK_CAST (obj, gtk_plot_bubble_get_type (), GtkPlotBubble)
#define GTK_TYPE_PLOT_BUBBLE        (gtk_plot_bubble_get_type ())
#define GTK_PLOT_BUBBLE_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_plot_bubble_get_type(), GtkPlotBubbleClass)
#define GTK_IS_PLOT_BUBBLE(obj)     GTK_CHECK_TYPE (obj, gtk_plot_bubble_get_type ())

typedef struct _GtkPlotBubble             GtkPlotBubble;
typedef struct _GtkPlotBubbleClass        GtkPlotBubbleClass;


struct _GtkPlotBubble
{
  GtkPlotData data;

  gdouble scale_max;
  guint size_max;

  gboolean show_scale;

  gint labels_precision;
  gint labels_style;

  gchar *labels_prefix;
  gchar *labels_suffix;
};

struct _GtkPlotBubbleClass
{
  GtkPlotDataClass parent_class;
};


GtkType		gtk_plot_bubble_get_type		(void);
GtkWidget*	gtk_plot_bubble_new		(void);
void		gtk_plot_bubble_show_scale	(GtkPlotBubble *bubble,
						 gboolean show);
void		gtk_plot_bubble_set_scale_max	(GtkPlotBubble *bubble,
						 gdouble scale_max);
void		gtk_plot_bubble_set_size_max	(GtkPlotBubble *bubble,
						 guint size_max);
void		gtk_plot_bubble_set_labels_precision
                                                (GtkPlotBubble *bubble,
						 gint precision);
void		gtk_plot_bubble_set_labels_style  (GtkPlotBubble *bubble,
						 GtkPlotLabelStyle style);
void		gtk_plot_bubble_set_labels_prefix (GtkPlotBubble *bubble,
						 const gchar *prefix);
void		gtk_plot_bubble_set_labels_suffix (GtkPlotBubble *bubble,
						 const gchar *suffix);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_PLOT_BUBBLE_H__ */
