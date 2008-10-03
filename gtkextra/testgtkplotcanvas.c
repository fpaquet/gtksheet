#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotcanvas.h"
#include "gtkplotcanvasrectangle.h"
#include "gtkplotcanvasellipse.h"
#include "gtkplotcanvasline.h"
#include "gtkplotcanvastext.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotcanvaspixmap.h"
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

gint
select_item(GtkWidget *widget, GdkEvent *event, GtkPlotCanvasChild *child, 
            gpointer data)
{
  gint n = 0;
  gdouble *x = NULL, *y = NULL;

  if(GTK_IS_PLOT_CANVAS_TEXT(child))
        printf("Item selected: TEXT\n");
  if(GTK_IS_PLOT_CANVAS_PIXMAP(child))
        printf("Item selected: PIXMAP\n");
  if(GTK_IS_PLOT_CANVAS_RECTANGLE(child))
        printf("Item selected: RECTANGLE\n");
  if(GTK_IS_PLOT_CANVAS_ELLIPSE(child))
        printf("Item selected: ELLIPSE\n");
  if(GTK_IS_PLOT_CANVAS_LINE(child))
        printf("Item selected: LINE\n");
  if(GTK_IS_PLOT_CANVAS_PLOT(child)){
    switch(GTK_PLOT_CANVAS_PLOT(child)->pos){
      case GTK_PLOT_CANVAS_PLOT_IN_TITLE:
        printf("Item selected: TITLE\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_LEGENDS:
        printf("Item selected: LEGENDS\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_PLOT:
        printf("Item selected: PLOT\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_AXIS:
        printf("Item selected: AXIS\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_MARKER:
        printf("Item selected: MARKER\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_GRADIENT:
        printf("Item selected: GRADIENT\n");
        break;
      case GTK_PLOT_CANVAS_PLOT_IN_DATA:
        x = gtk_plot_data_get_x(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
        y = gtk_plot_data_get_y(GTK_PLOT_CANVAS_PLOT(child)->data, &n); 
        n = GTK_PLOT_CANVAS_PLOT(child)->datapoint;
        printf("Item selected: DATA\n");
        printf("Active point: %d -> %f %f\n", 
                GTK_PLOT_CANVAS_PLOT(child)->datapoint, x[n], y[n]);
        break;
      default:
        break;
    }

  }

  return TRUE;
}

int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *active_plot;
 GtkWidget *canvas;
 GtkPlotCanvasChild *child = NULL;
 gint page_width, page_height;
 gfloat scale = 1.;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlotCanvas Demo");
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
 GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS|GTK_PLOT_CANVAS_CAN_SELECT);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1), canvas);
 gtk_widget_show(canvas);

/*
 gdk_color_parse("light blue", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas), &color);
*/

 active_plot = gtk_plot_new(NULL);
 gtk_widget_show(active_plot);
 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .1, .1, .8, .65);

 child = gtk_plot_canvas_rectangle_new(GTK_PLOT_LINE_SOLID, 1, NULL, NULL, GTK_PLOT_BORDER_SHADOW, FALSE);
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, 0.2, 0.2, 0.3, 0.5);

 child = gtk_plot_canvas_ellipse_new(GTK_PLOT_LINE_SOLID, 1, NULL, NULL, FALSE);
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, 0.5, 0.45, 0.75, 0.6);

 child = gtk_plot_canvas_line_new(GTK_PLOT_LINE_SOLID, 1, NULL, GTK_PLOT_CANVAS_ARROW_END);
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, 0.15, 0.4, 0.2, 0.6);

 child = gtk_plot_canvas_text_new("Helvetica", 24, 0, NULL, NULL, TRUE, GTK_JUSTIFY_CENTER, "This is a text box");
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, 0.5, 0.05, 0.6, 0.1);

 gtk_signal_connect(GTK_OBJECT(canvas), "select_item",
                    (GtkSignalFunc) select_item, NULL);

 gtk_widget_show(window1);

 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "democanvas.ps", 0, 0, 
                           GTK_PLOT_LETTER);

 gtk_main();

 return(0);
}

