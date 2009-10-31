#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pango.h>
#include <glib.h>
#include <string.h>
#include "gtksheet.h"
#include "testgtksheet.h"

#define DEFAULT_PRECISION 3
#define DEFAULT_SPACE 8

/* internal functions*/
GtkSheet *get_current_sheet(TestMainWindow *self);
void _create_border_combo(TestMainWindow *self);
void _create_toolbar(TestMainWindow *self);
void _create_sheet1(TestMainWindow *self);
void _create_sheet2(TestMainWindow *self);
void _create_sheet3(TestMainWindow *self);
void _add_widget_to_toolbar(TestMainWindow *self, 
                            GtkWidget *widget, 
                            gboolean separator, 
                            const gchar *tooltip);
/*callbacks*/
void destroy_cb (GtkWidget *widget, gpointer data);
gint activate_sheet_cell_cb(GtkWidget *widget, gint row, gint column, gpointer data);
void color_changed_cb(GtkWidget *widget, gpointer data);
void font_changed_cb(GtkWidget *widget, gpointer data);
static void _selected_range_cb(GtkWidget *widget, gpointer data);
void justify_center_cb(GtkWidget *widget, gpointer data);
void justify_left_cb(GtkWidget *widget, gpointer data);
void justify_right_cb(GtkWidget *widget, gpointer data);
void do_hide_row_titles(GtkWidget *widget, gpointer data);
void do_hide_column_titles(GtkWidget *widget, gpointer data);
void do_show_row_titles(GtkWidget *widget, gpointer data);
void do_show_column_titles(GtkWidget *widget, gpointer data);
void show_entry_cb(GtkWidget *widget, gpointer data);
void show_sheet_entry_cb(GtkWidget *widget, gpointer data);
void activate_entry_cb(GtkWidget *widget, gpointer data);
void show_child_cb(GtkWidget *widget, gpointer data);
static void border_changed_cb(GtkWidget *widget, gpointer data);
gboolean change_entry(GtkWidget *widget, 
                      gint row, gint col, gint *new_row, gint *new_col,
                      gpointer data);
void parse_numbers(GtkWidget *widget, gpointer data);
gint do_popup(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean example1_key_press_cb(GtkWidget *widget, GdkEventKey *key);

/* Border combo pixmaps */
static char *xpm_border[]={
"15 15 2 1",
"      c None",
"X     c #000000000000",
"               ",
" X X X X X X X ",
"               ",
" X     X     X ",
"               ",
" X     X     X ",
"               ", 
" X X X X X X X ",
"               ",
" X     X     X ",
"               ",
" X     X     X ",
"               ",
" X X X X X X X ",
"               "};

static char *full   =" XXXXXXXXXXXXX ";
static char *dotted =" X X X X X X X ";
static char *side111=" X     X     X ";
static char *side000="               ";
static char *side101=" X           X ";
static char *side010="       X       ";
static char *side100=" X             ";
static char *side001="             X ";

/* Register type */
G_DEFINE_TYPE (TestMainWindow, test_main_window, GTK_TYPE_WINDOW);

static void
test_main_window_class_init (TestMainWindowClass *klass)
{
    /*GObjectClass *object_class = G_OBJECT_CLASS (klass); */

    /*object_class->finalize = test_main_window_finalize; */
}

static void
test_main_window_init (TestMainWindow *self)
{
    g_return_if_fail(self != NULL);
    g_return_if_fail(TEST_IS_MAIN_WINDOW(self));

    GtkWidget *scrwin, *main_vbox;
    guint i;

    gtk_window_set_title(GTK_WINDOW(self), "GtkSheet Demo");
    gtk_widget_set_size_request(GTK_WIDGET(self), 500, 500);
    g_signal_connect (self, "destroy", (GCallback) destroy_cb, NULL);


    /* Create the show/hide row and column titles*/
    GtkWidget *hide_row_titles = gtk_button_new_with_label("Hide Row Titles"); 
    GtkWidget *hide_column_titles = gtk_button_new_with_label("Hide Column Titles"); 
    GtkWidget *show_row_titles = gtk_button_new_with_label("Show Row Titles"); 
    GtkWidget *show_column_titles = gtk_button_new_with_label("Show Column Titles");

    GtkWidget *show_hide_box = gtk_hbox_new(FALSE, 1); 
    gtk_box_pack_start(GTK_BOX(show_hide_box), hide_row_titles, TRUE, TRUE, 0); 
    gtk_box_pack_start(GTK_BOX(show_hide_box), hide_column_titles, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(show_hide_box), show_row_titles, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(show_hide_box), show_column_titles, TRUE, TRUE, 0);

    g_signal_connect(GTK_OBJECT(hide_row_titles), "clicked",
	         (GCallback) do_hide_row_titles, self);

    g_signal_connect(GTK_OBJECT(hide_column_titles), "clicked",
	         (GCallback) do_hide_column_titles, self);

    g_signal_connect(GTK_OBJECT(show_row_titles), "clicked",
	         (GCallback) do_show_row_titles, self);

    g_signal_connect(show_column_titles, "clicked",
	         (GCallback) do_show_column_titles, self);

    /* Create the sheets */
    self->notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(self->notebook), GTK_POS_BOTTOM);

    _create_sheet1(self);
    _create_sheet2(self);
    _create_sheet3(self);
        
    for (i=0; i < N_EXAMPLES; i++)
    {
        GtkSheet *sheet = self->sheets[i];
        GtkWidget *label;
        gchar text[100];

        g_signal_connect(G_OBJECT(sheet), "activate",
                         (GCallback)activate_sheet_cell_cb, self);
        g_signal_connect(G_OBJECT(sheet), "notify::selected-range", 
                         (GCallback)_selected_range_cb, self);
        g_signal_connect( gtk_sheet_get_entry(sheet), "changed", 
                          (GCallback)show_entry_cb, self);
        /*g_signal_connect( self->sheet, "changed", (GCallback)sheet_changed_cb, self);*/
        scrwin = gtk_scrolled_window_new(NULL, NULL);
        g_snprintf(text, 100, "Example %d", i);
        label = gtk_label_new(text);
        gtk_container_add(GTK_CONTAINER(scrwin), GTK_WIDGET(sheet));
        gtk_notebook_append_page(GTK_NOTEBOOK(self->notebook), 
                                 GTK_WIDGET(scrwin), label);
    }

    /* Create the toolbar */
    _create_toolbar(self);

    /* Create the spreadsheet entry */
    GtkRequisition request;

    GtkWidget *status_box = gtk_hbox_new(FALSE, 1);
    gtk_container_set_border_width(GTK_CONTAINER(status_box),0);
    
    self->location = gtk_label_new("");
    gtk_widget_size_request(self->location, &request); 
    gtk_widget_set_size_request(self->location, 160, request.height);
    gtk_box_pack_start(GTK_BOX(status_box), self->location, FALSE, TRUE, 0);

    self->entry=gtk_entry_new(); 
    gtk_box_pack_start(GTK_BOX(status_box), self->entry, TRUE, TRUE, 0); 

    g_signal_connect(self->entry, "activate", (GCallback)activate_entry_cb, self);

    /* Pack widgets */
    main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_box_pack_start(GTK_BOX(main_vbox), show_hide_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), self->toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), status_box, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), self->notebook, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(self), main_vbox);

    gtk_widget_show_all( GTK_WIDGET(self) );
} 

