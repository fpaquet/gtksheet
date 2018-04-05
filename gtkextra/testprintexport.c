#include <gtkextra/gtkextra.h>
#include <cairo-ps.h>
#include <cairo-pdf.h>
#include <math.h>
#include <string.h>



struct dialogData {
	GtkWidget *window;
	GtkWidget *canvas;
};


void destroy_window_cb(GtkWidget *widget, gpointer data) {
	gtk_main_quit();
}

gboolean delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
	return FALSE;
}

void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, struct dialogData *dd) {
        cairo_t *cr;

	//the following two lines do all the magic
        cr = gtk_print_context_get_cairo_context(context);
        gtk_plot_canvas_export_cairo(GTK_PLOT_CANVAS(dd->canvas),cr);

        return;
}

void print_button_clicked_cb(GtkWidget *button, struct dialogData *dd) {
	GtkPrintOperation *operation;
        GError *error = NULL;
        GtkPrintOperationResult res;
	GtkPrintSettings *print_settings;
	GtkPageSetup *page_setup;

	//print settings
        print_settings = gtk_print_settings_new();
        gtk_print_settings_set_orientation(print_settings,GTK_PAGE_ORIENTATION_LANDSCAPE);
        gtk_print_settings_set_paper_size(print_settings,gtk_paper_size_new(GTK_PAPER_NAME_A4));
        page_setup = gtk_page_setup_new();
        gtk_page_setup_set_orientation(page_setup,GTK_PAGE_ORIENTATION_LANDSCAPE);
        gtk_page_setup_set_paper_size_and_default_margins(page_setup,gtk_paper_size_new(GTK_PAPER_NAME_A4));

        operation = gtk_print_operation_new();
        gtk_print_operation_set_print_settings(operation,print_settings);
        gtk_print_operation_set_default_page_setup(operation,page_setup);
        gtk_print_operation_set_show_progress(operation,TRUE);
        gtk_print_operation_set_track_print_status(operation, TRUE);
        g_signal_connect(G_OBJECT(operation), "draw-page", G_CALLBACK(draw_page), dd);
        gtk_print_operation_set_n_pages(operation, 1);
        
        res = gtk_print_operation_run(operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(dd->window),&error);

        if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
                g_object_unref(print_settings);
                print_settings = g_object_ref(gtk_print_operation_get_print_settings(operation));
        }


        g_object_unref(operation);



        return;

}

