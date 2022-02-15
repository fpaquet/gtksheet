/* GtkSheetColumn widget for Gtk+.
 * Copyright 2011  Fredy Paquet <fredy@opag.ch>
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

/**
 * SECTION: gtksheetcolumn
 * @short_description: A spreadsheet column widget for #GtkSheet
 *
 * The #GtkSheetColumn provides properties for sheet columns. 
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <pango/pango.h>

#define __GTKSHEET_H_INSIDE__

#include "gtksheet-compat.h"
#include "gtksheet.h"
#include "gtksheetcolumn.h"
#include "gtksheet-marshal.h"
#include "gtksheettypebuiltins.h"

#undef GTK_SHEET_COL_DEBUG

#ifdef DEBUG
#define GTK_SHEET_COL_DEBUG 1  /* define to activate debug output */
#endif

#ifdef GTK_SHEET_COL_DEBUG
#   define GTK_SHEET_COL_DEBUG_BUILDER   0
#   define GTK_SHEET_COL_DEBUG_DRAW  0
#   define GTK_SHEET_COL_DEBUG_PROPERTIES  0
#   define GTK_SHEET_COL_DEBUG_SIZE  0
#   define GTK_SHEET_COL_DEBUG_REALIZE  0  /* (un)realize, destroy, finalize */
#   define GTK_SHEET_COL_DEBUG_BUTTON  0   /* column button and label */

#   define GTK_SHEET_ENABLE_DEBUG_MACROS
#   undef GTK_SHEET_ENABLE_DEBUG_MACROS
#endif

#include "gtksheetdebug.h"

/* local macros */

/* beware - repeated macro definitions from gtksheet.c */

#define COLPTR(sheet, colidx) (sheet->column[colidx])

#define MIN_VIEW_COLUMN(sheet)  sheet->view.col0
#define MAX_VIEW_COLUMN(sheet)  sheet->view.coli

#define GTK_DATA_TEXT_VIEW_BUFFER_MAX_SIZE (G_MAXINT / 2)

#define SET_ACTIVE_CELL(r, c) \
    { \
        /*g_debug("%s(%d): SET_ACTIVE_CELL(%d, %d)", \
            __FUNCTION__, __LINE__, (r), (c));*/ \
        sheet->active_cell.row = (r); \
        sheet->active_cell.col = (c); \
    }

static GInitiallyUnowned *sheet_column_parent_class = NULL;

enum _GtkSheetColumnProperties
{
    PROP_0,
    PROP_SHEET_COLUMN_0,  /* dummy */
    PROP_SHEET_COLUMN_POSITION,  /* position of the column */
    PROP_SHEET_COLUMN_LABEL,  /* gtk_sheet_column_button_add_label() */
    PROP_SHEET_COLUMN_WIDTH,  /* gtk_sheet_set_column_width() */
    PROP_SHEET_COLUMN_JUSTIFICATION,  /* gtk_sheet_column_set_justification() */
    PROP_SHEET_COLUMN_ISKEY,  /* gtk_sheet_column_set_iskey() */
    PROP_SHEET_COLUMN_READONLY,  /* gtk_sheet_column_set_readonly() */
    PROP_SHEET_COLUMN_DATATYPE,  /* gtk_sheet_column_set_datatype() */
    PROP_SHEET_COLUMN_DATAFMT,  /* gtk_sheet_column_set_format() */
    PROP_SHEET_COLUMN_DESCRIPTION,  /* gtk_sheet_column_set_description() */
    PROP_SHEET_COLUMN_ENTRY_TYPE,  /* gtk_sheet_column_set_entry_type() */
    PROP_SHEET_COLUMN_VJUST,  /* gtk_sheet_column_set_vjustification() */
    PROP_SHEET_COLUMN_VISIBLE,  /* gtk_sheet_column_set_visibility() */
    PROP_SHEET_COLUMN_MAX_LENGTH,  /* max char length */
    PROP_SHEET_COLUMN_MAX_LENGTH_BYTES,  /* max byte length  */
    PROP_SHEET_COLUMN_WRAP_MODE,  /* wrap_mode */
    PROP_SHEET_COLUMN_IS_SECRET,  /* item entry visibility */
};

static gboolean
_column_button_press_handler(
    GtkWidget *widget,
    GdkEvent  *event,
    gpointer   user_data)
{
#if 0
    g_debug("%s(%d): %s %p", 
        __FUNCTION__, __LINE__,
        G_OBJECT_TYPE_NAME(widget), widget);
#endif

    GtkSheetColumn *colobj = GTK_SHEET_COLUMN(
        gtk_widget_get_parent(GTK_WIDGET(widget)));

    GtkSheet *sheet = GTK_SHEET(
        gtk_widget_get_parent(GTK_WIDGET(colobj)));

    if (colobj->col_button == widget)
    {
        /* patch window in event and propagate to sheet
           - this trick works in gtk_sheet_button_press_handler()
           which uses gdk_window_get_device_position() to process event
           coordinates.
           - for gtk_sheet_get_pixel_info() to work, we need to transform
           cordinates to the column_title_window
           */

        GdkEventButton *button_event = (GdkEventButton *) event;

        gdk_window_coords_to_parent(
            button_event->window,
            button_event->x, button_event->y,
            &button_event->x, &button_event->y);

        button_event->window = sheet->column_title_window;

        gboolean retval;
        g_signal_emit_by_name(sheet, "button-press-event", event, &retval);
    }

    return(TRUE); /* stop other handlers from being invoked */
}

/**
 * _gtk_sheet_column_button_add_label_internal:
 * @sheet: a #GtkSheet
 * @col: column number
 * @label: text label
 *
 * Set button label.It is used to set a column title.
 */
static void
_gtk_sheet_column_button_add_label_internal(
    GtkSheet *sheet, GtkSheetColumn *colobj, const gchar *label)
{
    g_assert(GTK_IS_SHEET_COLUMN(colobj));

#if GTK_SHEET_COL_DEBUG_BUTTON > 0
    g_debug("%s(%d): called %s %p colobj %p col_button %s %p label %s", 
        __FUNCTION__, __LINE__, 
        sheet ? G_OBJECT_TYPE_NAME(sheet) : "(null)", sheet,
        colobj,
        colobj->col_button ? G_OBJECT_TYPE_NAME(colobj->col_button) : "(null)",
        colobj->col_button,
        label);
#endif

    if (!colobj->col_button)
    {
        /* add GtkToggleButton */
        colobj->col_button = gtk_toggle_button_new_with_label (label);
        g_object_ref_sink(colobj->col_button);

#if GTK_SHEET_COL_DEBUG_BUTTON > 0
        g_debug("%s(%d): %p %s created %p %s label %s", 
            __FUNCTION__, __LINE__, 
            colobj, gtk_widget_get_name(colobj),
            colobj->col_button, gtk_widget_get_name(colobj->col_button),
            label ? label : "");
#endif

        GtkStyleContext *context = gtk_widget_get_style_context(
            GTK_WIDGET(colobj->col_button));
        gtk_style_context_add_class (context, GTK_STYLE_CLASS_TOP);

        gtk_container_add(GTK_CONTAINER(colobj), colobj->col_button);

        /* hack to disable left mouse button */
        g_signal_connect(
            G_OBJECT(colobj->col_button), 
            "button-press-event", 
            G_CALLBACK(_column_button_press_handler), NULL);
    }

    g_assert(GTK_IS_TOGGLE_BUTTON(colobj->col_button));

    gtk_button_set_label(GTK_BUTTON(colobj->col_button), label);

    /* enable ellipsize on label
       https://github.com/fpaquet/gtksheet/issues/20
       */
    GtkWidget *label_widget = gtk_bin_get_child(GTK_BIN(colobj->col_button));

    if (label_widget && GTK_IS_LABEL(label_widget))
    {
	gtk_label_set_ellipsize(GTK_LABEL(label_widget), PANGO_ELLIPSIZE_END);
    }
}

/**
 * _gtk_sheet_column_button_get_label_internal:
 * 
 * Retrieve column button label, if possible.
 * 
 * @param colobj a #GtkSheetColumn
 * 
 * @return label or NULL
 */
static const char *
_gtk_sheet_column_button_get_label_internal(GtkSheetColumn *colobj)
{
    g_return_val_if_fail(colobj != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET_COLUMN(colobj), NULL);

    gchar const *label = NULL;
    if (colobj->col_button && GTK_IS_BUTTON(colobj->col_button))
    {
        label = gtk_button_get_label(GTK_BUTTON(colobj->col_button));
    }
    return(label);
}

