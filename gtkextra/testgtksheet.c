/* testgtksheet - GtkSheet widget for Gtk+.
 * Copyright (C) 1999-2001 Adrian E. Feiguin <adrian@ifir.ifir.edu.ar>
 *
 * Based on GtkClist widget by Jay Painter, but major changes.
 * Memory allocation routines inspired on SC (Spreadsheet Calculator)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <string.h>
#include "gtkextra.h"
#include "pixmaps.h"

#define DEFAULT_PRECISION 3
#define DEFAULT_SPACE 8
#define NUM_SHEETS 3 

GtkWidget *window;
GtkWidget *main_vbox;
GtkWidget *notebook;
GtkWidget **sheets;
GtkWidget **scrolled_windows;
GtkWidget *show_hide_box;
GtkWidget *status_box;
GtkWidget *location;
GtkWidget *entry;
GtkWidget *fgcolorcombo;
GtkWidget *bgcolorcombo;
GtkWidget *bordercombo;
GtkWidget *bg_pixmap;
GtkWidget *fg_pixmap;
GtkWidget *toolbar;
GtkWidget *left_button;
GtkWidget *center_button;
GtkWidget *right_button;
GtkWidget *tpixmap;
GtkWidget *bullet[10];
GtkWidget *smile;
GtkWidget *calendar;
GtkWidget *popup;


void
    quit ()
{
    gtk_main_quit();
}

static gint
    popup_activated(GtkWidget *widget, gpointer data)
{
    GtkSheet *sheet;
    gint cur_page;
    gchar *item;

    cur_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    sheet=GTK_SHEET(sheets[cur_page]);

    item = (gchar *)data;

    if (strcmp(item,"Add Column")==0)
        gtk_sheet_add_column(sheet,1);

    if (strcmp(item,"Add Row")==0)
        gtk_sheet_add_row(sheet,1);

    if (strcmp(item,"Insert Row")==0)
    {
        if (sheet->state==GTK_SHEET_ROW_SELECTED)
            gtk_sheet_insert_rows(sheet,sheet->range.row0,                       
                                  sheet->range.rowi-sheet->range.row0+1);
    }

    if (strcmp(item,"Insert Column")==0)
    {
        if (sheet->state==GTK_SHEET_COLUMN_SELECTED)
            gtk_sheet_insert_columns(sheet,sheet->range.col0,                       
                                     sheet->range.coli-sheet->range.col0+1);

    }

    if (strcmp(item,"Delete Row")==0)
    {
        if (sheet->state==GTK_SHEET_ROW_SELECTED)
            gtk_sheet_delete_rows(sheet,sheet->range.row0,
                                  sheet->range.rowi-sheet->range.row0+1);
    }

    if (strcmp(item,"Delete Column")==0)
    {
        if (sheet->state==GTK_SHEET_COLUMN_SELECTED)
            gtk_sheet_delete_columns(sheet,sheet->range.col0,
                                     sheet->range.coli-sheet->range.col0+1);
    }

    if (strcmp(item,"Clear Cells")==0)
    {
        if (sheet->state!=GTK_SHEET_NORMAL)
            gtk_sheet_range_clear(sheet, &sheet->range);
    }

    gtk_widget_destroy(popup);
    return (TRUE);
}

static GtkWidget *
    build_menu(GtkWidget *sheet)
{
    static char *items[]={
        "Add Column",
        "Add Row",
        "Insert Row",
        "Insert Column",
        "Delete Row",
        "Delete Column",
        "Clear Cells"
    };
    GtkWidget *menu;
    GtkWidget *item;
    int i;

    menu=gtk_menu_new();

    for (i=0; i < (sizeof(items)/sizeof(items[0])) ; i++)
    {
        item=gtk_menu_item_new_with_label(items[i]);

        g_signal_connect(G_OBJECT(item),"activate",
                         (void *) popup_activated,
                         items[i]);

        gtk_widget_set_sensitive(GTK_WIDGET(item), TRUE);
        gtk_widget_set_can_focus(GTK_WIDGET(item), TRUE);

        switch (i)
        {
            case 2:
                if (GTK_SHEET(sheet)->state!=GTK_SHEET_ROW_SELECTED)
                {
                    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
                    gtk_widget_set_can_focus(GTK_WIDGET(item), FALSE);
                }
                break;

            case 3:
                if (GTK_SHEET(sheet)->state!=GTK_SHEET_COLUMN_SELECTED)
                {
                    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
                    gtk_widget_set_can_focus(GTK_WIDGET(item), FALSE);
                }
                break;

            case 4:
                if (GTK_SHEET(sheet)->state!=GTK_SHEET_ROW_SELECTED)
                {
                    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
                    gtk_widget_set_can_focus(GTK_WIDGET(item), FALSE);
                }
                break;

            case 5:
                if (GTK_SHEET(sheet)->state!=GTK_SHEET_COLUMN_SELECTED)
                {
                    gtk_widget_set_sensitive(GTK_WIDGET(item), FALSE);
                    gtk_widget_set_can_focus(GTK_WIDGET(item), FALSE);
                }
                break;
        } 

        gtk_widget_show(item);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }

    return (menu);
}

gint
    do_popup(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    GdkModifierType mods;
    GtkWidget *sheet = GTK_WIDGET(widget);

    gdk_window_get_device_position (event->window, event->device,
	NULL, NULL, &mods);

    if (mods&GDK_BUTTON3_MASK)
    {
        if (popup)
        {
            gtk_widget_destroy(GTK_WIDGET(popup));
            popup = NULL;
        }

        popup=build_menu(sheet);

        gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL,
                       event->button, event->time);
    }
    return (FALSE);
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
    enum
    {
        EMPTY, TEXT, NUMERIC
    };

    PangoFontDescription *font_desc = NULL;
    GtkStyleContext *style_context = gtk_widget_get_style_context(GTK_WIDGET(sheet));

    gtk_style_context_get (style_context, GTK_STATE_FLAG_NORMAL,
                       GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
    metrics = pango_context_get_metrics(context, font_desc, 
	pango_context_get_language(context));
    char_width = pango_font_metrics_get_approximate_char_width(metrics);
    pango_font_metrics_unref(metrics);

    cell_width = gtk_sheet_get_column_width(sheet, sheet->active_cell.col);
    space= (double)cell_width/(double)char_width;

    intspace=MIN(space, DEFAULT_SPACE);

    format=EMPTY;

    if (strlen(text) != 0)
    {
        for (ipos=0; ipos<strlen(text); ipos++)
        {
            switch (nchar=text[ipos])
            {
                case '.':
                case ' ':
                case ',':
                case '-':
                case '+':
                case 'd':
                case 'D':
                case 'E':
                case 'e':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    nonzero=TRUE;
                    break;
                case '0':
                    break;
                default:
                    format=TEXT;
            }
            if (format != EMPTY) break;
        }
        val=atof(text);
        if (format!=EMPTY || (val==0. && nonzero))
            format = TEXT;
        else
            format = NUMERIC;
    }

    switch (format)
    {
        case TEXT:
        case EMPTY:
            strcpy(label, text);
            return;

        case NUMERIC:
            val=atof(text);
            *justification = GTK_JUSTIFY_RIGHT;
    }

    auxval= val < 0 ? -val : val;

    while (auxval<1 && auxval != 0.)
    {
        auxval=auxval*10.;
        digspace+=1;
    }

    if (digspace+DEFAULT_PRECISION+1>intspace || digspace > DEFAULT_PRECISION)
    {
        sprintf (label, "%*.*E", intspace, DEFAULT_PRECISION, val);
    }
    else
    {
        intspace=MIN(intspace, strlen(text)-digspace-1);
        sprintf (label, "%*.*f", intspace, DEFAULT_PRECISION, val);
        if (strlen(label) > space)
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
    gchar *text;

    sheet=GTK_SHEET(widget);

    gtk_sheet_get_attributes(sheet, sheet->active_cell.row,
                             sheet->active_cell.col,
                             &attr); 

    justification = attr.justification; 

    text = gtk_sheet_get_entry_text(sheet);

    format_text(sheet, text, &justification, label);

    gtk_sheet_set_cell(sheet, sheet->active_cell.row,
                       sheet->active_cell.col,
                       justification, label); 

    g_free(text);
}

gint 
    clipboard_handler(GtkWidget *widget, GdkEventKey *key)
{
    GtkSheet *sheet;

    sheet = GTK_SHEET(widget);

    if (key->state & GDK_CONTROL_MASK 
		|| key->keyval == GDK_KEY_Control_L 
		|| key->keyval == GDK_KEY_Control_R)
    {
        if ((key->keyval=='c' || key->keyval == 'C') 
			&& sheet->state != GTK_STATE_NORMAL)
        {
            if (gtk_sheet_in_clip(sheet)) gtk_sheet_unclip_range(sheet);
            gtk_sheet_clip_range(sheet, &sheet->range);
/*            gtk_sheet_unselect_range(sheet);
*/
        }
        if (key->keyval=='x' || key->keyval == 'X')
            gtk_sheet_unclip_range(sheet);
    }

    return (FALSE);
}

