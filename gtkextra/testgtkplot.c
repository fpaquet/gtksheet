#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "icons.h"
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotbar.h"
#include "gtkplotcanvas.h"
#include "gtkplotcanvastext.h"
#include "gtkplotcanvasline.h"
#include "gtkplotcanvasellipse.h"
#include "gtkplotcanvasrectangle.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotcanvaspixmap.h"
#include "gtkplotps.h"
#include "gtkplotprint.h"
#include "gtkplotcairo.h"

GdkPixmap *pixmap;
GtkWidget **plots;
GtkWidget **buttons;
GtkPlotData *dataset[5];
gint nlayers = 0;
GtkWidget *active_plot;

static void put_child(GtkPlotCanvas *canvas, gdouble x, gdouble y);


static void
draw_page (GtkPrintOperation *operation,
           GtkPrintContext   *context,
           gint               page_nr,
           gpointer           user_data)
{
  cairo_t *cr;
  GdkPixmap *pixmap;
  GtkPlotCanvas *canvas = GTK_PLOT_CANVAS(user_data);

  cr = gtk_print_context_get_cairo_context (context);

  double width = gtk_print_context_get_width (context);
  double height = gtk_print_context_get_height (context);
  cairo_scale (cr, width/canvas->pixmap_width, height/canvas->pixmap_height);
  gtk_plot_canvas_export_cairo(canvas, cr);
}

void
test_print(GtkPlotCanvas *canvas, GtkWidget *window)
{
  GtkPrintOperation *operation;
  GError *error = NULL;
  operation = gtk_print_operation_new();

  g_signal_connect (G_OBJECT (operation), "draw-page",
                    G_CALLBACK (draw_page), canvas);

  gtk_print_operation_set_n_pages (operation, 1);

  gtk_print_operation_run (operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (window), &error);

  if (error)
    {
      GtkWidget *dialog;

      dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                       GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_ERROR,
                                       GTK_BUTTONS_CLOSE,
                                       "%s", error->message);

      g_signal_connect (dialog, "response",
                        G_CALLBACK (gtk_widget_destroy), NULL);

      gtk_widget_show (dialog);

      g_error_free (error);
    }

  g_object_unref(operation);
}


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
move_item(GtkWidget *widget, GtkPlotCanvasChild *item,
          gdouble x, gdouble y, gpointer data)
{
  GtkPlotCanvas *canvas = NULL;
                                                                                
  canvas = GTK_PLOT_CANVAS(widget);
                                                                                
  if(GTK_IS_PLOT_CANVAS_PLOT(item) && GTK_PLOT_CANVAS_PLOT(item)->pos == GTK_PLOT_CANVAS_PLOT_IN_DATA){
      gdouble *array_x = NULL;
      gdouble *array_y = NULL;
      gint n;
      array_x = gtk_plot_data_get_x(GTK_PLOT_CANVAS_PLOT(item)->data, &n);
      array_y = gtk_plot_data_get_y(GTK_PLOT_CANVAS_PLOT(item)->data, &n);
      printf("MOVING DATA\n");
      printf("Active point: %d", GTK_PLOT_CANVAS_PLOT(item)->datapoint);
  }
                                                                                
  return TRUE;
}
*/

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

  return FALSE;
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
 gtk_widget_set_size_request(buttons[nlayers-1], size, size); 
 gtk_fixed_put(GTK_FIXED(canvas), buttons[nlayers-1], (nlayers-1)*size, 0);
 gtk_widget_show(buttons[nlayers-1]);

 g_signal_connect(GTK_OBJECT(buttons[nlayers-1]), "toggled",
                    (void *) activate_plot, canvas);

 plots[nlayers-1] = gtk_plot_new_with_size(NULL, .5, .25);
 gtk_widget_show(plots[nlayers-1]);

 activate_plot(buttons[nlayers-1],canvas);

 return plots[nlayers-1];
}

gboolean
my_tick_label(GtkPlotAxis *axis, gdouble *tick_value, gchar *label, gpointer data)
{
  gboolean return_value = FALSE;

  if(*tick_value == 0.0){
    g_snprintf(label, 100, "custom label at 0.0");
    return_value = TRUE;
  }
  return return_value;
}