static void
gtk_sheet_column_set_property(GObject *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GtkSheetColumn *colobj = GTK_SHEET_COLUMN(object);
    GtkSheet *sheet = colobj->sheet;
    gint col = gtk_sheet_column_get_index(colobj);

#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
    g_debug("gtk_sheet_column_set_property: %s called (%d)", pspec->name, property_id);
#endif

    /* note: glade/gtkbuilder will set column properties before the column gets
       added to the sheet and before the sheet gets realized and mapped.
       if the column was not yet added (col < 0), we cannot use public interface functions.
       */

    switch(property_id)
    {
        case PROP_SHEET_COLUMN_POSITION:
            {
                GtkSheetColumn *swapcol;
                gint newcol = g_value_get_int(value);

                if (!sheet) return;
                if (newcol < 0 || newcol > sheet->maxcol) return;

                if (col < 0) return;
                if (newcol == col) return;

#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
                g_debug("gtk_sheet_column_set_property: swapping column %d/%d", col, newcol);
#endif

                /* method: swap */
                swapcol = sheet->column[newcol];
                sheet->column[newcol] = sheet->column[col];
                sheet->column[col] = swapcol;

                /* todo: swap cell data! */

                _gtk_sheet_reset_text_column(sheet, MIN(col, newcol));
                _gtk_sheet_recalc_left_xpixels(sheet);
            }
            break;

        case PROP_SHEET_COLUMN_LABEL:
            {
                const gchar *label = g_value_get_string(value);

                _gtk_sheet_column_button_add_label_internal(
                    sheet, colobj, label);
            }
            break;

        case PROP_SHEET_COLUMN_WIDTH:
            {
                gint width = g_value_get_int(value);

                if (width < 0) return;
                if (width < GTK_SHEET_COLUMN_MIN_WIDTH) width = GTK_SHEET_COLUMN_DEFAULT_WIDTH;

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->width = width;
                }
                else
                {
#if GTK_SHEET_COL_DEBUG_SIZE > 0
                    g_debug("gtk_sheet_column_set_property[%d]: set width %d", col, width);
#endif
                    gtk_sheet_set_column_width(sheet, col, width);
                }
            }
            break;

        case PROP_SHEET_COLUMN_JUSTIFICATION:
            {
                gint justification = g_value_get_enum(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->justification = justification;
                }
                else
                    gtk_sheet_column_set_justification(sheet, col, justification);
            }
            break;

        case PROP_SHEET_COLUMN_ISKEY:
            {
                gint is_key = g_value_get_boolean(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->is_key = is_key;
                }
                else
                    gtk_sheet_column_set_iskey(sheet, col, is_key);
            }
            break;

        case PROP_SHEET_COLUMN_READONLY:
            {
                gint is_readonly = g_value_get_boolean(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->is_readonly = is_readonly;
                }
                else
                    gtk_sheet_column_set_readonly(sheet, col, is_readonly);
            }
            break;

        case PROP_SHEET_COLUMN_DATATYPE:
            {
                const gchar *data_type = g_value_get_string(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    if (colobj->data_type) g_free(colobj->data_type);
                    colobj->data_type = g_strdup(data_type);
                }
                else
                    gtk_sheet_column_set_datatype(sheet, col, data_type);
            }
            break;

        case PROP_SHEET_COLUMN_DATAFMT:
            {
                const gchar *data_format = g_value_get_string(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    if (colobj->data_format) g_free(colobj->data_format);
                    colobj->data_format = g_strdup(data_format);
                }
                else
                    gtk_sheet_column_set_format(sheet, col, data_format);
            }
            break;

        case PROP_SHEET_COLUMN_DESCRIPTION:
            {
                const gchar *description = g_value_get_string(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    if (colobj->description) g_free(colobj->description);
                    colobj->description = g_strdup(description);
                }
                else
                    gtk_sheet_column_set_description(sheet, col, description);
            }
            break;

        case PROP_SHEET_COLUMN_ENTRY_TYPE:
            {
                GType entry_type = _gtk_sheet_entry_type_to_gtype(
                    g_value_get_enum(value));

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->entry_type = entry_type;
                }
                else
                    gtk_sheet_column_set_entry_type(sheet, col, entry_type);
            }
            break;

        case PROP_SHEET_COLUMN_VJUST:
            {
                GtkSheetVerticalJustification vjust = g_value_get_enum(value);

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->vjust = vjust;
                }
                else
                    gtk_sheet_column_set_vjustification(sheet, col, vjust);
            }
            break;

        case PROP_SHEET_COLUMN_VISIBLE:
            {
                gint visible = g_value_get_boolean(value);

#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
                g_debug("gtk_sheet_column_set_property: col %d visible %d sheet %p", 
                    col, visible, sheet);
#endif

                if ((col < 0) 
                    || !G_IS_OBJECT(sheet) 
                    || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    GTK_SHEET_COLUMN_SET_VISIBLE(colobj, visible);
                }
                else
                    gtk_sheet_column_set_visibility(sheet, col, visible);
            }
            break;

        case PROP_SHEET_COLUMN_MAX_LENGTH:
            colobj->max_length = g_value_get_int(value);
            break;

        case PROP_SHEET_COLUMN_MAX_LENGTH_BYTES:
            colobj->max_length = g_value_get_int(value);
            break;

        case PROP_SHEET_COLUMN_WRAP_MODE:
            colobj->wrap_mode = g_value_get_enum(value);
            break;

        case PROP_SHEET_COLUMN_IS_SECRET:
            colobj->is_secret = g_value_get_boolean(value);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }

    if (G_IS_OBJECT(sheet) && gtk_widget_get_realized(GTK_WIDGET(sheet))
        && !gtk_sheet_is_frozen(sheet))
    {
        _gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
}

static void
gtk_sheet_column_get_property(GObject *object,
                              guint property_id,
                              GValue *value,
                              GParamSpec *pspec)
{
    GtkSheetColumn *colobj = GTK_SHEET_COLUMN(object);

    switch(property_id)
    {
        case PROP_SHEET_COLUMN_POSITION:
            {
                GtkSheet *sheet = colobj->sheet;
                if (!sheet) return;

                gint col = gtk_sheet_column_get_index(colobj);
                if (col >= 0) g_value_set_int(value, col);
            }
            break;

        case PROP_SHEET_COLUMN_LABEL:
            g_value_set_string(value,
                _gtk_sheet_column_button_get_label_internal(colobj));
            break;

        case PROP_SHEET_COLUMN_WIDTH:
            g_value_set_int(value, colobj->width);
            break;

        case PROP_SHEET_COLUMN_JUSTIFICATION:
            g_value_set_enum(value, colobj->justification);
            break;

        case PROP_SHEET_COLUMN_ISKEY:
            g_value_set_boolean(value, colobj->is_key);
            break;

        case PROP_SHEET_COLUMN_READONLY:
            g_value_set_boolean(value, colobj->is_readonly);
            break;

        case PROP_SHEET_COLUMN_DATATYPE:
            g_value_set_string(value, colobj->data_type);
            break;

        case PROP_SHEET_COLUMN_DATAFMT:
            g_value_set_string(value, colobj->data_format);
            break;

        case PROP_SHEET_COLUMN_DESCRIPTION:
            g_value_set_string(value, colobj->description);
            break;

        case PROP_SHEET_COLUMN_ENTRY_TYPE:
            {
                GtkSheetEntryType e = _gtk_sheet_entry_type_from_gtype(colobj->entry_type);
                g_value_set_enum(value, e);
            }
            break;

        case PROP_SHEET_COLUMN_VJUST:
            g_value_set_enum(value, colobj->vjust);
            break;

        case PROP_SHEET_COLUMN_VISIBLE:
            g_value_set_boolean(value, GTK_SHEET_COLUMN_IS_VISIBLE(colobj));
            break;

        case PROP_SHEET_COLUMN_MAX_LENGTH:
            g_value_set_int(value, colobj->max_length);
            break;

        case PROP_SHEET_COLUMN_MAX_LENGTH_BYTES:
            g_value_set_int(value, colobj->max_length_bytes);
            break;

        case PROP_SHEET_COLUMN_WRAP_MODE:
            g_value_set_enum(value, colobj->wrap_mode);
            break;

        case PROP_SHEET_COLUMN_IS_SECRET:
            g_value_set_boolean(value, colobj->is_secret);
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
            break;
    }
}