void 
    sheet_entry_changed_handler(GtkWidget *widget, gpointer data)
{
    char *text; 
    GtkSheet *sheet;
    gint cur_page;

    cur_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    sheet = GTK_SHEET(sheets[cur_page]);

    if (!gtk_widget_has_focus(gtk_sheet_get_entry_widget(sheet))) return;

    if ((text = gtk_sheet_get_entry_text(sheet)))
    {
        gtk_entry_set_text(GTK_ENTRY(entry), text);
        g_free(text);
    }
}

void 
    resize_handler(GtkWidget *widget, GtkSheetRange *old_range, 
                   GtkSheetRange *new_range, 
                   gpointer data)
{
    printf("OLD SELECTION: %d %d %d %d\n",old_range->row0, old_range->col0,
           old_range->rowi, old_range->coli);
    printf("NEW SELECTION: %d %d %d %d\n",new_range->row0, new_range->col0,
           new_range->rowi, new_range->coli);
}

void 
    move_handler(GtkWidget *widget, GtkSheetRange *old_range, 
                 GtkSheetRange *new_range, 
                 gpointer data)
{
    printf("OLD SELECTION: %d %d %d %d\n",old_range->row0, old_range->col0,
           old_range->rowi, old_range->coli);
    printf("NEW SELECTION: %d %d %d %d\n",new_range->row0, new_range->col0,
           new_range->rowi, new_range->coli);
}

gboolean
    change_entry(GtkWidget *widget, 
                 gint row, gint col, gint *new_row, gint *new_col,
                 gpointer data)
{
    gboolean changed = FALSE;
    GtkSheet *sheet = GTK_SHEET(widget);

    printf("change_entry: %d %d -> %d %d\n", row, col, *new_row, *new_col);

    if (*new_col == 1 && (col != 1 || sheet->state != GTK_STATE_NORMAL))
    {
	printf("change_entry: GtkEntry\n");
        gtk_sheet_change_entry(sheet, gtk_entry_get_type());
        changed = TRUE;
    }

    if (*new_col == 2 && (col != 2 || sheet->state != GTK_STATE_NORMAL))
    {
        GtkSpinButton *spin_button;
        GtkAdjustment *adjustment;
        gdouble value = 0.0;
        gchar *text;

	printf("change_entry: GtkSpinButton\n");
        gtk_sheet_change_entry(sheet, gtk_spin_button_get_type ());
        changed = TRUE;

        text = gtk_sheet_cell_get_text(sheet, *new_row, *new_col);
        if (text) value = atof(text);

        adjustment = (GtkAdjustment *) gtk_adjustment_new(value, 0.0, 100.0, 1.0, 10.0, 0.0);

        spin_button = (GtkSpinButton *) gtk_sheet_get_entry(sheet);
        gtk_spin_button_configure(spin_button, adjustment, 0.0, 3);
    }

    if (*new_col == 3 && (col != 3 || sheet->state != GTK_STATE_NORMAL))
    {
	printf("change_entry: GtkTextView\n");
        gtk_sheet_change_entry(sheet, GTK_TYPE_TEXT_VIEW);
        changed = TRUE;
    }

    if (*new_col == 4 && (col != 4 || sheet->state != GTK_STATE_NORMAL))
    {
	printf("change_entry: GtkItemEntry\n");
        gtk_sheet_change_entry(sheet, GTK_TYPE_DATA_ENTRY);
        changed = TRUE;
    }

    if (*new_col >= 5 && (col < 5 || sheet->state != GTK_STATE_NORMAL))
    {
	printf("change_entry: GtkDataTextView\n");
        gtk_sheet_change_entry(sheet, gtk_data_text_view_get_type());
        changed = TRUE;
    }

    if (changed)
    {
        /* Beware: you need to reconnect the "changed" signal
           after every call to gtk_sheet_change_entry()! */

        gtk_sheet_entry_signal_connect_changed(sheet, 
	    G_CALLBACK(sheet_entry_changed_handler));
    }

    return (TRUE);
}


