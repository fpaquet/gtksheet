#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkplot.h"
#include "gtkplot3d.h"
#include "gtkplotdata.h"
#include "gtkplotsurface.h"
#include "gtkplotcsurface.h"
#include "gtkplotcanvas.h"
#include "gtkplotps.h"
#include "gtkplotprint.h"
#include "gtkplotcanvastext.h"
#include "gtkplotcanvasline.h"
#include "gtkplotcanvasellipse.h"
#include "gtkplotcanvasrectangle.h"
#include "gtkplotcanvasplot.h"
#include "gtkplotcanvaspixmap.h"

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

gdouble function(GtkPlot *plot, 
                 GtkPlotData *data, 
                 gdouble x, gdouble y, gboolean *err)
{
 gdouble z;
 *err = FALSE;

 z = cos(((x-0.5)*(x-0.5) + (y-0.5)*(y-0.5))*24) / 4. + .5;

/* z = (x*x + y*y) / 9.0;
*/

 return z;
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
 gtk_fixed_put(GTK_FIXED(canvas), buttons[nlayers-1], (nlayers-1)*20, 0);
 gtk_widget_show(buttons[nlayers-1]);

 plots[nlayers-1] = gtk_plot3d_new_with_size(NULL, .70, .70);

 gtk_widget_show(plots[nlayers-1]);

 return plots[nlayers-1];
}

void
rotate_x(GtkWidget *button, gpointer data)
{
 gtk_plot3d_rotate_x(GTK_PLOT3D(active_plot), 10.);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(data));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(data));
}

void
rotate_y(GtkWidget *button, gpointer data)
{
 gtk_plot3d_rotate_y(GTK_PLOT3D(active_plot), 10.);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(data));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(data));
}

void
rotate_z(GtkWidget *button, gpointer data)
{
 gtk_plot3d_rotate_z(GTK_PLOT3D(active_plot), 10.);

 gtk_plot_canvas_paint(GTK_PLOT_CANVAS(data));
 gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(data));
}


#define MAXNODES 50000

int main(int argc, char *argv[]){

 GtkWidget *window1;
 GtkWidget *vbox1;
 GtkWidget *scrollw1;
 GtkWidget *canvas;
 GtkWidget *button;
 GtkWidget *surface;
 gdouble *x,*y,*z,*a;
 gint cnt= 0;
 gint page_width, page_height;
 gfloat scale = 1.;
 char buffer[1000];
 FILE *f;
 gdouble xmin=1e99;
 gdouble xmax=-1e99;
 gdouble ymin=1e99;
 gdouble ymax=-1e99;
 gdouble zmin=1e99;
 gdouble zmax=-1e99;
 gdouble dx,dy,dz;
 gint n, nx, ny;
 GtkPlotCanvasChild *child;
 
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

 scrollw1=gtk_scrolled_window_new(NULL, NULL);
 gtk_container_border_width(GTK_CONTAINER(scrollw1),0);
 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw1),
				GTK_POLICY_ALWAYS,GTK_POLICY_ALWAYS);
 gtk_box_pack_start(GTK_BOX(vbox1),scrollw1, TRUE, TRUE,0);

 canvas = gtk_plot_canvas_new(page_width, page_height, 1.);
 GTK_PLOT_CANVAS_SET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_DND_FLAGS);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1), canvas);

/*
 gdk_color_parse("light blue", &color);
 gdk_color_alloc(gtk_widget_get_colormap(canvas), &color);
 gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas), &color);
*/


 active_plot = new_layer(canvas);

 child = gtk_plot_canvas_plot_new(GTK_PLOT(active_plot));
 gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .10, .06, .85, .85);


 x = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 y = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 z = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 a = (gdouble *)g_malloc(1600 * sizeof(gdouble));
 n = 0;

 for(nx = 0; nx < 40; nx++)
   for(ny = 0; ny < 40; ny++)
   {
     x[n]=0.025*(gdouble)nx;
     y[n]=0.025*(gdouble)ny;
     z[n]=cos(((x[n]-0.5)*(x[n]-0.5) + (y[n]-0.5)*(y[n]-0.5))*24) / 4. + .5;
     a[n]=((x[n]-0.5)*(x[n]-0.5) + (y[n]-0.5)*(y[n]-0.5));
     n++;
   }

 surface = gtk_plot_surface_new();

 gtk_plot_surface_set_points(GTK_PLOT_SURFACE(surface), 
                             x, y, z, NULL, NULL, NULL, 40, 40); 
 gtk_plot_data_set_legend(GTK_PLOT_DATA(surface), "cos ((r-r\\s0\\N)\\S2\\N)");

 gtk_plot_surface_use_amplitud(GTK_PLOT_SURFACE(surface), TRUE);
 gtk_plot_data_set_a(GTK_PLOT_DATA(surface), a);
 gtk_plot_data_gradient_set_visible(GTK_PLOT_DATA(surface), TRUE);