static void gtk_sheet_column_class_init_properties(GObjectClass *gobject_class)
{
    GParamSpec *pspec;

    gobject_class->set_property = gtk_sheet_column_set_property;
    gobject_class->get_property = gtk_sheet_column_get_property;

    /**
     * GtkSheetColumn:position:
     *
     * The packing position of the column
     */
    pspec = g_param_spec_int("position", "Position",
                             "Packing position",
                             0, 1024, 0,
                             G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_POSITION, pspec);

    /**
     * GtkSheetColumn:label:
     *
     * Label of the column button
     */
    pspec = g_param_spec_string("label", "Column Button Label",
                                "Label of the column button",
                                "" /* default value */,
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_LABEL, pspec);

    /**
     * GtkSheetColumn:width:
     *
     * Width of the column
     */
    pspec = g_param_spec_int("width", "Width",
                             "Width of the column",
                             -1, 8192, -1,
                             G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_WIDTH, pspec);

    /**
     * GtkSheetColumn:justification:
     *
     * Justification of the column
     */
    pspec = g_param_spec_enum("justification", "Justification",
                              "Column justification (GTK_JUSTIFY_LEFT, RIGHT, CENTER)",
                              GTK_TYPE_JUSTIFICATION,
                              GTK_SHEET_COLUMN_DEFAULT_JUSTIFICATION,
                              G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_JUSTIFICATION, pspec);

    /**
     * GtkSheetColumn:iskey:
     *
     * Flag for key columns
     */
    pspec = g_param_spec_boolean("iskey", "Key column",
                                 "Wether this is a key column",
                                 FALSE,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_ISKEY, pspec);

    /**
     * GtkSheetColumn:readonly:
     *
     * Lock column contents for editing.
     */
    pspec = g_param_spec_boolean("readonly", "Readonly",
                                 "Column contents are locked for editing",
                                 FALSE,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_READONLY, pspec);

    /**
     * GtkSheetColumn:datatype:
     *
     * no functionality, a datatype hint for the application because 
     * any widget content is text
     */
    pspec = g_param_spec_string("datatype", "Data type",
                                "Data type for application use",
                                "",
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_DATATYPE, pspec);

    /**
     * GtkSheetColumn:dataformat:
     *
     * a formatting string that controls what you see when the 
     * widget doesn't contain input focus
     */
    pspec = g_param_spec_string("dataformat", "Data format",
                                "A formatting string that controls what you see when the widget doesn't contain input focus",
                                "",
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_DATAFMT, pspec);

    /**
     * GtkSheetColumn:description:
     *
     * Description of column contents
     */
    pspec = g_param_spec_string("description", "Description",
                                "Description of column contents",
                                "",
                                G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_DESCRIPTION, pspec);

    /**
     * GtkSheetColumn:entry-type:
     *
     * Column cell entry widget type
     */
    pspec = g_param_spec_enum("entry-type", "Entry Type",
                              "Supersedes sheet entry type, if not default",
                              gtk_sheet_entry_type_get_type(),
                              GTK_SHEET_ENTRY_TYPE_DEFAULT,
                              G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_ENTRY_TYPE, pspec);

    /**
     * GtkSheetColumn:vjust:
     *
     * Column vertical cell text justification
     */
    pspec = g_param_spec_enum("vjust", "Vertical justification",
                              "Supersedes sheet vertical cell text justification",
                              gtk_sheet_vertical_justification_get_type(),
                              GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT,
                              G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_VJUST, pspec);

    /**
     * GtkSheetColumn:visible:
     *
     * Visible property for columns
     */
    pspec = g_param_spec_boolean("visible", "Column is visible",
                                 "Wether the column is visible",
                                 FALSE,
                                 G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_VISIBLE, pspec);

    /**
     * GtkSheetColumn:max-length:
     *
     * Maximum number orf characters in this column, Zero if no 
    *  maximum.
    *
    *  This property is passed to the sheet entry editor. It is
    *  supported for  the following editors: #GtkItemEntry,
    *  #GtkEntry, #GtkDataTextView
    *
    * Since: 3.0.6 
     */
    pspec = g_param_spec_int("max-length", "Maximum char length",
                             "Maximum number orf characters in this column, Zero if no maximum.",
                             0, GTK_DATA_TEXT_VIEW_BUFFER_MAX_SIZE, 0,
                             G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
                                    PROP_SHEET_COLUMN_MAX_LENGTH, pspec);

    /**
     * GtkSheetColumn:max-length-bytes:
     *
     * Set the maximum length in bytes for the GtkDataEntry. For 
     * details see #gtk_data_entry_set_max_length_bytes. 
     *
     *  This property is passed to the sheet entry editor. It is
     *  supported for  the following editors: #GtkItemEntry,
     *  #GtkDataTextView.
     *
     * Sometimes, systems cannot handle UTF-8 string length
     * correctly, to overcome this problem, you can use the maximum 
     * string length in bytes. When setting both limits, max-length 
     *  and max-length-bytes, both must be fulfilled.
     *  
     * Since: 3.0.6 
     */
    pspec = g_param_spec_int("max-length-bytes", "Maximum bytes length",
                             "The maximum number of bytes for this entry. Zero if no maximum",
                             0, GTK_DATA_TEXT_VIEW_BUFFER_MAX_SIZE, 0,
                             G_PARAM_READWRITE);
    g_object_class_install_property(
        gobject_class, PROP_SHEET_COLUMN_MAX_LENGTH_BYTES, pspec);

    /**
     * GtkSheetColumn:wrap-mode:
     *
     *  This property is passed to the sheet entry editor. It is
     *  supported for  the following editors: #GtkTextView,
     *  #GtkDataTextView.
     *
     * Since: 3.0.6 
     */
    pspec = g_param_spec_enum(
        "wrap-mode", 
        "Wrap-mode",
        "Whether to wrap lines never, at word boundaries, or at character boundaries",
        GTK_TYPE_WRAP_MODE,
        GTK_WRAP_NONE,
        G_PARAM_READWRITE);

    g_object_class_install_property(
        gobject_class, PROP_SHEET_COLUMN_WRAP_MODE, pspec);

    /**
     * GtkSheetColumn:is_secret:
     *
     *  This property is passed to the sheet entry editor. It is
     *  supported for  the following editors: #GtkDataEntry,
     *  #GtkEntry.
     *  
     * Since: 4.3.1
     */
    pspec = g_param_spec_boolean(
        "is_secret", 
        "Column contains secret data",
        "TRUE displays the invisible char instead of the actual text (password mode).",
        FALSE,
        G_PARAM_READWRITE);

    g_object_class_install_property(
        gobject_class, PROP_SHEET_COLUMN_IS_SECRET, pspec);

}

static void
gtk_sheet_column_init(GtkSheetColumn *column)
{
    column->sheet = NULL;
    column->title = NULL;
    column->width = GTK_SHEET_COLUMN_DEFAULT_WIDTH;
    column->requisition = GTK_SHEET_COLUMN_DEFAULT_WIDTH;
    column->left_xpixel = 0;
    column->max_extent_width = 0;

    column->col_button = NULL;

#if GTK_SHEET_OPTIMIZE_COLUMN_DRAW>0
    column->left_text_column = column->right_text_column = 0;
#endif
    column->is_visible = TRUE;
    column->vis_col_index = -1;

    column->justification = GTK_SHEET_COLUMN_DEFAULT_JUSTIFICATION;
    column->vjust = GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT;

    column->is_key = FALSE;
    column->is_readonly = FALSE;
    column->data_format = NULL;
    column->data_type = NULL;
    column->description = NULL;
    column->is_secret = FALSE;  /* item entry visibility */

    column->entry_type = G_TYPE_NONE;
    column->max_length = 0;
    column->max_length_bytes = 0;
    column->wrap_mode = GTK_WRAP_NONE;

    GTK_SHEET_COLUMN_SET_VISIBLE(column, TRUE);
    GTK_SHEET_COLUMN_SET_SENSITIVE(column, TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(column), TRUE);
    gtk_widget_set_has_window(GTK_WIDGET(column), FALSE);

    /* not yet attached - column index does not exist
       so we create an empty column title button, needed for Glade
       */
    _gtk_sheet_column_button_add_label_internal(NULL, column, NULL);
}

/*
 * gtk_sheet_column_finalize_handler:
 * 
 * this is the #GtkSheetColumn object class "finalize" handler
 * 
 * @param gobject the #GtkSheetColumn
 */
