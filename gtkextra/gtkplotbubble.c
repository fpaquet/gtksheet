/* gtkplotbubble - bubble plots widget for gtk+
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
#include "gtkplot.h"
#include "gtkplot3d.h"
#include "gtkplotpolar.h"
#include "gtkplotdata.h"
#include "gtkplotbubble.h"
#include "gtkpsfont.h"

#define P_(string) string 

static void gtk_plot_bubble_class_init 	(GtkPlotBubbleClass *klass);
static void gtk_plot_bubble_init 		(GtkPlotBubble *data);
static void gtk_plot_bubble_destroy 	(GtkObject *data);
static void gtk_plot_bubble_set_property       (GObject *object,
                                                guint            prop_id,
                                                const GValue    *value,
                                                GParamSpec      *pspec);
static void gtk_plot_bubble_get_property       (GObject *object,
                                                guint            prop_id,
                                                GValue          *value,
                                                GParamSpec      *pspec);
static void gtk_plot_bubble_get_legend_size(GtkPlotData *data, 
					 gint *width, gint *height);
static void gtk_plot_bubble_draw_legend	(GtkPlotData *data, 
					 gint x, gint y);
static void gtk_plot_bubble_draw_symbol	(GtkPlotData *data,
                                         gdouble x, 
					 gdouble y, 
					 gdouble z,
					 gdouble a,
                                         gdouble dx, 
					 gdouble dy, 
					 gdouble dz, 
					 gdouble da);

extern inline gint roundint (gdouble x);

enum {
  ARG_0,
  ARG_CENTERED,
  ARG_STYLE,
  ARG_WIDTH,
  ARG_LENGTH,
  ARG_SCALE_MAX,
  ARG_SIZE_MAX,
  ARG_SHOW_SCALE,
  ARG_LABEL_PRECISION,
  ARG_LABEL_STYLE,
  ARG_LABEL_PREFIX,
  ARG_LABEL_SUFFIX,
};

static GtkPlotDataClass *parent_class = NULL;

GtkType
gtk_plot_bubble_get_type (void)
{
  static GtkType data_type = 0;

  if (!data_type)
    {
      GtkTypeInfo data_info =
      {
	"GtkPlotBubble",
	sizeof (GtkPlotBubble),
	sizeof (GtkPlotBubbleClass),
	(GtkClassInitFunc) gtk_plot_bubble_class_init,
	(GtkObjectInitFunc) gtk_plot_bubble_init,
	/* reserved 1*/ NULL,
        /* reserved 2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      data_type = gtk_type_unique (gtk_plot_data_get_type(), &data_info);
    }
  return data_type;
}

static void
gtk_plot_bubble_class_init (GtkPlotBubbleClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkPlotDataClass *data_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  parent_class = gtk_type_class (gtk_plot_data_get_type ());

  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;
  data_class = (GtkPlotDataClass *) klass;
  

  gobject_class->set_property = gtk_plot_bubble_set_property;
  gobject_class->get_property = gtk_plot_bubble_get_property;
  object_class->destroy = gtk_plot_bubble_destroy;

  g_object_class_install_property(gobject_class,
                           ARG_SCALE_MAX,
  g_param_spec_double ("scale_max",
                           P_("Scale Max"),
                           P_("Scale Max."),
                           0, G_MAXDOUBLE, 0.0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));
  g_object_class_install_property(gobject_class,
                           ARG_SIZE_MAX,
  g_param_spec_int ("size_max",
                           P_("Size Max."),
                           P_("Size Max."),
                           0, G_MAXINT, 0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));
  g_object_class_install_property(gobject_class,
                           ARG_SHOW_SCALE,
  g_param_spec_boolean ("show_scale",
                           P_("Show Scale"),
                           P_("Show scale in gradient legend"),
                           FALSE,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));
  g_object_class_install_property(gobject_class,
                           ARG_LABEL_PRECISION,
  g_param_spec_int ("labels_precision",
                           P_("Labels Precision"),
                           P_("Labels Precision"),
                           0, G_MAXINT, 0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));
  g_object_class_install_property(gobject_class,
                           ARG_LABEL_STYLE,
  g_param_spec_int ("labels_style",
                           P_("Labels Style"),
                           P_("Labels Style"),
                           0, G_MAXINT, 0,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));
  g_object_class_install_property(gobject_class,
                           ARG_LABEL_PREFIX,
  g_param_spec_string ("labels_prefix",
                           P_("Labels Prefix"),
                           P_("Labels Prefix"),
			   NULL,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));
  g_object_class_install_property(gobject_class,
                           ARG_LABEL_SUFFIX,
  g_param_spec_string ("labels_suffix",
                           P_("Labels Suffix"),
                           P_("Labels Suffix"),
			   NULL,
                           G_PARAM_READABLE|G_PARAM_WRITABLE));

  data_class->draw_legend = gtk_plot_bubble_draw_legend;
  data_class->get_legend_size = gtk_plot_bubble_get_legend_size;
  data_class->draw_symbol = gtk_plot_bubble_draw_symbol;
}


static void
gtk_plot_bubble_init (GtkPlotBubble *dataset)
{
  GtkWidget *widget;
  GdkColor black, white;
  GdkColormap *colormap;
  GtkPlotArray *dim;

  widget = GTK_WIDGET(dataset);

  colormap = gdk_colormap_get_system();

  gdk_color_black(colormap, &black);
  gdk_color_white(colormap, &white);

  GTK_PLOT_DATA(dataset)->symbol.symbol_style = GTK_PLOT_SYMBOL_EMPTY;
  GTK_PLOT_DATA(dataset)->symbol.color = black;
  GTK_PLOT_DATA(dataset)->line.line_style = GTK_PLOT_LINE_SOLID;
  GTK_PLOT_DATA(dataset)->line.line_width = 1;
  GTK_PLOT_DATA(dataset)->line.color = black;

  dataset->size_max = 50;
  dataset->scale_max = 1.;
  dataset->show_scale = TRUE;

  dataset->labels_precision = 3;
  dataset->labels_style = GTK_PLOT_LABEL_FLOAT;
  dataset->labels_prefix = NULL;
  dataset->labels_suffix = NULL;
  
  dim = gtk_plot_data_find_dimension(GTK_PLOT_DATA(dataset), "y");
  gtk_plot_array_set_independent(dim, TRUE);
  dim = gtk_plot_data_find_dimension(GTK_PLOT_DATA(dataset), "z");
  gtk_plot_array_set_independent(dim, TRUE);
  dim = gtk_plot_data_find_dimension(GTK_PLOT_DATA(dataset), "a");
  gtk_plot_array_set_required(dim, TRUE);
}

static void
gtk_plot_bubble_set_property (GObject      *object,
                         guint            prop_id,
                         const GValue          *value,
                         GParamSpec      *pspec)
{
  GtkPlotBubble *data;

  data = GTK_PLOT_BUBBLE (object);

  switch (prop_id)
    {
      case ARG_SCALE_MAX:
        data->scale_max  = g_value_get_double(value);
        break;
      case ARG_SIZE_MAX:
        data->size_max  = g_value_get_int(value);
        break;
      case ARG_SHOW_SCALE:
        data->show_scale  = g_value_get_boolean(value);
        break;
      case ARG_LABEL_PRECISION:
        data->labels_precision  = g_value_get_int(value);
        break;
      case ARG_LABEL_STYLE:
        data->labels_style  = g_value_get_int(value);
        break;
      case ARG_LABEL_PREFIX:
        gtk_plot_bubble_set_labels_prefix(data, g_value_get_string(value));
        break;
      case ARG_LABEL_SUFFIX:
        gtk_plot_bubble_set_labels_suffix(data, g_value_get_string(value));
        break;
    }
}

static void
gtk_plot_bubble_get_property (GObject      *object,
                         guint            prop_id,
                         GValue          *value,
                         GParamSpec      *pspec)
{
  GtkPlotBubble *data;

  data = GTK_PLOT_BUBBLE (object);

  switch (prop_id)
    {
      case ARG_SCALE_MAX:
        g_value_set_double(value, data->scale_max);
        break;
      case ARG_SIZE_MAX:
        g_value_set_int(value, data->size_max);
        break;
      case ARG_SHOW_SCALE:
        g_value_set_boolean(value, data->show_scale);
        break;
      case ARG_LABEL_PRECISION:
        g_value_set_int(value, data->labels_precision);
        break;
      case ARG_LABEL_STYLE:
        g_value_set_int(value, data->labels_style);
        break;
      case ARG_LABEL_PREFIX:
        g_value_set_string(value, data->labels_prefix);
        break;
      case ARG_LABEL_SUFFIX:
        g_value_set_string(value, data->labels_suffix);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

GtkWidget*
gtk_plot_bubble_new ()
{
  GtkWidget *widget;

  widget = gtk_type_new (gtk_plot_bubble_get_type ());

  return (widget);
}

static void
gtk_plot_bubble_destroy(GtkObject *object)
{
  GtkPlotBubble *bubble = GTK_PLOT_BUBBLE(object);

  if(bubble->labels_prefix) g_free(bubble->labels_prefix);
  bubble->labels_prefix = NULL;
  if(bubble->labels_suffix) g_free(bubble->labels_suffix);
  bubble->labels_suffix = NULL;

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_plot_bubble_draw_symbol(GtkPlotData *dataset,
                          gdouble x, gdouble y, gdouble z, gdouble a,
                          gdouble dx, gdouble dy, gdouble dz, gdouble da)
{
  GtkPlot *plot;
  GtkPlotBubble *bubble = NULL;
  GdkRectangle area, clip_area;
  gdouble m;
  gdouble x1 = 0.0, y1 = 0.0;
  gdouble r;

  g_return_if_fail(GTK_IS_PLOT_BUBBLE(dataset));

  bubble = GTK_PLOT_BUBBLE(dataset);

  g_return_if_fail(dataset->plot != NULL);
  g_return_if_fail(GTK_WIDGET_VISIBLE(dataset->plot));

  plot = dataset->plot;

  m = plot->magnification;
  area.x = GTK_WIDGET(plot)->allocation.x;
  area.y = GTK_WIDGET(plot)->allocation.y;
  area.width = GTK_WIDGET(plot)->allocation.width;
  area.height = GTK_WIDGET(plot)->allocation.height;

  clip_area.x = area.x + roundint(plot->x * area.width);
  clip_area.y = area.y + roundint(plot->y * area.height);
  clip_area.width = roundint(plot->width * area.width);
  clip_area.height = roundint(plot->height * area.height);

/*
  gtk_plot_pc_clip(plot->pc, &clip_area);
*/

  r = fabs(a);
  r /= bubble->scale_max;
  r = r * bubble->size_max;
  dataset->symbol.size = r;

  if(GTK_IS_PLOT3D(plot)){
       gdouble z1 = 0.0;
       if(plot->clip_data && 
          (x < plot->xmin || x > plot->xmax || y <plot->ymin || y > plot->ymax || z < GTK_PLOT3D(plot)->zmin || z > GTK_PLOT3D(plot)->zmax))
            return;
       gtk_plot3d_get_pixel(GTK_PLOT3D(plot), x, y, z,
                            &x1, &y1, &z1);
  }else{
       if(plot->clip_data && !GTK_IS_PLOT_POLAR(plot) &&  
          (x < plot->xmin || x > plot->xmax || y <plot->ymin || y > plot->ymax))
            return;
       gtk_plot_get_pixel(plot, x, y, &x1, &y1);
  }
  gtk_plot_data_draw_symbol(dataset, x1, y1);

