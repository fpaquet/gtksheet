#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplot3d.h"
#include "gtkplotdata.h"
#include "gtkplotsurface.h"
#include "gtkplotcanvas.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotcanvastext.h"
#include "gtkplotcanvasrectangle.h"
#include "gtkplotcanvasellipse.h"
#include "gtkplotcanvasline.h"
#include "gtkplotcanvaspixmap.h"
#include "gtkplotcsurface.h"
#include "gtkplotps.h"
#include "gtkplotprint.h"

GdkPixmap *pixmap=NULL;
GtkWidget **plots;
GtkWidget **buttons;
GtkPlotData *dataset[5];
GtkPlot3D *plot3d;
gint nlayers = 0;
GtkWidget *active_plot;

static void activate_plot(GtkWidget *widget, gpointer data);

void
quit ()
{
  gtk_main_quit();
}

gdouble function(GtkPlot *plot, 
                 GtkPlotData *data, 
                 gdouble x, gdouble y, gboolean *err)
{
 gdouble z;
 *err = FALSE;

 z = cos(((x-0.5)*(x-0.5) + (y-0.5)*(y-0.5))*24) / 3. + .5;
 return z;
}

gint
select_item(GtkWidget *widget, GdkEvent *event, GtkPlotCanvasChild *child, 
            gpointer data)
{
  GtkWidget **widget_list = NULL;
  GtkWidget *active_widget = NULL;
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

    widget_list = plots;
    active_widget = GTK_WIDGET(GTK_PLOT_CANVAS_PLOT(child)->plot);
  }

  return TRUE;
}

static void
activate_plot(GtkWidget *widget, gpointer data)
{
  GtkWidget **widget_list = NULL;
  GtkWidget *active_widget = NULL;
  GtkWidget *canvas = NULL;
  gint n = 0;

  canvas = GTK_WIDGET(data);
  widget_list = buttons;
  active_widget = widget;

  while(n < nlayers)
    {
      g_signal_handlers_block_by_func(GTK_OBJECT(buttons[n]), G_CALLBACK(activate_plot), data);
      if(widget_list[n] == active_widget){
            active_plot = plots[n];
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[n]), TRUE);
      }else{
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[n]), FALSE);
      }
      g_signal_handlers_unblock_by_func(GTK_OBJECT(buttons[n]), G_CALLBACK(activate_plot), data);
      n++;
    }
}

GtkWidget *
new_layer(GtkWidget *canvas, GType type)
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
 gtk_widget_set_size_request(buttons[nlayers-1], size, size);
 gtk_fixed_put(GTK_FIXED(canvas), buttons[nlayers-1], (nlayers-1)*size, 0);
 gtk_widget_show(buttons[nlayers-1]);


 if(type == G_TYPE_PLOT3D)
   plots[nlayers-1] = gtk_plot3d_new_with_size(NULL, .50, .50);
 else
   plots[nlayers-1] = gtk_plot_new_with_size(NULL, .40, .40);

 gtk_widget_show(plots[nlayers-1]);
 g_signal_connect(GTK_OBJECT(buttons[nlayers-1]), "toggled", G_CALLBACK(activate_plot), NULL);

 return plots[nlayers-1];
}

void
rotate_x(GtkWidget *button, gpointer data)
{
 if(!GTK_IS_PLOT3D(active_plot)) return;

 gtk_plot3d_rotate_x(GTK_PLOT3D(active_plot), 10.);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(data));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(data));
}

void
rotate_y(GtkWidget *button, gpointer data)
{
 if(!GTK_IS_PLOT3D(active_plot)) return;

 gtk_plot3d_rotate_y(GTK_PLOT3D(active_plot), 10.);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(data));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(data));
}

void
rotate_z(GtkWidget *button, gpointer data)
{
 if(!GTK_IS_PLOT3D(active_plot)) return;

 gtk_plot3d_rotate_z(GTK_PLOT3D(active_plot), 10.);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(data));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(data));
}


int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *canvas;
 GtkWidget *button;
 GtkWidget *surface;
 GtkPlotCanvasChild *child;
 gint page_width, page_height;
 gfloat scale = 1.;
 gint n;
 static gdouble *x, *y, *z;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlot3D Demo");
 gtk_widget_set_size_request(window1,550,650);
 gtk_container_border_width(GTK_CONTAINER(window1),0);

 g_signal_connect (GTK_OBJECT (window1), "destroy",
		     G_CALLBACK (quit), NULL);

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

 plot3d = GTK_PLOT3D(new_layer(canvas, G_TYPE_PLOT3D));
 child = gtk_plot_canvas_plot_new(GTK_PLOT(plot3d));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .16, .05, .65, .45);
 gtk_widget_show(GTK_WIDGET(plot3d));

 x = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 y = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 z = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 n = 0;