static void
gtk_sheet_column_finalize_handler(GObject *gobject)
{
#if GTK_SHEET_COL_DEBUG_REALIZE > 0
    g_debug("%s(%d): called %p", __FUNCTION__, __LINE__, gobject);
#endif
    GtkSheetColumn *column = GTK_SHEET_COLUMN(gobject);

    if (column->title)
    {
        g_free(column->title);
        column->title = NULL;
    }

    if (column->col_button)
    {
#if GTK_SHEET_COL_DEBUG_BUTTON > 0
        g_debug("%s(%d): destroy %p %s", 
            __FUNCTION__, __LINE__, 
            column->col_button, gtk_widget_get_name(column->col_button));
#endif
        gtk_widget_destroy(column->col_button);
        column->col_button = NULL;
    }

    if (column->data_format)
    {
        g_free(column->data_format);
        column->data_format = NULL;
    }

    if (column->description)
    {
        g_free(column->description);
        column->description = NULL;
    }

#if GTK_SHEET_COL_DEBUG_REALIZE > 0
    g_debug("%s(%d): chaining up %p", __FUNCTION__, __LINE__, gobject);
#endif
    G_OBJECT_CLASS(sheet_column_parent_class)->finalize(gobject);

#if GTK_SHEET_COL_DEBUG_REALIZE > 0
    g_debug("%s(%d): done %p", __FUNCTION__, __LINE__, gobject);
#endif
}

/**
 * _gtk_sheet_column_check_windows:
 * 
 * The sheet column and all widgets below must use column_title_window.
 * 
 * This is mandatory, because coordinates calculated in 
 * _gtk_sheet_column_buttons_size_allocate are relative to the 
 * column_title_window.
 * 
 * To test this, 
 * - Drag a notebook tab containing a #GtkSheet from one notebook into another notebook
 * - Add columns in Glade
 * - Cut/Paste a #GtkSheet in Glade into another toplevel window
 * 
 * @param colobj the #GtkSheetColumn
 * @param sheet  the #GtkSheet
 */
void
_gtk_sheet_column_check_windows(
    GtkSheetColumn *colobj, GtkSheet *sheet)
{
    g_return_if_fail(colobj != NULL);
    g_return_if_fail(GTK_IS_SHEET_COLUMN(colobj));
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!GTK_IS_SHEET_COLUMN(colobj)) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(colobj))) return;

    if (gtk_widget_get_window(GTK_WIDGET(colobj))
        != sheet->column_title_window)
    {
        gtk_widget_set_window(
            GTK_WIDGET(colobj), sheet->column_title_window);
    }

    if (colobj->col_button)
    {
        if (gtk_widget_get_window(colobj->col_button)
            != sheet->column_title_window)
        {
            gtk_widget_set_window(
                GTK_WIDGET(colobj->col_button),
                sheet->column_title_window);

            /* is there a child attached ?

               */
            GtkWidget *child = gtk_bin_get_child(
                GTK_BIN(colobj->col_button));

            if (child && GTK_IS_WIDGET(child))
            {
                if (gtk_widget_get_window(child)
                    != sheet->column_title_window)
                {
                    gtk_widget_set_window(
                        GTK_WIDGET(child),
                        sheet->column_title_window);
                }
            }
        }
    }
}

/**
 * _gtk_sheet_column_realize:
 * 
 * realize sheet column
 * 
 * @param widget the #GtkSheetColumn
 */
void
_gtk_sheet_column_realize(GtkSheetColumn *colobj, GtkSheet *sheet)
{
    g_return_if_fail(colobj != NULL);
    g_return_if_fail(GTK_IS_SHEET_COLUMN(colobj));
    g_return_if_fail(GTK_IS_SHEET(sheet));

#if 0
    g_debug("%s(%d) called %s %p", 
        __FUNCTION__, __LINE__, G_OBJECT_TYPE_NAME(colobj), colobj);
#endif

    if (gtk_widget_get_realized(GTK_WIDGET(colobj))) return;

    if (sheet->column_titles_visible)
    {
        if (!gtk_widget_get_parent(GTK_WIDGET(colobj)))
        {
            DEBUG_WIDGET_SET_PARENT(colobj, sheet);
            gtk_widget_set_parent(
                GTK_WIDGET(colobj), GTK_WIDGET(sheet));
        }

        DEBUG_WIDGET_SET_PARENT_WIN(
            colobj, sheet->column_title_window);
        gtk_widget_set_parent_window(
            GTK_WIDGET(colobj), sheet->column_title_window);
        
        gtk_widget_set_realized(GTK_WIDGET(colobj), TRUE);

        if (colobj->col_button)
        {
            DEBUG_WIDGET_SET_PARENT_WIN(
                colobj->col_button, sheet->column_title_window);
            gtk_widget_set_parent_window(
                colobj->col_button, sheet->column_title_window);

            gtk_widget_realize(colobj->col_button);
        }
    }
}

/**
 * _gtk_sheet_column_unrealize:
 * 
 * unrealize sheet column
 * 
 * @param widget the #GtkSheetColumn
 */
static void
_gtk_sheet_column_unrealize(GtkWidget *widget)
{
    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET_COLUMN(widget));

    GtkSheetColumn *colobj = GTK_SHEET_COLUMN(widget);

#if GTK_SHEET_COL_DEBUG_REALIZE > 0
    g_debug(
        "%s(%d): called %s %p", 
        __FUNCTION__, __LINE__, G_OBJECT_TYPE_NAME(colobj), colobj);
#endif

    if (!gtk_widget_get_realized(GTK_WIDGET(colobj))) return;

    if (colobj->col_button)
    {
#if GTK_SHEET_COL_DEBUG_REALIZE > 0
        g_debug(
            "%s(%d): unrealize col_button %p %s", 
            __FUNCTION__, __LINE__, 
            colobj->col_button, gtk_widget_get_name(colobj->col_button));
#endif
        gtk_widget_hide(colobj->col_button);
        gtk_widget_unmap(colobj->col_button);
        gtk_widget_unrealize(colobj->col_button);
    }

    /* this function fixes a crash in Gtk 3.22.16
       which shows up in gdb with G_SLICE=always-malloc.
       Even if we set gtk_widget_set_has_window(colobj, FALSE)
       Gtk tries to destroy the non existing colobj window:
     
       #0 in g_type_check_instance_is_fundamentally_a
       #1 in g_object_unref (_object=0x1a5e120) at gobject.c:3082
       #2 in gtk_widget_real_unrealize (widget=0x1a8efc0) at gtkwidget.c:12409
       #6 in gtk_widget_unrealize (widget=0x1a8efc0) at gtkwidget.c:5520
       #9 in gtk_sheet_unrealize_handler (widget=0x1a62940) at gtksheet.c:6954
       */

    gtk_widget_set_realized(GTK_WIDGET(colobj), FALSE);

#if GTK_SHEET_COL_DEBUG_REALIZE > 0
    g_debug(
        "%s(%d): done %p %s parent %p", 
        __FUNCTION__, __LINE__, 
        colobj, gtk_widget_get_name(colobj), gtk_widget_get_parent(colobj));
#endif
}

static void
gtk_sheet_column_class_init(GtkSheetColumnClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    sheet_column_parent_class = g_type_class_peek_parent(klass);

    gobject_class->finalize = gtk_sheet_column_finalize_handler;
    widget_class->unrealize = _gtk_sheet_column_unrealize;

    gtk_sheet_column_class_init_properties(gobject_class);

    gtk_widget_class_set_css_name (widget_class, "button");
}

static void
gtk_sheet_column_set_buildable_property(GtkBuildable  *buildable,
                                        GtkBuilder    *builder,
                                        const gchar   *name,
                                        const GValue  *value)
{
#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
    g_debug("gtk_sheet_column_set_buildable_property: %s", name);
#endif

#if 0
    if (strcmp(name, "visible") == 0)
    {
        gboolean v = g_value_get_boolean(value);
#   if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
        g_debug("gtk_sheet_column_set_buildable_property: %s = %s", name,
                v ? "true" : "false");
#   endif
        GTK_SHEET_COLUMN_SET_VISIBLE(buildable, v);
    }
    else if (strcmp(name, "width-request") == 0)
    {
#   if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
        g_debug("gtk_sheet_column_set_buildable_property: width-request = %d",
                GTK_SHEET_COLUMN(buildable)->width);
#   endif
        GTK_SHEET_COLUMN(buildable)->width = g_value_get_int(value);
    }
    else
#endif
    g_object_set_property(G_OBJECT(buildable), name, value);
}


