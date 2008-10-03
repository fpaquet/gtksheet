#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotbox.h"
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

gdouble function(GtkPlot *plot, GtkPlotData *data, gdouble x, gboolean *err)
{
 gdouble y;
 *err = FALSE;

 y = (-.5+.3*sin(3.*x)*sin(50.*x));
/* y = 100*pow(x,2);
 y = 1./(10*x);
*/

 return y;
}

gdouble gaussian(GtkPlot *plot, GtkPlotData *data, gdouble x, gboolean *err)
{
 gdouble y;
 *err = FALSE;
 y = .65*exp(-.5*pow(x-.5,2)/.02);

 return y;
}

/*
gint
select_item(GtkWidget *widget, GdkEvent *event, GtkPlotCanvasChild *item, 
            gpointer data)
{
  GtkWidget **widget_list = NULL;
  GtkWidget *active_widget = NULL;
  GtkWidget *canvas = NULL;
  gint n = 0;

  switch(item->type){
    case GTK_PLOT_CANVAS_TEXT:
        printf("Item selected: TEXT\n");
        break;
    case GTK_PLOT_CANVAS_TITLE:
        printf("Item selected: TITLE\n");
        break;
    case GTK_PLOT_CANVAS_LEGENDS:
        printf("Item selected: LEGENDS\n");
        break;
    case GTK_PLOT_CANVAS_PLOT:
        printf("Item selected: PLOT\n");
        break;
    case GTK_PLOT_CANVAS_AXIS:
        printf("Item selected: AXIS\n");
        break;
    case GTK_PLOT_CANVAS_DATA:
        printf("Item selected: DATA\n");
        printf("Active point: %d -> %f %f\n", 
                GTK_PLOT_CANVAS(widget)->active_point,
                GTK_PLOT_CANVAS(widget)->active_x, 
                GTK_PLOT_CANVAS(widget)->active_y); 
        break;
    case GTK_PLOT_CANVAS_NONE:
        printf("Item selected: NONE\n");
        break;
    case GTK_PLOT_CANVAS_CUSTOM:
        printf("This is a custom child. Isn't it neat?\n");
        break;
    default:
        ;
  }

  canvas = GTK_WIDGET(widget);
  widget_list = plots;
  active_widget = GTK_WIDGET(gtk_plot_canvas_get_active_plot(GTK_PLOT_CANVAS(widget)));

  return TRUE;
}
*/

gint
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
      gtk_signal_handler_block_by_func(GTK_OBJECT(buttons[n]), GTK_SIGNAL_FUNC(activate_plot), data);
      if(widget_list[n] == active_widget){
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[n]), TRUE);
      }else{
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[n]), FALSE);
      }
      gtk_signal_handler_unblock_by_func(GTK_OBJECT(buttons[n]), GTK_SIGNAL_FUNC(activate_plot), data);
                                                                                
      n++;
    }

  return TRUE;
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

 gtk_signal_connect(GTK_OBJECT(buttons[nlayers-1]), "toggled",
                    (GtkSignalFunc) activate_plot, canvas);

 plots[nlayers-1] = gtk_plot_new_with_size(NULL, .5, .25);
 gtk_widget_show(plots[nlayers-1]);

 activate_plot(buttons[nlayers-1],canvas);

 return plots[nlayers-1];
}


void
build_example1(GtkWidget *active_plot)
{
 GdkColor color1, color2;
 static gdouble px[]={0., 0.2, 0.4, 0.6, 0.8, 1.0};
 static gdouble py[]={.243,.045,.075,.213,.05,.124};
 static gdouble pz[]={.56, .12, .123, .5, .2, .21};
 static gdouble dx[]={.10, .18, .17, .16, .082, .34};
 static gdouble dy[]={.1,.1,.1,.1,.1,.1};

 dataset[0] = GTK_PLOT_DATA(gtk_plot_box_new(GTK_ORIENTATION_VERTICAL));

/*
 dataset[0] = GTK_PLOT_DATA(gtk_plot_data_new());
 gtk_plot_autoscale(GTK_PLOT(active_plot));
*/

 gtk_plot_add_data(GTK_PLOT(active_plot), dataset[0]);
 gtk_widget_show(GTK_WIDGET(dataset[0]));
 gtk_plot_data_set_points(dataset[0], px, py, dx, dy, 6);
 gtk_plot_data_set_z(dataset[0], pz);

 gtk_plot_data_show_zerrbars(dataset[0]);

 gdk_color_parse("red", &color1);
 gdk_color_alloc(gdk_colormap_get_system(), &color1); 
 gdk_color_parse("yellow", &color2);
 gdk_color_alloc(gdk_colormap_get_system(), &color2); 

 gtk_plot_data_set_symbol(dataset[0],
                          GTK_PLOT_SYMBOL_CIRCLE,
			  GTK_PLOT_SYMBOL_FILLED,
                          10, 2, &color2, &color1);
 gtk_plot_data_set_line_attributes(dataset[0],
                                   GTK_PLOT_LINE_NONE,
                                   0, 0, 1, &color1);
/*
 gtk_plot_data_set_x_attributes(dataset[0],
                                GTK_PLOT_LINE_SOLID,
                                0, &active_plot->style->black);
 gtk_plot_data_set_y_attributes(dataset[0],
                                GTK_PLOT_LINE_SOLID,
                                0, &active_plot->style->black);
*/

 gtk_plot_data_set_legend(dataset[0], "Boxes");

}

int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *active_plot;
 GtkWidget *canvas;
 GtkPlotCanvasChild *child;
 gint page_width, page_height;
 gfloat scale = 1.;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlotBox Demo");
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
 gtk_plot_set_range(GTK_PLOT(active_plot), -1. ,1., -1., 1.4);
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
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .06, .65, .31);
 gtk_widget_show(active_plot);

 build_example1(active_plot);

/*
 gtk_signal_connect(GTK_OBJECT(canvas), "select_item",
                    (GtkSignalFunc) select_item, NULL);
*/

 gtk_widget_show(window1);

 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "demoboxes.ps", 0, 0, 
                           GTK_PLOT_LETTER);
 
 gtk_main();

 return(0);
}