void
build_example1(GtkWidget *plot)
{
 GdkColor color;
 GtkPlotAxis *axis;

 static gdouble px1[]={0., 0.2, 0.4, 0.6, 0.8, 1.0};
 static gdouble py1[]={.2, .4, .5, .35, .30, .40};
 static gdouble dx1[]={.2, .2, .2, .2, .2, .2};
 static gdouble dy1[]={.1, .1, .1, .1, .1, .1};

 static gdouble px2[]={0., -0.2, -0.4, -0.6, -0.8, -1.0};
 static gdouble py2[]={.2, .4, .5, .35, .30, .40};
 static gdouble dx2[]={.2, .2, .2, .2, .2, .2};
 static gdouble dy2[]={.1, .1, .1, .1, .1, .1};

 /* CUSTOM TICK LABELS */

 gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT), TRUE);
 axis = gtk_plot_get_axis(GTK_PLOT(plot), GTK_PLOT_AXIS_RIGHT);
 g_signal_connect(GTK_OBJECT(axis), "tick_label", 
                    G_CALLBACK(my_tick_label), NULL);

 dataset[0] = GTK_PLOT_DATA(gtk_plot_data_new());
 gtk_plot_add_data(GTK_PLOT(plot), dataset[0]);
 gtk_widget_show(GTK_WIDGET(dataset[0]));
 gtk_plot_data_set_points(dataset[0], px1, py1, dx1, dy1, 6);

 gtk_plot_data_add_marker(dataset[0], 3);
/*
 gtk_plot_data_gradient_set_visible(dataset[0], TRUE);
*/

 gdk_color_parse("red", &color);
 gdk_color_alloc(gdk_colormap_get_system(), &color); 

 gtk_plot_data_set_symbol(dataset[0],
                          GTK_PLOT_SYMBOL_DIAMOND,
			  GTK_PLOT_SYMBOL_EMPTY,
                          10, 2, &color, &color);
 gtk_plot_data_set_line_attributes(dataset[0],
                                   GTK_PLOT_LINE_SOLID,
                                   0, 0, 1, &color);

 gtk_plot_data_set_connector(dataset[0], GTK_PLOT_CONNECT_SPLINE);

 gtk_plot_data_show_yerrbars(dataset[0]);
 gtk_plot_data_set_legend(dataset[0], "Spline + EY");

 dataset[3] = GTK_PLOT_DATA(gtk_plot_data_new());
 gtk_plot_add_data(GTK_PLOT(plot), dataset[3]);
 gtk_widget_show(GTK_WIDGET(dataset[3]));
 gtk_plot_data_set_points(dataset[3], px2, py2, dx2, dy2, 6);
 gtk_plot_data_set_symbol(dataset[3],
                             GTK_PLOT_SYMBOL_SQUARE,
			     GTK_PLOT_SYMBOL_OPAQUE,
                             8, 2, 
                             &gtk_widget_get_style(plot)->black,
                             &gtk_widget_get_style(plot)->black);
 gtk_plot_data_set_line_attributes(dataset[3],
                                   GTK_PLOT_LINE_SOLID,
                                   0, 0, 4, &color);
 gtk_plot_data_set_connector(dataset[3], GTK_PLOT_CONNECT_STRAIGHT);

 gtk_plot_data_set_x_attributes(dataset[3], 
                                GTK_PLOT_LINE_SOLID,
                                0, 0, 0, &gtk_widget_get_style(plot)->black);
 gtk_plot_data_set_y_attributes(dataset[3], 
                                GTK_PLOT_LINE_SOLID,
                                0, 0, 0, &gtk_widget_get_style(plot)->black);

 gtk_plot_data_set_legend(dataset[3], "Line + Symbol");

 
 gdk_color_parse("blue", &color);
 gdk_color_alloc(gdk_colormap_get_system(), &color); 

 dataset[1] = gtk_plot_add_function(GTK_PLOT(plot), (GtkPlotFunc)function);
 gtk_widget_show(GTK_WIDGET(dataset[1]));
 gtk_plot_data_set_line_attributes(dataset[1],
                                   GTK_PLOT_LINE_SOLID,
                                   0, 0, 0, &color);

 gtk_plot_data_set_legend(dataset[1], "Function Plot");
}