void
_create_border_combo(TestMainWindow *self) 
{
    GtkWidget *combo;
    GtkListStore *store;
    GdkPixbuf *pixbuf;
    const char *border[18];
    GtkTreeIter iter = {0, 0, 0, 0};
    GtkCellRenderer *cell;
    guint i;

    store = gtk_list_store_new(1, GDK_TYPE_PIXBUF);

    for(i=0; i<18; i++)
        border[i]=xpm_border[i];

    /* EMPTY */
    pixbuf = gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* TOP */    
    border[4] = full;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* BOTTOM */
    border[4] = dotted;
    border[16] = full;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* RIGHT */
    border[16]=dotted;
    for (i=5; i<16; i+=2)
        border[i] = side001;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* LEFT */
    for (i=5; i<16; i+=2)
        border[i] = side100;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* V101 */
    for (i=5; i<16; i+=2)
        border[i] = side101;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* H101 */
    for (i=5; i<16; i+=2)
        border[i] = side000;
    border[4] = full;
    border[16] = full;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* V111 */
    border[4] = dotted;
    border[16] = dotted;
    for (i=5; i<16; i+=2)
        border[i] = side111;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* H111 */
    for (i=5; i<16; i+=2)
        border[i] = side000;
    border[4] = full;
    border[16] = full;
    border[10] = full;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* CROSS */
    border[4] = dotted;
    border[16] = dotted;
    for (i=5; i<16; i+=2)
        border[i] = side010;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* SIDES */
    for (i=5; i<16; i+=2)
        border[i] = side101;
    border[4] = full;
    border[16] = full;
    border[10] = dotted;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    /* FULL */
    for (i=5; i<16; i+=2)
        border[i] = side111;
    border[4]=full;
    border[10]=full;
    border[16]=full;
    pixbuf =  gdk_pixbuf_new_from_xpm_data(border);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, pixbuf, -1);

    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(store));
    cell = gtk_cell_renderer_pixbuf_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), cell, TRUE);
    gtk_cell_layout_add_attribute(GTK_CELL_LAYOUT(combo), cell, "pixbuf", 0);
    gtk_combo_box_set_wrap_width(GTK_COMBO_BOX(combo), 4);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    
    self->border_combo = combo;
}


GtkSheet *
get_current_sheet(TestMainWindow *self)
{   
    gint page = gtk_notebook_get_current_page( GTK_NOTEBOOK(self->notebook) );

    if (page < 0 || page >= N_EXAMPLES)
        return NULL;
    else
        return self->sheets[page];
        
}