void alarm_change(GtkWidget *widget, gint row, gint col, 
                  gpointer data)
{
    printf("CHANGE CELL: %d %d\n",row,col);
}

gboolean alarm_activate(GtkWidget *widget, gint row, gint col, 
                        gpointer data)
{
    GtkSheetRange range;

    printf("ACTIVATE CELL: %d %d\n",row,col);

    range.row0 = range.rowi = row;
    range.col0 = range.coli = col;
/*
 gtk_sheet_range_set_justification(GTK_SHEET(widget), &range, GTK_JUSTIFY_LEFT);
*/
    return (TRUE);
}

gboolean alarm_deactivate(GtkWidget *widget, gint row, gint col, 
                          gpointer data)
{
    GtkSheetRange range;
    gchar *text;
    GtkSheet *sheet = GTK_SHEET(widget);

    printf("DEACTIVATE CELL: %d %d\n",row,col);

    range.row0 = range.rowi = row;
    range.col0 = range.coli = col;
/*
    gtk_sheet_range_set_justification(GTK_SHEET(widget), &range, GTK_JUSTIFY_RIGHT);
*/
    text = g_strdup(gtk_sheet_cell_get_text(GTK_SHEET(widget), row, col));

    if (text && strlen(text) > 0)
    {
        gtk_sheet_set_cell_text(sheet, row, col, text);
        g_free(text);
    }

    return (TRUE);
}

gboolean alarm_traverse(GtkWidget *widget, 
                        gint row, gint col, gint *new_row, gint *new_col,
                        gpointer data)
{
    printf("TRAVERSE: %d %d %d %d\n",row,col,*new_row,*new_col);
    return (TRUE);
}

void show_child(GtkWidget *widget, gpointer data)
{
    if (!gtk_widget_get_mapped(calendar))
        gtk_sheet_attach_floating(GTK_SHEET(sheets[0]), calendar, 2, 7);
/*         gtk_sheet_put(GTK_SHEET(sheets[0]), curve, 550, 120);
*/
}

void
    build_example1(GtkWidget *widget)
{
    GtkSheet *sheet;
    GtkWidget *show_button;
    GtkSheetRange range;
    GdkRectangle area;
    GdkRGBA color;
    gchar font_name1[]="Arial 36";
    gchar font_name2[]="Arial 28";

    gchar name[10];
    gint i;

    sheet=GTK_SHEET(widget);

    gdk_rgba_parse(&color, "light yellow");
    gtk_sheet_set_background(sheet, &color);
    gdk_rgba_parse(&color, "light blue");
    gtk_sheet_set_grid(sheet, &color);

    for (i=0; i<=sheet->maxcol; i++)
    {
        name[0]='A'+i;
        name[1]='\0';

        gtk_sheet_column_button_add_label(sheet, i, name);
        gtk_sheet_set_column_title(sheet, i, name);
    }

    gtk_sheet_row_button_add_label(sheet, 0, "This is\na multiline\nlabel");
    gtk_sheet_row_button_add_label(sheet, 1, "This is long label");
    gtk_sheet_row_set_tooltip_markup(sheet, 1, "This row has a <b>long label</b>.");
/*
    gtk_sheet_column_button_add_label(sheet, 7, "This is\na multiline\nlabel");
    gtk_sheet_column_button_add_label(sheet, 8, "This is long label");
*/ 
    gtk_sheet_row_button_justify(sheet, 0, GTK_JUSTIFY_RIGHT);

    range.row0=1;
    range.rowi=2;
    range.col0=1;
    range.coli=3;

    gtk_sheet_clip_range(sheet, &range);

    PangoFontDescription *font_desc;

    font_desc = pango_font_description_from_string (font_name2);
    gtk_sheet_range_set_font(sheet, &range, font_desc);
    pango_font_description_free(font_desc);

    gdk_rgba_parse(&color, "red");
    gtk_sheet_range_set_foreground(sheet, &range, &color);

    gtk_sheet_set_cell(sheet, 1,2, GTK_JUSTIFY_CENTER,
                       "Welcome to");

    range.row0=2;

    font_desc = pango_font_description_from_string (font_name1);
    gtk_sheet_range_set_font(sheet, &range, font_desc);
    pango_font_description_free(font_desc);

    gdk_rgba_parse(&color, "blue");
    gtk_sheet_range_set_foreground(sheet, &range, &color);

    gtk_sheet_set_cell(sheet, 2,2, GTK_JUSTIFY_CENTER, "GtkSheet");

    range.row0=3;
    range.rowi=3;
    range.col0=0;
    range.coli=4;
    gdk_rgba_parse(&color, "dark gray");
    gtk_sheet_range_set_background(sheet, &range, &color);
    gdk_rgba_parse(&color, "green");
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
                       "(The selection handler has not been implemented yet)");
    gtk_sheet_set_cell(sheet, 10, 1, GTK_JUSTIFY_LEFT,
                       "You can add buttons, charts, pixmaps, and other widgets");

    g_signal_connect(G_OBJECT(sheet),
                     "key_press_event",
                     (void *) clipboard_handler, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "resize_range",
                     (void *) resize_handler, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "move_range",
                     (void *) move_handler, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "changed",
                     (void *) alarm_change, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "activate",
                     (void *) alarm_activate, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "deactivate",
                     (void *) alarm_deactivate, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "traverse",
                     (void *) alarm_traverse, 
                     NULL);

    calendar=gtk_calendar_new();
    gtk_widget_show(calendar);

    GdkPixbuf *bullet_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) bullet_xpm);

    for (i=0; i<5; i++)
    {
        bullet[i] = gtk_image_new_from_pixbuf(bullet_pixbuf);
        gtk_widget_show(bullet[i]);
        gtk_sheet_get_cell_area(GTK_SHEET(sheets[0]), 4+i, 0, &area);
/*      gtk_sheet_put(GTK_SHEET(sheets[0]), bullet[i], 
                    area.x+area.width/2-8, area.y+area.height/2-8);
*/
        gtk_sheet_attach(GTK_SHEET(sheets[0]), bullet[i], 4+i, 0, GTK_EXPAND, GTK_EXPAND, 0, 0);
    }

    bullet[5] = gtk_image_new_from_pixbuf(bullet_pixbuf);
    gtk_widget_show(bullet[5]);
    gtk_sheet_get_cell_area(GTK_SHEET(sheets[0]), 10, 0, &area);