void export_button_clicked_cb(GtkWidget *button, struct dialogData *dd) {
	GtkWidget *dialog;
        GtkFileFilter *filter;
        gchar *filename;
        cairo_t *cr;
        cairo_surface_t *surface;

	dialog = gtk_file_chooser_dialog_new("Export spectra", 
                GTK_WINDOW(dd->window), GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
        filter = gtk_file_filter_new();
        gtk_file_filter_add_pattern(filter,"*.eps");
        gtk_file_filter_set_name(filter,"EPS (Encapsulated PostScript)");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        filter = gtk_file_filter_new();
        gtk_file_filter_add_pattern(filter,"*.pdf");
        gtk_file_filter_set_name(filter,"PDF (Adobe Portable Document Format)");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        filter = gtk_file_filter_new();
        gtk_file_filter_add_pattern(filter,"*.png");
        gtk_file_filter_set_name(filter,"PNG (Portable Network Graphics)");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
                filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                //get selected filter
                filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
                if (strncmp(gtk_file_filter_get_name(filter),"EPS", 3) == 0) {
                        if (strcmp(filename+strlen(filename)-4, ".eps") != 0) {
                                filename = (gchar *) g_realloc(filename,sizeof(gchar)*(strlen(filename)+4));
                                strcat(filename,".eps");
                        }
                        surface = cairo_ps_surface_create(filename,842,595);
                        cairo_ps_surface_set_eps(surface,1);
                        cr = cairo_create(surface);

                        gtk_plot_canvas_export_cairo(GTK_PLOT_CANVAS(dd->canvas),cr);
                        //gtk_plot_canvas_paint(GTK_PLOT_CANVAS(dd->canvas));
                        cairo_show_page(cr);
                        cairo_surface_destroy(surface);
                        cairo_destroy(cr);

                }
		else if (strncmp(gtk_file_filter_get_name(filter),"PDF", 3) == 0) {
                        if (strcmp(filename+strlen(filename)-4, ".pdf") != 0) {
                                filename = (gchar *) g_realloc(filename,sizeof(gchar)*(strlen(filename)+4));
                                strcat(filename,".pdf");
                        }
                        surface = cairo_pdf_surface_create(filename,842.0,595.0);
                        cr = cairo_create(surface);
                        gtk_plot_canvas_export_cairo(GTK_PLOT_CANVAS(dd->canvas),cr);
                        //gtk_plot_canvas_paint(GTK_PLOT_CANVAS(dd->canvas));
                        cairo_show_page(cr);
                        cairo_surface_destroy(surface);
                        cairo_destroy(cr);
                }
                else if (strncmp(gtk_file_filter_get_name(filter),"PNG", 3) == 0) {
                        if (strcmp(filename+strlen(filename)-4, ".png") != 0) {
                                filename = (gchar *) g_realloc(filename,sizeof(gchar)*(strlen(filename)+4));
                                strcat(filename,".png");
                        }
                        surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 842, 595);
                        cr = cairo_create(surface);
                        gtk_plot_canvas_export_cairo(GTK_PLOT_CANVAS(dd->canvas),cr);
                        //gtk_plot_canvas_paint(GTK_PLOT_CANVAS(dd->canvas));
                        cairo_surface_write_to_png(surface,filename);
                        cairo_surface_destroy(surface);
                        cairo_destroy(cr);
                }
                g_free(filename);
                gtk_widget_destroy(dialog);
	}
	else
                gtk_widget_destroy(dialog);


        return;

}