void
_create_sheet1(TestMainWindow *self)
{
    GtkSheet *sheet;
    GdkColor bg, grid, color;
    GValue val = {0, }, val2 = {0,};
    gchar *s;
    gchar name[10];
    GtkSheetRange range;
    const gchar *font_name1="Arial 36";
    const gchar *font_name2="Arial 28";
    PangoFontDescription *font;

    sheet = GTK_SHEET(gtk_sheet_new(1000,26, NULL));

    GdkColormap *colormap = gtk_widget_get_colormap(GTK_WIDGET(sheet));
    gdk_color_parse("light yellow", &bg);
    gdk_color_parse("light blue", &grid);
    gdk_colormap_alloc_color(colormap, &bg, TRUE, TRUE);
    gdk_colormap_alloc_color(colormap, &grid, TRUE, TRUE);

    gtk_sheet_set_background(sheet, &bg);
    gtk_sheet_set_grid(sheet, &grid);

    /* Test initializing with a NULL title */
    g_value_init(&val, G_TYPE_STRING);
    g_object_get_property(G_OBJECT(sheet),"title",  &val);
    s = g_value_dup_string(&val);
    g_message("Sheet title is %s", s);
    g_free(s);
    g_message("Resetting gvalue");

    g_value_init(&val2, G_TYPE_STRING);
    g_message("Setting gvalue");
    g_value_set_string(&val2, g_strdup("Sheet Example"));
    g_message("Setting title property again");
    g_object_set_property(G_OBJECT(sheet), "title", &val2);

    /* Se column names*/
    guint i, cols = gtk_sheet_get_columns_count(sheet);
    for (i=0; i< cols; i++)
    { 
        name[0] =  'A' + i;
        name[1] = '\0';
        gtk_sheet_column_button_add_label(GTK_SHEET(sheet), i, name);
        gtk_sheet_set_column_title(GTK_SHEET(sheet), i, name);
    }
    gtk_sheet_row_button_add_label(sheet, 0, "This is\na multiline\nlabel");
    gtk_sheet_row_button_add_label(sheet, 1, "This is long label");
    gtk_sheet_row_button_justify(sheet, 0, GTK_JUSTIFY_RIGHT);

    range.row0=1;
    range.rowi=2;
    range.col0=1;
    range.coli=3;
    gtk_sheet_clip_range(sheet, &range);
    font = pango_font_description_from_string (font_name2);
    gtk_sheet_range_set_font(sheet, &range, font);
    gdk_color_parse("red", &color);
    gdk_color_alloc(colormap, &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);
    gtk_sheet_set_cell(sheet, 1,2, GTK_JUSTIFY_CENTER,
                       "Welcome to");

    range.row0=2;
    font = pango_font_description_from_string (font_name1);
    gtk_sheet_range_set_font(sheet, &range, font);
    gdk_color_parse("blue", &color);
    gdk_color_alloc(colormap, &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);

    gtk_sheet_set_cell(sheet, 2,2, GTK_JUSTIFY_CENTER, "GtkSheet");

    range.row0=3;
    range.rowi=3;
    range.col0=0;
    range.coli=4;
    gdk_color_parse("dark gray", &color);
    gdk_color_alloc(colormap, &color);
    gtk_sheet_range_set_background(sheet, &range, &color);
    gdk_color_parse("green", &color);
    gdk_color_alloc(colormap, &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);

    gtk_sheet_set_cell(sheet,3,2,GTK_JUSTIFY_CENTER,
        "a Matrix widget for Gtk+");

    gtk_sheet_set_cell(sheet,4,1,GTK_JUSTIFY_LEFT,
        "GtkSheet is a matrix where you can allocate cells of text.");
    gtk_sheet_set_cell(sheet,5,1,GTK_JUSTIFY_LEFT,
        "Cell contents can be edited interactively with an specially designed entry");
    gtk_sheet_set_cell(sheet,6,1,GTK_JUSTIFY_LEFT,
        "You can change colors, borders, and many other attributes");
    gtk_sheet_set_cell(sheet, 7,1, GTK_JUSTIFY_LEFT,
        "Drag & drop or resize the selection clicking the corner or border");
    gtk_sheet_set_cell(sheet, 8, 1, GTK_JUSTIFY_LEFT,
        "Store the selection on the clipboard pressing Ctrl-C");
    gtk_sheet_set_cell(sheet, 9, 1, GTK_JUSTIFY_LEFT,
        "You can add buttons, charts, pixmaps, and other widgets");

    GtkWidget *bullet[10];
    GdkBitmap *mask;
    GdkPixmap *pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, 
                                                              &mask, NULL,
			                                                  bullet_xpm);
    for (i=0; i<=5; i++) {
        bullet[i] = gtk_image_new_from_pixmap(pixmap, mask);
        gtk_sheet_attach(sheet, bullet[i], 4+i, 0, GTK_EXPAND, GTK_EXPAND, 0, 0);
    }
    g_object_unref(pixmap);
    g_object_unref(mask);

    pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap, &mask, NULL,
			                                       smile_xpm);
    GtkWidget *smile = gtk_image_new_from_pixmap(pixmap, mask);
    gtk_sheet_button_attach(sheet, smile, -1, 5);
    gdk_pixmap_unref(pixmap);
    gdk_bitmap_unref(mask);

    self->curve = gtk_curve_new();
    gtk_widget_show(self->curve);
    gtk_curve_set_range(GTK_CURVE(self->curve), 0, 200, 0, 200);

    GtkWidget *show_button = gtk_button_new_with_label("Show me a plot");
    gtk_widget_set_size_request(show_button, 100,60);
    gtk_sheet_attach(sheet, show_button, 12, 2, GTK_FILL, GTK_FILL, 5, 5);

    g_signal_connect(show_button, "clicked", (GCallback)show_child_cb, self);

    g_signal_connect (sheet,
                        "key_press_event",
                        (GCallback) example1_key_press_cb, 
                        self);

    self->sheets[0] = sheet;
}