/* gtk_sheet_put(GTK_SHEET(sheets[0]), bullet[i], 
               area.x+area.width/2-8, area.y+area.height/2-8);
*/
    gtk_sheet_attach(GTK_SHEET(sheets[0]), bullet[5], 10, 0, GTK_EXPAND, GTK_EXPAND, 0, 0);

    GdkPixbuf *smile_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) smile_xpm);
    smile = gtk_image_new_from_pixbuf(smile_pixbuf);
    gtk_widget_show(smile);
    gtk_sheet_button_attach(GTK_SHEET(sheets[0]), smile, -1, 5);

    gtk_sheet_column_set_tooltip_markup(GTK_SHEET(sheets[0]), 
                                        5, "This column has a <b>Smilie</b> in the title button.");
    gtk_sheet_cell_set_tooltip_text(GTK_SHEET(sheets[0]), 
                                      1, 5, "This single cell has its own tooltip text");

    show_button=gtk_button_new_with_label("Show me a plot");
    gtk_widget_show(show_button);
    gtk_widget_set_size_request(show_button, 100,60);
    gtk_sheet_get_cell_area(GTK_SHEET(sheets[0]), 12, 2, &area);
/* gtk_sheet_put(GTK_SHEET(sheets[0]), show_button, area.x, area.y);
*/
    gtk_sheet_attach(GTK_SHEET(sheets[0]), show_button, 12, 2, GTK_FILL, GTK_FILL, 5, 5);

    g_signal_connect(G_OBJECT(show_button), "clicked",
                     (void *) show_child, 
                     NULL);
/*
 g_signal_connect(G_OBJECT(sheet),
                    "button_press_event",
                    (void *) do_popup, 
                    NULL);
*/
}


void
    build_example2(GtkWidget *widget)
{
    GtkSheet *sheet;
    GtkSheetRange range;
    GdkRGBA color;
    GtkWidget *b;

    sheet=GTK_SHEET(widget);
    gtk_sheet_set_autoscroll(sheet, TRUE);
/*
 gtk_sheet_set_justify_entry(sheet, TRUE);
*/
    gtk_sheet_set_selection_mode(sheet, GTK_SELECTION_SINGLE);

    range.row0=0;
    range.rowi=2;
    range.col0=0;
    range.coli=sheet->maxcol;
    gtk_sheet_range_set_editable(sheet, &range, FALSE);
    gdk_rgba_parse(&color, "light gray");
    gtk_sheet_range_set_background(sheet, &range, &color);
    gdk_rgba_parse(&color, "blue");
    gtk_sheet_range_set_foreground(sheet, &range, &color);
    range.row0=1;
    gdk_rgba_parse(&color, "red");
    gtk_sheet_range_set_foreground(sheet, &range, &color);
    range.row0=2;
    gdk_rgba_parse(&color, "black");
    gtk_sheet_range_set_foreground(sheet, &range, &color);

/*
 gtk_sheet_row_set_sensitivity(sheet, 0, FALSE);
 gtk_sheet_row_set_sensitivity(sheet, 1, FALSE);
 gtk_sheet_row_set_sensitivity(sheet, 2, FALSE);
*/
    gtk_sheet_set_cell(sheet, 0,2, GTK_JUSTIFY_CENTER,
                       "Click the right mouse button to display a popup");
    gtk_sheet_set_cell(sheet, 1,2, GTK_JUSTIFY_CENTER,
                       "You can connect a parser to the 'set cell' signal");
    gtk_sheet_set_cell(sheet, 2,2, GTK_JUSTIFY_CENTER,
                       "(Try typing numbers)");
    gtk_sheet_set_active_cell(sheet, 3, 0);

/*
 gtk_sheet_set_update_policy(sheet, GTK_UPDATE_CONTINUOUS, GTK_UPDATE_CONTINUOUS);
*/

    g_signal_connect(G_OBJECT(sheet),
                     "button_press_event",
                     (void *) do_popup, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "set_cell",
                     (void *) parse_numbers,
                     NULL);                   

    g_signal_connect(G_OBJECT(sheet),
                     "activate",
                     (void *) alarm_activate, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "deactivate",
                     (void *) alarm_deactivate, 
                     NULL);

    g_signal_connect(G_OBJECT(sheet),
                     "traverse",
                     (void *) alarm_traverse, 
                     NULL);

    gtk_sheet_set_row_height(GTK_SHEET(sheets[1]), 12, 60);
    b = gtk_button_new_with_label("GTK_FILL");
    gtk_sheet_attach(GTK_SHEET(sheets[1]), b, 12, 2, GTK_FILL, GTK_FILL, 5, 5);
    gtk_widget_show(b);
    b = gtk_button_new_with_label("GTK_EXPAND");
    gtk_sheet_attach(GTK_SHEET(sheets[1]), b, 12, 3, GTK_EXPAND, GTK_EXPAND, 5, 5); gtk_widget_show(b);
    b = gtk_button_new_with_label("GTK_SHRINK");
    gtk_sheet_attach(GTK_SHEET(sheets[1]), b, 12, 4, GTK_SHRINK, GTK_SHRINK, 5, 5); gtk_widget_show(b);

    range.row0 = range.col0 = range.rowi = range.coli = 4;
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER | GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER, 
	1, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    range.row0 = range.col0 = range.rowi = range.coli = 5;
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER | GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER, 
	2, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    range.row0 = range.col0 = range.rowi = range.coli = 6;
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER | GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER, 
	3, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    range.row0 = range.col0 = range.rowi = range.coli = 7;
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER | GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER, 
	4, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    range.row0 = range.col0 = range.rowi = range.coli = 8;
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER | GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER, 
	5, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);


}

