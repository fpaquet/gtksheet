#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotcanvas.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotcanvastext.h"

GdkPixmap *pixmap=NULL;
GtkWidget **plots;
GtkPlotData *dataset;
gint nlayers = 0;
gint32 timer;
GtkWidget *canvas;
gdouble *px;
gdouble *py;
static gchar point_label[100];

void
quit ()
{
  gtk_main_quit();
  g_free(plots);
}


GtkWidget *
new_layer(GtkWidget *canvas)
{
 gchar label[10];

 nlayers++;

 plots = (GtkWidget **)g_realloc(plots, nlayers * sizeof(GtkWidget *));

 sprintf(label, "%d", nlayers);
 
 plots[nlayers-1] = gtk_plot_new_with_size(NULL, .65, .45);

 return plots[nlayers-1];
}

void iter 				(GtkPlot *plot,
                                         GtkPlotData *data,
                                         gint iter,
                                         gdouble *x,
                                         gdouble *y,
                                         gdouble *z,
                                         gdouble *a,
                                         gdouble *dx,
                                         gdouble *dy,
                                         gdouble *dz,
                                         gdouble *da,
                                         gchar **label,
                                         gboolean *error)
{
 *x = iter;
 *y = rand()%10/10.;
 sprintf(point_label, "%d\n", iter);
 *label = point_label;
 *error = FALSE;
}


int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *active_plot;
 GdkColor color;
 gint page_width, page_height;
 gfloat scale = 1.;
 GtkPlotCanvasChild *child;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlot Real Time Demo");
 gtk_widget_set_usize(window1,550,600);
 gtk_container_border_width(GTK_CONTAINER(window1),0);

 gtk_signal_connect (GTK_OBJECT (window1), "destroy",
		     GTK_SIGNAL_FUNC (quit), NULL);

 vbox1=gtk_vbox_new(FALSE,0);
 gtk_container_add(GTK_CONTAINER(window1),vbox1);
 gtk_widget_show(vbox1);

 scrollw1=gtk_scrolled_window_new(NULL, NULL);
 gtk_container_border_width(GTK_CONTAINER(scrollw1),0);
 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw1),
				GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
 gtk_box_pack_start(GTK_BOX(vbox1),scrollw1, TRUE, TRUE,0);
 gtk_widget_show(scrollw1);

 canvas = gtk_plot_canvas_new(page_width, page_height, 1.);
 GTK_PLOT_CANVAS_UNSET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1), canvas);

 gdk_color_parse("light blue", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas), &color);

 gtk_widget_show(canvas);

 active_plot = new_layer(canvas);
 gdk_color_parse("light yellow", &color);
 gdk_color_alloc(gtk_widget_get_colormap(active_plot), &color);
 gtk_plot_set_background(GTK_PLOT(active_plot), &color);

 gdk_color_parse("white", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_legends_set_attributes(GTK_PLOT(active_plot),
                                 NULL, 0,
				 NULL,
                                 &color);
 gtk_plot_set_range(GTK_PLOT(active_plot), 0. ,20., 0., 1.);
 gtk_plot_set_ticks(GTK_PLOT(active_plot), GTK_PLOT_AXIS_X, 2, 1);
 gtk_plot_set_ticks(GTK_PLOT(active_plot), GTK_PLOT_AXIS_Y, .1, 1);

 gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(active_plot), 2), 0, 0);
 gtk_plot_axis_set_labels_style(gtk_plot_get_axis(GTK_PLOT(active_plot), 3), 0, 0);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP), TRUE);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
 gtk_plot_grids_set_visible(GTK_PLOT(active_plot), TRUE, TRUE, TRUE, TRUE);

 gtk_widget_show(active_plot);
 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .15, .80, .65);

 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP));
 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT));
 gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_LEFT), "Intensity");
 gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_BOTTOM), "Point");
 gtk_plot_set_legends_border(GTK_PLOT(active_plot), 2, 3);
 gtk_plot_legends_move(GTK_PLOT(active_plot), .60, .10);

 gtk_widget_show(window1);

 child = gtk_plot_canvas_text_new("Times-BoldItalic", 20, 0, NULL, NULL, TRUE,
                          GTK_JUSTIFY_CENTER, 
                          "Iterator Demo");
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .45, .05, 0., 0.); 

 dataset = GTK_PLOT_DATA(gtk_plot_data_new_iterator((GtkPlotIterator)iter,
						     20,
						     GTK_PLOT_DATA_X|GTK_PLOT_DATA_Y|GTK_PLOT_DATA_LABELS));
 gtk_plot_data_show_labels(dataset, TRUE);

 gtk_plot_add_data(GTK_PLOT(active_plot), dataset);

 gtk_widget_show(GTK_WIDGET(dataset));

 gdk_color_parse("red", &color);
 gdk_color_alloc(gdk_colormap_get_system(), &color);

 gtk_plot_data_set_legend(dataset, "Iterator");
 gtk_plot_data_set_symbol(dataset,
                             GTK_PLOT_SYMBOL_DIAMOND,
                             GTK_PLOT_SYMBOL_OPAQUE,
                             10, 2, &color, &color);
 gtk_plot_data_set_line_attributes(dataset,
                                   GTK_PLOT_LINE_SOLID,
                                   0, 0, 1, &color);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));

/*
 gtk_plot_clip_data(GTK_PLOT(active_plot), TRUE);
*/

 gtk_main();

 return(0);
}