void
build_example2(GtkWidget *plot)
{
 GdkColor color;
 static double px2[] = {.1, .2, .3, .4, .5, .6, .7, .8};
 static double py2[] = {.012, .067, .24, .5, .65, .5, .24, .067};
 static double dx2[] = {.1, .1, .1, .1, .1, .1, .1, .1};

 dataset[4] = gtk_plot_add_function(GTK_PLOT(plot), (GtkPlotFunc)gaussian);
 gtk_widget_show(GTK_WIDGET(dataset[4]));
 gdk_color_parse("dark green", &color);
 gdk_color_alloc(gdk_colormap_get_system(), &color); 
 gtk_plot_data_set_line_attributes(dataset[4],
                                   GTK_PLOT_LINE_DASHED,
                                   0, 0, 2, &color);

 gtk_plot_data_set_legend(dataset[4], "Gaussian");


 gdk_color_parse("blue", &color);
 gdk_color_alloc(gdk_colormap_get_system(), &color); 
/*
 GTK_PLOT(plot)->xscale = GTK_PLOT_SCALE_LOG10;
*/

 dataset[2] = GTK_PLOT_DATA(gtk_plot_bar_new(GTK_ORIENTATION_VERTICAL));
 gtk_plot_add_data(GTK_PLOT(plot), dataset[2]);
 gtk_widget_show(GTK_WIDGET(dataset[2]));
 gtk_plot_data_set_points(dataset[2], px2, py2, dx2, NULL, 8);

 gtk_plot_data_set_symbol(dataset[2],
                          GTK_PLOT_SYMBOL_NONE,
			  GTK_PLOT_SYMBOL_OPAQUE,
                          10, 2, &color, &color);

 gtk_plot_data_set_line_attributes(dataset[2],
                                   GTK_PLOT_LINE_NONE,
                                   0, 0, 1, &color);
 gtk_plot_data_set_legend(dataset[2], "V Bars");

 gtk_plot_set_break(GTK_PLOT(plot), GTK_PLOT_AXIS_Y, 0.7, 0.72, .05, 4, GTK_PLOT_SCALE_LINEAR, .6);
}

int main(int argc, char *argv[]){
 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *canvas;
 GtkPlotCanvasChild *child;
 GdkColor color;
 gint page_width, page_height;
 gfloat scale = 1.;
 gchar *custom_labels[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
 GtkPlotArray *array;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlot Demo");
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

 canvas = gtk_plot_canvas_new(page_width, page_height, 1.0);
 GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1), canvas);

/*
 gdk_color_parse("light blue", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas), &color);
*/

 gtk_widget_show(canvas);

 active_plot = new_layer(canvas);
/*
 gtk_plot_clip_data(GTK_PLOT(active_plot), TRUE);
*/
 gtk_plot_set_range(GTK_PLOT(active_plot), -1., 1., -1., 1.4);
 gtk_plot_legends_move(GTK_PLOT(active_plot), .500, .05);
 gtk_plot_set_legends_border(GTK_PLOT(active_plot), 0, 0);
 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP));
 gtk_plot_axis_show_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_BOTTOM), 15, 3);
 gtk_plot_set_ticks(GTK_PLOT(active_plot), GTK_PLOT_AXIS_X, 1., 1);
 gtk_plot_set_ticks(GTK_PLOT(active_plot), GTK_PLOT_AXIS_Y, 1., 1);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP), TRUE);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
 gtk_plot_x0_set_visible(GTK_PLOT(active_plot), TRUE);
 gtk_plot_y0_set_visible(GTK_PLOT(active_plot), TRUE);
 gtk_plot_axis_set_labels_suffix(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_LEFT), "%");

 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .06, .65, .31);
 gtk_widget_show(active_plot);
 GTK_PLOT_CANVAS_PLOT(child)->flags |= GTK_PLOT_CANVAS_PLOT_SELECT_POINT;
 GTK_PLOT_CANVAS_PLOT(child)->flags |= GTK_PLOT_CANVAS_PLOT_DND_POINT;

 build_example1(active_plot);

 active_plot = new_layer(canvas);
 gdk_color_parse("light yellow", &color);
 gdk_color_alloc(gtk_widget_get_colormap(active_plot), &color);
 gtk_plot_set_background(GTK_PLOT(active_plot), &color);

 gdk_color_parse("light blue", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_legends_set_attributes(GTK_PLOT(active_plot),
                                 NULL, 0,
				 NULL,
                                 &color);
 gtk_plot_set_range(GTK_PLOT(active_plot), 0. , 1., 0., .85);
/*
 gtk_plot_set_range(GTK_PLOT(active_plot), 0.1 , 100., 0., .85);
 gtk_plot_set_xscale(GTK_PLOT(active_plot), GTK_PLOT_SCALE_LOG10);
*/
 gtk_plot_axis_set_labels_numbers(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_LEFT), GTK_PLOT_LABEL_FLOAT, 2);
 gtk_plot_axis_set_labels_numbers(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), GTK_PLOT_LABEL_FLOAT, 2);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP), TRUE);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
 gtk_plot_grids_set_visible(GTK_PLOT(active_plot), TRUE, TRUE, TRUE, TRUE);
 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP));
 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT));
 gtk_plot_set_legends_border(GTK_PLOT(active_plot), 2, 3);
 gtk_plot_legends_move(GTK_PLOT(active_plot), .58, .05);
 gtk_widget_show(active_plot);

 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .4, .65, .65);
 gtk_widget_show(active_plot);

 build_example2(active_plot);

 g_signal_connect(GTK_OBJECT(canvas), "select_item",
                    (void *) select_item, NULL);