/*
 for(nx = 0; nx < 40; nx++)
   for(ny = 0; ny < 40; ny++)
   {
     x[n]=0.025*(gdouble)nx;
     y[n]=0.025*(gdouble)ny;
     z[n]=cos(((x[n]-0.5)*(x[n]-0.5) + (y[n]-0.5)*(y[n]-0.5))*24) / 4. + .5;
     n++;
   }

 surface = gtk_plot_csurface_new();
 gtk_plot_surface_set_points(GTK_PLOT_SURFACE(surface), 
                             x, y, z, NULL, NULL, NULL, 40, 40); 
 gtk_plot_data_set_legend(GTK_PLOT_DATA(surface), "cos ((r-r\\s0\\N)\\S2\\N)");
*/
/*
 for(nx = 0; nx < 10; nx++)
   for(ny = 0; ny < 10; ny++)
   {
     x[n]=(gdouble)0.1*(gdouble)nx;
     y[n]=(gdouble)0.1*(gdouble)ny;
     z[n]=(x[n]-0.5)*(x[n]-0.5) + (y[n]-0.5)*(y[n]-0.5);
     z[n]=x[n]*x[n] + y[n]*y[n];
     n++;
   }

 surface = gtk_plot_csurface_new();
 gtk_plot_surface_set_points(GTK_PLOT_SURFACE(surface), 
                             x, y, z, NULL, NULL, NULL, 10, 10); 

*/

 gtk_plot3d_set_minor_ticks(GTK_PLOT3D(plot3d), GTK_PLOT_AXIS_X, 3);
 gtk_plot3d_set_minor_ticks(GTK_PLOT3D(plot3d), GTK_PLOT_AXIS_Y, 3);

 gtk_plot3d_minor_grids_set_visible(GTK_PLOT3D(plot3d), FALSE, FALSE, FALSE);


 surface = gtk_plot_csurface_new_function((GtkPlotFunc3D) function);

 gtk_plot_surface_set_xstep(GTK_PLOT_SURFACE(surface), .05);
 gtk_plot_surface_set_ystep(GTK_PLOT_SURFACE(surface), .05);

 gtk_plot_data_set_legend(GTK_PLOT_DATA(surface), "cos ((r-r\\s0\\N)\\S2\\N)");

 gtk_plot_add_data(GTK_PLOT(plot3d), GTK_PLOT_DATA(surface));
 gtk_widget_show(surface);

 gtk_plot_data_set_gradient(GTK_PLOT_DATA(surface), .2, .8, 6, 0);
 gtk_plot_data_set_gradient(GTK_PLOT_DATA(surface), 0.00001, 0.99999, 10, 0);
/* 
 gtk_plot_surface_set_transparent(GTK_PLOT_SURFACE(surface), TRUE);
*/
 gtk_plot_surface_set_transparent(GTK_PLOT_SURFACE(surface), FALSE);
 gtk_plot_csurface_set_projection(GTK_PLOT_CSURFACE(surface), GTK_PLOT_PROJECT_EMPTY);
 GTK_PLOT_CSURFACE(surface)->levels_line.line_style = GTK_PLOT_LINE_NONE;
 GTK_PLOT_CSURFACE(surface)->sublevels_line.line_style = GTK_PLOT_LINE_NONE;
/*******************/
 active_plot = new_layer(canvas, G_TYPE_PLOT);
 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .26, .54, .65, .90);

 gtk_plot_axis_set_minor_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_LEFT), 1);
 gtk_plot_axis_set_minor_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_BOTTOM), 1);

 surface = gtk_plot_csurface_new_function((GtkPlotFunc3D) function);

 gtk_plot_surface_set_xstep(GTK_PLOT_SURFACE(surface), .050);
 gtk_plot_surface_set_ystep(GTK_PLOT_SURFACE(surface), .050);

 gtk_plot_data_set_legend(GTK_PLOT_DATA(surface), "cos ((r-r\\s0\\N)\\S2\\N)");
 gtk_plot_add_data(GTK_PLOT(active_plot), GTK_PLOT_DATA(surface));
 gtk_widget_show(surface);

 gtk_plot_data_set_gradient(GTK_PLOT_DATA(surface), .2, .8, 3, 0);

 gtk_plot_surface_set_grid_visible(GTK_PLOT_SURFACE(surface), FALSE);
 gtk_plot_surface_set_transparent(GTK_PLOT_SURFACE(surface), TRUE);
 gtk_plot_csurface_set_projection(GTK_PLOT_CSURFACE(surface), GTK_PLOT_PROJECT_FULL);

/*
 gtk_plot_csurface_set_lines_visible(GTK_PLOT_CSURFACE(surface), FALSE);
*/


 button = gtk_button_new_with_label("Rotate X");
 gtk_fixed_put(GTK_FIXED(canvas), button, 80, 0);
 gtk_widget_show(button);

 g_signal_connect(GTK_OBJECT(button), "clicked",
                    G_CALLBACK(rotate_x), canvas);

 button = gtk_button_new_with_label("Rotate Y");
 gtk_fixed_put(GTK_FIXED(canvas), button, 160, 0);
 gtk_widget_show(button);

 g_signal_connect(GTK_OBJECT(button), "clicked",
                    G_CALLBACK(rotate_y), canvas);

 button = gtk_button_new_with_label("Rotate Z");
 gtk_fixed_put(GTK_FIXED(canvas), button, 240, 0);
 gtk_widget_show(button);

 g_signal_connect(GTK_OBJECT(button), "clicked",
                    G_CALLBACK(rotate_z), canvas);

 g_signal_connect(GTK_OBJECT(canvas), "select_item",
                    (void *) select_item, NULL);
                                                                                
 gtk_widget_show(window1);

 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "democsurface.ps", 0, 0,
                           GTK_PLOT_LETTER);

 gtk_main();

 return(0);
}