void
_create_sheet2(TestMainWindow *self)
{
    GtkSheetRange range;
    GdkColor color;
    GtkWidget *b;

    GtkWidget *widget = gtk_sheet_new(1000, 26, "Example 2");
    GtkSheet *sheet = GTK_SHEET(widget);

    gtk_sheet_set_autoscroll(sheet, TRUE);
    /*gtk_sheet_set_selection_mode(sheet, GTK_SELECTION_SINGLE);*/

    range.row0=0;
    range.rowi=2;
    range.col0=0;
    range.coli=sheet->maxcol;
    gtk_sheet_range_set_editable(sheet, &range, FALSE);
    gdk_color_parse("light gray", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_background(sheet, &range, &color);
    gdk_color_parse("blue", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);
    range.row0=1;
    gdk_color_parse("red", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);
    range.row0=2;
    gdk_color_parse("black", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);

    gtk_sheet_set_cell(sheet, 0,2, GTK_JUSTIFY_CENTER,
                    "Click the right mouse button to display a popup");
    gtk_sheet_set_cell(sheet, 1,2, GTK_JUSTIFY_CENTER,
                    "You can connect a parser to the 'set cell' signal");
    gtk_sheet_set_cell(sheet, 2,2, GTK_JUSTIFY_CENTER,
                    "(Try typing numbers)");
    gtk_sheet_set_active_cell(sheet, 3, 0);
    
    g_signal_connect(sheet,
                    "button_press_event",
                    (GCallback) do_popup, 
                    self);

    g_signal_connect(sheet,
                    "set_cell",
                    (GCallback) parse_numbers,
                    NULL);                   
    /*
    gtk_signal_connect(GTK_OBJECT(sheet),
                    "activate",
                    (GtkSignalFunc) alarm_activate, 
                    NULL);

    gtk_signal_connect(GTK_OBJECT(sheet),
                    "deactivate",
                    (GtkSignalFunc) alarm_deactivate, 
                    NULL);

    gtk_signal_connect(GTK_OBJECT(sheet),
                    "traverse",
                    (GtkSignalFunc) alarm_traverse, 
                    NULL);*/

    gtk_sheet_set_row_height(sheet, 12, 60);
    b = gtk_button_new_with_label("GTK_FILL");
    gtk_sheet_attach(sheet, b, 12, 2, GTK_FILL, GTK_FILL, 5, 5);
    b = gtk_button_new_with_label("GTK_EXPAND");
    gtk_sheet_attach(sheet, b, 12, 3, GTK_EXPAND, GTK_EXPAND, 5, 5);
    b = gtk_button_new_with_label("GTK_SHRINK");
    gtk_sheet_attach(sheet, b, 12, 4, GTK_SHRINK, GTK_SHRINK, 5, 5);

    self->sheets[1] = sheet;
}

void
_create_sheet3(TestMainWindow *self)
{
    GtkSheet *sheet;
    GtkSheetRange range;
    GdkColor color; 
    GtkWidget *widget = gtk_sheet_new(1000, 26, "Example 3");
    sheet=GTK_SHEET(widget);

    gtk_sheet_show_grid(sheet, FALSE);

    range.row0=0;
    range.rowi=10;
    range.col0=0;
    range.coli=6;
    gdk_color_parse("orange", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_background(sheet, &range, &color);
    gdk_color_parse("violet", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_foreground(sheet, &range, &color);
    range.row0=1;
    gdk_color_parse("blue", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_background(sheet, &range, &color);
    range.coli=0;
    gdk_color_parse("dark green", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_background(sheet, &range, &color);

    range.row0=0; 
    gdk_color_parse("dark blue", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, GTK_SHEET_RIGHT_BORDER, 4, 1);
    range.coli=0;
    range.col0=0;
    range.rowi=0;
    gdk_color_parse("red", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_background(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, GTK_SHEET_RIGHT_BORDER|
                                          GTK_SHEET_BOTTOM_BORDER, 4, 0);
    range.rowi=0;
    range.col0=1;
    range.coli=6;
    gdk_color_parse("dark blue", &color);
    gdk_color_alloc(gtk_widget_get_colormap(widget), &color);
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, GTK_SHEET_BOTTOM_BORDER, 4, 1);

    gtk_sheet_set_autoresize(sheet, TRUE);

    gtk_sheet_change_entry(sheet, GTK_TYPE_COMBO);


    g_signal_connect(sheet, "traverse", (GCallback) change_entry, NULL);

    self->sheets[2] = sheet;
}

void
_create_toolbar(TestMainWindow *self)
{
    self->toolbar = gtk_toolbar_new();
    self->tooltips = gtk_tooltips_new();
        /*font button*/
    self->font_button = gtk_font_button_new();
    g_signal_connect( G_OBJECT(self->font_button), "font-set",
                      (GCallback)font_changed_cb, self);

    _add_widget_to_toolbar(self, self->font_button, TRUE, 
                           "Change the font of the selected cells");
        /*text justification buttons */
    self->justify_left = gtk_action_new("justleft",  NULL,
                                        "Justify selected cells to the left",
                                        GTK_STOCK_JUSTIFY_LEFT);
    g_signal_connect(self->justify_left, "activate", (GCallback)justify_left_cb, self);
    gtk_toolbar_insert( GTK_TOOLBAR(self->toolbar), 
                        GTK_TOOL_ITEM(gtk_action_create_tool_item(self->justify_left)), -1);
    self->justify_center = gtk_action_new("justcenter",  NULL,
                                        "Justify selected cells to the center",
                                        GTK_STOCK_JUSTIFY_CENTER);
    g_signal_connect(self->justify_center, "activate", (GCallback)justify_center_cb, self);
    gtk_toolbar_insert( GTK_TOOLBAR(self->toolbar), 
                        GTK_TOOL_ITEM(gtk_action_create_tool_item(self->justify_center)), -1);
    self->justify_right = gtk_action_new("justright",  NULL,
                                        "Justify selected cells to the right",
                                        GTK_STOCK_JUSTIFY_RIGHT);
    g_signal_connect(self->justify_right, "activate", (GCallback)justify_right_cb, self);
    gtk_toolbar_insert( GTK_TOOLBAR(self->toolbar), 
                        GTK_TOOL_ITEM(gtk_action_create_tool_item(self->justify_right)), -1);
    gtk_toolbar_insert(GTK_TOOLBAR(self->toolbar), 
                       gtk_separator_tool_item_new(),
                       -1);
        /* background/foreground color buttons */
    self->fg_color_button = gtk_color_button_new();
    g_signal_connect(G_OBJECT(self->fg_color_button), "color-set", 
                     (GCallback)color_changed_cb, self);
    _add_widget_to_toolbar(self, self->fg_color_button, FALSE, 
                           "Change the foreground color of the selected cells");

    self->bg_color_button = gtk_color_button_new();
    g_signal_connect(G_OBJECT(self->bg_color_button), "color-set", 
                     (GCallback)color_changed_cb, self);
    _add_widget_to_toolbar(self, self->bg_color_button, TRUE, 
                           "Change the background color of the selected cells");

       /* Create the combo box */
    _create_border_combo(self);
    g_signal_connect(self->border_combo, "changed", 
                     (GCallback)border_changed_cb, self);    
    _add_widget_to_toolbar(self, self->border_combo, FALSE,
                           "Change the border of the selected cells");
}

void
_add_widget_to_toolbar(TestMainWindow *self, GtkWidget *widget, gboolean separator, const gchar *tooltip)
{
    GtkToolItem *ti = gtk_tool_item_new();

    gtk_container_add(GTK_CONTAINER(ti), widget);
    if (tooltip)
        gtk_tool_item_set_tooltip_text( ti, tooltip);
    gtk_toolbar_insert(GTK_TOOLBAR(self->toolbar), ti, -1);
    if (separator)
       gtk_toolbar_insert(GTK_TOOLBAR(self->toolbar), 
                          gtk_separator_tool_item_new(),
                          -1);
}

static gint
popup_activated(GtkWidget *widget, gpointer data)
{
    PopupData *popup_data = (PopupData *)data;
    const gchar *item = popup_data->menu_text;
    TestMainWindow *self = popup_data->main_win;
    GtkSheet *sheet = get_current_sheet(self);    

    if(strcmp(item,"Append Column")==0)
        gtk_sheet_add_column(sheet,1);

    if(strcmp(item,"Append Row")==0)
        gtk_sheet_add_row(sheet,1);

    if(strcmp(item,"Insert Row")==0){
        if(sheet->state==GTK_SHEET_ROW_SELECTED)
            gtk_sheet_insert_rows(sheet, sheet->range.row0,                       
                                         sheet->range.rowi - sheet->range.row0+1);
    }

    if(strcmp(item,"Insert Column")==0){
        if(sheet->state==GTK_SHEET_COLUMN_SELECTED)
            gtk_sheet_insert_columns(sheet, sheet->range.col0,                       
                                            sheet->range.coli - sheet->range.col0+1);
    } 

    if(strcmp(item,"Delete Row")==0){
        if(sheet->state==GTK_SHEET_ROW_SELECTED)
            gtk_sheet_delete_rows(sheet, sheet->range.row0,
                                         sheet->range.rowi - sheet->range.row0+1);
    }

    if(strcmp(item,"Delete Column")==0){
        if(sheet->state==GTK_SHEET_COLUMN_SELECTED)
            gtk_sheet_delete_columns(sheet, sheet->range.col0,
                                            sheet->range.coli - sheet->range.col0+1);   
    } 

    if(strcmp(item,"Clear Cells")==0){
        if(sheet->state!=GTK_SHEET_NORMAL)
            gtk_sheet_range_clear(sheet, &sheet->range);
    } 

    gtk_widget_destroy(self->popup);
    return TRUE;
}

void
destroy_data(gpointer *data, GClosure *closure)
{   
    g_free(data);
}

static GtkWidget *
build_menu(TestMainWindow *self, GtkSheet *sheet)
{
	static char *items[]={
        "Append Column",
        "Append Row",
        "Insert Row",
        "Insert Column",
        "Delete Row",
        "Delete Column",
        "Clear Cells"
	};    
	GtkWidget *menu;
	GtkWidget *item;
	int i;
    PopupData *data;

	menu = gtk_menu_new();

	for (i=0; i < (sizeof(items) / sizeof(items[0])) ; i++) {
        item = gtk_menu_item_new_with_label(items[i]);

        data = g_new( PopupData, 1);
        data->main_win = self;
        data->menu_text = items[i];
        g_signal_connect_data(item, "activate", 
                              (GCallback) popup_activated, data, 
                              (GClosureNotify)destroy_data, 0);

        GTK_WIDGET_SET_FLAGS (item, GTK_SENSITIVE | GTK_CAN_FOCUS);
        switch(i){
          case 2:
            if (sheet->state != GTK_SHEET_ROW_SELECTED)
              GTK_WIDGET_UNSET_FLAGS (item, 
                                      GTK_SENSITIVE | GTK_CAN_FOCUS);
            break;
          case 3:
            if (sheet->state != GTK_SHEET_COLUMN_SELECTED)
              GTK_WIDGET_UNSET_FLAGS (item, 
                                      GTK_SENSITIVE | GTK_CAN_FOCUS);
            break;
          case 4:
            if (sheet->state != GTK_SHEET_ROW_SELECTED)
              GTK_WIDGET_UNSET_FLAGS (item, 
                                      GTK_SENSITIVE | GTK_CAN_FOCUS);
            break;
          case 5:
            if (sheet->state != GTK_SHEET_COLUMN_SELECTED)
              GTK_WIDGET_UNSET_FLAGS (item, 
                                      GTK_SENSITIVE | GTK_CAN_FOCUS);
            break;
        } 

        gtk_widget_show(item);
	    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	}

	return menu;
}

gboolean
do_popup(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    g_return_val_if_fail(TEST_IS_MAIN_WINDOW(data), FALSE);

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GdkModifierType mods;
    GtkWidget *sheet;

    sheet = GTK_WIDGET(widget);
    

    gdk_window_get_pointer (sheet->window, NULL, NULL, &mods);
    if (mods & GDK_BUTTON3_MASK)
    {
        /*if (self->popup)
           g_free(self->popup);*/

        self->popup = build_menu(self, GTK_SHEET(sheet));

        gtk_menu_popup(GTK_MENU(self->popup), NULL, NULL, NULL, NULL,
                       event->button, event->time);
    }

    return FALSE;
}

void
format_text (GtkSheet *sheet, const gchar *text, gint *justification, char *label)
{
  double auxval;
  int digspace=0;
  int cell_width, char_width;
  double val = 0.0;
  int format;
  double space;
  int intspace;
  int nonzero=FALSE;
  int ipos;
  char nchar;
  PangoContext *context;
  PangoFontMetrics *metrics;
  size_t len;
  enum {EMPTY, TEXT, NUMERIC};

  context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
  metrics = pango_context_get_metrics(context, 
                                      GTK_WIDGET(sheet)->style->font_desc, 
                                      pango_context_get_language(context));
  char_width = pango_font_metrics_get_approximate_char_width(metrics);
  pango_font_metrics_unref(metrics);

  cell_width = sheet->column[sheet->active_cell.col].width;
  space = (double)cell_width/(double)char_width;

  intspace = MIN(space, DEFAULT_SPACE);

  format = EMPTY;
  len = strlen(text);
  if (len != 0)
  { 
    for(ipos=0; ipos<len; ipos++) {
        nchar = text[ipos];
        switch (nchar) {
            case '.':
            case ' ': case ',':
            case '-': case '+':
            case 'd': case 'D':
            case 'E': case 'e':
            case '1': case '2': case '3': case '4': case '5': case '6':
            case '7': case '8': case '9':
                nonzero=TRUE;
                break;
            case '0':
                break;
            default:
                format=TEXT;
        }
        if(format != EMPTY) 
            break;
    }
    val = atof(text);
    if (format!=EMPTY || (val==0. && nonzero))
        format = TEXT;
    else
        format = NUMERIC;
  }

  switch (format) {
    case TEXT:
    case EMPTY:
      strcpy(label, text);
      return;
    case NUMERIC:
      val=atof(text);
      *justification = GTK_JUSTIFY_RIGHT;
  }

  auxval= val < 0 ? -val : val;

  while (auxval<1 && auxval != 0.) {
    auxval=auxval*10.;
    digspace+=1;
  }

  if ((digspace+DEFAULT_PRECISION+1 > intspace) || (digspace > DEFAULT_PRECISION)) {
    sprintf (label, "%*.*E", intspace, DEFAULT_PRECISION, val);
  } else
  {
    intspace=MIN(intspace, strlen(text)-digspace-1);
    sprintf (label, "%*.*f", intspace, DEFAULT_PRECISION, val);
    if(strlen(label) > space)
      sprintf (label, "%*.*E", intspace, DEFAULT_PRECISION, val);
  }
}

void
parse_numbers(GtkWidget *widget, gpointer data)
{
    GtkSheet *sheet;
    char label[400];
    gint justification;
    GtkSheetCellAttr attr;

    sheet=GTK_SHEET(widget);

    gtk_sheet_get_attributes(sheet, sheet->active_cell.row,
                                    sheet->active_cell.col,
                                    &attr); 

    justification = attr.justification; 
    format_text(sheet, gtk_entry_get_text(GTK_ENTRY(sheet->sheet_entry)), 
                       &justification, label);

    gtk_sheet_set_cell(sheet, sheet->active_cell.row,
                              sheet->active_cell.col,
                              justification, label); 
}

gboolean
change_entry(GtkWidget *widget, 
             gint row, gint col, gint *new_row, gint *new_col,
             gpointer data)
{
  GtkSheet *sheet;

  sheet = GTK_SHEET(widget);

  if(*new_col == 0 && (col != 0 || sheet->state != GTK_STATE_NORMAL))
         gtk_sheet_change_entry(sheet, GTK_TYPE_COMBO);

  if(*new_col == 1 && (col != 1 || sheet->state != GTK_STATE_NORMAL))
         gtk_sheet_change_entry(sheet, GTK_TYPE_ENTRY);

  if(*new_col == 2 && (col != 2 || sheet->state != GTK_STATE_NORMAL))
         gtk_sheet_change_entry(sheet, GTK_TYPE_SPIN_BUTTON);

  if(*new_col >= 3 && (col < 3 || sheet->state != GTK_STATE_NORMAL))
         gtk_sheet_change_entry(sheet, GTK_TYPE_ITEM_ENTRY);

  return TRUE;
}

gboolean
copy_to_clipboard(GtkSheet *sheet)
{
    guint row, col;
    gint colspan = sheet->range.coli - sheet->range.col0;
    gint rowspan = sheet->range.rowi - sheet->range.row0;

    if ((colspan < 0 || rowspan < 0) || (colspan == 0 && rowspan == 0))
        return FALSE;

    GString *buf = g_string_sized_new((colspan+1)*rowspan);    

    for (row=sheet->range.row0; row<=sheet->range.rowi; row++) 
    {
        for (col=sheet->range.col0; col<=sheet->range.coli; col++) 
        {
            /* Get cell text an determine length*/
            const gchar *text = gtk_sheet_cell_get_text(sheet, row, col);
            if (text) {
                if (col == sheet->range.coli)
                    g_string_append_printf(buf, "%s\n", text);
                else
                    g_string_append_printf(buf, "%s\t", text);
            } else {
                if (col == sheet->range.coli)
                    g_string_append_c(buf, '\n');
                else
                    g_string_append_c(buf, '\t');
            }
        }        
    }

    gtk_clipboard_set_text( gtk_clipboard_get(GDK_SELECTION_CLIPBOARD),
                            buf->str,
                            buf->len);
    g_string_free(buf, TRUE);

    return TRUE;
}

gboolean 
example1_key_press_cb(GtkWidget *widget, GdkEventKey *key)
{
    GtkSheet *sheet;
    gboolean copy = FALSE;

    sheet = GTK_SHEET(widget);

    if (key->state & GDK_CONTROL_MASK || key->keyval==GDK_Control_L ||
        key->keyval==GDK_Control_R) 
    {
        if((key->keyval=='c' || key->keyval == 'C') && sheet->state != GTK_STATE_NORMAL) {
            if(gtk_sheet_in_clip(sheet)) 
                gtk_sheet_unclip_range(sheet);
            gtk_sheet_clip_range(sheet, &sheet->range);
            /*gtk_sheet_unselect_range(sheet);*/
            copy = TRUE;   
        }
        if(key->keyval=='x' || key->keyval == 'X') {
            gtk_sheet_unclip_range(sheet);    
            copy = TRUE;
        }
    }
    
    if (copy)
        copy_to_clipboard(sheet);
        
    return FALSE;
}

void show_child_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));
    
    TestMainWindow *self = TEST_MAIN_WINDOW(data);

    if(!GTK_WIDGET_MAPPED(self->curve))
         gtk_sheet_attach_floating(self->sheets[0], self->curve, 2, 7);
}

void activate_entry_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    GtkEntry *sheet_entry = GTK_ENTRY(gtk_sheet_get_entry(sheet));
    gint row, col;
    gint justification = GTK_JUSTIFY_LEFT;
  
    row=sheet->active_cell.row; col=sheet->active_cell.col;

    if(GTK_IS_ITEM_ENTRY(sheet_entry))
        justification = GTK_ITEM_ENTRY(sheet_entry)->justification;

    gtk_sheet_set_cell(sheet, row, col,
                              justification,
                              gtk_entry_get_text (sheet_entry));
}

void show_sheet_entry_cb(GtkWidget *widget, gpointer data)
{

    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    if (!GTK_WIDGET_HAS_FOCUS(widget)) return;

    const char *text;
    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    GtkEntry *sheet_entry = GTK_ENTRY( gtk_sheet_get_entry(sheet) );

    text = gtk_entry_get_text(GTK_ENTRY(self->entry));
    if (text)
        gtk_entry_set_text( sheet_entry, text );
}

void  show_entry_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    if (!GTK_WIDGET_HAS_FOCUS(widget)) return;

    const char *text;
    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    GtkEntry *sheet_entry = GTK_ENTRY(gtk_sheet_get_entry(sheet));
    
    text = gtk_entry_get_text( sheet_entry );
    if (text)
        gtk_entry_set_text( GTK_ENTRY(self->entry), text );
}