static void
gtk_sheet_column_buildable_init(GtkBuildableIface *iface)
{
#if GTK_SHEET_COL_DEBUG_BUILDER > 0
    g_debug("gtk_sheet_column_buildable_init");
#endif
    iface->set_buildable_property = gtk_sheet_column_set_buildable_property;
}



/* Type initialisation */

GType
gtk_sheet_column_get_type(void)
{
    static GType sheet_column_type = 0;

    if (!sheet_column_type)
    {
        static const GTypeInfo sheet_column_info =
        {
            sizeof(GtkSheetColumnClass),
            NULL,
            NULL,
            (GClassInitFunc)gtk_sheet_column_class_init,
            NULL,
            NULL,
            sizeof(GtkSheetColumn),
            0,
            (GInstanceInitFunc)gtk_sheet_column_init,
            NULL,
        };

        static const GInterfaceInfo interface_info = {
            (GInterfaceInitFunc)gtk_sheet_column_buildable_init,
            (GInterfaceFinalizeFunc)NULL,
            (gpointer)NULL
        };

        sheet_column_type = g_type_register_static(
            gtk_bin_get_type(),
            "GtkSheetColumn",
            &sheet_column_info,
            0);

        g_type_add_interface_static(
            sheet_column_type,
            GTK_TYPE_BUILDABLE,
            &interface_info);
    }
    return (sheet_column_type);
}

/**
 * gtk_sheet_column_get: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Get a #GtkSheetColumn
 *  
 * Returns:	(transfer none): the requested #GtkSheetColumn or 
 * NULL 
 */
GtkSheetColumn *gtk_sheet_column_get(GtkSheet *sheet, gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return (NULL);

    return (COLPTR(sheet, col));
}


/**
 * gtk_sheet_column_get_index:
 * @colobj: #GtkSheetColumn to find
 * 
 * find index of @colobj in GtkSheet
 * 
 * Returns: column index or -1
 */
gint
gtk_sheet_column_get_index(GtkSheetColumn *colobj)
{
    GtkSheet *sheet = colobj->sheet;
    int i;

    if (!sheet) return (-1);

    for (i = 0; i <= sheet->maxcol; i++)
    {
        if (COLPTR(sheet, i) == colobj) return (i);
    }
    return (-1);
}


/**
 * gtk_sheet_column_rightmost_visible: 
 * @sheet:  the sheet
 * 
 * rightmost visible column index 
 *  
 * Returns: index of rightmost visible column or -1 (if none)
 */
static inline gint
gtk_sheet_column_rightmost_visible(GtkSheet *sheet)
{
    gint i, res = -1;

    for (i = 0; i <= sheet->maxcol; i++)
    {
        if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i))) res = i;
    }

    return (res);
}

/**
 * _gtk_sheet_column_left_xpixel:
 * @sheet:  the #GtkSheet
 * @col:    column index 
 *  
 * gives the left pixel of the given column in context of the sheet's hoffset
 * 
 * Returns: pixel offset
 */
gint
_gtk_sheet_column_left_xpixel(GtkSheet *sheet, gint col)
{
    if (col < 0 || col > sheet->maxcol) return (sheet->hoffset);
    return (sheet->hoffset + COLPTR(sheet, col)->left_xpixel);
}

/**
 * _gtk_sheet_column_right_xpixel:
 * @sheet:  the #GtkSheet
 * @col:    column index 
 *  
 * gives the right pixel of the given column in context of the 
 * sheet's hoffset 
 * 
 * Returns: pixel offset
 */
gint
_gtk_sheet_column_right_xpixel(GtkSheet *sheet, gint col)
{
    gint xpixel = _gtk_sheet_column_left_xpixel(sheet, col);
    if (0 <= col && col <= sheet->maxcol) xpixel += COLPTR(sheet, col)->width;
    return (xpixel);
}






/**
 * _gtk_sheet_column_size_request:
 * @sheet:  the #GtkSheet
 * @col: column index
 * @button_requisition: the requisition width
 *  
 * size request handler for all sheet buttons
 */
void
_gtk_sheet_column_size_request(GtkSheet *sheet,
                               gint col,
                               guint *requisition)
{
    GtkRequisition button_requisition;

    *requisition = 0;

    g_assert(0 <= col && col <= sheet->maxcol);
    GtkSheetColumn *colobj = COLPTR(sheet, col);

    if (colobj->col_button)
    {
        gtk_widget_get_preferred_size(
            colobj->col_button, &button_requisition, NULL);

        *requisition = button_requisition.width;
    }

    GList *children = sheet->children;
    while (children)
    {
        GtkSheetChild *child = (GtkSheetChild *)children->data;

        if (child->attached_to_cell
            && child->col == col
            && child->row != -1
            && !child->floating
            && !child->xshrink)
        {
            GtkRequisition child_min_size, child_nat_size;

            gtk_widget_get_preferred_size (
                child->widget, &child_min_size, &child_nat_size);

            if (child_min_size.width + 2 * child->xpadding > *requisition)
                *requisition = child_min_size.width + 2 * child->xpadding;
        }
        children = children->next;
    }

    COLPTR(sheet, col)->requisition = *requisition;

#if GTK_SHEET_COL_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_column_size_request: col %d = %d",
        col, *requisition);
#endif
}

/**
 * _gtk_sheet_column_buttons_size_allocate:
 * @sheet:  the #GtkSheet 
 *  
 * column title button size allocation
 */
void
_gtk_sheet_column_buttons_size_allocate(GtkSheet *sheet)
{
    gint col, x, width;
    GdkRectangle *cta = &sheet->column_title_area;

    if (!sheet->column_titles_visible) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

#if GTK_SHEET_COL_DEBUG_SIZE > 0
    g_debug("%s(%d): called %s %p", 
        __FUNCTION__, __LINE__, G_OBJECT_TYPE_NAME(sheet), sheet);
#endif

    width = sheet->sheet_window_width;
    x = 0;

    if (sheet->row_titles_visible)
    {
        /* negative widths can result in pixman_region32_init_rect warnings */
        if (width >= sheet->row_title_area.width) 
            width -= sheet->row_title_area.width;
        x = sheet->row_title_area.width;
    }

    /* if neccessary, resize the column title window */
    if (cta->width != width || cta->x != x)
    {
        cta->width = width;
        cta->x = x;

        gdk_window_move_resize(
            sheet->column_title_window,
            cta->x, cta->y,
            cta->width, cta->height);
    }

    if (!gtk_widget_is_visible(GTK_WIDGET(sheet))) return;

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);
    _gtk_sheet_recalc_view_range(sheet);

    for (col = MIN_VIEW_COLUMN(sheet);
          col <= MAX_VIEW_COLUMN(sheet) && col <= sheet->maxcol;
          col++)
    {
        g_assert(0 <= col && col <= sheet->maxcol);
        GtkSheetColumn *colobj = COLPTR(sheet, col);

        if (!GTK_SHEET_COLUMN_IS_VISIBLE(colobj))
        {
            continue;
        }

        if (colobj->col_button)
        {
            /* pixel gap between column buttons */
#define COL_BUTTON_GAP 0

            /* it looks as if the sheet grid is misaligned
               1 pixel too far on the left side,
               except for the right side of the last column
               */
#define COL_BUTTON_GRID_SHIFT -1

            GdkRectangle allocation;
            allocation.x =  _gtk_sheet_column_left_xpixel(sheet, col) + COL_BUTTON_GRID_SHIFT + COL_BUTTON_GAP;
            allocation.y = 0;
            allocation.width = colobj->width + 1 - COL_BUTTON_GAP*2;
            allocation.height = sheet->column_title_area.height;

            if (!gtk_widget_get_realized(GTK_WIDGET(colobj)))
                _gtk_sheet_column_realize(colobj, sheet);

            gtk_widget_set_visible(colobj->col_button, TRUE);

#if 1
            gtk_widget_get_preferred_size(
                colobj->col_button, NULL, NULL);
#else
            {
                /* this will result in buttons larger than columns */
                GtkRequisition button_requisition;

                gtk_widget_get_preferred_size(
                    colobj->col_button, &button_requisition, NULL);
                
                //allocation.width = MAX(allocation.width, button_requisition.width);
                //allocation.height = MAX(allocation.height, button_requisition.height);
            }
#endif

            /* because _gtk_sheet_recalc_left_xpixels
               adds row_title_area.width to cx,
               we need to subtract it here again
            */ 
            if (sheet->row_titles_visible)
                allocation.x -= sheet->row_title_area.width;

            gtk_widget_size_allocate(colobj->col_button, &allocation);

            if (sheet)
            {
                gint minimum_height, natural_height;

                gtk_widget_get_preferred_height(
                    GTK_WIDGET(colobj->col_button),
                    &minimum_height, &natural_height);

                if (minimum_height > sheet->column_title_area.height)
                    gtk_sheet_set_column_titles_height(sheet, minimum_height);
            }

            gtk_widget_get_preferred_size(
                GTK_WIDGET(colobj), NULL, NULL);
            gtk_widget_size_allocate(
                GTK_WIDGET(colobj), &allocation);

            _gtk_sheet_column_check_windows(colobj, sheet);

            GtkWidget *parent = gtk_widget_get_parent(
                colobj->col_button);

            if (parent != GTK_WIDGET(colobj))
            {
#if 0
                if (parent)
                {
                    g_object_ref(colobj->col_button);
                    gtk_widget_unparent(colobj->col_button);
                }
#endif
                GtkWidget *child = gtk_bin_get_child(GTK_BIN(colobj));

                if (child) // remove Glade placeholder
                {
                    gtk_widget_destroy(child);
                }

                //DEBUG_WIDGET_SET_PARENT(colobj->col_button, sheet);
                //gtk_widget_set_parent(colobj->col_button, GTK_WIDGET(sheet));
                gtk_container_add(
                    GTK_CONTAINER(colobj), 
                    GTK_WIDGET(colobj->col_button));
            }

            /* clip on column_title_window */
            if (gtk_widget_get_mapped(colobj->col_button))
            {
                if (allocation.x < 0) allocation.x = 0;
                
                if (allocation.x + allocation.width > cta->width)
                    allocation.width = cta->width - allocation.x;
                
                gtk_widget_set_clip(colobj->col_button, &allocation);
            }
        }
        _gtk_sheet_draw_button(sheet, -1, col, NULL);
    }
}