/*
 g_signal_connect(GTK_OBJECT(canvas), "move_item",
                    (void ) move_item, NULL);
*/

 child = gtk_plot_canvas_text_new("Times-BoldItalic", 16, 0, NULL, NULL, TRUE,
                          GTK_JUSTIFY_CENTER,
                          "DnD titles, legends and plots");
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .40, .020, .0, .0);
 child = gtk_plot_canvas_text_new("Times-Roman", 16, 0, NULL, NULL, TRUE,
                          GTK_JUSTIFY_CENTER,
                          "You can use \\ssubscripts\\b\\b\\b\\b\\b\\b\\b\\b\\b\\b\\N\\Ssuperscripts");
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .40, .720, .0, .0);

 child = gtk_plot_canvas_text_new("Times-Roman", 12, 0, NULL, NULL, TRUE,
                          GTK_JUSTIFY_CENTER, 
                          "Format text mixing \\Bbold \\N\\i, italics, \\ggreek \\4\\N and \\+different fonts");
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .40, .765, .0, .0); 

 gtk_plot_text_set_border(&GTK_PLOT_CANVAS_TEXT(child)->text, 
                          GTK_PLOT_BORDER_SHADOW, 2, 0, 2);

 array = GTK_PLOT_ARRAY(gtk_plot_array_new(NULL, custom_labels, 12, G_TYPE_STRING, FALSE)); 
 gtk_plot_axis_set_tick_labels(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_BOTTOM), array); 
 gtk_plot_axis_use_custom_tick_labels(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_BOTTOM), TRUE);

 put_child(GTK_PLOT_CANVAS(canvas), .5, .5);

 gtk_widget_show_all(window1);

 //test_print(GTK_PLOT_CANVAS(canvas),window1);
/*
 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "demoplot.ps", GTK_PLOT_PORTRAIT, FALSE, GTK_PLOT_LETTER);
*/
/* 
 gtk_plot_canvas_export_ps_with_size(GTK_PLOT_CANVAS(canvas), "demoplot.ps", GTK_PLOT_PORTRAIT, TRUE, GTK_PLOT_PSPOINTS, 300, 400);
*/

 gtk_main();

 return(0);
}

static void
put_child(GtkPlotCanvas *canvas, gdouble x, gdouble y)
{
  GdkColormap *colormap;
  GtkPlotCanvasChild *child;
  GdkBitmap *mask;

  colormap = gdk_colormap_get_system();

  pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask, NULL,
                                                 plot_icon2);
  child = gtk_plot_canvas_pixmap_new(pixmap, mask);

  gtk_plot_canvas_put_child(canvas, child, x, y, x+.05, y+.05); 

  gdk_pixmap_unref(pixmap);
  gdk_bitmap_unref(mask);
}
