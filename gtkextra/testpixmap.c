#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotpixmap.h"
#include "gtkplotcanvas.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotps.h"
#include "gtkplotprint.h"

GdkPixmap *pixmap=NULL;
GtkWidget **plots;
GtkWidget **buttons;
GtkPlotData *dataset[5];
gint nlayers = 0;


void
quit ()
{
  gtk_main_quit();
}


GtkWidget *
new_layer(GtkWidget *canvas)
{
 gchar label[10];
 GtkRequisition req;
 gint size;

 nlayers++;

 buttons = (GtkWidget **)g_realloc(buttons, nlayers * sizeof(GtkWidget *));
 plots = (GtkWidget **)g_realloc(plots, nlayers * sizeof(GtkWidget *));

 sprintf(label, "%d", nlayers);
 
 buttons[nlayers-1] = gtk_toggle_button_new_with_label(label);
/* gtk_button_set_relief(GTK_BUTTON(buttons[nlayers-1]), GTK_RELIEF_NONE);
*/
 gtk_widget_size_request(buttons[nlayers-1], &req);
 size = MAX(req.width,req.height);
 gtk_widget_set_usize(buttons[nlayers-1], size, size);
 gtk_fixed_put(GTK_FIXED(canvas), buttons[nlayers-1], (nlayers-1)*20, 0);
 gtk_widget_show(buttons[nlayers-1]);

 plots[nlayers-1] = gtk_plot_new_with_size(NULL, .5, .25);
 gtk_widget_show(plots[nlayers-1]);

 return plots[nlayers-1];
}


void
build_example1(GtkWidget *active_plot)
{
 GdkColormap *colormap;
 GdkPixmap *pixmap;
 GdkBitmap *mask;
 static gdouble px1[]={0., 0.2, 0.4, 0.6, 0.8, 1.0};
 static gdouble py1[]={.2, .4, .5, .35, .30, .40};
 static gdouble px2[]={0., 0.2, 0.4, 0.6, 0.8, 1.0};
 static gdouble py2[]={.12, .22, .27, .12, .52, .62};

 colormap = gdk_colormap_get_system();

 pixmap = gdk_pixmap_colormap_create_from_xpm(NULL, colormap, &mask, NULL,
                                              "cloud.xpm");

 dataset[0] = GTK_PLOT_DATA(gtk_plot_pixmap_new(pixmap, mask));
 gtk_plot_add_data(GTK_PLOT(active_plot), dataset[0]);
 gtk_widget_show(GTK_WIDGET(dataset[0]));
 gtk_plot_data_set_points(dataset[0], px1, py1, px2, py2, 6);
 gtk_plot_data_set_legend(dataset[0], "Pixmap 1");

 gdk_pixmap_unref(pixmap);
 gdk_bitmap_unref(mask);

 pixmap = gdk_pixmap_colormap_create_from_xpm(NULL, colormap, &mask, NULL,
                                              "suncloud.xpm");

 dataset[1] = GTK_PLOT_DATA(gtk_plot_pixmap_new(pixmap, mask));
 gtk_plot_add_data(GTK_PLOT(active_plot), dataset[1]);
 gtk_widget_show(GTK_WIDGET(dataset[1]));
 gtk_plot_data_set_points(dataset[1], px2, py2, NULL, NULL, 6);
 gtk_plot_data_set_legend(dataset[1], "Pixmap 2");

 gdk_pixmap_unref(pixmap);
 gdk_bitmap_unref(mask);
}

int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *active_plot;
 GtkWidget *canvas;
 gint page_width, page_height;
 gfloat scale = 1.;
 GtkPlotCanvasChild *child;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlotFlux Demo");
 gtk_widget_set_usize(window1,550,650);
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
 GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1), canvas);

/*
 gdk_color_parse("light blue", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas), &color);
*/

 gtk_widget_show(canvas);

 active_plot = new_layer(canvas);
 gtk_plot_set_range(GTK_PLOT(active_plot), 0. ,1., 0., 1.4);
 gtk_plot_legends_move(GTK_PLOT(active_plot), .500, .05);
 gtk_plot_set_legends_border(GTK_PLOT(active_plot), 0, 0);
 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP));
 gtk_plot_axis_show_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), 3), 15, 3);
 gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), 0), 1., 1);
 gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), 1), 1., 1);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP), TRUE);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
 gtk_plot_x0_set_visible(GTK_PLOT(active_plot), TRUE);
 gtk_plot_y0_set_visible(GTK_PLOT(active_plot), TRUE);


 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .06, .65, .35);
 gtk_widget_show(active_plot);

 build_example1(active_plot);

 gtk_widget_show(window1);

 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "demopixmap.ps", 0, 0, 
                           GTK_PLOT_LETTER);
 
 gtk_main();

 return(0);
}