void
    build_example3(GtkWidget *widget)
{
    GtkSheet *sheet;
    GtkSheetRange range;
    GdkRGBA color;

    sheet=GTK_SHEET(widget);

    gtk_sheet_show_grid(sheet, FALSE);

    range.row0=0;
    range.rowi=10;
    range.col0=0;
    range.coli=6;

    gdk_rgba_parse(&color, "orange");
    gtk_sheet_range_set_background(sheet, &range, &color);
    gdk_rgba_parse(&color, "violet");
    gtk_sheet_range_set_foreground(sheet, &range, &color);

    range.row0=1;
    gdk_rgba_parse(&color, "light blue");
    gtk_sheet_range_set_background(sheet, &range, &color);

    range.coli=0;
    gdk_rgba_parse(&color, "light green");
    gtk_sheet_range_set_background(sheet, &range, &color);

    range.row0=0; 
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_RIGHT_BORDER, 
	4, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    range.coli=0;
    range.col0=0;
    range.rowi=0;
    gdk_rgba_parse(&color, "red");
    gtk_sheet_range_set_background(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_RIGHT_BORDER|GTK_SHEET_BOTTOM_BORDER, 
	4, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    range.rowi=0;
    range.col0=1;
    range.coli=6;
    gdk_rgba_parse(&color, "dark blue");
    gtk_sheet_range_set_border_color(sheet, &range, &color);
    gtk_sheet_range_set_border(sheet, &range, 
	GTK_SHEET_BOTTOM_BORDER, 
	4, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    gtk_sheet_set_autoresize(sheet, TRUE);

    gtk_sheet_column_button_add_label(sheet, 0, "GtkCombo");
    gtk_sheet_column_button_add_label(sheet, 1, "GtkEntry");
    gtk_sheet_column_button_add_label(sheet, 2, "GtkSpinButton");
    gtk_sheet_column_button_add_label(sheet, 3, "GtkTextView");

    gtk_sheet_column_button_add_label(sheet, 4, "GtkItemEntry\nmax 10 chars");
    g_object_set(gtk_sheet_column_get(sheet, 4),"max-length", 10, NULL);

    gtk_sheet_column_button_add_label(sheet, 5, "GtkDataTextView\nmax 20 chars");
    g_object_set(gtk_sheet_column_get(sheet, 5),"max-length", 20, NULL);

    gtk_sheet_column_button_add_label(sheet, 6, "GtkDataTextView\nno limit");

    gtk_sheet_entry_signal_connect_changed(sheet,
                                           G_CALLBACK(sheet_entry_changed_handler));

    g_signal_connect(G_OBJECT(sheet),
                     "traverse",
                     (void *) change_entry, 
                     NULL);
}

/*
void
build_example4(GtkWidget *widget)
{
 GtkSheet *sheet;

 sheet=GTK_SHEET(widget);
 g_signal_connect(G_OBJECT(sheet),
                    "button_press_event",
                    (void *) do_popup, 
                    NULL);
}
*/

void
    set_cell(GtkWidget *widget, gchar *insert, gint text_legth, gint position, gpointer data)
{
    char *text;
    GtkSheet *sheet = GTK_SHEET(widget);

    if ((text = gtk_sheet_get_entry_text(sheet)))
    {
        gtk_entry_set_text(GTK_ENTRY(entry), text);
        g_free(text);
    }
} 

void
    entry_changed_handler(GtkWidget *widget, gpointer data)
{
    const char *text;
    GtkSheet *sheet;
    gint cur_page;

    if (!gtk_widget_has_focus(widget)) return;

    cur_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    sheet = GTK_SHEET(sheets[cur_page]);

    if ((text = gtk_entry_get_text(GTK_ENTRY(entry))))
    {
        gtk_sheet_set_entry_text(sheet, text);
    }
}

void 
    activate_sheet_entry(GtkWidget *widget, gpointer data)
{
    GtkSheet *sheet;
    gint cur_page, row, col;

    cur_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    sheet=GTK_SHEET(sheets[cur_page]);
    row=sheet->active_cell.row; col=sheet->active_cell.col;

    gtk_sheet_set_cell_text(sheet, row, col, gtk_sheet_get_entry_text(sheet));
}

void 
    justify_left(GtkWidget *widget)
{
    GtkSheet *current;
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(left_button),
                                 GTK_STATE_ACTIVE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_button),
                                 GTK_STATE_NORMAL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(center_button),
                                 GTK_STATE_NORMAL);

    gtk_sheet_range_set_justification(current, &current->range,
                                      GTK_JUSTIFY_LEFT);
}

void 
    justify_center(GtkWidget *widget) 
{
    GtkSheet *current;
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(center_button), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_button), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(left_button), FALSE);

    gtk_sheet_range_set_justification(current, &current->range,
                                      GTK_JUSTIFY_CENTER);
}

void 
    justify_right(GtkWidget *widget) 
{
    GtkSheet *current;
    gint cur_page;

    cur_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_button), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(left_button), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(center_button), FALSE);

    gtk_sheet_range_set_justification(current, &current->range,
                                      GTK_JUSTIFY_RIGHT);
}