/**
 * gtk_sheet_set_column_width:
 * @sheet: a #GtkSheet.
 * @column: column number.
 * @width: the width of the column.
 *
 * Set column width. Check minimum width.
 * 
 * Return TRUE when changed
 */
gboolean
gtk_sheet_set_column_width(GtkSheet *sheet, gint col, guint width)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (col < 0 || col > sheet->maxcol) return FALSE;

#if GTK_SHEET_COL_DEBUG_SIZE > 0
    g_debug("%s(%d): col %d width %d", 
        __FUNCTION__, __LINE__, col, width);
#endif

    guint min_width;
    _gtk_sheet_column_size_request(sheet, col, &min_width);

    if (width < min_width) return FALSE;

    COLPTR(sheet, col)->width = width;

    _gtk_sheet_recalc_left_xpixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet))
        && !gtk_sheet_is_frozen(sheet))
    {
        _gtk_sheet_column_buttons_size_allocate(sheet);
        _gtk_sheet_scrollbar_adjust(sheet);
        _gtk_sheet_entry_size_allocate(sheet);
        _gtk_sheet_position_children(sheet);
        _gtk_sheet_range_draw(sheet, NULL, TRUE);
    }

    g_signal_emit_by_name(
        G_OBJECT(sheet), "new-column-width", col, width);

    return TRUE;
}


/**
 * gtk_sheet_get_column_width:
 * @sheet: a #GtkSheet
 * @column: column number
 *
 * Get column width.
 *
 * Returns: column width
 */
gint
gtk_sheet_get_column_width(GtkSheet *sheet, gint col)
{
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    return (COLPTR(sheet, col)->width);
}


/**
 * gtk_sheet_column_button_add_label:
 * @sheet: a #GtkSheet
 * @col: column number
 * @label: text label
 *
 * Set button label.It is used to set a column title.
 */
void
gtk_sheet_column_button_add_label(
    GtkSheet *sheet, gint col, const gchar *label)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;
    GtkSheetColumn *colobj = COLPTR(sheet, col);

#if GTK_SHEET_COL_DEBUG_BUTTON > 0
    g_debug("%s(%d): called %p %s label %s", 
        __FUNCTION__, __LINE__,
        colobj, gtk_widget_get_name(colobj), label);
#endif

    _gtk_sheet_column_button_add_label_internal(sheet, colobj, label);

    if (!gtk_sheet_is_frozen(sheet))
    {
        _gtk_sheet_draw_button(sheet, -1, col, NULL);
    }

    g_signal_emit_by_name(G_OBJECT(sheet), "changed", -1, col);
}

/**
 * gtk_sheet_column_set_justification:
 * @sheet: a #GtkSheet.
 * @col: column number
 * @just: a #GtkJustification : GTK_JUSTIFY_LEFT, RIGHT, CENTER
 *
 * Set column justification (GTK_JUSTIFY_LEFT, RIGHT, CENTER).
 * The default value is GTK_JUSTIFY_LEFT. 
 * If autoformat is on, the default justification for numbers is 
 * GTK_JUSTIFY_RIGHT. 
 */
void
gtk_sheet_column_set_justification(GtkSheet *sheet, gint col,
                                   GtkJustification justification)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->justification = justification;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet))
        && !gtk_sheet_is_frozen(sheet)
        && col >= MIN_VIEW_COLUMN(sheet)
        && col <= MAX_VIEW_COLUMN(sheet))
    {
        _gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
}

/**
 * gtk_sheet_column_get_justification:
 * @sheet: a #GtkSheet.
 * @col: column number
 *
 * Get the column justification. 
 *  
 * Returns: a #GtkJustification
 */
GtkJustification
gtk_sheet_column_get_justification(GtkSheet *sheet, gint col)
{
    g_return_val_if_fail(sheet != NULL, GTK_JUSTIFY_LEFT);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), GTK_JUSTIFY_LEFT);
    if (col < 0 || col > sheet->maxcol) return (GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT);

    return (COLPTR(sheet, col)->justification);
}

/**
 * gtk_sheet_column_set_vjustification:
 * @sheet: a #GtkSheet.
 * @col: column number
 * @vjust: a #GtkSheetVerticalJustification
 *
 * Set vertical cell text jjustification
 */
void
gtk_sheet_column_set_vjustification(GtkSheet *sheet, gint col,
                                    GtkSheetVerticalJustification vjust)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->vjust = vjust;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet))
        && !gtk_sheet_is_frozen(sheet)
        && col >= MIN_VIEW_COLUMN(sheet)
        && col <= MAX_VIEW_COLUMN(sheet))
    {
        _gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
}

/**
 * gtk_sheet_column_get_vjustification:
 * @sheet: a #GtkSheet.
 * @col: column number
 *
 * Get the vertical cell text justification. This overrides the 
 * default vertical cell text justification of the #GtkSheet. 
 *  
 * Returns: a #GtkSheetVerticalJustification
 */
GtkSheetVerticalJustification
gtk_sheet_column_get_vjustification(GtkSheet *sheet, gint col)
{
    g_return_val_if_fail(sheet != NULL, GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT);
    if (col < 0 || col > sheet->maxcol) return (GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT);

    return (COLPTR(sheet, col)->vjust);
}


/**
 * gtk_sheet_column_get_iskey: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the column is_key flag 
 *  
 * Returns:	the is_key flag
 */
gboolean gtk_sheet_column_get_iskey(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (col < 0 || col > sheet->maxcol) return (FALSE);

    return (COLPTR(sheet, col)->is_key);
}

/**
 * gtk_sheet_column_set_iskey: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @is_key:  the column is_key flag 
 *  
 * Sets the column is_key flag. This flag has no effect on 
 * calculation or presentation, it is reserved for application 
 * usage. 
 */
void gtk_sheet_column_set_iskey(GtkSheet *sheet, const gint col,
                                const gboolean is_key)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->is_key = is_key;
}

/**
 * gtk_sheet_column_get_readonly: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the column readonly flag 
 *  
 * Returns:	the readonly flag
 */
gboolean gtk_sheet_column_get_readonly(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (col < 0 || col > sheet->maxcol) return (FALSE);

    return (COLPTR(sheet, col)->is_readonly);
}

/**
 * gtk_sheet_column_set_readonly: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @is_readonly:  the column is_readonly flag 
 *  
 * Sets the column readonly flag. 
 * A cell is editable if the sheet is not locked, the column is 
 * not readonly and the cell (-range) was set to editable. 
 */
void gtk_sheet_column_set_readonly(GtkSheet *sheet, const gint col,
                                   const gboolean is_readonly)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->is_readonly = is_readonly;
}