/*
  gtk_plot_pc_clip(plot->pc, NULL);
*/
}

static void
gtk_plot_bubble_get_legend_size(GtkPlotData *data, gint *width, gint *height)
{
  GtkPlotBubble *bubble;
  GtkPlot *plot = NULL;
  GtkPlotText legend;
  gint lascent, ldescent, lheight, lwidth;
  gdouble m;
  gchar new_label[100], text[100];

  bubble = GTK_PLOT_BUBBLE(data);
  plot = data->plot;

  m = plot->magnification;
  legend = plot->legends_attr;

  if(data->legend)
    legend.text = data->legend;
  else
    legend.text = "";

  *width = *height = 0;
  if(data->show_legend)
    gtk_plot_text_get_size(legend.text, legend.angle, legend.font,
                           roundint(legend.height * m), 
                           width, height,
                           &lascent, &ldescent);
  

  if(bubble->show_scale){
    gchar aux_text[100];
    gtk_plot_axis_parse_label(data->gradient, bubble->scale_max, bubble->labels_precision, bubble->labels_style, text);
    if(bubble->labels_prefix){
      g_snprintf(aux_text, 100, "%s%s", bubble->labels_prefix, text);
      g_snprintf(text, 100, aux_text);
    }
    if(bubble->labels_suffix){
      g_snprintf(aux_text, 100, "%s%s", text, bubble->labels_suffix);
      g_snprintf(text, 100, aux_text);
    }

    g_snprintf(new_label, 100, "%s", text);

    legend.text = new_label;
    gtk_plot_text_get_size(legend.text, 0, legend.font,
                           roundint(legend.height * m), 
                           &lwidth, &lheight,
                           &lascent, &ldescent);

    *width = MAX(*width, roundint(m*bubble->size_max));
    *width = MAX(*width, lwidth);
    *width += roundint(m * 8);
    *height += MAX(lheight , lheight + roundint(m*bubble->size_max));
  }
}