void justify_center_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    gtk_sheet_range_set_justification( sheet, &(sheet->range),
                                       GTK_JUSTIFY_CENTER);
}

void justify_left_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    gtk_sheet_range_set_justification( sheet, &(sheet->range),
                                       GTK_JUSTIFY_LEFT);
}

void justify_right_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    gtk_sheet_range_set_justification( sheet, &(sheet->range),
                                       GTK_JUSTIFY_RIGHT);
}


void
destroy_cb (GtkWidget *widget, gpointer data)
{
  gtk_main_quit();
}

gint
activate_sheet_cell_cb(GtkWidget *widget, gint row, gint column, gpointer data) 
{
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);
    g_return_val_if_fail(TEST_IS_MAIN_WINDOW(data), FALSE);

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    const gchar *text, *coltitle, *rowtitle;
    GtkSheetCellAttr attrs;
    PangoFontDescription *font_desc;
    char *font_name;
    gchar cell[100];
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return FALSE;

    coltitle = gtk_sheet_get_column_title(sheet, column);
    rowtitle = gtk_sheet_get_row_title(sheet, row);
    if (coltitle && rowtitle)
        g_snprintf(cell, 100, "  %s:%s  ", coltitle, rowtitle);
    else if (coltitle)
        g_snprintf(cell, 100, "  %s:%d  ", coltitle, row);
    else if (rowtitle)
        g_snprintf(cell, 100, "  %d:%s  ", column, rowtitle);
    else
        g_snprintf(cell, 100, "  %d:%d  ", column, row);
    gtk_label_set_text( GTK_LABEL(self->location), cell);

    text = gtk_sheet_cell_get_text(sheet, row, column);
    if (text)
        gtk_entry_set_text(GTK_ENTRY(self->entry), text);
    else
        gtk_entry_set_text(GTK_ENTRY(self->entry), "");

    gtk_sheet_get_attributes(sheet, row, column, &attrs);

    gtk_entry_set_editable(GTK_ENTRY(self->entry), attrs.is_editable);

    font_desc = (attrs.font_desc == NULL)? widget->style->font_desc : attrs.font_desc;
    font_name = pango_font_description_to_string(font_desc);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(self->font_button), font_name);
    g_free(font_name);

    GValue val = {0, };
    g_value_init(&val, G_TYPE_VALUE_ARRAY);
    g_object_get_property(G_OBJECT(sheet), "active-cell", &val);
    GValueArray *active_cell_array = g_value_get_boxed(&val);
    if (!active_cell_array)
        g_warning("Could not retrieve the active cell!");
    
    gtk_color_button_set_color(GTK_COLOR_BUTTON(self->bg_color_button),
                               &(attrs.background));
    gtk_color_button_set_color(GTK_COLOR_BUTTON(self->fg_color_button),
                               &(attrs.foreground));

    return TRUE;
}

