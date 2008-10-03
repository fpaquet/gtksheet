#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplotdata.h"
#include "gtkplotsegment.h"
#include "gtkplotcanvas.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotps.h"
#include "gtkplotprint.h"
#include "gtkplotdt.h"

GdkPixmap *pixmap=NULL;
GtkWidget **plots;
GtkWidget **buttons;
GtkPlotData *dataset[5];
gint nlayers = 0;
GtkWidget *active_plot;


void
quit ()
{
  gtk_main_quit();
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
      gtk_signal_handler_block_by_func(GTK_OBJECT(buttons[n]), GTK_SIGNAL_FUNC(activate_plot), data);
      if(widget_list[n] == active_widget){
            active_plot = plots[n];
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

 plots[nlayers-1] = gtk_plot_new_with_size(NULL, .5, .5);
 gtk_widget_show(plots[nlayers-1]);

 activate_plot(buttons[nlayers-1],canvas);

 return plots[nlayers-1];
}


void
build_example1(GtkWidget *active_plot,GtkPlotDT *data, gint num)
{
 GdkColor color;
 gdouble *px1= (gdouble *)g_malloc(sizeof(gdouble)*num*3);
 gdouble *py1= (gdouble *)g_malloc(sizeof(gdouble)*num*3);
 gdouble *px2= (gdouble *)g_malloc(sizeof(gdouble)*num*3);
 gdouble *py2= (gdouble *)g_malloc(sizeof(gdouble)*num*3);
 gdouble ax,ay,bx,by,cx,cy;
 GtkPlotDTtriangle *t= NULL;
 GList *list = NULL;
 gint i = 0;

 list = data->triangles;
 while(list){
   t = (GtkPlotDTtriangle *)list->data;
   ax= gtk_plot_dt_get_node(data,t->a)->x;
   ay= gtk_plot_dt_get_node(data,t->a)->y;
   bx= gtk_plot_dt_get_node(data,t->b)->x;
   by= gtk_plot_dt_get_node(data,t->b)->y;
   cx= gtk_plot_dt_get_node(data,t->c)->x;
   cy= gtk_plot_dt_get_node(data,t->c)->y;
   px1[i*3+0]= ax;    py1[i*3+0]= ay;
   px2[i*3+0]= bx; py2[i*3+0]= by;
   px1[i*3+1]= bx;    py1[i*3+1]= by;
   px2[i*3+1]= cx; py2[i*3+1]= cy;
   px1[i*3+2]= cx;    py1[i*3+2]= cy;
   px2[i*3+2]= ax; py2[i*3+2]= ay;

   i++;
   list = list->next;
 }
 fprintf(stderr,"%d triangles, %d points\n",num,num*3);

 dataset[0] = GTK_PLOT_DATA(gtk_plot_segment_new());
 gtk_plot_segment_set_relative(GTK_PLOT_SEGMENT(dataset[0]), FALSE);
 gtk_plot_add_data(GTK_PLOT(active_plot), dataset[0]);
 gtk_widget_show(GTK_WIDGET(dataset[0]));
 gtk_plot_data_set_points(dataset[0], px1, py1, px2, py2, num*3);

 gtk_plot_segment_center(GTK_PLOT_SEGMENT(dataset[0]), FALSE);


 gdk_color_parse("red", &color);
 gdk_color_alloc(gdk_colormap_get_system(), &color); 
 gtk_plot_segment_set_arrow(GTK_PLOT_SEGMENT(dataset[0]), 0, 0, 0);
 gtk_plot_data_set_symbol(dataset[0],
                          GTK_PLOT_SYMBOL_CIRCLE,
			  GTK_PLOT_SYMBOL_OPAQUE,
                          0,1, &color, &color);
 gtk_plot_data_set_line_attributes(dataset[0],
                                   GTK_PLOT_LINE_NONE,
                                   0, 0, 0, &color);

 gtk_plot_data_set_legend(dataset[0], "Delaunay triangulization");

}

static int
simpleprogressbar(double p)
{
  static int lastprog= -1;
  int prog,i;
  if (p<0) p=0; if (p>1) p=1;
  prog= 70*p;
  if (prog>lastprog) {
    fprintf(stderr,"\r %3d%% ",(int)(p*100));
    for (i=0; i<prog; i++) fprintf(stderr,"*");
    lastprog= prog;
  }
  return 0;
}

/*
static int
gtkprogressbar(double p)
{
  static int lastprog= -1;
  int prog,i;
  if (p<0) p=0; if (p>1) p=1;
  prog= 70*p;
  if (prog>lastprog) {
    fprintf(stderr,"\r %3d%% ",(int)(p*100));
    for (i=0; i<prog; i++) fprintf(stderr,"*");
    lastprog= prog;
  }
  return 0;
}
*/

int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *canvas;
 GtkPlotCanvasChild *child;
 gint page_width, page_height;
 gfloat scale = 1.;
 GtkPlotDTnode p;
 GtkPlotDT *dtdata;
 char buffer[1000];
 FILE *f;
 gdouble xmin=1e99;
 gdouble xmax=-1e99;
 gdouble ymin=1e99;
 gdouble ymax=-1e99;
 gdouble dx,dy;
 gint num_triangles = 0;
 
 page_width = GTK_PLOT_LETTER_W * scale;
 page_height = GTK_PLOT_LETTER_H * scale;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkPlotDT Demo");
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

 if (argc!=2) {
   fprintf(stderr,"\nUsage:\n\ttestgtkplotdt X-Y-FILE\n");
   exit(-1);
 }
 if (!(f=fopen(argv[1],"r"))) {
   fprintf(stderr,"\ncould not open file '%s' for reading\n",argv[1]);
   exit(-2);
 }
 
 /* init with nodelist size 0 */
 dtdata= GTK_PLOT_DT(gtk_plot_dt_new(0));
 if (!dtdata) exit(-1);
 /* register the progressmeter */
 dtdata->pbar= simpleprogressbar;
 
 /* read X/Y pairs from f: */
 while (fgets(buffer,1000,f)) {
   if (sscanf(buffer,"%lf %lf", &p.x, &p.y)==2) {
     /* add this node */
     gtk_plot_dt_add_node(dtdata,p);
     if (xmin>p.x) xmin= p.x;
     if (xmax<p.x) xmax= p.x;
     if (ymin>p.y) ymin= p.y;
     if (ymax<p.y) ymax= p.y;
   }
 }
 dx= (xmax-xmin)*.02;
 dy= (ymax-ymin)*.02;
 fclose(f);
 /* start the triangulation */
 fprintf(stderr,"data ranges from (%g,%g) to (%g,%g)\n",
	 xmin,ymin,xmax,ymax);
 num_triangles= gtk_plot_dt_triangulate(dtdata);

 active_plot = new_layer(canvas);
 gtk_plot_set_range(GTK_PLOT(active_plot), xmin-dx, xmax+dx, ymin-dy, ymax+dy);
 gtk_plot_legends_move(GTK_PLOT(active_plot), .500, .05);
 gtk_plot_set_legends_border(GTK_PLOT(active_plot), 0, 0);
 gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP));
 gtk_plot_axis_show_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), 3), 15, 3);
 gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), 0), (xmax-xmin)/10., 1);
 gtk_plot_axis_set_ticks(gtk_plot_get_axis(GTK_PLOT(active_plot), 1), (ymax-ymin)/10., 1);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_TOP), TRUE);
 gtk_plot_axis_set_visible(gtk_plot_get_axis(GTK_PLOT(active_plot), GTK_PLOT_AXIS_RIGHT), TRUE);
 gtk_plot_x0_set_visible(GTK_PLOT(active_plot), TRUE);
 gtk_plot_y0_set_visible(GTK_PLOT(active_plot), TRUE);


 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15, .06, .65, .65);
 gtk_widget_show(active_plot);

 build_example1(active_plot,dtdata, num_triangles);

 gtk_widget_show(window1);

 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "demodtsegment.ps", 0, 0, 
                           GTK_PLOT_LETTER);
 
 gtk_main();

 return(0);
}