gint
    activate_sheet_cell(GtkWidget *widget, gint row, gint column, gpointer data) 
{
    GtkSheet *sheet;
    GtkWidget *sheet_entry;
    char cell[100];
    const char *text;
    GtkSheetCellAttr attributes;

    sheet=GTK_SHEET(widget);
    sheet_entry = gtk_sheet_get_entry(sheet);

    if (gtk_sheet_get_column_title(GTK_SHEET(widget), column))
        sprintf(cell,"  %s:%d  ", gtk_sheet_get_column_title(GTK_SHEET(widget), column), row);
    else
        sprintf(cell, " ROW: %d COLUMN: %d ", row, column);

    gtk_label_set_text(GTK_LABEL(location), cell);

    if (GTK_IS_ENTRY(sheet_entry) || GTK_IS_DATA_ENTRY(sheet_entry))
    {
        gtk_entry_set_max_length(GTK_ENTRY(entry),
	    gtk_entry_get_max_length(GTK_ENTRY(sheet_entry)));
    }

    if ((text = gtk_sheet_get_entry_text(sheet)))
    {
        gtk_entry_set_text(GTK_ENTRY(entry), text);
    }

    gtk_sheet_get_attributes(sheet,sheet->active_cell.row,
                             sheet->active_cell.col, &attributes);

    gtk_editable_set_editable(GTK_EDITABLE(entry), attributes.is_editable);

    switch (attributes.justification)
    {
        case GTK_JUSTIFY_LEFT:
            justify_left(NULL);
            break;
        case GTK_JUSTIFY_CENTER:
            justify_center(NULL);
            break;
        case GTK_JUSTIFY_RIGHT:
            justify_right(NULL);
            break;
        default:
            justify_left(NULL);
            break;
    }

    return (TRUE);
}