/*
 for(nx = 0; nx < 10; nx++)
   for(ny = 0; ny < 10; ny++)
   {
     x[n]=(gdouble)0.1*(gdouble)nx;
     y[n]=(gdouble)0.1*(gdouble)ny;
     z[n]=(x[n]-0.5)*(x[n]-0.5) + (y[n]-0.5)*(y[n]-0.5);
     n++;
   }

 surface = gtk_plot_surface_new();

 gtk_plot_surface_set_points(GTK_PLOT_SURFACE(surface), 
                             x, y, z, NULL, NULL, NULL, 10, 10); 
*/

 gtk_plot3d_set_minor_ticks(GTK_PLOT3D(active_plot), GTK_PLOT_AXIS_X, 1);
 gtk_plot3d_set_minor_ticks(GTK_PLOT3D(active_plot), GTK_PLOT_AXIS_Y, 1);
 gtk_plot3d_show_ticks(GTK_PLOT3D(active_plot), 
                            GTK_PLOT_SIDE_XY, 
                            GTK_PLOT_TICKS_OUT, GTK_PLOT_TICKS_OUT);
 gtk_plot3d_show_ticks(GTK_PLOT3D(active_plot), 
                            GTK_PLOT_SIDE_XZ, 
                            GTK_PLOT_TICKS_OUT, GTK_PLOT_TICKS_OUT);
 gtk_plot3d_show_ticks(GTK_PLOT3D(active_plot), 
                            GTK_PLOT_SIDE_YX, 
                            GTK_PLOT_TICKS_OUT, GTK_PLOT_TICKS_OUT);
 gtk_plot3d_show_ticks(GTK_PLOT3D(active_plot), 
                            GTK_PLOT_SIDE_YZ, 
                            GTK_PLOT_TICKS_OUT, GTK_PLOT_TICKS_OUT);
 gtk_plot3d_show_ticks(GTK_PLOT3D(active_plot), 
                            GTK_PLOT_SIDE_ZX, 
                            GTK_PLOT_TICKS_OUT, GTK_PLOT_TICKS_OUT);
 gtk_plot3d_show_ticks(GTK_PLOT3D(active_plot), 
                            GTK_PLOT_SIDE_ZY, 
                            GTK_PLOT_TICKS_OUT, GTK_PLOT_TICKS_OUT);

 /* if a file was specified on the commandline, try to open and display it: */

 if (argc!=2 || !(f=fopen(argv[1],"r"))) {
/*
   x= y= z= NULL;
   surface = gtk_plot_surface_new_function((GtkPlotFunc3D) function);
   gtk_plot_surface_set_xstep(GTK_PLOT_SURFACE(surface), .025);
   gtk_plot_surface_set_ystep(GTK_PLOT_SURFACE(surface), .025);
   gtk_plot_data_set_legend(GTK_PLOT_DATA(surface), "cos ((r-r\\s0\\N)\\S2\\N)");
*/
   gtk_plot_add_data(GTK_PLOT(active_plot), GTK_PLOT_DATA(surface));
 } else {
   x= g_new(gdouble,MAXNODES);
   y= g_new(gdouble,MAXNODES);
   z= g_new(gdouble,MAXNODES);
   while (fgets(buffer,1000,f) && cnt<MAXNODES) 
     if (sscanf(buffer,"%lf %lf %lf", &x[cnt], &y[cnt], &z[cnt])==3) {
       z[cnt]*=10; /* REMOVE THIS BEFORE FLIGHT */
       if (xmin>x[cnt]) xmin= x[cnt];
       if (xmax<x[cnt]) xmax= x[cnt];
       if (ymin>y[cnt]) ymin= y[cnt];
       if (ymax<y[cnt]) ymax= y[cnt];
       if (zmin>z[cnt]) zmin= z[cnt];
       if (zmax<z[cnt]) zmax= z[cnt];
       cnt++;   
     }
   fclose(f);
   dx= (xmax-xmin)*.02;
   dy= (ymax-ymin)*.02;
   dz= (zmax-zmin)*.02;
   /* start the triangulation */
   fprintf(stderr,"data ranges from (%g,%g) to (%g,%g)\n",
	   xmin,ymin,xmax,ymax);

   /*
   gtk_plot3d_set_xrange(GTK_PLOT3D(active_plot), xmin-dx, xmax+dx);
   gtk_plot3d_set_yrange(GTK_PLOT3D(active_plot), ymin-dy, ymax+dy);
   gtk_plot3d_set_zrange(GTK_PLOT3D(active_plot), zmin-dz, zmax+dz);
   gtk_plot3d_set_ticks(GTK_PLOT3D(active_plot), GTK_PLOT_AXIS_X, (xmax-xmin)/10., 1);
   gtk_plot3d_set_ticks(GTK_PLOT3D(active_plot), GTK_PLOT_AXIS_Y, (ymax-ymin)/10., 1);
   gtk_plot3d_set_ticks(GTK_PLOT3D(active_plot), GTK_PLOT_AXIS_Z, (zmax-zmin)/10., 1);
   */

   surface = gtk_plot_csurface_new();
   gtk_plot_add_data(GTK_PLOT(active_plot), GTK_PLOT_DATA(surface));
   gtk_plot_surface_set_points(GTK_PLOT_SURFACE(surface), 
			       x, y, z, NULL, NULL, NULL, cnt, 1); 
   gtk_plot_data_set_legend(GTK_PLOT_DATA(surface), argv[1]);
   gtk_plot_data_gradient_autoscale_z(GTK_PLOT_DATA(surface));
 }