void
color_changed_cb(GtkWidget *widget, gpointer data) 
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GdkColor color;
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
    if (widget == self->bg_color_button)
        gtk_sheet_range_set_background(sheet, &(sheet->range), &color);
    else
        gtk_sheet_range_set_foreground(sheet, &(sheet->range), &color);
    
}

void
font_changed_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));
    g_return_if_fail(GTK_IS_FONT_BUTTON(widget));

    GtkFontButton *fontbutton = GTK_FONT_BUTTON(widget);
    GtkSheet *sheet = get_current_sheet(TEST_MAIN_WINDOW(data));

    if (!sheet) return;


    const gchar *font_name = gtk_font_button_get_font_name(fontbutton);
    PangoFontDescription *fd = pango_font_description_from_string(font_name);
    if (fd)
    {
        gtk_sheet_range_set_font(sheet, &(sheet->range), fd);
        pango_font_description_free(fd);
    }
}

static void
border_changed_cb(GtkWidget *widget, gpointer data)
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    TestMainWindow *self = TEST_MAIN_WINDOW(data);
    GtkSheet *sheet = get_current_sheet(self);

    if (!sheet) return;

    GtkSheetRange range, auxrange;
    gint border_mask, border_width=3;
    gint auxcol, auxrow;
    gint i,j, border;

    range = sheet->range;
    gtk_sheet_range_set_border(sheet, &range, 0, 0, 0);

    border = gtk_combo_box_get_active( GTK_COMBO_BOX(widget) );

    switch (border) 
    {
        case 0:
            break;
        case 1:
            border_mask = GTK_SHEET_TOP_BORDER;
            range.rowi = range.row0;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 2:
            border_mask = GTK_SHEET_BOTTOM_BORDER;
            range.row0 = range.rowi;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 3:
            border_mask = GTK_SHEET_RIGHT_BORDER;
            range.col0 = range.coli;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 4:
            border_mask = GTK_SHEET_LEFT_BORDER;
            range.coli = range.col0;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 5:
            if(range.col0 == range.coli) {
                border_mask = GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER;
                gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
                break;
            }
            border_mask = GTK_SHEET_LEFT_BORDER;
            auxcol=range.coli;
            range.coli = range.col0;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            border_mask = GTK_SHEET_RIGHT_BORDER;
            range.col0 = range.coli=auxcol; 
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 6:
            if(range.row0 == range.rowi){
                border_mask = GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER;
                gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
                break;
            }
            border_mask = GTK_SHEET_TOP_BORDER;
            auxrow=range.rowi;
            range.rowi = range.row0;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            border_mask = GTK_SHEET_BOTTOM_BORDER;
            range.row0=range.rowi=auxrow; 
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 7:
            border_mask = GTK_SHEET_RIGHT_BORDER | GTK_SHEET_LEFT_BORDER;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 8:
            border_mask = GTK_SHEET_BOTTOM_BORDER | GTK_SHEET_TOP_BORDER;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
        case 9:
            gtk_sheet_range_set_border(sheet, &range, 15, border_width, 0);
            for (i=range.row0; i<=range.rowi; i++)
                for (j=range.col0; j<=range.coli; j++) {
                    border_mask = 15;
                    auxrange.row0 = i;
                    auxrange.rowi = i;
                    auxrange.col0 = j;
                    auxrange.coli = j;
                    if (i == range.rowi) border_mask ^= GTK_SHEET_BOTTOM_BORDER;
                    if (i == range.row0) border_mask ^= GTK_SHEET_TOP_BORDER;
                    if (j == range.coli) border_mask ^= GTK_SHEET_RIGHT_BORDER;
                    if (j == range.col0) border_mask ^= GTK_SHEET_LEFT_BORDER;
                    if(border_mask != 15)
                        gtk_sheet_range_set_border( sheet, &auxrange, border_mask, 
                                                    border_width, 0);
                }
            break;
        case 10:
            for(i=range.row0; i<=range.rowi; i++)
                for(j=range.col0; j<=range.coli; j++){
                    border_mask = 0;
                    auxrange.row0 = i;
                    auxrange.rowi = i;
                    auxrange.col0 = j;
                    auxrange.coli = j;
                    if(i == range.rowi) border_mask |= GTK_SHEET_BOTTOM_BORDER;
                    if(i == range.row0) border_mask |= GTK_SHEET_TOP_BORDER;
                    if(j == range.coli) border_mask |= GTK_SHEET_RIGHT_BORDER;
                    if(j == range.col0) border_mask |= GTK_SHEET_LEFT_BORDER;
                    if(border_mask != 0)
                        gtk_sheet_range_set_border(sheet, &auxrange, border_mask, 
                                                   border_width, 0);
                    }
            break;
        case 11:
            border_mask = 15;
            gtk_sheet_range_set_border(sheet, &range, border_mask, border_width, 0);
            break;
    }
}