int main(int argc, char *argv[]) {

	GdkColor white, red;

	gtk_init(&argc, &argv);

	//create window
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy_window_cb), NULL);
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event_cb), NULL);
        gtk_window_set_title(GTK_WINDOW(window), "GtkExtra example");
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	//create box that will contain the buttons and the canvas
	GtkWidget *mainbox = gtk_vbox_new(FALSE, 2);
	gtk_container_set_border_width(GTK_CONTAINER(mainbox),5);
	gtk_container_add(GTK_CONTAINER(window), mainbox);

	GtkWidget *buttonbox = gtk_hbox_new(TRUE, 2);
	GtkWidget *printButton, *exportButton, *quitButton;
	
	printButton = gtk_button_new_from_stock(GTK_STOCK_PRINT);
	exportButton = gtk_button_new_from_stock(GTK_STOCK_SAVE_AS);
	quitButton = gtk_button_new_from_stock(GTK_STOCK_QUIT);

	gtk_box_pack_start(GTK_BOX(buttonbox), printButton, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(buttonbox), exportButton, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(buttonbox), quitButton, FALSE, FALSE, 1);

	struct dialogData *dd = g_malloc(sizeof(struct dialogData));
	dd->window = window;

	g_signal_connect(G_OBJECT(exportButton),"clicked",G_CALLBACK(export_button_clicked_cb), dd);
	g_signal_connect(G_OBJECT(printButton),"clicked",G_CALLBACK(print_button_clicked_cb), dd);
	g_signal_connect_swapped(G_OBJECT(quitButton), "clicked", G_CALLBACK(gtk_widget_destroy), window);

	gtk_box_pack_start(GTK_BOX(mainbox), buttonbox, FALSE, FALSE, 1);

	GtkWidget *canvas = gtk_plot_canvas_new(GTK_PLOT_A4_H, GTK_PLOT_A4_W, 0.8);
	gtk_box_pack_start(GTK_BOX(mainbox), canvas, FALSE, FALSE, 2);
	GTK_PLOT_CANVAS_UNSET_FLAGS(GTK_PLOT_CANVAS(canvas), GTK_PLOT_CANVAS_CAN_SELECT | GTK_PLOT_CANVAS_CAN_SELECT_ITEM);
	gdk_color_parse("white", &white);
	gdk_colormap_alloc_color(gdk_colormap_get_system(), &white, FALSE, TRUE);
	gdk_color_parse("red", &red);
	gdk_colormap_alloc_color(gdk_colormap_get_system(), &red, FALSE, TRUE);
	gtk_plot_canvas_set_background(GTK_PLOT_CANVAS(canvas),&white);
	dd->canvas = canvas;

	//lets add a graph to canvas
	//calculate a sine function
	double *xvals = g_malloc(sizeof(double)*1000);
	double *yvals = g_malloc(sizeof(double)*1000);

	int i;

	for (i = 0 ; i < 1000 ; i++) {
		xvals[i] = 4*M_PI*i/999;
		yvals[i] = sin(xvals[i]);
	}
	GtkWidget *plot_window;
        plot_window = gtk_plot_new_with_size(NULL,10.0, 0.1);
        gtk_plot_set_background(GTK_PLOT(plot_window),&white);
	gtk_plot_hide_legends(GTK_PLOT(plot_window));
	
	gtk_plot_set_ticks(GTK_PLOT(plot_window), GTK_PLOT_AXIS_X, M_PI_2, 5);
	gtk_plot_set_ticks(GTK_PLOT(plot_window), GTK_PLOT_AXIS_Y, 0.2, 5);
	gtk_plot_grids_set_visible(GTK_PLOT(plot_window),TRUE ,FALSE, TRUE,FALSE);
	gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_TOP));
	gtk_plot_axis_hide_title(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_RIGHT));
	gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_BOTTOM),"x");
	gtk_plot_axis_set_title(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_LEFT),"y=sin(x)");
	gtk_plot_axis_title_set_attributes(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_LEFT),"Helvetica",40,90,NULL,NULL,TRUE,GTK_JUSTIFY_CENTER);
	gtk_plot_axis_title_set_attributes(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_BOTTOM),"Helvetica",40,0,NULL,NULL,TRUE,GTK_JUSTIFY_CENTER);
	gtk_plot_axis_set_labels_attributes(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_LEFT),"Helvetica",20,0,NULL,NULL,TRUE,GTK_JUSTIFY_RIGHT);
	gtk_plot_axis_set_labels_attributes(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_RIGHT),"Helvetica",20,0,NULL,NULL,TRUE,GTK_JUSTIFY_LEFT);
	gtk_plot_axis_set_labels_attributes(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_BOTTOM),"Helvetica",20,0,NULL,NULL,TRUE,GTK_JUSTIFY_CENTER);
	gtk_plot_axis_show_labels(gtk_plot_get_axis(GTK_PLOT(plot_window), GTK_PLOT_AXIS_TOP),0);

	GtkPlotCanvasChild *child = gtk_plot_canvas_plot_new(GTK_PLOT(plot_window));
        gtk_plot_canvas_put_child(GTK_PLOT_CANVAS(canvas), child, .15,.05,.90,.85);
	GtkPlotData *dataset;
        dataset = GTK_PLOT_DATA(gtk_plot_data_new());
        gtk_plot_add_data(GTK_PLOT(plot_window),dataset);
        gtk_plot_data_set_numpoints(dataset, 1000);
        gtk_plot_data_set_x(dataset, xvals);
        gtk_plot_data_set_y(dataset, yvals);
	gtk_plot_set_range(GTK_PLOT(plot_window),0.0, 4*M_PI, -1.0, 1.0);
        gtk_plot_clip_data(GTK_PLOT(plot_window), TRUE);
        gtk_widget_show(GTK_WIDGET(dataset));
        gtk_plot_data_set_line_attributes(dataset,GTK_PLOT_LINE_SOLID,0,0,1,&red);
        gtk_plot_canvas_paint(GTK_PLOT_CANVAS(canvas));
        gtk_widget_queue_draw(GTK_WIDGET(canvas));
        gtk_plot_canvas_refresh(GTK_PLOT_CANVAS(canvas));
        gtk_plot_paint(GTK_PLOT(plot_window));
        gtk_plot_refresh(GTK_PLOT(plot_window),NULL);

	gtk_widget_show_all(window);

	gtk_main();	
}