/**
 * gtk_sheet_column_get_format: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the column data formatting pattern 
 *  
 * Returns:	the formatting pattern or NULL, You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_column_get_format(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return (NULL);

    return (g_strdup(COLPTR(sheet, col)->data_format));
}

/**
 * gtk_sheet_column_set_format: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @format:  the data_format pattern or NULL 
 *  
 * Sets the column data formatting pattern. 
 */
void gtk_sheet_column_set_format(GtkSheet *sheet, const gint col,
                                 const gchar *data_format)
{
    GtkSheetColumn *colp;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    colp = COLPTR(sheet, col);

    if (colp->data_format) g_free(colp->data_format);
    colp->data_format = g_strdup(data_format);
}

/**
 * gtk_sheet_column_get_datatype: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the column data_type for application use
 * 
 * Returns:	the datatype or NULL
 */
gchar *gtk_sheet_column_get_datatype(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return (NULL);

    return (g_strdup(COLPTR(sheet, col)->data_type));
}

/**
 * gtk_sheet_column_set_datatype: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @data_type:  the datatype 
 *  
 * Sets the column data data_type for application use 
 */
void gtk_sheet_column_set_datatype(GtkSheet *sheet, const gint col,
                                   const gchar *data_type)
{
    GtkSheetColumn *colp;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    colp = COLPTR(sheet, col);

    if (colp->data_type) g_free(colp->data_type);
    colp->data_type = g_strdup(data_type);
}

/**
 * gtk_sheet_column_get_description: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the column description 
 *  
 * Returns:	the description or NULL, You should free the
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_column_get_description(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return (NULL);

    return (g_strdup(COLPTR(sheet, col)->description));
}

/**
 * gtk_sheet_column_set_description: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @description:  the description or NULL 
 *  
 * Sets the column description. 
 */
void gtk_sheet_column_set_description(GtkSheet *sheet, const gint col,
                                      const gchar *description)
{
    GtkSheetColumn *colp;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    colp = COLPTR(sheet, col);

    if (colp->description) g_free(colp->description);
    colp->description = g_strdup(description);
}

/**
 * gtk_sheet_column_get_entry_type: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the column entry type if known 
 * 
 * Returns:	the entry type or GTK_SHEET_ENTRY_TYPE_DEFAULT
 */
GType
gtk_sheet_column_get_entry_type(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail(sheet != NULL, GTK_SHEET_ENTRY_TYPE_DEFAULT);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), GTK_SHEET_ENTRY_TYPE_DEFAULT);

    if (col < 0 || col > sheet->maxcol) return (GTK_SHEET_ENTRY_TYPE_DEFAULT);

    return (COLPTR(sheet, col)->entry_type);
}

/**
 * gtk_sheet_column_set_entry_type: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @entry_type:  the entry type or G_TYPE_NONE 
 *  
 * Supersedes the sheet entry type for this column. Pass 
 * G_TYPE_NONE to reset the column to the sheet entry type.
 */
void
gtk_sheet_column_set_entry_type(GtkSheet *sheet, const gint col,
                                const GType entry_type)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->entry_type = entry_type ? entry_type : G_TYPE_NONE;
}

/**
 * gtk_sheet_column_get_tooltip_markup: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the contents of the tooltip (markup) for the column 
 * 
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_column_get_tooltip_markup(GtkSheet *sheet,
                                           const gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return (NULL);

    return (gtk_widget_get_tooltip_markup(GTK_WIDGET(COLPTR(sheet, col))));
}

/**
 * gtk_sheet_column_set_tooltip_markup: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @markup:  	the contents of the tooltip for widget, or NULL. 
 *  
 * Sets markup as the contents of the tooltip, which is marked 
 * up with the Pango text markup language. 
 */
void gtk_sheet_column_set_tooltip_markup(GtkSheet *sheet,
                                         const gint col,
                                         const gchar *markup)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    gtk_widget_set_tooltip_markup(GTK_WIDGET(COLPTR(sheet, col)), markup);
}

/**
 * gtk_sheet_column_get_tooltip_text: 
 * @sheet:  a #GtkSheet. 
 * @col: column index 
 *  
 * Gets the contents of the tooltip for the column 
 *  
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_column_get_tooltip_text(GtkSheet *sheet,
                                         const gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return (NULL);

    return (gtk_widget_get_tooltip_text(GTK_WIDGET(COLPTR(sheet, col))));
}

/**
 * gtk_sheet_column_set_tooltip_text: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @text:  the contents of the tooltip for widget 
 *  
 * Sets text as the contents of the tooltip. 
 */
void gtk_sheet_column_set_tooltip_text(GtkSheet *sheet,
                                       const gint col,
                                       const gchar *text)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    gtk_widget_set_tooltip_text(GTK_WIDGET(COLPTR(sheet, col)), text);
}


/**
 * gtk_sheet_column_sensitive:
 * @sheet: a #GtkSheet.
 * @column: column number
 *
 * Get column button sensitivity. 
 *  
 * Returns: 
 * TRUE - the column is sensitive, FALSE - insensitive or not 
 * existant 
 */
gboolean
gtk_sheet_column_sensitive(GtkSheet *sheet, gint column)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (column < 0 || column > sheet->maxcol) return (FALSE);

    return (GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)));
}

/**
 * gtk_sheet_column_set_sensitivity:
 * @sheet: a #GtkSheet.
 * @column: column number
 * @sensitive: TRUE or FALSE
 *
 * Set column button sensitivity. If sensitivity is TRUE it can be toggled, otherwise it acts as a title.
 */
void
gtk_sheet_column_set_sensitivity(
    GtkSheet *sheet, gint col, gboolean sensitive)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
    g_debug("gtk_sheet_column_set_sensitivity: col %d", col);
#endif

    GTK_SHEET_COLUMN_SET_SENSITIVE(COLPTR(sheet, col), sensitive);
}

/**
 * gtk_sheet_columns_set_sensitivity:
 * @sheet: a #GtkSheet.
 * @sensitive: TRUE or FALSE
 *
 * Set all columns buttons sensitivity. If sensitivity is TRUE
 * button can be toggled, otherwise  act as titles. The sheet itself
 * has no such property, it is a convenience function to set the
 * property for all existing columns.
 */
void
gtk_sheet_columns_set_sensitivity(GtkSheet *sheet, gboolean sensitive)
{
    gint i;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    for (i = 0; i <= sheet->maxcol; i++) gtk_sheet_column_set_sensitivity(sheet, i, sensitive);
}

/**
 * gtk_sheet_columns_set_resizable:
 * @sheet: a #GtkSheet.
 * @resizable: TRUE or FALSE
 *
 * Set columns resizable status.
 */
void
gtk_sheet_columns_set_resizable(GtkSheet *sheet, gboolean resizable)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->columns_resizable = resizable;
}

/**
 * gtk_sheet_columns_resizable:
 * @sheet: a #GtkSheet.
 *
 * Get columns resizable status.
 *
 * Returns: TRUE or FALSE
 */
gboolean
gtk_sheet_columns_resizable(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->columns_resizable);
}

/**
 * _gtk_sheet_column_button_set:
 * @sheet:  the #GtkSheet
 * @col:    column index 
 *  
 * activate and draw column button
 */
void
_gtk_sheet_column_button_set(GtkSheet *sheet, gint col)
{
    if (col < 0 || col > sheet->maxcol) return;

    g_assert(0 <= col && col <= sheet->maxcol);
    GtkSheetColumn *colobj = COLPTR(sheet, col);

    if (colobj->col_button
        && GTK_IS_TOGGLE_BUTTON(colobj->col_button))
    {
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(colobj->col_button), TRUE);
    }
}

/**
 * _gtk_sheet_column_button_release:
 * @sheet:  the #GtkSheet
 * @col:    column index 
 *  
 * reset and draw column button
 */
void
_gtk_sheet_column_button_release(GtkSheet *sheet, gint col)
{
    if (col < 0 || col > sheet->maxcol) return;

    g_assert(0 <= col && col <= sheet->maxcol);
    GtkSheetColumn *colobj = COLPTR(sheet, col);

    if (colobj->col_button
        && GTK_IS_TOGGLE_BUTTON(colobj->col_button))
    {
        gtk_toggle_button_set_active(
            GTK_TOGGLE_BUTTON(colobj->col_button), FALSE);
    }
}

/**
 * gtk_sheet_column_visible:
 * @sheet: a #GtkSheet.
 * @column: column number
 *
 * Get column visible property. 
 *  
 * Returns: TRUE - visible, FALSE - hidden or not existant 
 */
