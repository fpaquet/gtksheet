/* GtkSheet widget for Gtk+.
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

/**
 * SECTION: gtksheet
 * @short_description: A spreadsheet widget for Gtk2
 *
 * #GtkSheet is a matrix widget for GTK+. It consists of an scrollable grid of
 * cells where you can allocate text. Cell contents can be edited interactively
 * through a specially designed entry, GtkItemEntry. It is also a container
 * subclass, allowing you to display buttons, curves, pixmaps and any other
 * widget in it. You can also set many attributes as: border, foreground and
 * background color, text justification, and more. The testgtksheet program
 * shows how easy is to create a spreadsheet-like GUI using this widget set.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtktable.h>
#include <gtk/gtkbox.h>
#include <gtk/gtkmain.h>
#include <gtk/gtktypeutils.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkcontainer.h>
#include <gtk/gtkpixmap.h>
#include <pango/pango.h>
#include "gtkextra-compat.h"
#include "gtkitementry.h"
#include "gtksheet.h"
#include "gtkextra-marshal.h"
#include "gtkextratypebuiltins.h"

#undef GTK_SHEET_DEBUG

#ifdef DEBUG
    #  define GTK_SHEET_DEBUG 1  /* define to activate debug output */
#endif



/* sheet flags */
enum _GtkSheetFlags
{
    GTK_SHEET_IS_LOCKED  = 1 << 0,    /* sheet is not editable */
    GTK_SHEET_IS_FROZEN  = 1 << 1,    /* frontend updates temporarily disabled */
    GTK_SHEET_IN_XDRAG  = 1 << 2,    /* column being resized */
    GTK_SHEET_IN_YDRAG  = 1 << 3,    /* row being resized */
    GTK_SHEET_IN_DRAG  = 1 << 4,    /* cell selection being moved */
    GTK_SHEET_IN_SELECTION  = 1 << 5,   /* cell selection being created */
    GTK_SHEET_IN_RESIZE  = 1 << 6,  /* cell selection being resized */
    GTK_SHEET_IN_CLIP  = 1 << 7,    /* cell selection in clipboard */
    GTK_SHEET_REDRAW_PENDING  = 1 << 8,
};

enum _GtkSheetProperties
{
    PROP_GTK_SHEET_0,  /* dummy */
    PROP_GTK_SHEET_TITLE, /* gtk_sheet_set_title() */
    PROP_GTK_SHEET_NCOLS, /* number of colunms - necessary for glade object creation */
    PROP_GTK_SHEET_NROWS,  /* number of rows - necessary for glade object creation */
    PROP_GTK_SHEET_LOCKED,  /* gtk_sheet_set_locked() */
    PROP_GTK_SHEET_SELECTION_MODE,  /* gtk_sheet_set_selection_mode() */
    PROP_GTK_SHEET_AUTO_RESIZE,  /* gtk_sheet_set_autoresize() */
    PROP_GTK_SHEET_AUTO_SCROLL,  /* gtk_sheet_set_autoscroll() */
    PROP_GTK_SHEET_CLIP_TEXT,  /* gtk_sheet_set_clip_text() */
    PROP_GTK_SHEET_JUSTIFY_ENTRY,  /* gtk_sheet_set_justify_entry() */
    PROP_GTK_SHEET_BG_COLOR,  /* gtk_sheet_set_background() */
    PROP_GTK_SHEET_GRID_VISIBLE,  /* gtk_sheet_show_grid() */
    PROP_GTK_SHEET_GRID_COLOR,  /* gtk_sheet_set_grid() */
    PROP_GTK_SHEET_COLUMN_TITLES_VISIBLE,  /* gtk_sheet_show_column_titles() */
    PROP_GTK_SHEET_COLUMNS_RESIZABLE,  /* gtk_sheet_columns_set_resizable() */
    PROP_GTK_SHEET_COLUMN_TITLES_HEIGHT,  /* gtk_sheet_set_column_titles_height() */
    PROP_GTK_SHEET_ROW_TITLES_VISIBLE,  /* gtk_sheet_show_row_titles() */
    PROP_GTK_SHEET_ROWS_RESIZABLE,  /* gtk_sheet_rows_set_resizable() */
    PROP_GTK_SHEET_ROW_TITLES_WIDTH,  /* gtk_sheet_set_row_titles_width() */
    PROP_GTK_SHEET_ENTRY_TYPE,  /* gtk_sheet_change_entry() */
};

enum _GtkSheetColumnProperties
{
    PROP_0,
    PROP_GTK_SHEET_COLUMN_0,  /* dummy */
    PROP_GTK_SHEET_COLUMN_POSITION,  /* position of the column */
    PROP_GTK_SHEET_COLUMN_LABEL,  /* gtk_sheet_column_button_add_label() */
    PROP_GTK_SHEET_COLUMN_WIDTH,  /* gtk_sheet_set_column_width() */
    PROP_GTK_SHEET_COLUMN_JUSTIFICATION,  /* gtk_sheet_column_set_justification() */
    PROP_GTK_SHEET_COLUMN_ISKEY,  /* gtk_sheet_column_set_iskey() */
    PROP_GTK_SHEET_COLUMN_READONLY,  /* gtk_sheet_column_set_readonly() */
    PROP_GTK_SHEET_COLUMN_DATAFMT,  /* gtk_sheet_column_set_format() */
    PROP_GTK_SHEET_COLUMN_DATATYPE,  /* gtk_sheet_column_set_datatype() */
    PROP_GTK_SHEET_COLUMN_DESCRIPTION,  /* gtk_sheet_column_set_description() */
    PROP_GTK_SHEET_COLUMN_ENTRY_TYPE,  /* gtk_sheet_column_set_entry_type() */
};

/* Signals */

enum _GtkSheetSignals
{
    SELECT_ROW,
    SELECT_COLUMN,
    SELECT_RANGE,
    CLIP_RANGE,
    RESIZE_RANGE,
    MOVE_RANGE,
    TRAVERSE,
    DEACTIVATE,
    ACTIVATE,
    SET_CELL,
    CLEAR_CELL,
    CHANGED,
    NEW_COL_WIDTH,
    NEW_ROW_HEIGHT,
    LAST_SIGNAL
};
static guint sheet_signals[LAST_SIGNAL] = {0};

typedef enum _GtkSheetArea
{
    ON_SHEET_BUTTON_AREA,
    ON_ROW_TITLES_AREA,
    ON_COLUMN_TITLES_AREA,
    ON_CELL_AREA
} GtkSheetArea;

#define GTK_SHEET_FLAGS(sheet)             (GTK_SHEET (sheet)->flags)
#define GTK_SHEET_SET_FLAGS(sheet,flag)    (GTK_SHEET_FLAGS (sheet) |= (flag))
#define GTK_SHEET_UNSET_FLAGS(sheet,flag)  (GTK_SHEET_FLAGS (sheet) &= ~(flag))

#define GTK_SHEET_IS_FROZEN(sheet)   (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IS_FROZEN)
#define GTK_SHEET_IN_XDRAG(sheet)    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_XDRAG)
#define GTK_SHEET_IN_YDRAG(sheet)    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_YDRAG)
#define GTK_SHEET_IN_DRAG(sheet)     (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_DRAG)
#define GTK_SHEET_IN_SELECTION(sheet) (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_SELECTION)
#define GTK_SHEET_IN_RESIZE(sheet) (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_RESIZE)
#define GTK_SHEET_IN_CLIP(sheet) (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_CLIP)
#define GTK_SHEET_REDRAW_PENDING(sheet)   (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_REDRAW_PENDING)

/* data access macros - no frontend update! */

#define COLPTR(sheet, colidx) (sheet->column[colidx])

#define GTK_SHEET_COLUMN_IS_VISIBLE(colptr)  (gtk_widget_get_visible(GTK_WIDGET(colptr)))
#define GTK_SHEET_COLUMN_SET_VISIBLE(colptr, value) (gtk_widget_set_visible(GTK_WIDGET(colptr), value))
#define GTK_SHEET_COLUMN_IS_SENSITIVE(colptr)  (gtk_widget_is_sensitive(GTK_WIDGET(colptr)))
#define GTK_SHEET_COLUMN_SET_SENSITIVE(colptr, value) (gtk_widget_set_sensitive(GTK_WIDGET(colptr), value))

#define ROWPTR(sheet, rowidx) (&sheet->row[rowidx])

#define GTK_SHEET_ROW_IS_VISIBLE(rowptr)  ((rowptr)->is_visible)
#define GTK_SHEET_ROW_SET_VISIBLE(rowptr, value) ((rowptr)->is_visible = (value))
#define GTK_SHEET_ROW_IS_SENSITIVE(rowptr)  ((rowptr)->is_sensitive)
#define GTK_SHEET_ROW_SET_SENSITIVE(rowptr, value) ((rowptr)->is_sensitive = (value))

/* defaults */

#define CELL_SPACING 1
#define DRAG_WIDTH 6
#define TIMEOUT_SCROLL 20
#define TIMEOUT_FLASH 200
#define TIME_INTERVAL 8
#define COLUMN_MIN_WIDTH 10
#define MINROWS 0
#define MINCOLS 0
#define MAXLENGTH 30
#define CELLOFFSET 4
#define GTK_SHEET_DEFAULT_COLUMN_WIDTH 80
#define GTK_SHEET_DEFAULT_COLUMN_JUSTIFICATION GTK_JUSTIFY_LEFT
#define GTK_SHEET_DEFAULT_ROW_HEIGHT 24
#define GTK_SHEET_DEFAULT_BG_COLOR      "white"
#define GTK_SHEET_DEFAULT_GRID_COLOR  "gray"
#define GTK_SHEET_DEFAULT_TM_COLOR  "red"   /* tooltip marker */
#define GTK_SHEET_DEFAULT_TM_SIZE  4  /* pixels, size of tooltip marker */

#ifdef GTK_SHEET_DEBUG
    #  define GTK_SHEET_DEBUG_COLOR  "green"
static GdkColor debug_color;

    #include <stdarg.h>
static void g_debug_popup(char *fmt, ...)  /* used to intercept/debug drawing sequences */
{
    va_list ap;
    va_start(ap, fmt);

    GtkWidget *dialog = gtk_message_dialog_new (NULL,
                                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                                GTK_MESSAGE_ERROR,
                                                GTK_BUTTONS_CLOSE,
                                                fmt, ap);

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}
#endif

static inline gint
    gtk_sheet_row_height(GtkSheet *sheet, gint row)
{
    return(sheet->row[row].height);
}

static inline gint
    gtk_sheet_column_width(GtkSheet *sheet, gint col)
{
    return(sheet->column[col]->width);
}

static inline guint 
    DEFAULT_ROW_HEIGHT(GtkWidget *widget)
{
    if (!gtk_widget_get_style(widget)->font_desc)
        return(GTK_SHEET_DEFAULT_ROW_HEIGHT);
    else
    {
        PangoContext *context = gtk_widget_get_pango_context(widget);
        PangoFontMetrics *metrics = pango_context_get_metrics(context,
                                                              gtk_widget_get_style(widget)->font_desc,
                                                              pango_context_get_language(context));
        guint val = pango_font_metrics_get_descent(metrics) +
            pango_font_metrics_get_ascent(metrics);
        pango_font_metrics_unref(metrics);
        return(PANGO_PIXELS(val)+2*CELLOFFSET);
    }
}

static inline guint 
    DEFAULT_FONT_ASCENT(GtkWidget *widget)
{
    if (!gtk_widget_get_style(widget)->font_desc) return(12);
    else
    {
        PangoContext *context = gtk_widget_get_pango_context(widget);
        PangoFontMetrics *metrics = pango_context_get_metrics(context,
                                                              gtk_widget_get_style(widget)->font_desc,
                                                              pango_context_get_language(context));
        guint val = pango_font_metrics_get_ascent(metrics);
        pango_font_metrics_unref(metrics);
        return(PANGO_PIXELS(val));
    }
}

static inline guint 
    STRING_WIDTH(GtkWidget *widget, 
                 PangoFontDescription *font, const gchar *text)
{
    PangoRectangle rect;
    PangoLayout *layout;

    layout = gtk_widget_create_pango_layout (widget, text);
    pango_layout_set_font_description (layout, font);

    pango_layout_get_extents (layout, NULL, &rect);

    g_object_unref(G_OBJECT(layout));

    return(PANGO_PIXELS(rect.width));
}

static inline 
    guint DEFAULT_FONT_DESCENT(GtkWidget *widget)
{
    if (!gtk_widget_get_style(widget)->font_desc) return(12);

    PangoContext *context = gtk_widget_get_pango_context(widget);
    PangoFontMetrics *metrics = pango_context_get_metrics(context,
                                                          gtk_widget_get_style(widget)->font_desc,
                                                          pango_context_get_language(context));
    guint val =  pango_font_metrics_get_descent(metrics);
    pango_font_metrics_unref(metrics);

    return(PANGO_PIXELS(val));
}

/* gives the top/bottom pixel of the given row in context of the sheet's voffset */

static inline gint
    ROW_TOP_YPIXEL(GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow) return(sheet->voffset);
    return(sheet->voffset + sheet->row[row].top_ypixel);
}

static inline gint
    ROW_BOTTOM_YPIXEL(GtkSheet *sheet, gint row)
{
    gint ypixel = ROW_TOP_YPIXEL(sheet, row);
    if (0 <= row && row <= sheet->maxrow) ypixel += sheet->row[row].height;
    return(ypixel);
}


/* returns the row index from a y pixel location in the
 * context of the sheet's voffset
 * beware: the top border belongs to the row, the bottom border to the next */

static inline gint
    ROW_FROM_YPIXEL(GtkSheet *sheet, gint y)
{
    gint i, cy;

    cy = sheet->voffset;
    if (sheet->column_titles_visible) cy += sheet->column_title_area.height;

    if (y < cy) return (0);

    for (i = 0; i <= sheet->maxrow; i++)
    {
        if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
        {
            if (cy <= y  && y < (cy + sheet->row[i].height)) return (i);
            cy += sheet->row[i].height;
        }
    }

    /* no match */
    return(sheet->maxrow + 1);
}


/* gives the left/right pixel of the given column in context of the sheet's hoffset */

static inline gint
    COLUMN_LEFT_XPIXEL(GtkSheet *sheet, gint col)
{
    if (col < 0 || col > sheet->maxcol) return(sheet->hoffset);
    return(sheet->hoffset + sheet->column[col]->left_xpixel);
}

static inline gint
    COLUMN_RIGHT_XPIXEL(GtkSheet *sheet, gint col)
{
    gint xpixel = COLUMN_LEFT_XPIXEL(sheet, col);
    if (0 <= col && col <= sheet->maxcol) xpixel += sheet->column[col]->width;
    return(xpixel);
}


/* returns the column index from a x pixel location in the
 * context of the sheet's hoffset
 * beware: the left border belongs to the column, the right border to the next  */

static inline gint
    COLUMN_FROM_XPIXEL (GtkSheet * sheet, gint x)
{
    gint i, cx;

    cx = sheet->hoffset;
    if (sheet->row_titles_visible) cx += sheet->row_title_area.width;

    if (x < cx) return (0);

    for (i = 0; i <= sheet->maxcol; i++)
    {
        if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
        {
            if (cx <= x  && x < (cx + sheet->column[i]->width)) return (i);
            cx += sheet->column[i]->width;
        }
    }

    /* no match */
    return(sheet->maxcol + 1);
}

/* returns the total height of the sheet */

static inline gint 
    SHEET_HEIGHT(GtkSheet *sheet)
{
    gint i,cx;

    cx = ( sheet->column_titles_visible ? sheet->column_title_area.height : 0);

    for (i=0;i<=sheet->maxrow; i++)
    {
        if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i))) cx += sheet->row[i].height;
    }

    return(cx);
}

/* returns the total width of the sheet */

static inline gint 
    SHEET_WIDTH(GtkSheet *sheet)
{
    gint i,cx;

    cx = ( sheet->row_titles_visible ? sheet->row_title_area.width : 0);

    for (i=0;i<=sheet->maxcol; i++)
    {
        if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i))) cx += sheet->column[i]->width;
    }

    return(cx);
}

#define MIN_VISIBLE_ROW(sheet)  sheet->view.row0
#define MAX_VISIBLE_ROW(sheet)  sheet->view.rowi
#define MIN_VISIBLE_COLUMN(sheet)  sheet->view.col0
#define MAX_VISIBLE_COLUMN(sheet)  sheet->view.coli

static void
    gtk_sheet_recalc_view_range(GtkSheet *sheet)
{
    sheet->view.row0 = ROW_FROM_YPIXEL(sheet, 
                                       sheet->column_titles_visible ? sheet->column_title_area.height : 0);
    sheet->view.rowi = ROW_FROM_YPIXEL(sheet, 
                                       sheet->sheet_window_height - 1);

    sheet->view.col0 = COLUMN_FROM_XPIXEL(sheet, 
                                          sheet->row_titles_visible ? sheet->row_title_area.width : 0);
    sheet->view.coli = COLUMN_FROM_XPIXEL(sheet, 
                                          sheet->sheet_window_width - 1);
}

/* force range into bounds */

static void 
    fixup_range(GtkSheet *sheet, GtkSheetRange *range)
{
    if (range->row0 < 0) range->row0 = 0;
    if (range->rowi > sheet->maxrow) range->rowi = sheet->maxrow;
    if (range->col0 < 0) range->col0 = 0;
    if (range->coli > sheet->maxcol) range->coli = sheet->maxcol;
}

/*
 * POSSIBLE_XDRAG
 * 
 * Drag grip test for column width dragging
 * 
 * @param sheet
 * @param x
 * @param drag_column
 * 
 * @return 
 */
static inline gint
    POSSIBLE_XDRAG(GtkSheet *sheet, gint x, gint *drag_column)
{
    gint column, xdrag;

    column = COLUMN_FROM_XPIXEL(sheet, x);
    if (column < 0 || column > sheet->maxcol) return(FALSE);

    xdrag = COLUMN_LEFT_XPIXEL(sheet, column);

    if (column > 0 && x <= xdrag+DRAG_WIDTH/2)  /* you pick it at the left border */
    {
        while (column > 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column-1))) column--;

        --column;  /* you really want to resize the column on the left side */

        if (column < 0 || column > sheet->maxcol) return(FALSE);

        *drag_column = column;
        return(TRUE);
#if 0
        return(GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)));
#endif
    }

    xdrag = COLUMN_RIGHT_XPIXEL(sheet, column);

    if (xdrag-DRAG_WIDTH/2 <= x && x <= xdrag+DRAG_WIDTH/2)
    {
        *drag_column=column;
        return(TRUE);
#if 0
        return(GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)));
#endif
    }

    return(FALSE);
}

/*
 * POSSIBLE_YDRAG
 * 
 * Drag grip test for row height dragging
 * 
 * @param sheet
 * @param y
 * @param drag_row
 * 
 * @return 
 */
static inline gint
    POSSIBLE_YDRAG(GtkSheet *sheet, gint y, gint *drag_row)
{
    gint row, ydrag;

    row = ROW_FROM_YPIXEL(sheet, y);
    if (row < 0 || row > sheet->maxrow) return(FALSE);

    ydrag = ROW_TOP_YPIXEL(sheet, row);

    if (row > 0 && y <= ydrag+DRAG_WIDTH/2)  /* you pick it at the top border */
    {
        while (row > 0 && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row-1))) row--;

        --row;  /* you really want to resize the row above */

        if (row < 0 || row > sheet->maxrow) return(FALSE);

        *drag_row=row;
        return(TRUE);
#if 0
        return(GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)));
#endif
    }

    ydrag = ROW_BOTTOM_YPIXEL(sheet, row);

    if (ydrag-DRAG_WIDTH/2 <= y && y <= ydrag+DRAG_WIDTH/2)
    {
        *drag_row=row;
        return(TRUE);
#if 0
        return(GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)));
#endif
    }

    return(FALSE);
}

/*
 * POSSIBLE_DRAG
 * 
 * Dragging grip test for a cell range
 * 
 * @param sheet
 * @param x
 * @param y
 * @param drag_row
 * @param drag_column
 * 
 * @return 
 */
static inline gint 
    POSSIBLE_DRAG(GtkSheet *sheet, gint x, gint y, gint *drag_row, gint *drag_column)
{
    gint ydrag, xdrag;

    *drag_column=COLUMN_FROM_XPIXEL(sheet, x);
    *drag_row=ROW_FROM_YPIXEL(sheet, y);

    /* drag grip at top/bottom border */

    if (x >= COLUMN_LEFT_XPIXEL(sheet, sheet->range.col0) - DRAG_WIDTH/2 &&
        x <= COLUMN_RIGHT_XPIXEL(sheet, sheet->range.coli) + DRAG_WIDTH/2)
    {
        ydrag = ROW_TOP_YPIXEL(sheet, sheet->range.row0);

        if (ydrag-DRAG_WIDTH/2 <= y && y <= ydrag+DRAG_WIDTH/2)
        {
            *drag_row=sheet->range.row0;
            return (TRUE);
        }

        ydrag = ROW_BOTTOM_YPIXEL(sheet, sheet->range.rowi);

        if (ydrag-DRAG_WIDTH/2 <= y && y <= ydrag+DRAG_WIDTH/2)
        {
            *drag_row=sheet->range.rowi;
            return (TRUE);
        }

    }

    /* drag grip at left/right border */

    if (y >= ROW_TOP_YPIXEL(sheet,sheet->range.row0) - DRAG_WIDTH/2 &&
        y <= ROW_BOTTOM_YPIXEL(sheet,sheet->range.rowi) + DRAG_WIDTH/2)
    {
        xdrag = COLUMN_LEFT_XPIXEL(sheet, sheet->range.col0);

        if (xdrag-DRAG_WIDTH/2 <= x && x <= xdrag+DRAG_WIDTH/2)
        {
            *drag_column=sheet->range.col0;
            return (TRUE);
        }

        xdrag = COLUMN_RIGHT_XPIXEL(sheet, sheet->range.coli);

        if (xdrag-DRAG_WIDTH/2 <= x && x <= xdrag+DRAG_WIDTH/2)
        {
            *drag_column=sheet->range.coli;
            return (TRUE);
        }
    }

    return (FALSE);
}

static inline gint 
    POSSIBLE_RESIZE(GtkSheet *sheet, gint x, gint y, gint *drag_row, gint *drag_column)
{
    gint xdrag, ydrag;

    xdrag = COLUMN_RIGHT_XPIXEL(sheet, sheet->range.coli);
    ydrag = ROW_BOTTOM_YPIXEL(sheet,sheet->range.rowi);

    if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
        ydrag = ROW_TOP_YPIXEL(sheet, sheet->view.row0);

    if (sheet->state == GTK_SHEET_ROW_SELECTED)
        xdrag = COLUMN_LEFT_XPIXEL(sheet, sheet->view.col0);

    *drag_column=COLUMN_FROM_XPIXEL(sheet, x);
    *drag_row=ROW_FROM_YPIXEL(sheet, y);

    if (xdrag-DRAG_WIDTH/2 <= x && x <= xdrag+DRAG_WIDTH/2 &&
        ydrag-DRAG_WIDTH/2 <= y && y <= ydrag+DRAG_WIDTH/2)
    {
        return (TRUE);
    }

    return (FALSE);
}

/* Prototypes (extern) */

extern void _gtkextra_signal_emit(GtkObject *object, guint signal_id, ...);

/* Prototypes (static) */

static void gtk_sheet_class_init(GtkSheetClass *klass);
static void gtk_sheet_init(GtkSheet *sheet);
static void gtk_sheet_column_class_init(GtkSheetColumnClass *klass);
static void gtk_sheet_column_init(GtkSheetColumn *column);
static void gtk_sheet_destroy_handler(GtkObject * object);
static void gtk_sheet_finalize_handler(GObject * object);
static void gtk_sheet_style_set_handler(GtkWidget *widget, GtkStyle  *previous_style);
static void gtk_sheet_realize_handler(GtkWidget *widget);
static void gtk_sheet_unrealize_handler(GtkWidget *widget);
static void gtk_sheet_map_handler(GtkWidget *widget);
static void gtk_sheet_unmap_handler(GtkWidget *widget);

static gboolean gtk_sheet_expose_handler(GtkWidget *widget, 
                                         GdkEventExpose *event);

static void gtk_sheet_forall_handler(GtkContainer *container, 
                             gboolean include_internals, GtkCallback  callback, gpointer callback_data);

static void gtk_sheet_set_scroll_adjustments(GtkSheet *sheet, 
                                             GtkAdjustment *hadjustment, GtkAdjustment *vadjustment);

static gboolean gtk_sheet_button_press_handler(GtkWidget *widget, GdkEventButton *event);
static gboolean gtk_sheet_button_release_handler(GtkWidget *widget, GdkEventButton *event);
static gboolean gtk_sheet_motion_handler(GtkWidget *widget, GdkEventMotion *event);

static gboolean gtk_sheet_entry_key_press_handler(GtkWidget *widget, 
                                                  GdkEventKey *key, gpointer user_data);

static gboolean gtk_sheet_key_press_handler(GtkWidget *widget, GdkEventKey *key);

static void gtk_sheet_size_request_handler(GtkWidget *widget, GtkRequisition *requisition);
static void gtk_sheet_size_allocate_handler(GtkWidget * widget, GtkAllocation *allocation);

/* Sheet queries */

static gint gtk_sheet_range_isvisible(GtkSheet *sheet, GtkSheetRange range);
static gint gtk_sheet_cell_isvisible(GtkSheet *sheet, gint row, gint column);

/* Clipped Range */

static gint gtk_sheet_scroll(gpointer data);
static gint gtk_sheet_flash(gpointer data);

/* Drawing Routines */

/* draw cell background and frame */
static void gtk_sheet_cell_draw_default(GtkSheet *sheet, gint row, gint column);

/* draw cell border */
static void gtk_sheet_cell_draw_border(GtkSheet *sheet, 
                                       gint row, gint column, gint mask);

/* draw cell contents */
static void gtk_sheet_cell_draw_label(GtkSheet *sheet, gint row, gint column);

/* draw visible part of range. If range==NULL then draw the whole screen */
static void gtk_sheet_range_draw(GtkSheet *sheet, const GtkSheetRange *range);

/* highlight the visible part of the selected range */
static void gtk_sheet_range_draw_selection(GtkSheet *sheet, GtkSheetRange range);

/* Selection */

static gint gtk_sheet_move_query(GtkSheet *sheet, gint row, gint column);
static void gtk_sheet_real_select_range(GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_real_unselect_range(GtkSheet *sheet, const GtkSheetRange *range);
static void gtk_sheet_extend_selection(GtkSheet *sheet, gint row, gint column);
static void gtk_sheet_new_selection(GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_draw_border(GtkSheet *sheet, GtkSheetRange range);
static void gtk_sheet_draw_corners(GtkSheet *sheet, GtkSheetRange range);

/* Active Cell handling */

static void gtk_sheet_entry_changed_handler(GtkWidget *widget, gpointer data);
static gboolean gtk_sheet_deactivate_cell(GtkSheet *sheet);
static void gtk_sheet_hide_active_cell(GtkSheet *sheet);
static gboolean gtk_sheet_activate_cell(GtkSheet *sheet, gint row, gint col);
static void gtk_sheet_draw_active_cell(GtkSheet *sheet);
static void gtk_sheet_show_active_cell(GtkSheet *sheet);
static void gtk_sheet_click_cell(GtkSheet *sheet, gint row, gint column, gboolean *veto);

/* Backing Pixmap */

static void gtk_sheet_make_backing_pixmap(GtkSheet *sheet, guint width, guint height);
static void gtk_sheet_draw_backing_pixmap(GtkSheet *sheet, GtkSheetRange range);
/* Scrollbars */

static void adjust_scrollbars(GtkSheet *sheet);
static void vadjustment_changed_handler(GtkAdjustment *adjustment, gpointer data);
static void hadjustment_changed_handler(GtkAdjustment *adjustment, gpointer data);
static void vadjustment_value_changed_handler(GtkAdjustment *adjustment, gpointer data);
static void hadjustment_value_changed_handler(GtkAdjustment *adjustment, gpointer data);

static void draw_xor_vline(GtkSheet *sheet);
static void draw_xor_hline(GtkSheet *sheet);
static void draw_xor_rectangle(GtkSheet *sheet, GtkSheetRange range);
static void gtk_sheet_draw_flashing_range(GtkSheet *sheet, GtkSheetRange range);
static guint new_column_width(GtkSheet *sheet, gint column, gint *x);
static guint new_row_height(GtkSheet *sheet, gint row, gint *y);

/* Sheet Button */

static void create_global_button(GtkSheet *sheet);
static void global_button_press_handler(GtkWidget *widget, gpointer data);
/* Sheet Entry */

static void create_sheet_entry(GtkSheet *sheet);
static void gtk_sheet_size_allocate_entry(GtkSheet *sheet);
static void gtk_sheet_entry_set_max_size(GtkSheet *sheet);

/* Sheet button gadgets */

static void size_allocate_column_title_buttons(GtkSheet * sheet);
static void size_allocate_row_title_buttons(GtkSheet * sheet);

static void gtk_sheet_recalc_top_ypixels(GtkSheet *sheet);
static void gtk_sheet_recalc_left_xpixels(GtkSheet *sheet);
static void gtk_sheet_recalc_text_column(GtkSheet *sheet, gint start_column);

static void row_button_set(GtkSheet *sheet, gint row);
static void column_button_set(GtkSheet *sheet, gint column);
static void row_button_release(GtkSheet *sheet, gint row);
static void column_button_release(GtkSheet *sheet, gint column);
static void gtk_sheet_button_draw(GtkSheet *sheet, gint row, gint column);
static void size_allocate_global_button(GtkSheet *sheet);
static void gtk_sheet_button_size_request(GtkSheet *sheet,
                                          GtkSheetButton *button, GtkRequisition *requisition);

/* Attributes routines */

static void gtk_sheet_set_cell_attributes(GtkSheet *sheet, 
                                          gint row, gint col, GtkSheetCellAttr attributes);
static void init_attributes(GtkSheet *sheet, gint col, GtkSheetCellAttr *attributes);

/* Memory allocation routines */
static void gtk_sheet_real_range_clear(GtkSheet *sheet, 
                                       const GtkSheetRange *range, gboolean delete);
static void gtk_sheet_real_cell_clear(GtkSheet *sheet, 
                                      gint row, gint column, gboolean delete);

static GtkSheetCell *gtk_sheet_cell_new(void);

static void AddRows (GtkSheet *sheet, gint position, gint nrows);
static void AddColumns (GtkSheet *sheet, gint position, gint ncols);
static void InsertRow (GtkSheet *sheet, gint row, gint nrows);
static void InsertColumn (GtkSheet *sheet, gint col, gint ncols);
static void DeleteRow (GtkSheet *sheet, gint row, gint nrows);
static void DeleteColumn (GtkSheet *sheet, gint col, gint ncols);
static gint GrowSheet (GtkSheet *sheet, gint newrows, gint newcols);
static void CheckBounds(GtkSheet *sheet, gint row, gint col);
static void CheckCellData(GtkSheet *sheet, const gint row, const gint col);

/* Container Functions */
static void gtk_sheet_remove_handler(GtkContainer *container, GtkWidget *widget);
static void gtk_sheet_realize_child(GtkSheet *sheet, GtkSheetChild *child);
static void gtk_sheet_position_child(GtkSheet *sheet, GtkSheetChild *child);
static void gtk_sheet_position_children(GtkSheet *sheet);
static void gtk_sheet_child_show(GtkSheetChild *child);
static void gtk_sheet_child_hide(GtkSheetChild *child);
static void gtk_sheet_column_size_request(GtkSheet *sheet, gint col, guint *requisition);
static void gtk_sheet_row_size_request(GtkSheet *sheet, gint row, guint *requisition);

/* GtkBuildableIface */

/*
 * gtk_sheet_buildable_add_child_internal
 * 
 * embed a foreign created #GtkSheetColumn into an existing #GtkSheet
 * Used by GtkBuilder and Glade.
 * 
 * @param sheet  The sheet where the column should be added
 * @param child  The column to be added
 * @param name   The #GtkWidget name of the column
 */

void
    gtk_sheet_buildable_add_child_internal(GtkSheet *sheet, 
                                           GtkSheetColumn *child, 
                                           const char *name)
{
    int col;

    g_return_if_fail(GTK_IS_SHEET(sheet));
    g_return_if_fail(GTK_IS_SHEET_COLUMN(child));

    gtk_sheet_add_column(sheet, 1);
    col = gtk_sheet_get_columns_count(sheet) - 1;

    if (sheet->column[col])
    {
        sheet->column[col]->sheet = NULL;

        g_object_unref(sheet->column[col]);
        sheet->column[col] = NULL;
    }

    child->sheet = sheet;
    sheet->column[col] = child;
    g_object_ref_sink(G_OBJECT(child));

    if (name) gtk_widget_set_name(GTK_WIDGET(child), name);

    gtk_sheet_recalc_text_column(sheet, col);
    gtk_sheet_recalc_left_xpixels(sheet);
}

static void
    gtk_sheet_buildable_add_child(
                                 GtkBuildable  *buildable,
                                 GtkBuilder    *builder,
                                 GObject       *child,
                                 const gchar   *type)
{
    GtkSheet *sheet;
    GtkSheetColumn *newcol;
    const gchar *name = gtk_widget_get_name(GTK_WIDGET(child));

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_buildable_add_child %p: %s type %s", child,
            name ? name : "NULL", 
            type ? type : "NULL" );
#endif

    sheet = GTK_SHEET(buildable);
    newcol = GTK_SHEET_COLUMN(child);

    {
        gchar *strval;

        g_object_get(G_OBJECT(newcol), "label", &strval, NULL);
#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_buildable_add_child: label=%s", strval ? strval : "NULL");
#endif

        g_free(strval);
    }

    gtk_sheet_buildable_add_child_internal(sheet, newcol, name);
}

static void
    gtk_sheet_buildable_init (GtkBuildableIface *iface)
{
#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_buildable_init");
#endif
    iface->add_child = gtk_sheet_buildable_add_child;
}

static GtkSheetArea 
    gtk_sheet_get_area_at(GtkSheet *sheet, gint x, gint y)
{
    if (sheet->column_titles_visible && (y < sheet->column_title_area.height))
    {
        if (sheet->row_titles_visible && (x < sheet->row_title_area.width))
            return(ON_SHEET_BUTTON_AREA);

        return(ON_COLUMN_TITLES_AREA);
    }
    else
    {
        if (sheet->row_titles_visible && (x < sheet->row_title_area.width))
            return(ON_ROW_TITLES_AREA);
    }
    return(ON_CELL_AREA);
}

/*
 * gtk_sheet_query_tooltip - tooltip handler
 * 
 * Preference rule for the tooltip search is cell->row->column->sheet
 * 
 * @param widget
 * @param x
 * @param y
 * @param keyboard_mode
 * @param tooltip
 * @param user_data
 * 
 * @return TRUE or FALSE wether to show the tip or not
 */
static gboolean
    gtk_sheet_query_tooltip_handler(GtkWidget *widget,
                            gint x, gint y, 
                            gboolean keyboard_mode,
                            GtkTooltip *tooltip,
                            gpointer user_data)
{
    gchar *tip;
    GtkSheetArea area;
    gint row = -1, col = -1;
    GtkSheet *sheet = GTK_SHEET(widget);

    if (!sheet) return(FALSE);

    area = gtk_sheet_get_area_at(sheet, x, y);

    if (area == ON_CELL_AREA)
    {
        if (row < 0) row = ROW_FROM_YPIXEL(sheet, y);
        if (col < 0) col = COLUMN_FROM_XPIXEL(sheet, x);

        if ((0 <= row && row <= sheet->maxrow && 0 <= col && col <= sheet->maxcol)
            && (row <= sheet->maxallocrow && col <= sheet->maxalloccol)
            && (sheet->data[row] && sheet->data[row][col]))
        {
            GtkSheetCell *cell = sheet->data[row][col];

            tip = cell->tooltip_markup;
            if (tip && tip[0])
            {
                gtk_tooltip_set_markup(tooltip, tip);
                return(TRUE);
            }

            tip = cell->tooltip_text;
            if (tip && tip[0])
            {
                gtk_tooltip_set_text(tooltip, tip);
                return(TRUE);
            }
        }

        area = ON_ROW_TITLES_AREA;  /* fallback */
    }

    if (area == ON_ROW_TITLES_AREA)
    {
        if (row < 0) row = ROW_FROM_YPIXEL(sheet, y);

        if (0 <= row && row <= sheet->maxrow)
        {
            GtkSheetRow *rowp = ROWPTR(sheet, row);

            tip = rowp->tooltip_markup;
            if (tip && tip[0])
            {
                gtk_tooltip_set_markup(tooltip, tip);
                return(TRUE);
            }

            tip = rowp->tooltip_text;
            if (tip && tip[0])
            {
                gtk_tooltip_set_text(tooltip, tip);
                return(TRUE);
            }
        }

        area = ON_COLUMN_TITLES_AREA;  /* fallback */
    }

    if (area == ON_COLUMN_TITLES_AREA)
    {
        if (col < 0) col = COLUMN_FROM_XPIXEL(sheet, x);

        if (0 <= col && col <= sheet->maxcol)
        {
            GtkSheetColumn *column = COLPTR(sheet, col);

            tip = gtk_widget_get_tooltip_markup(GTK_WIDGET(column));
            if (tip && tip[0])
            {
                gtk_tooltip_set_markup(tooltip, tip);
                g_free(tip);
                return(TRUE);
            }

            tip = gtk_widget_get_tooltip_text(GTK_WIDGET(column));
            if (tip && tip[0])
            {
                gtk_tooltip_set_text(tooltip, tip);
                g_free(tip);
                return(TRUE);
            }
        }
    }

    /* fallback to sheet tip */

    tip = gtk_widget_get_tooltip_markup(widget);
    if (tip && tip[0])
    {
        gtk_tooltip_set_markup(tooltip, tip);
        g_free(tip);
        return(TRUE);
    }

    tip = gtk_widget_get_tooltip_text(widget);
    if (tip && tip[0])
    {
        gtk_tooltip_set_text(tooltip, tip);
        g_free(tip);
        return(TRUE);
    }

    return(FALSE);
}

/* Type initialisation */

static GtkContainerClass *sheet_parent_class = NULL;
static GtkObjectClass *sheet_column_parent_class = NULL;

GType
    gtk_sheet_get_type (void)
{
    static GType sheet_type = 0;

    if (!sheet_type)
    {
        static const GTypeInfo sheet_info =
        {
            sizeof (GtkSheetClass),
            NULL,
            NULL,
            (GClassInitFunc) gtk_sheet_class_init,
            NULL,
            NULL,
            sizeof (GtkSheet),
            0,
            (GInstanceInitFunc) gtk_sheet_init,
            NULL,
        };

        static const GInterfaceInfo interface_info = {
            (GInterfaceInitFunc) gtk_sheet_buildable_init,
            (GInterfaceFinalizeFunc) NULL,
            (gpointer) NULL
        }; 

        sheet_type = g_type_register_static (gtk_container_get_type(),
                                             "GtkSheet",
                                             &sheet_info,
                                             0);

        g_type_add_interface_static (sheet_type, GTK_TYPE_BUILDABLE, 
                                     &interface_info);
    }
    return(sheet_type);
}

static void gtk_sheet_column_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);

static void
    gtk_sheet_column_set_buildable_property(GtkBuildable  *buildable,
                                            GtkBuilder    *builder,
                                            const gchar   *name,
                                            const GValue  *value)
{
#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_column_set_buildable_property: %s", name);
#endif

    if (strcmp(name, "visible") == 0)
    {
        GTK_SHEET_COLUMN_SET_VISIBLE(buildable, g_value_get_boolean(value));
    }
#if 0
    else if (strcmp(name, "width-request") == 0)
    {
    #ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_column_set_buildable_property: width-request = %d", 
                GTK_SHEET_COLUMN(buildable)->width);
    #endif
        GTK_SHEET_COLUMN(buildable)->width = g_value_get_int(value);
    }
#endif
    else
        g_object_set_property(G_OBJECT(buildable), name, value);
}


static void
    gtk_sheet_column_buildable_init (GtkBuildableIface *iface)
{
#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_column_buildable_init");
#endif
    iface->set_buildable_property = gtk_sheet_column_set_buildable_property;
}

GType
    gtk_sheet_column_get_type (void)
{
    static GType sheet_column_type = 0;

    if (!sheet_column_type)
    {
        static const GTypeInfo sheet_column_info =
        {
            sizeof (GtkSheetColumnClass),
            NULL,
            NULL,
            (GClassInitFunc) gtk_sheet_column_class_init,
            NULL,
            NULL,
            sizeof (GtkSheetColumn),
            0,
            (GInstanceInitFunc) gtk_sheet_column_init,
            NULL,
        };

        static const GInterfaceInfo interface_info = {
            (GInterfaceInitFunc) gtk_sheet_column_buildable_init,
            (GInterfaceFinalizeFunc) NULL,
            (gpointer) NULL
        };

        sheet_column_type = g_type_register_static (gtk_widget_get_type(),
                                                    "GtkSheetColumn",
                                                    &sheet_column_info,
                                                    0);

        g_type_add_interface_static (sheet_column_type, 
                                     GTK_TYPE_BUILDABLE, 
                                     &interface_info);
    }
    return(sheet_column_type);
}



static GtkSheetRange*
    gtk_sheet_range_copy (const GtkSheetRange *range)
{
    GtkSheetRange *new_range;

    g_return_val_if_fail (range != NULL, NULL);

    new_range = g_new (GtkSheetRange, 1);

    *new_range = *range;

    return(new_range);
}

static void
    gtk_sheet_range_free (GtkSheetRange *range)
{
    g_return_if_fail (range != NULL);

    g_free (range);
}

GType
    gtk_sheet_range_get_type (void)
{
    static GType sheet_range_type=0;

    if (!sheet_range_type)
    {
        sheet_range_type = g_boxed_type_register_static("GtkSheetRange", (GBoxedCopyFunc)gtk_sheet_range_copy, (GBoxedFreeFunc)gtk_sheet_range_free);
    }
    return(sheet_range_type);
}

static GtkSheetEntryType
    map_gtype_2_sheet_entry_type(GType entry_type)
{
    if (entry_type == G_TYPE_ITEM_ENTRY)
        return(GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY);

    else if (entry_type == GTK_TYPE_ENTRY)
        return(GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY);

    else if (entry_type == GTK_TYPE_TEXT_VIEW)
        return(GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW);

    else if (entry_type == GTK_TYPE_SPIN_BUTTON)
        return(GTK_SHEET_ENTRY_TYPE_GTK_SPIN_BUTTON);

    else if (entry_type == GTK_TYPE_COMBO_BOX)
        return(GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX);

    else if (entry_type == GTK_TYPE_COMBO_BOX_ENTRY)
        return(GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX_ENTRY);

    else if (entry_type == GTK_TYPE_COMBO)
        return(GTK_SHEET_ENTRY_TYPE_GTK_COMBO);

    return(GTK_SHEET_ENTRY_TYPE_DEFAULT);
}

static GType
    map_sheet_entry_type_2_gtype(GtkSheetEntryType ety)
{
    switch(ety)
    {
        case GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY:
            return(G_TYPE_ITEM_ENTRY);

        case GTK_SHEET_ENTRY_TYPE_GTK_ENTRY:
            return(GTK_TYPE_ENTRY);

        case GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW:
            return(GTK_TYPE_TEXT_VIEW);

        case GTK_SHEET_ENTRY_TYPE_GTK_SPIN_BUTTON:
            return(GTK_TYPE_SPIN_BUTTON);

        case GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX:
            return(GTK_TYPE_COMBO_BOX);

        case GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX_ENTRY:
            return(GTK_TYPE_COMBO_BOX_ENTRY);

        case GTK_SHEET_ENTRY_TYPE_GTK_COMBO:
            return(GTK_TYPE_COMBO);

        default: break;
    }
    return(G_TYPE_NONE);
}

/*
 * gtk_sheet_set_property - set sheet property
 * gtk_sheet_get_property - get sheet property
 * gtk_sheet_class_init_properties - initialize class properties
 * gtk_sheet_class_init_signals - initialize class signals
 */

static void 
    gtk_sheet_set_property (GObject *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GtkSheet *sheet = GTK_SHEET(object);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_set_property: %s", pspec->name);
#endif

    switch (property_id)
    {
        case PROP_GTK_SHEET_TITLE:
            gtk_sheet_set_title(sheet, g_value_get_string(value));
            break;

        case PROP_GTK_SHEET_NROWS:
            {
                gint newval = g_value_get_int(value);

                if (newval < 0) break;

#ifdef GTK_SHEET_DEBUG
                g_debug("gtk_sheet_set_property: newval = %d sheet->maxrow %d", newval, sheet->maxrow);
#endif
                if (newval < (sheet->maxrow+1))
                {
                    gtk_sheet_delete_rows(sheet, newval, (sheet->maxrow+1) - newval);
                    gtk_sheet_recalc_view_range(sheet);
                }
                else if (newval > (sheet->maxrow+1))
                {
                    gtk_sheet_add_row(sheet, newval - (sheet->maxrow+1));
                    gtk_sheet_recalc_view_range(sheet);
                }
            }
            break;

        case PROP_GTK_SHEET_NCOLS:
            {
                gint newval = g_value_get_int(value);

                if (newval < 0) break;
#ifdef GTK_SHEET_DEBUG
                g_debug("gtk_sheet_set_property: newval = %d sheet->maxcol %d", newval, sheet->maxcol);
#endif
                if (newval < (sheet->maxcol+1))
                {
                    gtk_sheet_delete_columns(sheet, newval, (sheet->maxcol+1) - newval);
                    gtk_sheet_recalc_view_range(sheet);
                }
                else if (newval > (sheet->maxcol+1))
                {
                    gtk_sheet_add_column(sheet, newval - (sheet->maxcol+1));
                    gtk_sheet_recalc_view_range(sheet);
                }
            }
            break;

        case PROP_GTK_SHEET_LOCKED:
            gtk_sheet_set_locked(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_SELECTION_MODE:
            gtk_sheet_set_selection_mode(sheet, g_value_get_enum(value));
            break;

        case PROP_GTK_SHEET_AUTO_RESIZE:
            gtk_sheet_set_autoresize(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_AUTO_SCROLL:
            gtk_sheet_set_autoscroll(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_CLIP_TEXT:
            gtk_sheet_set_clip_text(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_JUSTIFY_ENTRY:
            gtk_sheet_set_justify_entry(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_BG_COLOR:
            gtk_sheet_set_background(sheet, g_value_get_boxed (value));
            break;

        case PROP_GTK_SHEET_GRID_VISIBLE:
            gtk_sheet_show_grid(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_GRID_COLOR:
            gtk_sheet_set_grid(sheet, g_value_get_boxed (value));
            break;

        case PROP_GTK_SHEET_COLUMN_TITLES_VISIBLE:
            if (g_value_get_boolean(value))
                gtk_sheet_show_column_titles(sheet);
            else
                gtk_sheet_hide_column_titles(sheet);
            break;

        case PROP_GTK_SHEET_COLUMNS_RESIZABLE:
            gtk_sheet_columns_set_resizable(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_COLUMN_TITLES_HEIGHT:
            gtk_sheet_set_column_titles_height(sheet, g_value_get_uint (value));
            break;

        case PROP_GTK_SHEET_ROW_TITLES_VISIBLE:
            if (g_value_get_boolean(value))
                gtk_sheet_show_row_titles(sheet);
            else
                gtk_sheet_hide_row_titles(sheet);
            break;

        case PROP_GTK_SHEET_ROWS_RESIZABLE:
            gtk_sheet_rows_set_resizable(sheet, g_value_get_boolean(value));
            break;

        case PROP_GTK_SHEET_ROW_TITLES_WIDTH:
            gtk_sheet_set_row_titles_width(sheet, g_value_get_uint(value));
            break;

        case PROP_GTK_SHEET_ENTRY_TYPE:
            {
                GType entry_type = map_sheet_entry_type_2_gtype(g_value_get_enum(value));
                if (entry_type == G_TYPE_NONE) entry_type = 0;
                gtk_sheet_change_entry(sheet, entry_type);
            }
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
    gtk_sheet_range_draw (sheet, NULL);

    /* this one will not work, it simply does noop
   gtk_widget_queue_draw(GTK_WIDGET(sheet));*/
}

static void 
    gtk_sheet_get_property (GObject    *object,
                            guint       property_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    GtkSheet *sheet = GTK_SHEET(object);

    switch (property_id)
    {
        case PROP_GTK_SHEET_TITLE:
            g_value_set_string (value, sheet->title);
            break;

        case PROP_GTK_SHEET_NROWS:
            g_value_set_int (value, sheet->maxrow+1);
            break;

        case PROP_GTK_SHEET_NCOLS:
            g_value_set_int (value, sheet->maxcol+1);
            break;

        case PROP_GTK_SHEET_LOCKED:
            g_value_set_boolean (value, sheet->locked);
            break;

        case PROP_GTK_SHEET_SELECTION_MODE:
            g_value_set_enum (value, sheet->selection_mode);
            break;

        case PROP_GTK_SHEET_AUTO_RESIZE:
            g_value_set_boolean (value, sheet->autoresize);
            break;

        case PROP_GTK_SHEET_AUTO_SCROLL:
            g_value_set_boolean (value, sheet->autoscroll);
            break;

        case PROP_GTK_SHEET_CLIP_TEXT:
            g_value_set_boolean (value, sheet->clip_text);
            break;

        case PROP_GTK_SHEET_JUSTIFY_ENTRY:
            g_value_set_boolean (value, sheet->justify_entry);
            break;

        case PROP_GTK_SHEET_BG_COLOR:
            g_value_set_boxed (value, &sheet->bg_color);
            break;

        case PROP_GTK_SHEET_GRID_VISIBLE:
            g_value_set_boolean (value, sheet->show_grid);
            break;

        case PROP_GTK_SHEET_GRID_COLOR:
            g_value_set_boxed (value, &sheet->grid_color);
            break;

        case PROP_GTK_SHEET_COLUMN_TITLES_VISIBLE:
            g_value_set_boolean (value, sheet->column_titles_visible);
            break;

        case PROP_GTK_SHEET_COLUMNS_RESIZABLE:
            g_value_set_boolean (value, sheet->columns_resizable);
            break;

        case PROP_GTK_SHEET_COLUMN_TITLES_HEIGHT:
            g_value_set_uint (value, sheet->column_title_area.height);
            break;

        case PROP_GTK_SHEET_ROW_TITLES_VISIBLE:
            g_value_set_boolean (value, sheet->row_titles_visible);
            break;

        case PROP_GTK_SHEET_ROWS_RESIZABLE:
            g_value_set_boolean (value, sheet->rows_resizable);
            break;

        case PROP_GTK_SHEET_ROW_TITLES_WIDTH:
            g_value_set_uint (value, sheet->row_title_area.width);
            break;

        case PROP_GTK_SHEET_ENTRY_TYPE:
            g_value_set_enum(value, map_gtype_2_sheet_entry_type(sheet->entry_type));
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

static void gtk_sheet_class_init_properties(GObjectClass *gobject_class)
{
    GParamSpec *pspec;

    gobject_class->set_property = gtk_sheet_set_property;
    gobject_class->get_property = gtk_sheet_get_property;

    /**
     * GtkSheet:title:
     *
     * The sheets title string
     */
    pspec = g_param_spec_string ("title", "Sheet title",
                                 "The sheets title string",
                                 "GtkSheet" /* default value */,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_TITLE, pspec);

#if 0
    /**
     * GtkSheet:n-cols:
     *
     * Number of columns in the sheet
     */
    pspec = g_param_spec_int ("n-cols", "Number of columns",
                              "Number of columns in the sheet",
                              0, 1024, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_NCOLS, pspec);
#endif

    /**
     * GtkSheet:n-rows:
     *
     * Number of rows in the sheet
     */
    pspec = g_param_spec_int ("n-rows", "Number of rows",
                              "Number of rows in the sheet",
                              0, 1000000, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_NROWS, pspec);

    /**
     * GtkSheet:locked:
     *
     * If the sheet ist locked, it is not editable, cell contents
     * cannot be changed by the user.
     */
    pspec = g_param_spec_boolean ("locked", "Locked",
                                  "If the sheet is locked, it is not editable, cell contents cannot be changed by the user",
                                  FALSE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_LOCKED, pspec);

    /**
     * GtkSheet:selection-mode:
     *
     * Sets the selection mode of the cells in a #GtkSheet
     */
    pspec = g_param_spec_enum ("selection-mode", "Selection mode",
                               "Sets the selection mode of the cells in a sheet",
                               gtk_selection_mode_get_type(),
                               GTK_SELECTION_BROWSE,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_SELECTION_MODE, pspec);

    /**
     * GtkSheet:autoresize:
     *
     * Autoreisize cells while typing
     */
    pspec = g_param_spec_boolean ("autoresize", "Autoresize cells",
                                  "Autoreisize cells while typing",
                                  FALSE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_AUTO_RESIZE, pspec);

    /**
     * GtkSheet:autoscroll:
     *
     * The sheet will be automatically scrolled when you move beyond
     * the last visible row/column
     */
    pspec = g_param_spec_boolean ("autoscroll", "Autoscroll sheet",
                                  "The sheet will be automatically scrolled when you move beyond the last row/column",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_AUTO_SCROLL, pspec);

    /**
     * GtkSheet:clip-text:
     *
     * Clip text in cells
     */
    pspec = g_param_spec_boolean ("clip-text", "Clip cell text",
                                  "Clip text in cells",
                                  FALSE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_CLIP_TEXT, pspec);

    pspec = g_param_spec_boolean ("justify-entry", "Justify cell entry",
                                  "Adapt cell entry editor to the cell justification",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_JUSTIFY_ENTRY, pspec);

    /**
     * GtkSheet:bgcolor:
     *
     * Background color of the sheet
     */
    pspec = g_param_spec_boxed ("bgcolor", "Background color",
                                "Background color of the sheet",
                                GDK_TYPE_COLOR,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_BG_COLOR, pspec);

    /**
     * GtkSheet:grid-visible:
     *
     * Sets the visibility of grid
     */
    pspec = g_param_spec_boolean ("grid-visible", "Grid visible",
                                  "Sets the visibility of grid",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_GRID_VISIBLE, pspec);

    /**
     * GtkSheet:grid-color:
     *
     * Color of the grid
     */
    pspec = g_param_spec_boxed ("grid-color", "Grid color",
                                "Color of the grid",
                                GDK_TYPE_COLOR,
                                G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_GRID_COLOR, pspec);

    /**
     * GtkSheet:col-titles-visible:
     *
     * Visibility of the column titles
     */
    pspec = g_param_spec_boolean ("col-titles-visible", "Column titles visible",
                                  "Visibility of the column titles",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_COLUMN_TITLES_VISIBLE, pspec);

    /**
     * GtkSheet:columns-resizable:
     *
     * Columns resizable
     */
    pspec = g_param_spec_boolean ("columns-resizable", "Columns resizable",
                                  "Columns resizable",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_COLUMNS_RESIZABLE, pspec);

    /**
     * GtkSheet:col-titles-height:
     *
     * Height of the column titles
     */
    pspec = g_param_spec_uint ("col-titles-height", "Column titles height",
                               "Height of the column title area",
                               0, 1024, GTK_SHEET_DEFAULT_ROW_HEIGHT,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_COLUMN_TITLES_HEIGHT, pspec);

    /**
     * GtkSheet:row-titles-visible:
     *
     * Row titles visible
     */
    pspec = g_param_spec_boolean ("row-titles-visible", "Row titles visible",
                                  "Row titles visible",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_ROW_TITLES_VISIBLE, pspec);

    /**
     * GtkSheet:rows-resizable:
     *
     * Rows resizable
     */
    pspec = g_param_spec_boolean ("rows-resizable", "Rows resizable",
                                  "Rows resizable",
                                  TRUE,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_ROWS_RESIZABLE, pspec);

    /**
     * GtkSheet:row-titles-width:
     *
     * Width of the row title area
     */
    pspec = g_param_spec_uint ("row-titles-width", "Row titles width",
                               "Width of the row title area",
                               0, 2048, GTK_SHEET_DEFAULT_COLUMN_WIDTH,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, PROP_GTK_SHEET_ROW_TITLES_WIDTH, pspec);

    /**
     * GtkSheet:entry-type:
     *
     * Sheet cell entry widget type
     */
    pspec = g_param_spec_enum ("entry-type", "Entry Type",
                                  "Sheet entry type, if not default",
                                  gtk_sheet_entry_type_get_type(),
                                  GTK_SHEET_ENTRY_TYPE_DEFAULT,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_ENTRY_TYPE, pspec);
}

static void gtk_sheet_class_init_signals(GtkObjectClass *object_class, GtkWidgetClass *widget_class)
{
    /**
     * GtkSheet::select-row:
     * @sheet: the sheet widget that emitted the signal
     * @row: the newly selected row index
     *
     * Emmited when a row has been selected.
     */
    sheet_signals[SELECT_ROW] =
        g_signal_new ("select-row",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, select_row),
                      NULL, NULL,
                      gtkextra_VOID__INT,
                      G_TYPE_NONE, 1, G_TYPE_INT);

    /**
     * GtkSheet::select-column:
     * @sheet: the sheet widget that emitted the signal
     * @select_column: the newly selected column index
     *
     * Emmited when a column has been selected.
     */
    sheet_signals[SELECT_COLUMN] =
        g_signal_new ("select-column",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, select_column),
                      NULL, NULL,
                      gtkextra_VOID__INT,
                      G_TYPE_NONE, 1, G_TYPE_INT);

    /**
     * GtkSheet::select-range:
     * @sheet: the sheet widget that emitted the signal
     * @select_range: the newly selected #GtkSheetRange
     *
     * Emmited when a #GtkSheetRange has been selected.
     */
    sheet_signals[SELECT_RANGE] =
        g_signal_new ("select-range",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, select_range),
                      NULL, NULL,
                      gtkextra_VOID__BOXED,
                      G_TYPE_NONE, 1, G_TYPE_SHEET_RANGE);

    /**
     * GtkSheet::clip-range:
     * @sheet: the sheet widget that emitted the signal
     * @clip_range: the newly selected #GtkSheetRange
     *
     * Emmited when a #GtkSheetRange is clipping.
     */
    sheet_signals[CLIP_RANGE] =
        g_signal_new ("clip-range",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, clip_range),
                      NULL, NULL,
                      gtkextra_VOID__BOXED,
                      G_TYPE_NONE, 1, G_TYPE_SHEET_RANGE);

    /**
    * GtkSheet::resize-range:
    * @sheet: the sheet widget that emitted the signal
    * @resize_range: the newly selected #GtkSheetRange
    *
    * Emmited when a #GtkSheetRange is resized.
    */
    sheet_signals[RESIZE_RANGE] =
        g_signal_new ("resize-range",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, resize_range),
                      NULL, NULL,
                      gtkextra_VOID__BOXED_BOXED,
                      G_TYPE_NONE, 2, G_TYPE_SHEET_RANGE, G_TYPE_SHEET_RANGE);

    /**
     * GtkSheet::move-range:
     * @sheet: the sheet widget that emitted the signal.
     * @move_range: the newly selected #GtkSheetRange.
     *
     * Emmited when a #GtkSheetRange is moved.
     */
    sheet_signals[MOVE_RANGE] =
        g_signal_new ("move-range",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, move_range),
                      NULL, NULL,
                      gtkextra_VOID__BOXED_BOXED,
                      G_TYPE_NONE, 2, G_TYPE_SHEET_RANGE, G_TYPE_SHEET_RANGE);

    /**
     * GtkSheet::traverse:
     * @sheet: the sheet widget that emitted the signal.
     * @row: row number.
     * @column: column number.
     * @*new_row: FIXME:: What is this for?
     * @*new_column: FIXME:: What is this for?
     *
     * The "traverse" is emited before "deactivate_cell" and allows to veto the movement.
     * In such case, the entry will remain in the site and the other signals will not be emited.
     */
    sheet_signals[TRAVERSE] =
        g_signal_new ("traverse",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, traverse),
                      NULL, NULL,
                      gtkextra_BOOLEAN__INT_INT_POINTER_POINTER,
                      G_TYPE_BOOLEAN, 4, G_TYPE_INT, G_TYPE_INT,
                      G_TYPE_POINTER, G_TYPE_POINTER);

    /**
     * GtkSheet::deactivate:
     * @sheet: the sheet widget that emitted the signal
     * @row: row number of deactivated cell.
     * @column: column number of deactivated cell.
     *
     * Emmited whenever a cell is deactivated(you click on other cell or start a new selection)
     */
    sheet_signals[DEACTIVATE] =
        g_signal_new ("deactivate",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, deactivate),
                      NULL, NULL,
                      gtkextra_BOOLEAN__INT_INT,
                      G_TYPE_BOOLEAN, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::activate:
     * @sheet: the sheet widget that emitted the signal
     * @row: row number of activated cell.
     * @column: column number of activated cell.
     *
     * Emmited whenever a cell is activated(you click on it),
     */
    sheet_signals[ACTIVATE] =
        g_signal_new ("activate",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, activate),
                      NULL, NULL,
                      gtkextra_BOOLEAN__INT_INT,
                      G_TYPE_BOOLEAN, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::set-cell:
     * @sheet: the sheet widget that emitted the signal
     * @row: row number of activated cell.
     * @column: column number of activated cell.
     *
     * Emited when clicking on a non-empty cell.
     */
    sheet_signals[SET_CELL] =
        g_signal_new ("set-cell",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, set_cell),
                      NULL, NULL,
                      gtkextra_VOID__INT_INT,
                      G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::clear-cell:
     * @sheet: the sheet widget that emitted the signal
     * @row: row number of cleared cell.
     * @column: column number of cleared cell.
     *
     * Emited when when the content of the cell is erased.
     */
    sheet_signals[CLEAR_CELL] =
        g_signal_new ("clear-cell",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, clear_cell),
                      NULL, NULL,
                      gtkextra_VOID__INT_INT,
                      G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::changed:
     * @sheet: the sheet widget that emitted the signal
     * @row: row number of changed cell.
     * @column: column number of changed cell.
     *
     * "Emited when typing into the active cell, changing its content.
     * It is emitted after each key press in cell and after deactivating cell.
     */
    sheet_signals[CHANGED] =
        g_signal_new ("changed",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, changed),
                      NULL, NULL,
                      gtkextra_VOID__INT_INT,
                      G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::new-column-width:
     * @sheet: the sheet widget that emitted the signal
     * @row: modified row number.
     * @width: new column width
     *
     * Emited when the width of a column is modified.
     */
    sheet_signals[NEW_COL_WIDTH] =
        g_signal_new ("new-column-width",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, changed),
                      NULL, NULL,
                      gtkextra_VOID__INT_INT,
                      G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::new-row-height:
     * @sheet: the sheet widget that emitted the signal
     * @col: modified dolumn number.
     * @height: new row height.
     *
     * Emited when the height of a row is modified.
     */
    sheet_signals[NEW_ROW_HEIGHT] =
        g_signal_new ("new-row-height",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, changed),
                      NULL, NULL,
                      gtkextra_VOID__INT_INT,
                      G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::set-scroll-adjustments:
     * @sheet: the sheet widget that emitted the signal
     * @hadjustment: horizontal #GtkAdjustment.
     * @vadjustment: vertical #GtkAdkjustment.
     *
     * Emited when scroll adjustments are set.
     */
    widget_class->set_scroll_adjustments_signal =
        g_signal_new ("set-scroll-adjustments",
                      G_TYPE_FROM_CLASS(object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GtkSheetClass, set_scroll_adjustments),
                      NULL, NULL,
                      gtkextra_VOID__OBJECT_OBJECT,
                      G_TYPE_NONE, 2, gtk_adjustment_get_type (),
                      gtk_adjustment_get_type () );
}

/*
 * gtk_sheet_class_init - GtkSheet class initialisation
 *
 * @param klass
 */

static void
    gtk_sheet_class_init (GtkSheetClass * klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    object_class = (GtkObjectClass *) klass;
    widget_class = (GtkWidgetClass *) klass;
    container_class = (GtkContainerClass *) klass;

    sheet_parent_class = g_type_class_peek_parent (klass);

    gtk_sheet_class_init_signals(object_class, widget_class);

    container_class->add = NULL;
    container_class->remove = gtk_sheet_remove_handler;
    container_class->forall = gtk_sheet_forall_handler;

    object_class->destroy = gtk_sheet_destroy_handler;
    gobject_class->finalize = gtk_sheet_finalize_handler;

    gtk_sheet_class_init_properties(gobject_class);

    widget_class->realize = gtk_sheet_realize_handler;
    widget_class->unrealize = gtk_sheet_unrealize_handler;
    widget_class->map = gtk_sheet_map_handler;
    widget_class->unmap = gtk_sheet_unmap_handler;
    widget_class->style_set = gtk_sheet_style_set_handler;
    widget_class->button_press_event = gtk_sheet_button_press_handler;
    widget_class->button_release_event = gtk_sheet_button_release_handler;
    widget_class->motion_notify_event = gtk_sheet_motion_handler;
    widget_class->key_press_event = gtk_sheet_key_press_handler;
    widget_class->expose_event = gtk_sheet_expose_handler;
    widget_class->size_request = gtk_sheet_size_request_handler;
    widget_class->size_allocate = gtk_sheet_size_allocate_handler;
    widget_class->focus_in_event = NULL;
    widget_class->focus_out_event = NULL;

    klass->set_scroll_adjustments = gtk_sheet_set_scroll_adjustments;
    klass->select_row = NULL;
    klass->select_column = NULL;
    klass->select_range = NULL;
    klass->clip_range = NULL;
    klass->resize_range = NULL;
    klass->move_range = NULL;
    klass->traverse = NULL;
    klass->deactivate = NULL;
    klass->activate = NULL;
    klass->set_cell = NULL;
    klass->clear_cell = NULL;
    klass->changed = NULL;
}

static void
    gtk_sheet_init (GtkSheet *sheet)
{
    sheet->flags = 0;
    sheet->selection_mode = GTK_SELECTION_BROWSE;
    sheet->autoresize = FALSE;
    sheet->autoscroll = TRUE;
    sheet->clip_text = FALSE;
    sheet->justify_entry = TRUE;
    sheet->locked = FALSE;

    sheet->freeze_count = 0;

#ifdef GTK_SHEET_DEBUG
    gdk_color_parse(GTK_SHEET_DEBUG_COLOR, &debug_color);
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &debug_color, FALSE, TRUE);
#endif

    gdk_color_parse(GTK_SHEET_DEFAULT_BG_COLOR, &sheet->bg_color);
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->bg_color, FALSE, TRUE);

    gdk_color_parse(GTK_SHEET_DEFAULT_GRID_COLOR, &sheet->grid_color);
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->grid_color, FALSE, TRUE);
    sheet->show_grid = TRUE;

    gdk_color_parse(GTK_SHEET_DEFAULT_TM_COLOR, &sheet->tm_color);
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->tm_color, FALSE, TRUE);

    sheet->children = NULL;

    sheet->internal_allocation.x = 0;
    sheet->internal_allocation.y = 0;
    sheet->internal_allocation.width = 0;
    sheet->internal_allocation.height = 0;

    sheet->title = NULL;

    sheet->row = NULL;
    sheet->column = NULL;

    sheet->rows_resizable = TRUE;
    sheet->columns_resizable = TRUE;

    sheet->maxrow = -1;
    sheet->maxcol = -1;

    sheet->view.row0 = -1;
    sheet->view.col0 = -1;
    sheet->view.rowi = -1;
    sheet->view.coli = -1;

    sheet->data = NULL;

    sheet->maxallocrow = -1;
    sheet->maxalloccol = -1;

    sheet->active_cell.row = -1;
    sheet->active_cell.col = -1;

    sheet->sheet_entry = NULL;
    sheet->entry_type = 0;

    sheet->selection_cell.row = -1;
    sheet->selection_cell.col = -1;

    sheet->timer = 0;
    sheet->clip_timer = 0;
    sheet->interval = 0;

    sheet->button = NULL;
    sheet->state = GTK_SHEET_NORMAL;

    sheet->range.row0 = -1;
    sheet->range.rowi = -1;
    sheet->range.col0 = -1;
    sheet->range.coli = -1;

    sheet->sheet_window = NULL;
    sheet->sheet_window_width = 0;
    sheet->sheet_window_height = 0;

    sheet->pixmap=NULL;

    sheet->hoffset = 0;
    sheet->voffset = 0;

    sheet->old_hadjustment = 0;
    sheet->old_vadjustment = 0;

    sheet->hadjustment = NULL;
    sheet->vadjustment = NULL;

    sheet->shadow_type =   GTK_SHADOW_NONE;

    sheet->column_title_area.x=0;
    sheet->column_title_area.y=0;
    sheet->column_title_area.width=0;
    sheet->column_title_area.height=DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet));
    sheet->column_title_window=NULL;
    sheet->column_titles_visible = TRUE;

    sheet->row_title_window=NULL;
    sheet->row_title_area.x=0;
    sheet->row_title_area.y=0;
    sheet->row_title_area.width=GTK_SHEET_DEFAULT_COLUMN_WIDTH;
    sheet->row_title_area.height=0;
    sheet->row_titles_visible = TRUE;

    sheet->xor_gc = NULL;
    sheet->fg_gc = NULL;
    sheet->bg_gc = NULL;

    sheet->cursor_drag = gdk_cursor_new(GDK_PLUS);
    sheet->x_drag = 0;
    sheet->y_drag = 0;

    /* uninitialized
    sheet->drag_cell;
    sheet->drag_range;
    sheet->clip_range;
       */

    GTK_WIDGET_UNSET_FLAGS (sheet, GTK_NO_WINDOW);
    GTK_WIDGET_SET_FLAGS (sheet, GTK_CAN_FOCUS);

    /* for glade to be able to construct the object, we need to complete initialisation here */
    gtk_sheet_construct(sheet, 0, 0, "GtkSheet");

    g_signal_connect(G_OBJECT(sheet),
                     "query-tooltip",
                     (void *) gtk_sheet_query_tooltip_handler,
                     NULL);

    /* make sure the tooltip signal fires even if the sheet itself has no tooltip */
    gtk_widget_set_has_tooltip(GTK_WIDGET(sheet), TRUE);
}

/*
 * gtk_sheet_column_find
 * 
 * find index of @colobj in GtkSheet
 * 
 * @param colobj
 * 
 * @return column index or -1
 */
static gint 
    gtk_sheet_column_find(GtkSheetColumn *colobj)
{
    GtkSheet *sheet = colobj->sheet;
    int i;

    if (!sheet) return(-1);

    for (i=0; i<=sheet->maxcol; i++)
    {
        if (sheet->column[i] == colobj) return(i);
    }
    return(-1);
}

static void 
    gtk_sheet_column_set_property (GObject *object,
                                   guint         property_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
    GtkSheetColumn *colobj = GTK_SHEET_COLUMN(object);
    GtkSheet *sheet = colobj->sheet;
    gint col = gtk_sheet_column_find(colobj);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_column_set_property: %s called (%d)", pspec->name, property_id);
#endif

    /* note: glade/gtkbuilder will set column properties before the column gets
       added to the sheet and before the sheet gets realized and mapped.
       if the column was not yet added (col < 0), we cannot use public interface functions.
       */

    switch (property_id)
    {
        case PROP_GTK_SHEET_COLUMN_POSITION:
            {
                GtkSheetColumn *swapcol;
                uint newcol = g_value_get_int(value);

                if (!sheet) return;
                if (newcol < 0 || newcol > sheet->maxcol) return;

                if (col < 0) return;
                if (newcol == col) return;

#ifdef GTK_SHEET_DEBUG
                g_debug("gtk_sheet_column_set_property: swapping column %d/%d", col, newcol);
#endif

                /* method: swap */
                swapcol = sheet->column[newcol];
                sheet->column[newcol] = sheet->column[col];
                sheet->column[col] = swapcol;

                /* todo: swap cell data! */

                gtk_sheet_recalc_text_column(sheet, MIN(col, newcol));
                gtk_sheet_recalc_left_xpixels(sheet);
            }
            break;

        case PROP_GTK_SHEET_COLUMN_LABEL:
            {
                const gchar *label = g_value_get_string(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    GtkSheetButton *button = &colobj->button;
                    if (button->label) g_free (button->label);
                    button->label = g_strdup (label);
                }
                else
                {
                    gtk_sheet_column_button_add_label(sheet, col, label);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_WIDTH:
            {
                gint width = g_value_get_int(value);

                if (width < 0) return;
                if (width < COLUMN_MIN_WIDTH) width = GTK_SHEET_DEFAULT_COLUMN_WIDTH;

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->width = width;
                }
                else
                {
                    gtk_sheet_set_column_width(sheet, col, width);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_JUSTIFICATION:
            {
                gint justification = g_value_get_enum(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->justification = justification;
                }
                else
                {
                    gtk_sheet_column_set_justification(sheet, col, justification);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_ISKEY:
            {
                gint is_key = g_value_get_boolean(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->is_key = is_key;
                }
                else
                {
                    gtk_sheet_column_set_iskey(sheet, col, is_key);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_READONLY:
            {
                gint is_readonly = g_value_get_boolean(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->is_readonly = is_readonly;
                }
                else
                {
                    gtk_sheet_column_set_readonly(sheet, col, is_readonly);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_DATAFMT:
            {
                const gchar *data_format = g_value_get_string(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    if (colobj->data_format) g_free(colobj->data_format);
                    colobj->data_format = g_strdup(data_format);
                }
                else
                {
                    gtk_sheet_column_set_format(sheet, col, data_format);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_DATATYPE:
            {
                GtkSheetDataType data_type = g_value_get_enum(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->data_type = data_type;
                }
                else
                {
                    gtk_sheet_column_set_datatype(sheet, col, data_type);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_DESCRIPTION:
            {
                const gchar *description = g_value_get_string(value);

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    if (colobj->description) g_free(colobj->description);
                    colobj->description = g_strdup(description);
                }
                else
                {
                    gtk_sheet_column_set_description(sheet, col, description);
                }
            }
            break;

        case PROP_GTK_SHEET_COLUMN_ENTRY_TYPE:
            {
                GType entry_type = map_sheet_entry_type_2_gtype(g_value_get_enum(value));

                if ((col < 0) || !gtk_widget_get_realized(GTK_WIDGET(sheet)))
                {
                    colobj->entry_type = entry_type;
                }
                else
                {
                    gtk_sheet_column_set_entry_type(sheet, col, entry_type);
                }
            }
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }

    if (sheet && gtk_widget_get_realized(GTK_WIDGET(sheet)) 
        && !GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_range_draw (sheet, NULL);
    }
}

static void 
    gtk_sheet_column_get_property(GObject *object,
                                   guint property_id,
                                   GValue *value,
                                   GParamSpec *pspec)
{
    GtkSheetColumn *colobj = GTK_SHEET_COLUMN(object);
    GtkSheet *sheet = colobj->sheet;
    gint col = gtk_sheet_column_find(colobj);

    switch (property_id)
    {
        case PROP_GTK_SHEET_COLUMN_POSITION:
            {
                if (!sheet) return;
                if (col >= 0) g_value_set_int (value, col);
            }
            break;

        case PROP_GTK_SHEET_COLUMN_LABEL:
            g_value_set_string(value, colobj->button.label);
            break;

        case PROP_GTK_SHEET_COLUMN_WIDTH:
            g_value_set_int(value, colobj->width);
            break;

        case PROP_GTK_SHEET_COLUMN_JUSTIFICATION:
            g_value_set_enum(value, colobj->justification);
            break;

        case PROP_GTK_SHEET_COLUMN_ISKEY:
            g_value_set_boolean(value, colobj->is_key);
            break;

        case PROP_GTK_SHEET_COLUMN_READONLY:
            g_value_set_boolean(value, colobj->is_readonly);
            break;

        case PROP_GTK_SHEET_COLUMN_DATAFMT:
            g_value_set_string(value, colobj->data_format);
            break;

        case PROP_GTK_SHEET_COLUMN_DATATYPE:
            g_value_set_enum(value, colobj->data_type);
            break;

        case PROP_GTK_SHEET_COLUMN_DESCRIPTION:
            g_value_set_string(value, colobj->description);
            break;

        case PROP_GTK_SHEET_COLUMN_ENTRY_TYPE:
            {
                GtkSheetEntryType et = map_gtype_2_sheet_entry_type(colobj->entry_type);
                g_value_set_enum(value, et);
            }
            break;

        default:
            /* We don't have any other property... */
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
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
    pspec = g_param_spec_int ("position", "Position",
                              "Packing position",
                              0, 1024, 0,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_POSITION, pspec);

    /**
     * GtkSheetColumn:label:
     *
     * Label of the column button
     */
    pspec = g_param_spec_string ("label", "Column Button Label",
                                 "Label of the column button",
                                 "" /* default value */,
                                 G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_LABEL, pspec);

    /**
     * GtkSheetColumn:width:
     *
     * Width of the column
     */
    pspec = g_param_spec_int ("width", "Width",
                              "Width of the column",
                              -1, 8192, -1,
                              G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_WIDTH, pspec);

    /**
     * GtkSheetColumn:justification:
     *
     * Justification of the column
     */
    pspec = g_param_spec_enum ("justification", "Justification",
                               "Column justification (GTK_JUSTIFY_LEFT, RIGHT, CENTER)",
                               GTK_TYPE_JUSTIFICATION,
                               GTK_SHEET_DEFAULT_COLUMN_JUSTIFICATION,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_JUSTIFICATION, pspec);

    /**
     * GtkSheetColumn:iskey:
     *
     * Flag for key columns
     */
    pspec = g_param_spec_boolean ("iskey", "Key column",
                               "Wether this is a key column",
                               FALSE,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_ISKEY, pspec);

    /**
     * GtkSheetColumn:readonly:
     *
     * Lock column contents for editing.
     */
    pspec = g_param_spec_boolean ("readonly", "Readonly",
                               "Column contents are locked for editing",
                               FALSE,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_READONLY, pspec);

    /**
     * GtkSheetColumn:dataformat:
     *
     * Format pattern for cell contents
     */
    pspec = g_param_spec_string ("dataformat", "Format",
                               "Formatting pattern for cell contents",
                               "",
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_DATAFMT, pspec);

    /**
     * GtkSheetColumn:datatype:
     *
     * Data type for cell content validation
     */
    pspec = g_param_spec_enum ("datatype", "Data type",
                               "Data type for cell content validation",
                               gtk_sheet_data_type_get_type(),
                               GTK_SHEET_DATA_TYPE_NONE,
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_DATATYPE, pspec);

    /**
     * GtkSheetColumn:description:
     *
     * Description of column contents
     */
    pspec = g_param_spec_string ("description", "Description",
                               "Description of column contents",
                               "",
                               G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_DESCRIPTION, pspec);

    /**
     * GtkSheetColumn:entry-type:
     *
     * Column cell entry widget type
     */
    pspec = g_param_spec_enum ("entry-type", "Entry Type",
                                  "Supersedes sheet entry type, if not default",
                                  gtk_sheet_entry_type_get_type(),
                                  GTK_SHEET_ENTRY_TYPE_DEFAULT,
                                  G_PARAM_READWRITE);
    g_object_class_install_property (gobject_class, 
                                     PROP_GTK_SHEET_COLUMN_ENTRY_TYPE, pspec);
}

static void
    gtk_sheet_column_init (GtkSheetColumn *column)
{
    column->sheet = NULL;
    column->title = NULL;
    column->width = GTK_SHEET_DEFAULT_COLUMN_WIDTH;
    column->left_xpixel = 0;
    column->requisition = GTK_SHEET_DEFAULT_COLUMN_WIDTH;

    column->button.state = GTK_STATE_NORMAL;
    column->button.label = NULL;
    column->button.label_visible = TRUE;
    column->button.child = NULL;
    column->button.justification = GTK_JUSTIFY_CENTER;

    column->left_text_column = column->right_text_column = 0;

    column->justification = GTK_SHEET_DEFAULT_COLUMN_JUSTIFICATION;

    column->is_key = FALSE;
    column->is_readonly = FALSE;
    column->data_format = NULL;
    column->data_type = GTK_SHEET_DATA_TYPE_NONE;
    column->description = NULL;
    column->entry_type = GTK_SHEET_ENTRY_TYPE_DEFAULT;

    GTK_SHEET_COLUMN_SET_VISIBLE(column, TRUE);
    GTK_SHEET_COLUMN_SET_SENSITIVE(column, TRUE);
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
    GtkSheetColumn *column = GTK_SHEET_COLUMN(gobject);

    if (column->title)
    {
        g_free(column->title);
        column->title = NULL;
    }

    if (column->button.label)
    {
        g_free(column->button.label);
        column->button.label = NULL;
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

    G_OBJECT_CLASS (sheet_column_parent_class)->finalize(gobject);
}

static void
    gtk_sheet_column_class_init (GtkSheetColumnClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    sheet_column_parent_class = g_type_class_peek_parent (klass);

    gobject_class->finalize = gtk_sheet_column_finalize_handler;

    gtk_sheet_column_class_init_properties(gobject_class);
}


static void
    gtk_sheet_row_init(GtkSheetRow *row)
{
    row->name = NULL;
    row->height = GTK_SHEET_DEFAULT_ROW_HEIGHT;
    row->requisition = GTK_SHEET_DEFAULT_ROW_HEIGHT;
    row->top_ypixel = 0;

    row->button.state = GTK_STATE_NORMAL;
    row->button.label = NULL;
    row->button.label_visible = TRUE;
    row->button.child = NULL;
    row->button.justification = GTK_JUSTIFY_CENTER;

    row->tooltip_markup = NULL;
    row->tooltip_text = NULL;

    GTK_SHEET_ROW_SET_VISIBLE(row, TRUE);
    GTK_SHEET_ROW_SET_SENSITIVE(row, TRUE);
}

static void
    gtk_sheet_row_finalize(GtkSheetRow *row)
{
    if (row->name)
    {
        g_free(row->name);
        row->name = NULL;
    }

    if (row->button.label)
    {
        g_free(row->button.label);
        row->button.label = NULL;
    }

    if (row->tooltip_markup)
    {
        g_free(row->tooltip_markup);
        row->tooltip_markup = NULL;
    }

    if (row->tooltip_text)
    {
        g_free(row->tooltip_text);
        row->tooltip_text = NULL;
    }
}

/**
 * gtk_sheet_new:
 * @rows: initial number of rows
 * @columns: initial number of columns
 * @title: sheet title
 *
 * Creates a new sheet widget with the given number of rows and columns.
 *
 * Returns: the new sheet #GtkSheet
 */
GtkWidget *
    gtk_sheet_new (guint rows, guint columns, const gchar *title)
{
    GtkWidget *widget;

    /* sanity check */
    g_return_val_if_fail (columns >= MINCOLS, NULL);
    g_return_val_if_fail (rows >= MINROWS, NULL);

    widget = gtk_widget_new (gtk_sheet_get_type (), NULL);

    gtk_sheet_construct(GTK_SHEET(widget), rows, columns, title);

    return(widget);
}

/**
 * gtk_sheet_construct:
 * @sheet: a #GtkSheet
 * @rows: number of rows
 * @columns: number of columns
 * @title: sheet title
 *
 * Initializes an existent #GtkSheet with the given number of rows and columns.
 */
void
    gtk_sheet_construct (GtkSheet *sheet, guint rows, guint columns, const gchar *title)
{
    sheet->data=(GtkSheetCell ***)g_malloc(sizeof(GtkSheetCell **));

    sheet->data[0] = (GtkSheetCell **)g_malloc(sizeof(GtkSheetCell *)+sizeof(gdouble));
    sheet->data[0][0] = NULL;

    /* set number of rows and columns */
    GrowSheet(sheet, MINROWS, MINCOLS);

    /* Init heading row/column, add normal rows/columns */
    AddRows(sheet, sheet->maxrow+1, rows);
    AddColumns(sheet, sheet->maxcol+1, columns);

    /* create sheet entry */
    sheet->entry_type = 0;
    create_sheet_entry (sheet);

    /* create global selection button */
    create_global_button(sheet);

    if (title)
    {
        if (sheet->title) g_free(sheet->title);
        sheet->title = g_strdup(title);
    }
}

/**
 * gtk_sheet_new_browser:
 * @rows: initial number of rows
 * @columns: initial number of columns
 * @title: sheet title
 *
 * Creates a new browser sheet. Its cells cannot be edited(read-only).
 *
 * Returns: the new read-only #GtkSheet
 */
GtkWidget *
    gtk_sheet_new_browser(guint rows, guint columns, const gchar *title)
{
    GtkWidget *widget;

    widget = gtk_widget_new (gtk_sheet_get_type (), NULL);

    gtk_sheet_construct_browser(GTK_SHEET(widget), rows, columns, title);

    return(widget);
}

/**
 * gtk_sheet_construct_browser:
 * @sheet: a #GtkSheet
 * @rows: number of rows
 * @columns: number of columns
 * @title: sheet title
 *
 * Initializes an existent read-only #GtkSheet with the given number of rows and columns.
 */
void
    gtk_sheet_construct_browser(GtkSheet *sheet, guint rows, guint columns,
                                const gchar *title)
{
    gtk_sheet_construct(sheet, rows, columns, title);

    gtk_sheet_set_locked(sheet, TRUE);
    sheet->autoresize = TRUE;
}

/**
 * gtk_sheet_new_with_custom_entry:
 * @rows: initial number of rows
 * @columns: initial number of columns
 * @title: sheet title
 * @entry_type: a #GType
 *
 * Creates a new sheet widget with the given number of rows and columns and a custome entry type.
 *
 * Returns: the new sheet #GtkSheet
 */
GtkWidget *
    gtk_sheet_new_with_custom_entry (guint rows, guint columns, const gchar *title,
                                     GType entry_type)
{
    GtkWidget *widget;

    widget = gtk_widget_new (gtk_sheet_get_type (), NULL);

    gtk_sheet_construct_with_custom_entry(GTK_SHEET(widget),
                                          rows, columns, title, entry_type);

    return(widget);
}

/**
 * gtk_sheet_construct_with_custom_entry:
 * @sheet: a #GtkSheet
 * @rows: number of rows
 * @columns: number of columns
 * @title: sheet title
 * @entry_type: a #GType
 *
 * Initializes an existent read-only #GtkSheet 
 * with the given number of rows and columns and a custom entry.
 */
void
    gtk_sheet_construct_with_custom_entry (GtkSheet *sheet,
                                           guint rows, guint columns,
                                           const gchar *title,
                                           GType entry_type)
{
    gtk_sheet_construct(sheet, rows, columns, title);

    sheet->entry_type = entry_type;
    create_sheet_entry(sheet);
}

/**
 * gtk_sheet_change_entry:
 * @sheet: a #GtkSheet
 * @entry_type: a #GType
 *
 * Changes the current entry of the cell in #GtkSheet.
 */
void
    gtk_sheet_change_entry(GtkSheet *sheet, GType entry_type)
{
    gint state;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    state = sheet->state;

    if (state == GTK_SHEET_NORMAL)
        gtk_sheet_hide_active_cell(sheet);

    sheet->entry_type = entry_type;
    create_sheet_entry(sheet);

    if (state == GTK_SHEET_NORMAL)
    {
        gtk_sheet_show_active_cell(sheet);

        gtk_sheet_entry_signal_connect_changed(sheet, 
                                               (GtkSignalFunc) gtk_sheet_entry_changed_handler);
    }
}

/**
 * gtk_sheet_show_grid:
 * @sheet: a #GtkSheet
 * @show : TRUE(grid visible) or FALSE(grid invisible)
 *
 * Sets the visibility of grid in #GtkSheet.
 */
void
    gtk_sheet_show_grid(GtkSheet *sheet, gboolean show)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (show == sheet->show_grid) return;

    sheet->show_grid = show;

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, NULL);
}

/**
 * gtk_sheet_grid_visible:
 * @sheet: a #GtkSheet
 *
 * Gets the visibility of grid in #GtkSheet.
 *
 * Returns: TRUE(grid visible) or FALSE(grid invisible)
 */
gboolean
    gtk_sheet_grid_visible(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    return(sheet->show_grid);
}

/**
 * gtk_sheet_set_background:
 * @sheet: a #GtkSheet
 * @color: a #GdkColor structure
 *
 * Sets the background color of the #GtkSheet.
 * If pass NULL, the sheet will be reset to the default color.
 */
void
    gtk_sheet_set_background(GtkSheet *sheet, GdkColor *color)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!color)
    {
        gdk_color_parse(GTK_SHEET_DEFAULT_BG_COLOR, &sheet->bg_color);
    }
    else
    {
        sheet->bg_color = *color;
    }
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->bg_color, FALSE, TRUE);

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, NULL);
}

/**
 * gtk_sheet_set_grid:
 * @sheet: a #GtkSheet
 * @color: a #GdkColor structure
 *
 * Set the grid color.
 * If pass NULL, the grid will be reset to the default color.
 */
void
    gtk_sheet_set_grid(GtkSheet *sheet, GdkColor *color)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!color)
    {
        gdk_color_parse(GTK_SHEET_DEFAULT_GRID_COLOR, &sheet->grid_color);
    }
    else
    {
        sheet->grid_color = *color;
    }
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->grid_color, FALSE, TRUE);

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, NULL);
}

/**
 * gtk_sheet_get_columns_count:
 * @sheet: a #GtkSheet
 *
 * Get the number of the columns of the #GtkSheet.
 *
 * Returns: number of columns.
 */
guint
    gtk_sheet_get_columns_count(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    return(sheet->maxcol + 1);
}

/**
 * gtk_sheet_get_rows_count:
 * @sheet: a #GtkSheet
 *
 * Get the number of the rows of the #GtkSheet.
 *
 * Returns: number of rows.
 */
guint
    gtk_sheet_get_rows_count(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    return(sheet->maxrow + 1);
}

/**
 * gtk_sheet_get_state:
 * @sheet: a #GtkSheet
 *
 * Get the selection state of the sheet (#GtkSheetState).
 *
 * Returns:
 * GTK_SHEET_NORMAL,GTK_SHEET_ROW_SELECTED,GTK_SHEET_COLUMN_SELECTED,GTK_SHEET_RANGE_SELECTED
 */
GtkSheetState
    gtk_sheet_get_state(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    return(sheet->state);
}

/**
 * gtk_sheet_set_selection_mode:
 * @sheet: a #GtkSheet
 * @mode: GTK_SELECTION_SINGLE or GTK_SELECTION_BROWSE
 *
 * Sets the selection mode of the cells in a #GtkSheet.
 */
void
    gtk_sheet_set_selection_mode(GtkSheet *sheet, GtkSelectionMode mode)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
        gtk_sheet_real_unselect_range(sheet, NULL);

    sheet->selection_mode = mode;
}

/**
 * gtk_sheet_set_autoresize:
 * @sheet: a #GtkSheet
 * @autoresize: TRUE or FALSE
 *
 * The cells will be autoresized as you type text if autoresize=TRUE.
 * If you want the cells to be autoresized when you pack widgets look at gtk_sheet_attach_*()
 */
void
    gtk_sheet_set_autoresize (GtkSheet *sheet, gboolean autoresize)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->autoresize = autoresize;
}

/**
 * gtk_sheet_autoresize:
 * @sheet: a #GtkSheet
 *
 * Gets the autoresize mode of #GtkSheet.
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_autoresize (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->autoresize);
}

static void
    gtk_sheet_autoresize_column (GtkSheet *sheet, gint column)
{
    gint text_width = 0;
    gint row;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column > sheet->maxalloccol || column > sheet->maxcol) return;

    for (row = 0; row <= sheet->maxallocrow; row++)
    {
        GtkSheetCell **cell = &sheet->data[row][column];

        if (*cell && (*cell)->text && (*cell)->text[0])
        {
            GtkSheetCellAttr attributes;

            gtk_sheet_get_attributes(sheet, row, column, &attributes);

            if (attributes.is_visible)
            {
                gint width = STRING_WIDTH(GTK_WIDGET(sheet),
                                          attributes.font_desc,
                                          (*cell)->text)
                + 2*CELLOFFSET + attributes.border.width;
                text_width = MAX (text_width, width);
            }
        }
    }

    if (text_width != sheet->column[column]->width)
    {
        gtk_sheet_set_column_width(sheet, column, text_width);
        GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_REDRAW_PENDING);
    }
}

/**
 * gtk_sheet_set_autoscroll:
 * @sheet: a #GtkSheet
 * @autoscroll: TRUE or FALSE
 *
 * The sheet will be automatically scrolled when you move beyond
 * the last row/column in #GtkSheet.
 */
void
    gtk_sheet_set_autoscroll (GtkSheet *sheet, gboolean autoscroll)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->autoscroll = autoscroll;
}

/**
 * gtk_sheet_autoscroll:
 * @sheet: a #GtkSheet
 *
 * Get the autoscroll mode of #GtkSheet.
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_autoscroll (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->autoscroll);
}

/**
 * gtk_sheet_set_clip_text:
 * @sheet: a #GtkSheet
 * @clip_text: TRUE or FALSE
 *
 * Clip text in cell.
 */
void
    gtk_sheet_set_clip_text  (GtkSheet *sheet, gboolean clip_text)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->clip_text = clip_text;
}

/**
 * gtk_sheet_clip_text:
 * @sheet: a #GtkSheet
 *
 * Get clip text mode in #GtkSheet.
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_clip_text (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->clip_text);
}

/**
 * gtk_sheet_set_justify_entry:
 * @sheet: a #GtkSheet
 * @justify: TRUE or FALSE
 *
 * Justify cell entry editor in #GtkSheet.
 */
void
    gtk_sheet_set_justify_entry (GtkSheet *sheet, gboolean justify)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->justify_entry = justify;
}

/**
 * gtk_sheet_justify_entry:
 * @sheet: a #GtkSheet
 *
 * Get the cell entry editor justification setting from 
 * #GtkSheet. 
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_justify_entry (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->justify_entry);
}

/**
 * gtk_sheet_set_locked:
 * @sheet: a #GtkSheet
 * @locked: TRUE or FALSE
 *
 * Lock the #GtkSheet, which means it is no longer editable,
 * cell contents cannot be changed by the user.
                                                                */
void
    gtk_sheet_set_locked (GtkSheet *sheet, gboolean locked)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->locked = locked;
}

/**
 * gtk_sheet_locked:
 * @sheet: a #GtkSheet
 *
 * Get the lock status of #GtkSheet, locked means the sheet is
 * not editable, cell contents cannot be changed by the user.
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_locked (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->locked);
}

/* This routine has problems with gtk+-1.2 related with the
 * label/button drawing - I think it's a bug in gtk+-1.2 */

/**
 * gtk_sheet_set_title:
 * @sheet: a #GtkSheet
 * @title: #GtkSheet title
 *
 * Set  #GtkSheet title. The widget will keep a copy of the
 * string.
 */
void
    gtk_sheet_set_title(GtkSheet *sheet, const gchar *title)
{
/*  GtkWidget *old_widget;
*/  
    GtkWidget *label;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (title != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (sheet->title) g_free (sheet->title);
    sheet->title = g_strdup (title);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)) || !title) return;

    if (gtk_bin_get_child(GTK_BIN(sheet->button)))
        label = gtk_bin_get_child(GTK_BIN(sheet->button));
/*
  gtk_label_set_text(GTK_LABEL(label), title);
*/
    size_allocate_global_button(sheet);

    /* remove and destroy the old widget */
/*
  old_widget = gtk_bin_get_child(GTK_BIN (sheet->button));
  if (old_widget)
    {
      gtk_container_remove (GTK_CONTAINER (sheet->button), old_widget);
    }

  label = gtk_label_new (title);
  gtk_misc_set_alignment(GTK_MISC(label), 0.5 , 0.5 );

  gtk_container_add (GTK_CONTAINER (sheet->button), label);
  gtk_widget_show (label);

  size_allocate_global_button(sheet);

  g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, -1, -1);

  if(old_widget)
      gtk_widget_destroy (old_widget);
*/
}

/**
 * gtk_sheet_freeze:
 * @sheet: a #GtkSheet
 *
 * Freeze all visual updates of the #GtkSheet.
 * The updates will occure in a more efficient way than if you made them on a unfrozen #GtkSheet .
 */
void
    gtk_sheet_freeze (GtkSheet *sheet)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->freeze_count++;
    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);
}

/**
 * gtk_sheet_thaw:
 * @sheet: a #GtkSheet
 *
 * Thaw the sheet after you have made a number of changes on a frozen sheet.
 * The updates will occure in a more efficient way than if you made them on a unfrozen sheet .
 */
void
    gtk_sheet_thaw(GtkSheet *sheet)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (sheet->freeze_count == 0) return;

    sheet->freeze_count--;
    if (sheet->freeze_count > 0) return;

    adjust_scrollbars(sheet);

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    sheet->old_vadjustment = -1.;
    sheet->old_hadjustment = -1.;

    if (sheet->hadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
    }

    if (sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }

    if (sheet->state == GTK_STATE_NORMAL)
    {
        if (sheet->sheet_entry && gtk_widget_get_mapped(sheet->sheet_entry))
        {
            gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
            /*
                gtk_sheet_entry_signal_connect_changed(sheet, gtk_sheet_entry_changed_handler);
                gtk_sheet_show_active_cell(sheet);
            */
        }
    }
}

/**
 * gtk_sheet_set_row_titles_width:
 * @sheet: a #GtkSheet
 * @width: row titles width.
 *
 * Resize row titles area.
 */
void
    gtk_sheet_set_row_titles_width(GtkSheet *sheet, guint width)
{
    if (width < COLUMN_MIN_WIDTH) return;

    sheet->row_title_area.width = width;

    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_left_xpixels(sheet);
    gtk_sheet_recalc_view_range(sheet);

    adjust_scrollbars(sheet);

    sheet->old_hadjustment = -1.;

    if (sheet->hadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
    }

    size_allocate_global_button(sheet);
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
    if (height < DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet))) return;

    sheet->column_title_area.height = height;

    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_left_xpixels(sheet);
    gtk_sheet_recalc_view_range(sheet);

    adjust_scrollbars(sheet);

    sheet->old_vadjustment = -1.;

    if (sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }
    size_allocate_global_button(sheet);
}

/**
 * gtk_sheet_show_column_titles:
 * @sheet: a #GtkSheet
 *
 * Show column titles .
 */
void
    gtk_sheet_show_column_titles(GtkSheet *sheet)
{
    gint col;

    if (sheet->column_titles_visible) return;

    sheet->column_titles_visible = TRUE;
    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_left_xpixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        gdk_window_show(sheet->column_title_window);

        gdk_window_move_resize (sheet->column_title_window,
                                sheet->column_title_area.x,
                                sheet->column_title_area.y,
                                sheet->column_title_area.width,
                                sheet->column_title_area.height);

        for (col = MIN_VISIBLE_COLUMN(sheet); col <= MAX_VISIBLE_COLUMN(sheet); col++)
        {
            GtkSheetChild *child;
            if (col < 0 || col > sheet->maxcol) continue;

            child = sheet->column[col]->button.child;
            if (child) gtk_sheet_child_show(child);
        }
        adjust_scrollbars(sheet);
    }

    sheet->old_vadjustment = -1.;

    if (sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }
    size_allocate_global_button(sheet);
}

/**
 * gtk_sheet_show_row_titles:
 * @sheet: a #GtkSheet
 *
 * Show row titles .
 */
void
    gtk_sheet_show_row_titles(GtkSheet *sheet)
{
    gint row;

    if (sheet->row_titles_visible) return;

    sheet->row_titles_visible = TRUE;
    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_left_xpixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        gdk_window_show(sheet->row_title_window);

        gdk_window_move_resize (sheet->row_title_window,
                                sheet->row_title_area.x,
                                sheet->row_title_area.y,
                                sheet->row_title_area.width,
                                sheet->row_title_area.height);

        for (row = MIN_VISIBLE_ROW(sheet); row <= MAX_VISIBLE_ROW(sheet); row++)
        {
            GtkSheetChild *child;
            if (row < 0 || row > sheet->maxrow) continue;

            child = sheet->row[row].button.child;
            if (child) gtk_sheet_child_show(child);
        }
        adjust_scrollbars(sheet);
    }

    sheet->old_hadjustment = -1.;

    if (sheet->hadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
    }

    size_allocate_global_button(sheet);
}

/**
 * gtk_sheet_hide_column_titles:
 * @sheet: a #GtkSheet
 *
 * Hide column titles .
 */
void
    gtk_sheet_hide_column_titles(GtkSheet *sheet)
{
    gint col;

    if (!sheet->column_titles_visible) return;

    sheet->column_titles_visible = FALSE;
    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_left_xpixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        if (sheet->column_title_window)
            gdk_window_hide(sheet->column_title_window);

        if (gtk_widget_get_visible(sheet->button))
            gtk_widget_hide(sheet->button);

        for (col = MIN_VISIBLE_COLUMN(sheet); col <= MAX_VISIBLE_COLUMN(sheet); col++)
        {
            GtkSheetChild *child;
            if (col < 0 || col > sheet->maxcol) continue;

            child = sheet->column[col]->button.child;
            if (child) gtk_sheet_child_hide(child);
        }
        adjust_scrollbars(sheet);
    }

    sheet->old_vadjustment = -1.;

    if (sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }
}

/**
 * gtk_sheet_hide_row_titles:
 * @sheet: a #GtkSheet
 *
 * Hide row titles .
 */
void
    gtk_sheet_hide_row_titles(GtkSheet *sheet)
{
    gint row;

    if (!sheet->row_titles_visible) return;

    sheet->row_titles_visible = FALSE;
    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_left_xpixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        if (sheet->row_title_window)
            gdk_window_hide(sheet->row_title_window);

        if (gtk_widget_get_visible(sheet->button))
            gtk_widget_hide(sheet->button);

        for (row = MIN_VISIBLE_ROW(sheet); row <= MAX_VISIBLE_ROW(sheet); row++)
        {
            GtkSheetChild *child;
            if (row < 0 || row > sheet->maxrow) continue;

            child = sheet->row[row].button.child;
            if (child) gtk_sheet_child_hide(child);
        }
        adjust_scrollbars(sheet);
    }

    sheet->old_hadjustment = -1.;

    if (sheet->hadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
    }
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
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->column_titles_visible);
}

/**
 * gtk_sheet_row_titles_visible:
 * @sheet: a #GtkSheet
 *
 * Get the visibility of row column titles .
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_row_titles_visible(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->row_titles_visible);
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
    gtk_sheet_set_column_title (GtkSheet * sheet,
                                gint column,
                                const gchar * title)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (sheet->column[column]->title) g_free (sheet->column[column]->title);
    sheet->column[column]->title = g_strdup(title);
}

/**
 * gtk_sheet_set_row_title:
 * @sheet: a #GtkSheet
 * @row: row number
 * @title: row title
 *
 * Set row title.
 */
void
    gtk_sheet_set_row_title (GtkSheet * sheet,
                             gint row,
                             const gchar * title)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (sheet->row[row].name)
        g_free (sheet->row[row].name);

    sheet->row[row].name = g_strdup (title);
}

/**
 * gtk_sheet_get_row_title:
 * @sheet: a #GtkSheet
 * @row: row number
 *
 * Get row title.
 *
 * Returns: a pointer to the row title or NULL. 
 * Do not modify or free it. 
 */
const gchar *
    gtk_sheet_get_row_title (GtkSheet * sheet,
                             gint row)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    return(sheet->row[row].name);
}

/**
 * gtk_sheet_get_column_title:
 * @sheet: a #GtkSheet
 * @column: column number
 *
 * Get column title.
 *
 * Returns: column title
 */
const gchar *
    gtk_sheet_get_column_title (GtkSheet * sheet,
                                gint column)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    return(sheet->column[column]->title);
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
const gint
    gtk_sheet_get_column_width (GtkSheet *sheet, gint column)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    return(sheet->column[column]->width);
}

/**
 * gtk_sheet_row_button_add_label:
 * @sheet: a #GtkSheet
 * @row: row number
 * @label: text label
 *
 * Set button label.It is used to set a row title.
 */
void
    gtk_sheet_row_button_add_label(GtkSheet *sheet, gint row, const gchar *label)
{
    GtkSheetButton *button;
    GtkRequisition req;
    gboolean aux;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    button = &sheet->row[row].button;
    if (button->label) g_free (button->label);
    button->label = g_strdup (label);

    aux = gtk_sheet_autoresize(sheet);
    gtk_sheet_set_autoresize(sheet, TRUE);
    gtk_sheet_button_size_request(sheet, button, &req);
    gtk_sheet_set_autoresize(sheet, aux);

    if (req.height > sheet->row[row].height)
        gtk_sheet_set_row_height(sheet, row, req.height);

    if (req.width > sheet->row_title_area.width)
    {
        gtk_sheet_set_row_titles_width(sheet, req.width);
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_button_draw(sheet, row, -1);
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, row, -1);
    }
}

/**
 * gtk_sheet_row_button_get_label:
 * @sheet: a #GtkSheet
 * @row: row number
 *
 * Get a row button label.
 *
 * Returns: In case of succes , a pointer to label
 * text.Otherwise NULL
 */
const gchar *
    gtk_sheet_row_button_get_label(GtkSheet *sheet, gint row)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (row < 0 || row > sheet->maxrow) return(NULL);

    return(sheet->row[row].button.label);
}

/**
 * gtk_sheet_row_label_set_visibility:
 * @sheet: a #GtkSheet
 * @row: row number
 * @visible: TRUE or FALSE
 *
 * Set row label visibility.
 */
void
    gtk_sheet_row_label_set_visibility(GtkSheet *sheet, gint row, gboolean visible)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    sheet->row[row].button.label_visible = visible;

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_button_draw(sheet, row, -1);
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, row, -1);
    }
}

/**
 * gtk_sheet_rows_labels_set_visibility:
 * @sheet: a #GtkSheet
 * @visible: TRUE or FALSE
 *
 * Set all rows label visibility. The sheet itself
 * has no such property, this is a convenience function to set
 * the property for all existing rows.
 */
void
    gtk_sheet_rows_labels_set_visibility(GtkSheet *sheet, gboolean visible)
{
    gint i;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    for (i = 0; i <= sheet->maxrow; i++)
        gtk_sheet_row_label_set_visibility(sheet, i, visible);
}

/**
 * gtk_sheet_column_button_add_label:
 * @sheet: a #GtkSheet
 * @column: column number
 * @label: text label
 *
 * Set button label.It is used to set a column title.
 */
void
    gtk_sheet_column_button_add_label(GtkSheet *sheet, gint column, const gchar *label)
{
    GtkSheetButton *button;
    GtkRequisition req;
    gboolean aux;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column >sheet->maxcol) return;

    button = &sheet->column[column]->button;
    if (button->label) g_free (button->label);
    button->label = g_strdup (label);

    aux = gtk_sheet_autoresize(sheet);
    gtk_sheet_set_autoresize(sheet, TRUE);
    gtk_sheet_button_size_request(sheet, button, &req);
    gtk_sheet_set_autoresize(sheet, aux);

    if (req.width > sheet->column[column]->width)
        gtk_sheet_set_column_width(sheet, column, req.width);

    if (req.height > sheet->column_title_area.height)
        gtk_sheet_set_column_titles_height(sheet, req.height);

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_button_draw(sheet, -1, column);
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, -1, column);
    }
}

/**
 * gtk_sheet_column_button_get_label:
 * @sheet: a #GtkSheet.
 * @column: column number.
 *
 * Get column button label.
 *
 * Returns: Column button label.
 */
const gchar *
    gtk_sheet_column_button_get_label(GtkSheet *sheet, gint column)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (column < 0 || column >sheet->maxcol) return(NULL);

    return(sheet->column[column]->button.label);
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
    gtk_sheet_column_label_set_visibility(GtkSheet *sheet, gint col, gboolean visible)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col < 0 || col > sheet->maxcol) return;

    sheet->column[col]->button.label_visible = visible;

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_button_draw(sheet, -1, col);
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, -1, col);
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

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    for (i = 0; i <= sheet->maxcol; i++)
        gtk_sheet_column_label_set_visibility(sheet, i, visible);
}

/**
 * gtk_sheet_row_button_justify:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @justification : a #GtkJustification :GTK_JUSTIFY_LEFT, RIGHT, CENTER
 *
 * Set the justification(alignment) of the row buttons.
 */
void
    gtk_sheet_row_button_justify(GtkSheet *sheet, gint row,
                                 GtkJustification justification)
{
    GtkSheetButton *button;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    button = &sheet->row[row].button;
    button->justification = justification;

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_button_draw(sheet, row, -1);
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, row, -1);
    }
}

/**
 * gtk_sheet_column_button_justify:
 * @sheet: a #GtkSheet.
 * @column: column number
 * @justification : a #GtkJustification :GTK_JUSTIFY_LEFT, RIGHT, CENTER
 *
 * Set the justification(alignment) of the column buttons.
 */
void
    gtk_sheet_column_button_justify(GtkSheet *sheet, gint column,
                                    GtkJustification justification)
{
    GtkSheetButton *button;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column > sheet->maxcol) return;

    button = &sheet->column[column]->button;
    button->justification = justification;

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_button_draw(sheet, -1, column);
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, -1, column);
    }
}

/**
 * gtk_sheet_moveto:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @column: column number
 * @row_align: row alignment
 * @col_align: column alignment
 *
 * Scroll the viewing area of the sheet to the given column and row;
 * row_align and col_align are between 0-1 representing the location the row should appear on the screnn, 0.0 being top or left,
 * 1.0 being bottom or right; if row or column is negative then there is no change
 */

void
    gtk_sheet_moveto(GtkSheet *sheet,
                     gint row, 
                     gint column,
                     gfloat row_align, 
                     gfloat col_align)
{
    gint x, y;
    guint width, height;
    gint adjust;
    gint min_row, min_col;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));
    g_return_if_fail (sheet->hadjustment != NULL);
    g_return_if_fail (sheet->vadjustment != NULL);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_moveto: row %d column %d row_align %g col_align %g", 
            row, column, row_align, col_align);
#endif

    if (row < 0 || row > sheet->maxrow) return;
    if (column < 0 || column > sheet->maxcol) return;

    height = sheet->sheet_window_height;
    width = sheet->sheet_window_width;

    /* adjust vertical scrollbar */

    if (row >= 0 && row_align >=0.)
    {
/*
      y = ROW_TOP_YPIXEL(sheet, row) - sheet->voffset -
          row_align*height-
          (1.-row_align)*sheet->row[row].height;
*/
        y = ROW_TOP_YPIXEL (sheet, row) - sheet->voffset
            - (gint) ( row_align*height + (1. - row_align) * sheet->row[row].height);

        /* This forces the sheet to scroll when you don't see the entire cell */
        min_row = row;
        adjust = 0;
        if (row_align == 1.)
        {
            while (min_row >= 0 && min_row > MIN_VISIBLE_ROW(sheet))
            {
                if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, min_row)))
                    adjust += sheet->row[min_row].height;

                if (adjust >= height)
                {
                    break;
                }
                min_row--;
            }
            min_row = MAX(min_row, 0);
            y = ROW_TOP_YPIXEL(sheet, min_row) - sheet->voffset +
                sheet->row[min_row].height - 1;
        }

        if (y < 0)
            sheet->vadjustment->value = 0.0;
        else
            sheet->vadjustment->value = y;

        sheet->old_vadjustment = -1.;
        if (sheet->vadjustment)
        {
            g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
        }
    }

    /* adjust horizontal scrollbar */
    if (column >= 0 && col_align >= 0.)
    {
/*
      x = COLUMN_LEFT_XPIXEL (sheet, column) - sheet->hoffset -
          col_align*width -
          (1.-col_align) * sheet->column[column]->width;
*/
        x = COLUMN_LEFT_XPIXEL (sheet, column) - sheet->hoffset
            - (gint) ( col_align*width + (1.-col_align) * sheet->column[column]->width);

        /* This forces the sheet to scroll when you don't see the entire cell */
        min_col = column;
        adjust = 0;
        if (col_align == 1.)
        {
            while (min_col >= 0 && min_col > MIN_VISIBLE_COLUMN(sheet))
            {
                if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, min_col)))
                    adjust += sheet->column[min_col]->width;

                if (adjust >= width)
                {
                    break;
                }
                min_col--;
            }
            min_col = MAX(min_col, 0);
            x = COLUMN_LEFT_XPIXEL(sheet, min_col) - sheet->hoffset +
                sheet->column[min_col]->width - 1;
        }

        if (x < 0)
            sheet->hadjustment->value = 0.0;
        else
            sheet->hadjustment->value = x;

        sheet->old_vadjustment = -1.;

        if (sheet->hadjustment)
        {
            g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
        }
    }
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
    gtk_sheet_column_sensitive (GtkSheet *sheet, gint column)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (column < 0 || column > sheet->maxcol) return(FALSE);

    return(GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)));
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
    gtk_sheet_column_set_sensitivity(GtkSheet *sheet, gint column, gboolean sensitive)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column > sheet->maxcol) return;

    GTK_SHEET_COLUMN_SET_SENSITIVE(COLPTR(sheet, column), sensitive);

    if (!sensitive)
        sheet->column[column]->button.state=GTK_STATE_INSENSITIVE;
    else
        sheet->column[column]->button.state=GTK_STATE_NORMAL;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet))
        gtk_sheet_button_draw(sheet, -1, column);
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

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    for (i=0; i<=sheet->maxcol; i++)
        gtk_sheet_column_set_sensitivity(sheet, i, sensitive);
}

/**
 * gtk_sheet_columns_set_resizable:
 * @sheet: a #GtkSheet.
 * @resizable: TRUE or FALSE
 *
 * Set columns resizable status.
 */
void
    gtk_sheet_columns_set_resizable (GtkSheet *sheet, gboolean resizable)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

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
    gtk_sheet_columns_resizable (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->columns_resizable);
}

/**
 * gtk_sheet_row_sensitive:
 * @sheet: a #GtkSheet.
 * @row: row number
 *
 * Get row button sensitivity. 
 *  
 * Returns: 
 * TRUE - is sensitive,  
 * FALSE - insensitive or not existant 
 */
gboolean 
    gtk_sheet_row_sensitive (GtkSheet *sheet, gint row)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (row < 0 || row > sheet->maxrow) return(FALSE);

    return(GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)));
}

/**
 * gtk_sheet_row_set_sensitivity:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @sensitive: TRUE or FALSE
 *
 * Set row button sensitivity. If sensitivity is TRUE can be toggled, otherwise it acts as a title .
 */
void
    gtk_sheet_row_set_sensitivity(GtkSheet *sheet, gint row,  gboolean sensitive)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    GTK_SHEET_ROW_SET_SENSITIVE(ROWPTR(sheet, row), sensitive);

    if (!sensitive)
        sheet->row[row].button.state=GTK_STATE_INSENSITIVE;
    else
        sheet->row[row].button.state=GTK_STATE_NORMAL;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet))
        gtk_sheet_button_draw(sheet, row, -1);
}

/**
 * gtk_sheet_rows_set_sensitivity:
 * @sheet: a #GtkSheet.
 * @sensitive: TRUE or FALSE
 *
 * Set rows buttons sensitivity. If sensitivity is TRUE button
 * can be toggled, otherwise act as titles. The sheet itself
 * has no such property, it is a convenience function to set the
 * property for all existing rows.
 */
void
    gtk_sheet_rows_set_sensitivity(GtkSheet *sheet, gboolean sensitive)
{
    gint i;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    for (i=0; i<=sheet->maxrow; i++)
        gtk_sheet_row_set_sensitivity(sheet, i, sensitive);
}

/**
 * gtk_sheet_rows_set_resizable:
 * @sheet: a #GtkSheet.
 * @resizable: TRUE or FALSE
 *
 * Set rows resizable status.
 */
void
    gtk_sheet_rows_set_resizable (GtkSheet *sheet, gboolean resizable)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    sheet->rows_resizable = resizable;
}

/**
 * gtk_sheet_rows_resizable:
 * @sheet: a #GtkSheet.
 *
 * Get rows resizable status.
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_rows_resizable (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(sheet->rows_resizable);
}


/**
 * gtk_sheet_column_visible:
 * @sheet: a #GtkSheet.
 * @column: column number
 *
 * Get column visibility. 
 *  
 * Returns: TRUE - visible, FALSE - hidden or not existant 
 */
gboolean 
    gtk_sheet_column_visible (GtkSheet *sheet, gint column)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (column < 0 || column > sheet->maxcol) return(FALSE);

    return(GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column)));
}

/**
 * gtk_sheet_column_set_visibility:
 * @sheet: a #GtkSheet.
 * @column: column number
 * @visible: TRUE or FALSE
 *
 * Set column visibility. The default value is TRUE. If FALSE, the column is hidden.
 */
void
    gtk_sheet_column_set_visibility(GtkSheet *sheet, gint column, gboolean visible)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column > sheet->maxcol) return;
    if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column)) == visible) return;

    GTK_SHEET_COLUMN_SET_VISIBLE(sheet->column[column], visible);

    gtk_sheet_recalc_left_xpixels(sheet);

    if (!GTK_SHEET_IS_FROZEN(sheet) &&
        gtk_sheet_cell_isvisible(sheet, MIN_VISIBLE_ROW(sheet), column))
    {
        gtk_sheet_range_draw(sheet, NULL);
        size_allocate_column_title_buttons(sheet);
    }
}

/**
 * gtk_sheet_row_visible:
 * @sheet: a #GtkSheet.
 * @row: row number
 *
 * Get row visibility. 
 *  
 * Returns: 
 * TRUE - is visible 
 * FALSE - invisible or not existant 
 */
gboolean 
    gtk_sheet_row_visible (GtkSheet *sheet, gint row)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (row < 0 || row > sheet->maxrow) return(FALSE);

    return(GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)));
}

/**
 * gtk_sheet_row_set_visibility:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @visible: TRUE or FALSE
 *
 * Set row visibility. The default value is TRUE. If FALSE, the row is hidden.
 */
void
    gtk_sheet_row_set_visibility(GtkSheet *sheet, gint row, gboolean visible)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;
    if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) == visible) return;

    GTK_SHEET_ROW_SET_VISIBLE(&sheet->row[row], visible);

    gtk_sheet_recalc_top_ypixels(sheet);

    if (!GTK_SHEET_IS_FROZEN(sheet) &&
        gtk_sheet_cell_isvisible(sheet, row, MIN_VISIBLE_COLUMN(sheet)))
    {
        gtk_sheet_range_draw(sheet, NULL);
        size_allocate_row_title_buttons(sheet);
    }
}


/**
 * gtk_sheet_get_tooltip_markup: 
 * @sheet:  a #GtkSheet. 
 *  
 * Gets the contents of the tooltip (markup) for sheet 
 * 
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_get_tooltip_markup(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    return(gtk_widget_get_tooltip_markup(GTK_WIDGET(sheet)));
}

/**
 * gtk_sheet_set_tooltip_markup: 
 * @sheet:  a #GtkSheet.
 * @markup:  	the contents of the tooltip for widget, or NULL. 
 *  
 * Sets markup as the contents of the tooltip, which is marked 
 * up with the Pango text markup language. 
 */
void gtk_sheet_set_tooltip_markup(GtkSheet *sheet, 
                                  const gchar *markup)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    gtk_widget_set_tooltip_markup(GTK_WIDGET(sheet), markup);
}

/**
 * gtk_sheet_get_tooltip_text:
 * @sheet:  a #GtkSheet. 
 *  
 * Gets the contents of the tooltip for the #GtkSheet
 * 
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_get_tooltip_text(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    return(gtk_widget_get_tooltip_text(GTK_WIDGET(sheet)));
}

/**
 * gtk_sheet_set_tooltip_text: 
 * @sheet:  a #GtkSheet.
 * @text:  the contents of the tooltip for widget 
 *  
 * Sets text as the contents of the tooltip. 
 */
void gtk_sheet_set_tooltip_text(GtkSheet *sheet, 
                                const gchar *text)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    gtk_widget_set_tooltip_text(GTK_WIDGET(sheet), text);
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
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);

    return(gtk_widget_get_tooltip_markup(GTK_WIDGET(COLPTR(sheet, col))));
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
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

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
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);

    return(gtk_widget_get_tooltip_text(GTK_WIDGET(COLPTR(sheet, col))));
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
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col < 0 || col > sheet->maxcol) return;

    gtk_widget_set_tooltip_text(GTK_WIDGET(COLPTR(sheet, col)), text);
}

/**
 * gtk_sheet_row_get_tooltip_markup: 
 * @sheet:  a #GtkSheet. 
 * @row: row index 
 *  
 * Gets the contents of the tooltip (markup) for the column 
 *  
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_row_get_tooltip_markup(GtkSheet *sheet, 
                                        const gint row)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (row < 0 || row > sheet->maxrow) return(NULL);

    return(g_strdup(ROWPTR(sheet, row)->tooltip_markup));
}

/**
 * gtk_sheet_row_set_tooltip_markup: 
 * @sheet:  a #GtkSheet.
 * @row: row index 
 * @markup:  	the contents of the tooltip for widget, or NULL. 
 *  
 * Sets markup as the contents of the tooltip, which is marked 
 * up with the Pango text markup language. 
 */
void gtk_sheet_row_set_tooltip_markup(GtkSheet *sheet, 
                                      const gint row, 
                                      const gchar *markup)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    if (sheet->row[row].tooltip_markup) g_free(sheet->row[row].tooltip_markup);
    sheet->row[row].tooltip_markup = g_strdup(markup);
}

/**
 * gtk_sheet_row_get_tooltip_text: 
 * @sheet:  a #GtkSheet. 
 * @row: row index 
 *  
 * Gets the contents of the tooltip for the column 
 * 
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_row_get_tooltip_text(GtkSheet *sheet, 
                                      const gint row)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (row < 0 || row > sheet->maxrow) return(NULL);

    return(g_strdup(ROWPTR(sheet, row)->tooltip_text));
}

/**
 * gtk_sheet_row_set_tooltip_text: 
 * @sheet:  a #GtkSheet.
 * @row: row index 
 * @text:  the contents of the tooltip for widget 
 *  
 * Sets text as the contents of the tooltip. 
 */
void gtk_sheet_row_set_tooltip_text(GtkSheet *sheet, 
                                    const gint row, 
                                    const gchar *text)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    if (sheet->row[row].tooltip_text) g_free(sheet->row[row].tooltip_text);
    sheet->row[row].tooltip_text = g_strdup(text);
}

/**
 * gtk_sheet_cell_get_tooltip_markup: 
 * @sheet:  a #GtkSheet. 
 * @row: row index 
 * @col: column index 
 *  
 * Gets the contents of the tooltip (markup) for the column 
 * 
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_cell_get_tooltip_markup(GtkSheet *sheet, 
                                         const gint row, const gint col)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);
    if (row < 0 || row > sheet->maxrow) return(NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol) return(NULL);

    if (!sheet->data[row]) return(NULL);
    if (!sheet->data[row][col]) return(NULL);

    return(g_strdup(sheet->data[row][col]->tooltip_markup));
}

/**
 * gtk_sheet_cell_set_tooltip_markup: 
 * @sheet:  a #GtkSheet.
 * @row: row index 
 * @col: column index 
 * @markup:  	the contents of the tooltip for widget, or NULL. 
 *  
 * Sets markup as the contents of the tooltip, which is marked 
 * up with the Pango text markup language. 
 */
void gtk_sheet_cell_set_tooltip_markup(GtkSheet *sheet, 
                                       const gint row, const gint col, 
                                       const gchar *markup)
{
    GtkSheetCell *cell;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col < 0 || col > sheet->maxcol) return;
    if (row < 0 || row > sheet->maxrow) return;

    CheckCellData(sheet, row, col);
    cell = sheet->data[row][col];

    if (cell->tooltip_markup)
    {
        g_free(cell->tooltip_markup);
        cell->tooltip_markup = NULL;
    }

    cell->tooltip_markup = g_strdup(markup);
}

/**
 * gtk_sheet_cell_get_tooltip_text: 
 * @sheet:  a #GtkSheet. 
 * @row: row index 
 * @col: column index 
 * 
 * Gets the contents of the tooltip for the column 
 *  
 * Returns:	the tooltip text, or NULL. You should free the 
 *          returned string with g_free() when done.
 */
gchar *gtk_sheet_cell_get_tooltip_text(GtkSheet *sheet, 
                                       const gint row, const gint col)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);
    if (row < 0 || row > sheet->maxrow) return(NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol) return(NULL);

    if (!sheet->data[row]) return(NULL);
    if (!sheet->data[row][col]) return(NULL);

    return(g_strdup(sheet->data[row][col]->tooltip_text));
}

/**
 * gtk_sheet_cell_set_tooltip_text: 
 * @sheet:  a #GtkSheet.
 * @row: row index 
 * @col: column index 
 * @text:  the contents of the tooltip for widget 
 *  
 * Sets text as the contents of the tooltip. 
 */
void gtk_sheet_cell_set_tooltip_text(GtkSheet *sheet, 
                                     const gint row, const gint col, 
                                     const gchar *text)
{
    GtkSheetCell *cell;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col < 0 || col > sheet->maxcol) return;
    if (row < 0 || row > sheet->maxrow) return;

    CheckCellData(sheet, row, col);
    cell = sheet->data[row][col];

    if (cell->tooltip_text)
    {
        g_free(cell->tooltip_text);
        cell->tooltip_text = NULL;
    }

    cell->tooltip_markup = g_strdup(text);
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
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (col < 0 || col > sheet->maxcol) return(FALSE);

    return(COLPTR(sheet, col)->is_key);
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
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

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
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (col < 0 || col > sheet->maxcol) return(FALSE);

    return(COLPTR(sheet, col)->is_readonly);
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
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

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
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);

    return(g_strdup(COLPTR(sheet, col)->data_format));
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

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

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
 * Gets the column data type used for content validation in data 
 * entry
 * 
 * Returns:	the datatype
 */
GtkSheetDataType gtk_sheet_column_get_datatype(GtkSheet *sheet, const gint col)
{
    g_return_val_if_fail (sheet != NULL, GTK_SHEET_DATA_TYPE_NONE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), GTK_SHEET_DATA_TYPE_NONE);

    if (col < 0 || col > sheet->maxcol) return(GTK_SHEET_DATA_TYPE_NONE);

    return(COLPTR(sheet, col)->data_type);
}

/**
 * gtk_sheet_column_set_datatype: 
 * @sheet:  a #GtkSheet.
 * @col: column index 
 * @data_type:  the datatype 
 *  
 * Sets the column data type used for content validation in data 
 * entry. 
 */
void gtk_sheet_column_set_datatype(GtkSheet *sheet, const gint col, 
                                   const GtkSheetDataType data_type)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->data_type = data_type;
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
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col < 0 || col > sheet->maxcol) return(NULL);

    return(g_strdup(COLPTR(sheet, col)->description));
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

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

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
    g_return_val_if_fail (sheet != NULL, GTK_SHEET_ENTRY_TYPE_DEFAULT);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), GTK_SHEET_ENTRY_TYPE_DEFAULT);

    if (col < 0 || col > sheet->maxcol) return(GTK_SHEET_ENTRY_TYPE_DEFAULT);

    return(COLPTR(sheet, col)->entry_type);
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
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col < 0 || col > sheet->maxcol) return;

    COLPTR(sheet, col)->entry_type = entry_type;
}



/**
 * gtk_sheet_select_row:
 * @sheet: a #GtkSheet.
 * @row: row number
 *
 * Select the row. The range is then highlighted, and the bounds are stored in sheet->range.
 */
void
    gtk_sheet_select_row (GtkSheet * sheet, gint row)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow) return;

    if (sheet->state != GTK_SHEET_NORMAL)
    {
        gtk_sheet_real_unselect_range(sheet, NULL);
    }
    else
    {
        gboolean veto = TRUE;
        veto = gtk_sheet_deactivate_cell(sheet);
        if (!veto) return;
    }

    sheet->state=GTK_SHEET_ROW_SELECTED;
    sheet->range.row0=row;
    sheet->range.col0=0;
    sheet->range.rowi=row;
    sheet->range.coli=sheet->maxcol;
    sheet->active_cell.row=row;
    sheet->active_cell.col=0;

    g_signal_emit (GTK_OBJECT (sheet), sheet_signals[SELECT_ROW], 0, row);
    gtk_sheet_real_select_range(sheet, NULL);
}

/**
 * gtk_sheet_select_column:
 * @sheet: a #GtkSheet.
 * @column: column number
 *
 * Select the column. The range is then highlighted, and the bounds are stored in sheet->range.
 */
void
    gtk_sheet_select_column (GtkSheet * sheet, gint column)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column > sheet->maxcol) return;

    if (sheet->state != GTK_SHEET_NORMAL)
    {
        gtk_sheet_real_unselect_range(sheet, NULL);
    }
    else
    {
        gboolean veto = TRUE;
        veto = gtk_sheet_deactivate_cell(sheet);
        if (!veto) return;
    }

    sheet->state=GTK_SHEET_COLUMN_SELECTED;
    sheet->range.row0=0;
    sheet->range.col0=column;
    sheet->range.rowi=sheet->maxrow;
    sheet->range.coli=column;
    sheet->active_cell.row=0;
    sheet->active_cell.col=column;

    g_signal_emit (GTK_OBJECT (sheet), sheet_signals[SELECT_COLUMN], 0, column);
    gtk_sheet_real_select_range(sheet, NULL);
}

/**
 * gtk_sheet_clip_range:
 * @sheet: a #GtkSheet.
 * @range: #GtkSheetRange to be saved
 *
 * Save selected range to "clipboard".
 */
void
    gtk_sheet_clip_range (GtkSheet *sheet, const GtkSheetRange *range)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (GTK_SHEET_IN_CLIP(sheet)) return;

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_CLIP);

    if (range == NULL)
        sheet->clip_range = sheet->range;
    else
        sheet->clip_range=*range;

    sheet->interval=0;
    sheet->clip_timer=g_timeout_add_full(0, TIMEOUT_FLASH, gtk_sheet_flash, sheet, NULL);

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CLIP_RANGE], 0, &sheet->clip_range);
}

/**
 * gtk_sheet_unclip_range:
 * @sheet: a #GtkSheet.
 *
 * Free clipboard.
 */
void
    gtk_sheet_unclip_range(GtkSheet *sheet)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!GTK_SHEET_IN_CLIP(sheet)) return;

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_CLIP);
    g_source_remove(sheet->clip_timer);
    gtk_sheet_range_draw(sheet, &sheet->clip_range);

    if (gtk_sheet_range_isvisible(sheet, sheet->range))
        gtk_sheet_range_draw(sheet, &sheet->range);
}

/**
 * gtk_sheet_in_clip:
 * @sheet: a #GtkSheet.
 *
 * Get the clip status of #GtkSheet.
 *
 * Returns: TRUE or FALSE
 */
gboolean
    gtk_sheet_in_clip (GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    return(GTK_SHEET_IN_CLIP(sheet));
}


static gint
    gtk_sheet_flash(gpointer data)
{
    GtkSheet *sheet;
    gint x,y,width,height;
    GdkRectangle clip_area;

    sheet=GTK_SHEET(data);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return(TRUE);
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return(TRUE);
    if (!gtk_sheet_range_isvisible(sheet, sheet->clip_range)) return(TRUE);
    if (GTK_SHEET_IN_XDRAG(sheet)) return(TRUE);
    if (GTK_SHEET_IN_YDRAG(sheet)) return(TRUE);

    GDK_THREADS_ENTER();

    x=COLUMN_LEFT_XPIXEL(sheet,sheet->clip_range.col0)+1;
    y=ROW_TOP_YPIXEL(sheet,sheet->clip_range.row0)+1;
    width=COLUMN_LEFT_XPIXEL(sheet,sheet->clip_range.coli)-x+
        sheet->column[sheet->clip_range.coli]->width-1;
    height=ROW_TOP_YPIXEL(sheet,sheet->clip_range.rowi)-y+
        sheet->row[sheet->clip_range.rowi].height-1;

    clip_area.x=COLUMN_LEFT_XPIXEL(sheet, MIN_VISIBLE_COLUMN(sheet));
    clip_area.y=ROW_TOP_YPIXEL(sheet, MIN_VISIBLE_ROW(sheet));
    clip_area.width=sheet->sheet_window_width;
    clip_area.height=sheet->sheet_window_height;

    if (x<0)
    {
        width=width+x+1;
        x=-1;
    }
    if (width>clip_area.width) width=clip_area.width+10;

    if (y<0)
    {
        height=height+y+1;
        y=-1;
    }
    if (height>clip_area.height) height=clip_area.height+10;

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    x, y,
                    x, y,
                    1, height);

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    x, y,
                    x, y,
                    width, 1);

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    x, y+height,
                    x, y+height,
                    width, 1);

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    x+width, y,
                    x+width, y,
                    1, height);

    sheet->interval=sheet->interval+1;
    if (sheet->interval==TIME_INTERVAL) sheet->interval=0;

    gdk_gc_set_dashes(sheet->xor_gc, sheet->interval, (gint8*)"\4\4", 2);
    gtk_sheet_draw_flashing_range(sheet,sheet->clip_range);
    gdk_gc_set_dashes(sheet->xor_gc, 0, (gint8*)"\4\4", 2);

    GDK_THREADS_LEAVE();

    return(TRUE);
}

static void
    gtk_sheet_draw_flashing_range(GtkSheet *sheet, GtkSheetRange range)
{
    GdkRectangle clip_area;
    gint x,y,width,height;

    if (!gtk_sheet_range_isvisible(sheet, sheet->clip_range)) return;

    clip_area.x=COLUMN_LEFT_XPIXEL(sheet, MIN_VISIBLE_COLUMN(sheet));
    clip_area.y=ROW_TOP_YPIXEL(sheet, MIN_VISIBLE_ROW(sheet));
    clip_area.width=sheet->sheet_window_width;
    clip_area.height=sheet->sheet_window_height;

    gdk_gc_set_clip_rectangle(sheet->xor_gc, &clip_area);

    x=COLUMN_LEFT_XPIXEL(sheet,sheet->clip_range.col0)+1;
    y=ROW_TOP_YPIXEL(sheet,sheet->clip_range.row0)+1;
    width=COLUMN_LEFT_XPIXEL(sheet,sheet->clip_range.coli)-x+
        sheet->column[sheet->clip_range.coli]->width-1;
    height=ROW_TOP_YPIXEL(sheet,sheet->clip_range.rowi)-y+
        sheet->row[sheet->clip_range.rowi].height-1;

    if (x<0)
    {
        width=width+x+1;
        x=-1;
    }
    if (width>clip_area.width) width=clip_area.width+10;

    if (y<0)
    {
        height=height+y+1;
        y=-1;
    }
    if (height>clip_area.height) height=clip_area.height+10;

    gdk_gc_set_line_attributes(sheet->xor_gc, 1, 1, 0 ,0 );

    gdk_draw_rectangle(sheet->sheet_window, sheet->xor_gc, FALSE,
                       x, y,
                       width, height);

    gdk_gc_set_line_attributes (sheet->xor_gc, 1, 0, 0, 0);

    gdk_gc_set_clip_rectangle(sheet->xor_gc, NULL);
}

static gint
    gtk_sheet_range_isvisible(GtkSheet *sheet, GtkSheetRange range)
{
    g_return_val_if_fail (sheet != NULL, FALSE);

    if (range.row0 > MAX_VISIBLE_ROW (sheet)) return (FALSE);
    if (range.rowi < MIN_VISIBLE_ROW (sheet)) return (FALSE);

    if (range.col0 > MAX_VISIBLE_COLUMN (sheet)) return (FALSE);
    if (range.coli < MIN_VISIBLE_COLUMN (sheet)) return (FALSE);

    return (TRUE);
}

static gint
    gtk_sheet_cell_isvisible (GtkSheet * sheet,
                              gint row, gint column)
{
    GtkSheetRange range;

    range.row0 = row;
    range.col0 = column;
    range.rowi = row;
    range.coli = column;

    return(gtk_sheet_range_isvisible(sheet, range));
}

/**
 * gtk_sheet_get_visible_range:
 * @sheet: a #GtkSheet.
 * @range : a selected #GtkSheetRange
 * struct _GtkSheetRange { gint row0,col0; //  upper-left cell
 * 			  gint rowi,coli;  // lower-right cell  };
 *
 * Get sheet's ranges in a #GkSheetRange structure.
 */
void
    gtk_sheet_get_visible_range(GtkSheet *sheet, GtkSheetRange *range)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet)) ;
    g_return_if_fail (range != NULL);

    range->row0 = MIN_VISIBLE_ROW(sheet);
    range->col0 = MIN_VISIBLE_COLUMN(sheet);
    range->rowi = MAX_VISIBLE_ROW(sheet);
    range->coli = MAX_VISIBLE_COLUMN(sheet);
}

/**
 * gtk_sheet_get_vadjustment:
 * @sheet: a #GtkSheet.
 *
 * Get vertical scroll adjustments.
 *
 * Returns: a #GtkAdjustment
 */
GtkAdjustment *
    gtk_sheet_get_vadjustment (GtkSheet * sheet)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    return(sheet->vadjustment);
}

/**
 * gtk_sheet_get_hadjustment:
 * @sheet: a #GtkSheet.
 *
 * Get horizontal scroll adjustments.
 *
 * Returns: a #GtkAdjustment
 */
GtkAdjustment *
    gtk_sheet_get_hadjustment (GtkSheet * sheet)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    return(sheet->hadjustment);
}

/**
 * gtk_sheet_set_vadjustment:
 * @sheet: a #GtkSheet.
 * @adjustment: a #GtkAdjustment
 *
 * Change vertical scroll adjustments.
 */
void
    gtk_sheet_set_vadjustment (GtkSheet *sheet, GtkAdjustment *adjustment)
{
    GtkAdjustment *old_adjustment;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (adjustment) g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
    if (sheet->vadjustment == adjustment) return;

    old_adjustment = sheet->vadjustment;

    if (sheet->vadjustment)
    {
        g_signal_handlers_disconnect_matched (
                                             GTK_OBJECT (sheet->vadjustment), 
                                             G_SIGNAL_MATCH_DATA,
                                             0, 0, NULL, NULL, sheet);
        g_object_unref (G_OBJECT (sheet->vadjustment));
    }

    sheet->vadjustment = adjustment;

    if (sheet->vadjustment)
    {
        g_object_ref (G_OBJECT (sheet->vadjustment));
        g_object_ref_sink (G_OBJECT (sheet->vadjustment));
        g_object_unref (G_OBJECT (sheet->vadjustment));

        g_signal_connect (GTK_OBJECT (sheet->vadjustment), "changed",
                          (void *) vadjustment_changed_handler,
                          (gpointer) sheet);
        g_signal_connect (GTK_OBJECT (sheet->vadjustment), "value_changed",
                          (void *) vadjustment_value_changed_handler,
                          (gpointer) sheet);
    }

    if (!sheet->vadjustment || !old_adjustment)
    {
        gtk_widget_queue_resize (GTK_WIDGET (sheet));
        return;
    }

    sheet->old_vadjustment = sheet->vadjustment->value;
}

/**
 * gtk_sheet_set_hadjustment:
 * @sheet: a #GtkSheet.
 * @adjustment: a #GtkAdjustment
 *
 * Change horizontal scroll adjustments.
 */
void
    gtk_sheet_set_hadjustment (GtkSheet *sheet, GtkAdjustment *adjustment)
{
    GtkAdjustment *old_adjustment;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (adjustment) g_return_if_fail (GTK_IS_ADJUSTMENT (adjustment));
    if (sheet->hadjustment == adjustment) return;

    old_adjustment = sheet->hadjustment;

    if (sheet->hadjustment)
    {
        g_signal_handlers_disconnect_matched (
                                             GTK_OBJECT (sheet->hadjustment), 
                                             G_SIGNAL_MATCH_DATA,
                                             0, 0, NULL, NULL, sheet);
        g_object_unref (GTK_OBJECT (sheet->hadjustment));
    }

    sheet->hadjustment = adjustment;

    if (sheet->hadjustment)
    {
        g_object_ref (G_OBJECT (sheet->hadjustment));
        g_object_ref_sink (G_OBJECT (sheet->hadjustment));
        g_object_unref (G_OBJECT (sheet->hadjustment));

        g_signal_connect (GTK_OBJECT (sheet->hadjustment), "changed",
                          (void *) hadjustment_changed_handler,
                          (gpointer) sheet);
        g_signal_connect (GTK_OBJECT (sheet->hadjustment), "value_changed",
                          (void *) hadjustment_value_changed_handler,
                          (gpointer) sheet);
    }

    if (!sheet->hadjustment || !old_adjustment)
    {
        gtk_widget_queue_resize (GTK_WIDGET (sheet));
        return;
    }

    sheet->old_hadjustment = sheet->hadjustment->value;
}

/**
 * gtk_sheet_set_scroll_adjustments:
 * @sheet: a #GtkSheet.
 * @hadjustment: a #GtkAdjustment
 * @vadjustment: a #GtkAdjustment
 *
 * Change horizontal and vertical scroll adjustments.
 */
static void
    gtk_sheet_set_scroll_adjustments (GtkSheet *sheet,
                                      GtkAdjustment *hadjustment, 
                                      GtkAdjustment *vadjustment)
{
    if (sheet->hadjustment != hadjustment)
        gtk_sheet_set_hadjustment (sheet, hadjustment);

    if (sheet->vadjustment != vadjustment)
        gtk_sheet_set_vadjustment (sheet, vadjustment);
}

/*
 * gtk_sheet_finalize_handler:
 * 
 * this is the #GtkSheet object class "finalize" signal handler
 * 
 * @param object the #GtkSheet
 */
static void
    gtk_sheet_finalize_handler (GObject * object)
{
    GtkSheet *sheet;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_SHEET (object));

    sheet = GTK_SHEET (object);

    /* get rid of all the cells */
    gtk_sheet_range_clear (sheet, NULL);
    gtk_sheet_range_delete(sheet, NULL);

    gtk_sheet_delete_rows (sheet, 0, sheet->maxrow + 1);
    gtk_sheet_delete_columns (sheet, 0, sheet->maxcol + 1);

    DeleteRow (sheet, 0, sheet->maxrow + 1);
    DeleteColumn (sheet, 0, sheet->maxcol + 1);

    g_free(sheet->row);
    sheet->row = NULL;

    if (sheet->column)  /* free remaining column array, no gobjects there */
    {
        g_free(sheet->column);
        sheet->column = NULL;
    }

    g_free(sheet->data);
    sheet->data = NULL;

    if (sheet->title)
    {
        g_free(sheet->title);
        sheet->title = NULL;
    }

    if (G_OBJECT_CLASS (sheet_parent_class)->finalize)
        (*G_OBJECT_CLASS (sheet_parent_class)->finalize) (object);
}

/*
 * gtk_sheet_destroy_handler:
 * 
 * this is the #GtkSheet object class "finalize" handler
 * 
 * @param object
 */
static void
    gtk_sheet_destroy_handler (GtkObject * object)
{
    GtkSheet *sheet;
    GList *children;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_SHEET (object));

    sheet = GTK_SHEET (object);

    /* destroy the entry */
    if (sheet->sheet_entry && GTK_IS_WIDGET(sheet->sheet_entry))
    {
        gtk_widget_destroy (sheet->sheet_entry);
        sheet->sheet_entry = NULL;
    }

    /* destroy the global selection button */
    if (sheet->button && GTK_IS_WIDGET(sheet->button))
    {
#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_destroy: destroying old entry %p", sheet->button);
#endif
        gtk_widget_destroy (sheet->button);
        sheet->button = NULL;
    }

    if (sheet->timer)
    {
        g_source_remove(sheet->timer);
        sheet->timer = 0;
    }

    if (sheet->clip_timer)
    {
        g_source_remove(sheet->clip_timer);
        sheet->clip_timer = 0;
    }

    /* unref adjustments */
    if (sheet->hadjustment)
    {
        g_signal_handlers_disconnect_matched (
                                             GTK_OBJECT (sheet->hadjustment), 
                                             G_SIGNAL_MATCH_DATA,
                                             0, 0, NULL, NULL, sheet);
        g_object_unref (G_OBJECT (sheet->hadjustment));
        sheet->hadjustment = NULL;
    }
    if (sheet->vadjustment)
    {
        g_signal_handlers_disconnect_matched (
                                             GTK_OBJECT (sheet->vadjustment), 
                                             G_SIGNAL_MATCH_DATA,
                                             0, 0, NULL, NULL, sheet);
        g_object_unref (G_OBJECT (sheet->vadjustment));
        sheet->vadjustment = NULL;
    }

    children = sheet->children;
    while (children)
    {
        GtkSheetChild *child = (GtkSheetChild *)children->data;
        if (child && child->widget)
            gtk_sheet_remove_handler(GTK_CONTAINER(sheet), child->widget);
        children = sheet->children;
    }
    sheet->children = NULL;

    if (GTK_OBJECT_CLASS (sheet_parent_class)->destroy)
        (*GTK_OBJECT_CLASS (sheet_parent_class)->destroy) (object);
}

/*
 * gtk_sheet_style_set_handler:
 * 
 * this is the #GtkSheet widget class "style-set" signal handler
 * 
 * @param widget the #GtkSheet
 * @param previous_style
 */
static void
    gtk_sheet_style_set_handler (GtkWidget *widget, GtkStyle  *previous_style)
{
    GtkSheet *sheet;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));

    if (GTK_WIDGET_CLASS (sheet_parent_class)->style_set)
        (*GTK_WIDGET_CLASS (sheet_parent_class)->style_set) (widget, previous_style);

    sheet = GTK_SHEET (widget);

    if (gtk_widget_get_realized(widget))
    {
        gtk_style_set_background (gtk_widget_get_style(widget), 
                                  gtk_widget_get_window(widget), 
                                  gtk_widget_get_state(widget));
    }
}

/*
 * gtk_sheet_realize_handler:
 * 
 * this is the #GtkSheet widget class "realize" signal handler
 * 
 * @param widget the #GtkSheet
 */
static void
    gtk_sheet_realize_handler (GtkWidget * widget)
{
    GtkSheet *sheet;
    GdkWindowAttr attributes;
    gint attributes_mask;
    GdkGCValues values, auxvalues;
    GdkColormap *colormap;
    GtkSheetChild *child;
    GList *children;
    GtkAllocation allocation;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));

    sheet = GTK_SHEET (widget);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_realize: called (%p)", sheet->sheet_entry);
#endif

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    gtk_widget_get_allocation(widget, &allocation);
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);

    attributes.event_mask = gtk_widget_get_events (widget);
    attributes.event_mask |= (
                             GDK_EXPOSURE_MASK |
                             GDK_BUTTON_PRESS_MASK |
                             GDK_BUTTON_RELEASE_MASK |
                             GDK_KEY_PRESS_MASK |
                             GDK_POINTER_MOTION_MASK |
                             GDK_POINTER_MOTION_HINT_MASK);

    attributes_mask = GDK_WA_X | GDK_WA_Y | 
        GDK_WA_VISUAL | GDK_WA_COLORMAP | GDK_WA_CURSOR;

    attributes.cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);

    /* main window */
    gtk_widget_set_window(widget, 
                          gdk_window_new (gtk_widget_get_parent_window (widget), 
                                          &attributes, attributes_mask));

    gdk_window_set_user_data (gtk_widget_get_window(widget), sheet);

    gtk_widget_set_style(widget, 
                         gtk_style_attach(gtk_widget_get_style(widget),
                                          gtk_widget_get_window(widget)));

    gtk_style_set_background (gtk_widget_get_style(widget),     
                              gtk_widget_get_window(widget), 
                              GTK_STATE_NORMAL);

    attributes.x = 0;
    if (sheet->row_titles_visible)
        attributes.x = sheet->row_title_area.width;
    attributes.y = 0;
    attributes.width = sheet->column_title_area.width;
    attributes.height = sheet->column_title_area.height;

    /* column-title window */
    sheet->column_title_window = gdk_window_new (
                                                gtk_widget_get_window(widget), 
                                                &attributes, attributes_mask);
    gdk_window_set_user_data (sheet->column_title_window, sheet);
    gtk_style_set_background (gtk_widget_get_style(widget),
                              sheet->column_title_window, GTK_STATE_NORMAL);

    attributes.x = 0;
    attributes.y = 0;
    if (sheet->column_titles_visible)
        attributes.y = sheet->column_title_area.height;
    attributes.width = sheet->row_title_area.width;
    attributes.height = sheet->row_title_area.height;

    /* row-title window */
    sheet->row_title_window = gdk_window_new (
                                             gtk_widget_get_window(widget), 
                                             &attributes, attributes_mask);
    gdk_window_set_user_data (sheet->row_title_window, sheet);
    gtk_style_set_background (gtk_widget_get_style(widget), 
                              sheet->row_title_window, GTK_STATE_NORMAL);

    /* sheet-window */
    attributes.cursor = gdk_cursor_new(GDK_PLUS);

    attributes.x = 0;
    attributes.y = 0;
    attributes.width = sheet->sheet_window_width;
    attributes.height = sheet->sheet_window_height;

    sheet->sheet_window = gdk_window_new (
                                         gtk_widget_get_window(widget), 
                                         &attributes, attributes_mask);
    gdk_window_set_user_data (sheet->sheet_window, sheet);

    gdk_window_set_background (sheet->sheet_window, 
                               &(gtk_widget_get_style(widget)->white));
    gdk_window_show (sheet->sheet_window);

    /* backing_pixmap */
    gtk_sheet_make_backing_pixmap(sheet, 0, 0);

    /* GCs */
    if (sheet->fg_gc) gdk_gc_unref(sheet->fg_gc);
    sheet->fg_gc = gdk_gc_new (gtk_widget_get_window(widget));

    if (sheet->bg_gc) gdk_gc_unref(sheet->bg_gc);
    sheet->bg_gc = gdk_gc_new (gtk_widget_get_window(widget));

    colormap = gtk_widget_get_colormap(widget);

    gdk_color_white(colormap, &(gtk_widget_get_style(widget)->white));
    gdk_color_black(colormap, &(gtk_widget_get_style(widget)->black));

    gdk_gc_get_values(sheet->fg_gc, &auxvalues);

    values.foreground = gtk_widget_get_style(widget)->white;
    values.function = GDK_INVERT;
    values.subwindow_mode = GDK_INCLUDE_INFERIORS;

    if (sheet->xor_gc) gdk_gc_unref(sheet->xor_gc);
    sheet->xor_gc = gdk_gc_new_with_values(gtk_widget_get_window(widget),
                                           &values,
                                           GDK_GC_FOREGROUND |
                                           GDK_GC_FUNCTION |
                                           GDK_GC_SUBWINDOW);

    if (sheet->sheet_entry->parent)
    {
        g_object_ref(sheet->sheet_entry);
        gtk_widget_unparent(sheet->sheet_entry);
    }
    gtk_widget_set_parent_window (sheet->sheet_entry, sheet->sheet_window);
    gtk_widget_set_parent(sheet->sheet_entry, GTK_WIDGET(sheet));

    if (sheet->button && sheet->button->parent)
    {
        g_object_ref(sheet->button);
        gtk_widget_unparent(sheet->button);
    }
    gtk_widget_set_parent_window(sheet->button, sheet->sheet_window);
    gtk_widget_set_parent(sheet->button, GTK_WIDGET(sheet));

/*
  gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
*/

    if (!sheet->cursor_drag)
        sheet->cursor_drag = gdk_cursor_new(GDK_PLUS);

    if (sheet->column_titles_visible)
        gdk_window_show(sheet->column_title_window);
    if (sheet->row_titles_visible)
        gdk_window_show(sheet->row_title_window);

    size_allocate_row_title_buttons(sheet);
    size_allocate_column_title_buttons(sheet);

    gtk_sheet_set_title(sheet, sheet->title);

    children = sheet->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        gtk_sheet_realize_child(sheet, child);
    }
}

static void
    create_global_button(GtkSheet *sheet)
{
    sheet->button = gtk_button_new_with_label(" ");

    g_signal_connect (GTK_OBJECT (sheet->button),
                      "button-press-event",
                      (void *) global_button_press_handler,
                      (gpointer) sheet);
}

static void
    size_allocate_global_button(GtkSheet *sheet)
{
    GtkAllocation allocation;

    if (!sheet->column_titles_visible) return;
    if (!sheet->row_titles_visible) return;

    gtk_widget_size_request(sheet->button, NULL);

    allocation.x=0;
    allocation.y=0;
    allocation.width=sheet->row_title_area.width;
    allocation.height=sheet->column_title_area.height;

    gtk_widget_size_allocate(sheet->button, &allocation);
    gtk_widget_show(sheet->button);
}

/*
 * global_button_clicked_handler:<p>
 * this is the #GtkSheet global button "button-press-event" handler
 * 
 * @param widget the global sheet button that received the signal
 * @param data   the #GtkSheet passed on signal connection
 */
static void
    global_button_press_handler(GtkWidget *widget, gpointer data)
{
    gboolean veto;

    gtk_sheet_click_cell(GTK_SHEET(data), -1, -1, &veto);
    gtk_widget_grab_focus(GTK_WIDGET(data));
}


/*
 * gtk_sheet_unrealize_handler:
 * 
 * this is the #GtkSheet widget class "unrealize" signal handler
 * 
 * @param widget the #GtkSheet
 */
static void
    gtk_sheet_unrealize_handler (GtkWidget * widget)
{
    GtkSheet *sheet;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));

    sheet = GTK_SHEET (widget);

    gdk_cursor_destroy (sheet->cursor_drag);

    gdk_gc_destroy (sheet->xor_gc);
    gdk_gc_destroy (sheet->fg_gc);
    gdk_gc_destroy (sheet->bg_gc);

    gdk_window_destroy (sheet->sheet_window);
    gdk_window_destroy (sheet->column_title_window);
    gdk_window_destroy (sheet->row_title_window);

    if (sheet->pixmap)
    {
        g_object_unref (G_OBJECT(sheet->pixmap));
        sheet->pixmap = NULL;
    }

    sheet->column_title_window=NULL;
    sheet->sheet_window = NULL;
    sheet->cursor_drag = NULL;
    sheet->xor_gc = NULL;
    sheet->fg_gc = NULL;
    sheet->bg_gc = NULL;

    if (GTK_WIDGET_CLASS (sheet_parent_class)->unrealize)
        (* GTK_WIDGET_CLASS (sheet_parent_class)->unrealize) (widget);
}

/*
 * gtk_sheet_map_handler:
 * 
 * this is the #GtkSheet widget class "map" signal handler
 * 
 * @param widget the #GtkSheet
 */
static void
    gtk_sheet_map_handler (GtkWidget * widget)
{
    GtkSheet *sheet;
    GtkWidget *WdChild;
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));

    sheet = GTK_SHEET (widget);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_map: called");
#endif

    if (!gtk_widget_get_mapped (widget))
    {
        GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

        if (!sheet->cursor_drag) sheet->cursor_drag=gdk_cursor_new(GDK_PLUS);

        gdk_window_show (gtk_widget_get_window(widget));
        gdk_window_show (sheet->sheet_window);

        if (sheet->column_titles_visible)
        {
            size_allocate_column_title_buttons(sheet);
            gdk_window_show (sheet->column_title_window);
        }

        if (sheet->row_titles_visible)
        {
            size_allocate_row_title_buttons(sheet);
            gdk_window_show (sheet->row_title_window);
        }

        if (!gtk_widget_get_mapped (sheet->sheet_entry))
        {
            gtk_widget_show (sheet->sheet_entry);
            gtk_widget_map (sheet->sheet_entry);
        }

        if (gtk_widget_get_visible (sheet->button) &&
            !gtk_widget_get_mapped (sheet->button))
        {
            gtk_widget_show(sheet->button);
            gtk_widget_map (sheet->button);
        }

        if ((WdChild = gtk_bin_get_child(GTK_BIN(sheet->button))))
        {
            if (gtk_widget_get_visible (WdChild) &&
                !gtk_widget_get_mapped (WdChild))
            {
                gtk_widget_map (WdChild);
            }
        }

        gtk_sheet_range_draw(sheet, NULL);

        gtk_sheet_activate_cell(sheet, 
                                sheet->active_cell.row, 
                                sheet->active_cell.col);

        children = sheet->children;
        while (children)
        {
            child = children->data;
            children = children->next;

            if (gtk_widget_get_visible (child->widget) &&
                !gtk_widget_get_mapped (child->widget))
            {
                gtk_widget_map (child->widget);
                gtk_sheet_position_child(sheet, child);
            }
        }
    }
}

/*
 * gtk_sheet_unmap_handler:
 * 
 * this is the #GtkSheet widget class "unmap" signal handler
 * 
 * @param widget the #GtkSheet
 */
static void
    gtk_sheet_unmap_handler (GtkWidget * widget)
{
    GtkSheet *sheet;
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));

    sheet = GTK_SHEET (widget);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_unmap: called");
#endif

    if (gtk_widget_get_mapped (widget))
    {
        GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);

        gdk_window_hide (sheet->sheet_window);

        if (sheet->column_titles_visible)
            gdk_window_hide (sheet->column_title_window);

        if (sheet->row_titles_visible)
            gdk_window_hide (sheet->row_title_window);

        gdk_window_hide (gtk_widget_get_window(widget));

        if (gtk_widget_get_mapped (sheet->sheet_entry))
            gtk_widget_unmap (sheet->sheet_entry);

        if (gtk_widget_get_mapped (sheet->button))
            gtk_widget_unmap (sheet->button);

        children = sheet->children;
        while (children)
        {
            child = children->data;
            children = children->next;

            if (gtk_widget_get_visible (child->widget) &&
                gtk_widget_get_mapped (child->widget))
            {
                gtk_widget_unmap (child->widget);
            }
        }

    }
}

/*
 * gtk_sheet_draw_tm - draw tooltip marker
 * 
 * @param sheet
 */
static void
    gtk_sheet_draw_tooltip_marker(GtkSheet *sheet, 
                      GtkSheetArea area,
                      const gint row, const gint col)
{
    switch(area)
    {
        case ON_CELL_AREA:
            if (row <= sheet->maxallocrow && col <= sheet->maxalloccol 
                && sheet->data[row] && sheet->data[row][col])
            {
                GtkSheetCell *cell = sheet->data[row][col];

                if (cell->tooltip_markup || cell->tooltip_text)
                {
                    GdkPoint p[3];

                    gdk_gc_set_foreground (sheet->bg_gc, &sheet->tm_color);

                    p[0].x = COLUMN_LEFT_XPIXEL(sheet,col) + COLPTR(sheet, col)->width
                        - GTK_SHEET_DEFAULT_TM_SIZE;
                    p[0].y = ROW_TOP_YPIXEL(sheet,row) + 1;

                    p[1].x = p[0].x + GTK_SHEET_DEFAULT_TM_SIZE;
                    p[1].y = p[0].y;

                    p[2].x = p[1].x;
                    p[2].y = p[1].y + GTK_SHEET_DEFAULT_TM_SIZE;

                    /* draw cell tooltip marker */
                    gdk_draw_polygon(sheet->pixmap, 
                                     sheet->bg_gc,
                                     TRUE, p, 3);
                }
            }
            break;

        case ON_ROW_TITLES_AREA:
            if (0 <= row && row <= sheet->maxrow)
            {
                GtkSheetRow *rowp = ROWPTR(sheet, row);
                GdkWindow *window = sheet->row_title_window;

                if (rowp->tooltip_markup || rowp->tooltip_text)
                {
                    GdkPoint p[3];

                    gdk_gc_set_foreground (sheet->bg_gc, &sheet->tm_color);

                    p[0].x = sheet->row_title_area.width - 1
                        - GTK_SHEET_DEFAULT_TM_SIZE;
                    p[0].y = ROW_TOP_YPIXEL(sheet,row) + 1;
                    if (sheet->column_titles_visible) p[0].y -= sheet->column_title_area.height;

                    p[1].x = p[0].x + GTK_SHEET_DEFAULT_TM_SIZE;
                    p[1].y = p[0].y;

                    p[2].x = p[1].x;
                    p[2].y = p[1].y + GTK_SHEET_DEFAULT_TM_SIZE;

                    /* draw cell tooltip marker */
                    gdk_draw_polygon(window, 
                                     sheet->bg_gc,
                                     TRUE, p, 3);
                }
            }
            break;

        case ON_COLUMN_TITLES_AREA:
            if (0 <= col && col <= sheet->maxcol)
            {
                GtkSheetColumn *column = COLPTR(sheet, col);
                GdkWindow *window = sheet->column_title_window;

                if (gtk_widget_get_has_tooltip(GTK_WIDGET(column)))
                {
                    GdkPoint p[3];

                    gdk_gc_set_foreground (sheet->bg_gc, &sheet->tm_color);

                    p[0].x = COLUMN_RIGHT_XPIXEL(sheet, col) + CELL_SPACING - 1
                        - GTK_SHEET_DEFAULT_TM_SIZE;
                    if (sheet->row_titles_visible) p[0].x -= sheet->row_title_area.width;
                    p[0].y = 0;

                    p[1].x = p[0].x + GTK_SHEET_DEFAULT_TM_SIZE;
                    p[1].y = p[0].y;

                    p[2].x = p[1].x;
                    p[2].y = p[1].y + GTK_SHEET_DEFAULT_TM_SIZE;

                    /* draw cell tooltip marker */
                    gdk_draw_polygon(window, 
                                     sheet->bg_gc,
                                     TRUE, p, 3);
                }
            }
            break;

        default: return;
    }
}

static void
    gtk_sheet_cell_draw_default (GtkSheet *sheet, gint row, gint col)
{
    GtkWidget *widget;
    GdkGC *fg_gc, *bg_gc;
    GtkSheetCellAttr attributes;
    GdkRectangle area;

    g_return_if_fail (sheet != NULL);

    /* bail now if we arn't drawable yet */
    if (!gtk_widget_is_drawable (GTK_WIDGET(sheet))) return;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;
    if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) return;

    widget = GTK_WIDGET (sheet);

    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    /* select GC for background rectangle */
    gdk_gc_set_foreground (sheet->fg_gc, &attributes.foreground);
    gdk_gc_set_foreground (sheet->bg_gc, &attributes.background);

    fg_gc = sheet->fg_gc;
    bg_gc = sheet->bg_gc;

    area.x = COLUMN_LEFT_XPIXEL(sheet,col);
    area.y = ROW_TOP_YPIXEL(sheet,row);
    area.width = COLPTR(sheet, col)->width;
    area.height = ROWPTR(sheet, row)->height;

    /* fill cell background */
    gdk_draw_rectangle (sheet->pixmap,
                        bg_gc,
                        TRUE,
                        area.x, area.y,
                        area.width, area.height);

    gdk_gc_set_line_attributes (sheet->fg_gc, 1, 0, 0, 0);

    if (sheet->show_grid)
    {
        gdk_gc_set_foreground (sheet->bg_gc, &sheet->grid_color);

#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_cell_draw_default(%d,%d): x %d y %d w %d h %d",
                row, col,
                area.x, area.y, area.width, area.height);
#endif

        /* draw grid rectangle */
        gdk_draw_rectangle (sheet->pixmap,
                            sheet->bg_gc,
                            FALSE,
                            area.x, area.y,
                            area.width, area.height);
    }

    gtk_sheet_draw_tooltip_marker(sheet, ON_CELL_AREA, row, col);
}

static void
    gtk_sheet_cell_draw_border (GtkSheet *sheet, gint row, gint col, gint mask)
{
    GtkWidget *widget;
    GdkGC *fg_gc, *bg_gc;
    GtkSheetCellAttr attributes;
    GdkRectangle area;
    guint width;

    g_return_if_fail (sheet != NULL);

    /* bail now if we arn't drawable yet */
    if (!gtk_widget_is_drawable (GTK_WIDGET(sheet))) return;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;
    if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) return;

    widget = GTK_WIDGET (sheet);

    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    /* select GC for background rectangle */
    gdk_gc_set_foreground (sheet->fg_gc, &attributes.border.color);
    gdk_gc_set_foreground (sheet->bg_gc, &attributes.background);

    fg_gc = sheet->fg_gc;
    bg_gc = sheet->bg_gc;

    area.x=COLUMN_LEFT_XPIXEL(sheet,col);
    area.y=ROW_TOP_YPIXEL(sheet,row);
    area.width = sheet->column[col]->width;
    area.height=sheet->row[row].height;

    width = attributes.border.width;
    gdk_gc_set_line_attributes(sheet->fg_gc, attributes.border.width,
                               attributes.border.line_style,
                               attributes.border.cap_style,
                               attributes.border.join_style);
    if (width>0)
    {

        if (attributes.border.mask & GTK_SHEET_LEFT_BORDER & mask)
            gdk_draw_line(sheet->pixmap, sheet->fg_gc,
                          area.x, area.y-width/2,
                          area.x, area.y+area.height+width/2+1);

        if (attributes.border.mask & GTK_SHEET_RIGHT_BORDER & mask)
            gdk_draw_line(sheet->pixmap, sheet->fg_gc,
                          area.x+area.width, area.y-width/2,
                          area.x+area.width,
                          area.y+area.height+width/2+1);

        if (attributes.border.mask & GTK_SHEET_TOP_BORDER & mask)
            gdk_draw_line(sheet->pixmap, sheet->fg_gc,
                          area.x-width/2,area.y,
                          area.x+area.width+width/2+1,
                          area.y);

        if (attributes.border.mask & GTK_SHEET_BOTTOM_BORDER & mask)
            gdk_draw_line(sheet->pixmap, sheet->fg_gc,
                          area.x-width/2, area.y+area.height,
                          area.x+area.width+width/2+1,
                          area.y+area.height);
    }

}


static void
    gtk_sheet_cell_draw_label(GtkSheet *sheet, gint row, gint col)
{
    GtkWidget *widget;
    GdkRectangle area, clip_area;
    gint i;
    gint text_width, text_height, y;
    gint xoffset=0;
    gint size, sizel, sizer;
    GdkGC *fg_gc, *bg_gc;
    GtkSheetCellAttr attributes;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoRectangle logical_rect;
    PangoLayoutLine *line;
    PangoFontMetrics *metrics;
    PangoContext *context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
    gint ascent, descent, y_pos;

    char *label;

    g_return_if_fail (sheet != NULL);

    /* bail now if we aren't drawable yet */
    if (!GTK_WIDGET_DRAWABLE (sheet)) return;

    if (row < 0 || row > sheet->maxallocrow) return;
    if (col < 0 || col > sheet->maxalloccol) return;

    if (!sheet->data[row]) return;
    if (!sheet->data[row][col]) return;
    if (!sheet->data[row][col]->text || !sheet->data[row][col]->text[0]) return;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;

    /* bail now if we aren't drawable yet */
    if (!gtk_widget_is_drawable (GTK_WIDGET(sheet))) return;

    if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) return;

    widget = GTK_WIDGET(sheet);
    label = sheet->data[row][col]->text;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    /* select GC for background rectangle */
    gdk_gc_set_foreground (sheet->fg_gc, &attributes.foreground);
    gdk_gc_set_foreground (sheet->bg_gc, &attributes.background);

    fg_gc = sheet->fg_gc;
    bg_gc = sheet->bg_gc;

    area.x=COLUMN_LEFT_XPIXEL(sheet,col);
    area.y=ROW_TOP_YPIXEL(sheet,row);
    area.width = sheet->column[col]->width;
    area.height=sheet->row[row].height;

    clip_area = area;

    layout = gtk_widget_create_pango_layout (GTK_WIDGET(sheet), label);
    pango_layout_set_font_description (layout, attributes.font_desc);

    pango_layout_get_pixel_extents (layout, NULL, &rect);

    line = pango_layout_get_lines (layout)->data;
    pango_layout_line_get_extents (line, NULL, &logical_rect);

    metrics = pango_context_get_metrics(context,
                                        attributes.font_desc,
                                        pango_context_get_language(context));

    ascent = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
    descent = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;

    pango_font_metrics_unref(metrics);

    /* Align primarily for locale's ascent/descent */

    logical_rect.height /= PANGO_SCALE;
    logical_rect.y /= PANGO_SCALE;
    y_pos =  area.height - logical_rect.height;

    if (logical_rect.height > area.height)
        y_pos = (logical_rect.height - area.height - 2*CELLOFFSET) / 2;
    else if (y_pos < 0)
        y_pos = 0;
    else if (y_pos + logical_rect.height > area.height)
        y_pos = area.height - logical_rect.height;

    text_width = rect.width;
    text_height = rect.height;
    y = area.y + y_pos - CELLOFFSET;

    switch (attributes.justification)
    {
        case GTK_JUSTIFY_RIGHT:
            size=area.width;
            area.x+=area.width;
            if (!gtk_sheet_clip_text(sheet))
            {
                for (i=col-1; i>=MIN_VISIBLE_COLUMN(sheet); i--)
                {
                    if (gtk_sheet_cell_get_text(sheet, row, i)) break;
                    if (size >= text_width+CELLOFFSET) break;
                    if (i < 0 || i > sheet->maxcol) continue;

                    size += sheet->column[i]->width;
                    sheet->column[i]->right_text_column = MAX(col, sheet->column[i]->right_text_column);
                }
                area.width=size;
            }
            area.x-=size;
            xoffset+=area.width-text_width - 2 * CELLOFFSET - attributes.border.width/2;
            break;

        case GTK_JUSTIFY_CENTER:
            sizel=area.width/2;
            sizer=area.width/2;
            area.x+=area.width/2;
            if (!gtk_sheet_clip_text(sheet))
            {
                for (i=col+1; i<=MAX_VISIBLE_COLUMN(sheet); i++)
                {
                    if (gtk_sheet_cell_get_text(sheet, row, i)) break;
                    if (sizer>=text_width/2) break;
                    if (i < 0 || i > sheet->maxcol) continue;

                    sizer += sheet->column[i]->width;
                    sheet->column[i]->left_text_column = MIN(col, sheet->column[i]->left_text_column);
                }
                for (i=col-1; i>=MIN_VISIBLE_COLUMN(sheet); i--)
                {
                    if (gtk_sheet_cell_get_text(sheet, row, i)) break;
                    if (sizel>=text_width/2) break;
                    if (i < 0 || i > sheet->maxcol) continue;

                    sizel += sheet->column[i]->width;
                    sheet->column[i]->right_text_column = MAX(col, sheet->column[i]->right_text_column);
                }
                size=MIN(sizel, sizer);
            }
            area.x-=sizel;
            xoffset+= sizel - text_width/2 - CELLOFFSET;
            area.width=sizel+sizer;
            break;

        case GTK_JUSTIFY_LEFT:
        default:
            size=area.width;
            if (!gtk_sheet_clip_text(sheet))
            {
                for (i=col+1; i<=MAX_VISIBLE_COLUMN(sheet); i++)
                {
                    if (gtk_sheet_cell_get_text(sheet, row, i)) break;
                    if (size>=text_width+CELLOFFSET) break;
                    if (i < 0 || i > sheet->maxcol) continue;

                    size += sheet->column[i]->width;
                    sheet->column[i]->left_text_column = MIN(col, sheet->column[i]->left_text_column);
                }
                area.width=size;
            }
            xoffset += attributes.border.width/2;
            break;
    }

    if (!gtk_sheet_clip_text(sheet)) clip_area = area;
    gdk_gc_set_clip_rectangle(fg_gc, &clip_area);


    gdk_draw_layout (sheet->pixmap, fg_gc,
                     area.x + xoffset + CELLOFFSET,
                     y,
                     layout);

    gdk_gc_set_clip_rectangle(fg_gc, NULL);
    g_object_unref(G_OBJECT(layout));

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    area.x,
                    area.y,
                    area.x,
                    area.y,
                    area.width,
                    area.height);
}



static void
    gtk_sheet_range_draw(GtkSheet *sheet, const GtkSheetRange *range)
{
    gint i,j;
    GtkSheetRange drawing_range;
    GdkRectangle area;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_SHEET(sheet));

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_mapped(GTK_WIDGET(sheet))) return;

    if (range)
    {
        drawing_range.row0 = MAX(range->row0, MIN_VISIBLE_ROW(sheet));
        drawing_range.col0 = MAX(range->col0, MIN_VISIBLE_COLUMN(sheet));
        drawing_range.rowi = MIN(range->rowi, MAX_VISIBLE_ROW(sheet));
        drawing_range.coli = MIN(range->coli, MAX_VISIBLE_COLUMN(sheet));
    }
    else
    {
        drawing_range.row0 = MIN_VISIBLE_ROW(sheet);
        drawing_range.col0 = MIN_VISIBLE_COLUMN(sheet);
        drawing_range.rowi = MAX_VISIBLE_ROW(sheet);
        drawing_range.coli = MAX_VISIBLE_COLUMN(sheet);
    }

/*  
   gdk_draw_rectangle (sheet->pixmap,
                   GTK_WIDGET(sheet)->style->white_gc,
                   TRUE,
                   0,0,
                   sheet->sheet_window_width,sheet->sheet_window_height);
*/

    /* clear outer area beyond rightmost column */
    if (drawing_range.coli == MAX_VISIBLE_COLUMN(sheet))
    {
        if (sheet->maxcol >= 0)
        {
            area.x = COLUMN_LEFT_XPIXEL(sheet,sheet->maxcol)+
                sheet->column[sheet->maxcol]->width+1;
        }
        else
        {
            area.x = sheet->hoffset;
            if (sheet->row_titles_visible) area.x += sheet->row_title_area.width;
        }
        area.y=0;

        gdk_gc_set_foreground(sheet->fg_gc, &sheet->bg_color);

        gdk_draw_rectangle (sheet->pixmap,
                            sheet->fg_gc,
                            TRUE,
                            area.x, area.y,
                            sheet->sheet_window_width - area.x,
                            sheet->sheet_window_height);

        gdk_draw_pixmap(sheet->sheet_window,
                        gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                        sheet->pixmap,
                        area.x, area.y,
                        area.x, area.y,
                        sheet->sheet_window_width - area.x,
                        sheet->sheet_window_height);
    }

    /* clear outer area beyond last row */
    if (drawing_range.rowi == MAX_VISIBLE_ROW(sheet))
    {
        area.x=0;
        if (sheet->maxrow >= 0)
        {
            area.y = ROW_TOP_YPIXEL(sheet,sheet->maxrow)+
                sheet->row[sheet->maxrow].height+1;
        }
        else
        {
            area.y = sheet->voffset;
            if (sheet->column_titles_visible) area.y += sheet->column_title_area.height;
        }

        gdk_gc_set_foreground(sheet->fg_gc, &sheet->bg_color);

        gdk_draw_rectangle (sheet->pixmap,
                            sheet->fg_gc,
                            TRUE,
                            area.x, area.y,
                            sheet->sheet_window_width,
                            sheet->sheet_window_height - area.y);

        gdk_draw_pixmap(sheet->sheet_window,
                        gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                        sheet->pixmap,
                        area.x, area.y,
                        area.x, area.y,
                        sheet->sheet_window_width,
                        sheet->sheet_window_height - area.y);
    }

    /* draw grid and cells */
    for (i=drawing_range.row0; i<=drawing_range.rowi; i++)
    {
        for (j=drawing_range.col0; j<=drawing_range.coli; j++)
        {
            gtk_sheet_cell_draw_default(sheet, i, j);
        }
    }

    for (i=drawing_range.row0; i<=drawing_range.rowi; i++)
    {
        for (j=drawing_range.col0; j<=drawing_range.coli; j++)
        {
            gtk_sheet_cell_draw_border(sheet, i-1, j, GTK_SHEET_BOTTOM_BORDER);
            gtk_sheet_cell_draw_border(sheet, i+1, j, GTK_SHEET_TOP_BORDER);
            gtk_sheet_cell_draw_border(sheet, i, j-1, GTK_SHEET_RIGHT_BORDER);
            gtk_sheet_cell_draw_border(sheet, i, j+1, GTK_SHEET_LEFT_BORDER);
            gtk_sheet_cell_draw_border(sheet, i, j, 15);
        }
    }

    for (i=drawing_range.row0; i<=drawing_range.rowi; i++)
    {
        for (j=drawing_range.col0; j<=drawing_range.coli; j++)
        {
            if (i<=sheet->maxallocrow && j<=sheet->maxalloccol && sheet->data[i] && sheet->data[i][j])
            {
                gtk_sheet_cell_draw_label (sheet, i, j);
            }
        }
    }

    if (0 <= drawing_range.col0 && drawing_range.col0 <= sheet->maxcol)
    {
        for (i=drawing_range.row0; i<=drawing_range.rowi; i++)
        {
            for (j=sheet->column[drawing_range.col0]->left_text_column; 
                j<drawing_range.col0; j++)
            {
                if (i<=sheet->maxallocrow && j<=sheet->maxalloccol && 
                    sheet->data[i] && sheet->data[i][j])
                {
                    gtk_sheet_cell_draw_label (sheet, i, j);
                }
            }
        }
    }

    if (0 <= drawing_range.coli && drawing_range.coli <= sheet->maxcol)
    {
        for (i=drawing_range.row0; i<=drawing_range.rowi; i++)
        {
            for (j=drawing_range.coli+1; 
                j<=sheet->column[drawing_range.coli]->right_text_column; j++)
            {
                if (i<=sheet->maxallocrow && j<=sheet->maxalloccol && 
                    sheet->data[i] && sheet->data[i][j])
                {
                    gtk_sheet_cell_draw_label (sheet, i, j);
                }
            }
        }
    }

    gtk_sheet_draw_backing_pixmap(sheet, drawing_range);

    if (sheet->state != GTK_SHEET_NORMAL && 
        gtk_sheet_range_isvisible(sheet, sheet->range))
    {
        gtk_sheet_range_draw_selection(sheet, drawing_range);
    }

    if (sheet->state == GTK_STATE_NORMAL &&
        sheet->active_cell.row >= drawing_range.row0 && 
        sheet->active_cell.row <= drawing_range.rowi &&
        sheet->active_cell.col >= drawing_range.col0 &&
        sheet->active_cell.col <= drawing_range.coli)
    {
        gtk_sheet_show_active_cell(sheet);
    }
}

static void
    gtk_sheet_range_draw_selection(GtkSheet *sheet, GtkSheetRange range)
{
    GdkRectangle area;
    gint i,j;
    GtkSheetRange aux;

    if (range.col0 > sheet->range.coli || range.coli < sheet->range.col0 ||
        range.row0 > sheet->range.rowi || range.rowi < sheet->range.row0)
    {
#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_range_draw_selection: range outside");
#endif
        return;
    }

    if (!gtk_sheet_range_isvisible(sheet, range))
    {
#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_range_draw_selection: range invisible");
#endif
        return;
    }
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    aux=range;

    range.col0 = MAX(sheet->range.col0, range.col0);
    range.coli = MIN(sheet->range.coli, range.coli);
    range.row0 = MAX(sheet->range.row0, range.row0);
    range.rowi = MIN(sheet->range.rowi, range.rowi);

    range.col0 = MAX(range.col0, MIN_VISIBLE_COLUMN(sheet));
    range.coli = MIN(range.coli, MAX_VISIBLE_COLUMN(sheet));
    range.row0 = MAX(range.row0, MIN_VISIBLE_ROW(sheet));
    range.rowi = MIN(range.rowi, MAX_VISIBLE_ROW(sheet));

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_range_draw_selection: range r %d-%d c %d-%d",
            range.row0, range.rowi, range.col0, range.coli);
#endif

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            if (gtk_sheet_cell_get_state(sheet, i, j)==GTK_STATE_SELECTED &&
                GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && 
                GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
            {
                row_button_set(sheet, i);
                column_button_set(sheet, j);

                area.x = COLUMN_LEFT_XPIXEL(sheet,j);
                area.y = ROW_TOP_YPIXEL(sheet,i);
                area.width = sheet->column[j]->width;
                area.height = sheet->row[i].height;

                if (i == sheet->range.row0)
                {
                    area.y = area.y+2;
                    area.height = area.height-2;
                }
                if (i == sheet->range.rowi) area.height=area.height-3;

                if (j == sheet->range.col0)
                {
                    area.x=area.x+2;
                    area.width=area.width-2;
                }
                if (j == sheet->range.coli) area.width=area.width-3;

                if (i != sheet->active_cell.row || j != sheet->active_cell.col)
                {
                    gdk_draw_rectangle (sheet->sheet_window,
                                        sheet->xor_gc,
                                        TRUE,
                                        area.x+1,area.y+1,
                                        area.width,area.height);
                }
            }

        }
    }

    gtk_sheet_draw_border(sheet, sheet->range);
}

static void
    gtk_sheet_draw_backing_pixmap(GtkSheet *sheet, GtkSheetRange range)
{
    gint x,y,width,height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    x=COLUMN_LEFT_XPIXEL(sheet,range.col0);
    y=ROW_TOP_YPIXEL(sheet, range.row0);
    width=COLUMN_LEFT_XPIXEL(sheet, range.coli) - x;
    if (0 <= range.coli && range.coli <= sheet->maxcol) width += sheet->column[range.coli]->width;
    height=ROW_TOP_YPIXEL(sheet, range.rowi) - y;
    if (0 <= range.rowi && range.rowi <= sheet->maxrow) height += sheet->row[range.rowi].height;

    if (range.row0 == sheet->range.row0)
    {
        y = y-5;
        height = height+5;
    }
    if (range.rowi == sheet->range.rowi) height = height+5;

    if (range.col0 == sheet->range.col0)
    {
        x = x-5;
        width = width+5;
    }
    if (range.coli == sheet->range.coli) width = width+5;

    width=MIN(width, sheet->sheet_window_width-x);
    height=MIN(height, sheet->sheet_window_height-y);

    x--;
    y--;
    width += 2;
    height += 2;

    x = (sheet->row_titles_visible) ? MAX(x, sheet->row_title_area.width) : MAX(x, 0);
    y = (sheet->column_titles_visible) ? MAX(y, sheet->column_title_area.height) : MAX(y, 0);

    if (range.coli >= sheet->maxcol) width = sheet->sheet_window_width-x;
    if (range.rowi >= sheet->maxrow) height=sheet->sheet_window_height-y;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_draw_backing_pixmap: x %d y %d w %d h %d",
            x, y, width+1, height+1);
#endif

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    x, y,
                    x, y,
                    width+1,
                    height+1);
}

static inline void 
    gtk_sheet_cell_init(GtkSheetCell *cell)
{
    cell->area.x = cell->area.y = cell->area.width = cell->area.height = 0;

    cell->row = cell->col = -1;

    cell->attributes = NULL;
    cell->text = cell->link = NULL;

    cell->tooltip_markup = cell->tooltip_text = NULL;
}

static void
    gtk_sheet_cell_finalize(GtkSheet *sheet, GtkSheetCell *cell)
{
    g_return_if_fail(cell != NULL);

    if (cell->text)
    {
        g_free(cell->text);
        cell->text = NULL;

        if (GTK_IS_OBJECT(sheet) && G_OBJECT(sheet)->ref_count > 0)
            g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CLEAR_CELL], 0, 
                          cell->row, cell->col);
    }

    if (cell->link)
    {
        cell->link = NULL;
    }

    if (cell->tooltip_markup)
    {
        g_free(cell->tooltip_markup);
        cell->tooltip_markup = NULL;
    }

    if (cell->tooltip_text)
    {
        g_free(cell->tooltip_text);
        cell->tooltip_text = NULL;
    }

    if (cell->attributes)
    {
        g_free(cell->attributes);
        cell->attributes = NULL;
    }
}

static GtkSheetCell *
    gtk_sheet_cell_new(void)
{
    GtkSheetCell *cell = g_new(GtkSheetCell, 1);
    gtk_sheet_cell_init(cell);
    return (cell);
}

/**
 * gtk_sheet_set_cell_text:
 * @sheet: a #GtkSheet.
 * @row: row_number
 * @col: column number
 * @text: cell text
 *
 * Set cell contents and allocate memory if needed. No 
 * justifcation is made. attributes and links remain unchanged.
 */
void
    gtk_sheet_set_cell_text(GtkSheet *sheet, gint row, gint col, const gchar *text)
{
    GtkSheetCellAttr attributes;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col > sheet->maxcol || row > sheet->maxrow) return;
    if (col < 0 || row < 0) return;

    gtk_sheet_get_attributes(sheet, row, col, &attributes);
    gtk_sheet_set_cell(sheet, row, col, attributes.justification, text);
}

/**
 * gtk_sheet_set_cell:
 * @sheet: a #GtkSheet.
 * @row: row_number
 * @col: column number
 * @justification: a #GtkJustification :GTK_JUSTIFY_LEFT, RIGHT, CENTER
 * @text: cell text
 *
 * Set cell contents and allocate memory if needed.
 */
void
    gtk_sheet_set_cell(GtkSheet *sheet, gint row, gint col,
                       GtkJustification justification,
                       const gchar *text)
{
    GtkSheetCell *cell;
    GtkSheetRange range;
    gint text_width;
    GtkSheetCellAttr attributes;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));
    if (col > sheet->maxcol || row > sheet->maxrow) return;
    if (col < 0 || row < 0) return;

    CheckCellData(sheet, row, col);

    cell = sheet->data[row][col];

    gtk_sheet_get_attributes(sheet, row, col, &attributes);
    attributes.justification = justification;
    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_set_cell: text = %s", text ? text : "");
#endif

    if (cell->text)
    {
        g_free(cell->text);
        cell->text = NULL;
    }

    if (text) cell->text=g_strdup(text);

    if (attributes.is_visible)
    {
        text_width = 0;

        if (cell->text && cell->text[0])
        {
            text_width = STRING_WIDTH(GTK_WIDGET(sheet), 
                                      attributes.font_desc, cell->text);
        }

        range.row0 = row;
        range.rowi = row;
        range.col0 = sheet->view.col0;
        range.coli = sheet->view.coli;

        if (gtk_sheet_autoresize(sheet) &&
            text_width > COLPTR(sheet, col)->width - 2*CELLOFFSET - attributes.border.width)
        {
            gtk_sheet_set_column_width(sheet, col, text_width+2*CELLOFFSET+attributes.border.width);
            GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_REDRAW_PENDING);
        }
        else
        {
            if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
        }
    }
    g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CHANGED], 0, row, col);
}

/**
 * gtk_sheet_cell_clear:
 * @sheet: a #GtkSheet.
 * @row: row_number
 * @column: column number
 *
 * Clear cell contents.
 */
void
    gtk_sheet_cell_clear (GtkSheet *sheet, gint row, gint column)
{
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));
    if (column > sheet->maxcol || row > sheet->maxrow) return;
    if (column > sheet->maxalloccol || row > sheet->maxallocrow) return;
    if (column < 0 || row < 0) return;

    range.row0 = row;
    range.rowi = row;
    range.col0 = sheet->view.col0;
    range.coli = sheet->view.coli;

    gtk_sheet_real_cell_clear(sheet, row, column, FALSE);

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_range_draw(sheet, &range);
    }
}

/**
 * gtk_sheet_cell_delete:
 * @sheet: a #GtkSheet.
 * @row: row_number
 * @column: column number
 *
 * Clear cell contents and remove links.
 */
void
    gtk_sheet_cell_delete (GtkSheet *sheet, gint row, gint column)
{
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));
    if (column > sheet->maxcol || row > sheet->maxrow) return;
    if (column > sheet->maxalloccol || row > sheet->maxallocrow) return;
    if (column < 0 || row < 0) return;

    range.row0 = row;
    range.rowi = row;
    range.col0 = sheet->view.col0;
    range.coli = sheet->view.coli;

    gtk_sheet_real_cell_clear(sheet, row, column, TRUE);

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
        gtk_sheet_range_draw(sheet, &range);
    }
}

static void
    gtk_sheet_real_cell_clear (GtkSheet *sheet, 
                               gint row, gint column, 
                               gboolean delete)
{
    GtkSheetCell *cell;

    if (row > sheet->maxallocrow || column > sheet->maxalloccol) return;

    if (!sheet->data[row]) return;

    cell = sheet->data[row][column];
    if (!cell) return;

    if (cell->text)
    {
        g_free(cell->text);
        cell->text = NULL;

        if (GTK_IS_OBJECT(sheet) && G_OBJECT(sheet)->ref_count > 0)
            g_signal_emit(GTK_OBJECT(sheet),sheet_signals[CLEAR_CELL], 0, row, column);
    }

    if (cell->link)
    {
        cell->link = NULL;
    }

    if (cell->tooltip_markup)
    {
        g_free(cell->tooltip_markup);
        cell->tooltip_markup = NULL;
    }

    if (cell->tooltip_text)
    {
        g_free(cell->tooltip_text);
        cell->tooltip_text = NULL;
    }

    if (delete)
    {
        gtk_sheet_cell_finalize(sheet, cell);

        g_free(cell);
        sheet->data[row][column] = NULL;
    }
}

/**
 * gtk_sheet_range_clear:
 * @sheet: a #GtkSheet.
 * @range: a #GtkSheetRange
 *
 * Clear range contents. If range==NULL the whole sheet will be cleared.
 */
void
    gtk_sheet_range_clear (GtkSheet *sheet, const GtkSheetRange *range)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    gtk_sheet_real_range_clear(sheet, range, FALSE);
}

/**
 * gtk_sheet_range_delete:
 * @sheet: a #GtkSheet.
 * @range: a #GtkSheetRange
 *
 * Clear range contents and remove links.
 * FIXME:: if range==NULL whole sheet is deleted?
 */
void
    gtk_sheet_range_delete (GtkSheet *sheet, const GtkSheetRange *range)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    gtk_sheet_real_range_clear(sheet, range, TRUE);
}

static void
    gtk_sheet_real_range_clear (GtkSheet *sheet, const GtkSheetRange *range,
                                gboolean delete)
{
    gint i, j;
    GtkSheetRange clear;

    if (!range)
    {
        clear.row0=0;
        clear.rowi=sheet->maxallocrow;
        clear.col0=0;
        clear.coli=sheet->maxalloccol;
    }
    else
    {
        clear=*range;
    }

    clear.row0=MAX(clear.row0, 0);
    clear.col0=MAX(clear.col0, 0);
    clear.rowi=MIN(clear.rowi, sheet->maxallocrow);
    clear.coli=MIN(clear.coli, sheet->maxalloccol);

    for (i=clear.row0; i<=clear.rowi; i++)
    {
        for (j=clear.col0; j<=clear.coli; j++)
        {
            gtk_sheet_real_cell_clear(sheet, i, j, delete);
        }
    }

    gtk_sheet_range_draw(sheet, NULL);
}

/**
 * gtk_sheet_cell_get_text:
 * @sheet: a #GtkSheet
 * @row: row number
 * @col: column number
 *
 * Get cell text.
 *
 * Returns: a pointer to the cell text, or NULL. 
 * Do not modify or free it.
 */
gchar *
    gtk_sheet_cell_get_text (GtkSheet *sheet, gint row, gint col)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    if (col > sheet->maxcol || row > sheet->maxrow) return(NULL);
    if (col < 0 || row < 0) return(NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol) return(NULL);

    if (!sheet->data[row]) return(NULL);
    if (!sheet->data[row][col]) return(NULL);
    if (!sheet->data[row][col]->text) return(NULL);
    if (!sheet->data[row][col]->text[0]) return(NULL);

    return(sheet->data[row][col]->text);
}

/**
 * gtk_sheet_link_cell:
 * @sheet: a #GtkSheet
 * @row: row number
 * @col: column number
 * @link: pointer linked to the cell
 *
 * Link pointer to a cell.
 */
void
    gtk_sheet_link_cell(GtkSheet *sheet, gint row, gint col, gpointer link)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col > sheet->maxcol || row > sheet->maxrow) return;
    if (col < 0 || row < 0) return;

    CheckCellData(sheet, row, col);

    sheet->data[row][col]->link = link;
}

/**
 * gtk_sheet_get_link:
 * @sheet: a #GtkSheet
 * @row: row number
 * @col: column number
 *
 * Get link pointer from a cell.
 *
 * Returns: pointer linked to the cell
 */
gpointer
    gtk_sheet_get_link(GtkSheet *sheet, gint row, gint col)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);
    if (col > sheet->maxcol || row > sheet->maxrow) return (NULL);
    if (col < 0 || row < 0) return (NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol) return (NULL);
    if (!sheet->data[row]) return (NULL); /* Added by Chris Howell */
    if (!sheet->data[row][col]) return (NULL); /* Added by Bob Lissner */

    return(sheet->data[row][col]->link);
}

/**
 * gtk_sheet_remove_link:
 * @sheet: a #GtkSheet
 * @row: row number
 * @col: column number
 *
 * Remove link pointer from a cell.
 */
void
    gtk_sheet_remove_link(GtkSheet *sheet, gint row, gint col)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));
    if (col > sheet->maxcol || row > sheet->maxrow) return;
    if (col < 0 || row < 0) return;

    /* Fixed by Andreas Voegele */
    if (row <= sheet->maxallocrow && col <= sheet->maxalloccol &&
        sheet->data[row] && sheet->data[row][col] &&
        sheet->data[row][col]->link)
        sheet->data[row][col]->link = NULL;
}

/**
 * gtk_sheet_cell_get_state:
 * @sheet: a #GtkSheet
 * @row: row number
 * @col: column number
 *
 * Get status of a cell.
 *
 * Returns: a #GtkStateType:
 * GTK_SHEET_NORMAL,GTK_SHEET_ROW_SELECTED,GTK_SHEET_COLUMN_SELECTED,GTK_SHEET_RANGE_SELECTED
 */
GtkStateType
    gtk_sheet_cell_get_state (GtkSheet *sheet, gint row, gint col)
{
    gint state;
    GtkSheetRange *range;

    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);
    if (col > sheet->maxcol || row > sheet->maxrow) return (0);
    if (col < 0 || row < 0) return (0);

    state = sheet->state;
    range = &sheet->range;

    switch (state)
    {
        case GTK_SHEET_NORMAL:
            return (GTK_STATE_NORMAL);
            break;
        case GTK_SHEET_ROW_SELECTED:
            if (row>=range->row0 && row<=range->rowi)
                return (GTK_STATE_SELECTED);
            break;
        case GTK_SHEET_COLUMN_SELECTED:
            if (col>=range->col0 && col<=range->coli)
                return (GTK_STATE_SELECTED);
            break;
        case GTK_SHEET_RANGE_SELECTED:
            if (row >= range->row0 && row <= range->rowi && \
                col >= range->col0 && col <= range->coli)
                return (GTK_STATE_SELECTED);
            break;
    }
    return (GTK_STATE_NORMAL);
}

/**
 * gtk_sheet_get_pixel_info:
 * @sheet: a #GtkSheet
 * @x: x coordinate
 * @y: y coordinate
 * @row: cell row number
 * @column: cell column number
 *
 * Get row and column correspondig to the given position in the screen.
 *
 * Returns: TRUE(success) or FALSE(failure)
 */
gboolean
    gtk_sheet_get_pixel_info (GtkSheet *sheet,
                              gint x, gint y,
                              gint *row, gint *column)
{
    gint trow, tcol;

    *row = *column = -1;  /* init all output vars */

    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    trow = ROW_FROM_YPIXEL (sheet, y);
    *row = trow;

    tcol = COLUMN_FROM_XPIXEL (sheet, x);
    *column = tcol;

    /* bounds checking, return false if the user clicked
     * on a blank area */

    if (trow < 0 || trow > sheet->maxrow) return (FALSE);
    if (tcol < 0 || tcol > sheet->maxcol) return (FALSE);

    return (TRUE);
}

/**
 * gtk_sheet_get_cell_area:
 * @sheet: a #GtkSheet
 * @row: row number
 * @column: column number
 * @area: a #GdkRectangle area of the cell
 *
 * Get area of a given cell.
 *
 * Returns: TRUE(success) or FALSE(failure)
 */
gboolean
    gtk_sheet_get_cell_area  (GtkSheet * sheet,
                              gint row,
                              gint column,
                              GdkRectangle *area)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    if (row > sheet->maxrow || column > sheet->maxcol) return (FALSE);

    area->x = (column == -1) ? 0 : (COLUMN_LEFT_XPIXEL(sheet, column) -
                                    (sheet->row_titles_visible ? sheet->row_title_area.width : 0));
    area->y = (row == -1) ? 0 : (ROW_TOP_YPIXEL(sheet, row) -
                                 (sheet->column_titles_visible ? sheet->column_title_area.height : 0));
    area->width= (column == -1) ? sheet->row_title_area.width : sheet->column[column]->width;
    area->height= (row == -1) ? sheet->column_title_area.height : sheet->row[row].height;

/*
  if(row < 0 || column < 0) return FALSE;

  area->x = COLUMN_LEFT_XPIXEL(sheet, column);
  area->y = ROW_TOP_YPIXEL(sheet, row);
  if(sheet->row_titles_visible)
           area->x -= sheet->row_title_area.width;
  if(sheet->column_titles_visible)
           area->y -= sheet->column_title_area.height;

  area->width=sheet->column[column]->width;
  area->height=sheet->row[row].height;
*/
    return (TRUE);
}

/**
 * gtk_sheet_set_active_cell:
 * @sheet: a #GtkSheet
 * @row: row number
 * @column: column number
 *
 * Set active cell where the cell entry will be displayed . 
 *
 * Returns: FALSE if current cell can't be deactivated or
 * requested cell can't be activated
 */
gboolean
    gtk_sheet_set_active_cell (GtkSheet *sheet, gint row, gint column)
{
    g_return_val_if_fail (sheet != NULL, 0);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), 0);

    if (row < 0 || column < 0) return (FALSE);
    if (row > sheet->maxrow || column > sheet->maxcol) return (FALSE);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        if (!gtk_sheet_deactivate_cell(sheet)) return (FALSE);
    }

    sheet->active_cell.row = row;
    sheet->active_cell.col = column;

    if (!gtk_sheet_activate_cell(sheet, row, column)) return (FALSE);

    return (TRUE);
}

/**
 * gtk_sheet_get_active_cell:
 * @sheet: a #GtkSheet
 * @row: row number
 * @column: column number
 *
 * Store the coordinates of the active cell in row,col. 
 * If (row<0 || col<0) then there was no active cell in the 
 * sheet. 
 */
void
    gtk_sheet_get_active_cell (GtkSheet *sheet, gint *row, gint *column)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    *row = sheet->active_cell.row;
    *column = sheet->active_cell.col;
}


/*
 * gtk_sheet_entry_changed_handler:
 * 
 * this is the sheet_entry editable "changed" signal handler 
 *  
 * The signal is emited when typing into the entry, or changing 
 * its content. 
 *  
 * Its main purpose is to update the cell data text 
 * 
 * @param widget the sheet_entry widget
 * @param data the #GtkSheet, passed on signal creation
 */
static void
    gtk_sheet_entry_changed_handler(GtkWidget *widget, gpointer data)
{
    GtkSheet *sheet;
    gint row,col;
    char *text;

    g_return_if_fail (data != NULL);
    g_return_if_fail (GTK_IS_SHEET (data));

    sheet = GTK_SHEET(data);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_entry_changed_handler: called");
#endif

    if (!gtk_widget_get_visible(gtk_sheet_get_entry_widget(sheet))) return;
    if (sheet->state != GTK_STATE_NORMAL) return;

    row=sheet->active_cell.row;
    col=sheet->active_cell.col;

    if (row<0 || col<0) return;

    sheet->active_cell.row=-1;
    sheet->active_cell.col=-1;

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    text = gtk_sheet_get_entry_text(sheet);
    gtk_sheet_set_cell_text(sheet, row, col, text);
    g_free(text);

    if (sheet->freeze_count == 0)
        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    sheet->active_cell.row=row;;
    sheet->active_cell.col=col;

}


static gboolean
    gtk_sheet_deactivate_cell(GtkSheet *sheet)
{
    gboolean veto = TRUE;
    gint row,col;

    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (row < 0 || row > sheet->maxrow) return(TRUE);
    if (col < 0 || col > sheet->maxcol) return(TRUE);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return (FALSE);
    if (sheet->state != GTK_SHEET_NORMAL) return (FALSE);

    _gtkextra_signal_emit(GTK_OBJECT(sheet),sheet_signals[DEACTIVATE],
                          row, col, &veto);

    if (!veto)
    {
        sheet->active_cell.row = row;
        sheet->active_cell.col = col;

        return (FALSE);
    }

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_deactivate_cell: called");
#endif

    gtk_sheet_entry_signal_disconnect_by_func(sheet, 
                                              (GtkSignalFunc) gtk_sheet_entry_changed_handler);

    gtk_sheet_hide_active_cell(sheet);

    sheet->active_cell.row = -1;  /* reset before signal emission, to prevent recursion */
    sheet->active_cell.col = -1;


    if (GTK_SHEET_REDRAW_PENDING(sheet))
    {
        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_REDRAW_PENDING);
        gtk_sheet_range_draw(sheet, NULL);
    }

    return (TRUE);
}

static void
    gtk_sheet_hide_active_cell(GtkSheet *sheet)
{
    const char *text;
    gint row,col;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;

    if (sheet->freeze_count == 0)
        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    text = gtk_sheet_get_entry_text(sheet);

    /* todo: compare with existing text, only notify if text changes */
    gtk_sheet_set_cell_text(sheet, row, col, text);
    g_signal_emit(GTK_OBJECT(sheet),sheet_signals[SET_CELL], 0, row, col);

    column_button_release(sheet, col);
    row_button_release(sheet, row);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_hide_active_cell: called, text = %s", text ? text : "");
#endif

    /* Problem: unmap a GtkSpinButton will fire "changed" with value "0" 
    gtk_sheet_entry_signal_disconnect_by_func(sheet, 
                                              (GtkSignalFunc) gtk_sheet_entry_changed_handler);
    */

    gtk_widget_unmap(sheet->sheet_entry);

    gdk_draw_pixmap(sheet->sheet_window,
                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                    sheet->pixmap,
                    COLUMN_LEFT_XPIXEL(sheet,col)-1,
                    ROW_TOP_YPIXEL(sheet,row)-1,
                    COLUMN_LEFT_XPIXEL(sheet,col)-1,
                    ROW_TOP_YPIXEL(sheet,row)-1,
                    sheet->column[col]->width+4,
                    sheet->row[row].height+4);

    gtk_widget_grab_focus(GTK_WIDGET(sheet));

    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(sheet->sheet_entry), GTK_VISIBLE);

    g_free(text);
}

static gboolean
    gtk_sheet_activate_cell(GtkSheet *sheet, gint row, gint col)
{
    gboolean veto = TRUE;

    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if (row < 0 || col < 0) return (FALSE);
    if (row > sheet->maxrow || col > sheet->maxcol) return (FALSE);

/* 
   _gtkextra_signal_emit(GTK_OBJECT(sheet),sheet_signals[ACTIVATE], row, col, &veto);
    if(!gtk_widget_get_realized(GTK_WIDGET(sheet))) return veto;
    if (!veto) return FALSE;
*/

    if (sheet->state != GTK_SHEET_NORMAL)
    {
        sheet->state=GTK_SHEET_NORMAL;
        gtk_sheet_real_unselect_range(sheet, NULL);
    }

    sheet->range.row0 = row;
    sheet->range.col0 = col;
    sheet->range.rowi = row;
    sheet->range.coli = col;

    sheet->active_cell.row = row;
    sheet->active_cell.col = col;

    sheet->selection_cell.row = row;
    sheet->selection_cell.col = col;

    row_button_set(sheet, row);
    column_button_set(sheet, col);

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
    gtk_sheet_show_active_cell(sheet);

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_activate_cell: called");
#endif

    gtk_sheet_entry_signal_connect_changed(sheet,
                                           (GtkSignalFunc) gtk_sheet_entry_changed_handler);

    _gtkextra_signal_emit(GTK_OBJECT(sheet),sheet_signals[ACTIVATE], row, col, &veto);

    return (TRUE);
}

static void
    gtk_sheet_show_active_cell(GtkSheet *sheet)
{
    GtkSheetCell *cell;
    GtkWidget *sheet_entry;
    GtkSheetCellAttr attributes;
    gchar *text = NULL;
    gchar *old_text;
    GtkJustification justification;
    gint row, col;
    gboolean editable;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (row < 0 || col < 0) return;
    if (row > sheet->maxrow || col > sheet->maxcol) return;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (sheet->state != GTK_SHEET_NORMAL) return;
    if (GTK_SHEET_IN_SELECTION(sheet)) return;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_show_active_cell: called");
#endif

    GTK_WIDGET_SET_FLAGS(GTK_WIDGET(sheet->sheet_entry), GTK_VISIBLE);

    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    justification = GTK_JUSTIFY_LEFT;

    if (gtk_sheet_justify_entry(sheet))
        justification = attributes.justification;

    if (row <= sheet->maxallocrow && col <= sheet->maxalloccol)
    {
        if (sheet->data[row])
        {
            if (sheet->data[row][col])
            {
                cell = sheet->data[row][col];
                if (cell->text)
                    text = g_strdup(cell->text);
            }
        }
    }

    if (!text) text = g_strdup("");

    sheet_entry = gtk_sheet_get_entry(sheet);

    if (GTK_IS_ENTRY(sheet_entry))
    {
        gtk_entry_set_visibility(GTK_ENTRY(sheet_entry), attributes.is_visible);
    }

    editable = !(gtk_sheet_locked(sheet) || !attributes.is_editable || COLPTR(sheet, col)->is_readonly);
    gtk_sheet_set_entry_editable(sheet, editable);
    g_debug("activate: set to %s", text ? text : "NULL");

    old_text = gtk_sheet_get_entry_text(sheet);
    if (old_text && strcmp(old_text, text) != 0)
    {
        gtk_sheet_set_entry_text(sheet, text);
    }
    else
    {
        if (GTK_IS_ITEM_ENTRY(sheet_entry))
        {
              /* 5.8.2010/fp - the code below has no effect in GTK 2.18.9,
                 a right justified editable will pop to left justification
                 as soon as something gets selected, and will
                 pop back to right aligment, as soon as the
                 cursor ist moved. When this happens, the
                 justification value in the editable is correct. */

            gtk_item_entry_set_justification(GTK_ITEM_ENTRY(sheet_entry), justification);
        }
    }

    gtk_sheet_entry_set_max_size(sheet);
    gtk_sheet_size_allocate_entry(sheet);

    gtk_widget_map(sheet->sheet_entry);
    gtk_sheet_draw_active_cell(sheet);

    gtk_widget_grab_focus(GTK_WIDGET(sheet_entry));

    g_free(text);
    g_free(old_text);
}

static void
    gtk_sheet_draw_active_cell(GtkSheet *sheet)
{
    gint row, col;

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;

    if (!gtk_sheet_cell_isvisible(sheet, row, col)) return;

    row_button_set(sheet, row);
    column_button_set(sheet, col);

    gtk_sheet_draw_backing_pixmap(sheet, sheet->range);
    gtk_sheet_draw_border(sheet, sheet->range);
}


static void
    gtk_sheet_make_backing_pixmap (GtkSheet *sheet, guint width, guint height)
{
    gint pixmap_width, pixmap_height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    if (width == 0 && height == 0)
    {
        width=sheet->sheet_window_width+80;
        height=sheet->sheet_window_height+80;
    }

    if (!sheet->pixmap)
    {
        /* allocate */
        sheet->pixmap = gdk_pixmap_new (sheet->sheet_window,
                                        width, height,
                                        -1);
        if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, NULL);
    }
    else
    {
        /* reallocate if sizes don't match */
        gdk_window_get_size (sheet->pixmap, &pixmap_width, &pixmap_height);

        if ((pixmap_width != width) || (pixmap_height != height))
        {
            g_object_unref(G_OBJECT(sheet->pixmap));
            sheet->pixmap = gdk_pixmap_new (sheet->sheet_window,
                                            width, height,
                                            -1);
            if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, NULL);
        }
    }
}

static void
    gtk_sheet_new_selection(GtkSheet *sheet, GtkSheetRange *range)
{
    gint i,j, mask1, mask2;
    gint state, selected;
    gint x,y,width,height;
    GtkSheetRange new_range, aux_range;

    g_return_if_fail (sheet != NULL);

    if (range==NULL) range=&sheet->range;

    new_range=*range;

    range->row0=MIN(range->row0, sheet->range.row0);
    range->rowi=MAX(range->rowi, sheet->range.rowi);
    range->col0=MIN(range->col0, sheet->range.col0);
    range->coli=MAX(range->coli, sheet->range.coli);

    range->row0=MAX(range->row0, MIN_VISIBLE_ROW(sheet));
    range->rowi=MIN(range->rowi, MAX_VISIBLE_ROW(sheet));
    range->col0=MAX(range->col0, MIN_VISIBLE_COLUMN(sheet));
    range->coli=MIN(range->coli, MAX_VISIBLE_COLUMN(sheet));

    aux_range.row0=MAX(new_range.row0, MIN_VISIBLE_ROW(sheet));
    aux_range.rowi=MIN(new_range.rowi, MAX_VISIBLE_ROW(sheet));
    aux_range.col0=MAX(new_range.col0, MIN_VISIBLE_COLUMN(sheet));
    aux_range.coli=MIN(new_range.coli, MAX_VISIBLE_COLUMN(sheet));

    for (i=range->row0; i<=range->rowi; i++)
    {
        for (j=range->col0; j<=range->coli; j++)
        {
            state=gtk_sheet_cell_get_state(sheet, i, j);
            selected=(i<=new_range.rowi && i>=new_range.row0 &&
                      j<=new_range.coli && j>=new_range.col0) ? TRUE : FALSE;

            if (state==GTK_STATE_SELECTED && selected &&
                GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)) &&
                (i==sheet->range.row0 || i==sheet->range.rowi ||
                 j==sheet->range.col0 || j==sheet->range.coli ||
                 i==new_range.row0 || i==new_range.rowi ||
                 j==new_range.col0 || j==new_range.coli))
            {

                mask1 = i==sheet->range.row0 ? 1 : 0;
                mask1 = i==sheet->range.rowi ? mask1+2 : mask1;
                mask1 = j==sheet->range.col0 ? mask1+4 : mask1;
                mask1 = j==sheet->range.coli ? mask1+8 : mask1;

                mask2 = i==new_range.row0 ? 1 : 0;
                mask2 = i==new_range.rowi ? mask2+2 : mask2;
                mask2 = j==new_range.col0 ? mask2+4 : mask2;
                mask2 = j==new_range.coli ? mask2+8 : mask2;

                if (mask1 != mask2)
                {
                    x=COLUMN_LEFT_XPIXEL(sheet,j);
                    y=ROW_TOP_YPIXEL(sheet, i);
                    width=COLUMN_LEFT_XPIXEL(sheet, j)-x+sheet->column[j]->width;
                    height=ROW_TOP_YPIXEL(sheet, i)-y+sheet->row[i].height;

                    if (i==sheet->range.row0)
                    {
                        y=y-3;
                        height=height+3;
                    }
                    if (i==sheet->range.rowi) height=height+3;
                    if (j==sheet->range.col0)
                    {
                        x=x-3;
                        width=width+3;
                    }
                    if (j==sheet->range.coli) width=width+3;

                    gdk_draw_pixmap(sheet->sheet_window,
                                    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                                    sheet->pixmap,
                                    x+1,
                                    y+1,
                                    x+1,
                                    y+1,
                                    width,
                                    height);

                    if (i != sheet->active_cell.row || j != sheet->active_cell.col)
                    {
                        x=COLUMN_LEFT_XPIXEL(sheet,j);
                        y=ROW_TOP_YPIXEL(sheet, i);
                        width=COLUMN_LEFT_XPIXEL(sheet, j)-x+sheet->column[j]->width;
                        height=ROW_TOP_YPIXEL(sheet, i)-y+sheet->row[i].height;

                        if (i==new_range.row0)
                        {
                            y=y+2;
                            height=height-2;
                        }
                        if (i==new_range.rowi) height=height-3;
                        if (j==new_range.col0)
                        {
                            x=x+2;
                            width=width-2;
                        }
                        if (j==new_range.coli) width=width-3;

                        gdk_draw_rectangle (sheet->sheet_window,
                                            sheet->xor_gc,
                                            TRUE,
                                            x+1,y+1,
                                            width,height);
                    }
                }
            }
        }
    }

    for (i=range->row0; i<=range->rowi; i++)
    {
        for (j=range->col0; j<=range->coli; j++)
        {
            state=gtk_sheet_cell_get_state(sheet, i, j);
            selected=(i<=new_range.rowi && i>=new_range.row0 &&
                      j<=new_range.coli && j>=new_range.col0) ? TRUE : FALSE;

            if (state == GTK_STATE_SELECTED && !selected &&
                GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) &&
                GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
            {
                x=COLUMN_LEFT_XPIXEL(sheet,j);
                y=ROW_TOP_YPIXEL(sheet, i);
                width=COLUMN_LEFT_XPIXEL(sheet, j)-x+sheet->column[j]->width;
                height=ROW_TOP_YPIXEL(sheet, i)-y+sheet->row[i].height;

                if (i==sheet->range.row0)
                {
                    y=y-3;
                    height=height+3;
                }
                if (i==sheet->range.rowi) height=height+3;
                if (j==sheet->range.col0)
                {
                    x=x-3;
                    width=width+3;
                }
                if (j==sheet->range.coli) width=width+3;

                gdk_draw_pixmap(sheet->sheet_window,
                                gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                                sheet->pixmap,
                                x+1,
                                y+1,
                                x+1,
                                y+1,
                                width,
                                height);
            }
        }
    }

    for (i=range->row0; i<=range->rowi; i++)
    {
        for (j=range->col0; j<=range->coli; j++)
        {
            state=gtk_sheet_cell_get_state(sheet, i, j);
            selected=(i<=new_range.rowi && i>=new_range.row0 &&
                      j<=new_range.coli && j>=new_range.col0) ? TRUE : FALSE;

            if (state!=GTK_STATE_SELECTED && selected &&
                GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)) &&
                (i != sheet->active_cell.row || j != sheet->active_cell.col))
            {
                x=COLUMN_LEFT_XPIXEL(sheet,j);
                y=ROW_TOP_YPIXEL(sheet, i);
                width=COLUMN_LEFT_XPIXEL(sheet, j)-x+sheet->column[j]->width;
                height=ROW_TOP_YPIXEL(sheet, i)-y+sheet->row[i].height;

                if (i==new_range.row0)
                {
                    y=y+2;
                    height=height-2;
                }
                if (i==new_range.rowi) height=height-3;
                if (j==new_range.col0)
                {
                    x=x+2;
                    width=width-2;
                }
                if (j==new_range.coli) width=width-3;

                gdk_draw_rectangle (sheet->sheet_window,
                                    sheet->xor_gc,
                                    TRUE,
                                    x+1,y+1,
                                    width,height);
            }
        }
    }

    for (i=aux_range.row0; i<=aux_range.rowi; i++)
    {
        for (j=aux_range.col0; j<=aux_range.coli; j++)
        {
            if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
            {
                state=gtk_sheet_cell_get_state(sheet, i, j);

                mask1 = i==sheet->range.row0 ? 1 : 0;
                mask1 = i==sheet->range.rowi ? mask1+2 : mask1;
                mask1 = j==sheet->range.col0 ? mask1+4 : mask1;
                mask1 = j==sheet->range.coli ? mask1+8 : mask1;

                mask2 = i==new_range.row0 ? 1 : 0;
                mask2 = i==new_range.rowi ? mask2+2 : mask2;
                mask2 = j==new_range.col0 ? mask2+4 : mask2;
                mask2 = j==new_range.coli ? mask2+8 : mask2;
                if (mask2!=mask1 || (mask2==mask1 && state!=GTK_STATE_SELECTED))
                {
                    x=COLUMN_LEFT_XPIXEL(sheet,j);
                    y=ROW_TOP_YPIXEL(sheet, i);
                    width=sheet->column[j]->width;
                    height=sheet->row[i].height;
                    if (mask2 & 1)
                        gdk_draw_rectangle (sheet->sheet_window,
                                            sheet->xor_gc,
                                            TRUE,
                                            x+1,y-1,
                                            width,3);

                    if (mask2 & 2)
                        gdk_draw_rectangle (sheet->sheet_window,
                                            sheet->xor_gc,
                                            TRUE,
                                            x+1,y+height-1,
                                            width,3);

                    if (mask2 & 4)
                        gdk_draw_rectangle (sheet->sheet_window,
                                            sheet->xor_gc,
                                            TRUE,
                                            x-1,y+1,
                                            3,height);

                    if (mask2 & 8)
                        gdk_draw_rectangle (sheet->sheet_window,
                                            sheet->xor_gc,
                                            TRUE,
                                            x+width-1,y+1,
                                            3,height);
                }
            }
        }
    }

    *range=new_range;
    gtk_sheet_draw_corners(sheet, new_range);
}

static void
    gtk_sheet_draw_border (GtkSheet *sheet, GtkSheetRange new_range)
{
    GtkWidget *widget;
    GdkRectangle area;
    gint i;
    gint x,y,width,height;

    widget = GTK_WIDGET(sheet);

    x = COLUMN_LEFT_XPIXEL(sheet,new_range.col0);
    y = ROW_TOP_YPIXEL(sheet,new_range.row0);
    width = COLUMN_RIGHT_XPIXEL(sheet,new_range.coli) - x;
    height = ROW_BOTTOM_YPIXEL(sheet,new_range.rowi) - y;

    area.x = COLUMN_LEFT_XPIXEL(sheet, MIN_VISIBLE_COLUMN(sheet));
    area.y = ROW_TOP_YPIXEL(sheet, MIN_VISIBLE_ROW(sheet));
    area.width = sheet->sheet_window_width;
    area.height = sheet->sheet_window_height;

    if (x<0)
    {
        width=width+x;
        x=0;
    }
    if (width>area.width) width=area.width+10;
    if (y<0)
    {
        height=height+y;
        y=0;
    }
    if (height>area.height) height=area.height+10;

    gdk_gc_set_clip_rectangle(sheet->xor_gc, &area);

    for (i=-1; i<=1; ++i)
    {
        gdk_draw_rectangle (sheet->sheet_window,
                            sheet->xor_gc,
                            FALSE,
                            x+i, y+i,
                            width-2*i, height-2*i);
    }

    gdk_gc_set_clip_rectangle(sheet->xor_gc, NULL);
    gtk_sheet_draw_corners(sheet, new_range);
}

static void
    gtk_sheet_draw_corners(GtkSheet *sheet, GtkSheetRange range)
{
    gint x,y;
    guint width = 1;

    if (gtk_sheet_cell_isvisible(sheet, range.row0, range.col0))
    {
        x=COLUMN_LEFT_XPIXEL(sheet,range.col0);
        y=ROW_TOP_YPIXEL(sheet,range.row0);
        gdk_draw_pixmap(sheet->sheet_window,
                        gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                        sheet->pixmap,
                        x-1,
                        y-1,
                        x-1,
                        y-1,
                        3,
                        3);
        gdk_draw_rectangle (sheet->sheet_window,
                            sheet->xor_gc,
                            TRUE,
                            x-1,y-1,
                            3,3);
    }

    if (gtk_sheet_cell_isvisible(sheet, range.row0, range.coli) ||
        sheet->state == GTK_SHEET_COLUMN_SELECTED)
    {
        x=COLUMN_LEFT_XPIXEL(sheet,range.coli)+
            sheet->column[range.coli]->width;
        y=ROW_TOP_YPIXEL(sheet,range.row0);
        width = 1;
        if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
        {
            y = ROW_TOP_YPIXEL(sheet, sheet->view.row0)+3;
            width = 3;
        }
        gdk_draw_pixmap(sheet->sheet_window,
                        gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                        sheet->pixmap,
                        x-width,
                        y-width,
                        x-width,
                        y-width,
                        2*width+1,
                        2*width+1);
        gdk_draw_rectangle (sheet->sheet_window,
                            sheet->xor_gc,
                            TRUE,
                            x-width+width/2,y-width+width/2,
                            2+width,2+width);
    }

    if (gtk_sheet_cell_isvisible(sheet, range.rowi, range.col0) ||
        sheet->state == GTK_SHEET_ROW_SELECTED)
    {
        x=COLUMN_LEFT_XPIXEL(sheet,range.col0);
        y=ROW_TOP_YPIXEL(sheet,range.rowi)+
            sheet->row[range.rowi].height;
        width = 1;
        if (sheet->state == GTK_SHEET_ROW_SELECTED)
        {
            x = COLUMN_LEFT_XPIXEL(sheet, sheet->view.col0)+3;
            width = 3;
        }
        gdk_draw_pixmap(sheet->sheet_window,
                        gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                        sheet->pixmap,
                        x-width,
                        y-width,
                        x-width,
                        y-width,
                        2*width+1,
                        2*width+1);
        gdk_draw_rectangle (sheet->sheet_window,
                            sheet->xor_gc,
                            TRUE,
                            x-width+width/2,y-width+width/2,
                            2+width,2+width);
    }

    if (gtk_sheet_cell_isvisible(sheet, range.rowi, range.coli))
    {
        x=COLUMN_LEFT_XPIXEL(sheet,range.coli)+
            sheet->column[range.coli]->width;
        y=ROW_TOP_YPIXEL(sheet,range.rowi)+
            sheet->row[range.rowi].height;
        width = 1;
        if (sheet->state == GTK_SHEET_RANGE_SELECTED) width = 3;
        if (sheet->state == GTK_SHEET_NORMAL) width = 3;
        gdk_draw_pixmap(sheet->sheet_window,
                        gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                        sheet->pixmap,
                        x-width,
                        y-width,
                        x-width,
                        y-width,
                        2*width+1,
                        2*width+1);
        gdk_draw_rectangle (sheet->sheet_window,
                            sheet->xor_gc,
                            TRUE,
                            x-width+width/2,y-width+width/2,
                            2+width,2+width);

    }

}


static void
    gtk_sheet_real_select_range (GtkSheet * sheet,
                                 GtkSheetRange * range)
{
    gint i;
    gint state;

    g_return_if_fail (sheet != NULL);

    if (range==NULL) range=&sheet->range;

    if (range->row0 < 0 || range->rowi < 0) return;
    if (range->col0 < 0 || range->coli < 0) return;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_real_select_range: r %d-%d c %d-%d", 
            range->row0, range->rowi, range->col0, range->coli);
#endif

    state=sheet->state;

    if (state==GTK_SHEET_COLUMN_SELECTED || state==GTK_SHEET_RANGE_SELECTED)
    {
        for (i=sheet->range.col0; i< range->col0; i++)
            column_button_release(sheet, i);
        for (i=range->coli+1; i<= sheet->range.coli; i++)
            column_button_release(sheet, i);
        for (i=range->col0; i<=range->coli; i++)
        {
            column_button_set(sheet, i);
        }
    }

    if (state==GTK_SHEET_ROW_SELECTED || state==GTK_SHEET_RANGE_SELECTED)
    {
        for (i=sheet->range.row0; i< range->row0; i++)
            row_button_release(sheet, i);
        for (i=range->rowi+1; i<= sheet->range.rowi; i++)
            row_button_release(sheet, i);
        for (i=range->row0; i<=range->rowi; i++)
        {
            row_button_set(sheet, i);
        }
    }

    if (range->coli != sheet->range.coli || range->col0 != sheet->range.col0 ||
        range->rowi != sheet->range.rowi || range->row0 != sheet->range.row0)
    {

        gtk_sheet_new_selection(sheet, range);

        sheet->range.col0=range->col0;
        sheet->range.coli=range->coli;
        sheet->range.row0=range->row0;
        sheet->range.rowi=range->rowi;

    }
    else
    {
        gtk_sheet_draw_backing_pixmap(sheet, sheet->range);
        gtk_sheet_range_draw_selection(sheet, sheet->range);
    }

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[SELECT_RANGE], 0, range);
}

/**
 * gtk_sheet_select_range:
 * @sheet: a #GtkSheet
 * @range: a #GtkSheetRange
 *
 * Highlight the selected range and store bounds in sheet->range
 */
void
    gtk_sheet_select_range(GtkSheet * sheet, const GtkSheetRange *range)
{
    g_return_if_fail (sheet != NULL);

    if (range==NULL) range=&sheet->range;

    if (range->row0 < 0 || range->rowi < 0) return;
    if (range->col0 < 0 || range->coli < 0) return;

    if (sheet->state != GTK_SHEET_NORMAL)
        gtk_sheet_real_unselect_range(sheet, NULL);
    else
    {
        gboolean veto = TRUE;
        veto = gtk_sheet_deactivate_cell(sheet);
        if (!veto) return;
    }

    sheet->range.row0=range->row0;
    sheet->range.rowi=range->rowi;
    sheet->range.col0=range->col0;
    sheet->range.coli=range->coli;
    sheet->active_cell.row=range->row0;
    sheet->active_cell.col=range->col0;
    sheet->selection_cell.row=range->rowi;
    sheet->selection_cell.col=range->coli;

    sheet->state = GTK_SHEET_RANGE_SELECTED;
    gtk_sheet_real_select_range(sheet, NULL);

}
/**
 * gtk_sheet_unselect_range:
 * @sheet: a #GtkSheet
 *
 * Unselect the current selected range and clears the bounds in sheet->range.
 */
void
    gtk_sheet_unselect_range (GtkSheet * sheet)
{
    gtk_sheet_real_unselect_range(sheet, NULL);
    sheet->state = GTK_STATE_NORMAL;
    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
}


static void
    gtk_sheet_real_unselect_range (GtkSheet * sheet, const GtkSheetRange *range)
{
    gint i;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (gtk_widget_get_realized(GTK_WIDGET(sheet)));

    if (!range) range=&sheet->range;

    if (range->row0 < 0 || range->rowi < 0) return;
    if (range->col0 < 0 || range->coli < 0) return;

    if (gtk_sheet_range_isvisible (sheet, *range))
    {
        gtk_sheet_draw_backing_pixmap(sheet, *range);
    }

    for (i=range->col0; i<=range->coli; i++)
    {
        column_button_release(sheet, i);
    }

    for (i=range->row0; i<=range->rowi; i++)
    {
        row_button_release(sheet, i);
    }

    gtk_sheet_position_children(sheet);
}


/*
 * gtk_sheet_expose_handler:
 * 
 * this is the #GtkSheet widget class "expose-event" signal handler
 * 
 * @param widget the #GtkSheet
 * @param event  the GdkEventExpose which triggered this signal
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
    gtk_sheet_expose_handler (GtkWidget * widget, GdkEventExpose * event)
{
    GtkSheet *sheet;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);

    sheet = GTK_SHEET (widget);

    if (gtk_widget_is_drawable (widget))
    {
        if (event->window == sheet->row_title_window && sheet->row_titles_visible)
        {
            gint i;
            for (i = MIN_VISIBLE_ROW(sheet); i <= MAX_VISIBLE_ROW(sheet); i++)
                gtk_sheet_button_draw(sheet,i,-1);
        }

        if (event->window == sheet->column_title_window && sheet->column_titles_visible)
        {
            gint i;
            for (i = MIN_VISIBLE_COLUMN(sheet); i <= MAX_VISIBLE_COLUMN(sheet); i++)
                gtk_sheet_button_draw(sheet,-1,i);
        }

        if (event->window == sheet->sheet_window)
        {
            GtkSheetRange range;

            range.row0=ROW_FROM_YPIXEL(sheet,event->area.y);
            range.col0=COLUMN_FROM_XPIXEL(sheet,event->area.x);
            range.rowi=ROW_FROM_YPIXEL(sheet,event->area.y+event->area.height);
            range.coli=COLUMN_FROM_XPIXEL(sheet,event->area.x+event->area.width);

            gtk_sheet_draw_backing_pixmap(sheet, range);

            if (sheet->state != GTK_SHEET_NORMAL)
            {
                if (gtk_sheet_range_isvisible(sheet, sheet->range))
                    gtk_sheet_draw_backing_pixmap(sheet, sheet->range);
                if (GTK_SHEET_IN_RESIZE(sheet) || GTK_SHEET_IN_DRAG(sheet))
                    gtk_sheet_draw_backing_pixmap(sheet, sheet->drag_range);

                if (gtk_sheet_range_isvisible(sheet, sheet->range))
                    gtk_sheet_range_draw_selection(sheet, sheet->range);
                if (GTK_SHEET_IN_RESIZE(sheet) || GTK_SHEET_IN_DRAG(sheet))
                    draw_xor_rectangle(sheet, sheet->drag_range);
            }

            if ((!GTK_SHEET_IN_XDRAG(sheet)) && (!GTK_SHEET_IN_YDRAG(sheet)))
            {
                if (sheet->state == GTK_SHEET_NORMAL)
                {
                    gtk_sheet_draw_active_cell(sheet);
                    if (!GTK_SHEET_IN_SELECTION(sheet))
                        gtk_widget_queue_draw(sheet->sheet_entry);
                }
            }


        }

    }

    if (sheet->state != GTK_SHEET_NORMAL && GTK_SHEET_IN_SELECTION(sheet))
        gtk_widget_grab_focus(GTK_WIDGET(sheet));

    (* GTK_WIDGET_CLASS (sheet_parent_class)->expose_event) (widget, event);

    return (FALSE);
}


/*
 * gtk_sheet_button_press_handler:
 * 
 * this is the #GtkSheet widget class "button-press-event" handler
 * 
 * @param widget the #GtkSheet
 * @param event  the GdkEventButton which triggered this signal
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
    gtk_sheet_button_press_handler (GtkWidget * widget, GdkEventButton * event)
{
    GtkSheet *sheet;
    GdkModifierType mods;
    gint x, y, row, column;
    gboolean veto;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);
/*
    if(event->type != GDK_BUTTON_PRESS) return(TRUE);
*/

    gdk_window_get_pointer(gtk_widget_get_window(widget), 
                           NULL, NULL, &mods);
    if (!(mods & GDK_BUTTON1_MASK)) return(TRUE);

    sheet = GTK_SHEET (widget);

    /* press on resize windows */
    if (event->window == sheet->column_title_window && gtk_sheet_columns_resizable(sheet))
    {
        gtk_widget_get_pointer (widget, &sheet->x_drag, NULL);
        if (POSSIBLE_XDRAG(sheet, sheet->x_drag, &sheet->drag_cell.col))
        {
            guint req;
            if (event->type == GDK_2BUTTON_PRESS)
            {
                gtk_sheet_autoresize_column (sheet, sheet->drag_cell.col);
                GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
                return(TRUE);
            }
            gtk_sheet_column_size_request(sheet, sheet->drag_cell.col, &req);
            GTK_SHEET_SET_FLAGS (sheet, GTK_SHEET_IN_XDRAG);
            gdk_pointer_grab (sheet->column_title_window, FALSE,
                              GDK_POINTER_MOTION_HINT_MASK |
                              GDK_BUTTON1_MOTION_MASK |
                              GDK_BUTTON_RELEASE_MASK,
                              NULL, NULL, event->time);

            draw_xor_vline (sheet);
            return(TRUE);
        }
    }

    if (event->window == sheet->row_title_window && gtk_sheet_rows_resizable(sheet))
    {
        gtk_widget_get_pointer (widget, NULL, &sheet->y_drag);

        if (POSSIBLE_YDRAG(sheet, sheet->y_drag, &sheet->drag_cell.row))
        {
            guint req;
            gtk_sheet_row_size_request(sheet, sheet->drag_cell.row, &req);
            GTK_SHEET_SET_FLAGS (sheet, GTK_SHEET_IN_YDRAG);
            gdk_pointer_grab (sheet->row_title_window, FALSE,
                              GDK_POINTER_MOTION_HINT_MASK |
                              GDK_BUTTON1_MOTION_MASK |
                              GDK_BUTTON_RELEASE_MASK,
                              NULL, NULL, event->time);

            draw_xor_hline (sheet);
            return(TRUE);
        }
    }

    /* the sheet itself does not handle other than single click events */
    if (event->type != GDK_BUTTON_PRESS) return(FALSE);

    /* selections on the sheet */
    if (event->window == sheet->sheet_window)
    {
        gtk_widget_get_pointer (widget, &x, &y);
        gtk_sheet_get_pixel_info (sheet, x, y, &row, &column);
        gdk_pointer_grab (sheet->sheet_window, FALSE,
                          GDK_POINTER_MOTION_HINT_MASK |
                          GDK_BUTTON1_MOTION_MASK |
                          GDK_BUTTON_RELEASE_MASK,
                          NULL, NULL, event->time);
        gtk_grab_add(GTK_WIDGET(sheet));
        sheet->timer=g_timeout_add_full(0, TIMEOUT_SCROLL, gtk_sheet_scroll, sheet, NULL);
        gtk_widget_grab_focus(GTK_WIDGET(sheet));

        if (sheet->selection_mode != GTK_SELECTION_SINGLE &&
            sheet->cursor_drag->type==GDK_SIZING &&
            !GTK_SHEET_IN_SELECTION(sheet) && !GTK_SHEET_IN_RESIZE(sheet))
        {
            if (sheet->state==GTK_STATE_NORMAL)
            {
                row=sheet->active_cell.row;
                column=sheet->active_cell.col;
                if (!gtk_sheet_deactivate_cell(sheet)) return(FALSE);
                sheet->active_cell.row=row;
                sheet->active_cell.col=column;
                sheet->drag_range=sheet->range;
                sheet->state=GTK_SHEET_RANGE_SELECTED;
                gtk_sheet_select_range(sheet, &sheet->drag_range);
            }
            sheet->x_drag=x;
            sheet->y_drag=y;
            if (row > sheet->range.rowi) row--;
            if (column > sheet->range.coli) column--;
            sheet->drag_cell.row = row;
            sheet->drag_cell.col = column;
            sheet->drag_range=sheet->range;
            draw_xor_rectangle(sheet, sheet->drag_range);
            GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_RESIZE);
        }
        else if (sheet->cursor_drag->type==GDK_TOP_LEFT_ARROW &&
                 !GTK_SHEET_IN_SELECTION(sheet) && !GTK_SHEET_IN_DRAG(sheet))
        {
            if (sheet->state==GTK_STATE_NORMAL)
            {
                row=sheet->active_cell.row;
                column=sheet->active_cell.col;
                if (!gtk_sheet_deactivate_cell(sheet)) return(FALSE);
                sheet->active_cell.row=row;
                sheet->active_cell.col=column;
                sheet->drag_range=sheet->range;
                sheet->state=GTK_SHEET_RANGE_SELECTED;
                gtk_sheet_select_range(sheet, &sheet->drag_range);
            }
            sheet->x_drag=x;
            sheet->y_drag=y;
            if (row < sheet->range.row0) row++;
            if (row > sheet->range.rowi) row--;
            if (column < sheet->range.col0) column++;
            if (column > sheet->range.coli) column--;
            sheet->drag_cell.row=row;
            sheet->drag_cell.col=column;
            sheet->drag_range=sheet->range;
            draw_xor_rectangle(sheet, sheet->drag_range);
            GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_DRAG);
        }
        else
        {
            gtk_sheet_click_cell(sheet, row, column, &veto);
            if (veto) GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
        }
    }

    if (event->window == sheet->column_title_window)
    {
        gtk_widget_get_pointer (widget, &x, &y);
        column = COLUMN_FROM_XPIXEL(sheet, x);
        if (column < 0 || column > sheet->maxcol) return(FALSE);

        if (GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)))
        {
            gtk_sheet_click_cell(sheet, -1, column, &veto);
            gtk_grab_add(GTK_WIDGET(sheet));
            sheet->timer=g_timeout_add_full(0, TIMEOUT_SCROLL, gtk_sheet_scroll, sheet, NULL);
            gtk_widget_grab_focus(GTK_WIDGET(sheet));
            GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
        }
    }

    if (event->window == sheet->row_title_window)
    {
        gtk_widget_get_pointer (widget, &x, &y);
        row = ROW_FROM_YPIXEL(sheet, y);
        if (row < 0 || row > sheet->maxrow) return(FALSE);

        if (GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)))
        {
            gtk_sheet_click_cell(sheet, row, -1, &veto);
            gtk_grab_add(GTK_WIDGET(sheet));
            sheet->timer=g_timeout_add_full(0, TIMEOUT_SCROLL, gtk_sheet_scroll, sheet, NULL);
            gtk_widget_grab_focus(GTK_WIDGET(sheet));
            GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
        }
    }

    return(TRUE);
}

static gint
    gtk_sheet_scroll(gpointer data)
{
    GtkSheet *sheet;
    gint x,y,row,column;
    gint move;

    sheet=GTK_SHEET(data);

    GDK_THREADS_ENTER();

    gtk_widget_get_pointer (GTK_WIDGET(sheet), &x, &y);
    gtk_sheet_get_pixel_info (sheet, x, y, &row, &column);

    move=TRUE;

    if (GTK_SHEET_IN_SELECTION(sheet))
        gtk_sheet_extend_selection(sheet, row, column);

    if (GTK_SHEET_IN_DRAG(sheet) || GTK_SHEET_IN_RESIZE(sheet))
    {
        move=gtk_sheet_move_query(sheet, row, column);
        if (move) draw_xor_rectangle(sheet, sheet->drag_range);
    }

    GDK_THREADS_LEAVE();

    return (TRUE);

}

static void
    gtk_sheet_click_cell(GtkSheet *sheet, gint row, gint column, gboolean *veto)
{
    *veto = TRUE;

    if (row > sheet->maxrow || column > sheet->maxcol)
    {
        *veto = FALSE;
        return;
    }

    if (column >= 0 && row >= 0)
    {
        if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column)) || 
            !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
        {
            *veto = FALSE;
            return;
        }
    }

    _gtkextra_signal_emit(GTK_OBJECT(sheet), sheet_signals[TRAVERSE],
                          sheet->active_cell.row, sheet->active_cell.col,
                          &row, &column, veto);

    if (!*veto)
    {
        if (sheet->state == GTK_STATE_NORMAL) return;

        row = sheet->active_cell.row;
        column = sheet->active_cell.col;
        gtk_sheet_activate_cell(sheet, row, column);
        return;
    }

    if (row == -1 && column >= 0)
    {
        if (gtk_sheet_autoscroll(sheet))
            gtk_sheet_move_query(sheet, row, column);

        gtk_sheet_select_column(sheet, column);
        return;
    }
    if (column == -1 && row >= 0)
    {
        if (gtk_sheet_autoscroll(sheet))
            gtk_sheet_move_query(sheet, row, column);

        gtk_sheet_select_row(sheet, row);
        return;
    }

    if (row==-1 && column ==-1)
    {
        sheet->range.row0=0;
        sheet->range.col0=0;
        sheet->range.rowi=sheet->maxrow;
        sheet->range.coli=sheet->maxcol;

        sheet->active_cell.row=0;
        sheet->active_cell.col=0;

        gtk_sheet_select_range(sheet, NULL);
        return;
    }

    if (row!=-1 && column !=-1)
    {
        if (sheet->state != GTK_SHEET_NORMAL)
        {
            sheet->state = GTK_SHEET_NORMAL;
            gtk_sheet_real_unselect_range(sheet, NULL);
        }
        else
        {
            if (!gtk_sheet_deactivate_cell(sheet))
            {
                *veto = FALSE;
                return;
            }
        }

        if (gtk_sheet_autoscroll(sheet))
            gtk_sheet_move_query(sheet, row, column);

        sheet->active_cell.row=row;
        sheet->active_cell.col=column;

        sheet->selection_cell.row=row;
        sheet->selection_cell.col=column;

        sheet->range.row0=row;
        sheet->range.col0=column;
        sheet->range.rowi=row;
        sheet->range.coli=column;

        sheet->state=GTK_SHEET_NORMAL;

        GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
        gtk_sheet_draw_active_cell(sheet);
        return;
    }

    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
}

/*
 * gtk_sheet_button_release_handler:
 * 
 * this is the #GtkSheet widget class "button-release-event" handler
 * 
 * @param widget the #GtkSheet
 * @param event  the GdkEventButton which triggered this signal
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
    gtk_sheet_button_release_handler (GtkWidget *widget, GdkEventButton *event)
{
    GtkSheet *sheet;
    gint x,y;

    sheet=GTK_SHEET(widget);

    /* release on resize windows */
    if (GTK_SHEET_IN_XDRAG (sheet))
    {
        GTK_SHEET_UNSET_FLAGS (sheet, GTK_SHEET_IN_XDRAG);
        GTK_SHEET_UNSET_FLAGS (sheet, GTK_SHEET_IN_SELECTION);
        gtk_widget_get_pointer (widget, &x, NULL);
        gdk_pointer_ungrab (event->time);
        draw_xor_vline (sheet);

        gtk_sheet_set_column_width (sheet, sheet->drag_cell.col, new_column_width (sheet, sheet->drag_cell.col, &x));

        sheet->old_hadjustment = -1.;

        if (sheet->hadjustment)
        {
            g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment),  "value_changed");
        }
        return (TRUE);
    }

    if (GTK_SHEET_IN_YDRAG (sheet))
    {
        GTK_SHEET_UNSET_FLAGS (sheet, GTK_SHEET_IN_YDRAG);
        GTK_SHEET_UNSET_FLAGS (sheet, GTK_SHEET_IN_SELECTION);
        gtk_widget_get_pointer (widget, NULL, &y);
        gdk_pointer_ungrab (event->time);
        draw_xor_hline (sheet);

        gtk_sheet_set_row_height (sheet, sheet->drag_cell.row, new_row_height (sheet, sheet->drag_cell.row, &y));
        sheet->old_vadjustment = -1.;
        if (sheet->vadjustment)
        {
            g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
        }
        return (TRUE);
    }


    if (GTK_SHEET_IN_DRAG(sheet))
    {
        GtkSheetRange old_range;
        draw_xor_rectangle(sheet, sheet->drag_range);
        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_DRAG);
        gdk_pointer_ungrab (event->time);

        gtk_sheet_real_unselect_range(sheet, NULL);

        sheet->active_cell.row = sheet->active_cell.row +
            (sheet->drag_range.row0 - sheet->range.row0);
        sheet->active_cell.col = sheet->active_cell.col +
            (sheet->drag_range.col0 - sheet->range.col0);
        sheet->selection_cell.row = sheet->selection_cell.row +
            (sheet->drag_range.row0 - sheet->range.row0);
        sheet->selection_cell.col = sheet->selection_cell.col +
            (sheet->drag_range.col0 - sheet->range.col0);
        old_range=sheet->range;
        sheet->range=sheet->drag_range;
        sheet->drag_range=old_range;
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[MOVE_RANGE], 0,
                      &sheet->drag_range, &sheet->range);
        gtk_sheet_select_range(sheet, &sheet->range);
    }

    if (GTK_SHEET_IN_RESIZE(sheet))
    {
        GtkSheetRange old_range;
        draw_xor_rectangle(sheet, sheet->drag_range);
        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_RESIZE);
        gdk_pointer_ungrab (event->time);

        gtk_sheet_real_unselect_range(sheet, NULL);

        sheet->active_cell.row = sheet->active_cell.row +
            (sheet->drag_range.row0 - sheet->range.row0);
        sheet->active_cell.col = sheet->active_cell.col +
            (sheet->drag_range.col0 - sheet->range.col0);
        if (sheet->drag_range.row0 < sheet->range.row0)
            sheet->selection_cell.row = sheet->drag_range.row0;
        if (sheet->drag_range.rowi >= sheet->range.rowi)
            sheet->selection_cell.row = sheet->drag_range.rowi;
        if (sheet->drag_range.col0 < sheet->range.col0)
            sheet->selection_cell.col = sheet->drag_range.col0;
        if (sheet->drag_range.coli >= sheet->range.coli)
            sheet->selection_cell.col = sheet->drag_range.coli;
        old_range = sheet->range;
        sheet->range = sheet->drag_range;
        sheet->drag_range = old_range;

        if (sheet->state==GTK_STATE_NORMAL) sheet->state=GTK_SHEET_RANGE_SELECTED;
        g_signal_emit(GTK_OBJECT(sheet),sheet_signals[RESIZE_RANGE], 0, 
                      &sheet->drag_range, &sheet->range);
        gtk_sheet_select_range(sheet, &sheet->range);
    }

    if (sheet->state == GTK_SHEET_NORMAL && GTK_SHEET_IN_SELECTION(sheet))
    {
        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
        gdk_pointer_ungrab (event->time);
        gtk_sheet_activate_cell(sheet, sheet->active_cell.row,
                                sheet->active_cell.col);
    }

    if (GTK_SHEET_IN_SELECTION)
        gdk_pointer_ungrab (event->time);
    if (sheet->timer)
        g_source_remove(sheet->timer);
    gtk_grab_remove(GTK_WIDGET(sheet));

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);

    return (TRUE);
}

/*
 * gtk_sheet_motion_handler:<p>
 * this is the #GtkSheet widget class "motion-notify-event" signal handler
 * 
 * @param widget the #GtkSheet
 * @param event  the GdkEventMotion which triggered this signal
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
    gtk_sheet_motion_handler (GtkWidget * widget, GdkEventMotion * event)
{
    GtkSheet *sheet;
    GdkModifierType mods;
    GdkCursorType new_cursor;
    gint x, y, row, column;

    g_return_val_if_fail (widget != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (widget), FALSE);
    g_return_val_if_fail (event != NULL, FALSE);


    sheet = GTK_SHEET (widget);

    /* selections on the sheet */
    x = event->x;
    y = event->y;

    if (event->window == sheet->column_title_window && gtk_sheet_columns_resizable(sheet))
    {
        gtk_widget_get_pointer(widget, &x, &y);
        if (!GTK_SHEET_IN_SELECTION(sheet) && POSSIBLE_XDRAG(sheet, x, &column))
        {
            new_cursor=GDK_SB_H_DOUBLE_ARROW;
            if (new_cursor != sheet->cursor_drag->type)
            {
                gdk_cursor_destroy(sheet->cursor_drag);
                sheet->cursor_drag=gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
                gdk_window_set_cursor(sheet->column_title_window,sheet->cursor_drag);
            }
        }
        else
        {
            new_cursor=GDK_TOP_LEFT_ARROW;
            if (!GTK_SHEET_IN_XDRAG(sheet) && new_cursor != sheet->cursor_drag->type)
            {
                gdk_cursor_destroy(sheet->cursor_drag);
                sheet->cursor_drag=gdk_cursor_new(GDK_TOP_LEFT_ARROW);
                gdk_window_set_cursor(sheet->column_title_window,sheet->cursor_drag);
            }
        }
    }

    if (event->window == sheet->row_title_window && gtk_sheet_rows_resizable(sheet))
    {
        gtk_widget_get_pointer(widget, &x, &y);
        if (!GTK_SHEET_IN_SELECTION(sheet) && POSSIBLE_YDRAG(sheet,y, &column))
        {
            new_cursor=GDK_SB_V_DOUBLE_ARROW;
            if (new_cursor != sheet->cursor_drag->type)
            {
                gdk_cursor_destroy(sheet->cursor_drag);
                sheet->cursor_drag=gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
                gdk_window_set_cursor(sheet->row_title_window,sheet->cursor_drag);
            }
        }
        else
        {
            new_cursor=GDK_TOP_LEFT_ARROW;
            if (!GTK_SHEET_IN_YDRAG(sheet) && new_cursor != sheet->cursor_drag->type)
            {
                gdk_cursor_destroy(sheet->cursor_drag);
                sheet->cursor_drag=gdk_cursor_new(GDK_TOP_LEFT_ARROW);
                gdk_window_set_cursor(sheet->row_title_window,sheet->cursor_drag);
            }
        }
    }

    new_cursor=GDK_PLUS;
    if (!POSSIBLE_DRAG(sheet,x,y,&row,&column) && !GTK_SHEET_IN_DRAG(sheet) &&
        !POSSIBLE_RESIZE(sheet,x,y,&row,&column) && !GTK_SHEET_IN_RESIZE(sheet) &&
        event->window == sheet->sheet_window &&
        new_cursor != sheet->cursor_drag->type)
    {
        gdk_cursor_destroy(sheet->cursor_drag);
        sheet->cursor_drag=gdk_cursor_new(GDK_PLUS);
        gdk_window_set_cursor(sheet->sheet_window,sheet->cursor_drag);
    }

    new_cursor=GDK_TOP_LEFT_ARROW;
    if (!(POSSIBLE_RESIZE(sheet,x,y,&row,&column) || GTK_SHEET_IN_RESIZE(sheet)) &&
        (POSSIBLE_DRAG(sheet, x,y,&row,&column) || GTK_SHEET_IN_DRAG(sheet)) &&
        event->window == sheet->sheet_window &&
        new_cursor != sheet->cursor_drag->type)
    {
        gdk_cursor_destroy(sheet->cursor_drag);
        sheet->cursor_drag=gdk_cursor_new(GDK_TOP_LEFT_ARROW);
        gdk_window_set_cursor(sheet->sheet_window,sheet->cursor_drag);
    }

    new_cursor=GDK_SIZING;
    if (!GTK_SHEET_IN_DRAG(sheet) &&
        (POSSIBLE_RESIZE(sheet,x,y,&row,&column) || GTK_SHEET_IN_RESIZE(sheet)) &&
        event->window == sheet->sheet_window &&
        new_cursor != sheet->cursor_drag->type)
    {
        gdk_cursor_destroy(sheet->cursor_drag);
        sheet->cursor_drag=gdk_cursor_new(GDK_SIZING);
        gdk_window_set_cursor(sheet->sheet_window,sheet->cursor_drag);
    }

    gdk_window_get_pointer (gtk_widget_get_window(widget), &x, &y, &mods);
    if (!(mods & GDK_BUTTON1_MASK)) return (FALSE);

    if (GTK_SHEET_IN_XDRAG (sheet))
    {
        if (event->is_hint || 
            event->window != gtk_widget_get_window(widget))
        {
            gtk_widget_get_pointer (widget, &x, NULL);
        }
        else
            x = event->x;

        new_column_width (sheet, sheet->drag_cell.col, &x);
        if (x != sheet->x_drag)
        {
            draw_xor_vline (sheet);
            sheet->x_drag = x;
            draw_xor_vline (sheet);
        }
        return (TRUE);
    }

    if (GTK_SHEET_IN_YDRAG (sheet))
    {
        if (event->is_hint || 
            event->window != gtk_widget_get_window(widget))
        {
            gtk_widget_get_pointer (widget, NULL, &y);
        }
        else
            y = event->y;

        new_row_height (sheet, sheet->drag_cell.row, &y);
        if (y != sheet->y_drag)
        {
            draw_xor_hline (sheet);
            sheet->y_drag = y;
            draw_xor_hline (sheet);
        }
        return (TRUE);
    }

    if (GTK_SHEET_IN_DRAG(sheet))
    {
        GtkSheetRange aux;
        column=COLUMN_FROM_XPIXEL(sheet,x)-sheet->drag_cell.col;
        row=ROW_FROM_YPIXEL(sheet,y)-sheet->drag_cell.row;
        if (sheet->state==GTK_SHEET_COLUMN_SELECTED) row=0;
        if (sheet->state==GTK_SHEET_ROW_SELECTED) column=0;
        sheet->x_drag=x;
        sheet->y_drag=y;
        aux=sheet->range;
        if (aux.row0+row >= 0 && aux.rowi+row <= sheet->maxrow &&
            aux.col0+column >= 0 && aux.coli+column <= sheet->maxcol)
        {
            aux=sheet->drag_range;
            sheet->drag_range.row0=sheet->range.row0+row;
            sheet->drag_range.col0=sheet->range.col0+column;
            sheet->drag_range.rowi=sheet->range.rowi+row;
            sheet->drag_range.coli=sheet->range.coli+column;
            if (aux.row0 != sheet->drag_range.row0 ||
                aux.col0 != sheet->drag_range.col0)
            {
                draw_xor_rectangle (sheet, aux);
                draw_xor_rectangle (sheet, sheet->drag_range);
            }
        }
        return (TRUE);
    }

    if (GTK_SHEET_IN_RESIZE(sheet))
    {
        GtkSheetRange aux;
        gint v_h, current_col, current_row, col_threshold, row_threshold;
        v_h=1;

        /* use center of the selection to determine h_v */
        g_assert(0 <= sheet->drag_cell.col && sheet->drag_cell.col <= sheet->maxcol);
        g_assert(0 <= sheet->drag_cell.row && sheet->drag_cell.row <= sheet->maxrow);

        if (abs(x-(COLUMN_LEFT_XPIXEL(sheet,sheet->drag_cell.col)+sheet->column[sheet->drag_cell.col]->width/2)) >
            abs(y-(ROW_TOP_YPIXEL(sheet,sheet->drag_cell.row)+sheet->row[sheet->drag_cell.row].height/2)))
        {
            v_h=2;
        }

        current_col = MIN(sheet->maxcol, COLUMN_FROM_XPIXEL(sheet,x));
        current_row = MIN(sheet->maxrow, ROW_FROM_YPIXEL(sheet,y));

        column = current_col-sheet->drag_cell.col;
        row    = current_row-sheet->drag_cell.row;

#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_motion: row %d cr %d column %d", row, current_row, column);
#endif

        /*use half of column width resp. row height as threshold to expand selection*/
        col_threshold = COLUMN_LEFT_XPIXEL(sheet,current_col)+gtk_sheet_column_width (sheet,current_col)/2;
        if (column > 0)
        {
            if (x < col_threshold)
                column-=1;
        }
        else if (column < 0)
        {
            if (x > col_threshold)
                column+=1;
        }
        row_threshold = ROW_TOP_YPIXEL(sheet,current_row)+gtk_sheet_row_height (sheet, current_row)/2;
        if (row > 0)
        {
            if (y < row_threshold)
                row-=1;
        }
        else if (row < 0)
        {
            if (y > row_threshold)
                row+=1;
        }

        if (sheet->state==GTK_SHEET_COLUMN_SELECTED) row=0;
        if (sheet->state==GTK_SHEET_ROW_SELECTED) column=0;
        sheet->x_drag=x;
        sheet->y_drag=y;
        aux=sheet->range;

        if (v_h==1)
            column=0;
        else
            row=0;

#ifdef GTK_SHEET_DEBUG
        g_debug("gtk_sheet_motion: 2 state %d row %d cr %d th %d column %d", 
                v_h, row, current_row, row_threshold, column);
#endif

        if (aux.row0+row >= 0 && aux.rowi+row <= sheet->maxrow &&
            aux.col0+column >= 0 && aux.coli+column <= sheet->maxcol)
        {

            aux=sheet->drag_range;
            sheet->drag_range=sheet->range;

            if (row<0) sheet->drag_range.row0=sheet->range.row0+row;
            if (row>0) sheet->drag_range.rowi=sheet->range.rowi+row;
            if (column<0) sheet->drag_range.col0=sheet->range.col0+column;
            if (column>0) sheet->drag_range.coli=sheet->range.coli+column;

            if (aux.row0 != sheet->drag_range.row0 ||
                aux.rowi != sheet->drag_range.rowi ||
                aux.col0 != sheet->drag_range.col0 ||
                aux.coli != sheet->drag_range.coli)
            {
                draw_xor_rectangle (sheet, aux);
                draw_xor_rectangle (sheet, sheet->drag_range);
            }
        }
        return (TRUE);
    }



    gtk_sheet_get_pixel_info (sheet, x, y, &row, &column);

    if (sheet->state==GTK_SHEET_NORMAL && row==sheet->active_cell.row &&
        column==sheet->active_cell.col) return (TRUE);

    if (GTK_SHEET_IN_SELECTION(sheet) && mods&GDK_BUTTON1_MASK)
        gtk_sheet_extend_selection(sheet, row, column);

    return (TRUE);
}

static gint
    gtk_sheet_move_query(GtkSheet *sheet, gint row, gint column)
{
    gint row_move, column_move;
    gfloat row_align, col_align;
    guint height, width;
    gint new_row = row;
    gint new_col = column;

    row_move=FALSE;
    column_move=FALSE;
    row_align=-1.;
    col_align=-1.;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_move_query: row %d column %d visrow(%d,%d) viscol(%d,%d)", 
            row, column,
            MIN_VISIBLE_ROW(sheet), MAX_VISIBLE_ROW(sheet), 
            MIN_VISIBLE_COLUMN(sheet), MAX_VISIBLE_COLUMN(sheet));
#endif

    height = sheet->sheet_window_height;
    width = sheet->sheet_window_width;

    if (row >= MAX_VISIBLE_ROW(sheet) && sheet->state != GTK_SHEET_COLUMN_SELECTED)
    {
        row_align = 1.;
        new_row = MIN(sheet->maxrow, row + 1);
        row_move = TRUE;

        if (MAX_VISIBLE_ROW(sheet) == sheet->maxrow &&
            ROW_BOTTOM_YPIXEL(sheet, sheet->maxrow) < height)
        {
            row_move = FALSE;
            row_align = -1.;
        }
    }

    if (row < MIN_VISIBLE_ROW(sheet) && sheet->state != GTK_SHEET_COLUMN_SELECTED)
    {
        row_align= 0.;
        row_move = TRUE;
    }

    if (column >= MAX_VISIBLE_COLUMN(sheet) && sheet->state != GTK_SHEET_ROW_SELECTED)
    {
        col_align = 1.;
        new_col = MIN(sheet->maxcol, column + 1);
        column_move = TRUE;

        if (MAX_VISIBLE_COLUMN(sheet) == sheet->maxcol &&
            COLUMN_RIGHT_XPIXEL(sheet, sheet->maxcol) < width)
        {
            column_move = FALSE;
            col_align = -1.;
        }
    }

    if (column < MIN_VISIBLE_COLUMN(sheet) && sheet->state != GTK_SHEET_ROW_SELECTED)
    {
        col_align = 0.;
        column_move = TRUE;
    }

    if (row_move || column_move)
    {
        gtk_sheet_moveto(sheet, new_row, new_col, row_align, col_align);
    }

    return(row_move || column_move);
}

static void
    gtk_sheet_extend_selection(GtkSheet *sheet, gint row, gint column)
{
    GtkSheetRange range;
    gint state;
    gint r,c;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_extend_selection: row %d column %d", row, column);
#endif

    if (sheet->selection_mode == GTK_SELECTION_SINGLE) return;
    if (row == sheet->selection_cell.row && column == sheet->selection_cell.col) return;

    if (sheet->active_cell.row < 0 || sheet->active_cell.row > sheet->maxrow) return;
    if (sheet->active_cell.col < 0 || sheet->active_cell.col > sheet->maxcol) return;

    gtk_sheet_move_query(sheet, row, column);
    gtk_widget_grab_focus(GTK_WIDGET(sheet));

    if (GTK_SHEET_IN_DRAG(sheet)) return;

    state=sheet->state;

    switch (sheet->state)
    {
        case GTK_SHEET_ROW_SELECTED:
            column = sheet->maxcol;
            break;

        case GTK_SHEET_COLUMN_SELECTED:
            row = sheet->maxrow;
            break;

        case GTK_SHEET_NORMAL:
            r=sheet->active_cell.row;
            c=sheet->active_cell.col;

            sheet->range.col0=c;
            sheet->range.row0=r;
            sheet->range.coli=c;
            sheet->range.rowi=r;

            gdk_draw_pixmap(sheet->sheet_window,
                            gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
                            sheet->pixmap,
                            COLUMN_LEFT_XPIXEL(sheet,c)-1,
                            ROW_TOP_YPIXEL(sheet,r)-1,
                            COLUMN_LEFT_XPIXEL(sheet,c)-1,
                            ROW_TOP_YPIXEL(sheet,r)-1,
                            sheet->column[c]->width+4,
                            sheet->row[r].height+4);

            sheet->state=GTK_SHEET_RANGE_SELECTED;
            gtk_sheet_range_draw_selection(sheet, sheet->range);
            /* FALLTHROUGH */

        case GTK_SHEET_RANGE_SELECTED:
            sheet->state=GTK_SHEET_RANGE_SELECTED;
            /* FALLTHROUGH */
    }

    sheet->selection_cell.row = row;
    sheet->selection_cell.col = column;

    range.col0=MIN(column,sheet->active_cell.col);
    range.coli=MAX(column,sheet->active_cell.col);

    range.row0=MIN(row,sheet->active_cell.row);
    range.rowi=MAX(row,sheet->active_cell.row);

    range.coli=MIN(range.coli,sheet->maxcol);
    range.rowi=MIN(range.rowi,sheet->maxrow);

    if (range.row0 != sheet->range.row0 || range.rowi != sheet->range.rowi ||
        range.col0 != sheet->range.col0 || range.coli != sheet->range.coli ||
        state==GTK_SHEET_NORMAL)
    {
        gtk_sheet_real_select_range(sheet, &range);
    }
}

/*
 * gtk_sheet_entry_key_press_handler:
 * 
 * this event handler propagates the "key-press-event" signal 
 * from the sheet_entry to the #GtkSheet
 * 
 * @param widget the #GtkSheet (connected "swapped")
 * @param key    the GdkEventKey which triggered this signal
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
    gtk_sheet_entry_key_press_handler(GtkWidget *widget, GdkEventKey *key, gpointer user_data)
{
    gboolean stop_emission;

    g_signal_emit_by_name(GTK_OBJECT(widget), "key_press_event", key, &stop_emission);

    return (stop_emission);
}

/*
 * gtk_sheet_key_press_handler:
 * 
 * this is the #GtkSheet widget class "key-press-event" handler.
 * 
 * @param widget the #GtkSheet
 * @param key    the GdkEventKey which triggered this signal
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
    gtk_sheet_key_press_handler(GtkWidget *widget, GdkEventKey *key)
{
    GtkSheet *sheet;
    gint row, col;
    gint state;
    gboolean extend_selection = FALSE;
    gboolean force_move = FALSE;
    gboolean in_selection = FALSE;
    gboolean veto = TRUE;
    gint scroll = 1;
    gchar *text = NULL;
    gboolean text_is_empty;

    sheet = GTK_SHEET(widget);

    if (key->state & GDK_CONTROL_MASK || key->keyval==GDK_Control_L ||
        key->keyval==GDK_Control_R) return(FALSE);

/*
  {
    if(key->keyval=='c' || key->keyval == 'C' && sheet->state != GTK_STATE_NORMAL)
            gtk_sheet_clip_range(sheet, sheet->range);
    if(key->keyval=='x' || key->keyval == 'X')
            gtk_sheet_unclip_range(sheet);
    return FALSE;
  }
*/

    extend_selection = (key->state & GDK_SHIFT_MASK)
    || key->keyval==GDK_Shift_L
        || key->keyval==GDK_Shift_R;

    state=sheet->state;
    in_selection = GTK_SHEET_IN_SELECTION(sheet);
    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);

    switch (key->keyval)
    {
        case GDK_Return:
            if (GTK_IS_TEXT_VIEW(sheet->sheet_entry)) return(FALSE);
            /* FALLTHROUGH */

        case GDK_KP_Enter:
            if (sheet->state == GTK_SHEET_NORMAL &&
                !GTK_SHEET_IN_SELECTION(sheet))
                g_signal_stop_emission_by_name(GTK_OBJECT(gtk_sheet_get_entry(sheet)), 
                                               "key_press_event");
            row = sheet->active_cell.row;
            col = sheet->active_cell.col;
            if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
                row = MIN_VISIBLE_ROW(sheet)-1;
            if (sheet->state == GTK_SHEET_ROW_SELECTED)
                col = MIN_VISIBLE_COLUMN(sheet);
            if (row < sheet->maxrow)
            {
                row = row + scroll;
                while (row < sheet->maxrow && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row++;
                row=MIN(sheet->maxrow, row);
            }

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_ISO_Left_Tab:
            row = sheet->active_cell.row;
            col = sheet->active_cell.col;
            if (sheet->state == GTK_SHEET_ROW_SELECTED)
                col = MIN_VISIBLE_COLUMN(sheet)-1;
            if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
                row = MIN_VISIBLE_ROW(sheet);
            if (col > 0)
            {
                col = col - scroll;
                while (col>0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col--;
                col=MAX(0, col);
            }

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_Tab:
            row = sheet->active_cell.row;
            col = sheet->active_cell.col;
            if (sheet->state == GTK_SHEET_ROW_SELECTED)
                col = MIN_VISIBLE_COLUMN(sheet)-1;
            if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
                row = MIN_VISIBLE_ROW(sheet);
            if (col < sheet->maxcol)
            {
                col = col + scroll;
                while (col<sheet->maxcol && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col++;
                col=MIN(sheet->maxcol, col);
            }

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

/*    case GDK_BackSpace:
            if(sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0){
             if(sheet->active_cell.col > 0){
                  col = sheet->active_cell.col - scroll;
              row = sheet->active_cell.row;
                  while(col > 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col--;
             }
            }
            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;
*/
        case GDK_Page_Up:
            scroll=MAX_VISIBLE_ROW(sheet)-MIN_VISIBLE_ROW(sheet)+1;
            /* FALLTHROUGH */

        case GDK_Up:
            if (extend_selection)
            {
                if (state==GTK_STATE_NORMAL)
                {
                    row=sheet->active_cell.row;
                    col=sheet->active_cell.col;
                    gtk_sheet_click_cell(sheet, row, col, &veto);
                    if (!veto) break;
                }
                if (sheet->selection_cell.row > 0)
                {
                    row = sheet->selection_cell.row - scroll;
                    while (row > 0 && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row--;
                    row = MAX(0, row);
                    gtk_sheet_extend_selection(sheet, row, sheet->selection_cell.col);
                }
                return(TRUE);
            }

            col = sheet->active_cell.col;
            row = sheet->active_cell.row;
            if (state==GTK_SHEET_COLUMN_SELECTED)
                row = MIN_VISIBLE_ROW(sheet);
            if (state==GTK_SHEET_ROW_SELECTED)
                col = MIN_VISIBLE_COLUMN(sheet);
            row = row - scroll;
            while (row > 0 && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row--;
            row = MAX(0,row);

            text = gtk_sheet_get_entry_text(sheet);
            text_is_empty = (!text || !text[0]);
            g_free(text); text = NULL;

            if (!force_move && !text_is_empty && GTK_IS_TEXT_VIEW(sheet->sheet_entry)) 
                return(FALSE);

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_Page_Down:
            scroll=MAX_VISIBLE_ROW(sheet)-MIN_VISIBLE_ROW(sheet)+1;
            /* FALLTHROUGH */

        case GDK_Down:
            if (extend_selection)
            {
                if (state==GTK_STATE_NORMAL)
                {
                    row=sheet->active_cell.row;
                    col=sheet->active_cell.col;
                    gtk_sheet_click_cell(sheet, row, col, &veto);
                    if (!veto) break;
                }
                if (sheet->selection_cell.row < sheet->maxrow)
                {
                    row = sheet->selection_cell.row + scroll;
                    while (row < sheet->maxrow && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row++;
                    row = MIN(sheet->maxrow, row);
                    gtk_sheet_extend_selection(sheet, row, sheet->selection_cell.col);
                }
                return(TRUE);
            }

            col = sheet->active_cell.col;
            row = sheet->active_cell.row;
            if (sheet->active_cell.row < sheet->maxrow)
            {
                if (state==GTK_SHEET_COLUMN_SELECTED)
                    row = MIN_VISIBLE_ROW(sheet)-1;
                if (state==GTK_SHEET_ROW_SELECTED)
                    col = MIN_VISIBLE_COLUMN(sheet);
                row = row + scroll;
                while (row < sheet->maxrow && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row++;
                row = MIN(sheet->maxrow, row);
            }

            text = gtk_sheet_get_entry_text(sheet);
            text_is_empty = (!text || !text[0]);
            g_free(text); text = NULL;

            if (!force_move && !text_is_empty && GTK_IS_TEXT_VIEW(sheet->sheet_entry)) 
                return(FALSE);

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_Right:
            if (extend_selection)
            {
                if (state==GTK_STATE_NORMAL)
                {
                    row=sheet->active_cell.row;
                    col=sheet->active_cell.col;
                    gtk_sheet_click_cell(sheet, row, col, &veto);
                    if (!veto) break;
                }
                if (sheet->selection_cell.col < sheet->maxcol)
                {
                    col = sheet->selection_cell.col + scroll;
                    while (col < sheet->maxcol && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col++;
                    col = MIN(sheet->maxcol, col);
                    gtk_sheet_extend_selection(sheet, sheet->selection_cell.row, col);
                }
                return(TRUE);
            }
            col = sheet->active_cell.col;
            row = sheet->active_cell.row;

            if (state==GTK_SHEET_ROW_SELECTED) 
                col = MIN_VISIBLE_COLUMN(sheet)-1;

            if (state==GTK_SHEET_COLUMN_SELECTED)
                row = MIN_VISIBLE_ROW(sheet);

            col = col + scroll;
            while (col < sheet->maxcol && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col++;

            col = MIN(sheet->maxcol, col);

            text = gtk_sheet_get_entry_text(sheet);
            text_is_empty = (!text || !text[0]);
            g_free(text); text = NULL;
            
            if (!force_move && !text_is_empty) return(FALSE);

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_Left:
            if (extend_selection)
            {
                if (state==GTK_STATE_NORMAL)
                {
                    row=sheet->active_cell.row;
                    col=sheet->active_cell.col;
                    gtk_sheet_click_cell(sheet, row, col, &veto);
                    if (!veto) break;
                }
                if (sheet->selection_cell.col > 0)
                {
                    col = sheet->selection_cell.col - scroll;
                    while (col > 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col--;
                    col = MAX(0, col);
                    gtk_sheet_extend_selection(sheet, sheet->selection_cell.row, col);
                }
                return(TRUE);
            }
            col = sheet->active_cell.col;
            row = sheet->active_cell.row;

            if (state==GTK_SHEET_ROW_SELECTED)
                col = MIN_VISIBLE_COLUMN(sheet)-1;
            if (state==GTK_SHEET_COLUMN_SELECTED)
                row = MIN_VISIBLE_ROW(sheet);

            col = col - scroll;
            while (col > 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) col--;
            col = MAX(0, col);

            text = gtk_sheet_get_entry_text(sheet);
            text_is_empty = (!text || !text[0]);
            g_free(text); text = NULL;

            if (!force_move && !text_is_empty) return(FALSE);

            gtk_sheet_click_cell(sheet, row, col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_Home:
            row=0;
            while (row < sheet->maxrow && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row++;

            gtk_sheet_click_cell(sheet, row, sheet->active_cell.col, &veto);
            extend_selection = FALSE;
            break;

        case GDK_End:
            row=sheet->maxrow;
            while (row > 0 && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row--;

            gtk_sheet_click_cell(sheet, row, sheet->active_cell.col, &veto);
            extend_selection = FALSE;
            break;

        default:
            if (in_selection)
            {
                GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
                if (extend_selection) return(TRUE);
            }
            if (state == GTK_SHEET_ROW_SELECTED)
                sheet->active_cell.col = MIN_VISIBLE_COLUMN(sheet);
            if (state == GTK_SHEET_COLUMN_SELECTED)
                sheet->active_cell.row = MIN_VISIBLE_ROW(sheet);
            return(FALSE);
    }

    if (extend_selection) return(TRUE);

    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
    return(TRUE);
}

/*
 * gtk_sheet_size_request_handler:
 * 
 * this is the #GtkSheet widget class "size-request" signal handler
 * 
 * @param widget the #GtkSheet
 * @param requisition
 *               the #GtkRequisition
 */
static void
    gtk_sheet_size_request_handler (GtkWidget *widget, GtkRequisition *requisition)
{
    GtkSheet *sheet;
    GList *children;
    GtkSheetChild *child;
    GtkRequisition child_requisition;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));
    g_return_if_fail (requisition != NULL);

    sheet = GTK_SHEET (widget);

    requisition->width = 3 * GTK_SHEET_DEFAULT_COLUMN_WIDTH;
    requisition->height = 3 * DEFAULT_ROW_HEIGHT(widget);

    /* compute the size of the column title area */
    if (sheet->column_titles_visible)
        requisition->height += sheet->column_title_area.height;

    /* compute the size of the row title area */
    if (sheet->row_titles_visible)
        requisition->width += sheet->row_title_area.width;

    gtk_sheet_recalc_view_range(sheet);

    children = sheet->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        gtk_widget_size_request(child->widget, &child_requisition);
    }
}


/*
 * gtk_sheet_size_allocate_handler:
 * 
 * this is the #GtkSheet widget class "size-allocate" signal handler
 * 
 * @param widget     the #GtkSheet
 * @param allocation the #GtkAllocation
 */
static void
    gtk_sheet_size_allocate_handler (GtkWidget * widget, GtkAllocation * allocation)
{
    GtkSheet *sheet;
    GtkAllocation sheet_allocation;
    gint border_width;

    g_return_if_fail (widget != NULL);
    g_return_if_fail (GTK_IS_SHEET (widget));
    g_return_if_fail (allocation != NULL);

    sheet = GTK_SHEET (widget);

    gtk_widget_set_allocation(widget, allocation);
    border_width = GTK_CONTAINER(widget)->border_width;

    if (gtk_widget_get_realized (widget))
    {
        gdk_window_move_resize (gtk_widget_get_window(widget),
                                allocation->x + border_width,
                                allocation->y + border_width,
                                allocation->width - 2*border_width,
                                allocation->height - 2*border_width);
    }

    /* use internal allocation structure for all the math
     * because it's easier than always subtracting the container
     * border width */

    sheet->internal_allocation.x = 0;
    sheet->internal_allocation.y = 0;
    sheet->internal_allocation.width = allocation->width - 2 * border_width;
    sheet->internal_allocation.height = allocation->height - 2 * border_width;

    sheet_allocation.x = 0;
    sheet_allocation.y = 0;
    sheet_allocation.width = allocation->width - 2 * border_width;
    sheet_allocation.height = allocation->height - 2 * border_width;

    sheet->sheet_window_width = sheet_allocation.width;
    sheet->sheet_window_height = sheet_allocation.height;

    if (gtk_widget_get_realized (widget))
    {
        gdk_window_move_resize (sheet->sheet_window,
                                sheet_allocation.x,
                                sheet_allocation.y,
                                sheet_allocation.width,
                                sheet_allocation.height);
    }

    /* position the window which holds the column title buttons */

    sheet->column_title_area.x = 0;
    sheet->column_title_area.y = 0;
    if (sheet->row_titles_visible)
        sheet->column_title_area.x = sheet->row_title_area.width;
    sheet->column_title_area.width = sheet_allocation.width - sheet->column_title_area.x;

    if (gtk_widget_get_realized(widget) && sheet->column_titles_visible)
    {
        gdk_window_move_resize (sheet->column_title_window,
                                sheet->column_title_area.x,
                                sheet->column_title_area.y,
                                sheet->column_title_area.width,
                                sheet->column_title_area.height);
    }

    sheet->sheet_window_width = sheet_allocation.width;
    sheet->sheet_window_height = sheet_allocation.height;

    /* column button allocation */
    size_allocate_column_title_buttons (sheet);

    /* position the window which holds the row title buttons */
    sheet->row_title_area.x = 0;
    sheet->row_title_area.y = 0;
    if (sheet->column_titles_visible)
        sheet->row_title_area.y = sheet->column_title_area.height;
    sheet->row_title_area.height = sheet_allocation.height - sheet->row_title_area.y;

    if (gtk_widget_get_realized(widget) && sheet->row_titles_visible)
    {
        gdk_window_move_resize (sheet->row_title_window,
                                sheet->row_title_area.x,
                                sheet->row_title_area.y,
                                sheet->row_title_area.width,
                                sheet->row_title_area.height);
    }

    /* button allocation */
    size_allocate_column_title_buttons(sheet);
    size_allocate_row_title_buttons(sheet);

    gtk_sheet_recalc_view_range(sheet);

    /* re-scale backing pixmap */
    gtk_sheet_make_backing_pixmap(sheet, 0, 0);
    gtk_sheet_position_children(sheet);

    /* set the scrollbars adjustments */
    adjust_scrollbars (sheet);
}

static void
    size_allocate_column_title_buttons (GtkSheet *sheet)
{
    gint i;
    gint x,width;

    if (!sheet->column_titles_visible) return;
    if (!gtk_widget_get_realized (GTK_WIDGET(sheet))) return;

    width = sheet->sheet_window_width;
    x = 0;

    if (sheet->row_titles_visible)
    {
        width -= sheet->row_title_area.width;
        x = sheet->row_title_area.width;
    }

    /* if neccessary, resize the column title window */
    if (sheet->column_title_area.width != width || sheet->column_title_area.x != x)
    {
        sheet->column_title_area.width = width;
        sheet->column_title_area.x = x;
        gdk_window_move_resize (sheet->column_title_window,
                                sheet->column_title_area.x,
                                sheet->column_title_area.y,
                                sheet->column_title_area.width,
                                sheet->column_title_area.height);
    }

    /* if the right edge of the sheet is visible, clear it */
    if (MAX_VISIBLE_COLUMN(sheet) >= sheet->maxcol)
    {
        gdk_window_clear_area (sheet->column_title_window,
                               0,0,
                               sheet->column_title_area.width,
                               sheet->column_title_area.height);
    }

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return;

    for (i = MIN_VISIBLE_COLUMN(sheet); i <= MAX_VISIBLE_COLUMN(sheet); i++)
        gtk_sheet_button_draw(sheet,-1,i);
}

static void
    size_allocate_row_title_buttons(GtkSheet *sheet)
{
    gint i;
    gint y, height;

    if (!sheet->row_titles_visible) return;
    if (!gtk_widget_get_realized (GTK_WIDGET(sheet))) return;

    height = sheet->sheet_window_height;
    y = 0;

    if (sheet->column_titles_visible)
    {
        height -= sheet->column_title_area.height;
        y = sheet->column_title_area.height;
    }

    /* if neccessary, resize the row title window */
    if (sheet->row_title_area.height != height || sheet->row_title_area.y != y)
    {
        sheet->row_title_area.y = y;
        sheet->row_title_area.height = height;
        gdk_window_move_resize (sheet->row_title_window,
                                sheet->row_title_area.x,
                                sheet->row_title_area.y,
                                sheet->row_title_area.width,
                                sheet->row_title_area.height);
    }

    /* if the right edge of the sheet is visible, clear it */
    if (MAX_VISIBLE_ROW(sheet) >= sheet->maxrow)
    {
        gdk_window_clear_area (sheet->row_title_window,
                               0,0,
                               sheet->row_title_area.width,
                               sheet->row_title_area.height);
    }

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return;

    for (i = MIN_VISIBLE_ROW(sheet); i <= MAX_VISIBLE_ROW(sheet); i++)
        gtk_sheet_button_draw(sheet,i,-1);
}

static void
    gtk_sheet_recalc_top_ypixels(GtkSheet *sheet)
{
    gint i, cy;

    if (sheet->column_titles_visible)
        cy = sheet->column_title_area.height;
    else
        cy = 0;

    for (i=0; i<=sheet->maxrow; i++)
    {
        sheet->row[i].top_ypixel = cy;
        if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i))) cy += sheet->row[i].height;
    }
}

static void
    gtk_sheet_recalc_left_xpixels(GtkSheet *sheet)
{
    gint i, cx;

    if (sheet->row_titles_visible)
        cx = sheet->row_title_area.width;
    else
        cx = 0;

    for (i=0; i<=sheet->maxcol; i++)
    {
        sheet->column[i]->left_xpixel = cx;
        if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i))) cx += sheet->column[i]->width;
    }
}

static void
    gtk_sheet_recalc_text_column(GtkSheet *sheet, gint start_column)
{
    int i;

    g_assert(start_column >= -1);

    for (i=start_column+1; i<=sheet->maxcol; i++)  /* for the fresh columns */
    {
        sheet->column[i]->left_text_column = i;
        sheet->column[i]->right_text_column = i;
    }
}



static void
    gtk_sheet_size_allocate_entry(GtkSheet *sheet)
{
    GtkAllocation shentry_allocation;
    GtkSheetCellAttr attributes;
    GtkWidget *sheet_entry;
    GtkStyle *style = NULL, *previous_style = NULL;
    gint row, col;
    gint size, entry_max_size, text_size, column_width, row_height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_mapped(GTK_WIDGET(sheet))) return;
    if (sheet->maxrow < 0 || sheet->maxcol < 0) return;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_size_allocate_entry: called");
#endif

    sheet_entry = gtk_sheet_get_entry(sheet);

    gtk_sheet_get_attributes(sheet, 
                             sheet->active_cell.row, sheet->active_cell.col, 
                             &attributes);

    if (gtk_widget_get_realized(sheet->sheet_entry))
    {
        if (!gtk_widget_get_style(sheet_entry))
            gtk_widget_ensure_style(sheet_entry);

        previous_style = gtk_widget_get_style(sheet_entry);

        style = gtk_style_copy(previous_style);
        style->bg[GTK_STATE_NORMAL] = attributes.background;
        style->fg[GTK_STATE_NORMAL] = attributes.foreground;
        style->text[GTK_STATE_NORMAL] = attributes.foreground;
        style->bg[GTK_STATE_ACTIVE] = attributes.background;
        style->fg[GTK_STATE_ACTIVE] = attributes.foreground;
        style->text[GTK_STATE_ACTIVE] = attributes.foreground;

        pango_font_description_free(style->font_desc);
        style->font_desc = pango_font_description_copy(attributes.font_desc);

        gtk_widget_set_style(sheet_entry, style);
        gtk_widget_size_request(sheet->sheet_entry, NULL);
        gtk_widget_set_style(sheet_entry, previous_style);

        if (style != previous_style)
        {
            if (!GTK_IS_ITEM_ENTRY(sheet->sheet_entry))
            {
                style->bg[GTK_STATE_NORMAL] = previous_style->bg[GTK_STATE_NORMAL];
                style->fg[GTK_STATE_NORMAL] = previous_style->fg[GTK_STATE_NORMAL];
                style->bg[GTK_STATE_ACTIVE] = previous_style->bg[GTK_STATE_ACTIVE];
                style->fg[GTK_STATE_ACTIVE] = previous_style->fg[GTK_STATE_ACTIVE];
            }
            gtk_widget_set_style(sheet_entry, style);
        }
    }

    if (GTK_IS_ITEM_ENTRY(sheet_entry))
        entry_max_size = GTK_ITEM_ENTRY(sheet_entry)->text_max_size;
    else
        entry_max_size = 0;

    text_size = 0;
    {
        gchar *text = gtk_sheet_get_entry_text(sheet);

        if (text && text[0])
        {
            text_size = STRING_WIDTH(GTK_WIDGET(sheet), attributes.font_desc, text);
        }

        g_free(text); 
    }

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (0 <= col && col <= sheet->maxcol)
        column_width = sheet->column[col]->width;
    else
        column_width = GTK_SHEET_DEFAULT_COLUMN_WIDTH;

    if (0 <= row && row <= sheet->maxrow)
        row_height = sheet->row[row].height;
    else
        row_height = GTK_SHEET_DEFAULT_ROW_HEIGHT;

    size = MIN(text_size, entry_max_size);
    size = MAX(size, column_width-2*CELLOFFSET);

    shentry_allocation.x = COLUMN_LEFT_XPIXEL(sheet, col);
    shentry_allocation.y = ROW_TOP_YPIXEL(sheet, row);
    shentry_allocation.width = column_width;
    shentry_allocation.height = row_height;

    if (GTK_IS_ITEM_ENTRY(sheet->sheet_entry))
    {
        shentry_allocation.height -= 2*CELLOFFSET;
        shentry_allocation.y += CELLOFFSET;

        if (gtk_sheet_clip_text(sheet))
            shentry_allocation.width = column_width - 2*CELLOFFSET;
        else
            shentry_allocation.width = size;

        switch (GTK_ITEM_ENTRY(sheet_entry)->justification)
        {
            case GTK_JUSTIFY_CENTER:
                shentry_allocation.x += (column_width)/2 - size/2;
                break;

            case GTK_JUSTIFY_RIGHT:
                shentry_allocation.x += column_width - size - CELLOFFSET;
                break;

            case GTK_JUSTIFY_LEFT:
            case GTK_JUSTIFY_FILL:
                shentry_allocation.x += CELLOFFSET;
                break;
        }
    }

    if (!GTK_IS_ITEM_ENTRY(sheet->sheet_entry))
    {
        shentry_allocation.x += 2;
        shentry_allocation.y += 2;
        shentry_allocation.width -= MIN(shentry_allocation.width, 3);
        shentry_allocation.height -= MIN(shentry_allocation.height, 3);
    }

    gtk_widget_size_allocate(sheet->sheet_entry, &shentry_allocation);

    if (previous_style == style) g_object_unref(previous_style);
}

#ifdef GTK_SHEET_DEBUG
/* obj_ref debug code */
static void weak_notify(gpointer data, GObject *o)
{
    gchar *msg = data;  /* assume a string was passed as data */

    #ifdef GTK_SHEET_DEBUG
    g_debug("weak_notify: %p finalized (%s)", o, msg ? msg : "");
    #endif
}
#endif

static void
    gtk_sheet_entry_set_max_size(GtkSheet *sheet)
{
    gint i;
    gint size=0;
    gint sizel=0, sizer=0;
    gint row,col;
    GtkJustification justification;

    row=sheet->active_cell.row;
    col=sheet->active_cell.col;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_entry_set_max_size: called");
#endif

    if (!GTK_IS_ITEM_ENTRY(sheet->sheet_entry) || gtk_sheet_clip_text(sheet)) return;

    justification = GTK_ITEM_ENTRY(sheet->sheet_entry)->justification;

    switch (justification)
    {
        case GTK_JUSTIFY_FILL:
        case GTK_JUSTIFY_LEFT:
            for (i=col+1; i<=MAX_VISIBLE_COLUMN(sheet); i++)
            {
                if (gtk_sheet_cell_get_text(sheet, row, i)) break;

                if (i < 0 || i > sheet->maxcol) continue;
                size += sheet->column[i]->width;
            }
            size = MIN(size, sheet->sheet_window_width - COLUMN_LEFT_XPIXEL(sheet, col));
            break;

        case GTK_JUSTIFY_RIGHT:
            for (i=col-1; i>=MIN_VISIBLE_COLUMN(sheet); i--)
            {
                if (gtk_sheet_cell_get_text(sheet, row, i)) break;

                if (i < 0 || i > sheet->maxcol) continue;
                size += sheet->column[i]->width;
            }
            break;

        case GTK_JUSTIFY_CENTER:
            for (i=col+1; i<=MAX_VISIBLE_COLUMN(sheet); i++)
            {
                /* if (gtk_sheet_cell_get_text(sheet, row, i)) break; */

                if (i < 0 || i > sheet->maxcol) continue;
                sizer += sheet->column[i]->width;
            }
            for (i=col-1; i>=MIN_VISIBLE_COLUMN(sheet); i--)
            {
                if (gtk_sheet_cell_get_text(sheet, row, i)) break;

                if (i < 0 || i > sheet->maxcol) continue;
                sizel += sheet->column[i]->width;
            }
            size=2*MIN(sizel, sizer);
            break;
    }

    if (size!=0) size+=sheet->column[col]->width;
    GTK_ITEM_ENTRY(sheet->sheet_entry)->text_max_size=size;
}

static void
    create_sheet_entry(GtkSheet *sheet)
{
    GtkWidget *widget;
    GtkWidget *entry, *new_entry;
    GtkStyle *style;

    widget = GTK_WIDGET(sheet);

#ifdef GTK_SHEET_DEBUG
    g_debug("create_sheet_entry: called");
#endif

    style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(sheet)));

    if (sheet->sheet_entry)
    {
        /* avoids warnings */
        g_object_ref(sheet->sheet_entry);
        gtk_widget_unparent(sheet->sheet_entry);
#ifdef GTK_SHEET_DEBUG
        g_debug("create_sheet_entry: destroying old entry %p", sheet->sheet_entry);
#endif
        gtk_widget_destroy(sheet->sheet_entry);
        sheet->sheet_entry = NULL;
    }

    new_entry = gtk_widget_new(
        sheet->entry_type ? sheet->entry_type : G_TYPE_ITEM_ENTRY, NULL);

    sheet->sheet_entry = new_entry;
    entry = gtk_sheet_get_entry(sheet);

    if (!entry)  /* this was an unsupported entry type */
    {
        g_warning ("Unsupported entry type - widget must contain an GtkEditable or GtkTextView");
        gtk_widget_destroy(new_entry);
        
        new_entry = gtk_item_entry_new();
        sheet->sheet_entry = new_entry;
    }
    g_object_ref_sink(sheet->sheet_entry);

#ifdef GTK_SHEET_DEBUG
    g_object_weak_ref(G_OBJECT(sheet->sheet_entry), weak_notify, "Sheet-Entry");
#endif

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        gtk_widget_size_request(sheet->sheet_entry, NULL);

#ifdef GTK_SHEET_DEBUG
        g_debug("create_sheet_entry: sheet was realized");
#endif
        gtk_widget_set_parent_window (sheet->sheet_entry, sheet->sheet_window);
        gtk_widget_set_parent(sheet->sheet_entry, GTK_WIDGET(sheet));
        gtk_widget_realize(sheet->sheet_entry);
    }

    g_signal_connect_swapped(GTK_OBJECT(entry),"key_press_event",
                             (void *) gtk_sheet_entry_key_press_handler,
                             GTK_OBJECT(sheet));

    gtk_widget_show (sheet->sheet_entry);
}


/**
 * gtk_sheet_get_entry_type:
 * @sheet: a #GtkSheet
 *
 * Get sheets entry type, if known
 *
 * Returns: a #GtkSheetEntryType or GTK_SHEET_ENTRY_TYPE_DEFAULT
 */
GType 
    gtk_sheet_get_entry_type(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet, GTK_SHEET_ENTRY_TYPE_DEFAULT);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), GTK_SHEET_ENTRY_TYPE_DEFAULT);

    return(sheet->entry_type);
}

/**
 * gtk_sheet_get_entry:
 * @sheet: a #GtkSheet
 *
 * Get sheet's entry widget. 
 *  
 * If the entry widget is a container, the direct childs of the 
 * container are searched for a valid entry widget. If you want
 * the container itself to be returned, you should use 
 * #gtk_sheet_get_entry_widget() instead. 
 *
 * Returns: a #GtkWidget or NULL
 */
GtkWidget *
    gtk_sheet_get_entry(GtkSheet *sheet)
{
    GtkWidget *parent;
    GtkWidget *entry = NULL;
    GtkTableChild *table_child;
    GtkBoxChild *box_child;
    GList *children = NULL;

    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);
    g_return_val_if_fail (sheet->sheet_entry != NULL, NULL);

    if (GTK_IS_EDITABLE(sheet->sheet_entry)) return(sheet->sheet_entry);
    if (GTK_IS_TEXT_VIEW(sheet->sheet_entry)) return(sheet->sheet_entry);

    parent = GTK_WIDGET(sheet->sheet_entry);

    if (GTK_IS_TABLE(parent)) children = GTK_TABLE(parent)->children;
    if (GTK_IS_BOX(parent)) children = GTK_BOX(parent)->children;

    if (!children) return(NULL);

    while (children)
    {
        if (GTK_IS_TABLE(parent))
        {
            table_child = children->data;
            entry = table_child->widget;
        }

        if (GTK_IS_BOX(parent))
        {
            box_child = children->data;
            entry = box_child->widget;
        }

        if (GTK_IS_EDITABLE(entry)) return(entry);
        if (GTK_IS_TEXT_VIEW(entry)) return(entry);

        children = children->next;
    }

    return(NULL);
}

/**
 * gtk_sheet_get_entry_widget:
 * @sheet: a #GtkSheet
 *
 * Get sheet's entry widget. 
 *  
 * If the entry widget is a container, the container widget is 
 * returned. In order to get the entry in the container child, 
 * you might want to use #gtk_sheet_get_entry() instead. 
 *
 * Returns: a #GtkWidget or NULL
 */
GtkWidget *
    gtk_sheet_get_entry_widget(GtkSheet *sheet)
{
    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);
    g_return_val_if_fail (sheet->sheet_entry != NULL, NULL);

    return(sheet->sheet_entry);
}

/**
 * gtk_sheet_get_entry_text:
 * @sheet: a #GtkSheet
 *
 * Get the text out of the sheet_entry. 
 *  
 * This function is mainly used to synchronize the text of a 
 * second entry with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 *
 * Returns: a copy of the sheet_entry text or NULL. This 
 * function returns an allocated string, so g_free() it after 
 * usage! 
 */
gchar *gtk_sheet_get_entry_text(GtkSheet *sheet)
{
    gchar *text = NULL;
    GtkWidget *entry = NULL;

    g_return_val_if_fail (sheet != NULL, NULL);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), NULL);

    entry = gtk_sheet_get_entry(sheet);
    g_return_val_if_fail (entry != NULL, NULL);

    if (GTK_IS_EDITABLE(entry))
    {
        text = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_get_entry_text: got %s", text ? text : "");
#endif
    }
    else if (GTK_IS_TEXT_VIEW(entry))
    {
        GtkTextIter start,end;
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        text = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
    }
    else
    {
        g_warning("gtk_sheet_get_entry_text: no GTK_EDITABLE, don't know how to get the text.");
    }
    return(text);
}

/**
 * gtk_sheet_set_entry_text:
 * @sheet: a #GtkSheet 
 * @text: the text to be set or NULL 
 *
 * Set the text in the sheet_entry (and active cell).
 *  
 * This function is mainly used to synchronize the text of a 
 * second entry with the sheet_entry.
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 */
void gtk_sheet_set_entry_text(GtkSheet *sheet, const gchar *text)
{
    GtkWidget *entry = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail (entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
        gint position = 0;
        gtk_editable_delete_text(GTK_EDITABLE(entry), 0, -1);
        gtk_editable_insert_text(GTK_EDITABLE(entry), text, -1, &position);
    }
    else if (GTK_IS_TEXT_VIEW(entry))
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
        gtk_text_buffer_set_text(buffer, text, -1);
    }
    else
    {
        g_warning("gtk_sheet_set_entry_text: no GTK_EDITABLE, don't know how to set the text.");
    }
}

/**
 * gtk_sheet_set_entry_editable:
 * @sheet: a #GtkSheet 
 * @editable: editable flag
 *
 * Set the editable flag in the sheet_entry
 *  
 * This function is mainly used to synchronize the editable flag
 * of a second entry with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 */
void gtk_sheet_set_entry_editable(GtkSheet *sheet, const gboolean editable)
{
    GtkWidget *entry = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail (entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
        gtk_editable_set_editable(GTK_EDITABLE(entry), editable);
    }
    else if (GTK_IS_TEXT_VIEW(entry))
    {
        gtk_text_view_set_editable(GTK_TEXT_VIEW(entry), editable);
    }
    else
    {
        g_warning("gtk_sheet_set_entry_editable: no GTK_EDITABLE, don't know how to set editable.");
    }
}

/**
 * gtk_sheet_entry_signal_connect_changed:
 * @sheet: a #GtkSheet 
 * @handler: the signal handler
 *
 * Connect a handler to the sheet_entry "changed" signal
 *  
 * This function is mainly used to synchronize a second entry 
 * widget with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 *  
 * Returns: the handler id 
 */
gulong gtk_sheet_entry_signal_connect_changed(GtkSheet *sheet, GCallback handler)
{
    GtkWidget *entry = NULL;
    gulong handler_id = 0;

    g_return_val_if_fail(sheet != NULL, handler_id);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), handler_id);

    entry = gtk_sheet_get_entry(sheet);
    g_return_val_if_fail (entry != NULL, handler_id);

    if (GTK_IS_EDITABLE(entry))
    {
        handler_id = gtk_signal_connect(GTK_OBJECT(entry), 
                           "changed", handler, GTK_OBJECT(sheet));
    }
    else if (GTK_IS_TEXT_VIEW(entry))
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));

        handler_id = g_signal_connect(G_OBJECT(buffer),
                         "changed", handler, GTK_OBJECT(sheet));
    }
    else
    {
        g_warning("gtk_sheet_entry_signal_connect_changed: no GTK_EDITABLE, don't know how to set editable.");
    }
    return(handler_id);
}

/**
 * gtk_sheet_entry_signal_disconnect_by_func:
 * @sheet: a #GtkSheet 
 * @handler: the signal handler
 *
 * Disconnect a handler from the sheet_entry "changed" signal
 *  
 * This function is mainly used to synchronize a second entry 
 * widget with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 *  
 * Returns: the handler id 
 */
void gtk_sheet_entry_signal_disconnect_by_func(GtkSheet *sheet, GCallback handler)
{
    GtkWidget *entry = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail (entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
        g_signal_handlers_disconnect_by_func(G_OBJECT(entry),
                                             handler, GTK_OBJECT(sheet));
    }
    else if (GTK_IS_TEXT_VIEW(entry))
    {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));

        g_signal_handlers_disconnect_by_func(G_OBJECT(buffer),
                                             handler, GTK_OBJECT(sheet));
    }
    else
    {
        g_warning("gtk_sheet_entry_signal_disconnect_by_func: no GTK_EDITABLE, don't know how to set editable.");
    }
}



/* BUTTONS */
static void
    row_button_set (GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow) return;
    if (sheet->row[row].button.state == GTK_STATE_ACTIVE) return;

    sheet->row[row].button.state = GTK_STATE_ACTIVE;
    gtk_sheet_button_draw(sheet, row, -1);
}

static void
    column_button_set (GtkSheet *sheet, gint column)
{
    if (column < 0 || column > sheet->maxcol) return;
    if (sheet->column[column]->button.state == GTK_STATE_ACTIVE) return;

    sheet->column[column]->button.state = GTK_STATE_ACTIVE;
    gtk_sheet_button_draw(sheet, -1, column);
}

static void
    row_button_release (GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow) return;
    if (sheet->row[row].button.state == GTK_STATE_NORMAL) return;

    sheet->row[row].button.state = GTK_STATE_NORMAL;
    gtk_sheet_button_draw(sheet, row, -1);
}

static void
    column_button_release (GtkSheet *sheet, gint column)
{
    if (column < 0 || column > sheet->maxcol) return;
    if (sheet->column[column]->button.state == GTK_STATE_NORMAL) return;

    sheet->column[column]->button.state = GTK_STATE_NORMAL;
    gtk_sheet_button_draw(sheet, -1, column);
}

static void
    gtk_sheet_button_draw (GtkSheet *sheet, gint row, gint column)
{
    GdkWindow *window = NULL;
    GtkShadowType shadow_type;
    guint width = 0, height = 0;
    gint x = 0, y = 0;
    gint index = 0;
    gint text_width = 0, text_height = 0;
    GtkSheetButton *button = NULL;
    GtkSheetChild *child = NULL;
    GdkRectangle allocation;
    gboolean sensitive = FALSE;
    gint state = 0;
    gint len = 0;
    gchar *line = 0;
    gchar *words = 0;
    gchar label[10];
    PangoAlignment align = PANGO_ALIGN_LEFT;
    gboolean rtl;
    GtkSheetArea area;

    rtl = gtk_widget_get_direction(GTK_WIDGET(sheet)) == GTK_TEXT_DIR_RTL;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    if ((row == -1) && (column == -1)) return;

    if (row >= 0)
    {
        if (row > sheet->maxrow) return;
        if (!sheet->row_titles_visible) return;
        if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) return;
        if (row < MIN_VISIBLE_ROW(sheet)) return;
        if (row > MAX_VISIBLE_ROW(sheet)) return;
    }

    if (column >= 0)
    {
        if (column > sheet->maxcol) return;
        if (!sheet->column_titles_visible) return;
        if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column))) return;
        if (column < MIN_VISIBLE_COLUMN(sheet)) return;
        if (column > MAX_VISIBLE_COLUMN(sheet)) return;
    }

    if (row == -1)
    {
        window=sheet->column_title_window;
        button=&sheet->column[column]->button;
        index=column;
        x = COLUMN_LEFT_XPIXEL(sheet, column) + CELL_SPACING;
        if (sheet->row_titles_visible) x -= sheet->row_title_area.width;
        y = 0;
        width = sheet->column[column]->width;
        height = sheet->column_title_area.height;
        sensitive = GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column));
        area = ON_COLUMN_TITLES_AREA;
    }
    else if (column==-1)
    {
        window=sheet->row_title_window;
        button=&sheet->row[row].button;
        index=row;
        x = 0;
        y = ROW_TOP_YPIXEL(sheet, row)+CELL_SPACING;
        if (sheet->column_titles_visible) y -= sheet->column_title_area.height;
        width = sheet->row_title_area.width;
        height = sheet->row[row].height;
        sensitive = GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row));
        area = ON_ROW_TITLES_AREA;
    }

    allocation.x = x;
    allocation.y = y;
    allocation.width = width;
    allocation.height = height;

    gdk_window_clear_area (window,
                           x, y,
                           width, height);

    gtk_paint_box (gtk_widget_get_style(sheet->button), window,
                   GTK_STATE_NORMAL, GTK_SHADOW_OUT,
                   &allocation, GTK_WIDGET(sheet->button),
                   "button", x, y, width, height);

    state = button->state;
    if (!sensitive) state=GTK_STATE_INSENSITIVE;

    if (state == GTK_STATE_ACTIVE)
        shadow_type = GTK_SHADOW_IN;
    else
        shadow_type = GTK_SHADOW_OUT;

    if (state != GTK_STATE_NORMAL && state != GTK_STATE_INSENSITIVE)
        gtk_paint_box (gtk_widget_get_style(sheet->button), window,
                       button->state, shadow_type,
                       &allocation, GTK_WIDGET(sheet->button),
                       "button", x, y, width, height);

    if (button->label_visible)
    {
        text_height=DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet))-2*CELLOFFSET;

        gdk_gc_set_clip_rectangle(
                                 gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[button->state], 
                                 &allocation);
        gdk_gc_set_clip_rectangle(
                                 gtk_widget_get_style(GTK_WIDGET(sheet))->white_gc, 
                                 &allocation);

/*
    y += DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet))/2 + gtk_widget_get_style(sheet->button)->ythickness + DEFAULT_FONT_DESCENT(GTK_WIDGET(sheet));
*/
        y += 2*gtk_widget_get_style(sheet->button)->ythickness;

        if (button->label && button->label[0])
        {
            PangoLayout *layout = NULL;
            gint real_x = x, real_y = y;

            words=button->label;
            line = g_new(gchar, 1);
            line[0]='\0';

            while (words && *words != '\0')
            {
                if (*words != '\n')
                {
                    len=strlen(line);
                    line=g_realloc(line, len+2);
                    line[len]=*words;
                    line[len+1]='\0';
                }

                if (*words == '\n' || *(words+1) == '\0')
                {
                    text_width = STRING_WIDTH(
                                             GTK_WIDGET(sheet), 
                                             gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc,
                                             line);

                    layout = gtk_widget_create_pango_layout (GTK_WIDGET(sheet), line);
                    switch (button->justification)
                    {
                        case GTK_JUSTIFY_LEFT:
                            real_x = x + CELLOFFSET;
                            align = rtl ? PANGO_ALIGN_RIGHT : PANGO_ALIGN_LEFT;
                            break;
                        case GTK_JUSTIFY_RIGHT:
                            real_x = x + width - text_width - CELLOFFSET;
                            align = rtl ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT;
                            break;
                        case GTK_JUSTIFY_CENTER:
                        default:
                            real_x = x + (width - text_width)/2;
                            align = rtl ? PANGO_ALIGN_RIGHT : PANGO_ALIGN_LEFT;
                            pango_layout_set_justify (layout, TRUE);
                    }
                    pango_layout_set_alignment (layout, align);
                    gtk_paint_layout (
                                     gtk_widget_get_style(GTK_WIDGET(sheet)),
                                     window,
                                     state,
                                     FALSE,
                                     &allocation,
                                     GTK_WIDGET(sheet),
                                     "label",
                                     real_x, real_y,
                                     layout);
                    g_object_unref(G_OBJECT(layout));

                    real_y += text_height + 2;

                    g_free(line);
                    line = g_new(gchar, 1);
                    line[0]='\0';
                }
                words++;
            }
            g_free(line);
        }
        else
        {
            PangoLayout *layout = NULL;
            gint real_x = x, real_y = y;

            sprintf(label,"%d",index);
            text_width = STRING_WIDTH(GTK_WIDGET(sheet), 
                                      gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc,
                                      label);

            layout = gtk_widget_create_pango_layout (GTK_WIDGET(sheet), label);
            switch (button->justification)
            {
                case GTK_JUSTIFY_LEFT:
                    real_x = x + CELLOFFSET;
                    align = rtl ? PANGO_ALIGN_RIGHT : PANGO_ALIGN_LEFT;
                    break;

                case GTK_JUSTIFY_RIGHT:
                    real_x = x + width - text_width - CELLOFFSET;
                    align = rtl ? PANGO_ALIGN_LEFT : PANGO_ALIGN_RIGHT;
                    break;

                case GTK_JUSTIFY_CENTER:
                    /* FALLTHROUGH */

                default:
                    real_x = x + (width - text_width)/2;
                    align = rtl ? PANGO_ALIGN_RIGHT : PANGO_ALIGN_LEFT;
                    pango_layout_set_justify (layout, TRUE);
            }
            pango_layout_set_alignment (layout, align);
            gtk_paint_layout (gtk_widget_get_style(GTK_WIDGET(sheet)),
                              window,
                              state,
                              FALSE,
                              &allocation,
                              GTK_WIDGET(sheet),
                              "label",
                              real_x, real_y,
                              layout);
            g_object_unref(G_OBJECT(layout));
        }

        gdk_gc_set_clip_rectangle(
                                 gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[button->state], 
                                 NULL);
        gdk_gc_set_clip_rectangle(
                                 gtk_widget_get_style(GTK_WIDGET(sheet))->white_gc, 
                                 NULL);
    }

    gtk_sheet_draw_tooltip_marker(sheet, area, row, column);

    if ((child = button->child) && (child->widget))
    {
        GtkRequisition requisition;

        child->x = allocation.x;
        child->y = allocation.y;

        gtk_widget_get_requisition( child->widget, &requisition);

        child->x += (width - requisition.width) / 2;
        child->y += (height - requisition.height) / 2;

        allocation.x = child->x;
        allocation.y = child->y;
        allocation.width = requisition.width;
        allocation.height = requisition.height;

        x = child->x;
        y = child->y;

        gtk_widget_set_state(child->widget, button->state);

        if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && 
            gtk_widget_get_mapped(child->widget))
        {
            gtk_widget_size_allocate(child->widget, &allocation);
            gtk_widget_queue_draw(child->widget);
        }
    }
}


/* SCROLLBARS
 *
 * functions:
 *   adjust_scrollbars
 *   vadjustment_changed
 *   hadjustment_changed
 *   vadjustment_value_changed
 *   hadjustment_value_changed */

static void
    adjust_scrollbars (GtkSheet * sheet)
{

    if (sheet->vadjustment)
    {
        sheet->vadjustment->page_size = sheet->sheet_window_height;
        sheet->vadjustment->page_increment = sheet->sheet_window_height / 2;
        sheet->vadjustment->step_increment = DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet));
        sheet->vadjustment->lower = 0;
        sheet->vadjustment->upper = SHEET_HEIGHT (sheet) + 80;
/*
        if (sheet->sheet_window_height - sheet->voffset > SHEET_HEIGHT (sheet))
        {
          sheet->vadjustment->value = MAX(0, SHEET_HEIGHT (sheet) - sheet->sheet_window_height);
          g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
        }
*/
        g_signal_emit_by_name (GTK_OBJECT(sheet->vadjustment), "changed");
    }

    if (sheet->hadjustment)
    {
        sheet->hadjustment->page_size = sheet->sheet_window_width;
        sheet->hadjustment->page_increment = sheet->sheet_window_width / 2;
        sheet->hadjustment->step_increment = GTK_SHEET_DEFAULT_COLUMN_WIDTH;
        sheet->hadjustment->lower = 0;
        sheet->hadjustment->upper = SHEET_WIDTH (sheet)+ 80;
/*
        if (sheet->sheet_window_width - sheet->hoffset > SHEET_WIDTH (sheet))
        {
          sheet->hadjustment->value = MAX(0, SHEET_WIDTH (sheet) - sheet->sheet_window_width);
          g_signal_emit_by_name (GTK_OBJECT(sheet->hadjustment), "value_changed");
        }
*/
        g_signal_emit_by_name (GTK_OBJECT(sheet->hadjustment), "changed");
    }

/*
    if(gtk_widget_get_realized(sheet))
    {
        if(sheet->row_titles_visible){
                    size_allocate_row_title_buttons(sheet);
                    gdk_window_show(sheet->row_title_window);
        }

        if(sheet->column_titles_visible){
                    size_allocate_column_title_buttons(sheet);
                    gdk_window_show(sheet->column_title_window);
        }

        gtk_sheet_range_draw(sheet, NULL);
    }
*/
}


/*
 * vadjustment_changed_handler:
 * 
 * this is the #GtkSheet vertical adjustment "changed" signal handler
 * 
 * @param adjustment the #GtkAdjustment that received the signal
 * @param data the #GtkSheet passed on signal creation
 */
static void
    vadjustment_changed_handler (GtkAdjustment * adjustment,
                         gpointer data)
{
    GtkSheet *sheet;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);

    sheet = GTK_SHEET (data);
}

/*
 * hadjustment_changed_handler:
 * 
 * this is the #GtkSheet horizontal adjustment "change" handler
 * 
 * @param adjustment the #GtkAdjustment that received the signal
 * @param data       the #GtkSheet passed on signal creation
 */
static void
    hadjustment_changed_handler (GtkAdjustment * adjustment, gpointer data)
{
    GtkSheet *sheet;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);

    sheet = GTK_SHEET (data);
}


/*
 * vadjustment_value_changed_handler:
 * 
 * this is the #GtkSheet vertical adjustment "value-changed" signal handler
 * 
 * @param adjustment the #GtkAdjustment that received the signal
 * @param data       the #GtkSheet passed on signal creation
 */
static void
    vadjustment_value_changed_handler (GtkAdjustment * adjustment, gpointer data)
{
    GtkSheet *sheet;
    gint diff, value, old_value;
    gint i;
    gint row, new_row;
    gint y=0;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);
    g_return_if_fail (GTK_IS_SHEET (data));

    sheet = GTK_SHEET (data);

    if (GTK_SHEET_IS_FROZEN(sheet)) return;

    if (sheet->column_titles_visible)
        row=ROW_FROM_YPIXEL(sheet,sheet->column_title_area.height + CELL_SPACING);
    else
        row=ROW_FROM_YPIXEL(sheet,CELL_SPACING);

    old_value = -sheet->voffset;

    for (i=0; i<sheet->maxrow; i++)  /* all but the last row */
    {
        if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i))) y+=sheet->row[i].height;
        if (y > adjustment->value) break;
    }
    if (0 <= i && i <= sheet->maxrow) y -= sheet->row[i].height;
    new_row = i;

    y = MAX(y, 0);

#if 0
    if (adjustment->value > sheet->old_vadjustment && sheet->old_vadjustment > 0. &&
        0 <= new_row && new_row <= sheet->maxrow &&
        sheet->row[new_row].height > sheet->vadjustment->step_increment)
    {
        /* This avoids embarrassing twitching */
        if (row == new_row && row != sheet->maxrow &&
            adjustment->value - sheet->old_vadjustment >=
            sheet->vadjustment->step_increment &&
            new_row + 1 != MIN_VISIBLE_ROW(sheet))
        {
            new_row+=1;
            y=y+sheet->row[row].height;
        }
    }
#else
    if (adjustment->value > sheet->old_vadjustment && sheet->old_vadjustment > 0. &&
        0 <= new_row && new_row <= sheet->maxrow &&
        sheet->row[new_row].height > sheet->vadjustment->step_increment)
    {
        new_row+=1;
        y=y+sheet->row[row].height;
    }
#endif

    /* Negative old_adjustment enforces the redraw, otherwise avoid spureous redraw */
    if (sheet->old_vadjustment >= 0. && row == new_row)
    {
        sheet->old_vadjustment = sheet->vadjustment->value;
        return;
    }

#ifdef GTK_SHEET_DEBUG
    g_debug("hadjustment_value_changed: redraw ova %g row %d new %d", sheet->old_vadjustment, row, new_row);
#endif

    sheet->old_vadjustment = sheet->vadjustment->value;
    adjustment->value=y;

    if (new_row < 0 || new_row > sheet->maxrow)
    {
        sheet->vadjustment->step_increment = GTK_SHEET_DEFAULT_ROW_HEIGHT;
    }
    else if (new_row == 0)
    {
        sheet->vadjustment->step_increment = sheet->row[0].height;
    }
    else
    {
        sheet->vadjustment->step_increment =
            MIN(sheet->row[new_row].height, sheet->row[new_row-1].height);
    }

    sheet->vadjustment->value=adjustment->value;
    value = adjustment->value;

    if (value >= -sheet->voffset)
    {
        /* scroll down */
        diff = value + sheet->voffset;
    }
    else
    {
        /* scroll up */
        diff = -sheet->voffset - value;
    }

    sheet->voffset = -value;

    gtk_sheet_recalc_view_range(sheet);

    if (gtk_widget_get_realized(sheet->sheet_entry) &&
        sheet->state == GTK_SHEET_NORMAL &&
        sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0 &&
        !gtk_sheet_cell_isvisible(sheet, sheet->active_cell.row, sheet->active_cell.col))
    {
        gchar *text = gtk_sheet_get_entry_text(sheet);

        if (!text || !text[0])
        {
            gtk_sheet_cell_clear(sheet, 
                                 sheet->active_cell.row, 
                                 sheet->active_cell.col);
        }
        g_free(text);

        gtk_widget_unmap(sheet->sheet_entry);
    }


    gtk_sheet_position_children(sheet);

    gtk_sheet_range_draw(sheet, NULL);
    size_allocate_row_title_buttons(sheet);
    size_allocate_global_button(sheet);
}

/*
 * hadjustment_value_changed_handler:
 * 
 * this is the #GtkSheet horizontal adjustment "value-changed" handler
 * 
 * @param adjustment the #GtkAdjustment that received the signal
 * @param data       the #GtkSheet passed on signal creation
 */
static void
    hadjustment_value_changed_handler (GtkAdjustment * adjustment, gpointer data)
{
    GtkSheet *sheet;
    gint i, diff, value, old_value;
    gint column, new_column;
    gint x=0;

    g_return_if_fail (adjustment != NULL);
    g_return_if_fail (data != NULL);
    g_return_if_fail (GTK_IS_SHEET (data));

    sheet = GTK_SHEET (data);

    if (GTK_SHEET_IS_FROZEN(sheet)) return;

    if (sheet->row_titles_visible)
        column = COLUMN_FROM_XPIXEL(sheet,sheet->row_title_area.width + CELL_SPACING);
    else
        column = COLUMN_FROM_XPIXEL(sheet, CELL_SPACING);

    old_value = -sheet->hoffset;

    for (i=0; i<sheet->maxcol; i++)  /* all but the last column */
    {
        if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i))) x += sheet->column[i]->width;
        if (x > adjustment->value) break;
    }
    if (0 <= i && i <= sheet->maxcol) x -= sheet->column[i]->width;
    new_column=i;

    x = MAX(x, 0);

#ifdef GTK_SHEET_DEBUG
    g_debug("hadjustment_value_changed: A v %g nc %d x %d", adjustment->value, new_column, x);
#endif

#if 0
    if (adjustment->value > sheet->old_hadjustment && sheet->old_hadjustment > 0 &&
        0 <= new_column && new_column <= sheet->maxcol &&
        sheet->column[new_column]->width > sheet->hadjustment->step_increment)
    {
        /* This avoids embarrassing twitching */
        if (column == new_column && column != sheet->maxcol &&
            adjustment->value - sheet->old_hadjustment >=
            sheet->hadjustment->step_increment &&
            new_column + 1 != MIN_VISIBLE_COLUMN(sheet))
        {
            new_column+=1;
            x=x+sheet->column[column]->width;
        }
    }
#else
    if (adjustment->value > sheet->old_hadjustment && sheet->old_hadjustment > 0 &&
        0 <= new_column && new_column <= sheet->maxcol &&
        sheet->column[new_column]->width > sheet->hadjustment->step_increment)
    {
        new_column+=1;
        x=x+sheet->column[column]->width;
    }
#endif

#ifdef GTK_SHEET_DEBUG
    g_debug("hadjustment_value_changed: B v %g nc %d x %d", adjustment->value, new_column, x);
#endif

    /* Negative old_adjustment enforces the redraw, otherwise avoid spureous redraw */
    if (sheet->old_hadjustment >= 0. && new_column == column)
    {
        sheet->old_hadjustment = sheet->hadjustment->value;
        return;
    }

    sheet->old_hadjustment = sheet->hadjustment->value;
    adjustment->value=x;

    if (new_column < 0 || new_column > sheet->maxcol)
    {
        sheet->hadjustment->step_increment = GTK_SHEET_DEFAULT_COLUMN_WIDTH;
    }
    else if (new_column == 0)
    {
        sheet->hadjustment->step_increment = sheet->column[0]->width;
    }
    else
    {
        sheet->hadjustment->step_increment =
            MIN(sheet->column[new_column]->width, sheet->column[new_column-1]->width);
    }

    sheet->hadjustment->value=adjustment->value;
    value = adjustment->value;

    if (value >= -sheet->hoffset)
    {
        /* scroll right */
        diff = value + sheet->hoffset;
    }
    else
    {
        /* scroll left */
        diff = -sheet->hoffset - value;
    }

    sheet->hoffset = -value;

    gtk_sheet_recalc_view_range(sheet);

    if (gtk_widget_get_realized(sheet->sheet_entry) &&
        sheet->state == GTK_SHEET_NORMAL &&
        sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0 &&
        !gtk_sheet_cell_isvisible(sheet, sheet->active_cell.row, sheet->active_cell.col))
    {
        gchar *text = gtk_sheet_get_entry_text(sheet);

        if (!text || !text[0])
        {
            gtk_sheet_cell_clear(sheet, 
                                 sheet->active_cell.row, 
                                 sheet->active_cell.col);
        }
        g_free(text);

        gtk_widget_unmap(sheet->sheet_entry);
    }

    gtk_sheet_position_children(sheet);
    gtk_sheet_range_draw(sheet, NULL);
    size_allocate_column_title_buttons(sheet);
}


/* COLUMN RESIZING */
static void
    draw_xor_vline (GtkSheet * sheet)
{
    GtkWidget *widget;

    g_return_if_fail (sheet != NULL);

    widget = GTK_WIDGET (sheet);

    gdk_draw_line (gtk_widget_get_window(widget), sheet->xor_gc,
                   sheet->x_drag,
                   sheet->column_title_area.height,
                   sheet->x_drag,
                   sheet->sheet_window_height + 1);
}

/* ROW RESIZING */
static void
    draw_xor_hline (GtkSheet * sheet)
{
    GtkWidget *widget;

    g_return_if_fail (sheet != NULL);

    widget = GTK_WIDGET (sheet);

    gdk_draw_line (gtk_widget_get_window(widget), sheet->xor_gc,
                   sheet->row_title_area.width,
                   sheet->y_drag,
                   sheet->sheet_window_width + 1,
                   sheet->y_drag);
}

/* SELECTED RANGE */
static void
    draw_xor_rectangle(GtkSheet *sheet, GtkSheetRange range)
{
    gint i;
    GdkRectangle clip_area, area;
    GdkGCValues values;

    area.x=COLUMN_LEFT_XPIXEL(sheet, range.col0);
    area.y=ROW_TOP_YPIXEL(sheet, range.row0);
    area.width=COLUMN_LEFT_XPIXEL(sheet, range.coli)-area.x+
        sheet->column[range.coli]->width;
    area.height=ROW_TOP_YPIXEL(sheet, range.rowi)-area.y+
        sheet->row[range.rowi].height;

    clip_area.x=sheet->row_title_area.width;
    clip_area.y=sheet->column_title_area.height;
    clip_area.width=sheet->sheet_window_width;
    clip_area.height=sheet->sheet_window_height;

    if (!sheet->row_titles_visible) clip_area.x = 0;
    if (!sheet->column_titles_visible) clip_area.y = 0;

    if (area.x<0)
    {
        area.width=area.width+area.x;
        area.x=0;
    }
    if (area.width>clip_area.width) area.width=clip_area.width+10;
    if (area.y<0)
    {
        area.height=area.height+area.y;
        area.y=0;
    }
    if (area.height>clip_area.height) area.height=clip_area.height+10;

    clip_area.x--;
    clip_area.y--;
    clip_area.width+=3;
    clip_area.height+=3;

    gdk_gc_get_values(sheet->xor_gc, &values);

    gdk_gc_set_clip_rectangle(sheet->xor_gc, &clip_area);

    for (i=-1;i<=1;++i)
        gdk_draw_rectangle(sheet->sheet_window,
                           sheet->xor_gc,
                           FALSE,
                           area.x+i, area.y+i,
                           area.width-2*i, area.height-2*i);


    gdk_gc_set_clip_rectangle(sheet->xor_gc, NULL);

    gdk_gc_set_foreground(sheet->xor_gc, &values.foreground);
}


/* this function returns the new width of the column being resized given
 * the column and x position of the cursor; the x cursor position is passed
 * in as a pointer and automaticaly corrected if it's beyond min/max limits */

static guint
    new_column_width (GtkSheet *sheet, gint column, gint *x)
{
    gint cx, width;
    GtkRequisition requisition;

    cx = *x;

    requisition.width = sheet->column[column]->requisition;

    /* you can't shrink a column to less than its minimum width */
    if (cx < COLUMN_LEFT_XPIXEL (sheet, column) + requisition.width)
    {
        *x = cx = COLUMN_LEFT_XPIXEL (sheet, column) + requisition.width;
    }

    /* don't grow past the end of the window */
    /*
    if (cx > sheet->sheet_window_width)
      {
        *x = cx = sheet->sheet_window_width;
      }
      */

    /* calculate new column width making sure it doesn't end up
     * less than the minimum width */
    width = cx - COLUMN_LEFT_XPIXEL (sheet, column);
    if (width < requisition.width) width = requisition.width;

    sheet->column[column]->width = width;
    gtk_sheet_recalc_left_xpixels(sheet);
    gtk_sheet_recalc_view_range(sheet);

    size_allocate_column_title_buttons (sheet);

    return (width);
}

/* this function returns the new height of the row being resized given
 * the row and y position of the cursor; the y cursor position is passed
 * in as a pointer and automaticaly corrected if it's beyond min/max limits */

static guint
    new_row_height (GtkSheet *sheet, gint row, gint *y)
{
    GtkRequisition requisition;
    gint cy, height;

    cy = *y;

    requisition.height = sheet->row[row].requisition;

    /* you can't shrink a row to less than its minimum height */
    if (cy < ROW_TOP_YPIXEL (sheet, row) + requisition.height)
    {
        *y = cy = ROW_TOP_YPIXEL (sheet, row) + requisition.height;
    }

    /* don't grow past the end of the window */
    /*
    if (cy > sheet->sheet_window_height)
      {
        *y = cy = sheet->sheet_window_height;
      }
      */

    /* calculate new row height making sure it doesn't end up
     * less than the minimum height */
    height = (cy - ROW_TOP_YPIXEL (sheet, row));
    if (height < requisition.height) height = requisition.height;

    sheet->row[row].height = height;
    gtk_sheet_recalc_top_ypixels(sheet);
    gtk_sheet_recalc_view_range(sheet);

    size_allocate_row_title_buttons(sheet);

    return(height);
}

/**
 * gtk_sheet_set_column_width:
 * @sheet: a #GtkSheet.
 * @column: column number.
 * @width: the width of the column.
 *
 * Set column width.
 */
void
    gtk_sheet_set_column_width (GtkSheet * sheet,
                                gint column,
                                guint width)
{
    guint min_width;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (column < 0 || column > sheet->maxcol) return;

    gtk_sheet_column_size_request(sheet, column, &min_width);

    if (width < min_width) return;

    sheet->column[column]->width = width;

    gtk_sheet_recalc_left_xpixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet))
    {
        size_allocate_column_title_buttons (sheet);
        adjust_scrollbars (sheet);
        gtk_sheet_size_allocate_entry(sheet);
        gtk_sheet_range_draw (sheet, NULL);
    }
    else
    {
        g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CHANGED], 0, -1, column);
    }

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[NEW_COL_WIDTH], 0, column, width);
}

/**
 * gtk_sheet_set_row_height:
 * @sheet: a #GtkSheet.
 * @row: row number.
 * @height: row height(in pixels).
 *
 * Set row height.
 */
void
    gtk_sheet_set_row_height (GtkSheet * sheet,
                              gint row,
                              guint height)
{
    guint min_height;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (row < 0 || row > sheet->maxrow)
        return;

    gtk_sheet_row_size_request(sheet, row, &min_height);
    if (height < min_height) return;

    sheet->row[row].height = height;

    gtk_sheet_recalc_top_ypixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet))
    {
        size_allocate_row_title_buttons (sheet);
        adjust_scrollbars (sheet);
        gtk_sheet_size_allocate_entry(sheet);
        gtk_sheet_range_draw (sheet, NULL);
    }

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CHANGED], 0, row, -1);
    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[NEW_ROW_HEIGHT], 0, row, height);

}

/**
 * gtk_sheet_add_column:
 * @sheet: a #GtkSheet.
 * @ncols: number of columns to be appended.
 *
 * Append @ncols columns to the right of the sheet.
 */
void
    gtk_sheet_add_column(GtkSheet *sheet, guint ncols)
{

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    AddColumns(sheet, sheet->maxcol + 1, ncols);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    adjust_scrollbars(sheet);

    if (sheet->state==GTK_SHEET_ROW_SELECTED) sheet->range.coli+=ncols;

    sheet->old_hadjustment = -1.;

    if (!GTK_SHEET_IS_FROZEN(sheet) && sheet->hadjustment)
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment),
                               "value_changed");
}

/**
 * gtk_sheet_add_row:
 * @sheet: a #GtkSheet.
 * @nrows: number of rows to be appended.
 *
 * Append @nrows rows to the end of the sheet.
 */
void
    gtk_sheet_add_row(GtkSheet *sheet, guint nrows)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    AddRows(sheet, sheet->maxrow+1, nrows);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    if (sheet->state==GTK_SHEET_COLUMN_SELECTED) sheet->range.rowi+=nrows;

    adjust_scrollbars(sheet);

    sheet->old_vadjustment = -1.;
    if (!GTK_SHEET_IS_FROZEN(sheet) && sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }
}

/**
 * gtk_sheet_insert_rows:
 * @sheet: a #GtkSheet.
 * @row: row number.
 * @nrows: number of rows to be inserted.
 *
 * Insert @nrows rows before the given row and pull right.
 */
void
    gtk_sheet_insert_rows(GtkSheet *sheet, guint row, guint nrows)
{
    GList *children;
    GtkSheetChild *child;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
        gtk_sheet_real_unselect_range(sheet, NULL);

    InsertRow(sheet, row, nrows);

    children = sheet->children;
    while (children)
    {
        child = (GtkSheetChild *)children->data;

        if (child->attached_to_cell)
            if (child->row >= row) child->row += nrows;

        children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    if (sheet->state==GTK_SHEET_COLUMN_SELECTED) sheet->range.rowi+=nrows;
    adjust_scrollbars(sheet);

    sheet->old_vadjustment = -1.;
    if (!GTK_SHEET_IS_FROZEN(sheet) && sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }
}

/**
 * gtk_sheet_insert_columns:
 * @sheet: a #GtkSheet.
 * @col: column number.
 * @ncols: number of columns to be inserted.
 *
 * Insert @ncols columns before the given row and pull right.
 */
void
    gtk_sheet_insert_columns(GtkSheet *sheet, guint col, guint ncols)
{
    GList *children;
    GtkSheetChild *child;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
        gtk_sheet_real_unselect_range(sheet, NULL);

    InsertColumn(sheet, col, ncols);

    children = sheet->children;
    while (children)
    {
        child = (GtkSheetChild *)children->data;

        if (child->attached_to_cell)
            if (child->col >= col) child->col += ncols;

        children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    if (sheet->state==GTK_SHEET_ROW_SELECTED) sheet->range.coli+=ncols;
    adjust_scrollbars(sheet);

    sheet->old_hadjustment = -1.;

    if (!GTK_SHEET_IS_FROZEN(sheet) && sheet->hadjustment)
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
}

/**
 * gtk_sheet_delete_rows:
 * @sheet: a #GtkSheet.
 * @row: row number.
 * @nrows: number of rows to be deleted.
 *
 * Delete @nrows rows starting from @row.
 */
void
    gtk_sheet_delete_rows(GtkSheet *sheet, guint row, guint nrows)
{
    GList *children;
    GtkSheetChild *child;
    gint act_row, act_col;
    gboolean veto;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    nrows = MIN(nrows, sheet->maxrow-row+1);

    act_row = sheet->active_cell.row;
    act_col = sheet->active_cell.col;

    gtk_sheet_hide_active_cell(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
        gtk_sheet_real_unselect_range(sheet, NULL);

    DeleteRow(sheet, row, nrows);

    children = sheet->children;
    while (children)
    {
        child = (GtkSheetChild *) children->data;

        if (child->attached_to_cell &&
            child->row >= row && child->row < row+nrows)
        {
            gtk_container_remove(GTK_CONTAINER(sheet), child->widget);
            children = sheet->children;
        }
        else
            children = children->next;
    }

    children = sheet->children;
    while (children)
    {
        child = (GtkSheetChild *) children->data;

        if (child->attached_to_cell && child->row > row) child->row -= nrows;
        children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    sheet->active_cell.row = -1;
    sheet->active_cell.col = -1;

/* if(sheet->state == GTK_SHEET_ROW_SELECTED)
*/

    act_row = MIN(act_row, sheet->maxrow);
    act_row = MAX(act_row, 0);

    gtk_sheet_click_cell(sheet, act_row, act_col, &veto);
/*    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col); */

    adjust_scrollbars(sheet);

    sheet->old_vadjustment = -1.;
    if (!GTK_SHEET_IS_FROZEN(sheet) && sheet->vadjustment)
    {
        g_signal_emit_by_name (GTK_OBJECT (sheet->vadjustment), "value_changed");
    }
}

/**
 * gtk_sheet_delete_columns:
 * @sheet: a #GtkSheet.
 * @col: column number.
 * @ncols: number of columns to be deleted.
 *
 * Delete @ncols columns starting from @col.
 */
void
    gtk_sheet_delete_columns(GtkSheet *sheet, guint col, guint ncols)
{
    GList *children;
    GtkSheetChild *child;
    gint act_row, act_col;
    gboolean veto;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    ncols = MIN(ncols, sheet->maxcol-col+1);

    if (gtk_widget_get_realized(GTK_WIDGET (sheet)))
        gtk_sheet_real_unselect_range(sheet, NULL);

    DeleteColumn(sheet, col, ncols);

    children = sheet->children;
    while (children)
    {
        child = (GtkSheetChild *)children->data;

        if (child->attached_to_cell &&
            child->col >= col && child->col < col+ncols)
        {
            gtk_container_remove(GTK_CONTAINER(sheet), child->widget);
            children = sheet->children;
        }
        else
            children = children->next;
    }

    children = sheet->children;
    while (children)
    {
        child = (GtkSheetChild *)children->data;

        if (child->attached_to_cell && child->col > col) child->col -= ncols;
        children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    act_row = sheet->active_cell.row;
    act_col = sheet->active_cell.col;

    sheet->active_cell.row = -1;
    sheet->active_cell.col = -1;

/* if(sheet->state == GTK_SHEET_COLUMN_SELECTED)
*/

    act_col = MIN(act_col, sheet->maxcol);
    act_col = MAX(act_col, 0);

    gtk_sheet_click_cell(sheet, act_row, act_col, &veto);
    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);

    adjust_scrollbars(sheet);

    sheet->old_hadjustment = -1.;

    if (!GTK_SHEET_IS_FROZEN(sheet) && sheet->hadjustment)
        g_signal_emit_by_name (GTK_OBJECT (sheet->hadjustment), "value_changed");
}

/**
 * gtk_sheet_range_set_background:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @color: a #GdkColor.
 *
 * Set background color of the given range.
 */
void
    gtk_sheet_range_set_background(GtkSheet *sheet, const GtkSheetRange *urange, const GdkColor *color)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            if (color != NULL)
                attributes.background = *color;
            else
                attributes.background = sheet->bg_color;

            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }

    range.row0--;
    range.col0--;
    range.rowi++;
    range.coli++;

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
}

/**
 * gtk_sheet_range_set_foreground:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @color: a #GdkColor.
 *
 * Set foreground color of the given range.
 */
void
    gtk_sheet_range_set_foreground(GtkSheet *sheet, const GtkSheetRange *urange, const GdkColor *color)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);

            if (color != NULL)
                attributes.foreground = *color;
            else
                gdk_color_black(gdk_colormap_get_system(), &attributes.foreground);

            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);

}

/**
 * gtk_sheet_range_set_justification:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @just: a #GtkJustification : GTK_JUSTIFY_LEFT, RIGHT, CENTER.
 *
 * Set text justification (GTK_JUSTIFY_LEFT, RIGHT, CENTER) of the given range.
 * The default value is GTK_JUSTIFY_LEFT. If autoformat is on, the default justification for numbers is GTK_JUSTIFY_RIGHT.
 */
void
    gtk_sheet_range_set_justification(GtkSheet *sheet, const GtkSheetRange *urange,
                                      GtkJustification just)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            attributes.justification = just;
            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }
    }

    range.col0 = sheet->view.col0;
    range.coli = sheet->view.coli;

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
}

/**
 * gtk_sheet_column_set_justification:
 * @sheet: a #GtkSheet.
 * @col: column number
 * @just: a #GtkJustification : GTK_JUSTIFY_LEFT, RIGHT, CENTER
 *
 * Set column justification (GTK_JUSTIFY_LEFT, RIGHT, CENTER).
 * The default value is GTK_JUSTIFY_LEFT. If autoformat is on, the default justification for numbers is GTK_JUSTIFY_RIGHT.
 */
void
    gtk_sheet_column_set_justification(GtkSheet *sheet, gint col,
                                       GtkJustification justification)
{
    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col > sheet->maxcol) return;

    sheet->column[col]->justification = justification;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet) &&
        col >= MIN_VISIBLE_COLUMN(sheet) && col <= MAX_VISIBLE_COLUMN(sheet))
    {
        gtk_sheet_range_draw(sheet, NULL);
    }
}

/**
 * gtk_sheet_range_set_editable:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange
 * @editable: TRUE or FALSE
 *
 * Set if cell contents can be edited or not in the given range.
 */
void
    gtk_sheet_range_set_editable(GtkSheet *sheet, const GtkSheetRange *urange, gboolean editable)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            attributes.is_editable = editable;
            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }
    }

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
}

/**
 * gtk_sheet_range_set_visible:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @visible: TRUE or FALSE.
 *
 * Set if cell contents are visible or not in the given range: accepted values are TRUE or FALSE.
 */
void
    gtk_sheet_range_set_visible(GtkSheet *sheet, const GtkSheetRange *urange, gboolean visible)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            attributes.is_visible=visible;
            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }
    }

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
}

/**
 * gtk_sheet_range_set_border:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange where we set border style.
 * @mask: CELL_LEFT_BORDER, CELL_RIGHT_BORDER, CELL_TOP_BORDER,CELL_BOTTOM_BORDER
 * @width: width of the border line in pixels
 * @line_style: GdkLineStyle for the border line
 *
 * Set cell border style in the given range.
 */
void
    gtk_sheet_range_set_border(GtkSheet *sheet, const GtkSheetRange *urange, gint mask,
                               guint width, gint line_style)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            attributes.border.mask = mask;
            attributes.border.width = width;
            attributes.border.line_style=line_style;
            attributes.border.cap_style=GDK_CAP_NOT_LAST;
            attributes.border.join_style=GDK_JOIN_MITER;
            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }
    }

    range.row0--;
    range.col0--;
    range.rowi++;
    range.coli++;

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
}

/**
 * gtk_sheet_range_set_border_color:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange where we set border color.
 * @color: a #GdkColor.
 *
 * Set border color for the given range.
 */
void
    gtk_sheet_range_set_border_color(GtkSheet *sheet, 
                                     const GtkSheetRange *urange, 
                                     const GdkColor *color)
{
    gint i, j;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            attributes.border.color = *color;
            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }
    }

    if (!GTK_SHEET_IS_FROZEN(sheet)) gtk_sheet_range_draw(sheet, &range);
}

/**
 * gtk_sheet_range_set_font:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange where we set font.
 * @font: a #PangoFontDescription.
 *
 * Set font for the given range.
 */
void
    gtk_sheet_range_set_font(GtkSheet *sheet, 
                             const GtkSheetRange *urange, 
                             PangoFontDescription *font)
{
    gint i, j;
    gint font_height;
    GtkSheetCellAttr attributes;
    GtkSheetRange range;
    PangoContext *context;
    PangoFontMetrics *metrics;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (!urange)
        range = sheet->range;
    else
        range = *urange;

    gtk_sheet_freeze(sheet);

    context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
    metrics = pango_context_get_metrics(context,
                                        font,
                                        pango_context_get_language(context));
    font_height = pango_font_metrics_get_descent(metrics) +
        pango_font_metrics_get_ascent(metrics);
    font_height = PANGO_PIXELS(font_height) + 2*CELLOFFSET;

    for (i=range.row0; i<=range.rowi; i++)
    {
        for (j=range.col0; j<=range.coli; j++)
        {
            gtk_sheet_get_attributes(sheet, i, j, &attributes);
            attributes.font_desc = font;
            if (font_height > sheet->row[i].height)
            {
                sheet->row[i].height = font_height;
                gtk_sheet_recalc_top_ypixels(sheet);
            }

            gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
        }
    }

    gtk_sheet_thaw(sheet);
    pango_font_metrics_unref(metrics);
}

static void
    gtk_sheet_set_cell_attributes(GtkSheet *sheet, 
                                  gint row, gint col, 
                                  GtkSheetCellAttr attributes)
{
    GtkSheetCell *cell;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;

    CheckCellData(sheet, row, col);

    cell = sheet->data[row][col];

    if (!cell->attributes) cell->attributes = g_new(GtkSheetCellAttr, 1);

    *(cell->attributes) = attributes;
}

/**
 * gtk_sheet_get_attributes:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @col: column number
 * @attributes: #GtkSheetCellAttr of the given range
 *
 * Gett cell attributes of the given cell.
 *
 * Returns: TRUE means that the cell is currently allocated.
 */
gboolean
    gtk_sheet_get_attributes(GtkSheet *sheet, 
                             gint row, gint col, 
                             GtkSheetCellAttr *attributes)
{
    GtkSheetCell *cell;

    g_return_val_if_fail (sheet != NULL, FALSE);
    g_return_val_if_fail (GTK_IS_SHEET (sheet), FALSE);

    if ((row < 0 || row > sheet->maxrow) || (col < 0 || col > sheet->maxcol))
    {
        init_attributes(sheet, col, attributes);
        return (FALSE);
    }

    if (row > sheet->maxallocrow || col > sheet->maxalloccol || 
        !sheet->data[row] || !sheet->data[row][col])
    {
        init_attributes(sheet, col, attributes);
        return (FALSE);
    }

    cell = sheet->data[row][col];

    if (!cell->attributes)
    {
        init_attributes(sheet, col, attributes);
        return(FALSE);
    }

    *attributes = *(cell->attributes);

    if (COLPTR(sheet, col)->justification != GTK_SHEET_DEFAULT_COLUMN_JUSTIFICATION)
        attributes->justification = COLPTR(sheet, col)->justification;

    return(TRUE);
}

static void
    init_attributes(GtkSheet *sheet, gint col, GtkSheetCellAttr *attributes)
{
    /* DEFAULT VALUES */
    attributes->foreground = gtk_widget_get_style(GTK_WIDGET(sheet))->black;
    attributes->background = sheet->bg_color;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        GdkColormap *colormap;
        colormap = gdk_colormap_get_system();
        gdk_color_black(colormap, &attributes->foreground);
        attributes->background = sheet->bg_color;
    }

    if (col < 0 || col > sheet->maxcol)
        attributes->justification = GTK_SHEET_DEFAULT_COLUMN_JUSTIFICATION;
    else
        attributes->justification = sheet->column[col]->justification;

    attributes->border.width = 0;
    attributes->border.line_style = GDK_LINE_SOLID;
    attributes->border.cap_style = GDK_CAP_NOT_LAST;
    attributes->border.join_style = GDK_JOIN_MITER;
    attributes->border.mask = 0;
    attributes->border.color = gtk_widget_get_style(GTK_WIDGET(sheet))->black;
    attributes->is_editable = TRUE;
    attributes->is_visible = TRUE;
    attributes->font = gtk_widget_get_style(GTK_WIDGET(sheet))->private_font;
    attributes->font_desc = gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc;
}

/**********************************************************************
 * Memory allocation routines:
 * AddRow & AddColumn allocate memory for GtkSheetColumn & GtkSheetRow structs.
 * InsertRow
 * InsertColumn
 * DeleteRow
 * DeleteColumn
 * GrowSheet allocates memory for the sheet cells contents using an array of
 * pointers. Alternative to this could be a linked list or a hash table.
 * CheckBounds checks whether the given cell is currently allocated or not.
 * If not, it calls to GrowSheet.
 **********************************************************************/

static void
    AddColumns(GtkSheet *sheet, gint position, gint ncols)
{
    gint i;
    GtkSheetColumn *newobj;

    g_assert(ncols >= 0);
    g_assert(position >= 0 && position <= sheet->maxcol + 1);

    if (ncols > 0)
    {
        sheet->column = (GtkSheetColumn **) g_realloc(sheet->column,
                                                      (sheet->maxcol+1+ncols) * sizeof(GtkSheetColumn *));

        for (i=sheet->maxcol; i>=position; i--)  /* make space */
        {
            sheet->column[i+ncols] = sheet->column[i];
            sheet->column[i] = NULL;
        }

        for (i=0;i<ncols;i++)
        {
            gint newidx = position + i;

            newobj = g_object_new(G_TYPE_SHEET_COLUMN, NULL);

#ifdef GTK_SHEET_DEBUG
            g_object_weak_ref(G_OBJECT(newobj), weak_notify, "Sheet-Column");
#endif

            newobj->sheet = sheet;
            sheet->column[newidx] = newobj;
            g_object_ref_sink(newobj);
        }

        sheet->maxcol += ncols;

        gtk_sheet_recalc_text_column(sheet, sheet->maxcol - ncols);
        gtk_sheet_recalc_left_xpixels(sheet);
    }
}

static void
    InsertColumn(GtkSheet *sheet, gint position, gint ncols)
{
    gint i,j;

    g_assert(ncols >= 0);
    g_assert(position >= 0);

    AddColumns(sheet, position, ncols);

    gtk_sheet_recalc_text_column(sheet, sheet->maxcol - ncols);
    gtk_sheet_recalc_left_xpixels(sheet);

    if (position <= sheet->maxalloccol)  /* adjust allocated cells */
    {
        GrowSheet(sheet, 0, ncols);

        for (i=0; i<=sheet->maxallocrow; i++)
        {
            for (j=sheet->maxalloccol; j>=position+ncols; j--)
            {
                gtk_sheet_real_cell_clear(sheet, i, j, TRUE);

                sheet->data[i][j] = sheet->data[i][j-ncols];
                if (sheet->data[i][j]) sheet->data[i][j]->col = j;
                sheet->data[i][j-ncols] = NULL;
            }
        }
    }
}

static void
    DeleteColumn(GtkSheet *sheet, gint position, gint ncols)
{
    gint i,j;

    g_assert(ncols >= 0);
    g_assert(position >= 0);

    ncols = MIN(ncols, sheet->maxcol-position+1);

    if (ncols <= 0 || position > sheet->maxcol) return;

    for (i=position; i<position+ncols; i++)  /* dispose columns */
    {
        sheet->column[i]->sheet = NULL;

        g_object_unref(sheet->column[i]);
        sheet->column[i] = NULL;
    }

    for (i=position; i<=sheet->maxcol-ncols; i++)  /* shift columns into position*/
    {
        sheet->column[i] = sheet->column[i+ncols];
    }

    for (i=sheet->maxcol-ncols+1; i<=sheet->maxcol; i++)  /* clear tail */
    {
        sheet->column[i] = NULL;
    }

    /* to be done: shrink pointer pool via realloc */

    if (position <= sheet->maxalloccol)  /* shift data into position */
    {
        for (i=position; i<=sheet->maxcol-ncols; i++)
        {
            if (i<=sheet->maxalloccol)
            {
                for (j=0; j<=sheet->maxallocrow; j++)
                {
                    gtk_sheet_real_cell_clear(sheet, j, i, TRUE);

                    if (i+ncols <= sheet->maxalloccol)
                    {
                        sheet->data[j][i] = sheet->data[j][i+ncols];
                        sheet->data[j][i+ncols] = NULL;
                        if (sheet->data[j][i]) sheet->data[j][i]->col=i;
                    }
                }
            }

        }
        sheet->maxalloccol-=MIN(ncols,sheet->maxalloccol-position+1);
        sheet->maxalloccol = MIN(sheet->maxalloccol, sheet->maxcol);
    }

    sheet->maxcol -= ncols;

    fixup_range(sheet, &sheet->view);
    fixup_range(sheet, &sheet->range);

    gtk_sheet_recalc_text_column(sheet, position);
    gtk_sheet_recalc_left_xpixels(sheet);
}

static void
    AddRows(GtkSheet *sheet, gint position, gint nrows)
{
    gint i;

    g_assert(nrows >= 0);
    g_assert(position >= 0 && position <= sheet->maxrow + 1);

    if (nrows > 0)
    {
        sheet->row = (GtkSheetRow *) g_realloc(sheet->row,
                                               (sheet->maxrow+1+nrows) * sizeof(GtkSheetRow));

        for (i=sheet->maxrow; i>=position; i--)  /* make space */
        {
            sheet->row[i+nrows] = sheet->row[i];
#if 0
            sheet->row[i] = NULL;
#else
            gtk_sheet_row_init(&sheet->row[i]);
#endif
        }

        for (i=0; i<nrows; i++)
        {
            gint newidx = position + i;

            gtk_sheet_row_init(&sheet->row[newidx]);

            sheet->row[newidx].requisition = sheet->row[newidx].height =
                DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet));
        }
        sheet->maxrow += nrows;

        gtk_sheet_recalc_top_ypixels(sheet);
    }
}

static void
    InsertRow(GtkSheet *tbl, gint row, gint nrows)
{
    GtkSheetCell **pp;
    gint i,j;
    GtkSheetCell **auxdata;

    AddRows(tbl, row, nrows);

    gtk_sheet_recalc_top_ypixels(tbl);

    if (row <= tbl->maxallocrow)  /* adjust allocated cells */
    {
        GrowSheet(tbl,nrows,0);

        for (i=tbl->maxallocrow; i>=row+nrows; i--)
        {
            auxdata = tbl->data[i];
            tbl->data[i]=tbl->data[i-nrows];

            pp= tbl->data[i];
            for (j=0; j<=tbl->maxalloccol; j++,pp++)
            {
                if (*pp!=(GtkSheetCell *)NULL)
                    (*pp)->row=i;

            }
            tbl->data[i-nrows]=auxdata;
        }
    }
}

static void
    DeleteRow(GtkSheet *sheet, gint position, gint nrows)
{
    GtkSheetCell **auxdata = NULL;
    gint i,j;

    g_assert(nrows >= 0);
    g_assert(position >= 0);

    nrows=MIN(nrows, sheet->maxrow-position+1);

    if (nrows <= 0 || position > sheet->maxrow) return;

    for (i=position; i<position+nrows; i++)
    {
        gtk_sheet_row_finalize(&sheet->row[i]);
    }

    for (i=position; i<=sheet->maxrow-nrows; i++)
    {
        if (i+nrows <= sheet->maxrow)
        {
            sheet->row[i]=sheet->row[i+nrows];
        }
    }

    if (position <= sheet->maxallocrow)
    {
        for (i=position; i<=sheet->maxrow-nrows; i++)
        {
            if (i<=sheet->maxallocrow)
            {
                auxdata=sheet->data[i];
                for (j=0; j<=sheet->maxalloccol; j++)
                {
                    gtk_sheet_real_cell_clear(sheet, i, j, TRUE);
                }
            }
            if (i+nrows<=sheet->maxallocrow)
            {
                sheet->data[i] = sheet->data[i+nrows];
                sheet->data[i+nrows]=auxdata;

                for (j=0; j<=sheet->maxalloccol; j++)
                {
                    if (sheet->data[i][j]) sheet->data[i][j]->row=i;
                }
            }
        }

        for (i=sheet->maxrow-nrows+1; i<=sheet->maxallocrow; i++)
        {
            if (i > 0 && sheet->data[i])
            {
                g_free(sheet->data[i]);
                sheet->data[i] = NULL;
            }
        }
        sheet->maxallocrow-=MIN(nrows, sheet->maxallocrow-position+1);
    }

    sheet->maxrow-=nrows;
    sheet->maxallocrow = MIN(sheet->maxallocrow, sheet->maxrow);

    fixup_range(sheet, &sheet->view);
    fixup_range(sheet, &sheet->range);

    gtk_sheet_recalc_top_ypixels(sheet);
}

static gint
    GrowSheet(GtkSheet *tbl, gint newrows, gint newcols)
{
    gint i,j;
    gint inirow, inicol;

    inirow = tbl->maxallocrow + 1;
    inicol = tbl->maxalloccol + 1;

    tbl->maxalloccol = tbl->maxalloccol + newcols;
    tbl->maxallocrow = tbl->maxallocrow + newrows;

    if (newrows > 0)
    {
        tbl->data = (GtkSheetCell***)
        g_realloc(tbl->data,(tbl->maxallocrow+1)*sizeof(GtkSheetCell **)+sizeof(double));

        for (i=inirow; i <= tbl->maxallocrow; i++)
        {
            tbl->data[i] = (GtkSheetCell **) \
                g_malloc((tbl->maxcol+1)*sizeof(GtkSheetCell *)+sizeof(double));

            for (j=0; j<inicol; j++)
            {
                tbl->data[i][j] = NULL;
            }
        }

    }

    if (newcols > 0)
    {
        for (i=0; i <= tbl->maxallocrow; i++)
        {
            tbl->data[i] = (GtkSheetCell **) \
                g_realloc(tbl->data[i],(tbl->maxalloccol+1)*sizeof(GtkSheetCell *)+sizeof(double));
            for (j=inicol; j <= tbl->maxalloccol; j++)
            {
                tbl->data[i][j] = NULL;
            }
        }
    }

    return(0);
}

static void
    CheckBounds(GtkSheet *sheet, gint row, gint col)
{
    gint newrows=0, newcols=0;

    if (col > sheet->maxalloccol) newcols = col - sheet->maxalloccol;
    if (row > sheet->maxallocrow) newrows = row - sheet->maxallocrow;

    if (newrows > 0 || newcols > 0) GrowSheet(sheet, newrows, newcols);
}


/*
 * CheckCellData - verify existance of cell data, allocate if necessary
 * 
 * @param sheet
 * @param row
 * @param col
 */
static void
    CheckCellData(GtkSheet *sheet, const gint row, const gint col)
{
    GtkSheetCell **cell;

    g_return_if_fail (sheet != NULL);
    g_return_if_fail (GTK_IS_SHEET (sheet));

    if (col > sheet->maxcol || row > sheet->maxrow) return;
    if (col < 0 || row < 0) return;

    CheckBounds(sheet, row, col);

    cell = &sheet->data[row][col];

    if (!(*cell)) (*cell) = gtk_sheet_cell_new();

    (*cell)->row = row;
    (*cell)->col = col;
}

/********************************************************************
 * Container Functions:
 * gtk_sheet_add
 * gtk_sheet_put
 * gtk_sheet_attach
 * gtk_sheet_remove
 * gtk_sheet_move_child
 * gtk_sheet_position_child
 * gtk_sheet_position_children
 * gtk_sheet_realize_child
 * gtk_sheet_get_child_at
 ********************************************************************/

/**
 * gtk_sheet_put:
 * @sheet: a #GtkSheet.
 * @child: GtkWidget to be put
 * @x: x coordinate where we put the widget
 * @y: y coordinate where we put the widget
 *
 * Add widgets to the sheet.
 * The widget is floating in one given position (x,y) regardless of the configurations of rows/columns.
 * This means that cells do not resize depending on the widgets' size.
 * You can resize it yourself or use gtk_sheet_attach_*()
 * You may remove it with gtk_container_remove(GTK_CONTAINER(sheet), GtkWidget *child);
 *
 * Returns: TRUE means that the cell is currently allocated.
 */
GtkSheetChild *
    gtk_sheet_put(GtkSheet *sheet, GtkWidget *child, gint x, gint y)
{
    GtkRequisition child_requisition;
    GtkSheetChild *child_info;

    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);
    g_return_val_if_fail(child != NULL, NULL);
    g_return_val_if_fail(child->parent == NULL, NULL);

    child_info = g_new (GtkSheetChild, 1);
    child_info->widget = child;
    child_info->x = x;
    child_info->y = y;
    child_info->attached_to_cell = FALSE;
    child_info->floating = TRUE;
    child_info->xpadding = child_info->ypadding = 0;
    child_info->xexpand = child_info->yexpand = FALSE;
    child_info->xshrink = child_info->yshrink = FALSE;
    child_info->xfill = child_info->yfill = FALSE;

    sheet->children = g_list_append(sheet->children, child_info);

    gtk_widget_set_parent (child, GTK_WIDGET(sheet));

    gtk_widget_size_request(child, &child_requisition);

    if (gtk_widget_get_visible(GTK_WIDGET(sheet)))
    {
        if (gtk_widget_get_realized(GTK_WIDGET(sheet)) &&
            (!gtk_widget_get_realized(child) || gtk_widget_get_has_window(child)))
            gtk_sheet_realize_child(sheet, child_info);

        if (gtk_widget_get_mapped(GTK_WIDGET(sheet)) &&
            !gtk_widget_get_mapped(child))
            gtk_widget_map(child);
    }

    gtk_sheet_position_child(sheet, child_info);

/* This will avoid drawing on the titles */

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        if (sheet->row_titles_visible)
            gdk_window_show(sheet->row_title_window);
        if (sheet->column_titles_visible)
            gdk_window_show(sheet->column_title_window);
    }

    return(child_info);
}

/**
 * gtk_sheet_attach_floating:
 * @sheet: a #GtkSheet.
 * @widget: #GtkWidget to be put
 * @row: row number
 * @col: column number
 *
 * The widget is attached to the top-left corner of a cell (row,column) and moves with it when you change width,
 * height, or you delete of add row/columns
 */
void
    gtk_sheet_attach_floating       (GtkSheet *sheet,
                                     GtkWidget *widget,
                                     gint row, gint col)
{
    GdkRectangle area;
    GtkSheetChild *child;

    if (row < 0 || col < 0)
    {
        gtk_sheet_button_attach(sheet, widget, row, col);
        return;
    }

    gtk_sheet_get_cell_area(sheet, row, col, &area);
    child = gtk_sheet_put(sheet, widget, area.x, area.y);
    child->attached_to_cell = TRUE;
    child->row = row;
    child->col = col;
}

/**
 * gtk_sheet_attach_default:
 * @sheet: a #GtkSheet.
 * @widget: #GtkWidget to be put
 * @row: row number
 * @col: column number
 *
 * Attaches a child widget to the given cell with the 0,0 alignments.
 * Works basically like gtk_table_attach, with the same options, the widget is confined in the cell, and whether it fills the
 * cell, expands with it, or shrinks with it, depending on the options.
 * The child is reallocated each time the column or row changes, keeping attached to the same cell.
 * It's in fact gtk_sheet_attach() with GTK_EXPAND set.
 */
void
    gtk_sheet_attach_default        (GtkSheet *sheet,
                                     GtkWidget *widget,
                                     gint row, gint col)
{
    if (row < 0 || col < 0)
    {
        gtk_sheet_button_attach(sheet, widget, row, col);
        return;
    }

    gtk_sheet_attach(sheet, widget, row, col, GTK_EXPAND|GTK_FILL, GTK_EXPAND|GTK_FILL, 0, 0);
}

/**
 * gtk_sheet_attach:
 * @sheet: a #GtkSheet.
 * @widget: #GtkWidget to be put
 * @row: row number
 * @col: column number
 * @xoptions: if set GTK_EXPAND cell will expand/shrink on x direction
 * @yoptions: if set GTK_EXPAND cell will expand/shrink on y direction
 * @xpadding: x coordinate of the alignment
 * @ypadding: y coordinate of the alignment
 *
 * Attaches a child widget to the given cell with the given alignments.
 * Works basically like gtk_table_attach, with the same options, the widget is confined in the cell, and whether it fills the
 * cell, expands with it, or shrinks with it, depending on the options , if GTK_EXPAND is set.
 * The child is reallocated each time the column or row changes, keeping attached to the same cell.
 */
void
    gtk_sheet_attach        (GtkSheet *sheet,
                             GtkWidget *widget,
                             gint row, gint col,
                             gint xoptions,
                             gint yoptions,
                             gint xpadding,
                             gint ypadding)
{
    GdkRectangle area;
    GtkSheetChild *child = NULL;

    if (row < 0 || col < 0)
    {
        gtk_sheet_button_attach(sheet, widget, row, col);
        return;
    }

    child = g_new0(GtkSheetChild, 1);
    child->attached_to_cell = TRUE;
    child->floating = FALSE;
    child->widget = widget;
    child->row = row;
    child->col = col;
    child->xpadding = xpadding;
    child->ypadding = ypadding;
    child->xexpand = (xoptions & GTK_EXPAND) != 0;
    child->yexpand = (yoptions & GTK_EXPAND) != 0;
    child->xshrink = (xoptions & GTK_SHRINK) != 0;
    child->yshrink = (yoptions & GTK_SHRINK) != 0;
    child->xfill = (xoptions & GTK_FILL) != 0;
    child->yfill = (yoptions & GTK_FILL) != 0;

    sheet->children = g_list_append(sheet->children, child);

    gtk_sheet_get_cell_area(sheet, row, col, &area);

    child->x = area.x + child->xpadding;
    child->y = area.y + child->ypadding;

    if (gtk_widget_get_visible(GTK_WIDGET(sheet)))
    {
        if (gtk_widget_get_realized(GTK_WIDGET(sheet)) &&
            (!gtk_widget_get_realized(widget) || gtk_widget_get_has_window(widget)))
            gtk_sheet_realize_child(sheet, child);

        if (gtk_widget_get_mapped(GTK_WIDGET(sheet)) &&
            !gtk_widget_get_mapped(widget))
            gtk_widget_map(widget);
    }

    gtk_sheet_position_child(sheet, child);

/* This will avoid drawing on the titles */

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
        if (GTK_SHEET_ROW_TITLES_VISIBLE(sheet))
            gdk_window_show(sheet->row_title_window);
        if (GTK_SHEET_COL_TITLES_VISIBLE(sheet))
            gdk_window_show(sheet->column_title_window);
    }
}

/**
 * gtk_sheet_button_attach:
 * @sheet: a #GtkSheet.
 * @widget: #GtkWidget to be put
 * @row: row number
 * @col: column number
 *
 * Button attach works like cell attach but for the buttons.
 */
void
    gtk_sheet_button_attach     (GtkSheet *sheet,
                                 GtkWidget *widget,
                                 gint row, gint col)
{
    GtkSheetButton *button;
    GtkSheetChild *child;
    GtkRequisition button_requisition;

    if (row >= 0 && col >= 0) return;
    if (row < 0 && col < 0) return;

    child = g_new (GtkSheetChild, 1);
    child->widget = widget;
    child->x = 0;
    child->y = 0;
    child->attached_to_cell = TRUE;
    child->floating = FALSE;
    child->row = row;
    child->col = col;
    child->xpadding = child->ypadding = 0;
    child->xshrink = child->yshrink = FALSE;
    child->xfill = child->yfill = FALSE;

    if (row == -1)
    {
        button = &sheet->column[col]->button;
        button->child = child;
    }
    else
    {
        button = &sheet->row[row].button;
        button->child = child;
    }

    sheet->children = g_list_append(sheet->children, child);

    gtk_sheet_button_size_request(sheet, button, &button_requisition);

    if (row == -1)
    {
        if (button_requisition.height > sheet->column_title_area.height)
            sheet->column_title_area.height = button_requisition.height;
        if (button_requisition.width > sheet->column[col]->width)
            sheet->column[col]->width = button_requisition.width;
    }

    if (col == -1)
    {
        if (button_requisition.width > sheet->row_title_area.width)
            sheet->row_title_area.width = button_requisition.width;
        if (button_requisition.height > sheet->row[row].height)
            sheet->row[row].height = button_requisition.height;
    }

    if (gtk_widget_get_visible(GTK_WIDGET(sheet)))
    {
        if (gtk_widget_get_realized(GTK_WIDGET(sheet)) &&
            (!gtk_widget_get_realized(widget) || gtk_widget_get_has_window(widget)))
            gtk_sheet_realize_child(sheet, child);

        if (gtk_widget_get_mapped(GTK_WIDGET(sheet)) &&
            !gtk_widget_get_mapped(widget))
            gtk_widget_map(widget);
    }

    if (row == -1) size_allocate_column_title_buttons(sheet);
    if (col == -1) size_allocate_row_title_buttons(sheet);
}

static void
    label_size_request(GtkSheet *sheet, gchar *label, GtkRequisition *req)
{
    gchar *words;
    gchar word[1000];
    gint n = 0;
    gint row_height = DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet)) - 2*CELLOFFSET + 2;

    req->height = 0;
    req->width = 0;
    words=label;

    while (words && *words != '\0')
    {
        if (*words == '\n' || *(words+1) == '\0')
        {
            req->height += row_height;

            word[n] = '\0';
            req->width = MAX(req->width, STRING_WIDTH(GTK_WIDGET(sheet), 
                                                      gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc, 
                                                      word));
            n = 0;
        }
        else
        {
            word[n++] = *words;
        }
        words++;
    }

    if (n > 0) req->height -= 2;
}

static void
    gtk_sheet_button_size_request   (GtkSheet *sheet,
                                     GtkSheetButton *button,
                                     GtkRequisition *button_requisition)
{
    GtkRequisition requisition;
    GtkRequisition label_requisition;

    if (gtk_sheet_autoresize(sheet) && button->label && button->label[0])
    {
        label_size_request(sheet, button->label, &label_requisition);
        label_requisition.width += 2*CELLOFFSET;
        label_requisition.height += 2*CELLOFFSET;
    }
    else
    {
        label_requisition.height = DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet));
        label_requisition.width = COLUMN_MIN_WIDTH;
    }

    if (button->child)
    {
        gtk_widget_size_request(button->child->widget, &requisition);
        requisition.width += 2*button->child->xpadding;
        requisition.height += 2*button->child->ypadding;
        requisition.width += 2*gtk_widget_get_style(sheet->button)->xthickness;
        requisition.height += 2*gtk_widget_get_style(sheet->button)->ythickness;
    }
    else
    {
        requisition.height = DEFAULT_ROW_HEIGHT(GTK_WIDGET(sheet));
        requisition.width = COLUMN_MIN_WIDTH;
    }

    *button_requisition = requisition;
    button_requisition->width = MAX(requisition.width, label_requisition.width);
    button_requisition->height = MAX(requisition.height, label_requisition.height);

}

static void
    gtk_sheet_row_size_request      (GtkSheet *sheet,
                                     gint row,
                                     guint *requisition)
{
    GtkRequisition button_requisition;
    GList *children;

    gtk_sheet_button_size_request(sheet, &sheet->row[row].button, &button_requisition);

    *requisition = button_requisition.height;

    children = sheet->children;
    while (children)
    {
        GtkSheetChild *child = (GtkSheetChild *)children->data;
        GtkRequisition child_requisition;

        if (child->attached_to_cell && child->row == row && child->col != -1 && !child->floating && !child->yshrink)
        {
            gtk_widget_get_child_requisition(child->widget, &child_requisition);

            if (child_requisition.height + 2 * child->ypadding > *requisition)
                *requisition = child_requisition.height + 2 * child->ypadding;
        }
        children = children->next;
    }

    sheet->row[row].requisition = *requisition;
}

static void
    gtk_sheet_column_size_request (GtkSheet *sheet,
                                   gint col,
                                   guint *requisition)
{
    GtkRequisition button_requisition;
    GList *children;

    gtk_sheet_button_size_request(sheet, &sheet->column[col]->button, &button_requisition);

    *requisition = button_requisition.width;

    children = sheet->children;
    while (children)
    {
        GtkSheetChild *child = (GtkSheetChild *)children->data;
        GtkRequisition child_requisition;

        if (child->attached_to_cell && child->col == col && 
            child->row != -1 && !child->floating && !child->xshrink)
        {
            gtk_widget_get_child_requisition(child->widget, &child_requisition);

            if (child_requisition.width + 2 * child->xpadding > *requisition)
                *requisition = child_requisition.width + 2 * child->xpadding;
        }
        children = children->next;
    }

    sheet->column[col]->requisition = *requisition;

#ifdef GTK_SHEET_DEBUG
    g_debug("gtk_sheet_column_size_request: col %d = %d", col, *requisition);
#endif
}

/**
 * gtk_sheet_move_child:
 * @sheet: a #GtkSheet.
 * @widget: #GtkWidget to be put.
 * @x: x coord at which we move the widget.
 * @y: y coord at which we move the widget.
 *
 * Move widgets added with gtk_sheet_put() in the sheet.
 */
void
    gtk_sheet_move_child(GtkSheet *sheet, GtkWidget *widget, gint x, gint y)
{
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    children = sheet->children;
    while (children)
    {
        child = children->data;

        if (child->widget == widget)
        {
            child->x = x;
            child->y = y;
            child->row = ROW_FROM_YPIXEL(sheet, y);
            child->col = COLUMN_FROM_XPIXEL(sheet, x);
            gtk_sheet_position_child(sheet, child);
            return;
        }

        children = children->next;
    }

    g_warning("Widget must be a GtkSheet child");
}

static void
    gtk_sheet_position_child(GtkSheet *sheet, GtkSheetChild *child)
{
    GtkRequisition child_requisition;
    GtkAllocation child_allocation;
    gint xoffset = 0;
    gint yoffset = 0;
    gint x = 0, y = 0;
    GdkRectangle area;

    gtk_widget_get_child_requisition(child->widget, &child_requisition);

    if (sheet->column_titles_visible)
        yoffset = sheet->column_title_area.height;

    if (sheet->row_titles_visible)
        xoffset = sheet->row_title_area.width;

    if (child->attached_to_cell)
    {
/*
      child->x = COLUMN_LEFT_XPIXEL(sheet, child->col);
      child->y = ROW_TOP_YPIXEL(sheet, child->row);

      if(sheet->row_titles_visible)
                                    child->x-=sheet->row_title_area.width;
      if(sheet->column_titles_visible)
                                    child->y-=sheet->column_title_area.height;

      width = sheet->column[child->col]->width;
      height = sheet->row[child->row].height;
*/

        gtk_sheet_get_cell_area(sheet, child->row, child->col, &area);
        child->x = area.x + child->xpadding;
        child->y = area.y + child->ypadding;

        if (!child->floating)
        {
            if (child_requisition.width + 2*child->xpadding <= sheet->column[child->col]->width)
            {
                if (child->xfill)
                {
                    child_requisition.width = child_allocation.width = sheet->column[child->col]->width - 2*child->xpadding;
                }
                else
                {
                    if (child->xexpand)
                    {
                        child->x = area.x + sheet->column[child->col]->width / 2 -
                            child_requisition.width / 2;
                    }
                    child_allocation.width = child_requisition.width;
                }
            }
            else
            {
                if (!child->xshrink)
                {
                    gtk_sheet_set_column_width(sheet, child->col, child_requisition.width + 2 * child->xpadding);
                }
                child_allocation.width = sheet->column[child->col]->width - 2*child->xpadding;
            }

            if (child_requisition.height + 2*child->ypadding <= sheet->row[child->row].height)
            {
                if (child->yfill)
                {
                    child_requisition.height = child_allocation.height = sheet->row[child->row].height - 2*child->ypadding;
                }
                else
                {
                    if (child->yexpand)
                    {
                        child->y = area.y + sheet->row[child->row].height / 2 -
                            child_requisition.height / 2;
                    }
                    child_allocation.height = child_requisition.height;
                }
            }
            else
            {
                if (!child->yshrink)
                {
                    gtk_sheet_set_row_height(sheet, child->row, child_requisition.height + 2 * child->ypadding);
                }
                child_allocation.height = sheet->row[child->row].height - 2*child->ypadding;
            }
        }
        else
        {
            child_allocation.width = child_requisition.width;
            child_allocation.height = child_requisition.height;
        }

        x = child_allocation.x = child->x + xoffset;
        y = child_allocation.y = child->y + yoffset;
    }
    else
    {
        x = child_allocation.x = child->x + sheet->hoffset + xoffset;
        x = child_allocation.x = child->x + xoffset;
        y = child_allocation.y = child->y + sheet->voffset + yoffset;
        y = child_allocation.y = child->y + yoffset;
        child_allocation.width = child_requisition.width;
        child_allocation.height = child_requisition.height;
    }

    gtk_widget_size_allocate(child->widget, &child_allocation);
    gtk_widget_queue_draw(child->widget);
}

/*
 * gtk_sheet_forall_handler:
 * 
 * this is the #GtkSheet container child enumeration handler
 * 
 * @param container the #GtkSheet
 * @param include_internals
 *                  Flag wether to include internal childs
 * @param callback  a callback function
 * @param callback_data
 *                  callback user data
 */
static void
    gtk_sheet_forall_handler (GtkContainer *container,
                      gboolean      include_internals,
                      GtkCallback   callback,
                      gpointer      callback_data)
{
    GtkSheet *sheet;
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail (GTK_IS_SHEET (container));
    g_return_if_fail (callback != NULL);

    sheet = GTK_SHEET (container);
    children = sheet->children;
    while (children)
    {
        child = children->data;
        children = children->next;

        (* callback) (child->widget, callback_data);
    }
    if (sheet->button)
        (* callback) (sheet->button, callback_data);
    if (sheet->sheet_entry)
        (* callback) (sheet->sheet_entry, callback_data);
}


static void
    gtk_sheet_position_children(GtkSheet *sheet)
{
    GList *children;
    GtkSheetChild *child;

    children = sheet->children;

    while (children)
    {
        child = (GtkSheetChild *)children->data;

        if (child->col !=-1 && child->row != -1)
            gtk_sheet_position_child(sheet, child);

        if (child->row == -1)
        {
            if (child->col < MIN_VISIBLE_COLUMN(sheet) ||
                child->col > MAX_VISIBLE_COLUMN(sheet))
                gtk_sheet_child_hide(child);
            else
                gtk_sheet_child_show(child);
        }
        if (child->col == -1)
        {
            if (child->row < MIN_VISIBLE_ROW(sheet) ||
                child->row > MAX_VISIBLE_ROW(sheet))
                gtk_sheet_child_hide(child);
            else
                gtk_sheet_child_show(child);
        }

        children = children->next;
    }

}

/*
 * gtk_sheet_remove_handler:
 * 
 * this is the #GtkSheet container class "remove" handler
 * 
 * @param container the #GtkSheet
 * @param widget    the #GtkWidget to be removed
 */
static void
    gtk_sheet_remove_handler (GtkContainer *container, GtkWidget *widget)
{
    GtkSheet *sheet;
    GList *children;
    GtkSheetChild *child = 0;

    g_return_if_fail(container != NULL);
    g_return_if_fail(GTK_IS_SHEET(container));

    sheet = GTK_SHEET(container);

    children = sheet->children;

    while (children)
    {
        child = (GtkSheetChild *)children->data;

        if (child->widget == widget) break;

        children = children->next;
    }

    if (children)
    {
        if (child->row == -1)
            sheet->row[child->col].button.child = NULL;

        if (child->col == -1)
            sheet->column[child->row]->button.child = NULL;

        gtk_widget_unparent (widget);
        child->widget = NULL;

        sheet->children = g_list_remove_link (sheet->children, children);
        g_list_free_1 (children);
        g_free(child);
    }

}

static void
    gtk_sheet_realize_child(GtkSheet *sheet, GtkSheetChild *child)
{
    GtkWidget *widget;

    widget = GTK_WIDGET(sheet);

    if (gtk_widget_get_realized(widget))
    {
        if (child->row == -1)
            gtk_widget_set_parent_window(child->widget, sheet->column_title_window);
        else if (child->col == -1)
            gtk_widget_set_parent_window(child->widget, sheet->row_title_window);
        else
            gtk_widget_set_parent_window(child->widget, sheet->sheet_window);
    }

    gtk_widget_set_parent(child->widget, widget);
}


/**
 * gtk_sheet_get_child_at:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @col: column number
 *
 * Get the child attached at @row,@col.
 *
 * returns: the #GtkSheetChild attached to @row,@col or NULL
 */
GtkSheetChild *
    gtk_sheet_get_child_at(GtkSheet *sheet, gint row, gint col)
{
    GList *children;

    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    children = sheet->children;

    while (children)
    {
        GtkSheetChild *child = (GtkSheetChild *) children->data;

        if (child->attached_to_cell)
        {
            if (child->row == row && child->col == col) return(child);
        }

        children = children->next;
    }
    return(NULL);
}

static void
    gtk_sheet_child_hide(GtkSheetChild *child)
{
    g_return_if_fail(child != NULL);
    gtk_widget_hide(child->widget);
}

static void
    gtk_sheet_child_show(GtkSheetChild *child)
{
    g_return_if_fail(child != NULL);
    gtk_widget_show(child->widget);
}