void 
    change_border (GtkWidget *widget, gint border)
{
    GtkSheet *current;
    gint cur_page;
    GtkSheetRange range, auxrange;
    gint border_mask, border_width=3;
    gint auxcol, auxrow;
    gint i,j;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    range=current->range;

    gtk_sheet_range_set_border(current, &range, 
	0, 
	0, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);

    switch (border)
    {
        case 0:
            break;
        case 1:
            border_mask = GTK_SHEET_TOP_BORDER;
            range.rowi = range.row0;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 2:
            border_mask = GTK_SHEET_BOTTOM_BORDER;
            range.row0 = range.rowi;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 3:
            border_mask = GTK_SHEET_RIGHT_BORDER;
            range.col0 = range.coli;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 4:
            border_mask = GTK_SHEET_LEFT_BORDER;
            range.coli = range.col0;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 5:
            if (range.col0 == range.coli)
            {
                border_mask = GTK_SHEET_LEFT_BORDER | GTK_SHEET_RIGHT_BORDER;
                gtk_sheet_range_set_border(current, &range, 
		    border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
                break;
            }
            border_mask = GTK_SHEET_LEFT_BORDER;
            auxcol=range.coli;
            range.coli = range.col0;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            border_mask = GTK_SHEET_RIGHT_BORDER;
            range.col0 = range.coli=auxcol; 
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 6:
            if (range.row0 == range.rowi)
            {
                border_mask = GTK_SHEET_TOP_BORDER | GTK_SHEET_BOTTOM_BORDER;
                gtk_sheet_range_set_border(current, &range, 
		    border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
                break;
            }
            border_mask = GTK_SHEET_TOP_BORDER;
            auxrow=range.rowi;
            range.rowi = range.row0;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            border_mask = GTK_SHEET_BOTTOM_BORDER;
            range.row0=range.rowi=auxrow; 
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 7:
            border_mask = GTK_SHEET_RIGHT_BORDER | GTK_SHEET_LEFT_BORDER;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 8:
            border_mask = GTK_SHEET_BOTTOM_BORDER | GTK_SHEET_TOP_BORDER;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;
        case 9:
            gtk_sheet_range_set_border(current, &range, 
		15, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            for (i=range.row0; i<=range.rowi; i++)
                for (j=range.col0; j<=range.coli; j++)
                {
                    border_mask = 15;
                    auxrange.row0 = i;
                    auxrange.rowi = i;
                    auxrange.col0 = j;
                    auxrange.coli = j;
                    if (i == range.rowi) border_mask ^= GTK_SHEET_BOTTOM_BORDER;
                    if (i == range.row0) border_mask ^= GTK_SHEET_TOP_BORDER;
                    if (j == range.coli) border_mask ^= GTK_SHEET_RIGHT_BORDER;
                    if (j == range.col0) border_mask ^= GTK_SHEET_LEFT_BORDER;
                    if (border_mask != 15)
                        gtk_sheet_range_set_border(current, &auxrange, 
			    border_mask, border_width, 
			    CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
                }

            break;
        case 10:
            for (i=range.row0; i<=range.rowi; i++)
                for (j=range.col0; j<=range.coli; j++)
                {
                    border_mask = 0;
                    auxrange.row0 = i;
                    auxrange.rowi = i;
                    auxrange.col0 = j;
                    auxrange.coli = j;
                    if (i == range.rowi) border_mask |= GTK_SHEET_BOTTOM_BORDER;
                    if (i == range.row0) border_mask |= GTK_SHEET_TOP_BORDER;
                    if (j == range.coli) border_mask |= GTK_SHEET_RIGHT_BORDER;
                    if (j == range.col0) border_mask |= GTK_SHEET_LEFT_BORDER;
                    if (border_mask != 0)
                        gtk_sheet_range_set_border(current, &auxrange, 
			    border_mask, border_width, 
			    CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
                }

            break;
        case 11:
            border_mask = 15;
            gtk_sheet_range_set_border(current, &range, 
		border_mask, border_width, CAIRO_LINE_CAP_BUTT, CAIRO_LINE_JOIN_MITER);
            break;


    }

}


void 
    change_fg(GtkWidget *widget, gint i, GdkRGBA *color)
{
    GtkSheet *current;
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_range_set_foreground(current, &current->range, color);

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(window));
    gdk_cairo_set_source_rgba (cr, color);
    cairo_rectangle(cr, 5,20,16,4);
    cairo_fill(cr);
    cairo_destroy(cr);

    gtk_widget_queue_draw(fg_pixmap); 
}

void 
    change_bg(GtkWidget *widget, gint i, GdkRGBA *color)
{
    GtkSheet *current;
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_range_set_background(current, &current->range, color);

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(window));
    gdk_cairo_set_source_rgba (cr, color);
    cairo_rectangle(cr, 4,20,18,4);
    cairo_fill(cr);
    gtk_widget_draw(bg_pixmap, cr); 
    cairo_destroy(cr);
}

void 
    do_hide_row_titles(GtkWidget *widget) 
{
    GtkSheet *current;
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_hide_row_titles(current);
}

void 
    do_hide_column_titles(GtkWidget *widget) 
{
    GtkSheet *current; 
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_hide_column_titles(current);

}

void 
    do_show_row_titles(GtkWidget *widget) 
{
    GtkSheet *current; 
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_show_row_titles(current);
}

void 
    do_show_column_titles(GtkWidget *widget) 
{
    GtkSheet *current; 
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_show_column_titles(current);

}

//30.07.17/fp - font_combo is obsolete
/*
void
    new_font(GtkFontCombo *font_combo, gpointer data)
{
    GtkSheet *current; 
    gint cur_page;

    cur_page=gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    current=GTK_SHEET(sheets[cur_page]);

    gtk_sheet_range_set_font(current, &current->range, gtk_font_combo_get_font_description(font_combo));
}
*/

int main(int argc, char *argv[]) 
{
    GtkWidget *label; 
    GtkRequisition minimum_size, natural_size; 
    GtkWidget *hide_row_titles;
    GtkWidget *hide_column_titles; 
    GtkWidget *show_row_titles; 
    GtkWidget *show_column_titles;
    GtkWidget *font_combo;
    GtkWidget *toggle_combo;
    gint i;

    char *title[]= {"Example 1",
        "Example 2", 
        "Example 3", 
        "Example 4"};
    char *folder[]= {"Folder 1",
        "Folder 2", 
        "Folder 3", 
        "Folder 4"};

    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "GtkSheet Demo");
    gtk_widget_set_size_request(GTK_WIDGET(window), 900, 600);

    g_signal_connect (GTK_WIDGET(window), "destroy", G_CALLBACK(quit), NULL);

    main_vbox= gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_container_set_border_width(GTK_CONTAINER(main_vbox),0); 
    gtk_container_add(GTK_CONTAINER(window), main_vbox);
    gtk_widget_show(main_vbox);

    show_hide_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1); 
    hide_row_titles = gtk_button_new_with_label("Hide Row Titles"); 
    hide_column_titles = gtk_button_new_with_label("Hide Column Titles"); 
    show_row_titles = gtk_button_new_with_label("Show Row Titles"); 
    show_column_titles = gtk_button_new_with_label("Show Column Titles");

    gtk_box_pack_start(GTK_BOX(show_hide_box), hide_row_titles, TRUE, TRUE, 0); 
    gtk_box_pack_start(GTK_BOX(show_hide_box), hide_column_titles, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(show_hide_box), show_row_titles, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(show_hide_box), show_column_titles, TRUE, TRUE, 0);

    gtk_widget_show(hide_row_titles); 
    gtk_widget_show(hide_column_titles);
    gtk_widget_show(show_row_titles); 
    gtk_widget_show(show_column_titles);

    g_signal_connect(G_OBJECT(hide_row_titles), "clicked",
                     (void *) do_hide_row_titles, NULL);

    g_signal_connect(G_OBJECT(hide_column_titles), "clicked",
                     (void *) do_hide_column_titles, NULL);

    g_signal_connect(G_OBJECT(show_row_titles), "clicked",
                     (void *) do_show_row_titles, NULL);

    g_signal_connect(G_OBJECT(show_column_titles), "clicked",
                     (void *) do_show_column_titles, NULL);

    gtk_box_pack_start(GTK_BOX(main_vbox), show_hide_box, FALSE, TRUE, 0);
    gtk_widget_show(show_hide_box);

    toolbar=gtk_toolbar_new();

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

    //30.07.17/fp - font_combo is obsolete
    //font_combo = gtk_font_combo_new();
    //GtkToolItem *font_item = gtk_tool_item_new();
    //gtk_widget_show(GTK_WIDGET(font_item));
    //gtk_container_add(GTK_CONTAINER(font_item), GTK_WIDGET(font_combo));
    //gtk_toolbar_insert(GTK_TOOLBAR(toolbar), font_item, -1);

    //gtk_widget_set_size_request(GTK_FONT_COMBO(font_combo)->italic_button, 32, 32);
    //gtk_widget_set_size_request(GTK_FONT_COMBO(font_combo)->bold_button, 32, 32);
    //gtk_widget_show(font_combo);
    //g_signal_connect(G_OBJECT(font_combo), "changed", G_CALLBACK(new_font), NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

    left_button = gtk_toggle_button_new();
    GtkToolItem *left_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(left_item));
    gtk_container_add(GTK_CONTAINER(left_item), GTK_WIDGET(left_button));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), left_item, -1);
    gtk_widget_show(left_button);

    g_signal_connect(G_OBJECT(left_button),"released",
                     (void *) justify_left, NULL);

    center_button = gtk_toggle_button_new();
    GtkToolItem *center_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(center_item));
    gtk_container_add(GTK_CONTAINER(center_item), GTK_WIDGET(center_button));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), center_item, -1);
    gtk_widget_show(center_button);

    g_signal_connect(G_OBJECT(center_button), "released",
                     (void *) justify_center, NULL);


    right_button = gtk_toggle_button_new();
    GtkToolItem *right_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(right_item));
    gtk_container_add(GTK_CONTAINER(right_item), GTK_WIDGET(right_button));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), right_item, -1);
    gtk_widget_show(right_button);

    g_signal_connect(G_OBJECT(right_button), "released",
                     (void *) justify_right, NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);