static void
gtk_plot_bubble_draw_legend(GtkPlotData *data, gint x, gint y)
{
  GtkPlotBubble *bubble;
  GtkPlot *plot = NULL;
  GtkPlotText legend;
  GdkRectangle area;
  gint lascent, ldescent, lheight, lwidth;
  gdouble m;
  gint line_width;
  GtkPlotSymbolStyle style;

  bubble = GTK_PLOT_BUBBLE(data);
  style = data->symbol.symbol_style;

  g_return_if_fail(data->plot != NULL);
  g_return_if_fail(GTK_IS_PLOT(data->plot));
  g_return_if_fail(GTK_WIDGET_VISIBLE(data->plot));

  plot = data->plot;
  area.x = GTK_WIDGET(plot)->allocation.x;
  area.y = GTK_WIDGET(plot)->allocation.y;
  area.width = GTK_WIDGET(plot)->allocation.width;
  area.height = GTK_WIDGET(plot)->allocation.height;

  m = plot->magnification;
  legend = plot->legends_attr;

  if(data->legend)
    legend.text = data->legend;
  else
    legend.text = "";

  gtk_plot_text_get_size(legend.text, legend.angle, legend.font,
                         roundint(legend.height * m), 
                         &lwidth, &lheight,
                         &lascent, &ldescent);

  if(data->show_legend){
    line_width = plot->legends_line_width;

    legend.x = (gdouble)(area.x + x) / (gdouble)area.width;
    legend.y = (gdouble)(area.y + y + lascent) / (gdouble)area.height;

    gtk_plot_draw_text(plot, legend);
  }
  if(bubble->show_scale){
    gchar new_label[100], text_max[100];
    gint cx;

    gtk_plot_axis_parse_label(data->gradient, bubble->scale_max, bubble->labels_precision, bubble->labels_style, text_max);
    if(bubble->labels_prefix){
      gchar aux_text[100];
      g_snprintf(aux_text, 100, "%s%s", bubble->labels_prefix, text_max);
      g_snprintf(text_max, 100, aux_text);
    }
    if(bubble->labels_suffix){
      gchar aux_text[100];
      g_snprintf(aux_text, 100, "%s%s", text_max, bubble->labels_suffix);
      g_snprintf(text_max, 100, aux_text);
    }
    g_snprintf(new_label, 100, "%s", text_max);

    gtk_plot_text_get_size(new_label, 0, legend.font,
                           roundint(legend.height * m), 
                           &lwidth, &lheight,
                           &lascent, &ldescent);

    cx = MAX(lwidth, roundint(m*bubble->size_max));
    cx = x + cx / 2;

/*
    data->symbol.symbol_style = GTK_PLOT_SYMBOL_EMPTY;
*/
    data->symbol.border.color = legend.fg;

    data->symbol.size = bubble->size_max; 
    gtk_plot_data_draw_symbol(data, 
                         area.x + cx, 
                         area.y + y + lheight + roundint(bubble->size_max*m / 2));
    gtk_plot_data_draw_symbol(data, 
                         area.x + cx, 
                         area.y + y + lheight + roundint(m*bubble->size_max/2.));

    y += MAX(lheight, roundint(m*bubble->size_max) + 2 * lheight);  
    legend.x = (gdouble)(area.x + cx) / (gdouble)area.width;
    legend.y = (gdouble)(area.y + y) / (gdouble)area.height;
    
    legend.text = new_label;
    legend.justification = GTK_JUSTIFY_CENTER;
    gtk_plot_draw_text(plot, legend);

    y += lheight;  

  } else
    y += lheight;  

  data->symbol.symbol_style = style;
}