gboolean
gtk_sheet_column_visible(GtkSheet *sheet, gint column)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (column < 0 || column > sheet->maxcol) return (FALSE);

    return (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column)));
}

/**
 * gtk_sheet_column_set_visibility:
 * @sheet: a #GtkSheet.
 * @column: column number
 * @visible: TRUE or FALSE
 *
 * Set column visible property. 
 *
 * Default value is TRUE. When set to FALSE, the column is 
 * hidden. 
 */
void
gtk_sheet_column_set_visibility(GtkSheet *sheet, gint col, gboolean visible)
{
    GtkSheetColumn *colobj;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    colobj = COLPTR(sheet, col);
    if (GTK_SHEET_COLUMN_IS_VISIBLE(colobj) == visible) return;

    //gint act_row = sheet->active_cell.row;
    gint act_col = sheet->active_cell.col;

    if (act_col == col) /* hide active column -> disable active cell */
    {
        // FIXME - should deactivate active cell properly
        //_gtk_sheet_hide_active_cell(sheet);
        //SET_ACTIVE_CELL(-1, -1);

        gint old_state = sheet->state;

        if (gtk_widget_get_realized(GTK_WIDGET(sheet))
            && old_state == GTK_SHEET_NORMAL)
        {
            gboolean veto = _gtk_sheet_deactivate_cell(sheet);
            // there are 2 possible reasons for veto
            // no active_cell or DEACTIVATE signal returns veto
            if (!veto) return;
        }
    }

#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
    g_debug("gtk_sheet_column_set_visibility: col %d = %s, m %d r %d v %d parent %p", col,
            visible ? "true" : "false",
            gtk_widget_get_mapped(GTK_WIDGET(colobj)),
            gtk_widget_get_realized(GTK_WIDGET(colobj)),
            gtk_widget_get_visible(GTK_WIDGET(colobj)),
            gtk_widget_get_parent(GTK_WIDGET(colobj))
            );
#endif

    GTK_SHEET_COLUMN_SET_VISIBLE(colobj, visible);

    if (!visible && gtk_widget_get_mapped(colobj))
    {
        gtk_widget_unmap(colobj);
    }

    _gtk_sheet_range_fixup(sheet, &sheet->range);
    _gtk_sheet_recalc_left_xpixels(sheet);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
}

/**
 * gtk_sheet_column_button_justify:
 * @sheet: a #GtkSheet.
 * @col: column number
 * @justification: a #GtkJustification :GTK_JUSTIFY_LEFT, RIGHT, 
 *  			 CENTER
 *
 * Set the justification(alignment) of the column buttons.
 * 
 * This function does nothing, it is kept for
 * backward-compatibility.
 *  
 * Deprecated:4.1.3: Use widget properties instead
 */ 
void
gtk_sheet_column_button_justify(GtkSheet *sheet, gint col,
                                GtkJustification justification)
{
    g_debug("%s(%d): function deprecated by col_button",
        __FUNCTION__, __LINE__);
}

/**
 * gtk_sheet_column_button_get_label:
 * @sheet: a #GtkSheet.
 * @col: column number.
 *
 * Get column button label.
 *
 * Returns: Column button label. 
 *  
 * The text of the label widget. This string is owned by the widget and must not be modified or freed. 
 */
const gchar *
gtk_sheet_column_button_get_label(GtkSheet *sheet, gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);

    g_assert(0 <= col && col <= sheet->maxcol);
    GtkSheetColumn *colobj = COLPTR(sheet, col);

    return(_gtk_sheet_column_button_get_label_internal(colobj));
}

/**
 * gtk_sheet_column_label_set_visibility:
 * @sheet: a #GtkSheet.
 * @col: column number.
 * @visible: TRUE or FALSE
 *
 * Set column label visibility. The default value is TRUE. If FALSE, the column label is hidden.
 */
void
gtk_sheet_column_label_set_visibility(
    GtkSheet *sheet, gint col, gboolean visible)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

#if GTK_SHEET_COL_DEBUG_PROPERTIES > 0
    g_debug("gtk_sheet_column_label_set_visibility: col %d", col);
#endif

    g_assert(0 <= col && col <= sheet->maxcol);
    GtkSheetColumn *colobj = COLPTR(sheet, col);

    if (colobj->col_button)
    {
        gtk_widget_set_visible(colobj->col_button, visible);
    }
}

/**
 * gtk_sheet_columns_labels_set_visibility:
 * @sheet: a #GtkSheet.
 * @visible: TRUE or FALSE
 *
 * Set all columns labels visibility. The default value is TRUE.
 * If FALSE, the columns labels are hidden. The sheet itself
 * has no such property, it is a convenience function to set the
 * property for all existing columns.
 */
void
gtk_sheet_columns_labels_set_visibility(GtkSheet *sheet, gboolean visible)
{
    gint i;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    for (i = 0; i <= sheet->maxcol; i++)
        gtk_sheet_column_label_set_visibility(sheet, i, visible);
}

/**
 * gtk_sheet_set_column_titles_height:
 * @sheet: a #GtkSheet
 * @height: column title height.
 *
 * Resize column titles area .
 */
void
gtk_sheet_set_column_titles_height(GtkSheet *sheet, guint height)
{
    if (height < _gtk_sheet_row_default_height(GTK_WIDGET(sheet))) return;

    sheet->column_title_area.height = height;

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);
    _gtk_sheet_recalc_view_range(sheet);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);
}

/**
 * gtk_sheet_show_column_titles:
 * @sheet: a #GtkSheet
 *
 * Show column title area.
 */
void
gtk_sheet_show_column_titles(GtkSheet *sheet)
{
    if (sheet->column_titles_visible) return;

    sheet->column_titles_visible = TRUE;  /* must be set first */

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (gtk_sheet_is_frozen(sheet)) return;

    gdk_window_show(sheet->column_title_window);

    gdk_window_move_resize(sheet->column_title_window,
                           sheet->column_title_area.x,
                           sheet->column_title_area.y,
                           sheet->column_title_area.width,
                           sheet->column_title_area.height);

    _gtk_sheet_global_sheet_button_show(sheet);

    gint col;
    for (col = MIN_VIEW_COLUMN(sheet);
          col <= MAX_VIEW_COLUMN(sheet) && col <= sheet->maxcol;
          col++)
    {
        if (col < 0) continue;

        g_assert(0 <= col && col <= sheet->maxcol);
        GtkSheetColumn *colobj = COLPTR(sheet, col);

        if (colobj->col_button
            && gtk_widget_get_visible(GTK_WIDGET(colobj)))
        {
            gtk_widget_show(colobj->col_button);
        }
    }

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);
}

/**
 * gtk_sheet_hide_column_titles:
 * @sheet: a #GtkSheet
 *
 * Hide column title area.
 */
void
gtk_sheet_hide_column_titles(GtkSheet *sheet)
{
    if (!sheet->column_titles_visible) return;

    sheet->column_titles_visible = FALSE;  /* must be set first */

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (gtk_sheet_is_frozen(sheet)) return;

    if (sheet->column_title_window)
        gdk_window_hide(sheet->column_title_window);

    _gtk_sheet_global_sheet_button_hide(sheet);

    gint col;
    for (col = 0; col <= sheet->maxcol; col++)
    {
        g_assert(0 <= col && col <= sheet->maxcol);
        GtkSheetColumn *colobj = COLPTR(sheet, col);

        if (colobj->col_button) gtk_widget_hide(colobj->col_button);
    }

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);
}

/**
 * gtk_sheet_set_column_title:
 * @sheet: a #GtkSheet
 * @column: column number
 * @title: column title
 *
 * Set column title.
 */
void
gtk_sheet_set_column_title(GtkSheet *sheet,
    gint col,
    const gchar *title)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol) return;

    GtkSheetColumn *colobj = COLPTR(sheet, col);

    _gtk_sheet_column_button_add_label_internal(sheet, colobj, title);
}

/**
 * gtk_sheet_get_column_title:
 * @sheet: a #GtkSheet
 * @column: column number
 *
 * Get column title.
 *
 * Returns: column title, do not modify or free it.
 */
const gchar *
gtk_sheet_get_column_title(GtkSheet *sheet,
    gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (gtk_sheet_column_button_get_label(sheet, col));
}

/**
 * gtk_sheet_column_titles_visible:
 * @sheet: a #GtkSheet
 *
 * Get the visibility of sheet column titles .
 *
 * Returns: TRUE or FALSE
 */
gboolean
gtk_sheet_column_titles_visible(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->column_titles_visible);
}