static void
_selected_range_cb(GtkWidget *widget, gpointer data)
{
   GtkSheet *sheet = GTK_SHEET(widget);
   GValue val = {0,};
   GtkSheetRange *range;

   g_value_init(&val, GTK_TYPE_SHEET_RANGE);
   g_object_get_property(G_OBJECT(sheet), "selected-range", &val);
   range = g_value_get_boxed(&val);

   g_message("Selected range: Row %d to %d, Col %d  to %d",
             range->row0, range->rowi, 
             range->col0, range->coli);
   g_free(range);
}

void 
do_hide_row_titles(GtkWidget *widget, gpointer data) 
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    GtkSheet *sheet = get_current_sheet(TEST_MAIN_WINDOW(data));

    if (!sheet) return;

    gtk_sheet_hide_row_titles(sheet);
}

void 
do_hide_column_titles(GtkWidget *widget, gpointer data) 
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    GtkSheet *sheet = get_current_sheet(TEST_MAIN_WINDOW(data));

    if (!sheet) return;

    gtk_sheet_hide_column_titles(sheet);

}

void 
do_show_row_titles(GtkWidget *widget, gpointer data) 
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    GtkSheet *sheet = get_current_sheet(TEST_MAIN_WINDOW(data));

    if (!sheet) return;

    gtk_sheet_show_row_titles(sheet);
}

void 
do_show_column_titles(GtkWidget *widget, gpointer data) 
{
    g_return_if_fail(TEST_IS_MAIN_WINDOW(data));

    GtkSheet *sheet = get_current_sheet(TEST_MAIN_WINDOW(data));

    if (!sheet) return;

    gtk_sheet_show_column_titles(sheet);
}


int main(int argc, char *argv[])
{
    TestMainWindow *mainwin;

    gtk_init(&argc, &argv);

    mainwin = g_object_new(TEST_TYPE_MAIN_WINDOW, NULL);

    gtk_main();

    return 0;
}



