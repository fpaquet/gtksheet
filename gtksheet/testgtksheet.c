#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>
#include <glib.h>
#include <string.h>
#include "gtksheet.h"

GtkWidget *window;
GtkWidget *colorbutton;
GtkWidget *fontbutton;

void
quit ()
{
  gtk_main_quit();
}

gint
activate_sheet_cell_cb(GtkWidget *widget, gint row, gint column, gpointer data) 
{
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);

    GtkSheet *sheet;
    const gchar *text;
    GtkSheetCellAttr attrs;
    PangoFontDescription *font_desc;
    char *font_name;

    sheet=GTK_SHEET(widget);

    text = gtk_sheet_cell_get_text(sheet, row, column);
    if (text)
    {
        g_message("Row: %d Column: %d Text: %s", row, column, text);
    }

    gtk_sheet_get_attributes(sheet, row, column, &attrs);

    font_desc = (attrs.font_desc == NULL)? widget->style->font_desc : attrs.font_desc;
    font_name = pango_font_description_to_string(font_desc);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(fontbutton), font_name);
    g_free(font_name);

    gtk_color_button_set_color(GTK_COLOR_BUTTON(colorbutton), &(attrs.background));

    
   
}

void
bg_color_changed_cb(GtkWidget *widget, gpointer data) 
{
    g_return_if_fail(GTK_IS_SHEET(data));
    g_return_if_fail(GTK_IS_COLOR_BUTTON(widget));

    GtkColorButton *colorbtn = GTK_COLOR_BUTTON(widget);
    GtkSheet *sheet = GTK_SHEET(data);
    GdkColor color;

    /*
    Bug in GtkSheet: the background color of a cell is not respected!!!
    */
    gtk_color_button_get_color(colorbtn, &color);
    gtk_sheet_range_set_background(sheet, &(sheet->range), &color);
    
}

void
font_changed_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(GTK_IS_SHEET(data));
    g_return_if_fail(GTK_IS_FONT_BUTTON(widget));

    GtkSheet *sheet = GTK_SHEET(data);
    GtkFontButton *fontbutton = GTK_FONT_BUTTON(widget);

    const gchar *font_name = gtk_font_button_get_font_name(fontbutton);
    PangoFontDescription *fd = pango_font_description_from_string(font_name);
    if (fd)
    {
        gtk_sheet_range_set_font(sheet, &(sheet->range), fd);
        pango_font_description_free(fd);
    }
}

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GtkSheet Demo");
    gtk_widget_set_size_request(window, 500, 500);
    gtk_signal_connect (GTK_OBJECT (window), "destroy",
	                    GTK_SIGNAL_FUNC (quit), NULL);

    GdkColor bg, grid;
    GdkColormap *colormap = gdk_colormap_get_system();
    gdk_color_parse("light yellow", &bg);
    gdk_color_parse("light blue", &grid);
    gdk_colormap_alloc_color(colormap, &bg, TRUE, TRUE);
    gdk_colormap_alloc_color(colormap, &grid, TRUE, TRUE);

    GtkWidget *sheet = gtk_sheet_new(1000,26, "Sheet 1");
    gtk_sheet_set_background(GTK_SHEET(sheet), &bg);
    gtk_sheet_set_grid(GTK_SHEET(sheet), &grid);
    gtk_signal_connect(GTK_OBJECT(sheet), "activate",
                       (GtkSignalFunc)activate_sheet_cell_cb, NULL);
    GtkWidget *scrwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrwin), sheet);

    colorbutton = gtk_color_button_new();
    gtk_signal_connect(GTK_OBJECT(colorbutton), "color-set", 
                       GTK_SIGNAL_FUNC(bg_color_changed_cb), sheet);

    fontbutton = gtk_font_button_new();
    gtk_signal_connect(GTK_OBJECT(fontbutton), "font-set",
                       GTK_SIGNAL_FUNC(font_changed_cb), sheet);
       

    GtkWidget *hbox = gtk_hbox_new(FALSE , 5);
    gtk_box_pack_start(GTK_BOX(hbox), fontbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), colorbutton, FALSE, FALSE, 0);

    GtkWidget *main_vbox = gtk_vbox_new(FALSE,1);
    gtk_box_pack_start(GTK_BOX(main_vbox), hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrwin, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    gtk_widget_show_all( window );


    gtk_main();

}