/*
 gtk_plot_surface_use_height_gradient(GTK_PLOT_SURFACE(surface), TRUE);
 gtk_plot_data_set_gradient(GTK_PLOT_DATA(surface), .2, .8, 6, 0);
*/

/*
 gtk_plot_data_gradient_set_scale(GTK_PLOT_DATA(surface),GTK_PLOT_SCALE_LOG10);
*/

 gtk_plot3d_autoscale(GTK_PLOT3D(active_plot));
/*
 gtk_plot3d_set_xrange(GTK_PLOT3D(active_plot), -0.5,1);
 gtk_plot3d_set_yrange(GTK_PLOT3D(active_plot), -0.2,1);
 gtk_plot3d_set_zrange(GTK_PLOT3D(active_plot), 0.,1);
*/
/*
 gtk_plot3d_rotate_x(GTK_PLOT3D(active_plot), GTK_PLOT3D(active_plot)->a1+360-300);
 gtk_plot3d_rotate_y(GTK_PLOT3D(active_plot), GTK_PLOT3D(active_plot)->a2+360-360);
 gtk_plot3d_rotate_z(GTK_PLOT3D(active_plot), GTK_PLOT3D(active_plot)->a3+360-330);
*/
 gtk_widget_show(surface);


 button = gtk_button_new_with_label("Rotate X");
 gtk_fixed_put(GTK_FIXED(canvas), button, 80, 0);

 g_signal_connect(GTK_OBJECT(button), "clicked",
                    G_CALLBACK(rotate_x), canvas);

 button = gtk_button_new_with_label("Rotate Y");
 gtk_fixed_put(GTK_FIXED(canvas), button, 160, 0);

 g_signal_connect(GTK_OBJECT(button), "clicked",
                    G_CALLBACK(rotate_y), canvas);

 button = gtk_button_new_with_label("Rotate Z");
 gtk_fixed_put(GTK_FIXED(canvas), button, 240, 0);

/*
 gtk_plot3d_set_xfactor(GTK_PLOT3D(active_plot), 0.5);
*/
/*
 gtk_plot3d_set_xrange(GTK_PLOT3D(active_plot), 0., 2.0);
*/

 g_signal_connect(GTK_OBJECT(button), "clicked",
                    G_CALLBACK(rotate_z), canvas);

 gtk_widget_show_all(window1);

/*
 gtk_plot_canvas_export_ps(GTK_PLOT_CANVAS(canvas), "demo3d.ps", 0, 0,
                           GTK_PLOT_LETTER);
*/

 gtk_plot_canvas_export_ps_with_size(GTK_PLOT_CANVAS(canvas), "demo3d.ps", 
				     0, 0, GTK_PLOT_PSPOINTS,
                            	     GTK_PLOT_LETTER_W, GTK_PLOT_LETTER_H);


 gtk_main();

 return(0);
}