void            
gtk_plot_bubble_show_scale        (GtkPlotBubble *bubble, gboolean show)
{
  bubble->show_scale = show;
}

void            
gtk_plot_bubble_set_scale_max     (GtkPlotBubble *bubble, gdouble scale_max)
{
  bubble->scale_max = fabs(scale_max);
}

void            
gtk_plot_bubble_set_size_max      (GtkPlotBubble *bubble, guint size_max)
{
  bubble->size_max = size_max;
}

void
gtk_plot_bubble_set_labels_precision (GtkPlotBubble *bubble, gint precision)
{
  bubble->labels_precision = precision;
}

void
gtk_plot_bubble_set_labels_style (GtkPlotBubble *bubble, GtkPlotLabelStyle style)
{
  bubble->labels_style = style;
}

void
gtk_plot_bubble_set_labels_prefix (GtkPlotBubble *bubble, const gchar *prefix)
{
  if(bubble->labels_prefix) g_free(bubble->labels_prefix);
  bubble->labels_prefix = NULL;
  if(prefix) bubble->labels_prefix = g_strdup(prefix);
}

void
gtk_plot_bubble_set_labels_suffix (GtkPlotBubble *bubble, const gchar *suffix)
{
  if(bubble->labels_suffix) g_free(bubble->labels_suffix);
  bubble->labels_suffix = NULL;
  if(suffix) bubble->labels_suffix = g_strdup(suffix);
}