#if 0
    bordercombo=gtk_border_combo_new();
    GtkToolItem *border_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(border_item));
    gtk_container_add(GTK_CONTAINER(border_item), GTK_WIDGET(bordercombo));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), border_item, -1);
    gtk_widget_set_size_request(GTK_COMBO_BUTTON(bordercombo)->button, 32, 32);
    gtk_widget_show(bordercombo);

    g_signal_connect(G_OBJECT(bordercombo),
                     "changed", (void *)change_border, NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);
#endif

#if 0
    fgcolorcombo=gtk_color_combo_new();
    GtkToolItem *fgcolor_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(fgcolor_item));
    gtk_container_add(GTK_CONTAINER(fgcolor_item), GTK_WIDGET(fgcolorcombo));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), fgcolor_item, -1);
    gtk_widget_show(fgcolorcombo);

    g_signal_connect(G_OBJECT(fgcolorcombo),
                     "changed", (void *)change_fg, NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);
#endif

#if 0
    bgcolorcombo=gtk_color_combo_new();
    GtkToolItem *bgcolor_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(bgcolor_item));
    gtk_container_add(GTK_CONTAINER(bgcolor_item), GTK_WIDGET(bgcolorcombo));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), bgcolor_item, -1);
    gtk_widget_show(bgcolorcombo);

    g_signal_connect(G_OBJECT(bgcolorcombo),
                     "changed", (void *)change_bg, NULL);

    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);
#endif

#if 0
    toggle_combo = gtk_toggle_combo_new(5, 5);
    GtkToolItem *toggle_item = gtk_tool_item_new();
    gtk_widget_show(GTK_WIDGET(toggle_item));
    gtk_container_add(GTK_CONTAINER(toggle_item), GTK_WIDGET(toggle_combo));
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toggle_item, -1);
    gtk_widget_set_size_request(GTK_COMBO_BUTTON(toggle_combo)->button, 32, 32);
    gtk_widget_show(toggle_combo);
#endif

    gtk_box_pack_start(GTK_BOX(main_vbox), toolbar, FALSE, TRUE, 0);
    gtk_widget_show(toolbar);

    status_box=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_container_set_border_width(GTK_CONTAINER(status_box),0);
    gtk_box_pack_start(GTK_BOX(main_vbox), status_box, FALSE, TRUE, 0);
    gtk_widget_show(status_box);

    location=gtk_label_new(""); 
    gtk_widget_get_preferred_size (GTK_WIDGET(location), 
	&minimum_size, &natural_size);

    gtk_widget_set_size_request(location, 160, natural_size.height);
    gtk_box_pack_start(GTK_BOX(status_box), location, FALSE, TRUE, 0);
    gtk_widget_show(location);

    entry=gtk_entry_new(); 
    gtk_box_pack_start(GTK_BOX(status_box), entry, TRUE, TRUE, 0); 
    gtk_widget_show(entry);

    notebook=gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
    gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);
    gtk_widget_show(notebook);

    for (i=0; i<NUM_SHEETS; i++)
    {
        sheets=(GtkWidget **)realloc(sheets, (i+1)*sizeof(GtkWidget *));

        sheets[i]=gtk_sheet_new(1000,26,title[i]);

        scrolled_windows=(GtkWidget **)realloc(scrolled_windows, (i+1)*sizeof(GtkWidget *));
        scrolled_windows[i]=gtk_scrolled_window_new(NULL, NULL);

        gtk_container_add(GTK_CONTAINER(scrolled_windows[i]), sheets[i]);
        gtk_widget_show(sheets[i]);

        gtk_widget_show(scrolled_windows[i]);

        label=gtk_label_new(folder[i]);
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), GTK_WIDGET(scrolled_windows[i]),
                                 label);

        gtk_sheet_entry_signal_connect_changed(GTK_SHEET(sheets[i]),
                                               G_CALLBACK(sheet_entry_changed_handler));

        g_signal_connect(G_OBJECT(sheets[i]),
                         "activate", (void *)activate_sheet_cell,
                         NULL);
    }


    g_signal_connect(G_OBJECT(entry),
                     "changed", (void *)entry_changed_handler, NULL);

    g_signal_connect(G_OBJECT(entry),
                     "activate", (void *)activate_sheet_entry,
                     NULL);


    build_example1(sheets[0]); 
    build_example2(sheets[1]);
    build_example3(sheets[2]);

    GdkPixbuf *lj_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) left_just);
    tpixmap = gtk_image_new_from_pixbuf(lj_pixbuf);
    gtk_container_add(GTK_CONTAINER(left_button), tpixmap);
    gtk_widget_show(tpixmap);

    GdkPixbuf *cj_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) center_just);
    tpixmap = gtk_image_new_from_pixbuf(cj_pixbuf);
    gtk_container_add(GTK_CONTAINER(center_button), tpixmap);
    gtk_widget_show(tpixmap);

    GdkPixbuf *rj_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) right_just);
    tpixmap = gtk_image_new_from_pixbuf(rj_pixbuf);
    gtk_container_add(GTK_CONTAINER(right_button), tpixmap);
    gtk_widget_show(tpixmap);

    GdkPixbuf *paint_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) paint);
    bg_pixmap = gtk_image_new_from_pixbuf(paint_pixbuf);
#if 0
    gtk_container_add(GTK_CONTAINER(GTK_COMBO_BUTTON(bgcolorcombo)->button), bg_pixmap);
#endif
    gtk_widget_show(bg_pixmap);

    GdkPixbuf *font_pixbuf = gdk_pixbuf_new_from_xpm_data((const char **) font);
    fg_pixmap = gtk_image_new_from_pixbuf(font_pixbuf);
#if 0
    gtk_container_add(GTK_CONTAINER(GTK_COMBO_BUTTON(fgcolorcombo)->button),
                      fg_pixmap);
#endif
    gtk_widget_show(fg_pixmap);

    gtk_widget_show(window);
    gtk_main();

    return(0);
}


