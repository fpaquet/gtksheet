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
 * @short_description: A spreadsheet widget for Gtk+
 *
 * #GtkSheet is a matrix widget for GTK+. It consists of an scrollable grid of
 * cells where you can put text or other #GtkWidget's in. Cells
 * are organized in rows (#GtkSheetRow) and columns 
 * (#GtkSheetColumn). Cell contents can be edited interactively 
 * through a specially designed entry (#GtkItemEntry). A 
 * #GtkSheet is also a container subclass, allowing you to 
 * display buttons, curves, pixmaps and any other widget in it. 
 * You can also set many attributes as: border, foreground and 
 * background color, text justification, and more. The 
 * testgtksheet program shows how easy is to create a 
 * spreadsheet-like GUI using this widget set. 
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkpixmap.h>
#include <pango/pango.h>

#define __GTKEXTRA_H_INSIDE__

#include "gtkextra-compat.h"
#include "gtkitementry.h"
#include "gtkdatatextview.h"
#include "gtksheet.h"
#include "gtkdataformat.h"
#include "gtkextra-marshal.h"
#include "gtkextratypebuiltins.h"

#undef GTK_SHEET_DEBUG

#ifdef DEBUG
#   undef GTK_SHEET_DEBUG
#define GTK_SHEET_DEBUG  0  /* define to activate debug output */
#endif

#ifdef GTK_SHEET_DEBUG
#   define GTK_SHEET_DEBUG_ADJUSTMENT  0
#   define GTK_SHEET_DEBUG_ALLOCATION  0
#   define GTK_SHEET_DEBUG_BUILDER   0
#   define GTK_SHEET_DEBUG_CELL_ACTIVATION  0
#   define GTK_SHEET_DEBUG_CHILDREN  0
#   define GTK_SHEET_DEBUG_CLICK  0
#   define GTK_SHEET_DEBUG_COLORS  0
#   define GTK_SHEET_DEBUG_DRAW  0
#   define GTK_SHEET_DEBUG_DRAW_BACKGROUND  0
#   define GTK_SHEET_DEBUG_DRAW_BUTTON  0
#   define GTK_SHEET_DEBUG_DRAW_LABEL  0
#   define GTK_SHEET_DEBUG_ENTER_PRESSED   0
#   define GTK_SHEET_DEBUG_ENTRY   0
#   define GTK_SHEET_DEBUG_EXPOSE   0
#   define GTK_SHEET_DEBUG_FINALIZE  0
#   define GTK_SHEET_DEBUG_FONT_METRICS  0
#   define GTK_SHEET_DEBUG_FREEZE   0
#   define GTK_SHEET_DEBUG_KEYPRESS   0
#   define GTK_SHEET_DEBUG_MOUSE  0
#   define GTK_SHEET_DEBUG_MOVE  0
#   define GTK_SHEET_DEBUG_MOTION  0
#   define GTK_SHEET_DEBUG_PIXEL_INFO  0
#   define GTK_SHEET_DEBUG_PROPERTIES  0
#   define GTK_SHEET_DEBUG_REALIZE  0
#   define GTK_SHEET_DEBUG_SELECTION  0
#   define GTK_SHEET_DEBUG_SIGNALS   0
#   define GTK_SHEET_DEBUG_SIZE  0
#   define GTK_SHEET_DEBUG_SET_CELL_TIMER  0
#   define GTK_SHEET_DEBUG_SET_CELL_TEXT  0
#endif

#define GTK_SHEET_MOD_MASK  GDK_MOD1_MASK  /* main modifier for sheet navigation */

#ifndef GDK_KEY_KP_Up
#   define GDK_KEY_KP_Up GDK_KP_Up
#   define GDK_KEY_KP_Down GDK_KP_Down
#   define GDK_KEY_KP_Page_Up GDK_KP_Page_Up
#   define GDK_KEY_KP_Page_Down GDK_KP_Page_Down
#   define GDK_KEY_KP_Page_Left GDK_KP_Page_Left
#   define GDK_KEY_KP_Page_Right GDK_KP_Page_Right
#   define GDK_KEY_KP_Home GDK_KP_Home
#   define GDK_KEY_KP_End GDK_KP_End
#   define GDK_KEY_KP_Left GDK_KP_Left
#   define GDK_KEY_KP_Right GDK_KP_Right
#   define GDK_KEY_KP_Enter GDK_KP_Enter
#endif

#if !GTK_CHECK_VERSION(2,22,0)
static GdkCursorType
gdk_cursor_get_cursor_type (GdkCursor *cursor)
{
  g_return_val_if_fail (cursor != NULL, GDK_BLANK_CURSOR);
  return cursor->type;
}
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
    GTK_SHEET_IN_REDRAW_PENDING  = 1 << 8,  /* redraw on deactivate */
    GTK_SHEET_IN_AUTORESIZE_PENDING  = 1 << 9,  /* autoresize pending */
    GTK_SHEET_IS_DESTROYED = 1 << 10,  /* set by destruct handler */
};

enum _GtkSheetProperties
{
    PROP_GTK_SHEET_0,  /* dummy */
    PROP_GTK_SHEET_TITLE, /* gtk_sheet_set_title() */
    PROP_GTK_SHEET_DESCRIPTION,  /* gtk_sheet_set_description() */
    PROP_GTK_SHEET_NCOLS, /* number of colunms - necessary for glade object creation */
    PROP_GTK_SHEET_NROWS,  /* number of rows - necessary for glade object creation */
    PROP_GTK_SHEET_LOCKED,  /* gtk_sheet_set_locked() */
    PROP_GTK_SHEET_SELECTION_MODE,  /* gtk_sheet_set_selection_mode() */
    PROP_GTK_SHEET_AUTO_RESIZE,  /* gtk_sheet_set_autoresize() */
    PROP_GTK_SHEET_AUTO_RESIZE_ROWS,  /* gtk_sheet_set_autoresize_rows() */
    PROP_GTK_SHEET_AUTO_RESIZE_COLUMNS,  /* gtk_sheet_set_autoresize_columns() */
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
    PROP_GTK_SHEET_VJUST,  /* gtk_sheet_set_vjustification() */
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
    ENTRY_FOCUS_IN,
    ENTRY_FOCUS_OUT,
    ENTRY_POPULATE_POPUP,
    MOVE_CURSOR,
    ENTER_PRESSED,
    LAST_SIGNAL
};
static guint sheet_signals[LAST_SIGNAL] = { 0 };

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
#define GTK_SHEET_REDRAW_PENDING(sheet)   (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_REDRAW_PENDING)

/* data access macros - no frontend update! */

#define COLPTR(sheet, colidx) (sheet->column[colidx])
#define ROWPTR(sheet, rowidx) (&sheet->row[rowidx])

#define GTK_SHEET_ROW_IS_VISIBLE(rowptr)  ((rowptr)->is_visible)
#define GTK_SHEET_ROW_SET_VISIBLE(rowptr, value) ((rowptr)->is_visible = (value))
#define GTK_SHEET_ROW_IS_SENSITIVE(rowptr)  ((rowptr)->is_sensitive)
#define GTK_SHEET_ROW_SET_SENSITIVE(rowptr, value) ((rowptr)->is_sensitive = (value))
#define GTK_SHEET_ROW_CAN_FOCUS(rowptr) GTK_SHEET_ROW_IS_SENSITIVE(rowptr)


#define MIN_VIEW_ROW(sheet)  (sheet->view.row0)
#define MAX_VIEW_ROW(sheet)  (sheet->view.rowi)  /* beware: MAX_VISIBLE_ROW() can be maxrow+1 */
#define MIN_VIEW_COLUMN(sheet)  (sheet->view.col0)
#define MAX_VIEW_COLUMN(sheet)  (sheet->view.coli)  /* beware: MAX_VISIBLE_COLUMN() can be maxcol+1 */

#define COLUMN_UNREALIZED_MAX_WIDTH 512  /* unrealized maximum width */
#define COLUMN_REMNANT_PIXELS  32   /* maximized: free space left for others */
#define ROW_UNREALIZED_MAX_HEIGHT 128  /* unrealized maximum height */
#define ROW_REMNANT_PIXELS  32   /* maximized: free space left for others */

#define COLUMN_MAX_WIDTH(sheet) \
    (sheet->sheet_window_width < COLUMN_REMNANT_PIXELS ? \
    COLUMN_UNREALIZED_MAX_WIDTH : \
    sheet->sheet_window_width - COLUMN_REMNANT_PIXELS )

#if 0
#   define ROW_MAX_HEIGHT(sheet) \
    (sheet->sheet_window_height < ROW_REMNANT_PIXELS ? \
    ROW_UNREALIZED_MAX_HEIGHT : \
    sheet->sheet_window_height - ROW_REMNANT_PIXELS )
#else
#   define ROW_MAX_HEIGHT(sheet) \
    (sheet->sheet_window_height < ROW_REMNANT_PIXELS ? \
    ROW_UNREALIZED_MAX_HEIGHT : \
    sheet->sheet_window_height * 1/3 )
#endif

#define CELL_EXTENT_WIDTH(text_width, attr_border_width)  \
    (text_width + attr_border_width)

#define CELL_EXTENT_HEIGHT(text_height, attr_border_height)  \
    (text_height + attr_border_height)

#define COLUMN_EXTENT_TO_WIDTH(extent_width) \
    (extent_width + 2*CELLOFFSET > COLUMN_MAX_WIDTH(sheet) ? \
    COLUMN_MAX_WIDTH(sheet) : \
    extent_width + 2*CELLOFFSET)

#define ROW_EXTENT_TO_HEIGHT(extent_height) \
    (extent_height + 2*CELLOFFSET > ROW_MAX_HEIGHT(sheet) ? \
    ROW_MAX_HEIGHT(sheet) : \
    extent_height + 2*CELLOFFSET)

/* GtkSheetRange macros */

#define _RECT_IN_RANGE(row_0, row_i, col_0, col_i, range) \
     ((range)->row0 <= (row_0) && (row_i) <= (range)->rowi \
     && (range)->col0 <= (col_0) && (col_i) <= (range)->coli)

#define _POINT_IN_RANGE(row, col, range) \
    _RECT_IN_RANGE(row, row, col, col, range)

#define _RECT_EQ_RANGE(row_0, row_i, col_0, col_i, range) \
     ((row_0) == (range)->row0 && (row_i) == (range)->rowi \
     && (col_0) == (range)->col0 && (col_i) == (range)->coli)

#define _RECT_NEQ_RANGE(row_0, row_i, col_0, col_i, range) \
     ((row_0) != (range)->row0 || (row_i) != (range)->rowi \
     || (col_0) != (range)->col0 || (col_i) != (range)->coli)

#define _RANGE_EQ_RANGE(range1, range2) \
     ((range1)->row0 == (range2)->row0 && (range1)->rowi == (range2)->rowi \
     && (range1)->col0 == (range2)->col0 && (range1)->coli == (range2)->coli)

#define _RANGE_NEQ_RANGE(range1, range2) \
     ((range1)->row0) != (range2)->row0 || (range1)->rowi != (range2)->rowi \
     || (range1)->col0 != (range2)->col0 || (range1)->coli != (range2)->coli)




/* defaults */

#define CELL_SPACING 1
#define DRAG_WIDTH 6
#define TIMEOUT_SCROLL 20
#define TIMEOUT_FLASH 200
#define TIME_INTERVAL 8
#define MINROWS 0
#define MINCOLS 0
#define MAXLENGTH 30
#define CELLOFFSET 4

#define GTK_SHEET_ROW_DEFAULT_HEIGHT 24

#define GTK_SHEET_DEFAULT_FONT_ASCENT  12
#define GTK_SHEET_DEFAULT_FONT_DESCENT  12

#define GTK_SHEET_DEFAULT_BG_COLOR      "lightgray"
#define GTK_SHEET_DEFAULT_GRID_COLOR  "gray"
#define GTK_SHEET_DEFAULT_TM_COLOR  "red"   /* tooltip marker */
#define GTK_SHEET_DEFAULT_TM_SIZE  4  /* pixels, size of tooltip marker */

#define GTK_SHEET_PAGE_OVERLAP 1  /* rows to stay visible with PageUp/Dn */

#ifdef GTK_SHEET_DEBUG
#   define GTK_SHEET_DEBUG_COLOR  "green"
static GdkColor debug_color;

#   if 0
#       include <stdarg.h>
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
#   endif

#endif

/**
 * _gtk_sheet_row_default_height:
 * @widget: a #GtkWidget
 * 
 * Returns: row height in pixels 
 *  
 * calculate row height from font or return default row height 
 * when there is no font associated
 */
guint
_gtk_sheet_row_default_height(GtkWidget *widget)
{
    PangoFontDescription *font_desc = 
	gtk_widget_get_style(GTK_WIDGET(widget))->font_desc;

    if (!font_desc)
	return (GTK_SHEET_ROW_DEFAULT_HEIGHT);

    PangoContext *context = gtk_widget_get_pango_context(widget);

    PangoFontMetrics *metrics = pango_context_get_metrics(context,
	font_desc, pango_context_get_language(context));
    guint val = pango_font_metrics_get_descent(metrics) +
	pango_font_metrics_get_ascent(metrics);
    pango_font_metrics_unref(metrics);

    return (PANGO_PIXELS(val) + 2 * CELLOFFSET);
}

static inline guint
_default_font_ascent(GtkWidget *widget)
{
    PangoFontDescription *font_desc = 
	gtk_widget_get_style(GTK_WIDGET(widget))->font_desc;

    if (!font_desc)
	return (GTK_SHEET_DEFAULT_FONT_ASCENT);

    PangoContext *context = gtk_widget_get_pango_context(widget);

    PangoFontMetrics *metrics = pango_context_get_metrics(context,
	font_desc, pango_context_get_language(context));
    guint val = pango_font_metrics_get_ascent(metrics);
    pango_font_metrics_unref(metrics);

    return (PANGO_PIXELS(val));
}

static void _get_string_extent(GtkSheet *sheet, GtkSheetColumn *colptr,
    PangoFontDescription *font_desc, const gchar *text,
    guint *width, guint *height)
{
    PangoRectangle extent;
    PangoLayout *layout;

    layout = gtk_widget_create_pango_layout(GTK_WIDGET(sheet), text);
    pango_layout_set_font_description(layout, font_desc);

    if (colptr && !gtk_sheet_autoresize_columns(sheet))
    {
	switch(colptr->wrap_mode)
	{
	    case GTK_WRAP_NONE: 
		break;

	    case GTK_WRAP_CHAR:
		pango_layout_set_width(layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
		break;

	    case GTK_WRAP_WORD:
		pango_layout_set_width(layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
		break;

	    case GTK_WRAP_WORD_CHAR:
		pango_layout_set_width(layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
		break;
	}
    }


    pango_layout_get_pixel_extents(layout, NULL, &extent);

#if GTK_SHEET_DEBUG_FONT_METRICS > 0
    {
	PangoContext *context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
	PangoFontMetrics *metrics = pango_context_get_metrics(
	    context, font_desc, pango_context_get_language(context));

	gint ascent = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
	gint descent = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;
	gint spacing = pango_layout_get_spacing(layout) / PANGO_SCALE;

	pango_font_metrics_unref(metrics);

	g_debug("_get_string_extent(%s): ext (%d, %d, %d, %d) asc %d desc %d spac %d",
	    text,
	    extent.x, extent.y,
	    extent.width, extent.height,
	    ascent, descent, spacing);
    }
#endif

    g_object_unref(G_OBJECT(layout));

    if (width)
	*width = extent.width;
    if (height)
	*height = extent.height;
}

static inline guint
_default_font_descent(GtkWidget *widget)
{
    PangoFontDescription *font_desc = 
	gtk_widget_get_style(GTK_WIDGET(widget))->font_desc;

    if (!font_desc)
	return (GTK_SHEET_DEFAULT_FONT_DESCENT);

    PangoContext *context = gtk_widget_get_pango_context(widget);

    PangoFontMetrics *metrics = pango_context_get_metrics(context,
	font_desc, pango_context_get_language(context));
    guint val =  pango_font_metrics_get_descent(metrics);
    pango_font_metrics_unref(metrics);

    return (PANGO_PIXELS(val));
}

/* gives the top/bottom pixel of the given row in context of the sheet's voffset */

static inline gint
_gtk_sheet_row_top_ypixel(GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow)
	return (sheet->voffset);
    return (sheet->voffset + sheet->row[row].top_ypixel);
}

static inline gint
_gtk_sheet_row_bottom_ypixel(GtkSheet *sheet, gint row)
{
    gint ypixel = _gtk_sheet_row_top_ypixel(sheet, row);
    if (0 <= row && row <= sheet->maxrow)
	ypixel += sheet->row[row].height;
    return (ypixel);
}


/** 
 * _gtk_sheet_row_from_ypixel: 
 * @sheet:  the sheet
 * @y:      the pixel 
 *  
 * get row from y pixel location. returns the row index 
 * from a y pixel location (relative to the sheet window) 
 * honoring the sheet's scrolling offset and title visibility.
 * 
 * beware: the top border belongs to the row, the bottom border to the next
 *  
 * Returns: row index, -1 or maxcol+1 (beyond right edge)
 */

static inline gint
_gtk_sheet_row_from_ypixel(GtkSheet *sheet, gint y)
{
    gint i, cy;

    cy = sheet->voffset;
    if (sheet->column_titles_visible)
	cy += sheet->column_title_area.height;

    if (y < cy)
	return (-1);    /* top outside */

    for (i = 0; i <= sheet->maxrow; i++)
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	{
	    if (cy <= y  && y < (cy + sheet->row[i].height))
		return (i);
	    cy += sheet->row[i].height;
	}
    }

    /* no match */
    return (sheet->maxrow + 1);
}



/** 
 * _gtk_sheet_column_from_xpixel: 
 * @sheet:  the sheet
 * @x:      the pixel 
 *  
 * get column from x pixel location. returns the column index 
 * from a x pixel location  (relative to the sheet window) 
 * honoring the sheet's scrolling offset and title visibility.
 * 
 * beware: the left border belongs to the column, the right border to the next
 *  
 * Returns: column index, or maxcol+1 (beyond right edge)
 */
static inline gint
_gtk_sheet_column_from_xpixel(GtkSheet *sheet, gint x)
{
    gint i, cx;

    cx = sheet->hoffset;
    if (sheet->row_titles_visible)
	cx += sheet->row_title_area.width;

    if (x < cx) {
	return (-1);  /* left outside */
    }

    for (i = 0; i <= sheet->maxcol; i++)
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
	{
	    if (cx <= x  && x < (cx + COLPTR(sheet, i)->width))
		return (i);
	    cx += COLPTR(sheet, i)->width;
	}
    }

    /* no match */
    return (sheet->maxcol + 1);
}

/**
 * _gtk_sheet_first_visible_colidx: 
 * @sheet:  the sheet 
 *  
 * find index of leftmost visible column >= startidx
 *  
 * returns: column index or -1 
 */
static inline gint _gtk_sheet_first_visible_colidx(GtkSheet *sheet, gint startidx)
{
    gint i;
    for (i = startidx; i <= sheet->maxcol; i++)
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
	    return (i);
    }
    return (-1);
}

/**
 * _gtk_sheet_last_visible_colidx: 
 * @sheet:  the sheet 
 *  
 * find  index of rightmost visible column <= startidx
 *  
 * returns: column index or -1
 */
static inline gint _gtk_sheet_last_visible_colidx(GtkSheet *sheet, gint startidx)
{
    gint i;
    for (i = startidx; i >= 0; i--)
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
	    return (i);
    }
    return (-1);
}

/**
 * _gtk_sheet_first_visible_rowidx: 
 * @sheet:  the sheet 
 *  
 * find index of topmost visible row >= startidx
 *  
 * returns: row index or -1 
 */
static inline gint _gtk_sheet_first_visible_rowidx(GtkSheet *sheet, gint startidx)
{
    gint i;
    for (i = startidx; i <= sheet->maxrow; i++)
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    return (i);
    }
    return (-1);
}

/**
 * _gtk_sheet_last_visible_rowidx: 
 * @sheet:  the sheet 
 *  
 * find index of bottommost visible row <= startidx
 *  
 * returns: row index or -1 
 */
static inline gint _gtk_sheet_last_visible_rowidx(GtkSheet *sheet, gint startidx)
{
    gint i;
    for (i = startidx; i >= 0; i--)
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    return (i);
    }
    return (-1);
}

/**
 * _gtk_sheet_get_visible_range:
 * @sheet:  the sheet 
 * @visr:  pointer to store results
 *  
 * return visible sheet area 
 * [first visible row/col .. last visible row/col]
 *  
 * returns: TRUE if any visible cells exist and range is valid
 */
static inline gboolean _gtk_sheet_get_visible_range(GtkSheet *sheet,
    GtkSheetRange *visr)
{
    visr->row0 = visr->rowi = visr->col0 = visr->coli = -1;

    visr->row0 = _gtk_sheet_first_visible_rowidx(sheet, 0);
    if (visr->row0 < 0)
	return (FALSE);

    visr->rowi = _gtk_sheet_last_visible_rowidx(sheet, sheet->maxrow);
    if (visr->rowi < 0)
	return (FALSE);

    visr->col0 = _gtk_sheet_first_visible_colidx(sheet, 0);
    if (visr->col0 < 0)
	return (FALSE);

    visr->coli = _gtk_sheet_last_visible_colidx(sheet, sheet->maxcol);
    if (visr->coli < 0)
	return (FALSE);

    return (TRUE);
}

/**
 * _gtk_sheet_count_visible:
 * @sheet:  the sheet 
 * @range: the #GtkSheetRange to inspect 
 * @nrows: number of visible rows in range (out)
 * @ncols: number of visible columns in range (out)
 *  
 * count visible rows/cols in range
 */
static inline void _gtk_sheet_count_visible(GtkSheet *sheet,
    GtkSheetRange *range, gint *nrows, gint *ncols)
{
    gint i;
    *nrows = *ncols = 0;

    for (i = range->row0; i <= range->rowi; i++)
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    ++(*nrows);
    }
    for (i = range->col0; i <= range->coli; i++)
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
	    ++(*ncols);
    }
}



/**
 * _gtk_sheet_height: 
 * @sheet:  the #GtkSheet 
 * 
 * returns the total height of the sheet 
 *  
 * returns: total height of all visible rows, including 
 * col_titles area 
 */
static inline gint
_gtk_sheet_height(GtkSheet *sheet)
{
    gint i, cx;

    cx = (sheet->column_titles_visible ? sheet->column_title_area.height : 0);

    for (i = 0; i <= sheet->maxrow; i++)
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    cx += sheet->row[i].height;
    }

    return (cx);
}

/**
 * _gtk_sheet_width: 
 * @sheet:  the #GtkSheet 
 * 
 * total width of the sheet 
 *  
 * returns: total width of all visible columns, including 
 * row_titles area 
 */
static inline gint
_gtk_sheet_width(GtkSheet *sheet)
{
    gint i, cx;

    cx = (sheet->row_titles_visible ? sheet->row_title_area.width : 0);

    for (i = 0; i <= sheet->maxcol; i++)
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
	    cx += COLPTR(sheet, i)->width;
    }

    return (cx);
}

/**
 * _gtk_sheet_recalc_view_range:
 * @sheet:  the #GtkSheet
 * 
 * recalculate visible sheet range
 */
void
_gtk_sheet_recalc_view_range(GtkSheet *sheet)
{
    sheet->view.row0 = _gtk_sheet_row_from_ypixel(sheet,
	sheet->column_titles_visible ? sheet->column_title_area.height : 0);
    sheet->view.rowi = _gtk_sheet_row_from_ypixel(sheet,
	sheet->sheet_window_height - 1);

    sheet->view.col0 = _gtk_sheet_column_from_xpixel(sheet,
	sheet->row_titles_visible ? sheet->row_title_area.width : 0);
    sheet->view.coli = _gtk_sheet_column_from_xpixel(sheet,
	sheet->sheet_window_width - 1);
}

/**
 * _gtk_sheet_range_fixup:
 * 
 * @param sheet  the #GtkSheet
 * @param range  the range
 * 
 * force range into bounds
 */
void
_gtk_sheet_range_fixup(GtkSheet *sheet, GtkSheetRange *range)
{
    if (range->row0 < 0)
	range->row0 = 0;
    if (range->rowi > sheet->maxrow)
	range->rowi = sheet->maxrow;
    if (range->col0 < 0)
	range->col0 = 0;
    if (range->coli > sheet->maxcol)
	range->coli = sheet->maxcol;
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

    column = _gtk_sheet_column_from_xpixel(sheet, x);
    if (column < 0 || column > sheet->maxcol)
	return (FALSE);

    xdrag = _gtk_sheet_column_left_xpixel(sheet, column);

    if (column > 0 && x <= xdrag + DRAG_WIDTH / 2)  /* you pick it at the left border */
    {
	while (column > 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, column - 1))) column--;

	--column;  /* you really want to resize the column on the left side */

	if (column < 0 || column > sheet->maxcol)
	    return (FALSE);

	*drag_column = column;
	return (TRUE);
#if 0
	return(GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)));
#endif
    }

    xdrag = _gtk_sheet_column_right_xpixel(sheet, column);

    if (xdrag - DRAG_WIDTH / 2 <= x && x <= xdrag + DRAG_WIDTH / 2)
    {
	*drag_column = column;
	return (TRUE);
#if 0
	return(GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)));
#endif
    }

    return (FALSE);
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

    row = _gtk_sheet_row_from_ypixel(sheet, y);
    if (row < 0 || row > sheet->maxrow)
	return (FALSE);

    ydrag = _gtk_sheet_row_top_ypixel(sheet, row);

    if (row > 0 && y <= ydrag + DRAG_WIDTH / 2)  /* you pick it at the top border */
    {
	while (row > 0 && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row - 1))) row--;

	--row;  /* you really want to resize the row above */

	if (row < 0 || row > sheet->maxrow)
	    return (FALSE);

	*drag_row = row;
	return (TRUE);
#if 0
	return(GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)));
#endif
    }

    ydrag = _gtk_sheet_row_bottom_ypixel(sheet, row);

    if (ydrag - DRAG_WIDTH / 2 <= y && y <= ydrag + DRAG_WIDTH / 2)
    {
	*drag_row = row;
	return (TRUE);
#if 0
	return(GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)));
#endif
    }

    return (FALSE);
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

    *drag_column = _gtk_sheet_column_from_xpixel(sheet, x);
    *drag_row = _gtk_sheet_row_from_ypixel(sheet, y);

    /* drag grip at top/bottom border */

    if (x >= _gtk_sheet_column_left_xpixel(sheet, sheet->range.col0) - DRAG_WIDTH / 2 &&
	x <= _gtk_sheet_column_right_xpixel(sheet, sheet->range.coli) + DRAG_WIDTH / 2)
    {
	ydrag = _gtk_sheet_row_top_ypixel(sheet, sheet->range.row0);

	if (ydrag - DRAG_WIDTH / 2 <= y && y <= ydrag + DRAG_WIDTH / 2)
	{
	    *drag_row = sheet->range.row0;
	    return (TRUE);
	}

	ydrag = _gtk_sheet_row_bottom_ypixel(sheet, sheet->range.rowi);

	if (ydrag - DRAG_WIDTH / 2 <= y && y <= ydrag + DRAG_WIDTH / 2)
	{
	    *drag_row = sheet->range.rowi;
	    return (TRUE);
	}

    }

    /* drag grip at left/right border */

    if (y >= _gtk_sheet_row_top_ypixel(sheet, sheet->range.row0) - DRAG_WIDTH / 2 &&
	y <= _gtk_sheet_row_bottom_ypixel(sheet, sheet->range.rowi) + DRAG_WIDTH / 2)
    {
	xdrag = _gtk_sheet_column_left_xpixel(sheet, sheet->range.col0);

	if (xdrag - DRAG_WIDTH / 2 <= x && x <= xdrag + DRAG_WIDTH / 2)
	{
	    *drag_column = sheet->range.col0;
	    return (TRUE);
	}

	xdrag = _gtk_sheet_column_right_xpixel(sheet, sheet->range.coli);

	if (xdrag - DRAG_WIDTH / 2 <= x && x <= xdrag + DRAG_WIDTH / 2)
	{
	    *drag_column = sheet->range.coli;
	    return (TRUE);
	}
    }

    return (FALSE);
}

static inline gint
POSSIBLE_RESIZE(GtkSheet *sheet, gint x, gint y, gint *drag_row, gint *drag_column)
{
    gint xdrag, ydrag;

    xdrag = _gtk_sheet_column_right_xpixel(sheet, sheet->range.coli);
    ydrag = _gtk_sheet_row_bottom_ypixel(sheet, sheet->range.rowi);

    if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
	ydrag = _gtk_sheet_row_top_ypixel(sheet, sheet->view.row0);

    if (sheet->state == GTK_SHEET_ROW_SELECTED)
	xdrag = _gtk_sheet_column_left_xpixel(sheet, sheet->view.col0);

    *drag_column = _gtk_sheet_column_from_xpixel(sheet, x);
    *drag_row = _gtk_sheet_row_from_ypixel(sheet, y);

    if (xdrag - DRAG_WIDTH / 2 <= x && x <= xdrag + DRAG_WIDTH / 2 &&
	ydrag - DRAG_WIDTH / 2 <= y && y <= ydrag + DRAG_WIDTH / 2)
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
static void gtk_sheet_destroy_handler(GtkObject *object);
static void gtk_sheet_finalize_handler(GObject *object);
static void gtk_sheet_style_set_handler(GtkWidget *widget,
    GtkStyle  *previous_style);
static void gtk_sheet_realize_handler(GtkWidget *widget);
static void gtk_sheet_unrealize_handler(GtkWidget *widget);
static void gtk_sheet_map_handler(GtkWidget *widget);
static void gtk_sheet_unmap_handler(GtkWidget *widget);

static gboolean gtk_sheet_expose_handler(GtkWidget *widget,
    GdkEventExpose *event);

static void gtk_sheet_forall_handler(GtkContainer *container,
    gboolean include_internals,
    GtkCallback  callback,
    gpointer callback_data);

static void gtk_sheet_set_scroll_adjustments(GtkSheet *sheet,
    GtkAdjustment *hadjustment,
    GtkAdjustment *vadjustment);

static gboolean gtk_sheet_button_press_handler(GtkWidget *widget,
    GdkEventButton *event);
static gboolean gtk_sheet_button_release_handler(GtkWidget *widget,
    GdkEventButton *event);
static gboolean gtk_sheet_motion_handler(GtkWidget *widget,
    GdkEventMotion *event);

static gboolean gtk_sheet_entry_key_press_handler(GtkWidget *widget,
    GdkEventKey *key,
    gpointer user_data);

static gboolean gtk_sheet_key_press_handler(GtkWidget *widget,
    GdkEventKey *key);

static void gtk_sheet_size_request_handler(GtkWidget *widget,
    GtkRequisition *requisition);
static void gtk_sheet_size_allocate_handler(GtkWidget *widget,
    GtkAllocation *allocation);

static gboolean gtk_sheet_focus(GtkWidget *widget,
    GtkDirectionType  direction);

static void _gtk_sheet_move_cursor(GtkSheet *sheet,
    GtkMovementStep step,
    gint count,
    gboolean extend_selection);

/* Sheet queries */

static gint gtk_sheet_range_isvisible(GtkSheet *sheet, GtkSheetRange range);
static gint gtk_sheet_cell_isvisible(GtkSheet *sheet, gint row, gint column);

/* Clipped Range */

static gint _gtk_sheet_scroll_to_pointer(gpointer data);
static gint gtk_sheet_flash(gpointer data);

/* Drawing Routines */

/* draw cell background and frame */
static void _cell_draw_background(GtkSheet *sheet, gint row, gint column);

/* draw cell border */
static void _cell_draw_border(GtkSheet *sheet,
    gint row, gint column, gint mask);

/* draw cell contents */
static void _cell_draw_label(GtkSheet *sheet, gint row, gint column);

/* highlight the visible part of the selected range */
static void gtk_sheet_range_draw_selection(GtkSheet *sheet, GtkSheetRange range);

/* Selection */

static gint _gtk_sheet_move_query(GtkSheet *sheet, gint row, gint column, gboolean need_focus);
static void gtk_sheet_real_select_range(GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_real_unselect_range(GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_extend_selection(GtkSheet *sheet, gint row, gint column);
static void gtk_sheet_new_selection(GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_draw_border(GtkSheet *sheet, GtkSheetRange range);
static void gtk_sheet_draw_corners(GtkSheet *sheet, GtkSheetRange range);

/* Active Cell handling */

static void gtk_sheet_entry_changed_handler(GtkWidget *widget, gpointer data);
static gboolean gtk_sheet_deactivate_cell(GtkSheet *sheet);
static gboolean gtk_sheet_activate_cell(GtkSheet *sheet, gint row, gint col);
static void gtk_sheet_draw_active_cell(GtkSheet *sheet);
static void gtk_sheet_show_active_cell(GtkSheet *sheet);
static void gtk_sheet_click_cell(GtkSheet *sheet, gint row, gint column, gboolean *veto);

/* Backing Pixmap */

static void gtk_sheet_make_backing_pixmap(GtkSheet *sheet, guint width, guint height);
static void gtk_sheet_draw_backing_pixmap(GtkSheet *sheet, GtkSheetRange range);
/* Scrollbars */

static void _vadjustment_changed_handler(GtkAdjustment *adjustment, gpointer data);
static void _hadjustment_changed_handler(GtkAdjustment *adjustment, gpointer data);
static void _vadjustment_value_changed_handler(GtkAdjustment *adjustment, gpointer data);
static void _hadjustment_value_changed_handler(GtkAdjustment *adjustment, gpointer data);

static void draw_xor_vline(GtkSheet *sheet);
static void draw_xor_hline(GtkSheet *sheet);
static void draw_xor_rectangle(GtkSheet *sheet, GtkSheetRange range);
static void gtk_sheet_draw_flashing_range(GtkSheet *sheet, GtkSheetRange range);
static guint new_column_width(GtkSheet *sheet, gint column, gint *x);
static guint new_row_height(GtkSheet *sheet, gint row, gint *y);

/* Sheet Button */

static void create_global_button(GtkSheet *sheet);

/* Sheet Entry */

static void create_sheet_entry(GtkSheet *sheet, GType new_entry_type);
static void gtk_sheet_entry_set_max_size(GtkSheet *sheet);

/* Sheet button gadgets */

static void size_allocate_row_title_buttons(GtkSheet *sheet);

static void row_button_set(GtkSheet *sheet, gint row);
static void row_button_release(GtkSheet *sheet, gint row);
static void size_allocate_global_button(GtkSheet *sheet);

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

static void AddRows(GtkSheet *sheet, gint position, gint nrows);
static void AddColumns(GtkSheet *sheet, gint position, gint ncols);
static void InsertRow(GtkSheet *sheet, gint row, gint nrows);
static void InsertColumn(GtkSheet *sheet, gint col, gint ncols);
static void DeleteRow(GtkSheet *sheet, gint row, gint nrows);
static void DeleteColumn(GtkSheet *sheet, gint col, gint ncols);
static gint GrowSheet(GtkSheet *sheet, gint newrows, gint newcols);
static void CheckBounds(GtkSheet *sheet, gint row, gint col);
static void CheckCellData(GtkSheet *sheet, const gint row, const gint col);

/* Container Functions */
static void gtk_sheet_remove_handler(GtkContainer *container, GtkWidget *widget);
static void gtk_sheet_realize_child(GtkSheet *sheet, GtkSheetChild *child);
static void gtk_sheet_position_child(GtkSheet *sheet, GtkSheetChild *child);
static void gtk_sheet_position_children(GtkSheet *sheet);
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
	COLPTR(sheet, col)->sheet = NULL;

	g_object_unref(sheet->column[col]);
	sheet->column[col] = NULL;
    }

    child->sheet = sheet;
    sheet->column[col] = child;

    g_object_ref_sink(G_OBJECT(child));

#if GTK_SHEET_DEBUG_BUILDER > 0
    g_debug("gtk_sheet_buildable_add_child_internal: %s, m %d r %d v %d",
	name ? name : "NULL",
	gtk_widget_get_mapped(GTK_WIDGET(child)),
	gtk_widget_get_realized(GTK_WIDGET(child)),
	gtk_widget_get_visible(GTK_WIDGET(child))
	);
#endif

    /* we always set the parent in order to track into what sheet the column belongs,
       which leads to problems with invisible columns.
       When trying to set them visible, Gtk 2.24.5 terminates the application with
       ? Gtk - gtk_widget_realize: assertion `GTK_WIDGET_ANCHORED (widget) || GTK_IS_INVISIBLE (widget)' failed
       see also the fix in gtk_sheet_column_set_visibility()
       */
    gtk_widget_set_parent(GTK_WIDGET(child), GTK_WIDGET(sheet));

    if (name)
	gtk_widget_set_name(GTK_WIDGET(child), name);

    _gtk_sheet_reset_text_column(sheet, col);
    _gtk_sheet_recalc_left_xpixels(sheet);
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

#if GTK_SHEET_DEBUG_BUILDER > 0
    g_debug("gtk_sheet_buildable_add_child %p: %s type %s", child,
	name ? name : "NULL",
	type ? type : "NULL");
#endif

    sheet = GTK_SHEET(buildable);
    newcol = GTK_SHEET_COLUMN(child);

#if GTK_SHEET_DEBUG_BUILDER > 0
    {
	gchar *strval;

	g_object_get(G_OBJECT(newcol), "label", &strval, NULL);
	g_debug("gtk_sheet_buildable_add_child: label=%s", strval ? strval : "NULL");

	g_free(strval);
    }
#endif

    gtk_sheet_buildable_add_child_internal(sheet, newcol, name);
}

static void
gtk_sheet_buildable_init(GtkBuildableIface *iface)
{
#if GTK_SHEET_DEBUG_BUILDER > 0
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
	    return (ON_SHEET_BUTTON_AREA);

	return (ON_COLUMN_TITLES_AREA);
    }
    else
    {
	if (sheet->row_titles_visible && (x < sheet->row_title_area.width))
	    return (ON_ROW_TITLES_AREA);
    }
    return (ON_CELL_AREA);
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

    if (!sheet)
	return (FALSE);

    area = gtk_sheet_get_area_at(sheet, x, y);

    if (area == ON_CELL_AREA)
    {
	if (row < 0)
	    row = _gtk_sheet_row_from_ypixel(sheet, y);
	if (col < 0)
	    col = _gtk_sheet_column_from_xpixel(sheet, x);

	if ((0 <= row && row <= sheet->maxrow && 0 <= col && col <= sheet->maxcol)
	    && (row <= sheet->maxallocrow && col <= sheet->maxalloccol)
	    && (sheet->data[row] && sheet->data[row][col]))
	{
	    GtkSheetCell *cell = sheet->data[row][col];

	    tip = cell->tooltip_markup;
	    if (tip && tip[0])
	    {
		gtk_tooltip_set_markup(tooltip, tip);
		return (TRUE);
	    }

	    tip = cell->tooltip_text;
	    if (tip && tip[0])
	    {
		gtk_tooltip_set_text(tooltip, tip);
		return (TRUE);
	    }
	}

	area = ON_ROW_TITLES_AREA;  /* fallback */
    }

    if (area == ON_ROW_TITLES_AREA)
    {
	if (row < 0)
	    row = _gtk_sheet_row_from_ypixel(sheet, y);

	if (0 <= row && row <= sheet->maxrow)
	{
	    GtkSheetRow *rowp = ROWPTR(sheet, row);

	    tip = rowp->tooltip_markup;
	    if (tip && tip[0])
	    {
		gtk_tooltip_set_markup(tooltip, tip);
		return (TRUE);
	    }

	    tip = rowp->tooltip_text;
	    if (tip && tip[0])
	    {
		gtk_tooltip_set_text(tooltip, tip);
		return (TRUE);
	    }
	}

	area = ON_COLUMN_TITLES_AREA;  /* fallback */
    }

    if (area == ON_COLUMN_TITLES_AREA)
    {
	if (col < 0)
	    col = _gtk_sheet_column_from_xpixel(sheet, x);

	if (0 <= col && col <= sheet->maxcol)
	{
	    GtkSheetColumn *column = COLPTR(sheet, col);

	    tip = gtk_widget_get_tooltip_markup(GTK_WIDGET(column));
	    if (tip && tip[0])
	    {
		gtk_tooltip_set_markup(tooltip, tip);
		g_free(tip);
		return (TRUE);
	    }

	    tip = gtk_widget_get_tooltip_text(GTK_WIDGET(column));
	    if (tip && tip[0])
	    {
		gtk_tooltip_set_text(tooltip, tip);
		g_free(tip);
		return (TRUE);
	    }
	}
    }

    /* fallback to sheet tip */

    tip = gtk_widget_get_tooltip_markup(widget);
    if (tip && tip[0])
    {
	gtk_tooltip_set_markup(tooltip, tip);
	g_free(tip);
	return (TRUE);
    }

    tip = gtk_widget_get_tooltip_text(widget);
    if (tip && tip[0])
    {
	gtk_tooltip_set_text(tooltip, tip);
	g_free(tip);
	return (TRUE);
    }

    return (FALSE);
}

/* Type initialisation */

static GtkContainerClass *sheet_parent_class = NULL;

GType
gtk_sheet_get_type(void)
{
    static GType sheet_type = 0;

    if (!sheet_type)
    {
	static const GTypeInfo sheet_info =
	{
	    sizeof(GtkSheetClass),
	    NULL,
	    NULL,
	    (GClassInitFunc)gtk_sheet_class_init,
	    NULL,
	    NULL,
	    sizeof(GtkSheet),
	    0,
	    (GInstanceInitFunc)gtk_sheet_init,
	    NULL,
	};

	static const GInterfaceInfo interface_info = {
	    (GInterfaceInitFunc)gtk_sheet_buildable_init,
	    (GInterfaceFinalizeFunc)NULL,
	    (gpointer)NULL
	};

	sheet_type = g_type_register_static(gtk_container_get_type(),
	    "GtkSheet",
	    &sheet_info,
	    0);

	g_type_add_interface_static(sheet_type, GTK_TYPE_BUILDABLE,
	    &interface_info);
    }
    return (sheet_type);
}



static GtkSheetRange *
gtk_sheet_range_copy(const GtkSheetRange *range)
{
    GtkSheetRange *new_range;

    g_return_val_if_fail(range != NULL, NULL);

    new_range = g_new(GtkSheetRange, 1);

    *new_range = *range;

    return (new_range);
}

static void
gtk_sheet_range_free(GtkSheetRange *range)
{
    g_return_if_fail(range != NULL);

    g_free(range);
}

GType
gtk_sheet_range_get_type(void)
{
    static GType sheet_range_type = 0;

    if (!sheet_range_type)
    {
	sheet_range_type = g_boxed_type_register_static("GtkSheetRange", (GBoxedCopyFunc)gtk_sheet_range_copy, (GBoxedFreeFunc)gtk_sheet_range_free);
    }
    return (sheet_range_type);
}

/**
 * _gtk_sheet_entry_type_from_gtype:
 * @entry_type: type to be mapped
 * 
 * map gtype to #GtkSheetEntryType 
 *  
 * Returns: #GtkSheetEntryType or #GTK_SHEET_ENTRY_TYPE_DEFAULT
 */
GtkSheetEntryType
_gtk_sheet_entry_type_from_gtype(GType entry_type)
{
    if (entry_type == G_TYPE_ITEM_ENTRY)
	return (GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY);

    else if (entry_type == GTK_TYPE_ENTRY)
	return (GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY);

    else if (entry_type == GTK_TYPE_TEXT_VIEW)
	return (GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW);

    else if (entry_type == GTK_TYPE_DATA_TEXT_VIEW)
	return (GTK_SHEET_ENTRY_TYPE_GTK_DATA_TEXT_VIEW);

    else if (entry_type == GTK_TYPE_SPIN_BUTTON)
	return (GTK_SHEET_ENTRY_TYPE_GTK_SPIN_BUTTON);

    else if (entry_type == GTK_TYPE_COMBO_BOX)
	return (GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX);

#if !GTK_CHECK_VERSION(2,24,0)
    else if (entry_type == GTK_TYPE_COMBO_BOX_ENTRY)
	return (GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX_ENTRY);
#endif

#if !GTK_CHECK_VERSION(2,4,0)
    else if (entry_type == GTK_TYPE_COMBO)
	return (GTK_SHEET_ENTRY_TYPE_GTK_COMBO);
#endif

    return (GTK_SHEET_ENTRY_TYPE_DEFAULT);
}

/**
 * _gtk_sheet_entry_type_to_gtype:
 * @ety:    type to be mapped
 * 
 * map #GType to #GtkSheetEntryType 
 *  
 * Returns: #GType or #G_TYPE_NONE
 */
GType
_gtk_sheet_entry_type_to_gtype(GtkSheetEntryType ety)
{
    switch(ety)
    {
	case GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY:
	    return (G_TYPE_ITEM_ENTRY);

	case GTK_SHEET_ENTRY_TYPE_GTK_ENTRY:
	    return (GTK_TYPE_ENTRY);

	case GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW:
	    return (GTK_TYPE_TEXT_VIEW);

	case GTK_SHEET_ENTRY_TYPE_GTK_DATA_TEXT_VIEW:
	    return (GTK_TYPE_DATA_TEXT_VIEW);

	case GTK_SHEET_ENTRY_TYPE_GTK_SPIN_BUTTON:
	    return (GTK_TYPE_SPIN_BUTTON);

	case GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX:
	    return (GTK_TYPE_COMBO_BOX);

#if !GTK_CHECK_VERSION(2,24,0)
	case GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX_ENTRY:
	    return (GTK_TYPE_COMBO_BOX_ENTRY);
#endif

#if !GTK_CHECK_VERSION(2,4,0)
	case GTK_SHEET_ENTRY_TYPE_GTK_COMBO:
	    return (GTK_TYPE_COMBO);
#endif

	default:
	    break;
    }
    return (G_TYPE_NONE);
}

/*
 * gtk_sheet_set_property - set sheet property
 * gtk_sheet_get_property - get sheet property
 * gtk_sheet_class_init_properties - initialize class properties
 * gtk_sheet_class_init_signals - initialize class signals
 * gtk_sheet_class_init_bindings - initialize class key bindings
 */

static void
gtk_sheet_set_property(GObject *object,
    guint         property_id,
    const GValue *value,
    GParamSpec   *pspec)
{
    GtkSheet *sheet = GTK_SHEET(object);

#if GTK_SHEET_DEBUG_PROPERTIES > 0
    g_debug("gtk_sheet_set_property: %s", pspec->name);
#endif

    switch(property_id)
    {
	case PROP_GTK_SHEET_TITLE:
	    gtk_sheet_set_title(sheet, g_value_get_string(value));
	    break;

	case PROP_GTK_SHEET_DESCRIPTION:
	    gtk_sheet_set_description(sheet, g_value_get_string(value));
	    break;

	case PROP_GTK_SHEET_NROWS:
	    {
		gint newval = g_value_get_int(value);

		if (newval < 0)
		    break;

#if GTK_SHEET_DEBUG_PROPERTIES > 0
		g_debug("gtk_sheet_set_property: newval = %d sheet->maxrow %d", newval, sheet->maxrow);
#endif
		if (newval < (sheet->maxrow + 1))
		{
		    gtk_sheet_delete_rows(sheet, newval, (sheet->maxrow + 1) - newval);
		    _gtk_sheet_recalc_view_range(sheet);
		}
		else if (newval > (sheet->maxrow + 1))
		{
		    gtk_sheet_add_row(sheet, newval - (sheet->maxrow + 1));
		    _gtk_sheet_recalc_view_range(sheet);
		}
	    }
	    break;

	case PROP_GTK_SHEET_NCOLS:
	    {
		gint newval = g_value_get_int(value);

		if (newval < 0)
		    break;
#if GTK_SHEET_DEBUG_PROPERTIES > 0
		g_debug("gtk_sheet_set_property: newval = %d sheet->maxcol %d", newval, sheet->maxcol);
#endif
		if (newval < (sheet->maxcol + 1))
		{
		    gtk_sheet_delete_columns(sheet, newval, (sheet->maxcol + 1) - newval);
		    _gtk_sheet_recalc_view_range(sheet);
		}
		else if (newval > (sheet->maxcol + 1))
		{
		    gtk_sheet_add_column(sheet, newval - (sheet->maxcol + 1));
		    _gtk_sheet_recalc_view_range(sheet);
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

	case PROP_GTK_SHEET_AUTO_RESIZE_ROWS:
	    gtk_sheet_set_autoresize_rows(sheet, g_value_get_boolean(value));
	    break;

	case PROP_GTK_SHEET_AUTO_RESIZE_COLUMNS:
	    gtk_sheet_set_autoresize_columns(sheet, g_value_get_boolean(value));
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
	    gtk_sheet_set_background(sheet, g_value_get_boxed(value));
	    break;

	case PROP_GTK_SHEET_GRID_VISIBLE:
	    gtk_sheet_show_grid(sheet, g_value_get_boolean(value));
	    break;

	case PROP_GTK_SHEET_GRID_COLOR:
	    gtk_sheet_set_grid(sheet, g_value_get_boxed(value));
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
	    gtk_sheet_set_column_titles_height(sheet, g_value_get_uint(value));
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
		GType entry_type = _gtk_sheet_entry_type_to_gtype(g_value_get_enum(value));

		sheet->entry_type = entry_type;  /* wanted entry type */
		gtk_sheet_change_entry(sheet, entry_type);
	    }
	    break;

	case PROP_GTK_SHEET_VJUST:
	    gtk_sheet_set_vjustification(sheet, g_value_get_enum(value));
	    break;

	default:
	    /* We don't have any other property... */
	    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	    break;
    }
    _gtk_sheet_range_draw(sheet, NULL, TRUE);

    /* this one will not work, it simply does noop
   gtk_widget_queue_draw(GTK_WIDGET(sheet));*/
}

static void
gtk_sheet_get_property(GObject    *object,
    guint       property_id,
    GValue     *value,
    GParamSpec *pspec)
{
    GtkSheet *sheet = GTK_SHEET(object);

    switch(property_id)
    {
	case PROP_GTK_SHEET_TITLE:
	    g_value_set_string(value, sheet->title);
	    break;

	case PROP_GTK_SHEET_DESCRIPTION:
	    g_value_set_string(value, sheet->description);
	    break;

	case PROP_GTK_SHEET_NROWS:
	    g_value_set_int(value, sheet->maxrow + 1);
	    break;

	case PROP_GTK_SHEET_NCOLS:
	    g_value_set_int(value, sheet->maxcol + 1);
	    break;

	case PROP_GTK_SHEET_LOCKED:
	    g_value_set_boolean(value, sheet->locked);
	    break;

	case PROP_GTK_SHEET_SELECTION_MODE:
	    g_value_set_enum(value, sheet->selection_mode);
	    break;

	case PROP_GTK_SHEET_AUTO_RESIZE:
	    g_value_set_boolean(value, gtk_sheet_autoresize(sheet));
	    break;

	case PROP_GTK_SHEET_AUTO_RESIZE_ROWS:
	    g_value_set_boolean(value, gtk_sheet_autoresize_rows(sheet));
	    break;

	case PROP_GTK_SHEET_AUTO_RESIZE_COLUMNS:
	    g_value_set_boolean(value, gtk_sheet_autoresize_columns(sheet));
	    break;

	case PROP_GTK_SHEET_AUTO_SCROLL:
	    g_value_set_boolean(value, sheet->autoscroll);
	    break;

	case PROP_GTK_SHEET_CLIP_TEXT:
	    g_value_set_boolean(value, sheet->clip_text);
	    break;

	case PROP_GTK_SHEET_JUSTIFY_ENTRY:
	    g_value_set_boolean(value, sheet->justify_entry);
	    break;

	case PROP_GTK_SHEET_BG_COLOR:
	    g_value_set_boxed(value, &sheet->bg_color);
	    break;

	case PROP_GTK_SHEET_GRID_VISIBLE:
	    g_value_set_boolean(value, sheet->show_grid);
	    break;

	case PROP_GTK_SHEET_GRID_COLOR:
	    g_value_set_boxed(value, &sheet->grid_color);
	    break;

	case PROP_GTK_SHEET_COLUMN_TITLES_VISIBLE:
	    g_value_set_boolean(value, sheet->column_titles_visible);
	    break;

	case PROP_GTK_SHEET_COLUMNS_RESIZABLE:
	    g_value_set_boolean(value, sheet->columns_resizable);
	    break;

	case PROP_GTK_SHEET_COLUMN_TITLES_HEIGHT:
	    g_value_set_uint(value, sheet->column_title_area.height);
	    break;

	case PROP_GTK_SHEET_ROW_TITLES_VISIBLE:
	    g_value_set_boolean(value, sheet->row_titles_visible);
	    break;

	case PROP_GTK_SHEET_ROWS_RESIZABLE:
	    g_value_set_boolean(value, sheet->rows_resizable);
	    break;

	case PROP_GTK_SHEET_ROW_TITLES_WIDTH:
	    g_value_set_uint(value, sheet->row_title_area.width);
	    break;

	case PROP_GTK_SHEET_ENTRY_TYPE:
	    g_value_set_enum(value, _gtk_sheet_entry_type_from_gtype(sheet->entry_type));
	    break;

	case PROP_GTK_SHEET_VJUST:
	    g_value_set_enum(value, sheet->vjust);
	    break;

	default:
	    /* We don't have any other property... */
	    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
	    break;
    }
}

static void
gtk_sheet_class_init_properties(GObjectClass *gobject_class)
{
    GParamSpec *pspec;

    gobject_class->set_property = gtk_sheet_set_property;
    gobject_class->get_property = gtk_sheet_get_property;

    /**
     * GtkSheet:title:
     *
     * The sheets title string
     */
    pspec = g_param_spec_string("title", "Sheet title",
	"The sheets title string",
	"GtkSheet" /* default value */,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_TITLE, pspec);

    /**
     * GtkSheet:description:
     *
     * The sheets description, a place to store further information 
     * for application use 
     *  
     */
    pspec = g_param_spec_string("description", "Sheet description",
	"The sheets description and further information for application use",
	"" /* default value */,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_DESCRIPTION, pspec);

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
    pspec = g_param_spec_int("n-rows", "Number of rows",
	"Number of rows in the sheet",
	0, 1000000, 0,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_NROWS, pspec);

    /**
     * GtkSheet:locked:
     *
     * If the sheet ist locked, it is not editable, cell contents
     * cannot be modified by the user.
     */
    pspec = g_param_spec_boolean("locked", "Locked",
	"If the sheet is locked, it is not editable, cell contents cannot be modified by the user",
	FALSE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_LOCKED, pspec);

    /**
     * GtkSheet:selection-mode:
     *
     * Sets the selection mode of the cells in a #GtkSheet
     */
    pspec = g_param_spec_enum("selection-mode", "Selection mode",
	"Sets the selection mode of the cells in a sheet",
	gtk_selection_mode_get_type(),
	GTK_SELECTION_BROWSE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_SELECTION_MODE, pspec);

#if 1
    /**
     * GtkSheet:autoresize:
     *
     * Autoreisize cells while typing (rows and columns) 
     *  
     * deprecated: 3.0:  use autoresize-rows, autoresize-columns 
     * instead 
     */
    pspec = g_param_spec_boolean("autoresize", "Autoresize cells",
	"Autoreisize rows and columns while typing",
	FALSE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
	PROP_GTK_SHEET_AUTO_RESIZE, pspec);
#endif

    /**
     * GtkSheet:autoresize-rows:
     *
     * Autoreisize rows while typing
     */
    pspec = g_param_spec_boolean("autoresize-rows", "Autoresize rows",
	"Autoreisize rows while typing",
	FALSE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
	PROP_GTK_SHEET_AUTO_RESIZE_ROWS, pspec);

    /**
     * GtkSheet:autoresize-cols:
     *
     * Autoreisize columns while typing
     */
    pspec = g_param_spec_boolean("autoresize-cols", "Autoresize cols",
	"Autoreisize columns while typing",
	FALSE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
	PROP_GTK_SHEET_AUTO_RESIZE_COLUMNS, pspec);

    /**
     * GtkSheet:autoscroll:
     *
     * The sheet will be automatically scrolled when you move beyond
     * the last visible row/column
     */
    pspec = g_param_spec_boolean("autoscroll", "Autoscroll sheet",
	"The sheet will be automatically scrolled when you move beyond the last row/column",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_AUTO_SCROLL, pspec);

    /**
     * GtkSheet:clip-text:
     *
     * Clip text in cells
     */
    pspec = g_param_spec_boolean("clip-text", "Clip cell text",
	"Clip text in cells",
	FALSE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_CLIP_TEXT, pspec);

    pspec = g_param_spec_boolean("justify-entry", "Justify cell entry",
	"Adapt cell entry editor to the cell justification",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_JUSTIFY_ENTRY, pspec);

    /**
     * GtkSheet:bgcolor:
     *
     * Background color of the sheet
     */
    pspec = g_param_spec_boxed("bgcolor", "Background color",
	"Background color of the sheet",
	GDK_TYPE_COLOR,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_BG_COLOR, pspec);

    /**
     * GtkSheet:grid-visible:
     *
     * Sets the visibility of grid
     */
    pspec = g_param_spec_boolean("grid-visible", "Grid visible",
	"Sets the visibility of grid",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_GRID_VISIBLE, pspec);

    /**
     * GtkSheet:grid-color:
     *
     * Color of the grid
     */
    pspec = g_param_spec_boxed("grid-color", "Grid color",
	"Color of the grid",
	GDK_TYPE_COLOR,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_GRID_COLOR, pspec);

    /**
     * GtkSheet:col-titles-visible:
     *
     * Visibility of the column titles
     */
    pspec = g_param_spec_boolean("col-titles-visible", "Column titles visible",
	"Visibility of the column titles",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_COLUMN_TITLES_VISIBLE, pspec);

    /**
     * GtkSheet:columns-resizable:
     *
     * Columns resizable
     */
    pspec = g_param_spec_boolean("columns-resizable", "Columns resizable",
	"Columns resizable",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_COLUMNS_RESIZABLE, pspec);

    /**
     * GtkSheet:col-titles-height:
     *
     * Height of the column titles
     */
    pspec = g_param_spec_uint("col-titles-height", "Column titles height",
	"Height of the column title area",
	0, 1024, GTK_SHEET_ROW_DEFAULT_HEIGHT,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_COLUMN_TITLES_HEIGHT, pspec);

    /**
     * GtkSheet:row-titles-visible:
     *
     * Row titles visible
     */
    pspec = g_param_spec_boolean("row-titles-visible", "Row titles visible",
	"Row titles visible",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_ROW_TITLES_VISIBLE, pspec);

    /**
     * GtkSheet:rows-resizable:
     *
     * Rows resizable
     */
    pspec = g_param_spec_boolean("rows-resizable", "Rows resizable",
	"Rows resizable",
	TRUE,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_ROWS_RESIZABLE, pspec);

    /**
     * GtkSheet:row-titles-width:
     *
     * Width of the row title area
     */
    pspec = g_param_spec_uint("row-titles-width", "Row titles width",
	"Width of the row title area",
	0, 2048, GTK_SHEET_COLUMN_DEFAULT_WIDTH,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class, PROP_GTK_SHEET_ROW_TITLES_WIDTH, pspec);

    /**
     * GtkSheet:entry-type:
     *
     * Sheet cell entry widget type
     */
    pspec = g_param_spec_enum("entry-type", "Entry Type",
	"Sheet entry type, if not default",
	gtk_sheet_entry_type_get_type(),
	GTK_SHEET_ENTRY_TYPE_DEFAULT,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
	PROP_GTK_SHEET_ENTRY_TYPE, pspec);

    /**
     * GtkSheet:vjust:
     *
     * Default vertical cell text justification
     */
    pspec = g_param_spec_enum("vjust", "Vertical justification",
	"Default sheet vertical cell text justification",
	gtk_sheet_vertical_justification_get_type(),
	GTK_SHEET_VERTICAL_JUSTIFICATION_TOP,
	G_PARAM_READWRITE);
    g_object_class_install_property(gobject_class,
	PROP_GTK_SHEET_VJUST, pspec);
}

#if GTK_SHEET_DEBUG_SIGNALS > 0

static void gtk_sheet_debug_select_row(GtkSheet *sheet, gint row)
{
    g_debug("SIGNAL select-row %p row %d", sheet, row);
}

static void gtk_sheet_debug_select_column(GtkSheet *sheet, gint column)
{
    g_debug("SIGNAL select-column %p col %d", sheet, column);
}

static void gtk_sheet_debug_select_range(GtkSheet *sheet, GtkSheetRange *range)
{
    g_debug("SIGNAL select-range %p {%d, %d, %d, %d}", sheet,
	range->row0, range->col0, range->rowi, range->coli);
}

static void gtk_sheet_debug_clip_range(GtkSheet *sheet, GtkSheetRange *range)
{
    g_debug("SIGNAL clip-range %p {%d, %d, %d, %d}", sheet,
	range->row0, range->col0, range->rowi, range->coli);
}

static void gtk_sheet_debug_resize_range(GtkSheet *sheet,
    GtkSheetRange *old_range, GtkSheetRange *new_range)
{
    g_debug("SIGNAL resize-range %p {%d, %d, %d, %d} -> {%d, %d, %d, %d}", sheet,
	old_range->row0, old_range->col0, old_range->rowi, old_range->coli,
	new_range->row0, new_range->col0, new_range->rowi, new_range->coli);
}

static void gtk_sheet_debug_move_range(GtkSheet *sheet,
    GtkSheetRange *old_range, GtkSheetRange *new_range)
{
    g_debug("SIGNAL move-range %p {%d, %d, %d, %d} -> {%d, %d, %d, %d}", sheet,
	old_range->row0, old_range->col0, old_range->rowi, old_range->coli,
	new_range->row0, new_range->col0, new_range->rowi, new_range->coli);
}

static gboolean gtk_sheet_debug_traverse(GtkSheet *sheet,
    gint row, gint column, gint *new_row, gint *new_column)
{
    g_debug("SIGNAL traverse %p row %d col %d nrow %d ncol %d", sheet,
	row, column, *new_row, *new_column);
    return (TRUE);
}

static gboolean gtk_sheet_debug_deactivate(GtkSheet *sheet, gint row, gint column)
{
    g_debug("SIGNAL deactivate %p row %d col %d", sheet, row, column);
    return (TRUE);
}

static gboolean gtk_sheet_debug_activate(GtkSheet *sheet, gint row, gint column)
{
    g_debug("SIGNAL activate %p row %d col %d", sheet, row, column);
    return (TRUE);
}

static void gtk_sheet_debug_set_cell(GtkSheet *sheet, gint row, gint column)
{
    g_debug("SIGNAL set-cell %p row %d col %d", sheet, row, column);
}

static void gtk_sheet_debug_clear_cell(GtkSheet *sheet, gint row, gint column)
{
    g_debug("SIGNAL clear-cell %p row %d col %d", sheet, row, column);
}

#   if 0
static void gtk_sheet_debug_changed(GtkSheet *sheet, gint row, gint column)
{
    g_debug("SIGNAL changed %p row %d col %d", sheet, row, column);
}
#   endif

static void gtk_sheet_debug_new_column_width(GtkSheet *sheet, gint col, guint width)
{
    g_debug("SIGNAL new-column-width %p col %d width %d", sheet, col, width);
}

static void gtk_sheet_debug_new_row_height(GtkSheet *sheet, gint row, guint height)
{
    g_debug("SIGNAL new-row-height %p row %d height %d", sheet, row, height);
}

static gboolean gtk_sheet_debug_focus_in_event(GtkSheet *sheet, GdkEventFocus *event)
{
    g_debug("SIGNAL focus-in-event %p event %p", sheet, event);
    return (FALSE);
}

static gboolean gtk_sheet_debug_focus_out_event(GtkSheet *sheet, GdkEventFocus *event)
{
    g_debug("SIGNAL focus-out-event %p event %p", sheet, event);
    return (FALSE);
}

#endif

static void
_gtk_sheet_class_init_signals(GtkObjectClass *object_class,
    GtkWidgetClass *widget_class)
{
    /**
     * GtkSheet::select-row:
     * @sheet: the sheet widget that emitted the signal
     * @row: the newly selected row index
     *
     * Emmited when a row has been selected.
     */
    sheet_signals[SELECT_ROW] =
	g_signal_new("select-row",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, select_row),
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
	g_signal_new("select-column",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, select_column),
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
	g_signal_new("select-range",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, select_range),
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
	g_signal_new("clip-range",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, clip_range),
	NULL, NULL,
	gtkextra_VOID__BOXED,
	G_TYPE_NONE, 1, G_TYPE_SHEET_RANGE);

    /**
    * GtkSheet::resize-range:
    * @sheet: the sheet widget that emitted the signal
    * @old_range: the previous selected #GtkSheetRange.
    * @new_range: the newly selected #GtkSheetRange.
    *
    * Emmited when a #GtkSheetRange is resized.
    */
    sheet_signals[RESIZE_RANGE] =
	g_signal_new("resize-range",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, resize_range),
	NULL, NULL,
	gtkextra_VOID__BOXED_BOXED,
	G_TYPE_NONE, 2, G_TYPE_SHEET_RANGE, G_TYPE_SHEET_RANGE);

    /**
     * GtkSheet::move-range:
     * @sheet: the sheet widget that emitted the signal.
     * @old_range: the previous selected #GtkSheetRange.
     * @new_range: the newly selected #GtkSheetRange.
     *
     * Emmited when a #GtkSheetRange is moved.
     */
    sheet_signals[MOVE_RANGE] =
	g_signal_new("move-range",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, move_range),
	NULL, NULL,
	gtkextra_VOID__BOXED_BOXED,
	G_TYPE_NONE, 2, G_TYPE_SHEET_RANGE, G_TYPE_SHEET_RANGE);

    /**
     * GtkSheet::traverse:
     * @sheet: the sheet widget that emitted the signal.
     * @row: row number of old cell
     * @column: column number of old cell
     * @*new_row: row number of target cell, changeable
     * @*new_column: column number of target cell, changeable
     *
     * The "traverse" is emited before "deactivate" and allows to 
     * veto the movement. In such case, the entry will remain in the
     * site and the other signals will not be emited. 
     *  
     * The signal handler must return TRUE to allow the movement, 
     * FALSE to veto the movement.
     */
    sheet_signals[TRAVERSE] =
	g_signal_new("traverse",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, traverse),
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
     * Emmited whenever a cell is deactivated(you click on other 
     * cell or start a new selection). 
     *  
     * The signal handler must return TRUE in order to allow 
     * deactivation, FALSE to deny deactivation. 
     */
    sheet_signals[DEACTIVATE] =
	g_signal_new("deactivate",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, deactivate),
	NULL, NULL,
	gtkextra_BOOLEAN__INT_INT,
	G_TYPE_BOOLEAN, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::activate:
     * @sheet: the sheet widget that emitted the signal
     * @row: row number of activated cell.
     * @column: column number of activated cell.
     *
     * Emmited whenever a cell is activated(you click on it). 
     *  
     * FIXME:: The return value is ignored (not yet implemented).
     */
    sheet_signals[ACTIVATE] =
	g_signal_new("activate",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, activate),
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
	g_signal_new("set-cell",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, set_cell),
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
	g_signal_new("clear-cell",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, clear_cell),
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
	g_signal_new("changed",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, changed),
	NULL, NULL,
	gtkextra_VOID__INT_INT,
	G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::new-column-width:
     * @sheet: the sheet widget that emitted the signal
     * @col: modified column number.
     * @width: new column width
     *
     * Emited when the width of a column is modified.
     */
    sheet_signals[NEW_COL_WIDTH] =
	g_signal_new("new-column-width",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, changed),
	NULL, NULL,
	gtkextra_VOID__INT_INT,
	G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::new-row-height:
     * @sheet: the sheet widget that emitted the signal
     * @row: modified row number.
     * @height: new row height.
     *
     * Emited when the height of a row is modified.
     */
    sheet_signals[NEW_ROW_HEIGHT] =
	g_signal_new("new-row-height",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, changed),
	NULL, NULL,
	gtkextra_VOID__INT_INT,
	G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GtkSheet::entry-focus-in:
     * @sheet: the sheet widget that emitted the signal
     * @event: the #GdkEventFocus which triggered this signal
     *
     * The ::entry-focus-in signal will be emitted when the keyboard 
     * focus enters the sheet_entry editor. 
     *
     * Returns: %TRUE to stop other handlers from being invoked for the event.
     *   %FALSE to propagate the event further.
     *  
     * Since: 3.0.1
     */
    sheet_signals[ENTRY_FOCUS_IN] =
	g_signal_new("entry-focus-in",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, focus_in_event),
	NULL, NULL,
	gtkextra_BOOLEAN__BOXED,
	G_TYPE_BOOLEAN, 1,
	GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);

    /**
     * GtkSheet::entry-focus-out:
     * @sheet: the sheet widget that emitted the signal
     * @event: the #GdkEventFocus which triggered this signal
     *
     * The ::entry-focus-out signal will be emitted when the 
     * keyboard focus leaves the sheet_entry editor. 
     *
     * Returns: %TRUE to stop other handlers from being invoked for the event.
     *   %FALSE to propagate the event further.
     *  
     * Since: 3.0.1
     */
    sheet_signals[ENTRY_FOCUS_OUT] =
	g_signal_new("entry-focus-out",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, focus_out_event),
	NULL, NULL,
	gtkextra_BOOLEAN__BOXED,
	G_TYPE_BOOLEAN, 1,
	GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);

    /**
     * GtkSheet::populate-popup:
     * @sheet: the sheet widget that emitted the signal
     * @menu: the menu that ist being populated 
     *
     * The ::populate-popup signal will be emitted when the user 
     * activates the popup menu of the sheet_entry editor. 
     *  
     * The emission of this signal is only supported for #GtkEntry, 
     * #GtkDataEntry, #GtkItemEntry and #GtkTextView. 
     *  
     * Since: 3.0.1
     */
    sheet_signals[ENTRY_POPULATE_POPUP] =
	g_signal_new("populate-popup",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	0,
	NULL, NULL,
	gtkextra_VOID__OBJECT,
	G_TYPE_NONE, 1,
	gtk_menu_get_type());


    /**
     * GtkSheet::set-scroll-adjustments:
     * @sheet: the sheet widget that emitted the signal
     * @hadjustment: horizontal #GtkAdjustment.
     * @vadjustment: vertical #GtkAdkjustment.
     *
     * Emited when scroll adjustments are set.
     */
    widget_class->set_scroll_adjustments_signal =
	g_signal_new("set-scroll-adjustments",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, set_scroll_adjustments),
	NULL, NULL,
	gtkextra_VOID__OBJECT_OBJECT,
	G_TYPE_NONE, 2, gtk_adjustment_get_type(),
	gtk_adjustment_get_type());

    /**
     * GtkSheet::move-cursor:
     * @sheet: the sheet widget that emitted the signal
     * @step: the granularity of the move, as a #GtkMovementStep
     * @count: the number of @step units to move
     * @extend_selection: %TRUE if the move should extend the selection
     *  
     *  The ::move-cursor signal is a keybinding signal which gets
     *  emitted when the user initiates a cursor movement.
     *  
     *  Applications should not connect to it, but may emit it with
     * g_signal_emit_by_name() if they need to control the cursor 
     * programmatically. 
     *  
     * Since: 3.0.2
     */
    sheet_signals[MOVE_CURSOR] =
	g_signal_new("move-cursor",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	G_STRUCT_OFFSET(GtkSheetClass, move_cursor),
	NULL, NULL,
	gtkextra_VOID__ENUM_INT_BOOLEAN,
	G_TYPE_NONE, 3,
	GTK_TYPE_MOVEMENT_STEP,
	G_TYPE_INT,
	G_TYPE_BOOLEAN);

    /**
     * GtkSheet::enter-pressed:
     * @sheet: the sheet widget that emitted the signal
     * @event: (type Gdk.EventKey): the #GdkEventKey which triggered this signal.
     *
     * This signal intercepts RETURN and ENTER key-press-events 
     * before they are processed by the sheet-entry editor. Any 
     * modifier combinations on these keys may trigger the signal.
     *  
     * The default behaviour of the sheet-entry editor is to move 
     * the active cell, which might not be appropriate for the type 
     * of application. 
     *  
     * Returns: %TRUE to block the sheet-entry from processing 
     *   the event. %FALSE to propagate the event to the
     *   sheet-entry.
     */
    sheet_signals[ENTER_PRESSED] =
	g_signal_new("enter-pressed",
	G_TYPE_FROM_CLASS(object_class),
	G_SIGNAL_RUN_LAST,
	0,
	NULL, NULL,
	gtkextra_BOOLEAN__BOXED,
	G_TYPE_BOOLEAN, 1, GDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);

}

static void
_add_binding_ext(GtkBindingSet *binding_set,
    gint key, gint alt_key, GdkModifierType mods,
    GtkMovementStep step, gint count,
    gboolean extend_selection)
{
    gtk_binding_entry_remove(binding_set, key, mods);

    gtk_binding_entry_add_signal(binding_set,
	key, mods,
	"move-cursor", 3,
	G_TYPE_ENUM, step,
	G_TYPE_INT, count,
	G_TYPE_BOOLEAN, extend_selection);
    if (alt_key)
    {
	gtk_binding_entry_remove(binding_set, alt_key, mods);

	gtk_binding_entry_add_signal(binding_set,
	    alt_key, mods,
	    "move-cursor", 3,
	    G_TYPE_ENUM, step,
	    G_TYPE_INT, count,
	    G_TYPE_BOOLEAN, extend_selection);
    }
}

static void
_add_mod_binding(GtkBindingSet *binding_set,
    gint key, gint alt_key, GdkModifierType mods,
    GtkMovementStep step, gint count)
{
    _add_binding_ext(binding_set,
	key, alt_key, GTK_SHEET_MOD_MASK | mods,
	step, count,
	FALSE);

    _add_binding_ext(binding_set,
	key, alt_key, GTK_SHEET_MOD_MASK | GDK_SHIFT_MASK | mods,
	step, count,
	TRUE);
}

static void
_add_normal_binding(GtkBindingSet *binding_set,
    gint key, gint alt_key,
    GdkModifierType mods, GtkMovementStep step, gint count)
{
    _add_binding_ext(binding_set,
	key, alt_key, mods,
	step, count, FALSE);

    _add_binding_ext(binding_set,
	key, alt_key, GDK_SHIFT_MASK | mods,
	step, count, TRUE);
}

static void
_gtk_sheet_class_init_tab_bindings(GtkSheetClass *klass, GtkDirectionType dir)
{
    GtkBindingSet *b = gtk_binding_set_by_class(klass);
    GtkDirectionType prim_forw, prim_back, sec_forw, sec_back;

    switch(dir)
    {
	case GTK_DIR_TAB_FORWARD:
	case GTK_DIR_RIGHT:
	    prim_forw = GTK_DIR_TAB_FORWARD;
	    prim_back = GTK_DIR_TAB_BACKWARD;
	    sec_forw = GTK_DIR_DOWN;
	    sec_back = GTK_DIR_UP;
	    break;
	case GTK_DIR_TAB_BACKWARD:
	case GTK_DIR_LEFT:
	    prim_forw = GTK_DIR_TAB_BACKWARD;
	    prim_back = GTK_DIR_TAB_FORWARD;
	    sec_forw = GTK_DIR_UP;
	    sec_back = GTK_DIR_DOWN;
	    break;
	case GTK_DIR_UP:
	    prim_forw = GTK_DIR_UP;
	    prim_back = GTK_DIR_DOWN;
	    sec_forw = GTK_DIR_TAB_BACKWARD;
	    sec_back = GTK_DIR_TAB_FORWARD;
	    break;
	case GTK_DIR_DOWN:
	    prim_forw = GTK_DIR_DOWN;
	    prim_back = GTK_DIR_UP;
	    sec_forw = GTK_DIR_TAB_FORWARD;
	    sec_back = GTK_DIR_TAB_BACKWARD;
	    break;
    }

    /* Tab / Backtab (Normal) */
    _add_binding_ext(b, GDK_KEY_Tab, 0,
	0,
	GTK_MOVEMENT_LOGICAL_POSITIONS, prim_forw,
	FALSE);

    _add_binding_ext(b, GDK_KEY_ISO_Left_Tab, 0,
	GDK_SHIFT_MASK,
	GTK_MOVEMENT_LOGICAL_POSITIONS, prim_back,
	FALSE);

    /* Tab / Backtab (Mod-Ctrl) */
    _add_binding_ext(b, GDK_KEY_Tab, 0,
	GTK_SHEET_MOD_MASK | GDK_CONTROL_MASK,
	GTK_MOVEMENT_LOGICAL_POSITIONS, sec_forw,
	FALSE);

    _add_binding_ext(b, GDK_KEY_ISO_Left_Tab, 0,
	GTK_SHEET_MOD_MASK | GDK_CONTROL_MASK | GDK_SHIFT_MASK,
	GTK_MOVEMENT_LOGICAL_POSITIONS, sec_back,
	FALSE);

#if 1
    /* Return / Enter (Normal) */
    _add_binding_ext(b, GDK_KEY_Return, GDK_KEY_KP_Enter,
	0,
	GTK_MOVEMENT_LOGICAL_POSITIONS, prim_forw,
	FALSE);

    _add_binding_ext(b, GDK_KEY_Return, GDK_KEY_KP_Enter,
	GDK_SHIFT_MASK,
	GTK_MOVEMENT_LOGICAL_POSITIONS, prim_back,
	FALSE);

    /* Return / Enter (Mod-Ctrl) */
    _add_binding_ext(b, GDK_KEY_Return, GDK_KEY_KP_Enter,
	GTK_SHEET_MOD_MASK | GDK_CONTROL_MASK,
	GTK_MOVEMENT_LOGICAL_POSITIONS, sec_forw,
	FALSE);

    _add_binding_ext(b, GDK_KEY_Return, GDK_KEY_KP_Enter,
	GTK_SHEET_MOD_MASK | GDK_CONTROL_MASK | GDK_SHIFT_MASK,
	GTK_MOVEMENT_LOGICAL_POSITIONS, sec_back,
	FALSE);
#endif
}

static void
_gtk_sheet_class_init_bindings(GtkSheetClass *klass)
{
    GtkBindingSet *b = gtk_binding_set_by_class(klass);

    /* Up/Down (Normal) */
    _add_normal_binding(b, GDK_KEY_Up, GDK_KEY_KP_Up,
	0, GTK_MOVEMENT_DISPLAY_LINES, -1);

    _add_normal_binding(b, GDK_KEY_Down, GDK_KEY_KP_Down,
	0, GTK_MOVEMENT_DISPLAY_LINES, 1);

    /* Up / Down (Mod) */
    _add_mod_binding(b, GDK_KEY_Up, GDK_KEY_KP_Up,
	0, GTK_MOVEMENT_DISPLAY_LINES, -1);

    _add_mod_binding(b, GDK_KEY_Down, GDK_KEY_KP_Down,
	0, GTK_MOVEMENT_DISPLAY_LINES, 1);

    /* Up / Down (Mod-Ctrl) */
    _add_mod_binding(b, GDK_KEY_Up, GDK_KEY_KP_Up,
	GDK_CONTROL_MASK, GTK_MOVEMENT_PAGES, -1);

    _add_mod_binding(b, GDK_KEY_Down, GDK_KEY_KP_Down,
	GDK_CONTROL_MASK, GTK_MOVEMENT_PAGES, 1);

    /* PgUp / PgDn (Normal) */
    _add_normal_binding(b, GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up,
	0, GTK_MOVEMENT_PAGES, -1);

    _add_normal_binding(b, GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down,
	0, GTK_MOVEMENT_PAGES, 1);

    /* PgUp / PgDn (Mod) */
    _add_mod_binding(b, GDK_KEY_Page_Up, GDK_KEY_KP_Page_Up,
	0, GTK_MOVEMENT_PAGES, -1);

    _add_mod_binding(b, GDK_KEY_Page_Down, GDK_KEY_KP_Page_Down,
	0, GTK_MOVEMENT_PAGES, 1);

    /* Left / Right (Mod) */
    _add_mod_binding(b, GDK_KEY_Left, GDK_KEY_KP_Left,
	0, GTK_MOVEMENT_VISUAL_POSITIONS, -1);

    _add_mod_binding(b, GDK_KEY_Right, GDK_KEY_KP_Right,
	0, GTK_MOVEMENT_VISUAL_POSITIONS, 1);

    /* Left / Right (Mod-Ctrl) */
    _add_mod_binding(b, GDK_KEY_Left, GDK_KEY_KP_Left,
	GDK_CONTROL_MASK, GTK_MOVEMENT_HORIZONTAL_PAGES, -1);

    _add_mod_binding(b, GDK_KEY_Right, GDK_KEY_KP_Right,
	GDK_CONTROL_MASK, GTK_MOVEMENT_HORIZONTAL_PAGES, 1);

    /* Home / End (Mod) */
    _add_mod_binding(b, GDK_KEY_Home, GDK_KEY_KP_Home,
	0, GTK_MOVEMENT_DISPLAY_LINE_ENDS, -1);

    _add_mod_binding(b, GDK_KEY_End, GDK_KEY_KP_End,
	0, GTK_MOVEMENT_DISPLAY_LINE_ENDS, 1);

    /* Home / End (Mod-Ctrl) */
    _add_mod_binding(b, GDK_KEY_Home, GDK_KEY_KP_Home,
	GDK_CONTROL_MASK, GTK_MOVEMENT_BUFFER_ENDS, -1);

    _add_mod_binding(b, GDK_KEY_End, GDK_KEY_KP_End,
	GDK_CONTROL_MASK, GTK_MOVEMENT_BUFFER_ENDS, 1);

    /* Tab, Return, Enter */
    _gtk_sheet_class_init_tab_bindings(klass, GTK_DIR_TAB_FORWARD);
}



/**
 * _gtk_sheet_binding_filter: 
 * @sheet:  The #GtkSheet
 * @key:    The keypress event 
 *  
 * keypress binding filter
 * 
 * Some keypress events (with no MODS) are very convenient for the user to navigate the sheet may be processed by a sheet_entry itself. For example the Up/Down keys are very handy to navigate from row to row are used by GtkTextView to move Up/Down within a multi line text.
 * 
 * In order to get most out of both worlds, we generally allow useful bindings for all sheet_entry types and filter out useage collisions depending on the type of the sheet_entry.
 * 
 * Returns: TRUE to activate the binding on the sheet, FALSE to 
 *  	   let the sheet_entry process the keypress
 */
static gboolean _gtk_sheet_binding_filter(GtkSheet *sheet,
    GdkEventKey *key)
{
    if (key->state & GTK_SHEET_MOD_MASK)
	return (TRUE);

    if ( GTK_IS_DATA_TEXT_VIEW(sheet->sheet_entry)
	|| GTK_IS_TEXT_VIEW(sheet->sheet_entry) )
    {
	switch(key->keyval)
	{
	    case GDK_KEY_Return:
	    case GDK_KEY_Up:
	    case GDK_KEY_Down:
	    case GDK_KEY_Page_Up:
	    case GDK_KEY_Page_Down:
		return (FALSE);

	    default:
		break;
	}
    }
    return (TRUE);
}

/*
 * gtk_sheet_class_init - GtkSheet class initialisation
 *
 * @param klass
 */

static void
gtk_sheet_class_init(GtkSheetClass *klass)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    object_class = (GtkObjectClass *)klass;
    widget_class = (GtkWidgetClass *)klass;
    container_class = (GtkContainerClass *)klass;

    sheet_parent_class = g_type_class_peek_parent(klass);

    _gtk_sheet_class_init_signals(object_class, widget_class);
    _gtk_sheet_class_init_bindings(klass);

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
    widget_class->focus = gtk_sheet_focus;
    widget_class->focus_in_event = NULL;
    widget_class->focus_out_event = NULL;

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
    klass->new_column_width = NULL;
    klass->new_row_height = NULL;
    klass->focus_in_event = NULL;
    klass->focus_out_event = NULL;

#if GTK_SHEET_DEBUG_SIGNALS > 0
    klass->select_row = gtk_sheet_debug_select_row;
    klass->select_column = gtk_sheet_debug_select_column;
    klass->select_range = gtk_sheet_debug_select_range;
    klass->clip_range = gtk_sheet_debug_clip_range;
    klass->resize_range = gtk_sheet_debug_resize_range;
    klass->move_range = gtk_sheet_debug_move_range;
    klass->traverse = gtk_sheet_debug_traverse;
    klass->deactivate = gtk_sheet_debug_deactivate;
    klass->activate = gtk_sheet_debug_activate;
    klass->set_cell = gtk_sheet_debug_set_cell;
    klass->clear_cell = gtk_sheet_debug_clear_cell;
    /*klass->changed = gtk_sheet_debug_changed;*/
    klass->new_column_width = gtk_sheet_debug_new_column_width;
    klass->new_row_height = gtk_sheet_debug_new_row_height;
    klass->focus_in_event = gtk_sheet_debug_focus_in_event;
    klass->focus_out_event = gtk_sheet_debug_focus_out_event;
#endif

    klass->set_scroll_adjustments = gtk_sheet_set_scroll_adjustments;
    klass->move_cursor = _gtk_sheet_move_cursor;
}

static void
gtk_sheet_init(GtkSheet *sheet)
{
    sheet->flags = 0;
    sheet->selection_mode = GTK_SELECTION_BROWSE;
    sheet->autoresize_columns = FALSE;
    sheet->autoresize_rows = FALSE;
    sheet->autoscroll = TRUE;
    sheet->clip_text = FALSE;
    sheet->justify_entry = TRUE;
    sheet->locked = FALSE;

    sheet->freeze_count = 0;

#if GTK_SHEET_DEBUG_COLORS > 0
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
    sheet->entry_type = G_TYPE_NONE;
    sheet->installed_entry_type = G_TYPE_NONE;

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

    sheet->pixmap = NULL;

    sheet->hoffset = 0;
    sheet->voffset = 0;

    sheet->old_hadjustment = 0;
    sheet->old_vadjustment = 0;

    sheet->hadjustment = NULL;
    sheet->vadjustment = NULL;

    sheet->shadow_type =   GTK_SHADOW_NONE;
    sheet->vjust = GTK_SHEET_VERTICAL_JUSTIFICATION_TOP;

    sheet->column_title_area.x = 0;
    sheet->column_title_area.y = 0;
    sheet->column_title_area.width = 0;
    sheet->column_title_area.height = _gtk_sheet_row_default_height(GTK_WIDGET(sheet));
    sheet->column_title_window = NULL;
    sheet->column_titles_visible = TRUE;

    sheet->row_title_window = NULL;
    sheet->row_title_area.x = 0;
    sheet->row_title_area.y = 0;
    sheet->row_title_area.width = GTK_SHEET_COLUMN_DEFAULT_WIDTH;
    sheet->row_title_area.height = 0;
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

    gtk_widget_set_has_window(GTK_WIDGET(sheet), TRUE);
    gtk_widget_set_can_focus(GTK_WIDGET(sheet), TRUE);

    /* for glade to be able to construct the object, we need to complete initialisation here */
    gtk_sheet_construct(sheet, 0, 0, "GtkSheet");

    g_signal_connect(G_OBJECT(sheet),
	"query-tooltip",
	(void *)gtk_sheet_query_tooltip_handler,
	NULL);

    /* make sure the tooltip signal fires even if the sheet itself has no tooltip */
    gtk_widget_set_has_tooltip(GTK_WIDGET(sheet), TRUE);
}


static void
_gtk_sheet_row_init(GtkSheetRow *row)
{
    row->name = NULL;
    row->height = GTK_SHEET_ROW_DEFAULT_HEIGHT;
    row->requisition = GTK_SHEET_ROW_DEFAULT_HEIGHT;
    row->top_ypixel = 0;
    row->max_extent_height = 0;

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
gtk_sheet_new(guint rows, guint columns, const gchar *title)
{
    GtkWidget *widget;

    /* sanity check */
    g_return_val_if_fail(columns >= MINCOLS, NULL);
    g_return_val_if_fail(rows >= MINROWS, NULL);

    widget = gtk_widget_new(gtk_sheet_get_type(), NULL);

    gtk_sheet_construct(GTK_SHEET(widget), rows, columns, title);

    return (widget);
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
gtk_sheet_construct(GtkSheet *sheet, guint rows, guint columns, const gchar *title)
{
    sheet->data = (GtkSheetCell ***)g_malloc(sizeof(GtkSheetCell **));

    sheet->data[0] = (GtkSheetCell **)g_malloc(sizeof(GtkSheetCell *)+sizeof(gdouble));
    sheet->data[0][0] = NULL;

    /* set number of rows and columns */
    GrowSheet(sheet, MINROWS, MINCOLS);

    /* Init heading row/column, add normal rows/columns */
    AddRows(sheet, sheet->maxrow + 1, rows);
    AddColumns(sheet, sheet->maxcol + 1, columns);

    /* create sheet entry */
    create_sheet_entry(sheet, G_TYPE_NONE);

    /* create global selection button */
    create_global_button(sheet);

    if (title)
    {
	if (sheet->title)
	    g_free(sheet->title);
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

    widget = gtk_widget_new(gtk_sheet_get_type(), NULL);

    gtk_sheet_construct_browser(GTK_SHEET(widget), rows, columns, title);

    return (widget);
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
gtk_sheet_new_with_custom_entry(guint rows, guint columns, const gchar *title,
    GType entry_type)
{
    GtkWidget *widget;

    widget = gtk_widget_new(gtk_sheet_get_type(), NULL);

    gtk_sheet_construct_with_custom_entry(GTK_SHEET(widget),
	rows, columns, title,
	entry_type ? entry_type : G_TYPE_NONE);

    return (widget);
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
gtk_sheet_construct_with_custom_entry(GtkSheet *sheet,
    guint rows, guint columns,
    const gchar *title,
    GType entry_type)
{
    gtk_sheet_construct(sheet, rows, columns, title);

    create_sheet_entry(sheet, entry_type ? entry_type : G_TYPE_NONE);
}

/**
 * gtk_sheet_change_entry:
 * @sheet: a #GtkSheet
 * @entry_type: a #GType
 *
 * Changes the current entry of the cell in #GtkSheet. The old 
 * sheet entry widget gets dropped and a new entry widget is 
 * created. Beware: You will have to reconnect all your signal 
 * handlers after changing an entry.
 */
void
gtk_sheet_change_entry(GtkSheet *sheet, const GType entry_type)
{
    gint state;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    state = sheet->state;

    if (state == GTK_SHEET_NORMAL)
	_gtk_sheet_hide_active_cell(sheet);

    create_sheet_entry(sheet, entry_type ? entry_type : G_TYPE_NONE);

    sheet->entry_type = entry_type;  /* save wanted type, no matter wether it failed */

    if (state == GTK_SHEET_NORMAL)
    {
	gtk_sheet_show_active_cell(sheet);

	/* if the application changes the entry during emission of the TRAVERSE signal
	   an initialisation of the new entry can fire the "changed" signal and
	   the text will be written into the old active cell (which is WRONG)
	 
	   gtk_sheet_entry_signal_connect_changed(sheet, 
	       G_CALLBACK(gtk_sheet_entry_changed_handler));
	   */
    }
}

/**
 * gtk_sheet_show_grid:
 * @sheet: a #GtkSheet
 * @show: TRUE(grid visible) or FALSE(grid invisible)
 *
 * Sets the visibility of grid in #GtkSheet.
 */
void
gtk_sheet_show_grid(GtkSheet *sheet, gboolean show)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (show == sheet->show_grid)
	return;

    sheet->show_grid = show;

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
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
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    return (sheet->show_grid);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!color)
    {
	gdk_color_parse(GTK_SHEET_DEFAULT_BG_COLOR, &sheet->bg_color);
    }
    else
    {
	sheet->bg_color = *color;
    }
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->bg_color, FALSE, TRUE);

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!color)
    {
	gdk_color_parse(GTK_SHEET_DEFAULT_GRID_COLOR, &sheet->grid_color);
    }
    else
    {
	sheet->grid_color = *color;
    }
    gdk_colormap_alloc_color(gdk_colormap_get_system(), &sheet->grid_color, FALSE, TRUE);

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
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
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    return (sheet->maxrow + 1);
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
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    return (sheet->maxcol + 1);
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
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    return (sheet->state);
}

/**
 * gtk_sheet_get_selection:
 * @sheet: a #GtkSheet 
 * @state: where to store the #GtkSheetState, may be NULL 
 * @range: where to store the #GtkSheetRange
 *  
 * Inquire current cell selection state and range. 
 *  
 * Returns: TRUE: there is a selection, FALSE: no selection or error 
 */
gboolean
gtk_sheet_get_selection(GtkSheet *sheet, GtkSheetState *state, GtkSheetRange *range)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);
    g_return_val_if_fail(range != NULL, FALSE);

    if (state)
	*state = sheet->state;

    *range = sheet->range;

    return (TRUE);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    gtk_sheet_real_unselect_range(sheet, NULL);

    sheet->selection_mode = mode;
}

/**
 * gtk_sheet_set_autoresize:
 * @sheet: a #GtkSheet
 * @autoresize: TRUE or FALSE
 *
 * Controls wether cells will be autoresized upon deactivation, 
 * as you type text or set a cell_text value. If you want the 
 * cells to be autoresized when you pack widgets look at 
 * gtk_sheet_attach_*(). This function sets both: 
 * autoresize_columns and autoresize_cells. 
 */
void
gtk_sheet_set_autoresize(GtkSheet *sheet, gboolean autoresize)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->autoresize_columns = autoresize;
    sheet->autoresize_rows = autoresize;
}

/**
 * gtk_sheet_set_autoresize_columns:
 * @sheet: a #GtkSheet
 * @autoresize: TRUE or FALSE
 *
 * Controls wether columns will be autoresized upon 
 * deactivation, as you type text or set a cell_text value. If 
 * you want the cells to be autoresized when you pack widgets 
 * look at gtk_sheet_attach_*(). 
 */
void
gtk_sheet_set_autoresize_columns(GtkSheet *sheet, gboolean autoresize)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->autoresize_columns = autoresize;
}

/**
 * gtk_sheet_set_autoresize_rows:
 * @sheet: a #GtkSheet
 * @autoresize: TRUE or FALSE
 *
 * Controls wether rows will be autoresized upon deactivation,
 * as you type text or set a cell_text value. If you want the 
 * cells to be autoresized when you pack widgets look at 
 * gtk_sheet_attach_*(). 
 */
void
gtk_sheet_set_autoresize_rows(GtkSheet *sheet, gboolean autoresize)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->autoresize_rows = autoresize;
}


/**
 * gtk_sheet_autoresize:
 * @sheet: a #GtkSheet
 *
 * Gets the autoresize mode of #GtkSheet.
 *
 * Returns: TRUE if autoresize_columns or autoresize_rows was 
 * set, or FALSE if none
 */
gboolean
gtk_sheet_autoresize(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->autoresize_columns || sheet->autoresize_rows);
}

/**
 * gtk_sheet_autoresize_columns:
 * @sheet: a #GtkSheet
 *
 * Gets the autoresize mode for #GtkSheet columns.
 *
 * Returns: TRUE or FALSE
 */
gboolean
gtk_sheet_autoresize_columns(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->autoresize_columns);
}

/**
 * gtk_sheet_autoresize_rows:
 * @sheet: a #GtkSheet
 *
 * Gets the autoresize mode for #GtkSheet rows.
 *
 * Returns: TRUE or FALSE
 */
gboolean
gtk_sheet_autoresize_rows(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->autoresize_rows);
}

/**
 * _gtk_sheet_recalc_extent_width:
 * @sheet:  the #GtkSheet 
 * @col:    column to be recalculated
 *  
 * recalc maximum column extent width
 */
static void _gtk_sheet_recalc_extent_width(GtkSheet *sheet, gint col)
{
    gint new_width = 0;
    gint row;

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_recalc_extent_width[%d]: called", col);
#endif

    if (col < 0 || col > sheet->maxalloccol || col > sheet->maxcol)
	return;

    for (row = 0; row <= sheet->maxallocrow; row++)
    {
	GtkSheetRow *rowptr = ROWPTR(sheet, row);

	if (GTK_SHEET_ROW_IS_VISIBLE(rowptr))
	{
	    GtkSheetCell *cell = sheet->data[row][col];

	    if (cell && cell->text && cell->text[0])
	    {
		GtkSheetCellAttr attributes;
		gtk_sheet_get_attributes(sheet, row, col, &attributes);

		if (attributes.is_visible)
		{
		    if (cell->extent.width > new_width)
		    {
			new_width = cell->extent.width;
		    }
		}
	    }
	}
    }
    COLPTR(sheet, col)->max_extent_width = new_width;
}

/**
 * _gtk_sheet_recalc_extent_height:
 * @sheet:  the #GtkSheet 
 * @row:    row to be recalculated
 *  
 * recalc maximum row extent height
 */
static void _gtk_sheet_recalc_extent_height(GtkSheet *sheet, gint row)
{
    gint new_height = 0;
    gint col;

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_recalc_extent_height[%d]: called", row);
#endif

    if (row < 0 || row > sheet->maxallocrow || row > sheet->maxrow)
	return;

    for (col = 0; col <= sheet->maxalloccol; col++)
    {
	GtkSheetColumn *colptr = COLPTR(sheet, col);

	if (GTK_SHEET_COLUMN_IS_VISIBLE(colptr))
	{
	    GtkSheetCell *cell = sheet->data[row][col];

	    if (cell && cell->text && cell->text[0])
	    {
		GtkSheetCellAttr attributes;
		gtk_sheet_get_attributes(sheet, row, col, &attributes);

		if (attributes.is_visible)
		{
		    if (cell->extent.height > new_height)
		    {
			new_height = cell->extent.height;
		    }
		}
	    }
	}
    }

    ROWPTR(sheet, row)->max_extent_height = new_height;
}

/**
 * _gtk_sheet_update_extent:
 * @sheet:  the #GtkSheet
 * @cell:   the #GtkSheetCell
 * @row:    the row
 * @col:    the column
 * 
 * update cell extent and propagate to max row/column extent
 */
static void _gtk_sheet_update_extent(GtkSheet *sheet,
    GtkSheetCell *cell, gint row, gint col)
{
    guint text_width = 0, text_height = 0;
    guint new_extent_width = 0, new_extent_height = 0;
    GdkRectangle old_extent;
    GtkSheetColumn *colptr = COLPTR(sheet, col);
    GtkSheetRow *rowptr = ROWPTR(sheet, row);

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    g_return_if_fail(cell != NULL);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_update_extent[%d,%d]: called cell (xw %d,xh %d) colxw %d rowxh %d",
	row, col,
	cell->extent.width, cell->extent.height,
	colptr->max_extent_width, rowptr->max_extent_height);
#endif

    old_extent = cell->extent;  /* to check wether it was increased */

    if (!cell->text || !cell->text[0])
    {
	cell->extent.width = 0;
	cell->extent.height = 0;

	if (old_extent.height != 0)
	    _gtk_sheet_recalc_extent_height(sheet, row);
	if (old_extent.width != 0)
	    _gtk_sheet_recalc_extent_width(sheet, col);
	return;
    }

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    _get_string_extent(sheet, colptr,
	attributes.font_desc, cell->text, &text_width, &text_height);

    /* add borders */
    new_extent_width = CELL_EXTENT_WIDTH(text_width, attributes.border.width);
    new_extent_height = CELL_EXTENT_HEIGHT(text_height, 0);

    cell->extent.width = new_extent_width;
    cell->extent.height = new_extent_height;

    if (!GTK_SHEET_COLUMN_IS_VISIBLE(colptr))
	return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(rowptr))
	return;

    if (new_extent_width > old_extent.width)  /* wider */
    {
	if (new_extent_width > colptr->max_extent_width)
	{
	    colptr->max_extent_width = new_extent_width;
	}
    }
    else if (new_extent_width < old_extent.width)  /* narrower */
    {
	_gtk_sheet_recalc_extent_width(sheet, col);
    }

    if (new_extent_height > old_extent.height)  /* higher */
    {
	if (new_extent_height > rowptr->max_extent_height)
	{
	    rowptr->max_extent_height = new_extent_height;
	}
    }
    else if (new_extent_height < old_extent.height)  /* lower */
    {
	_gtk_sheet_recalc_extent_height(sheet, row);
    }

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_update_extent[%d,%d]: done cell (xw %d,xh %d) colxw %d rowxh %d",
	row, col,
	cell->extent.width, cell->extent.height,
	colptr->max_extent_width, rowptr->max_extent_height);
#endif
}

static void
_gtk_sheet_autoresize_column_internal(GtkSheet *sheet, gint col)
{
    gint new_width;
    GtkSheetColumn *colptr;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxalloccol || col > sheet->maxcol)
	return;

    colptr = COLPTR(sheet, col);

    if (!GTK_SHEET_COLUMN_IS_VISIBLE(colptr))
	return;

    new_width = COLUMN_EXTENT_TO_WIDTH(colptr->max_extent_width);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_autoresize_column_internal[%d]: called col w %d new w %d",
	col, colptr->width, new_width);
#endif

    if (new_width != colptr->width)
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("_gtk_sheet_autoresize_column_internal[%d]: set width %d",
	    col, new_width);
#endif
	gtk_sheet_set_column_width(sheet, col, new_width);
	GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_REDRAW_PENDING);
    }
}

static void
_gtk_sheet_autoresize_row_internal(GtkSheet *sheet, gint row)
{
    gint new_height;
    GtkSheetRow *rowptr;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxallocrow || row > sheet->maxrow)
	return;

    rowptr = ROWPTR(sheet, row);

    if (!GTK_SHEET_ROW_IS_VISIBLE(rowptr))
	return;

    new_height = ROW_EXTENT_TO_HEIGHT(rowptr->max_extent_height);

#if 0 && GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_autoresize_row_internal[%d]: win_h %d ext_h %d row_max_h %d",
	row, sheet->sheet_window_height, rowptr->max_extent_height,
	ROW_MAX_HEIGHT(sheet));
    g_debug("_gtk_sheet_autoresize_row_internal[%d]: called row h %d new h %d",
	row, rowptr->height, new_height);
#endif

    if (new_height != rowptr->height)
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("_gtk_sheet_autoresize_row_internal[%d]: set height %d",
	    row, new_height);
#endif
	gtk_sheet_set_row_height(sheet, row, new_height);
	GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_REDRAW_PENDING);
    }
}

static void gtk_sheet_autoresize_all(GtkSheet *sheet)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("gtk_sheet_autoresize_all: not realized");
#endif
	return;
    }

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("gtk_sheet_autoresize_all: running");
#endif

    if (gtk_sheet_autoresize_columns(sheet))
    {
	gint col;

#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("gtk_sheet_autoresize_all: columns");
#endif

	for (col = 0; col <= sheet->maxalloccol; col++)
	{
	    if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)))
	    {
		_gtk_sheet_autoresize_column_internal(sheet, col);
	    }
	}
    }

    if (gtk_sheet_autoresize_rows(sheet))
    {
	gint row;

#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("gtk_sheet_autoresize_all: rows");
#endif

	for (row = 0; row <= sheet->maxallocrow; row++)
	{
	    if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	    {
		_gtk_sheet_autoresize_row_internal(sheet, row);
	    }
	}
    }
}

/**
 * gtk_sheet_set_autoscroll:
 * @sheet: a #GtkSheet
 * @autoscroll: TRUE or FALSE
 *
 * The sheet will be automatically scrolled when you move beyond
 * the last row/col in #GtkSheet.
 */
void
gtk_sheet_set_autoscroll(GtkSheet *sheet, gboolean autoscroll)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

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
gtk_sheet_autoscroll(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->autoscroll);
}

/**
 * gtk_sheet_set_clip_text:
 * @sheet: a #GtkSheet
 * @clip_text: TRUE or FALSE
 *
 * Clip text in cell. When clip text mode is turned off, cell 
 * text is written over neighbour columns, as long as their 
 * contents are empty. 
 */
void
gtk_sheet_set_clip_text(GtkSheet *sheet, gboolean clip_text)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->clip_text = clip_text;
}

/**
 * gtk_sheet_clip_text:
 * @sheet: a #GtkSheet
 *
 * Get clip text mode in #GtkSheet. When clip text mode is 
 * turned off, cell text is written over neighbour columns, as 
 * long as their contents are empty. 
 *
 * Returns: TRUE or FALSE
 */
gboolean
gtk_sheet_clip_text(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->clip_text);
}

/**
 * gtk_sheet_set_justify_entry:
 * @sheet: a #GtkSheet
 * @justify: TRUE or FALSE
 *
 * Justify cell entry editor in #GtkSheet.
 */
void
gtk_sheet_set_justify_entry(GtkSheet *sheet, gboolean justify)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

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
gtk_sheet_justify_entry(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->justify_entry);
}

/**
 * gtk_sheet_set_vjustification:
 * @sheet: a #GtkSheet
 * @vjust: a #GtkSheetVerticalJustification
 *
 * Set the default vertical cell text justification for 
 * #GtkSheet. 
 */
void
gtk_sheet_set_vjustification(GtkSheet *sheet, GtkSheetVerticalJustification vjust)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->vjust = vjust;
}

/**
 * gtk_sheet_get_vjustification:
 * @sheet: a #GtkSheet
 *
 * Get the default vertical cell text justification from 
 * #GtkSheet. 
 *
 * Returns: the default #GtkSheetVerticalJustification
 */
GtkSheetVerticalJustification
gtk_sheet_get_vjustification(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->vjust);
}



/**
 * gtk_sheet_set_locked:
 * @sheet: a #GtkSheet
 * @locked: TRUE or FALSE
 *
 * Lock the #GtkSheet, which means it is no longer editable,
 * cell contents cannot be modified by the user.
    */
void
gtk_sheet_set_locked(GtkSheet *sheet, gboolean locked)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->locked = locked;
}

/**
 * gtk_sheet_locked:
 * @sheet: a #GtkSheet
 *
 * Get the lock status of #GtkSheet, locked means the sheet is
 * not editable, cell contents cannot be modified by the user.
 *
 * Returns: TRUE or FALSE
 */
gboolean
gtk_sheet_locked(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->locked);
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
    GtkWidget *label;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (sheet->title)
    {
	g_free(sheet->title);
	sheet->title = NULL;
    }
    if (title)
    {
	sheet->title = g_strdup(title);
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)) || !title)
	return;

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
 * gtk_sheet_set_description:
 * @sheet: a #GtkSheet
 * @description: #GtkSheet description
 *
 * Set  #GtkSheet description for application use. 
 */
void
gtk_sheet_set_description(GtkSheet *sheet, const gchar *description)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (sheet->description)
	g_free(sheet->description);
    sheet->description = g_strdup(description);
}

/**
 * gtk_sheet_get_description:
 * @sheet: a #GtkSheet
 * @description: #GtkSheet description
 *
 * Get sheet description.
 *
 * Returns: sheet description or NULL, do not modify or free it
 */
const gchar *
gtk_sheet_get_description(GtkSheet *sheet, const gchar *description)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (sheet->description);
}


/**
 * gtk_sheet_is_frozen:
 * @sheet: the #GtkSheet 
 *  
 * Get freeze status. 
 * 
 * Returns: TRUE or FALSE wether the sheet is frozen
 */
gboolean gtk_sheet_is_frozen(GtkSheet *sheet)
{
    return (GTK_SHEET_IS_FROZEN(sheet));
}


/**
 * gtk_sheet_freeze:
 * @sheet: a #GtkSheet
 *
 * Freeze all visual updates of the #GtkSheet.
 * The updates will occure in a more efficient way than if you made them on a unfrozen #GtkSheet .
 */
void
gtk_sheet_freeze(GtkSheet *sheet)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    sheet->freeze_count++;
    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

#if GTK_SHEET_DEBUG_FREEZE > 0
    g_debug("gtk_sheet_freeze: freeze_count == %d", sheet->freeze_count);
#endif

}


/**
 * _gtk_sheet_redraw_internal: 
 * @sheet:  to be redrawn 
 * @reset_hadjustment: wether to reset horizontal adjustment
 * @reset_vadjustment: wether to reset vertical adjustment
 *  
 *  do a complete sheet redraw. used after rows/cols have been
 *  appended/deleted or after combined operations while the
 *  sheet was frozen
 */
void _gtk_sheet_redraw_internal(GtkSheet *sheet,
    gboolean reset_hadjustment,
    gboolean reset_vadjustment)
{
    gboolean done = FALSE;  /* handle sheets with no scrollbars */

    if (reset_hadjustment)
	sheet->old_hadjustment = -1.;  /* causes redraw */
    if (reset_vadjustment)
	sheet->old_vadjustment = -1.;  /* causes redraw */

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (GTK_SHEET_IS_FROZEN(sheet))
	return;

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("_gtk_sheet_redraw_internal: called");
#endif

    _gtk_sheet_recalc_view_range(sheet);

    if (sheet->row_titles_visible || sheet->column_titles_visible)
    {
	size_allocate_global_button(sheet);
    }

    if (sheet->row_titles_visible)
    {
	size_allocate_row_title_buttons(sheet);
    }

    if (sheet->column_titles_visible)
    {
	_gtk_sheet_column_buttons_size_allocate(sheet);
    }

    /* send value_changed or redraw directly */

    if (sheet->vadjustment)
    {
	g_signal_emit_by_name(GTK_OBJECT(sheet->vadjustment), "value_changed");
	done = TRUE;
    }

    if (sheet->hadjustment)
    {
	g_signal_emit_by_name(GTK_OBJECT(sheet->hadjustment), "value_changed");
	done = TRUE;
    }

    if (!done)
    {
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (sheet->freeze_count == 0)
	return;

    sheet->freeze_count--;
    if (sheet->freeze_count > 0)
    {
#if GTK_SHEET_DEBUG_FREEZE > 0
	g_warning("gtk_sheet_thaw: ignored (freeze_count > 0)");
#endif
	return;
    }
#if GTK_SHEET_DEBUG_FREEZE > 0
    g_debug("gtk_sheet_thaw: freeze_count == %d", sheet->freeze_count);
#endif

    _gtk_sheet_scrollbar_adjust(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
	if (sheet->row_titles_visible)
	{
	    size_allocate_row_title_buttons(sheet);
	    gdk_window_show(sheet->row_title_window);
	}

	if (sheet->column_titles_visible)
	{
	    _gtk_sheet_column_buttons_size_allocate(sheet);
	    gdk_window_show(sheet->column_title_window);
	}
    }

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    if (gtk_sheet_autoresize(sheet))
    {
	GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_AUTORESIZE_PENDING);
    }

    _gtk_sheet_redraw_internal(sheet, TRUE, TRUE);

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
    if (width < GTK_SHEET_COLUMN_MIN_WIDTH)
	return;

    sheet->row_title_area.width = width;

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);
    _gtk_sheet_recalc_view_range(sheet);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
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

    if (sheet->row_titles_visible)
	return;

    sheet->row_titles_visible = TRUE;
    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (GTK_SHEET_IS_FROZEN(sheet))
	return;

    gdk_window_show(sheet->row_title_window);

    gdk_window_move_resize(sheet->row_title_window,
	sheet->row_title_area.x,
	sheet->row_title_area.y,
	sheet->row_title_area.width,
	sheet->row_title_area.height);

    for (row = MIN_VIEW_ROW(sheet); row <= MAX_VIEW_ROW(sheet) && row <= sheet->maxrow; row++)
    {
	GtkSheetChild *child;
	if (row < 0 || row > sheet->maxrow)
	    continue;

	child = sheet->row[row].button.child;
	if (child)
	    _gtk_sheet_child_show(child);
    }
    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
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

    if (!sheet->row_titles_visible)
	return;

    sheet->row_titles_visible = FALSE;
    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (GTK_SHEET_IS_FROZEN(sheet))
	return;

    if (sheet->row_title_window)
	gdk_window_hide(sheet->row_title_window);

    if (gtk_widget_get_visible(sheet->button))
	gtk_widget_hide(sheet->button);

    for (row = MIN_VIEW_ROW(sheet); row <= MAX_VIEW_ROW(sheet) && row <= sheet->maxrow; row++)
    {
	GtkSheetChild *child;
	if (row < 0 || row > sheet->maxrow)
	    continue;

	child = sheet->row[row].button.child;
	if (child)
	    _gtk_sheet_child_hide(child);
    }
    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
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
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->row_titles_visible);
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
gtk_sheet_set_row_title(GtkSheet *sheet,
    gint row,
    const gchar *title)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (sheet->row[row].name)
	g_free(sheet->row[row].name);

    sheet->row[row].name = g_strdup(title);
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
gtk_sheet_get_row_title(GtkSheet *sheet,
    gint row)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (sheet->row[row].name);
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
    gboolean aux_c, aux_r;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    button = &sheet->row[row].button;
    if (button->label)
	g_free(button->label);
    button->label = g_strdup(label);

    aux_c = gtk_sheet_autoresize_columns(sheet);
    aux_r = gtk_sheet_autoresize_rows(sheet);
    gtk_sheet_set_autoresize(sheet, FALSE);
    gtk_sheet_set_autoresize_rows(sheet, TRUE);
    _gtk_sheet_button_size_request(sheet, button, &req);
    gtk_sheet_set_autoresize_columns(sheet, aux_c);
    gtk_sheet_set_autoresize_rows(sheet, aux_r);

    if (req.height > sheet->row[row].height)
	gtk_sheet_set_row_height(sheet, row, req.height);

    if (req.width > sheet->row_title_area.width)
    {
	gtk_sheet_set_row_titles_width(sheet, req.width);
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	_gtk_sheet_draw_button(sheet, row, -1);
    }
    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CHANGED], 0, row, -1);
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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (row < 0 || row > sheet->maxrow)
	return (NULL);

    return (sheet->row[row].button.label);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    sheet->row[row].button.label_visible = visible;

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	_gtk_sheet_draw_button(sheet, row, -1);
	g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CHANGED], 0, row, -1);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    for (i = 0; i <= sheet->maxrow; i++) gtk_sheet_row_label_set_visibility(sheet, i, visible);
}


/**
 * gtk_sheet_row_button_justify:
 * @sheet: a #GtkSheet.
 * @row: row number
 * @justification: a #GtkJustification :GTK_JUSTIFY_LEFT, RIGHT, 
 *  			 CENTER
 *
 * Set the justification(alignment) of the row buttons.
 */
void
gtk_sheet_row_button_justify(GtkSheet *sheet, gint row,
    GtkJustification justification)
{
    GtkSheetButton *button;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    button = &sheet->row[row].button;
    button->justification = justification;

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	_gtk_sheet_draw_button(sheet, row, -1);
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
 *  
 * row_align and col_align are between 0-1 representing the location 
 * the row should appear on the screnn, 0 being top or left, 
 * 1 being bottom or right.
 * 
 * passing row_align/col_align of -1 will suppress movement in
 * that direction.
 *  
 * if row or column is negative then there is no change
 */

void
gtk_sheet_moveto(GtkSheet *sheet,
    gint row,
    gint col,
    gint row_align,
    gint col_align)
{
    gint x, y;
    guint width, height;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    g_return_if_fail(sheet->hadjustment != NULL);
    g_return_if_fail(sheet->vadjustment != NULL);

#if GTK_SHEET_DEBUG_MOVE > 0
    g_debug("gtk_sheet_moveto: row %d col %d row_align %d col_align %d",
	row, col, row_align, col_align);
#endif

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;

    height = sheet->sheet_window_height;
    width = sheet->sheet_window_width;

    /* adjust vertical scrollbar */

    if (row >= 0 && row_align >= 0)
    {
	if (row_align == 0)  /* align top cell border */
	{
	    y = _gtk_sheet_row_top_ypixel(sheet, row) - sheet->voffset;

	    if (sheet->column_titles_visible)
		y -= sheet->column_title_area.height; /* to bottom edge of title area*/
	}
	else  /* align bottom cell border */
	{
	    y = _gtk_sheet_row_top_ypixel(sheet, row) - sheet->voffset
		+ sheet->row[row].height;

	    y -= height;  /* to bottom edge of window */
	}

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	g_debug("gtk_sheet_moveto: rowTpx %d voffs %d height %d rheight %d colTw %s y %d", 
	    _gtk_sheet_row_top_ypixel(sheet, row), sheet->voffset, 
	    height, sheet->row[row].height, 
	    sheet->column_titles_visible ? "Yes" : "No",
	    y);
#endif

	if (y < 0)
	    gtk_adjustment_set_value(sheet->vadjustment, 0.0);
	else
	    gtk_adjustment_set_value(sheet->vadjustment, y);

	sheet->old_vadjustment = -1.;

	if (sheet->vadjustment)
	{
	    g_signal_emit_by_name(GTK_OBJECT(sheet->vadjustment), "value_changed");
	}
    }

    /* adjust horizontal scrollbar */
    if (col >= 0 && col_align >= 0)  /* left or right align */
    {
	if (col_align == 0)  /* align left cell border */
	{
	    x = _gtk_sheet_column_left_xpixel(sheet, col) - sheet->hoffset;

	    if (sheet->row_titles_visible)
		x -= sheet->row_title_area.width; /* to right edge of title area*/
	}
	else  /* align right cell border */
	{
	    x = _gtk_sheet_column_left_xpixel(sheet, col) - sheet->hoffset
		+ COLPTR(sheet, col)->width;

	    x -= width;  /* to right edge of window */
	}

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	g_debug("gtk_sheet_moveto: colLpx %d hoffs %d width %d cwidth %d rowTw %s x %d", 
	    _gtk_sheet_column_left_xpixel(sheet, col), sheet->hoffset, 
	    width, COLPTR(sheet, col)->width, 
	    sheet->row_titles_visible ? "Yes" : "No",
	    x);
#endif

	if (x < 0)
	    gtk_adjustment_set_value(sheet->hadjustment, 0.0);
	else
	    gtk_adjustment_set_value(sheet->hadjustment, x);

	sheet->old_hadjustment = -1.;

	if (sheet->hadjustment)
	{
	    g_signal_emit_by_name(GTK_OBJECT(sheet->hadjustment), "value_changed");
	}
    }
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
gtk_sheet_row_sensitive(GtkSheet *sheet, gint row)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (row < 0 || row > sheet->maxrow)
	return (FALSE);

    return (GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)));
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    GTK_SHEET_ROW_SET_SENSITIVE(ROWPTR(sheet, row), sensitive);

    if (!sensitive)
	sheet->row[row].button.state = GTK_STATE_INSENSITIVE;
    else
	sheet->row[row].button.state = GTK_STATE_NORMAL;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_draw_button(sheet, row, -1);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    for (i = 0; i <= sheet->maxrow; i++) gtk_sheet_row_set_sensitivity(sheet, i, sensitive);
}

/**
 * gtk_sheet_rows_set_resizable:
 * @sheet: a #GtkSheet.
 * @resizable: TRUE or FALSE
 *
 * Set rows resizable status.
 */
void
gtk_sheet_rows_set_resizable(GtkSheet *sheet, gboolean resizable)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

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
gtk_sheet_rows_resizable(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (sheet->rows_resizable);
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
gtk_sheet_row_visible(GtkSheet *sheet, gint row)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (row < 0 || row > sheet->maxrow)
	return (FALSE);

    return (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)));
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
    GtkSheetRow *rowobj;
    gint act_row, act_col;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    rowobj = ROWPTR(sheet, row);
    if (GTK_SHEET_ROW_IS_VISIBLE(rowobj) == visible)
	return;

    act_row = sheet->active_cell.row;
    act_col = sheet->active_cell.col;

    if (act_row == row)   /* hide active column -> disable active cell */
    {
	_gtk_sheet_hide_active_cell(sheet);

	sheet->active_cell.row = -1;
	sheet->active_cell.col = -1;
    }

    act_row = sheet->active_cell.row;
    act_col = sheet->active_cell.col;

    GTK_SHEET_ROW_SET_VISIBLE(rowobj, visible);

    _gtk_sheet_range_fixup(sheet, &sheet->range);
    _gtk_sheet_recalc_top_ypixels(sheet);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);
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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (gtk_widget_get_tooltip_markup(GTK_WIDGET(sheet)));
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (gtk_widget_get_tooltip_text(GTK_WIDGET(sheet)));
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    gtk_widget_set_tooltip_text(GTK_WIDGET(sheet), text);
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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (row < 0 || row > sheet->maxrow)
	return (NULL);

    return (g_strdup(ROWPTR(sheet, row)->tooltip_markup));
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    if (sheet->row[row].tooltip_markup)
	g_free(sheet->row[row].tooltip_markup);
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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (row < 0 || row > sheet->maxrow)
	return (NULL);

    return (g_strdup(ROWPTR(sheet, row)->tooltip_text));
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    if (sheet->row[row].tooltip_text)
	g_free(sheet->row[row].tooltip_text);
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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol)
	return (NULL);
    if (row < 0 || row > sheet->maxrow)
	return (NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol)
	return (NULL);

    if (!sheet->data[row])
	return (NULL);
    if (!sheet->data[row][col])
	return (NULL);

    return (g_strdup(sheet->data[row][col]->tooltip_markup));
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol)
	return;
    if (row < 0 || row > sheet->maxrow)
	return;

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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col < 0 || col > sheet->maxcol)
	return (NULL);
    if (row < 0 || row > sheet->maxrow)
	return (NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol)
	return (NULL);

    if (!sheet->data[row])
	return (NULL);
    if (!sheet->data[row][col])
	return (NULL);

    return (g_strdup(sheet->data[row][col]->tooltip_text));
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col < 0 || col > sheet->maxcol)
	return;
    if (row < 0 || row > sheet->maxrow)
	return;

    CheckCellData(sheet, row, col);
    cell = sheet->data[row][col];

    if (cell->tooltip_text)
    {
	g_free(cell->tooltip_text);
	cell->tooltip_text = NULL;
    }

    cell->tooltip_text = g_strdup(text);
}


/**
 * gtk_sheet_select_row:
 * @sheet: a #GtkSheet.
 * @row: row number
 *
 * Select the row. The range is then highlighted, and the bounds are stored in sheet->range.
 */
void
gtk_sheet_select_row(GtkSheet *sheet, gint row)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    if (sheet->state != GTK_SHEET_NORMAL)
    {
	gtk_sheet_real_unselect_range(sheet, NULL);
    }
    else
    {
	gboolean veto = TRUE;
	veto = gtk_sheet_deactivate_cell(sheet);
	if (!veto)
	    return;
    }

    sheet->state = GTK_SHEET_ROW_SELECTED;

    sheet->range.row0 = row;
    sheet->range.col0 = 0;
    sheet->range.rowi = row;
    sheet->range.coli = sheet->maxcol;

    sheet->active_cell.row = row;
    sheet->active_cell.col = 0;

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[SELECT_ROW], 0, row);
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
gtk_sheet_select_column(GtkSheet *sheet, gint column)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (column < 0 || column > sheet->maxcol)
	return;

    if (sheet->state != GTK_SHEET_NORMAL)
    {
	gtk_sheet_real_unselect_range(sheet, NULL);
    }
    else
    {
	gboolean veto = TRUE;
	veto = gtk_sheet_deactivate_cell(sheet);
	if (!veto)
	    return;
    }

    sheet->state = GTK_SHEET_COLUMN_SELECTED;

    sheet->range.row0 = 0;
    sheet->range.col0 = column;
    sheet->range.rowi = sheet->maxrow;
    sheet->range.coli = column;

    sheet->active_cell.row = 0;
    sheet->active_cell.col = column;

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[SELECT_COLUMN], 0, column);
    gtk_sheet_real_select_range(sheet, NULL);
}

/**
 * gtk_sheet_clip_range:
 * @sheet: a #GtkSheet.
 * @clip_range: #GtkSheetRange to be saved
 *
 * Save selected range to "clipboard".
 */
void
gtk_sheet_clip_range(GtkSheet *sheet, const GtkSheetRange *clip_range)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (GTK_SHEET_IN_CLIP(sheet))
	return;

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_CLIP);

    if (clip_range == NULL)
	sheet->clip_range = sheet->range;
    else
	sheet->clip_range = *clip_range;

    sheet->interval = 0;
    sheet->clip_timer = g_timeout_add_full(0, TIMEOUT_FLASH, gtk_sheet_flash, sheet, NULL);

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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!GTK_SHEET_IN_CLIP(sheet))
	return;

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_CLIP);
    g_source_remove(sheet->clip_timer);
    _gtk_sheet_range_draw(sheet, &sheet->clip_range, TRUE);

    if (gtk_sheet_range_isvisible(sheet, sheet->range))
	_gtk_sheet_range_draw(sheet, &sheet->range, TRUE);
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
gtk_sheet_in_clip(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    return (GTK_SHEET_IN_CLIP(sheet));
}


static gint
gtk_sheet_flash(gpointer data)
{
    GtkSheet *sheet;
    gint x, y, width, height;
    GdkRectangle clip_area;

    sheet = GTK_SHEET(data);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return (TRUE);
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return (TRUE);
    if (!gtk_sheet_range_isvisible(sheet, sheet->clip_range))
	return (TRUE);
    if (GTK_SHEET_IN_XDRAG(sheet))
	return (TRUE);
    if (GTK_SHEET_IN_YDRAG(sheet))
	return (TRUE);

    GDK_THREADS_ENTER();

    x = _gtk_sheet_column_left_xpixel(sheet, sheet->clip_range.col0) + 1;
    y = _gtk_sheet_row_top_ypixel(sheet, sheet->clip_range.row0) + 1;
    width = _gtk_sheet_column_left_xpixel(sheet, sheet->clip_range.coli) - x +
	COLPTR(sheet, sheet->clip_range.coli)->width - 1;
    height = _gtk_sheet_row_top_ypixel(sheet, sheet->clip_range.rowi) - y +
	sheet->row[sheet->clip_range.rowi].height - 1;

    clip_area.x = _gtk_sheet_column_left_xpixel(sheet, MIN_VIEW_COLUMN(sheet));
    clip_area.y = _gtk_sheet_row_top_ypixel(sheet, MIN_VIEW_ROW(sheet));
    clip_area.width = sheet->sheet_window_width;
    clip_area.height = sheet->sheet_window_height;

    if (x < 0)
    {
	width = width + x + 1;
	x = -1;
    }
    if (width > clip_area.width)
	width = clip_area.width + 10;

    if (y < 0)
    {
	height = height + y + 1;
	y = -1;
    }
    if (height > clip_area.height)
	height = clip_area.height + 10;

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
	x, y + height,
	x, y + height,
	width, 1);

    gdk_draw_pixmap(sheet->sheet_window,
	gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	sheet->pixmap,
	x + width, y,
	x + width, y,
	1, height);

    sheet->interval = sheet->interval + 1;
    if (sheet->interval == TIME_INTERVAL)
	sheet->interval = 0;

    gdk_gc_set_dashes(sheet->xor_gc, sheet->interval, (gint8 *)"\4\4", 2);
    gtk_sheet_draw_flashing_range(sheet, sheet->clip_range);
    gdk_gc_set_dashes(sheet->xor_gc, 0, (gint8 *)"\4\4", 2);

    GDK_THREADS_LEAVE();

    return (TRUE);
}

static void
gtk_sheet_draw_flashing_range(GtkSheet *sheet, GtkSheetRange range)
{
    GdkRectangle clip_area;
    gint x, y, width, height;

    if (!gtk_sheet_range_isvisible(sheet, sheet->clip_range))
	return;

    clip_area.x = _gtk_sheet_column_left_xpixel(sheet, MIN_VIEW_COLUMN(sheet));
    clip_area.y = _gtk_sheet_row_top_ypixel(sheet, MIN_VIEW_ROW(sheet));
    clip_area.width = sheet->sheet_window_width;
    clip_area.height = sheet->sheet_window_height;

    gdk_gc_set_clip_rectangle(sheet->xor_gc, &clip_area);

    x = _gtk_sheet_column_left_xpixel(sheet, sheet->clip_range.col0) + 1;
    y = _gtk_sheet_row_top_ypixel(sheet, sheet->clip_range.row0) + 1;
    width = _gtk_sheet_column_left_xpixel(sheet, sheet->clip_range.coli) - x +
	COLPTR(sheet, sheet->clip_range.coli)->width - 1;
    height = _gtk_sheet_row_top_ypixel(sheet, sheet->clip_range.rowi) - y +
	sheet->row[sheet->clip_range.rowi].height - 1;

    if (x < 0)
    {
	width = width + x + 1;
	x = -1;
    }
    if (width > clip_area.width)
	width = clip_area.width + 10;

    if (y < 0)
    {
	height = height + y + 1;
	y = -1;
    }
    if (height > clip_area.height)
	height = clip_area.height + 10;

    gdk_gc_set_line_attributes(sheet->xor_gc, 1, 1, 0, 0);

    gdk_draw_rectangle(sheet->sheet_window, sheet->xor_gc, FALSE,
	x, y,
	width, height);

    gdk_gc_set_line_attributes(sheet->xor_gc, 1, 0, 0, 0);

    gdk_gc_set_clip_rectangle(sheet->xor_gc, NULL);
}

static gint
gtk_sheet_range_isvisible(GtkSheet *sheet, GtkSheetRange range)
{
    g_return_val_if_fail(sheet != NULL, FALSE);

    if (range.row0 > MAX_VIEW_ROW(sheet))
	return (FALSE);
    if (range.rowi < MIN_VIEW_ROW(sheet))
	return (FALSE);

    if (range.col0 > MAX_VIEW_COLUMN(sheet))
	return (FALSE);
    if (range.coli < MIN_VIEW_COLUMN(sheet))
	return (FALSE);

    return (TRUE);
}

static gint
gtk_sheet_cell_isvisible(GtkSheet *sheet,
    gint row, gint column)
{
    GtkSheetRange range;

    range.row0 = row;
    range.col0 = column;
    range.rowi = row;
    range.coli = column;

    return (gtk_sheet_range_isvisible(sheet, range));
}

/**
 * gtk_sheet_get_visible_range:
 * @sheet: a #GtkSheet.
 * @range: a selected #GtkSheetRange
 * struct _GtkSheetRange { gint row0,col0; //  upper-left cell
 * 			  gint rowi,coli;  // lower-right cell  };
 *
 * Get sheet's ranges in a #GkSheetRange structure.
 */
void
gtk_sheet_get_visible_range(GtkSheet *sheet, GtkSheetRange *range)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    g_return_if_fail(range != NULL);

    range->row0 = MIN_VIEW_ROW(sheet);
    range->col0 = MIN_VIEW_COLUMN(sheet);
    range->rowi = MAX_VIEW_ROW(sheet);
    range->coli = MAX_VIEW_COLUMN(sheet);
}

/**
 * gtk_sheet_get_vadjustment:
 * @sheet: a #GtkSheet.
 *
 * Get vertical scroll adjustments.
 *
 * Returns: (transfer none) a #GtkAdjustment
 */
GtkAdjustment *
gtk_sheet_get_vadjustment(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (sheet->vadjustment);
}

/**
 * gtk_sheet_get_hadjustment:
 * @sheet: a #GtkSheet.
 *
 * Get horizontal scroll adjustments.
 *
 * Returns: (transfer none) a #GtkAdjustment
 */
GtkAdjustment *
gtk_sheet_get_hadjustment(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    return (sheet->hadjustment);
}

/**
 * gtk_sheet_set_vadjustment:
 * @sheet: a #GtkSheet.
 * @adjustment: a #GtkAdjustment
 *
 * Change vertical scroll adjustments.
 */
void
gtk_sheet_set_vadjustment(GtkSheet *sheet, GtkAdjustment *adjustment)
{
    GtkAdjustment *old_adjustment;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (adjustment)
	g_return_if_fail(GTK_IS_ADJUSTMENT(adjustment));
    if (sheet->vadjustment == adjustment)
	return;

    old_adjustment = sheet->vadjustment;

    if (sheet->vadjustment)
    {
	g_signal_handlers_disconnect_matched(
	    GTK_OBJECT(sheet->vadjustment),
	    G_SIGNAL_MATCH_DATA,
	    0, 0, NULL, NULL, sheet);
	g_object_unref(G_OBJECT(sheet->vadjustment));
    }

    sheet->vadjustment = adjustment;

    if (sheet->vadjustment)
    {
	g_object_ref(G_OBJECT(sheet->vadjustment));
	g_object_ref_sink(G_OBJECT(sheet->vadjustment));
	g_object_unref(G_OBJECT(sheet->vadjustment));

	g_signal_connect(GTK_OBJECT(sheet->vadjustment), "changed",
	    (void *)_vadjustment_changed_handler,
	    (gpointer)sheet);
	g_signal_connect(GTK_OBJECT(sheet->vadjustment), "value_changed",
	    (void *)_vadjustment_value_changed_handler,
	    (gpointer)sheet);
    }

    if (!sheet->vadjustment || !old_adjustment)
    {
	gtk_widget_queue_resize(GTK_WIDGET(sheet));
	return;
    }

    sheet->old_vadjustment = gtk_adjustment_get_value(sheet->vadjustment);
}

/**
 * gtk_sheet_set_hadjustment:
 * @sheet: a #GtkSheet.
 * @adjustment: a #GtkAdjustment
 *
 * Change horizontal scroll adjustments.
 */
void
gtk_sheet_set_hadjustment(GtkSheet *sheet, GtkAdjustment *adjustment)
{
    GtkAdjustment *old_adjustment;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (adjustment)
	g_return_if_fail(GTK_IS_ADJUSTMENT(adjustment));
    if (sheet->hadjustment == adjustment)
	return;

    old_adjustment = sheet->hadjustment;

    if (sheet->hadjustment)
    {
	g_signal_handlers_disconnect_matched(
	    GTK_OBJECT(sheet->hadjustment),
	    G_SIGNAL_MATCH_DATA,
	    0, 0, NULL, NULL, sheet);
	g_object_unref(GTK_OBJECT(sheet->hadjustment));
    }

    sheet->hadjustment = adjustment;

    if (sheet->hadjustment)
    {
	g_object_ref(G_OBJECT(sheet->hadjustment));
	g_object_ref_sink(G_OBJECT(sheet->hadjustment));
	g_object_unref(G_OBJECT(sheet->hadjustment));

	g_signal_connect(GTK_OBJECT(sheet->hadjustment), "changed",
	    (void *)_hadjustment_changed_handler,
	    (gpointer)sheet);
	g_signal_connect(GTK_OBJECT(sheet->hadjustment), "value_changed",
	    (void *)_hadjustment_value_changed_handler,
	    (gpointer)sheet);
    }

    if (!sheet->hadjustment || !old_adjustment)
    {
	gtk_widget_queue_resize(GTK_WIDGET(sheet));
	return;
    }

    sheet->old_hadjustment = gtk_adjustment_get_value(sheet->hadjustment);
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
gtk_sheet_set_scroll_adjustments(GtkSheet *sheet,
    GtkAdjustment *hadjustment,
    GtkAdjustment *vadjustment)
{
#if GTK_SHEET_DEBUG_SIGNALS > 0
    g_debug("SIGNAL set-scroll-adjustments %p", sheet);
#endif

    if (sheet->hadjustment != hadjustment)
	gtk_sheet_set_hadjustment(sheet, hadjustment);

    if (sheet->vadjustment != vadjustment)
	gtk_sheet_set_vadjustment(sheet, vadjustment);
}

/*
 * gtk_sheet_finalize_handler:
 * 
 * this is the #GtkSheet object class "finalize" signal handler
 * 
 * @param object the #GtkSheet
 */
static void
gtk_sheet_finalize_handler(GObject *object)
{
    GtkSheet *sheet;

    g_return_if_fail(object != NULL);
    g_return_if_fail(GTK_IS_SHEET(object));

    sheet = GTK_SHEET(object);

    /* get rid of all the cells */
    gtk_sheet_range_clear(sheet, NULL);
    gtk_sheet_range_delete(sheet, NULL);

    gtk_sheet_delete_rows(sheet, 0, sheet->maxrow + 1);
    gtk_sheet_delete_columns(sheet, 0, sheet->maxcol + 1);

    DeleteRow(sheet, 0, sheet->maxrow + 1);
    DeleteColumn(sheet, 0, sheet->maxcol + 1);

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

    if (G_OBJECT_CLASS(sheet_parent_class)->finalize)
	(*G_OBJECT_CLASS(sheet_parent_class)->finalize)(object);
}

/*
 * gtk_sheet_destroy_handler:
 * 
 * this is the #GtkSheet object class "finalize" handler
 * 
 * @param object
 */
static void
gtk_sheet_destroy_handler(GtkObject *object)
{
    GtkSheet *sheet;
    GList *children;

    g_return_if_fail(object != NULL);
    g_return_if_fail(GTK_IS_SHEET(object));

    sheet = GTK_SHEET(object);

    /* destroy the entry */
    if (sheet->sheet_entry && GTK_IS_WIDGET(sheet->sheet_entry))
    {
	gtk_widget_destroy(sheet->sheet_entry);
	sheet->sheet_entry = NULL;
    }

    /* destroy the global selection button */
    if (sheet->button && GTK_IS_WIDGET(sheet->button))
    {
#if GTK_SHEET_DEBUG_REALIZE > 0
	g_debug("gtk_sheet_destroy: destroying old entry %p", sheet->button);
#endif
	gtk_widget_destroy(sheet->button);
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
	g_signal_handlers_disconnect_matched(
	    GTK_OBJECT(sheet->hadjustment),
	    G_SIGNAL_MATCH_DATA,
	    0, 0, NULL, NULL, sheet);
	g_object_unref(G_OBJECT(sheet->hadjustment));
	sheet->hadjustment = NULL;
    }
    if (sheet->vadjustment)
    {
	g_signal_handlers_disconnect_matched(
	    GTK_OBJECT(sheet->vadjustment),
	    G_SIGNAL_MATCH_DATA,
	    0, 0, NULL, NULL, sheet);
	g_object_unref(G_OBJECT(sheet->vadjustment));
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

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_DESTROYED);

    if (GTK_OBJECT_CLASS(sheet_parent_class)->destroy)
	(*GTK_OBJECT_CLASS(sheet_parent_class)->destroy)(object);
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
gtk_sheet_style_set_handler(GtkWidget *widget, GtkStyle  *previous_style)
{
    GtkSheet *sheet;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    if (GTK_WIDGET_CLASS(sheet_parent_class)->style_set)
	(*GTK_WIDGET_CLASS(sheet_parent_class)->style_set)(widget, previous_style);

    sheet = GTK_SHEET(widget);

    if (gtk_widget_get_realized(widget))
    {
	gtk_style_set_background(gtk_widget_get_style(widget),
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
gtk_sheet_realize_handler(GtkWidget *widget)
{
    GtkSheet *sheet;
    GdkWindowAttr attributes;
    gint attributes_mask;
    GdkGCValues values, auxvalues;
    GdkColormap *colormap;
    GtkSheetChild *child;
    GList *children;
    GtkAllocation allocation;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    sheet = GTK_SHEET(widget);

    /* we need to recalc all positions, because row/column visibility
       may have changed between adding rows/cols and realisation
       PR#92947
       */
    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

#if GTK_SHEET_DEBUG_REALIZE > 0
    g_debug("gtk_sheet_realize_handler: called (%p)", sheet->sheet_entry);
#endif

    gtk_widget_set_realized_true(GTK_WIDGET(widget));

    gtk_widget_get_allocation(widget, &allocation);
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);

    attributes.event_mask = gtk_widget_get_events(widget);
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
	gdk_window_new(gtk_widget_get_parent_window(widget),
	    &attributes, attributes_mask));

    gdk_window_set_user_data(gtk_widget_get_window(widget), sheet);

    gtk_widget_set_style(widget,
	gtk_style_attach(gtk_widget_get_style(widget),
	    gtk_widget_get_window(widget)));

    gtk_style_set_background(gtk_widget_get_style(widget),
	gtk_widget_get_window(widget),
	GTK_STATE_NORMAL);

    attributes.x = 0;
    if (sheet->row_titles_visible)
	attributes.x = sheet->row_title_area.width;
    attributes.y = 0;
    attributes.width = sheet->column_title_area.width;
    attributes.height = sheet->column_title_area.height;

    /* column-title window */
    sheet->column_title_window = gdk_window_new(
	gtk_widget_get_window(widget),
	&attributes, attributes_mask);
    gdk_window_set_user_data(sheet->column_title_window, sheet);
    gtk_style_set_background(gtk_widget_get_style(widget),
	sheet->column_title_window, GTK_STATE_NORMAL);

    attributes.x = 0;
    attributes.y = 0;
    if (sheet->column_titles_visible)
	attributes.y = sheet->column_title_area.height;
    attributes.width = sheet->row_title_area.width;
    attributes.height = sheet->row_title_area.height;

    /* row-title window */
    sheet->row_title_window = gdk_window_new(
	gtk_widget_get_window(widget),
	&attributes, attributes_mask);
    gdk_window_set_user_data(sheet->row_title_window, sheet);
    gtk_style_set_background(gtk_widget_get_style(widget),
	sheet->row_title_window, GTK_STATE_NORMAL);

    /* sheet-window */
    attributes.cursor = gdk_cursor_new(GDK_PLUS);

    attributes.x = 0;
    attributes.y = 0;
    attributes.width = sheet->sheet_window_width;
    attributes.height = sheet->sheet_window_height;

    sheet->sheet_window = gdk_window_new(
	gtk_widget_get_window(widget),
	&attributes, attributes_mask);
    gdk_window_set_user_data(sheet->sheet_window, sheet);

    gdk_window_set_background(sheet->sheet_window,
	&(gtk_widget_get_style(widget)->white));
    gdk_window_show(sheet->sheet_window);

    /* backing_pixmap */
    gtk_sheet_make_backing_pixmap(sheet, 0, 0);

    /* GCs */
    if (sheet->fg_gc)
	gdk_gc_unref(sheet->fg_gc);
    sheet->fg_gc = gdk_gc_new(gtk_widget_get_window(widget));

    if (sheet->bg_gc)
	gdk_gc_unref(sheet->bg_gc);
    sheet->bg_gc = gdk_gc_new(gtk_widget_get_window(widget));

    colormap = gtk_widget_get_colormap(widget);

    gdk_color_white(colormap, &(gtk_widget_get_style(widget)->white));
    gdk_color_black(colormap, &(gtk_widget_get_style(widget)->black));

    gdk_gc_get_values(sheet->fg_gc, &auxvalues);

    values.foreground = gtk_widget_get_style(widget)->white;
    values.function = GDK_INVERT;
    values.subwindow_mode = GDK_INCLUDE_INFERIORS;

    if (sheet->xor_gc)
	gdk_gc_unref(sheet->xor_gc);
    sheet->xor_gc = gdk_gc_new_with_values(gtk_widget_get_window(widget),
	&values,
	GDK_GC_FOREGROUND |
	    GDK_GC_FUNCTION |
	    GDK_GC_SUBWINDOW);

    if (gtk_widget_get_parent(sheet->sheet_entry))
    {
	g_object_ref(sheet->sheet_entry);
	gtk_widget_unparent(sheet->sheet_entry);
    }
    gtk_widget_set_parent_window(sheet->sheet_entry, sheet->sheet_window);
    gtk_widget_set_parent(sheet->sheet_entry, GTK_WIDGET(sheet));

    if (sheet->button && gtk_widget_get_parent(sheet->button))
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
    _gtk_sheet_column_buttons_size_allocate(sheet);

    if (sheet->title)  /* re-initialize title to update GUI */
    {
	gchar *existing_title = g_strdup(sheet->title);
	gtk_sheet_set_title(sheet, existing_title);
	g_free(existing_title);
    }

    children = sheet->children;
    while (children)
    {
	child = children->data;
	children = children->next;

	gtk_sheet_realize_child(sheet, child);
    }
}

/*
 * global_button_clicked_handler:
 * this is the #GtkSheet global sheet button "button-press-event" handler. 
 *  
 * It will handle single-clicks of button 1 internally, selecting/deselecting 
 * all sheet cells. All other button press events are propagated to the sheet. 
 *  
 * You cann connect your own "button-press-event" handler to the sheet 
 * widget, and will receive all non-internally handled button-press-events.
 * 
 * @param widget the global sheet button that received the signal 
 * @param event  the GdkEventButton which triggered this signal
 * @param data   the #GtkSheet passed on signal connection 
 *  
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
global_button_press_handler(GtkWidget *widget,
    GdkEventButton *event,
    gpointer data)
{
    gboolean veto;
    GtkSheet *sheet = GTK_SHEET(data);
    gboolean retval = FALSE;

    if (!retval
	&& (event->type == GDK_BUTTON_PRESS)
	&& (event->button == 1))
    {
	gtk_sheet_click_cell(sheet, -1, -1, &veto);
	gtk_widget_grab_focus(GTK_WIDGET(sheet));
    }
    else
    {
	g_signal_emit_by_name(GTK_WIDGET(sheet), 
	    "button_press_event", 
	    event,
	    &retval);
    }

    return(retval);
}

static void
create_global_button(GtkSheet *sheet)
{
    sheet->button = gtk_button_new_with_label(" ");

    g_signal_connect(GTK_OBJECT(sheet->button),
	"button-press-event",
	G_CALLBACK(global_button_press_handler),
	(gpointer)sheet);
}

static void
size_allocate_global_button(GtkSheet *sheet)
{
    GtkAllocation allocation;

    if (!sheet->column_titles_visible)
	return;
    if (!sheet->row_titles_visible)
	return;

    gtk_widget_size_request(sheet->button, NULL);

    allocation.x = 0;
    allocation.y = 0;
    allocation.width = sheet->row_title_area.width;
    allocation.height = sheet->column_title_area.height;

    gtk_widget_size_allocate(sheet->button, &allocation);
    gtk_widget_show(sheet->button);
}


/*
 * gtk_sheet_unrealize_handler:
 * 
 * this is the #GtkSheet widget class "unrealize" signal handler
 * 
 * @param widget the #GtkSheet
 */
static void
gtk_sheet_unrealize_handler(GtkWidget *widget)
{
    GtkSheet *sheet;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    sheet = GTK_SHEET(widget);

    gdk_cursor_destroy(sheet->cursor_drag);

    gdk_gc_destroy(sheet->xor_gc);
    gdk_gc_destroy(sheet->fg_gc);
    gdk_gc_destroy(sheet->bg_gc);

    gdk_window_destroy(sheet->sheet_window);
    gdk_window_destroy(sheet->column_title_window);
    gdk_window_destroy(sheet->row_title_window);

    if (sheet->pixmap)
    {
	g_object_unref(G_OBJECT(sheet->pixmap));
	sheet->pixmap = NULL;
    }

    sheet->column_title_window = NULL;
    sheet->sheet_window = NULL;
    sheet->cursor_drag = NULL;
    sheet->xor_gc = NULL;
    sheet->fg_gc = NULL;
    sheet->bg_gc = NULL;

    if (GTK_WIDGET_CLASS(sheet_parent_class)->unrealize)
	(*GTK_WIDGET_CLASS(sheet_parent_class)->unrealize)(widget);
}

/*
 * gtk_sheet_map_handler:
 * 
 * this is the #GtkSheet widget class "map" signal handler
 * 
 * @param widget the #GtkSheet
 */
static void
gtk_sheet_map_handler(GtkWidget *widget)
{
    GtkSheet *sheet;
    GtkWidget *WdChild;
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_EXPOSE > 0
    g_debug("gtk_sheet_map_handler: called");
#endif

    if (!gtk_widget_get_mapped(widget))
    {
	gtk_widget_set_mapped_true(GTK_WIDGET(widget));

	if (!sheet->cursor_drag)
	    sheet->cursor_drag = gdk_cursor_new(GDK_PLUS);

	gdk_window_show(gtk_widget_get_window(widget));
	gdk_window_show(sheet->sheet_window);

	if (sheet->column_titles_visible)
	{
	    _gtk_sheet_column_buttons_size_allocate(sheet);
	    gdk_window_show(sheet->column_title_window);
	}

	if (sheet->row_titles_visible)
	{
	    size_allocate_row_title_buttons(sheet);
	    gdk_window_show(sheet->row_title_window);
	}

#if 0
	/* this will be done by gtk_sheet_activate_cell() below,
	   it causes trouble when there is no active cell in the sheet,
	   because sheet_entry will start to process events */
	if (!gtk_widget_get_mapped (sheet->sheet_entry))
	{
	    gtk_widget_show (sheet->sheet_entry);
	    gtk_widget_map (sheet->sheet_entry);
	}
#endif

	if (gtk_widget_get_visible(sheet->button) &&
	    !gtk_widget_get_mapped(sheet->button))
	{
	    gtk_widget_show(sheet->button);
	    gtk_widget_map(sheet->button);
	}

	if ((WdChild = gtk_bin_get_child(GTK_BIN(sheet->button))))
	{
	    if (gtk_widget_get_visible(WdChild) &&
		!gtk_widget_get_mapped(WdChild))
	    {
		gtk_widget_map(WdChild);
	    }
	}

#if GTK_SHEET_DEBUG_EXPOSE > 0
	g_debug("gtk_sheet_map_handler: calling _gtk_sheet_range_draw");
#endif

	_gtk_sheet_recalc_view_range(sheet);
	_gtk_sheet_range_draw(sheet, NULL, TRUE);

	/* this was already done above - why again?
	gtk_sheet_activate_cell(sheet, 
	    sheet->active_cell.row, 
	    sheet->active_cell.col);
	    */

	children = sheet->children;
	while (children)
	{
	    child = children->data;
	    children = children->next;

	    if (gtk_widget_get_visible(child->widget) &&
		!gtk_widget_get_mapped(child->widget))
	    {
		gtk_widget_map(child->widget);
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
gtk_sheet_unmap_handler(GtkWidget *widget)
{
    GtkSheet *sheet;
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("gtk_sheet_unmap: called");
#endif

    if (gtk_widget_get_mapped(widget))
    {
	gtk_widget_set_mapped_false(GTK_WIDGET(widget));

	gdk_window_hide(sheet->sheet_window);

	if (sheet->column_titles_visible)
	    gdk_window_hide(sheet->column_title_window);

	if (sheet->row_titles_visible)
	    gdk_window_hide(sheet->row_title_window);

	gdk_window_hide(gtk_widget_get_window(widget));

	if (gtk_widget_get_mapped(sheet->sheet_entry))
	    gtk_widget_unmap(sheet->sheet_entry);

	if (gtk_widget_get_mapped(sheet->button))
	    gtk_widget_unmap(sheet->button);

	children = sheet->children;
	while (children)
	{
	    child = children->data;
	    children = children->next;

	    if (gtk_widget_get_visible(child->widget) &&
		gtk_widget_get_mapped(child->widget))
	    {
		gtk_widget_unmap(child->widget);
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

		    gdk_gc_set_foreground(sheet->bg_gc, &sheet->tm_color);

		    p[0].x = _gtk_sheet_column_left_xpixel(sheet, col) + COLPTR(sheet, col)->width
			- GTK_SHEET_DEFAULT_TM_SIZE;
		    p[0].y = _gtk_sheet_row_top_ypixel(sheet, row) + 1;

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

		    gdk_gc_set_foreground(sheet->bg_gc, &sheet->tm_color);

		    p[0].x = sheet->row_title_area.width - 1
			- GTK_SHEET_DEFAULT_TM_SIZE;
		    p[0].y = _gtk_sheet_row_top_ypixel(sheet, row) + 1;
		    if (sheet->column_titles_visible)
			p[0].y -= sheet->column_title_area.height;

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

		    gdk_gc_set_foreground(sheet->bg_gc, &sheet->tm_color);

		    p[0].x = _gtk_sheet_column_right_xpixel(sheet, col) + CELL_SPACING - 1
			- GTK_SHEET_DEFAULT_TM_SIZE;
		    if (sheet->row_titles_visible)
			p[0].x -= sheet->row_title_area.width;
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

	default:
	    return;
    }
}

static void
_cell_draw_background(GtkSheet *sheet, gint row, gint col)
{
    GtkWidget *widget;
    GdkGC * fg_gc, *bg_gc;
    GdkRectangle area;

    g_return_if_fail(sheet != NULL);

    /* bail now if we arn't drawable yet */
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;
    if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)))
	return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	return;

    widget = GTK_WIDGET(sheet);

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    /* select GC for background rectangle */
    gdk_gc_set_foreground(sheet->fg_gc, &attributes.foreground);
    gdk_gc_set_foreground(sheet->bg_gc, &attributes.background);

    fg_gc = sheet->fg_gc;
    bg_gc = sheet->bg_gc;

    area.x = _gtk_sheet_column_left_xpixel(sheet, col);
    area.y = _gtk_sheet_row_top_ypixel(sheet, row);
    area.width = COLPTR(sheet, col)->width;
    area.height = ROWPTR(sheet, row)->height;

#if GTK_SHEET_DEBUG_DRAW_BACKGROUND>0
#if 1
    g_debug("_cell_draw_background(%d,%d): cellbg x %d y %d w %d h %d %s",
	row, col,
	area.x, area.y, area.width, area.height,
	gdk_color_to_string(&attributes.background));
#endif
#endif

    /* fill cell background */
    gdk_draw_rectangle(sheet->pixmap,
	bg_gc,
	TRUE,
	area.x, area.y,
	area.width, area.height);

    gdk_gc_set_line_attributes(sheet->fg_gc, 1, 0, 0, 0);

    if (sheet->show_grid)
    {
	gdk_gc_set_foreground(sheet->bg_gc, &sheet->grid_color);

#if GTK_SHEET_DEBUG_DRAW_BACKGROUND>0
#if 0
	g_debug("_cell_draw_background(%d,%d): grid x %d y %d w %d h %d %s",
		row, col,
		area.x, area.y, area.width, area.height,
		gdk_color_to_string(&attributes.background));
#endif
#endif

	/* draw grid rectangle */
	gdk_draw_rectangle(sheet->pixmap,
	    sheet->bg_gc,
	    FALSE,
	    area.x, area.y,
	    area.width, area.height);
    }

    gtk_sheet_draw_tooltip_marker(sheet, ON_CELL_AREA, row, col);
}

static void
_cell_draw_border(GtkSheet *sheet, gint row, gint col, gint mask)
{
    GtkWidget *widget;
    GdkGC * fg_gc, *bg_gc;
    GdkRectangle area;
    guint width;

    g_return_if_fail(sheet != NULL);

    /* bail now if we arn't drawable yet */
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;
    if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)))
	return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	return;

    widget = GTK_WIDGET(sheet);

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    /* select GC for background rectangle */
    gdk_gc_set_foreground(sheet->fg_gc, &attributes.border.color);
    gdk_gc_set_foreground(sheet->bg_gc, &attributes.background);

    fg_gc = sheet->fg_gc;
    bg_gc = sheet->bg_gc;

    area.x = _gtk_sheet_column_left_xpixel(sheet, col);
    area.y = _gtk_sheet_row_top_ypixel(sheet, row);
    area.width = COLPTR(sheet, col)->width;
    area.height = sheet->row[row].height;

    width = attributes.border.width;
    gdk_gc_set_line_attributes(sheet->fg_gc, attributes.border.width,
	attributes.border.line_style,
	attributes.border.cap_style,
	attributes.border.join_style);
    if (width > 0)
    {

	if (attributes.border.mask & GTK_SHEET_LEFT_BORDER & mask)
	    gdk_draw_line(sheet->pixmap, sheet->fg_gc,
		area.x, area.y - width / 2,
		area.x, area.y + area.height + width / 2 + 1);

	if (attributes.border.mask & GTK_SHEET_RIGHT_BORDER & mask)
	    gdk_draw_line(sheet->pixmap, sheet->fg_gc,
		area.x + area.width, area.y - width / 2,
		area.x + area.width,
		area.y + area.height + width / 2 + 1);

	if (attributes.border.mask & GTK_SHEET_TOP_BORDER & mask)
	    gdk_draw_line(sheet->pixmap, sheet->fg_gc,
		area.x - width / 2, area.y,
		area.x + area.width + width / 2 + 1,
		area.y);

	if (attributes.border.mask & GTK_SHEET_BOTTOM_BORDER & mask)
	    gdk_draw_line(sheet->pixmap, sheet->fg_gc,
		area.x - width / 2, area.y + area.height,
		area.x + area.width + width / 2 + 1,
		area.y + area.height);
    }

}


static void
_cell_draw_label(GtkSheet *sheet, gint row, gint col)
{
    GtkWidget *widget;
    GdkRectangle area, clip_area;
    gint i;
    gint text_width, text_height, y;
    gint xoffset = 0;
    gint size, sizel, sizer;
    GdkGC *gc;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoFontMetrics *metrics;
    PangoContext *context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
    gint ascent, descent, spacing, y_pos;
    GtkSheetVerticalJustification vjust;

    gchar * label, *dataformat;

    g_return_if_fail(sheet != NULL);

    /* bail now if we aren't drawable yet */
    if (!GTK_WIDGET_DRAWABLE(sheet))
	return;

    if (row < 0 || row > sheet->maxallocrow)
	return;
    if (col < 0 || col > sheet->maxalloccol)
	return;

    if (!sheet->data[row])
	return;
    if (!sheet->data[row][col])
	return;
    if (!sheet->data[row][col]->text || !sheet->data[row][col]->text[0])
	return;

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;

    /* bail now if we aren't drawable yet */
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;

    GtkSheetColumn *colptr = COLPTR(sheet, col);

    if (!GTK_SHEET_COLUMN_IS_VISIBLE(colptr))
	return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	return;

    widget = GTK_WIDGET(sheet);

    label = sheet->data[row][col]->text;
    dataformat = gtk_sheet_column_get_format(sheet, col);

    if (dataformat)
	label = gtk_data_format(label, dataformat);

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    /* select GC for background rectangle */
    gdk_gc_set_foreground(sheet->fg_gc, &attributes.foreground);
    gdk_gc_set_background(sheet->fg_gc, &attributes.background);

    gc = sheet->fg_gc;

    area.x = _gtk_sheet_column_left_xpixel(sheet, col);
    area.y = _gtk_sheet_row_top_ypixel(sheet, row);
    area.width = colptr->width;
    area.height = ROWPTR(sheet, row)->height;

    clip_area = area;

    layout = gtk_widget_create_pango_layout(GTK_WIDGET(sheet), label);
    pango_layout_set_font_description(layout, attributes.font_desc);

    if (!gtk_sheet_autoresize_columns(sheet))
    {
	switch(colptr->wrap_mode)
	{
	    case GTK_WRAP_NONE: 
		break;

	    case GTK_WRAP_CHAR:
		pango_layout_set_width(layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
		break;

	    case GTK_WRAP_WORD:
		pango_layout_set_width(layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
		break;

	    case GTK_WRAP_WORD_CHAR:
		pango_layout_set_width(layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
		break;
	}
    }

    pango_layout_get_pixel_extents(layout, NULL, &rect);

    metrics = pango_context_get_metrics(context,
	attributes.font_desc, pango_context_get_language(context));

    ascent = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
    descent = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;
    spacing = pango_layout_get_spacing(layout) / PANGO_SCALE;

    pango_font_metrics_unref(metrics);

    /* Align primarily for locale's ascent/descent */

    /* vertical cell text justification */
    {
	/* column->vjust overrides sheet->vjust */

	vjust = colptr->vjust;
	if (vjust == GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT)
	    vjust = sheet->vjust;

	/* Vertical justification is quantisized, so that all text lines using the
	   same font appear vertically aligned, even if adjacent columns have
	   different vjust settings.
	   */
	switch(vjust)
	{
	    case GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT:
	    case GTK_SHEET_VERTICAL_JUSTIFICATION_TOP:
		y_pos = CELLOFFSET;
		break;

	    case GTK_SHEET_VERTICAL_JUSTIFICATION_MIDDLE:
		{
		    /* the following works only if the whole text is set with same metrics */
		    register gint line_height = ascent + descent + spacing;
		    register gint area_lines = area.height / line_height;
		    register gint text_lines = rect.height / line_height;

		    y_pos = CELLOFFSET - ((text_lines - area_lines) / 2) * line_height;
		}
		break;

	    case GTK_SHEET_VERTICAL_JUSTIFICATION_BOTTOM:
		{
		    /* the following works only if the whole text is set with same metrics */
		    register gint line_height = ascent + descent + spacing;
		    register gint area_lines = area.height / line_height;
		    register gint area_height_quant = area_lines * line_height;

		    y_pos = CELLOFFSET + area_height_quant - rect.height;
		}
		break;
	}

	y = area.y + y_pos;
    }

    text_width = rect.width;
    text_height = rect.height;


    switch(attributes.justification)
    {
	case GTK_JUSTIFY_RIGHT:
	    size = area.width;  /* start with col size */
	    area.x += area.width;  /* anchor clip_area at right */

	    if (!gtk_sheet_clip_text(sheet))  /* text extends multiple cells */
	    {
		for (i = col - 1; i >= MIN_VIEW_COLUMN(sheet); i--)
		{
		    GtkSheetColumn *cpi = COLPTR(sheet, i);

		    if (i < 0 || i > sheet->maxcol)
			break;

		    if (!GTK_SHEET_COLUMN_IS_VISIBLE(cpi))
			continue;
		    if (gtk_sheet_cell_get_text(sheet, row, i))
			break;
		    if (size >= text_width + CELLOFFSET)
			break;

		    size += cpi->width;  /* extend to left */

#if GTK_SHEET_OPTIMIZE_COLUMN_DRAW>0
		    /* note: this column draws text on cpi */
		    cpi->right_text_column = MAX(col, cpi->right_text_column);
#if GTK_SHEET_DEBUG_DRAW > 0
		    g_debug("_cell_draw_label: right_text_column %d = %d",
			i, cpi->right_text_column);
#endif
#endif
		}
		area.width = size;  /* extend clip area */
	    }
	    area.x -= size;  /* shift left */
	    xoffset += area.width - text_width - 2 * CELLOFFSET - attributes.border.width / 2;
	    break;

	case GTK_JUSTIFY_CENTER:
	    sizel = sizer = area.width / 2;  /* start with half col size*/
	    area.x += area.width / 2;  /* anchor clip_area at center */

	    if (!gtk_sheet_clip_text(sheet))  /* text extends multiple cells */
	    {
		for (i = col + 1; i <= MAX_VIEW_COLUMN(sheet) && i <= sheet->maxcol; i++)
		{
		    GtkSheetColumn *cpi = COLPTR(sheet, i);

		    if (!GTK_SHEET_COLUMN_IS_VISIBLE(cpi))
			continue;
		    if (gtk_sheet_cell_get_text(sheet, row, i))
			break;
		    if (sizer >= text_width / 2)
			break;

		    sizer += cpi->width;  /* extend to right */

#if GTK_SHEET_OPTIMIZE_COLUMN_DRAW>0
		    /* note: this column draws text on cpi */
		    cpi->left_text_column = MIN(col, cpi->left_text_column);
#if GTK_SHEET_DEBUG_DRAW > 0
		    g_debug("_cell_draw_label: left_text_column %d = %d",
			i, cpi->left_text_column);
#endif
#endif
		}
		for (i = col - 1; i >= MIN_VIEW_COLUMN(sheet); i--)
		{
		    GtkSheetColumn *cpi = COLPTR(sheet, i);

		    if (i < 0 || i > sheet->maxcol)
			break;

		    if (!GTK_SHEET_COLUMN_IS_VISIBLE(cpi))
			continue;
		    if (gtk_sheet_cell_get_text(sheet, row, i))
			break;
		    if (sizel >= text_width / 2)
			break;

		    sizel += cpi->width;  /* extend to left */

#if GTK_SHEET_OPTIMIZE_COLUMN_DRAW>0
		    /* note: this column draws text on cpi */
		    cpi->right_text_column = MAX(col, cpi->right_text_column);
#if GTK_SHEET_DEBUG_DRAW > 0
		    g_debug("_cell_draw_label: right_text_column %d = %d",
			i, cpi->right_text_column);
#endif
#endif
		}
		area.width = sizel + sizer;  /* extend clip area */
	    }
	    area.x -= sizel;  /* shift left */
	    xoffset += sizel - text_width / 2 - CELLOFFSET;
	    break;

	case GTK_JUSTIFY_LEFT:
	default:
	    size = area.width;  /* start with col size, anchor at left */

	    if (!gtk_sheet_clip_text(sheet))  /* text extends multiple cells */
	    {
		for (i = col + 1; i <= MAX_VIEW_COLUMN(sheet) && i <= sheet->maxcol; i++)
		{
		    GtkSheetColumn *cpi = COLPTR(sheet, i);

		    if (!GTK_SHEET_COLUMN_IS_VISIBLE(cpi))
			continue;
		    if (gtk_sheet_cell_get_text(sheet, row, i))
			break;
		    if (size >= text_width + CELLOFFSET)
			break;

		    size += cpi->width;  /* extend to right */

#if GTK_SHEET_OPTIMIZE_COLUMN_DRAW>0
		    /* note: this column draws text on cpi */
		    cpi->left_text_column = MIN(col, cpi->left_text_column);
#if GTK_SHEET_DEBUG_DRAW > 0
		    g_debug("_cell_draw_label: left_text_column %d = %d",
			i, cpi->left_text_column);
#endif
#endif
		}
		area.width = size;  /* extend clip area */
	    }
	    xoffset += attributes.border.width / 2;
	    break;
    }

    if (!gtk_sheet_clip_text(sheet))  /* text extends multiple cells */
	clip_area = area;
    gdk_gc_set_clip_rectangle(gc, &clip_area);

#if GTK_SHEET_DEBUG_DRAW_LABEL>0
#if 0
    g_debug("_cell_draw_label(%d,%d): clip x %d y %d w %d h %d",
	    row, col,
	    clip_area.x, clip_area.y, clip_area.width, clip_area.height);
#endif
#endif


#if GTK_SHEET_DEBUG_DRAW_LABEL>0
#if 1
    g_debug("_cell_draw_label(%d,%d): x %d y %d fg %s bg %s",
	row, col,
	area.x + xoffset + CELLOFFSET, y,
	gdk_color_to_string(&attributes.foreground),
	gdk_color_to_string(&attributes.background)
	);
#endif
#endif

    gdk_draw_layout(sheet->pixmap, gc,
	area.x + xoffset + CELLOFFSET, y,
	layout);

    g_object_unref(G_OBJECT(layout));

    /* copy sheet->pixmap to window */

    gdk_draw_pixmap(sheet->sheet_window,
	gc,
	sheet->pixmap,
	area.x, area.y,
	area.x, area.y,
	area.width, area.height);

    gdk_gc_set_clip_rectangle(gc, NULL);
}



/**
 * _gtk_sheet_range_draw:
 * @sheet:  the #GtkSheet
 * @range:  the #GtkSheetRange or NULL
 * @activate_active_cell: TRUE to activate active cell after 
 *                      drawing
 *  
 * draw visible part of range. 
 * If @range == NULL then draw the whole screen.
 *  
 */
void
_gtk_sheet_range_draw(GtkSheet *sheet,
    const GtkSheetRange *range,
    gboolean activate_active_cell)
{
    gint row, col;
    GtkSheetRange drawing_range;
    GdkRectangle area;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_SHEET(sheet));

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("_gtk_sheet_range_draw: called");
#endif

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (!gtk_widget_get_mapped(GTK_WIDGET(sheet)))
	return;

    if (range)
    {
	drawing_range.row0 = MAX(range->row0, MIN_VIEW_ROW(sheet));
	drawing_range.rowi = MIN(range->rowi, MAX_VIEW_ROW(sheet));
	drawing_range.col0 = MAX(range->col0, MIN_VIEW_COLUMN(sheet));
	drawing_range.coli = MIN(range->coli, MAX_VIEW_COLUMN(sheet));
    }
    else
    {
	drawing_range.row0 = MIN_VIEW_ROW(sheet);
	drawing_range.rowi = MAX_VIEW_ROW(sheet);
	drawing_range.col0 = MIN_VIEW_COLUMN(sheet);
	drawing_range.coli = MAX_VIEW_COLUMN(sheet);
    }

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("_gtk_sheet_range_draw: row %d - %d col %d - %d",
	drawing_range.row0, drawing_range.rowi, drawing_range.col0, drawing_range.coli);
#endif

    if (drawing_range.row0 > drawing_range.rowi)
	return;
    if (drawing_range.col0 > drawing_range.coli)
	return;

/*  
   gdk_draw_rectangle (sheet->pixmap,
       GTK_WIDGET(sheet)->style->white_gc,
       TRUE,
       0,0,
       sheet->sheet_window_width,sheet->sheet_window_height);
*/

    /* clear outer area beyond rightmost column */
    if (drawing_range.coli >= MAX_VIEW_COLUMN(sheet))
    {
	gint maxcol = MAX_VIEW_COLUMN(sheet);  /* might not be visible */

	if (maxcol > sheet->maxcol)
	    maxcol = sheet->maxcol;

	while (maxcol >= 0
	    && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, maxcol))) --maxcol;

	if (maxcol >= 0)
	{
	    area.x = _gtk_sheet_column_left_xpixel(sheet, maxcol) +
		COLPTR(sheet, maxcol)->width;
	}
	else
	{
	    area.x = sheet->hoffset;
	    if (sheet->row_titles_visible)
		area.x += sheet->row_title_area.width;
	}
	area.width = sheet->sheet_window_width - area.x;
	area.y = 0;
	area.height = sheet->sheet_window_height;

	if (area.width > 0) /* beware, rightmost column might be partially visible */
	{
#if 0
	    gdk_gc_set_foreground(sheet->fg_gc, &sheet->bg_color);
#else
	    gdk_gc_set_foreground(sheet->fg_gc,
		&gtk_widget_get_style(GTK_WIDGET(sheet))->bg[GTK_STATE_NORMAL]);
#endif

	    gdk_draw_rectangle(sheet->pixmap,
		sheet->fg_gc,
		TRUE,  /* filled */
		area.x, area.y,
		area.width, area.height);

	    gdk_draw_pixmap(sheet->sheet_window,
		gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
		sheet->pixmap,
		area.x, area.y,
		area.x, area.y,
		area.width, area.height);
	}
    }

    /* clear outer area beyond last row */
    if (drawing_range.rowi >= MAX_VIEW_ROW(sheet))
    {
	gint maxrow = MAX_VIEW_ROW(sheet);  /* might not be visible */

	if (maxrow > sheet->maxrow)
	    maxrow = sheet->maxrow;

	while (maxrow >= 0
	    && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, maxrow))) --maxrow;

	area.x = 0;
	area.width = sheet->sheet_window_width;

	if (maxrow >= 0)
	{
	    area.y = _gtk_sheet_row_top_ypixel(sheet, maxrow) +
		ROWPTR(sheet, maxrow)->height;
	}
	else
	{
	    area.y = sheet->voffset;
	    if (sheet->column_titles_visible)
		area.y += sheet->column_title_area.height;
	}
	area.height = sheet->sheet_window_height - area.y;

	if (area.height > 0) /* beware, last row might be partially visible */
	{
#if 0
	    gdk_gc_set_foreground(sheet->fg_gc, &sheet->bg_color);
#else
	    gdk_gc_set_foreground(sheet->fg_gc,
		&gtk_widget_get_style(GTK_WIDGET(sheet))->bg[GTK_STATE_NORMAL]);
#endif

	    gdk_draw_rectangle(sheet->pixmap,
		sheet->fg_gc,
		TRUE,  /* filled */
		area.x, area.y,
		area.width, area.height);

	    gdk_draw_pixmap(sheet->sheet_window,
		gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
		sheet->pixmap,
		area.x, area.y,
		area.x, area.y,
		area.width, area.height);
	}
    }

    /* extend the drawing range to include all text sources */

    if (drawing_range.col0 < 0)
	drawing_range.col0 = 0;
    if (drawing_range.col0 > sheet->maxcol)
	drawing_range.col0 = sheet->maxcol;
    if (drawing_range.coli < 0)
	drawing_range.coli = 0;
    if (drawing_range.coli > sheet->maxcol)
	drawing_range.coli = sheet->maxcol;

    if (!gtk_sheet_clip_text(sheet))  /* text extends multiple cells */
    {
	drawing_range.col0 = MIN_VIEW_COLUMN(sheet);
	drawing_range.coli = MAX_VIEW_COLUMN(sheet);

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("_gtk_sheet_range_draw: extended: row %d - %d col %d - %d",
	drawing_range.row0, drawing_range.rowi, drawing_range.col0, drawing_range.coli);
#endif
    }

    /* draw grid and cells */
    for (row = drawing_range.row0; row <= drawing_range.rowi; row++)
    {
	for (col = drawing_range.col0; col <= drawing_range.coli; col++)
	{
	    _cell_draw_background(sheet, row, col);
	}
    }

    for (row = drawing_range.row0; row <= drawing_range.rowi; row++)
    {
	for (col = drawing_range.col0; col <= drawing_range.coli; col++)
	{
	    _cell_draw_border(sheet, row - 1, col, GTK_SHEET_BOTTOM_BORDER);
	    _cell_draw_border(sheet, row + 1, col, GTK_SHEET_TOP_BORDER);
	    _cell_draw_border(sheet, row, col - 1, GTK_SHEET_RIGHT_BORDER);
	    _cell_draw_border(sheet, row, col + 1, GTK_SHEET_LEFT_BORDER);
	    _cell_draw_border(sheet, row, col, 15);
	}
    }

    /* draw text within range (1) */

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("_gtk_sheet_range_draw: (1) row %d - %d col %d - %d",
	drawing_range.row0, drawing_range.rowi,
	drawing_range.col0, drawing_range.coli);
#endif

    for (row = drawing_range.row0; row <= drawing_range.rowi; row++)
    {
	for (col = drawing_range.col0; col <= drawing_range.coli; col++)
	{
	    if (row <= sheet->maxallocrow && col <= sheet->maxalloccol &&
		sheet->data[row] && sheet->data[row][col])
	    {
		_cell_draw_label(sheet, row, col);
	    }
	}
    }
#if 0
    /* draw text left outside range (2) */

    if (0 <= drawing_range.col0 && drawing_range.col0 <= sheet->maxcol)
    {
#if GTK_SHEET_DEBUG_DRAW > 0
	g_debug("_gtk_sheet_range_draw: (2) row %d - %d col %d - %d",
	    drawing_range.row0, drawing_range.rowi,
	    COLPTR(sheet, drawing_range.col0)->left_text_column, drawing_range.col0 - 1);
#endif

	for (row = drawing_range.row0; row <= drawing_range.rowi; row++)
	{
	    for (col = COLPTR(sheet, drawing_range.col0)->left_text_column;
		col < drawing_range.col0; col++)
	    {
#if GTK_SHEET_DEBUG_DRAW > 0
		g_debug("_gtk_sheet_range_draw: (2) %d %d", row, col);
#endif
		if (row <= sheet->maxallocrow && col <= sheet->maxalloccol &&
		    sheet->data[row] && sheet->data[row][col])
		{
		    _cell_draw_background(sheet, row, col);
		    _cell_draw_label(sheet, row, col);
		}
	    }
	}
    }

    /* draw text right outside range (3) */

    if (0 <= drawing_range.coli && drawing_range.coli <= sheet->maxcol)
    {
#if GTK_SHEET_DEBUG_DRAW > 0
	g_debug("_gtk_sheet_range_draw: (3) row %d - %d col %d - %d",
	    drawing_range.row0, drawing_range.rowi,
	    drawing_range.coli + 1, COLPTR(sheet, drawing_range.coli)->right_text_column);
#endif

	for (row = drawing_range.row0; row <= drawing_range.rowi; row++)
	{
	    for (col = drawing_range.coli + 1;
		col <= COLPTR(sheet, drawing_range.coli)->right_text_column; col++)
	    {
#if GTK_SHEET_DEBUG_DRAW > 0
		g_debug("_gtk_sheet_range_draw: (3) %d %d", row, col);
#endif
		_cell_draw_background(sheet, row, col);

		if (row <= sheet->maxallocrow && col <= sheet->maxalloccol &&
		    sheet->data[row] && sheet->data[row][col])
		{
		    _cell_draw_label(sheet, row, col);
		}
	    }
	}
    }
#endif
    gtk_sheet_draw_backing_pixmap(sheet, drawing_range);

    if (sheet->state != GTK_SHEET_NORMAL &&
	gtk_sheet_range_isvisible(sheet, sheet->range))
    {
	gtk_sheet_range_draw_selection(sheet, drawing_range);
    }

    if (activate_active_cell &&
	sheet->state == GTK_STATE_NORMAL &&
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
    gint i, j;
    GtkSheetRange aux;

    if (range.col0 > sheet->range.coli || range.coli < sheet->range.col0 ||
	range.row0 > sheet->range.rowi || range.rowi < sheet->range.row0)
    {
#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_range_draw_selection: range outside");
#endif
	return;
    }

    if (!gtk_sheet_range_isvisible(sheet, range))
    {
#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_range_draw_selection: range invisible");
#endif
	return;
    }
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    aux = range;

    range.col0 = MAX(sheet->range.col0, range.col0);
    range.coli = MIN(sheet->range.coli, range.coli);
    range.row0 = MAX(sheet->range.row0, range.row0);
    range.rowi = MIN(sheet->range.rowi, range.rowi);

    range.col0 = MAX(range.col0, MIN_VIEW_COLUMN(sheet));
    range.coli = MIN(range.coli, MAX_VIEW_COLUMN(sheet));
    range.row0 = MAX(range.row0, MIN_VIEW_ROW(sheet));
    range.rowi = MIN(range.rowi, MAX_VIEW_ROW(sheet));

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_range_draw_selection: range r %d-%d c %d-%d",
	range.row0, range.rowi, range.col0, range.coli);
#endif

    for (i = range.row0; i <= range.rowi; i++)
    {
	if (i > sheet->maxrow)
	    break;

	for (j = range.col0; j <= range.coli; j++)
	{
	    if (j > sheet->maxcol)
		break;

	    if (gtk_sheet_cell_get_state(sheet, i, j) == GTK_STATE_SELECTED &&
		GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) &&
		GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    {
		row_button_set(sheet, i);
		_gtk_sheet_column_button_set(sheet, j);

		area.x = _gtk_sheet_column_left_xpixel(sheet, j);
		area.y = _gtk_sheet_row_top_ypixel(sheet, i);
		area.width = COLPTR(sheet, j)->width;
		area.height = sheet->row[i].height;

		if (i == sheet->range.row0)
		{
		    area.y = area.y + 2;
		    area.height = area.height - 2;
		}
		if (i == sheet->range.rowi)
		    area.height = area.height - 3;

		if (j == sheet->range.col0)
		{
		    area.x = area.x + 2;
		    area.width = area.width - 2;
		}
		if (j == sheet->range.coli)
		    area.width = area.width - 3;

		if (i != sheet->active_cell.row || j != sheet->active_cell.col)
		{
		    gdk_draw_rectangle(sheet->sheet_window,
			sheet->xor_gc,
			TRUE,
			area.x + 1, area.y + 1,
			area.width, area.height);
		}
	    }

	}
    }

    gtk_sheet_draw_border(sheet, sheet->range);
}

static void
gtk_sheet_draw_backing_pixmap(GtkSheet *sheet, GtkSheetRange range)
{
    gint x, y, width, height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    x = _gtk_sheet_column_left_xpixel(sheet, range.col0);
    y = _gtk_sheet_row_top_ypixel(sheet, range.row0);
    width = _gtk_sheet_column_left_xpixel(sheet, range.coli) - x;
    if (0 <= range.coli && range.coli <= sheet->maxcol)
	width += COLPTR(sheet, range.coli)->width;
    height = _gtk_sheet_row_top_ypixel(sheet, range.rowi) - y;
    if (0 <= range.rowi && range.rowi <= sheet->maxrow)
	height += sheet->row[range.rowi].height;

    if (range.row0 == sheet->range.row0)
    {
	y = y - 5;
	height = height + 5;
    }
    if (range.rowi == sheet->range.rowi)
	height = height + 5;

    if (range.col0 == sheet->range.col0)
    {
	x = x - 5;
	width = width + 5;
    }
    if (range.coli == sheet->range.coli)
	width = width + 5;

    width = MIN(width, sheet->sheet_window_width - x);
    height = MIN(height, sheet->sheet_window_height - y);

    x--;
    y--;
    width += 2;
    height += 2;

    x = (sheet->row_titles_visible) ? MAX(x, sheet->row_title_area.width) : MAX(x, 0);
    y = (sheet->column_titles_visible) ? MAX(y, sheet->column_title_area.height) : MAX(y, 0);

    if (range.coli >= sheet->maxcol)
	width = sheet->sheet_window_width - x;
    if (range.rowi >= sheet->maxrow)
	height = sheet->sheet_window_height - y;

#if GTK_SHEET_DEBUG_EXPOSE > 0
    g_debug("gtk_sheet_draw_backing_pixmap: x %d y %d w %d h %d",
	x, y, width + 1, height + 1);
#endif

    gdk_draw_pixmap(sheet->sheet_window,
	gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	sheet->pixmap,
	x, y,
	x, y,
	width + 1,
	height + 1);
}

static inline void
gtk_sheet_cell_init(GtkSheetCell *cell)
{
    cell->extent.x = cell->extent.y = 0;
    cell->extent.width = cell->extent.height = 0;

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
	    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CLEAR_CELL], 0,
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
	GtkSheetCellAttr *attributes = cell->attributes;

	if (attributes->font_desc && attributes->do_font_desc_free)
	{
	    pango_font_description_free(attributes->font_desc);  /* free */
	    attributes->font_desc = NULL;
	}
	g_free(attributes);

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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

    GtkSheetCellAttr attributes;
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

#if GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    GTimer *tm = g_timer_new();
#endif

    CheckCellData(sheet, row, col);

#if 0 && GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st1: %0.6f", g_timer_elapsed(tm, NULL));
#endif

    cell = sheet->data[row][col];

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);
    attributes.justification = justification;
    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);

    if (cell->text)
    {
	g_free(cell->text);
	cell->text = NULL;
    }

    if (text)
    {
	gchar *dataformat = gtk_sheet_column_get_format(sheet, col);

	if (dataformat)
	    text = gtk_data_format_remove(text, dataformat);

#if GTK_SHEET_DEBUG_SET_CELL_TEXT > 0
	g_debug("gtk_sheet_set_cell[%p]: r %d c %d ar %d ac %d <%s>", 
	    sheet, row, col, sheet->active_cell.row, sheet->active_cell.col, text);
#endif

	cell->text = g_strdup(text); 
    }

#if 0 && GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st2: %0.6f", g_timer_elapsed(tm, NULL));
#endif

    _gtk_sheet_update_extent(sheet, cell, row, col);

#if GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st3: %0.6f", g_timer_elapsed(tm, NULL));
#endif

    if (attributes.is_visible)
    {
	gboolean need_draw = TRUE;

	/* PR#104553 - if sheet entry editor is active on the cell being modified,
	   we need to update it's contents
	   */
	if (row == sheet->active_cell.row && col == sheet->active_cell.col)
	{
#if GTK_SHEET_DEBUG_SET_CELL_TEXT > 0
	    g_debug("gtk_sheet_set_cell[%p]: update sheet entry");
#endif
	    gtk_sheet_set_entry_text(sheet, text);  /* PR#104553 */
	}

	if (gtk_sheet_autoresize(sheet))  /* handle immediate resize */
	{
	    if (cell->text && cell->text[0])
	    {
		if (gtk_sheet_autoresize_columns(sheet))
		{
		    GtkSheetColumn *colptr = COLPTR(sheet, col);
		    gint new_width = COLUMN_EXTENT_TO_WIDTH(colptr->max_extent_width);

		    if (new_width != colptr->width)
		    {
#if GTK_SHEET_DEBUG_SIZE > 0
			g_debug("gtk_sheet_set_cell[%d]: set col width %d", col, new_width);
#endif
			gtk_sheet_set_column_width(sheet, col, new_width);
			GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_REDRAW_PENDING);
			need_draw = FALSE;
		    }
		}

		if (gtk_sheet_autoresize_rows(sheet))
		{
		    GtkSheetRow *rowptr = ROWPTR(sheet, row);
		    gint new_height = ROW_EXTENT_TO_HEIGHT(rowptr->max_extent_height);

		    if (new_height != rowptr->height)
		    {
#if GTK_SHEET_DEBUG_SIZE > 0
			g_debug("gtk_sheet_set_cell[%d]: set row height %d", row, new_height);
#endif
			gtk_sheet_set_row_height(sheet, row, new_height);
			GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_REDRAW_PENDING);
			need_draw = FALSE;
		    }
		}
	    }
	}

	if (need_draw)
	{
	    GtkSheetRange range;

	    range.row0 = row;
	    range.rowi = row;
	    range.col0 = sheet->view.col0;
	    range.coli = sheet->view.coli;

	    if (!GTK_SHEET_IS_FROZEN(sheet))
		_gtk_sheet_range_draw(sheet, &range, TRUE);
	}
    }
#if GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st4: %0.6f", g_timer_elapsed(tm, NULL));
#endif

    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CHANGED], 0, row, col);

#if GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st9: %0.6f", g_timer_elapsed(tm, NULL));
    g_timer_destroy(tm);
#endif
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
gtk_sheet_cell_clear(GtkSheet *sheet, gint row, gint column)
{
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    if (column > sheet->maxcol || row > sheet->maxrow)
	return;
    if (column > sheet->maxalloccol || row > sheet->maxallocrow)
	return;
    if (column < 0 || row < 0)
	return;

    range.row0 = row;
    range.rowi = row;
    range.col0 = sheet->view.col0;
    range.coli = sheet->view.coli;

    gtk_sheet_real_cell_clear(sheet, row, column, FALSE);

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
gtk_sheet_cell_delete(GtkSheet *sheet, gint row, gint column)
{
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    if (column > sheet->maxcol || row > sheet->maxrow)
	return;
    if (column > sheet->maxalloccol || row > sheet->maxallocrow)
	return;
    if (column < 0 || row < 0)
	return;

    range.row0 = row;
    range.rowi = row;
    range.col0 = sheet->view.col0;
    range.coli = sheet->view.coli;

    gtk_sheet_real_cell_clear(sheet, row, column, TRUE);

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
}

static void
gtk_sheet_real_cell_clear(GtkSheet *sheet,
    gint row, gint column,
    gboolean delete)
{
    GtkSheetCell *cell;

    if (row > sheet->maxallocrow || column > sheet->maxalloccol)
	return;

    if (!sheet->data[row])
	return;

    cell = sheet->data[row][column];
    if (!cell)
	return;

    if (cell->text)
    {
	g_free(cell->text);
	cell->text = NULL;

	if (GTK_IS_OBJECT(sheet) && G_OBJECT(sheet)->ref_count > 0)
	    g_signal_emit(GTK_OBJECT(sheet), sheet_signals[CLEAR_CELL], 0, row, column);
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

#if GTK_SHEET_DEBUG_ALLOCATION > 0
	g_debug("gtk_sheet_real_cell_clear: freeing %d %d", row, column); 
#endif

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
gtk_sheet_range_clear(GtkSheet *sheet, const GtkSheetRange *range)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

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
gtk_sheet_range_delete(GtkSheet *sheet, const GtkSheetRange *range)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    gtk_sheet_real_range_clear(sheet, range, TRUE);
}

static void
gtk_sheet_real_range_clear(GtkSheet *sheet, const GtkSheetRange *range,
    gboolean delete)
{
    gint i, j;
    GtkSheetRange clear;

    if (!range)
    {
	clear.row0 = 0;
	clear.rowi = sheet->maxallocrow;
	clear.col0 = 0;
	clear.coli = sheet->maxalloccol;
    }
    else
    {
	clear = *range;
    }

    clear.row0 = MAX(clear.row0, 0);
    clear.col0 = MAX(clear.col0, 0);
    clear.rowi = MIN(clear.rowi, sheet->maxallocrow);
    clear.coli = MIN(clear.coli, sheet->maxalloccol);

    for (i = clear.row0; i <= clear.rowi; i++)
    {
	for (j = clear.col0; j <= clear.coli; j++)
	{
	    gtk_sheet_real_cell_clear(sheet, i, j, delete);
	}
    }

    _gtk_sheet_range_draw(sheet, NULL, TRUE);
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
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col > sheet->maxcol || row > sheet->maxrow)
	return (NULL);
    if (col < 0 || row < 0)
	return (NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol)
	return (NULL);

    if (!sheet->data[row])
	return (NULL);
    if (!sheet->data[row][col])
	return (NULL);
    if (!sheet->data[row][col]->text)
	return (NULL);
    if (!sheet->data[row][col]->text[0])
	return (NULL);

    return (sheet->data[row][col]->text);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

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
 * Returns: (transfer none) pointer linked to the cell
 */
gpointer
gtk_sheet_get_link(GtkSheet *sheet, gint row, gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);
    if (col > sheet->maxcol || row > sheet->maxrow)
	return (NULL);
    if (col < 0 || row < 0)
	return (NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol)
	return (NULL);
    if (!sheet->data[row])
	return (NULL); /* Added by Chris Howell */
    if (!sheet->data[row][col])
	return (NULL); /* Added by Bob Lissner */

    return (sheet->data[row][col]->link);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

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
gtk_sheet_cell_get_state(GtkSheet *sheet, gint row, gint col)
{
    gint state;
    GtkSheetRange *range;

    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);
    if (col > sheet->maxcol || row > sheet->maxrow)
	return (0);
    if (col < 0 || row < 0)
	return (0);

    state = sheet->state;
    range = &sheet->range;

    switch(state)
    {
	case GTK_SHEET_NORMAL:
	    return (GTK_STATE_NORMAL);
	    break;
	case GTK_SHEET_ROW_SELECTED:
	    if (row >= range->row0 && row <= range->rowi)
		return (GTK_STATE_SELECTED);
	    break;
	case GTK_SHEET_COLUMN_SELECTED:
	    if (col >= range->col0 && col <= range->coli)
		return (GTK_STATE_SELECTED);
	    break;
	case GTK_SHEET_RANGE_SELECTED:
	    if (row >= range->row0 && row <= range->rowi &&\
		    col >= range->col0 && col <= range->coli)
		return (GTK_STATE_SELECTED);
	    break;
    }
    return (GTK_STATE_NORMAL);
}

/**
 * gtk_sheet_get_pixel_info:
 * @sheet: a #GtkSheet
 * @window: base window for coordinates (null)
 * @x: x coordinate
 * @y: y coordinate
 * @row: cell row number
 * @column: cell column number
 *
 * Get row and column correspondig to the given position within 
 * the sheet. 
 *  
 * In order to decode clicks into to title area correctly, pass 
 * the GdkWindow from the button event. Omitting the 
 * window (NULL) defaults to the sheet window. 
 *  
 * row and column may return values in the range [-1 .. max+1] 
 * depending on wether the position lies within the title area, 
 * the sheet cell area or beyond the outermost row/column. 
 *  
 * All 9 sheet areas can be reliably determined by evaluating 
 * the returned row/column values (title area/cell 
 * area/outside). 
 *
 * Returns: TRUE if the position lies within the sheet cell area
 * or FALSE when outside (title area f.e.)
 */
gboolean
gtk_sheet_get_pixel_info(GtkSheet *sheet,
    GdkWindow *window, gint x, gint y, gint *row, gint *column)
{
    gint trow, tcol;

    *row = *column = -1;  /* init all output vars */

    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    /* there is a coordinate shift to be considered when
       clicking into a title area because: 
       - the sheet window covers the whole sheet 
       - the column titles window overlaps the sheet window,
          but may be shifted right when row titles are visible
      - the row titles window overlaps the sheet window,
         but may be shifted down when column titles are visible
       */

    if (sheet->column_titles_visible
	&& window == sheet->column_title_window)
    {
	if (sheet->row_titles_visible)
	{
#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
	    g_debug("gtk_sheet_get_pixel_info: shift x");
#endif
	    x += sheet->row_title_area.width;
	}
#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
	g_debug("gtk_sheet_get_pixel_info: r1");
#endif
	trow = -1;
	tcol = _gtk_sheet_column_from_xpixel(sheet, x);
    }
    else if (sheet->row_titles_visible
	&& window == sheet->row_title_window)
    {
	if (sheet->column_titles_visible)
	{
#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
	    g_debug("gtk_sheet_get_pixel_info: shift y");
#endif
	    y += sheet->column_title_area.height;
	}
#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
	g_debug("gtk_sheet_get_pixel_info: c1");
#endif
	trow = _gtk_sheet_row_from_ypixel(sheet, y);
	tcol = -1;
    }
    else if (sheet->column_titles_visible
	     && sheet->row_titles_visible
	     && x < sheet->row_title_area.width
	     && y < sheet->column_title_area.height )
    {
#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
	g_debug("gtk_sheet_get_pixel_info: sb");
#endif
	trow = -1;
	tcol = -1;
    }
    else
    {
#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
	g_debug("gtk_sheet_get_pixel_info: rN/cN");
#endif
	trow = _gtk_sheet_row_from_ypixel(sheet, y);
	tcol = _gtk_sheet_column_from_xpixel(sheet, x);
    }

#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
    g_debug("gtk_sheet_get_pixel_info: x %d y %d row %d col %d", x, y, trow, tcol);
#endif

    *row = trow;
    *column = tcol;

    /* bounds checking, return false if the user clicked
     * on a blank area */

    if (trow < 0 || trow > sheet->maxrow)
	return (FALSE);
    if (tcol < 0 || tcol > sheet->maxcol)
	return (FALSE);

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
gtk_sheet_get_cell_area(GtkSheet *sheet,
    gint row,
    gint col,
    GdkRectangle *area)
{
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    if (row > sheet->maxrow || col > sheet->maxcol)
	return (FALSE);

    area->x = (col == -1) ? 0 : (_gtk_sheet_column_left_xpixel(sheet, col) -
	(sheet->row_titles_visible ? sheet->row_title_area.width : 0));
    area->y = (row == -1) ? 0 : (_gtk_sheet_row_top_ypixel(sheet, row) -
	(sheet->column_titles_visible ? sheet->column_title_area.height : 0));
    area->width = (col == -1) ? sheet->row_title_area.width : COLPTR(sheet, col)->width;
    area->height = (row == -1) ? sheet->column_title_area.height : sheet->row[row].height;

/*
  if(row < 0 || col < 0) return FALSE;

  area->x = COLUMN_LEFT_XPIXEL(sheet, col);
  area->y = ROW_TOP_YPIXEL(sheet, row);
  if(sheet->row_titles_visible)
       area->x -= sheet->row_title_area.width;
  if(sheet->column_titles_visible)
       area->y -= sheet->column_title_area.height;

  area->width = COLPTR(sheet, col)->width;
  area->height = sheet->row[row].height;
*/
    return (TRUE);
}

/**
 * gtk_sheet_set_active_cell:
 * @sheet: a #GtkSheet
 * @row: row number
 * @column: column number
 *
 * Set active cell where the cell entry will be displayed. 
 * Use (row,col) = (-1,-1) to deactivate active cell. 
 *
 * Returns: FALSE if current cell can't be deactivated or
 * requested cell can't be activated
 */
gboolean
gtk_sheet_set_active_cell(GtkSheet *sheet, gint row, gint col)
{
    g_return_val_if_fail(sheet != NULL, 0);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), 0);

    if (row > sheet->maxrow || col > sheet->maxcol)
	return (FALSE);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_set_active_cell: row %d col %d", row, col);
#endif

    if (!gtk_widget_get_can_focus(GTK_WIDGET(sheet)))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("gtk_sheet_set_active_cell: row %d col %d abort: sheet, can-focus false", row, col);
#endif
	return (FALSE);
    }

    if (col >= 0 && !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col))))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("gtk_sheet_set_active_cell: row %d col %d abort: sheet column, can-focus false", row, col);
#endif
	return (FALSE);
    }

    if (col >= 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("gtk_sheet_set_active_cell: row %d col %d abort: sheet column, visible false", row, col);
#endif
	return (FALSE);
    }

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
#if 0
	gint old_row = sheet->active_cell.row;
	gint old_col = sheet->active_cell.col;
#endif

	if (!gtk_sheet_deactivate_cell(sheet))
	{
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	    g_debug("gtk_sheet_set_active_cell: abort: deactivation false");
#endif
	    return (FALSE);
	}

	/* the deactivate signal handler may have activated another cell here */
#if 0
	if ((sheet->active_cell.row != old_row) || (sheet->active_cell.col != old_col))
	{
#ifdef GTK_SHEET_DEBUG
	    g_debug("gtk_sheet_set_active_cell: deactivation moved active cell to row %d col %d",
		    sheet->active_cell.row, sheet->active_cell.col);
#endif
	    return(FALSE);
	}
#endif
    }

    if (row < 0 || col < 0)
    {
	sheet->range.row0 = -1;
	sheet->range.rowi = -1;
	sheet->range.col0 = -1;
	sheet->range.coli = -1;

	return (TRUE);
    }

    sheet->active_cell.row = row;
    sheet->active_cell.col = col;

    if (!gtk_sheet_activate_cell(sheet, row, col))
	return (FALSE);

    _gtk_sheet_move_query(sheet, row, col, TRUE);

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
gtk_sheet_get_active_cell(GtkSheet *sheet, gint *row, gint *column)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    *row = sheet->active_cell.row;
    *column = sheet->active_cell.col;
}

/**
 * gtk_sheet_set_tab_direction:
 * @sheet: a #GtkSheet
 * @dir: the primary tab direction
 *
 * Sets a primary movement direction to the Tab, Return and 
 * Enter keys, and assigns the opposite direction to the same 
 * keys with GDK_SHIFT_MASK. 
 *  
 * Transposed movement direction can be accessed with 
 * GTK_SHEET_MOD_MASK|GDK_CONTROL_MASK and 
 * GTK_SHEET_MOD_MASK|GDK_CONTROL_MASK|GDK_SHIFT_MASK. 
 *  
 * All bindings are defined for the #GtkSheetClass, so all sheet
 * instances use the same movement directions. 
 *  
 * Default: GTK_DIR_TAB_FORWARD.
 *  
 * Since: 3.0.2 
 */
void
gtk_sheet_set_tab_direction(GtkSheet *sheet, GtkDirectionType dir)
{
    GtkSheetClass *klass;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    klass = GTK_SHEET_GET_CLASS(sheet);

    _gtk_sheet_class_init_tab_bindings(klass, dir);
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
    gint row, col;
    char *text;

    g_return_if_fail(data != NULL);
    g_return_if_fail(GTK_IS_SHEET(data));

    sheet = GTK_SHEET(data);

    if (!gtk_widget_get_visible(gtk_sheet_get_entry_widget(sheet)))
	return;
    if (sheet->state != GTK_STATE_NORMAL)
	return;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (row < 0 || col < 0)
	return;

    sheet->active_cell.row = -1;
    sheet->active_cell.col = -1;

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    text = gtk_sheet_get_entry_text(sheet);
    gtk_sheet_set_cell_text(sheet, row, col, text);
    g_free(text);

    if (sheet->freeze_count == 0)
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    sheet->active_cell.row = row;;
    sheet->active_cell.col = col;
}


static gboolean
gtk_sheet_deactivate_cell(GtkSheet *sheet)
{
    gboolean veto = TRUE;
    gint row, col;

    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_deactivate_cell: called, row %d col %d", row, col);
#endif

    if (row < 0 || row > sheet->maxrow)
	return (TRUE);
    if (col < 0 || col > sheet->maxcol)
	return (TRUE);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return (FALSE);
    if (sheet->state != GTK_SHEET_NORMAL)
	return (FALSE);

    gtk_sheet_entry_signal_disconnect_by_func(sheet,
	G_CALLBACK(gtk_sheet_entry_changed_handler));

    _gtk_sheet_hide_active_cell(sheet);

    sheet->active_cell.row = -1;  /* reset before signal emission, to prevent recursion */
    sheet->active_cell.col = -1;

    /* beware: DEACTIVATE handler may call gtk_sheet_set_active_cell()
       */

    _gtkextra_signal_emit(GTK_OBJECT(sheet), sheet_signals[DEACTIVATE],
	row, col, &veto);

    if (!veto)
    {
	sheet->active_cell.row = row;
	sheet->active_cell.col = col;

	return (FALSE);
    }

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_deactivate_cell: running");
#endif


    if (GTK_SHEET_REDRAW_PENDING(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_REDRAW_PENDING);
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
    }

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_deactivate_cell: done row %d col %d", row, col);
#endif

    return (TRUE);
}

/**
 * _gtk_sheet_hide_active_cell:
 * 
 * @param sheet  the #GtkSheet 
 *  
 * hide active cell
 */
void
_gtk_sheet_hide_active_cell(GtkSheet *sheet)
{
    gint row, col;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (!gtk_widget_get_visible(sheet->sheet_entry))
	return;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: called row %d col %d", row, col);
#endif

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;

    if (sheet->freeze_count == 0)
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

#if 0
    /* transferring entry text to the cell gives problems when gtk_sheet_change_entry()
       is called during TRAVERSE signal emission. The text of the already changed
       entry gets written into the deactivated cell */
    {
	char *text = gtk_sheet_get_entry_text(sheet);

	/* todo: compare with existing text, only notify if text changes */
	gtk_sheet_set_cell_text(sheet, row, col, text);

	g_free(text);
    }
#endif

#if 0
    /* why shoud we first set the cursor to the cell we want hide ? */
    g_signal_emit(GTK_OBJECT(sheet),sheet_signals[SET_CELL], 0, row, col);
#endif

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	GtkSheetRange range;

	range.row0 = range.rowi = row;
	range.col0 = range.coli = col;

	_gtk_sheet_range_draw(sheet, &range, FALSE);  /* do not reactivate active cell!!! */
    }
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: _gtk_sheet_column_button_release");
#endif

    _gtk_sheet_column_button_release(sheet, col);
    row_button_release(sheet, row);
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: gtk_widget_unmap");
#endif


    gtk_widget_unmap(sheet->sheet_entry);

    gdk_draw_pixmap(sheet->sheet_window,
	gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	sheet->pixmap,
	_gtk_sheet_column_left_xpixel(sheet, col) - 1,
	_gtk_sheet_row_top_ypixel(sheet, row) - 1,
	_gtk_sheet_column_left_xpixel(sheet, col) - 1,
	_gtk_sheet_row_top_ypixel(sheet, row) - 1,
	COLPTR(sheet, col)->width + 4,
	sheet->row[row].height + 4);

#if 0
    /* why shoud we first set the cursor to the cell we want hide ? */
    gtk_widget_grab_focus(GTK_WIDGET(sheet));
#endif

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: gtk_widget_set_visible");
#endif

    gtk_widget_set_visible(GTK_WIDGET(sheet->sheet_entry), FALSE);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: done");
#endif
}

static gboolean
gtk_sheet_activate_cell(GtkSheet *sheet, gint row, gint col)
{
    gboolean veto = TRUE;

    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (GTK_SHEET_FLAGS(sheet) & GTK_SHEET_IS_DESTROYED) return(FALSE); /* PR#102114 */

    if (row < 0 || col < 0)
	return (FALSE);
    if (row > sheet->maxrow || col > sheet->maxcol)
	return (FALSE);

    if (!gtk_widget_get_can_focus(GTK_WIDGET(sheet)))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("gtk_sheet_activate_cell: row %d col %d abort: sheet, can-focus false", row, col);
#endif
	return (FALSE);
    }

    if (!gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col))))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("gtk_sheet_activate_cell: row %d col %d abort: sheet column, can-focus false", row, col);
#endif
	return (FALSE);
    }

/* 
   _gtkextra_signal_emit(GTK_OBJECT(sheet),sheet_signals[ACTIVATE], row, col, &veto);
    if(!gtk_widget_get_realized(GTK_WIDGET(sheet))) return veto;
    if (!veto) return FALSE;
*/

    if (sheet->state != GTK_SHEET_NORMAL)
    {
	sheet->state = GTK_SHEET_NORMAL;
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
    _gtk_sheet_column_button_set(sheet, col);

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
    gtk_sheet_show_active_cell(sheet);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_activate_cell: signal setup");
#endif

    gtk_sheet_entry_signal_connect_changed(sheet,
	G_CALLBACK(gtk_sheet_entry_changed_handler));

    _gtkextra_signal_emit(GTK_OBJECT(sheet), sheet_signals[ACTIVATE], row, col, &veto);

    return (TRUE);
}

static void _gtk_sheet_entry_preselect(GtkSheet *sheet)
{
    gboolean select_on_focus;
    gboolean editable = TRUE;  /* incomplete - refer to gtkitementry::gtk_entry_grab_focus() */
    gboolean in_click = FALSE;  /* incomplete - refer to gtkitementry::gtk_entry_grab_focus() */

#if GTK_SHEET_DEBUG_MOVE > 0
    g_debug("_gtk_sheet_entry_preselect: called");
#endif

    g_object_get(G_OBJECT(gtk_settings_get_default()),
	"gtk-entry-select-on-focus",
	&select_on_focus,
	NULL);

    if (select_on_focus && editable && !in_click)
    {
	gtk_sheet_entry_select_region(sheet, 0, -1);
    }
}

/**
 * _gtk_sheet_entry_setup: 
 * @sheet:  the #GtkSheet
 * @row:    row index
 * @col:    column index
 * @entry_widget: entry widget 
 * 
 * configure sheet_entry for use within the active cell
 * - justification 
 * - editable
 * - max_length 
 * - wrap_mode 
 * - color attributes
 * - font
 *  
 * this function must not dependant on any text contents.
 */
static void _gtk_sheet_entry_setup(GtkSheet *sheet, gint row, gint col,
    GtkWidget *entry_widget)
{
    GtkJustification justification = GTK_JUSTIFY_LEFT;
    gboolean editable;
    GtkStyle *style;
    GtkSheetColumn *colptr = COLPTR(sheet, col);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_entry_setup: row %d col %d", row, col);
#endif

    if (row < 0 || col < 0)
	return;

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    if (gtk_sheet_justify_entry(sheet))
	justification = attributes.justification;

    editable = !(gtk_sheet_locked(sheet)
	|| !attributes.is_editable
	|| colptr->is_readonly);

    gtk_sheet_set_entry_editable(sheet, editable);

    if (GTK_IS_ITEM_ENTRY(entry_widget))
    {
	GtkItemEntry *item_entry = GTK_ITEM_ENTRY(entry_widget);
	GtkEntry *entry = GTK_ENTRY(entry_widget);

	/* 5.8.2010/fp - the code below has no effect in GTK 2.18.9,
	   a right justified editable will pop to left justification
	   as soon as something gets selected, and will
	   pop back to right aligment, as soon as the
	   cursor ist moved. When this happens, the
	   justification value in the editable is correct. */

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("_gtk_sheet_entry_setup: GtkItemEntry justification %d", justification);
#endif
	gtk_item_entry_set_justification(item_entry, justification);
	gtk_item_entry_set_max_length_bytes(item_entry, colptr->max_length_bytes);

	gtk_entry_set_max_length(entry, colptr->max_length);
    }
    else if (GTK_IS_DATA_TEXT_VIEW(entry_widget))
    {
	GtkDataTextView *data_textview = GTK_DATA_TEXT_VIEW(entry_widget);
	GtkTextView *textview = GTK_TEXT_VIEW(entry_widget);

	gtk_text_view_set_wrap_mode(textview, colptr->wrap_mode);
	gtk_data_text_view_set_max_length(data_textview, colptr->max_length);
	gtk_data_text_view_set_max_length_bytes(data_textview, colptr->max_length_bytes);
    }
    else if (GTK_IS_TEXT_VIEW(entry_widget))
    {
	GtkTextView *textview = GTK_TEXT_VIEW(entry_widget);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("_gtk_sheet_entry_setup: GtkTextView justification %d", justification);
#endif
	gtk_text_view_set_justification(textview, justification);
	gtk_text_view_set_wrap_mode(textview, colptr->wrap_mode);
    }
    else if (GTK_IS_ENTRY(entry_widget))
    {
	GtkEntry *entry = GTK_ENTRY(entry_widget);
	gtk_entry_set_max_length(entry, colptr->max_length);
    }

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_entry_setup: style bg color %s",
	gdk_color_to_string(&attributes.background));
#endif

#if 1
    if (gtk_widget_get_realized(entry_widget))
    {
	style = gtk_widget_get_style(entry_widget);
	if (!style)
	    gtk_widget_ensure_style(entry_widget);  /* why this ?? */
	style = gtk_widget_get_style(entry_widget);

	style = gtk_style_copy(style);

	style->bg[GTK_STATE_NORMAL] = attributes.background;
	style->base[GTK_STATE_NORMAL] = attributes.background;
	style->fg[GTK_STATE_NORMAL] = attributes.foreground;
	style->text[GTK_STATE_NORMAL] = attributes.foreground;

	style->bg[GTK_STATE_ACTIVE] = attributes.background;
	style->base[GTK_STATE_ACTIVE] = attributes.background;
	style->fg[GTK_STATE_ACTIVE] = attributes.foreground;
	style->text[GTK_STATE_ACTIVE] = attributes.foreground;

	pango_font_description_free(style->font_desc);
	style->font_desc = pango_font_description_copy(attributes.font_desc);

	gtk_widget_set_style(entry_widget, style);

	g_object_unref(style); /* 22.06.13/fp */
    }
#endif
}

static void
gtk_sheet_show_active_cell(GtkSheet *sheet)
{
    gchar *text = NULL;
    gchar *old_text;
    gint row, col;
    gboolean is_visible = TRUE;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_show_active_cell: called row %d col %d", row, col);
#endif

    if (row < 0 || col < 0)
	return;
    if (row > sheet->maxrow || col > sheet->maxcol)
	return;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (sheet->state != GTK_SHEET_NORMAL)
	return;
    if (GTK_SHEET_IN_SELECTION(sheet))
	return;
    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

    /* we should send a ENTRY_CHANGE_REQUEST signal here */

    if ((row <= sheet->maxallocrow) && (col <= sheet->maxalloccol)
	&& sheet->data[row])
    {
	GtkSheetCell *cell = sheet->data[row][col];
	if (cell)
	{
	    if (cell->text)
		text = g_strdup(cell->text);
	    if (cell->attributes)
		is_visible = cell->attributes->is_visible;
	}
    }

    GtkWidget *entry_widget = gtk_sheet_get_entry(sheet);

    /* update visibility */

    gtk_widget_set_visible(GTK_WIDGET(sheet->sheet_entry), is_visible);

    if (GTK_IS_ENTRY(entry_widget))
    {
	gtk_entry_set_visibility(GTK_ENTRY(entry_widget), is_visible);
    }

    /* update text  */

    if (!text)
	text = g_strdup("");

    old_text = gtk_sheet_get_entry_text(sheet);

    /* the entry setup must be done before the text assigment,
       otherwise max_length may cause text assignment to fail
       */
    _gtk_sheet_entry_setup(sheet, row, col, entry_widget);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_show_active_cell: old_text <%s> text <%s>", old_text, text);
#endif

    if (!old_text || (old_text[0] != text[0]) || strcmp(old_text, text) != 0)
    {
	gtk_sheet_set_entry_text(sheet, text);
    }

    /* we should send an ENTRY_CONFIGURATION signal here */

    gtk_sheet_entry_set_max_size(sheet);
    _gtk_sheet_entry_size_allocate(sheet);

    gtk_widget_map(sheet->sheet_entry);
    gtk_sheet_draw_active_cell(sheet);

    _gtk_sheet_entry_preselect(sheet);

    gtk_widget_grab_focus(entry_widget);

    g_free(text);
    g_free(old_text);
}

static void
gtk_sheet_draw_active_cell(GtkSheet *sheet)
{
    gint row, col;

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;

    if (!gtk_sheet_cell_isvisible(sheet, row, col))
	return;

    row_button_set(sheet, row);
    _gtk_sheet_column_button_set(sheet, col);

    gtk_sheet_draw_backing_pixmap(sheet, sheet->range);
    gtk_sheet_draw_border(sheet, sheet->range);
}


static void
gtk_sheet_make_backing_pixmap(GtkSheet *sheet, guint width, guint height)
{
    gint pixmap_width, pixmap_height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if (width == 0 && height == 0)
    {
	width = sheet->sheet_window_width + 80;
	height = sheet->sheet_window_height + 80;
    }

    if (!sheet->pixmap)
    {
	/* allocate */
	sheet->pixmap = gdk_pixmap_new(sheet->sheet_window,
	    width, height,
	    -1);
	if (!GTK_SHEET_IS_FROZEN(sheet))
	    _gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
    else
    {
	/* reallocate if sizes don't match */
	gdk_window_get_size(sheet->pixmap, &pixmap_width, &pixmap_height);

	if ((pixmap_width != width) || (pixmap_height != height))
	{
	    g_object_unref(G_OBJECT(sheet->pixmap));
	    sheet->pixmap = gdk_pixmap_new(sheet->sheet_window,
		width, height,
		-1);
	    if (!GTK_SHEET_IS_FROZEN(sheet))
		_gtk_sheet_range_draw(sheet, NULL, TRUE);
	}
    }
}

static void
gtk_sheet_new_selection(GtkSheet *sheet, GtkSheetRange *range)
{
    gint i, j, mask1, mask2;
    gint state, selected;
    gint x, y, width, height;
    GtkSheetRange new_range, aux_range;

    g_return_if_fail(sheet != NULL);

    if (range == NULL)
	range = &sheet->range;

    new_range = *range;

    range->row0 = MIN(range->row0, sheet->range.row0);
    range->rowi = MAX(range->rowi, sheet->range.rowi);
    range->col0 = MIN(range->col0, sheet->range.col0);
    range->coli = MAX(range->coli, sheet->range.coli);

    range->row0 = MAX(range->row0, MIN_VIEW_ROW(sheet));
    range->rowi = MIN(range->rowi, MAX_VIEW_ROW(sheet));
    range->col0 = MAX(range->col0, MIN_VIEW_COLUMN(sheet));
    range->coli = MIN(range->coli, MAX_VIEW_COLUMN(sheet));

    aux_range.row0 = MAX(new_range.row0, MIN_VIEW_ROW(sheet));
    aux_range.rowi = MIN(new_range.rowi, MAX_VIEW_ROW(sheet));
    aux_range.col0 = MAX(new_range.col0, MIN_VIEW_COLUMN(sheet));
    aux_range.coli = MIN(new_range.coli, MAX_VIEW_COLUMN(sheet));

    for (i = range->row0; i <= range->rowi; i++)
    {
	if (i > sheet->maxrow)
	    break;

	for (j = range->col0; j <= range->coli; j++)
	{
	    if (j > sheet->maxcol)
		break;

	    state = gtk_sheet_cell_get_state(sheet, i, j);
	    selected = (i <= new_range.rowi && i >= new_range.row0 &&
		j <= new_range.coli && j >= new_range.col0) ? TRUE : FALSE;

	    if (state == GTK_STATE_SELECTED && selected &&
		GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)) &&
		(i == sheet->range.row0 || i == sheet->range.rowi ||
		    j == sheet->range.col0 || j == sheet->range.coli ||
		    i == new_range.row0 || i == new_range.rowi ||
		    j == new_range.col0 || j == new_range.coli))
	    {

		mask1 = i == sheet->range.row0 ? 1 : 0;
		mask1 = i == sheet->range.rowi ? mask1 + 2 : mask1;
		mask1 = j == sheet->range.col0 ? mask1 + 4 : mask1;
		mask1 = j == sheet->range.coli ? mask1 + 8 : mask1;

		mask2 = i == new_range.row0 ? 1 : 0;
		mask2 = i == new_range.rowi ? mask2 + 2 : mask2;
		mask2 = j == new_range.col0 ? mask2 + 4 : mask2;
		mask2 = j == new_range.coli ? mask2 + 8 : mask2;

		if (mask1 != mask2)
		{
		    x = _gtk_sheet_column_left_xpixel(sheet, j);
		    y = _gtk_sheet_row_top_ypixel(sheet, i);
		    width = _gtk_sheet_column_left_xpixel(sheet, j) - x + COLPTR(sheet, j)->width;
		    height = _gtk_sheet_row_top_ypixel(sheet, i) - y + sheet->row[i].height;

		    if (i == sheet->range.row0)
		    {
			y = y - 3;
			height = height + 3;
		    }
		    if (i == sheet->range.rowi)
			height = height + 3;
		    if (j == sheet->range.col0)
		    {
			x = x - 3;
			width = width + 3;
		    }
		    if (j == sheet->range.coli)
			width = width + 3;

		    gdk_draw_pixmap(sheet->sheet_window,
			gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
			sheet->pixmap,
			x + 1,
			y + 1,
			x + 1,
			y + 1,
			width,
			height);

		    if (i != sheet->active_cell.row || j != sheet->active_cell.col)
		    {
			x = _gtk_sheet_column_left_xpixel(sheet, j);
			y = _gtk_sheet_row_top_ypixel(sheet, i);
			width = _gtk_sheet_column_left_xpixel(sheet, j) - x + COLPTR(sheet, j)->width;
			height = _gtk_sheet_row_top_ypixel(sheet, i) - y + sheet->row[i].height;

			if (i == new_range.row0)
			{
			    y = y + 2;
			    height = height - 2;
			}
			if (i == new_range.rowi)
			    height = height - 3;
			if (j == new_range.col0)
			{
			    x = x + 2;
			    width = width - 2;
			}
			if (j == new_range.coli)
			    width = width - 3;

			gdk_draw_rectangle(sheet->sheet_window,
			    sheet->xor_gc,
			    TRUE,
			    x + 1, y + 1,
			    width, height);
		    }
		}
	    }
	}
    }

    for (i = range->row0; i <= range->rowi; i++)
    {
	for (j = range->col0; j <= range->coli; j++)
	{
	    state = gtk_sheet_cell_get_state(sheet, i, j);
	    selected = (i <= new_range.rowi && i >= new_range.row0 &&
		j <= new_range.coli && j >= new_range.col0) ? TRUE : FALSE;

	    if (state == GTK_STATE_SELECTED && !selected &&
		GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) &&
		GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    {
		x = _gtk_sheet_column_left_xpixel(sheet, j);
		y = _gtk_sheet_row_top_ypixel(sheet, i);
		width = _gtk_sheet_column_left_xpixel(sheet, j) - x + COLPTR(sheet, j)->width;
		height = _gtk_sheet_row_top_ypixel(sheet, i) - y + sheet->row[i].height;

		if (i == sheet->range.row0)
		{
		    y = y - 3;
		    height = height + 3;
		}
		if (i == sheet->range.rowi)
		    height = height + 3;
		if (j == sheet->range.col0)
		{
		    x = x - 3;
		    width = width + 3;
		}
		if (j == sheet->range.coli)
		    width = width + 3;

		gdk_draw_pixmap(sheet->sheet_window,
		    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
		    sheet->pixmap,
		    x + 1,
		    y + 1,
		    x + 1,
		    y + 1,
		    width,
		    height);
	    }
	}
    }

    for (i = range->row0; i <= range->rowi; i++)
    {
	for (j = range->col0; j <= range->coli; j++)
	{
	    state = gtk_sheet_cell_get_state(sheet, i, j);
	    selected = (i <= new_range.rowi && i >= new_range.row0 &&
		j <= new_range.coli && j >= new_range.col0) ? TRUE : FALSE;

	    if (state != GTK_STATE_SELECTED && selected &&
		GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)) &&
		(i != sheet->active_cell.row || j != sheet->active_cell.col))
	    {
		x = _gtk_sheet_column_left_xpixel(sheet, j);
		y = _gtk_sheet_row_top_ypixel(sheet, i);
		width = _gtk_sheet_column_left_xpixel(sheet, j) - x + COLPTR(sheet, j)->width;
		height = _gtk_sheet_row_top_ypixel(sheet, i) - y + sheet->row[i].height;

		if (i == new_range.row0)
		{
		    y = y + 2;
		    height = height - 2;
		}
		if (i == new_range.rowi)
		    height = height - 3;
		if (j == new_range.col0)
		{
		    x = x + 2;
		    width = width - 2;
		}
		if (j == new_range.coli)
		    width = width - 3;

		gdk_draw_rectangle(sheet->sheet_window,
		    sheet->xor_gc,
		    TRUE,
		    x + 1, y + 1,
		    width, height);
	    }
	}
    }

    for (i = aux_range.row0; i <= aux_range.rowi; i++)
    {
	for (j = aux_range.col0; j <= aux_range.coli; j++)
	{
	    if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, j)) && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    {
		state = gtk_sheet_cell_get_state(sheet, i, j);

		mask1 = i == sheet->range.row0 ? 1 : 0;
		mask1 = i == sheet->range.rowi ? mask1 + 2 : mask1;
		mask1 = j == sheet->range.col0 ? mask1 + 4 : mask1;
		mask1 = j == sheet->range.coli ? mask1 + 8 : mask1;

		mask2 = i == new_range.row0 ? 1 : 0;
		mask2 = i == new_range.rowi ? mask2 + 2 : mask2;
		mask2 = j == new_range.col0 ? mask2 + 4 : mask2;
		mask2 = j == new_range.coli ? mask2 + 8 : mask2;
		if (mask2 != mask1 || (mask2 == mask1 && state != GTK_STATE_SELECTED))
		{
		    x = _gtk_sheet_column_left_xpixel(sheet, j);
		    y = _gtk_sheet_row_top_ypixel(sheet, i);
		    width = COLPTR(sheet, j)->width;
		    height = sheet->row[i].height;
		    if (mask2 & 1)
			gdk_draw_rectangle(sheet->sheet_window,
			    sheet->xor_gc,
			    TRUE,
			    x + 1, y - 1,
			    width, 3);

		    if (mask2 & 2)
			gdk_draw_rectangle(sheet->sheet_window,
			    sheet->xor_gc,
			    TRUE,
			    x + 1, y + height - 1,
			    width, 3);

		    if (mask2 & 4)
			gdk_draw_rectangle(sheet->sheet_window,
			    sheet->xor_gc,
			    TRUE,
			    x - 1, y + 1,
			    3, height);

		    if (mask2 & 8)
			gdk_draw_rectangle(sheet->sheet_window,
			    sheet->xor_gc,
			    TRUE,
			    x + width - 1, y + 1,
			    3, height);
		}
	    }
	}
    }

    *range = new_range;
    gtk_sheet_draw_corners(sheet, new_range);
}

static void
gtk_sheet_draw_border(GtkSheet *sheet, GtkSheetRange new_range)
{
    GdkRectangle clip_area, area;
    gint i;

    area.x = _gtk_sheet_column_left_xpixel(sheet, new_range.col0);
    area.y = _gtk_sheet_row_top_ypixel(sheet, new_range.row0);
    area.width = _gtk_sheet_column_right_xpixel(sheet, new_range.coli) - area.x;
    area.height = _gtk_sheet_row_bottom_ypixel(sheet, new_range.rowi) - area.y;

    clip_area.x = sheet->row_title_area.width;
    clip_area.y = sheet->column_title_area.height;
    clip_area.width = sheet->sheet_window_width;
    clip_area.height = sheet->sheet_window_height;

    if (!sheet->row_titles_visible)
	clip_area.x = 0;
    if (!sheet->column_titles_visible)
	clip_area.y = 0;

    if (area.x < 0)
    {
	area.width = area.width + area.x;
	area.x = 0;
    }
    if (area.width > clip_area.width)
	area.width = clip_area.width + 10;
    if (area.y < 0)
    {
	area.height = area.height + area.y;
	area.y = 0;
    }
    if (area.height > clip_area.height)
	area.height = clip_area.height + 10;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_draw_border: clip_area (%d, %d, %d, %d)",
	clip_area.x, clip_area.y, clip_area.width, clip_area.height);
#endif

    clip_area.x--;
    clip_area.y--;
    clip_area.width += 3;
    clip_area.height += 3;

    gdk_gc_set_clip_rectangle(sheet->xor_gc, &clip_area);

    for (i = -1; i <= 1; ++i)
    {
	gdk_draw_rectangle(sheet->sheet_window,
	    sheet->xor_gc,
	    FALSE,
	    area.x + i, area.y + i,
	    area.width - 2 * i, area.height - 2 * i);
    }

    gdk_gc_set_clip_rectangle(sheet->xor_gc, NULL);
    gtk_sheet_draw_corners(sheet, new_range);
}

static void
gtk_sheet_draw_corners(GtkSheet *sheet, GtkSheetRange range)
{
    gint x, y;
    guint width = 1;

    if (gtk_sheet_cell_isvisible(sheet, range.row0, range.col0))
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.col0);
	y = _gtk_sheet_row_top_ypixel(sheet, range.row0);
	gdk_draw_pixmap(sheet->sheet_window,
	    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	    sheet->pixmap,
	    x - 1,
	    y - 1,
	    x - 1,
	    y - 1,
	    3,
	    3);
	gdk_draw_rectangle(sheet->sheet_window,
	    sheet->xor_gc,
	    TRUE,
	    x - 1, y - 1,
	    3, 3);
    }

    if (gtk_sheet_cell_isvisible(sheet, range.row0, range.coli) ||
	sheet->state == GTK_SHEET_COLUMN_SELECTED)
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.coli) +
	    COLPTR(sheet, range.coli)->width;
	y = _gtk_sheet_row_top_ypixel(sheet, range.row0);
	width = 1;
	if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
	{
	    y = _gtk_sheet_row_top_ypixel(sheet, sheet->view.row0) + 3;
	    width = 3;
	}
	gdk_draw_pixmap(sheet->sheet_window,
	    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	    sheet->pixmap,
	    x - width,
	    y - width,
	    x - width,
	    y - width,
	    2 * width + 1,
	    2 * width + 1);
	gdk_draw_rectangle(sheet->sheet_window,
	    sheet->xor_gc,
	    TRUE,
	    x - width + width / 2, y - width + width / 2,
	    2 + width, 2 + width);
    }

    if (gtk_sheet_cell_isvisible(sheet, range.rowi, range.col0) ||
	sheet->state == GTK_SHEET_ROW_SELECTED)
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.col0);
	y = _gtk_sheet_row_top_ypixel(sheet, range.rowi) +
	    sheet->row[range.rowi].height;
	width = 1;
	if (sheet->state == GTK_SHEET_ROW_SELECTED)
	{
	    x = _gtk_sheet_column_left_xpixel(sheet, sheet->view.col0) + 3;
	    width = 3;
	}
	gdk_draw_pixmap(sheet->sheet_window,
	    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	    sheet->pixmap,
	    x - width,
	    y - width,
	    x - width,
	    y - width,
	    2 * width + 1,
	    2 * width + 1);
	gdk_draw_rectangle(sheet->sheet_window,
	    sheet->xor_gc,
	    TRUE,
	    x - width + width / 2, y - width + width / 2,
	    2 + width, 2 + width);
    }

    if (gtk_sheet_cell_isvisible(sheet, range.rowi, range.coli))
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.coli) +
	    COLPTR(sheet, range.coli)->width;
	y = _gtk_sheet_row_top_ypixel(sheet, range.rowi) +
	    sheet->row[range.rowi].height;
	width = 1;
	if (sheet->state == GTK_SHEET_RANGE_SELECTED)
	    width = 3;
	if (sheet->state == GTK_SHEET_NORMAL)
	    width = 3;
	gdk_draw_pixmap(sheet->sheet_window,
	    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
	    sheet->pixmap,
	    x - width,
	    y - width,
	    x - width,
	    y - width,
	    2 * width + 1,
	    2 * width + 1);
	gdk_draw_rectangle(sheet->sheet_window,
	    sheet->xor_gc,
	    TRUE,
	    x - width + width / 2, y - width + width / 2,
	    2 + width, 2 + width);

    }

}


static void
gtk_sheet_real_select_range(GtkSheet *sheet, GtkSheetRange *range)
{
    gint i;
    gint state;

    g_return_if_fail(sheet != NULL);

    if (range == NULL)
	range = &sheet->range;

    if (range->row0 < 0 || range->rowi < 0)
	return;
    if (range->col0 < 0 || range->coli < 0)
	return;

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_real_select_range: {%d, %d, %d, %d}",
	range->row0, range->col0, range->rowi, range->coli);
#endif

    state = sheet->state;

    if (state == GTK_SHEET_COLUMN_SELECTED || state == GTK_SHEET_RANGE_SELECTED)
    {
	for (i = sheet->range.col0; i < range->col0; i++) _gtk_sheet_column_button_release(sheet, i);

	for (i = range->coli + 1; i <= sheet->range.coli; i++) _gtk_sheet_column_button_release(sheet, i);

	for (i = range->col0; i <= range->coli; i++)
	{
	    _gtk_sheet_column_button_set(sheet, i);
	}
    }

    if (state == GTK_SHEET_ROW_SELECTED || state == GTK_SHEET_RANGE_SELECTED)
    {
	for (i = sheet->range.row0; i < range->row0; i++) row_button_release(sheet, i);

	for (i = range->rowi + 1; i <= sheet->range.rowi; i++) row_button_release(sheet, i);

	for (i = range->row0; i <= range->rowi; i++)
	{
	    row_button_set(sheet, i);
	}
    }

    if (range->coli != sheet->range.coli || range->col0 != sheet->range.col0 ||
	range->rowi != sheet->range.rowi || range->row0 != sheet->range.row0)
    {
	gtk_sheet_new_selection(sheet, range);

	sheet->range.col0 = range->col0;
	sheet->range.coli = range->coli;
	sheet->range.row0 = range->row0;
	sheet->range.rowi = range->rowi;
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
gtk_sheet_select_range(GtkSheet *sheet, const GtkSheetRange *range)
{
    GtkSheetRange new_range;  /* buffer needed because gtk_sheet_real_unselect_range() will clear it */

    g_return_if_fail(sheet != NULL);

    if (!range)
	range = &sheet->range;

    new_range = *range;

    if (new_range.row0 < 0 || new_range.rowi < 0)
	return;
    if (new_range.col0 < 0 || new_range.coli < 0)
	return;

    if (sheet->state != GTK_SHEET_NORMAL)
    {
	/* this will clear sheet->range */
	gtk_sheet_real_unselect_range(sheet, NULL);
    }
    else
    {
	gboolean veto = TRUE;
	veto = gtk_sheet_deactivate_cell(sheet);
	if (!veto)
	    return;
    }

    sheet->range.row0 = new_range.row0;
    sheet->range.rowi = new_range.rowi;
    sheet->range.col0 = new_range.col0;
    sheet->range.coli = new_range.coli;

    sheet->active_cell.row = new_range.row0;
    sheet->active_cell.col = new_range.col0;

    sheet->selection_cell.row = new_range.rowi;
    sheet->selection_cell.col = new_range.coli;

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
gtk_sheet_unselect_range(GtkSheet *sheet)
{
    gtk_sheet_real_unselect_range(sheet, NULL);
    sheet->state = GTK_SHEET_NORMAL;
    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
}


static void
gtk_sheet_real_unselect_range(GtkSheet *sheet, GtkSheetRange *range)
{
    gint i;

    g_return_if_fail(sheet != NULL);
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if (!range)
	range = &sheet->range;

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_real_unselect_range: called {%d, %d, %d, %d}",
	range->row0, range->col0, range->rowi, range->coli);
#endif

    if (range->row0 < 0 || range->rowi < 0)
	return;
    if (range->col0 < 0 || range->coli < 0)
	return;

    if (gtk_sheet_range_isvisible(sheet, *range))
    {
#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_real_unselect_range: gtk_sheet_draw_backing_pixmap");
#endif
	gtk_sheet_draw_backing_pixmap(sheet, *range);
    }

    for (i = range->col0; i <= range->coli; i++)
    {
#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_real_unselect_range: _gtk_sheet_column_button_release(%d)", i);
#endif
	_gtk_sheet_column_button_release(sheet, i);
    }

    for (i = range->row0; i <= range->rowi; i++)
    {
#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_real_unselect_range: row_button_release(%d)", i);
#endif
	row_button_release(sheet, i);
    }

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_real_unselect_range: gtk_sheet_position_children()");
#endif
    gtk_sheet_position_children(sheet);

    /* reset range */
    range->row0 = range->rowi = range->col0 = range->coli = -1;
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
gtk_sheet_expose_handler(GtkWidget *widget, GdkEventExpose *event)
{
    GtkSheet *sheet;
    gint i;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    sheet = GTK_SHEET(widget);

    if (gtk_widget_is_drawable(widget))
    {
#if GTK_SHEET_DEBUG_EXPOSE > 0
	g_debug("gtk_sheet_expose_handler: called");
#endif

	if (event->window == sheet->row_title_window && sheet->row_titles_visible)
	{
#if GTK_SHEET_DEBUG_EXPOSE > 0
	    g_debug("gtk_sheet_expose_handler: row buttons");
#endif
	    for (i = MIN_VIEW_ROW(sheet); i <= MAX_VIEW_ROW(sheet) && i <= sheet->maxrow; i++)
	    {
		_gtk_sheet_draw_button(sheet, i, -1);
	    }
	}

	if (event->window == sheet->column_title_window && sheet->column_titles_visible)
	{
#if GTK_SHEET_DEBUG_EXPOSE > 0
	    g_debug("gtk_sheet_expose_handler: column buttons");
#endif
	    for (i = MIN_VIEW_COLUMN(sheet); i <= MAX_VIEW_COLUMN(sheet) && i <= sheet->maxcol; i++)
	    {
		_gtk_sheet_draw_button(sheet, -1, i);
	    }
	}

	if (event->window == sheet->sheet_window)
	{
	    GtkSheetRange range;

	    range.row0 = _gtk_sheet_row_from_ypixel(sheet, event->area.y);
	    range.col0 = _gtk_sheet_column_from_xpixel(sheet, event->area.x);
	    range.rowi = _gtk_sheet_row_from_ypixel(sheet, event->area.y + event->area.height);
	    range.coli = _gtk_sheet_column_from_xpixel(sheet, event->area.x + event->area.width);

#if GTK_SHEET_DEBUG_EXPOSE > 0
	    g_debug("gtk_sheet_expose_handler: backing pixmap (%d,%d) (%d,%d)",
		range.row0, range.col0, range.rowi, range.coli);
#endif

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

    (*GTK_WIDGET_CLASS(sheet_parent_class)->expose_event)(widget, event);

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
gtk_sheet_button_press_handler(GtkWidget *widget, GdkEventButton *event)
{
    GtkSheet *sheet;
    GdkModifierType mods;
    gint x, y, row, column;
    gboolean veto;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);
/*
    if(event->type != GDK_BUTTON_PRESS) return(TRUE);
*/

#if GTK_SHEET_DEBUG_MOUSE > 0
    g_debug("gtk_sheet_button_press_handler: called");
#endif

    gdk_window_get_pointer(gtk_widget_get_window(widget),
	NULL, NULL, &mods);

    if (!(mods & GDK_BUTTON1_MASK))
	return (TRUE);

    sheet = GTK_SHEET(widget);

    /* press on resize windows */
    if (event->window == sheet->column_title_window && gtk_sheet_columns_resizable(sheet))
    {
	gtk_widget_get_pointer(widget, &sheet->x_drag, NULL);
	if (POSSIBLE_XDRAG(sheet, sheet->x_drag, &sheet->drag_cell.col))
	{
	    guint req;
	    if (event->type == GDK_2BUTTON_PRESS)
	    {
		_gtk_sheet_autoresize_column_internal(sheet, sheet->drag_cell.col);
		GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
		return (TRUE);
	    }
	    _gtk_sheet_column_size_request(sheet, sheet->drag_cell.col, &req);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
	    gdk_pointer_grab(sheet->column_title_window, FALSE,
		GDK_POINTER_MOTION_HINT_MASK |
		    GDK_BUTTON1_MOTION_MASK |
		    GDK_BUTTON_RELEASE_MASK,
		NULL, NULL, event->time);

	    draw_xor_vline(sheet);
	    return (TRUE);
	}
    }

    if (event->window == sheet->row_title_window && gtk_sheet_rows_resizable(sheet))
    {
	gtk_widget_get_pointer(widget, NULL, &sheet->y_drag);

	if (POSSIBLE_YDRAG(sheet, sheet->y_drag, &sheet->drag_cell.row))
	{
	    guint req;
	    gtk_sheet_row_size_request(sheet, sheet->drag_cell.row, &req);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_YDRAG);
	    gdk_pointer_grab(sheet->row_title_window, FALSE,
		GDK_POINTER_MOTION_HINT_MASK |
		    GDK_BUTTON1_MOTION_MASK |
		    GDK_BUTTON_RELEASE_MASK,
		NULL, NULL, event->time);

	    draw_xor_hline(sheet);
	    return (TRUE);
	}
    }

    /* the sheet itself does not handle other than single click events */
    if (event->type != GDK_BUTTON_PRESS)
	return (FALSE);

    /* selections on the sheet */
    if (event->window == sheet->sheet_window)
    {
#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug("gtk_sheet_button_press_handler: on sheet");
#endif

	gtk_widget_get_pointer(widget, &x, &y);
	gtk_sheet_get_pixel_info(sheet, NULL, x, y, &row, &column);
	if (row < 0 && column < 0) return(FALSE);  /* chain up to global button press handler*/

#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug("gtk_sheet_button_press_handler: pointer grab (%d,%d) r %d c %d mr %d mc %d", 
	    x, y, row, column, sheet->maxrow, sheet->maxcol
	    );
#endif
	gdk_pointer_grab(sheet->sheet_window, FALSE,
	    GDK_POINTER_MOTION_HINT_MASK |
		GDK_BUTTON1_MOTION_MASK |
		GDK_BUTTON_RELEASE_MASK,
	    NULL, NULL, event->time);
	gtk_grab_add(GTK_WIDGET(sheet));
	sheet->timer = g_timeout_add_full(0, TIMEOUT_SCROLL, _gtk_sheet_scroll_to_pointer, sheet, NULL);
#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug("gtk_sheet_button_press_handler: grab focus");
#endif
	gtk_widget_grab_focus(GTK_WIDGET(sheet));

	if (sheet->selection_mode != GTK_SELECTION_SINGLE &&
	    gdk_cursor_get_cursor_type(sheet->cursor_drag) == GDK_SIZING &&
	    !GTK_SHEET_IN_SELECTION(sheet) && !GTK_SHEET_IN_RESIZE(sheet))
	{
	    if (sheet->state == GTK_STATE_NORMAL)
	    {
		gint row = sheet->active_cell.row;  /* PR#203012 */
		gint column = sheet->active_cell.col;  /* PR#203012 */

		if (!gtk_sheet_deactivate_cell(sheet))
		    return (FALSE);
		sheet->active_cell.row = row;
		sheet->active_cell.col = column;
		sheet->drag_range = sheet->range;
		sheet->state = GTK_SHEET_RANGE_SELECTED;
		gtk_sheet_select_range(sheet, &sheet->drag_range);
	    }
	    sheet->x_drag = x;
	    sheet->y_drag = y;
	    if (row > sheet->range.rowi)
		row--;
	    if (column > sheet->range.coli)
		column--;
	    sheet->drag_cell.row = row;
	    sheet->drag_cell.col = column;
	    sheet->drag_range = sheet->range;
	    draw_xor_rectangle(sheet, sheet->drag_range);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_RESIZE);
	}
	else if (gdk_cursor_get_cursor_type(sheet->cursor_drag) == GDK_TOP_LEFT_ARROW &&
	    !GTK_SHEET_IN_SELECTION(sheet) && !GTK_SHEET_IN_DRAG(sheet))
	{
	    if (sheet->state == GTK_STATE_NORMAL)
	    {
		gint row = sheet->active_cell.row;  /* PR#203012 */
		gint column = sheet->active_cell.col;  /* PR#203012 */

		if (!gtk_sheet_deactivate_cell(sheet))
		    return (FALSE);
		sheet->active_cell.row = row;
		sheet->active_cell.col = column;
		sheet->drag_range = sheet->range;
		sheet->state = GTK_SHEET_RANGE_SELECTED;
		gtk_sheet_select_range(sheet, &sheet->drag_range);
	    }
	    sheet->x_drag = x;
	    sheet->y_drag = y;
	    if (row < sheet->range.row0)
		row++;
	    if (row > sheet->range.rowi)
		row--;
	    if (column < sheet->range.col0)
		column++;
	    if (column > sheet->range.coli)
		column--;
	    sheet->drag_cell.row = row;
	    sheet->drag_cell.col = column;
	    sheet->drag_range = sheet->range;
#if GTK_SHEET_DEBUG_MOUSE > 0
	    g_debug("gtk_sheet_button_press_handler: drag_range r %d c %d (%d,%d, %d, %d) mr %d mc %d", 
		sheet->drag_cell.row, sheet->drag_cell.col, 
		sheet->drag_range.row0, sheet->drag_range.rowi, 
		sheet->drag_range.col0, sheet->drag_range.coli, 
		sheet->maxrow, sheet->maxcol
	    );
#endif
	    draw_xor_rectangle(sheet, sheet->drag_range);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_DRAG);
	}
	else
	{
#if GTK_SHEET_DEBUG_MOUSE > 0
	    g_debug("gtk_sheet_button_press_handler: on click cell");
#endif

	    gtk_sheet_click_cell(sheet, row, column, &veto);
	    if (veto)
		GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	}
	return(TRUE);
    }

    if (event->window == sheet->column_title_window)
    {
	gtk_widget_get_pointer(widget, &x, &y);
	column = _gtk_sheet_column_from_xpixel(sheet, x);
	if (column < 0 || column > sheet->maxcol)
	    return (FALSE);

	if (GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)))
	{
	    gtk_sheet_click_cell(sheet, -1, column, &veto);
	    gtk_grab_add(GTK_WIDGET(sheet));
	    sheet->timer = g_timeout_add_full(0, TIMEOUT_SCROLL, _gtk_sheet_scroll_to_pointer, sheet, NULL);
	    gtk_widget_grab_focus(GTK_WIDGET(sheet));
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	}
    }

    if (event->window == sheet->row_title_window)
    {
	gtk_widget_get_pointer(widget, &x, &y);
	row = _gtk_sheet_row_from_ypixel(sheet, y);
	if (row < 0 || row > sheet->maxrow)
	    return (FALSE);

	if (GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row)))
	{
	    gtk_sheet_click_cell(sheet, row, -1, &veto);
	    gtk_grab_add(GTK_WIDGET(sheet));
	    sheet->timer = g_timeout_add_full(0, TIMEOUT_SCROLL, _gtk_sheet_scroll_to_pointer, sheet, NULL);
	    gtk_widget_grab_focus(GTK_WIDGET(sheet));
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	}
    }

    return (TRUE);
}

static gint
_gtk_sheet_scroll_to_pointer(gpointer data)
{
    GtkSheet *sheet = GTK_SHEET(data);
    gint x, y, row, column;
    gint move = TRUE;

    GDK_THREADS_ENTER();

    gtk_widget_get_pointer(GTK_WIDGET(sheet), &x, &y);
    gtk_sheet_get_pixel_info(sheet, NULL, x, y, &row, &column);

    if (GTK_SHEET_IN_SELECTION(sheet))
    {
	GtkSheetRange visr;

	/* beware of invisible columns/rows */
	if (!_gtk_sheet_get_visible_range(sheet, &visr))
	    return (TRUE);

	if (_POINT_IN_RANGE(row, column, &visr))
	{
	    gtk_sheet_extend_selection(sheet, row, column);
	}
    }

    if (GTK_SHEET_IN_DRAG(sheet) || GTK_SHEET_IN_RESIZE(sheet))
    {
	move = _gtk_sheet_move_query(sheet, row, column, FALSE);
	if (move)
	    draw_xor_rectangle(sheet, sheet->drag_range);
    }

    GDK_THREADS_LEAVE();
    return (TRUE);
}

static void
gtk_sheet_click_cell(GtkSheet *sheet, gint row, gint col, gboolean *veto)
{
    *veto = TRUE;

#if GTK_SHEET_DEBUG_CLICK > 0
    g_debug("gtk_sheet_click_cell: called, row %d col %d", row, col);
#endif

    /* allow row,col < 0 here, see below */
    if (row > sheet->maxrow || col > sheet->maxcol)
    {
	*veto = FALSE;
	return;
    }

    if (col >= 0 && row >= 0)
    {
	if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) ||
	    !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	{
	    *veto = FALSE;
	    return;
	}
    }

    /* if we do not grab focus here, some entry widgets (i.e. GtkSpinButton)
       will not format contents correctly on field exit */

    gtk_widget_grab_focus(GTK_WIDGET(sheet));

    _gtkextra_signal_emit(GTK_OBJECT(sheet), sheet_signals[TRAVERSE],
	sheet->active_cell.row, sheet->active_cell.col,
	&row, &col, veto);

    if (!*veto)
    {
	if (sheet->state == GTK_STATE_NORMAL)
	    return;

	row = sheet->active_cell.row;
	col = sheet->active_cell.col;
	gtk_sheet_activate_cell(sheet, row, col);
	return;
    }

    if (row == -1 && col >= 0)  /* column button clicked */
    {
	if (gtk_sheet_autoscroll(sheet))
	    _gtk_sheet_move_query(sheet, row, col, FALSE);

	gtk_sheet_select_column(sheet, col);
	return;
    }
    if (col == -1 && row >= 0)  /* row button clicked */
    {
	if (gtk_sheet_autoscroll(sheet))
	    _gtk_sheet_move_query(sheet, row, col, FALSE);

	gtk_sheet_select_row(sheet, row);
	return;
    }

    if (row == -1 && col == -1)  /* global button clicked */
    {
	sheet->range.row0 = 0;
	sheet->range.col0 = 0;
	sheet->range.rowi = sheet->maxrow;
	sheet->range.coli = sheet->maxcol;

	sheet->active_cell.row = 0;
	sheet->active_cell.col = 0;

	if (sheet->state != GTK_STATE_NORMAL)   /* if any range is selected, clear it */
	    gtk_sheet_unselect_range(sheet);
	else
	    gtk_sheet_select_range(sheet, NULL);

	return;
    }

    if (row != -1 && col != -1)  /* sheet cell clicked */
    {
	GtkSheetColumn *colp = COLPTR(sheet, col);

	if (!gtk_widget_get_can_focus(GTK_WIDGET(sheet)))
	{
#if GTK_SHEET_DEBUG_CLICK > 0
	    g_debug("gtk_sheet_click_cell: row %d col %d VETO: sheet, can-focus false", row, col);
#endif
	    *veto = FALSE;
	    return;
	}

	if (!gtk_widget_get_can_focus(GTK_WIDGET(colp)))
	{
#if GTK_SHEET_DEBUG_CLICK > 0
	    g_debug("gtk_sheet_click_cell: row %d col %d VETO: sheet column, can-focus false", row, col);
#endif
	    *veto = FALSE;
	    return;
	}

	if (sheet->state != GTK_SHEET_NORMAL)
	{
	    sheet->state = GTK_SHEET_NORMAL;
	    gtk_sheet_real_unselect_range(sheet, NULL);
	}
	else
	{
#if GTK_SHEET_DEBUG_CLICK > 0
	    g_debug("gtk_sheet_click_cell: row %d col %d calling deactivate", row, col);
#endif
	    if (!gtk_sheet_deactivate_cell(sheet))
	    {
#if GTK_SHEET_DEBUG_CLICK > 0
		g_debug("gtk_sheet_click_cell: row %d col %d VETO: deactivate false", row, col);
#endif
		*veto = FALSE;
		return;
	    }
#if GTK_SHEET_DEBUG_CLICK > 0
	    g_debug("gtk_sheet_click_cell: row %d col %d back from deactivate", row, col);
#endif
	}

	/* auto switch column entry_type */
	/* not sure wether to move this code to gtk_sheet_show_active_cell() */
	{
	    GType installed_entry_type = sheet->installed_entry_type;
	    GType wanted_type =
		(colp->entry_type != G_TYPE_NONE) ? colp->entry_type : sheet->entry_type;

	    if (installed_entry_type != wanted_type)
	    {
		if (sheet->state == GTK_SHEET_NORMAL)
		    _gtk_sheet_hide_active_cell(sheet);

		create_sheet_entry(sheet, wanted_type ? wanted_type : G_TYPE_NONE);
	    }
	}

	/* DEACTIVATE handler might have called gtk_sheet_set_active_cell(),
	   so wie leave it, if it was changed
	   */
#if 0
	{
	    gint act_row = sheet->active_cell.row;
	    gint act_col = sheet->active_cell.col;

	    if (act_row != -1 && act_col != -1 &&
		(sheet->active_cell.row != row || sheet->active_cell.col != col))
	    {
		row = sheet->active_cell.row;
		col = sheet->active_cell.col;
	    }
	}
#endif

	if (gtk_sheet_autoscroll(sheet))
	    _gtk_sheet_move_query(sheet, row, col, TRUE);

	sheet->active_cell.row = row;
	sheet->active_cell.col = col;

	sheet->selection_cell.row = row;
	sheet->selection_cell.col = col;

	sheet->range.row0 = row;
	sheet->range.col0 = col;
	sheet->range.rowi = row;
	sheet->range.coli = col;

	sheet->state = GTK_SHEET_NORMAL;

	GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gtk_sheet_draw_active_cell(sheet);
	return;
    }

    g_assert_not_reached();
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
gtk_sheet_button_release_handler(GtkWidget *widget, GdkEventButton *event)
{
    GtkSheet *sheet;
    gint x, y;

    sheet = GTK_SHEET(widget);

    /* release on resize windows */
    if (GTK_SHEET_IN_XDRAG(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gtk_widget_get_pointer(widget, &x, NULL);
	gdk_pointer_ungrab(event->time);
	draw_xor_vline(sheet);

#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("gtk_sheet_button_release_handler[%d]: set width %d",
	    sheet->drag_cell.col, new_column_width(sheet, sheet->drag_cell.col, &x));
#endif
	gtk_sheet_set_column_width(sheet,
	    sheet->drag_cell.col, new_column_width(sheet, sheet->drag_cell.col, &x));

	sheet->old_hadjustment = -1.;

	if (sheet->hadjustment)
	{
	    g_signal_emit_by_name(GTK_OBJECT(sheet->hadjustment),  "value_changed");
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_YDRAG(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_YDRAG);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gtk_widget_get_pointer(widget, NULL, &y);
	gdk_pointer_ungrab(event->time);
	draw_xor_hline(sheet);

	gtk_sheet_set_row_height(sheet, sheet->drag_cell.row,
	    new_row_height(sheet, sheet->drag_cell.row, &y));

	sheet->old_vadjustment = -1.;
	if (sheet->vadjustment)
	{
	    g_signal_emit_by_name(GTK_OBJECT(sheet->vadjustment), "value_changed");
	}
	return (TRUE);
    }


    if (GTK_SHEET_IN_DRAG(sheet))
    {
	GtkSheetRange old_range;
	draw_xor_rectangle(sheet, sheet->drag_range);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_DRAG);
	gdk_pointer_ungrab(event->time);

	gtk_sheet_real_unselect_range(sheet, NULL);

	sheet->active_cell.row = sheet->active_cell.row +
	    (sheet->drag_range.row0 - sheet->range.row0);
	sheet->active_cell.col = sheet->active_cell.col +
	    (sheet->drag_range.col0 - sheet->range.col0);
	sheet->selection_cell.row = sheet->selection_cell.row +
	    (sheet->drag_range.row0 - sheet->range.row0);
	sheet->selection_cell.col = sheet->selection_cell.col +
	    (sheet->drag_range.col0 - sheet->range.col0);
	old_range = sheet->range;
	sheet->range = sheet->drag_range;
	sheet->drag_range = old_range;
	g_signal_emit(GTK_OBJECT(sheet), sheet_signals[MOVE_RANGE], 0,
	    &sheet->drag_range, &sheet->range);
	gtk_sheet_select_range(sheet, &sheet->range);
    }

    if (GTK_SHEET_IN_RESIZE(sheet))
    {
	GtkSheetRange old_range;
	draw_xor_rectangle(sheet, sheet->drag_range);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_RESIZE);
	gdk_pointer_ungrab(event->time);

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

	if (sheet->state == GTK_STATE_NORMAL)
	    sheet->state = GTK_SHEET_RANGE_SELECTED;
	g_signal_emit(GTK_OBJECT(sheet), sheet_signals[RESIZE_RANGE], 0,
	    &sheet->drag_range, &sheet->range);
	gtk_sheet_select_range(sheet, &sheet->range);
    }

    if (sheet->state == GTK_SHEET_NORMAL && GTK_SHEET_IN_SELECTION(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gdk_pointer_ungrab(event->time);
	gtk_sheet_activate_cell(sheet, sheet->active_cell.row,
	    sheet->active_cell.col);
    }

    if (GTK_SHEET_IN_SELECTION)
	gdk_pointer_ungrab(event->time);
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
gtk_sheet_motion_handler(GtkWidget *widget, GdkEventMotion *event)
{
    GtkSheet *sheet;
    GdkModifierType mods;
    GdkCursorType new_cursor;
    gint x, y, row, column;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);


    sheet = GTK_SHEET(widget);

    /* selections on the sheet */
    x = event->x;
    y = event->y;

#if GTK_SHEET_DEBUG_MOTION > 0
    g_debug("gtk_sheet_motion_handler: (%d,%d) inSel %s", x, y,
	GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No" );
#endif

    if (event->window == sheet->column_title_window 
	&& gtk_sheet_columns_resizable(sheet))
    {
	gtk_widget_get_pointer(widget, &x, &y);
	if (!GTK_SHEET_IN_SELECTION(sheet) && POSSIBLE_XDRAG(sheet, x, &column))
	{
	    new_cursor = GDK_SB_H_DOUBLE_ARROW;
	    if (new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
	    {
		gdk_cursor_destroy(sheet->cursor_drag);
		sheet->cursor_drag = gdk_cursor_new(GDK_SB_H_DOUBLE_ARROW);
		gdk_window_set_cursor(sheet->column_title_window, sheet->cursor_drag);
	    }
	}
	else
	{
	    new_cursor = GDK_TOP_LEFT_ARROW;
	    if (!GTK_SHEET_IN_XDRAG(sheet) && new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
	    {
		gdk_cursor_destroy(sheet->cursor_drag);
		sheet->cursor_drag = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
		gdk_window_set_cursor(sheet->column_title_window, sheet->cursor_drag);
	    }
	}
    }

    if (event->window == sheet->row_title_window 
	&& gtk_sheet_rows_resizable(sheet))
    {
	gtk_widget_get_pointer(widget, &x, &y);
	if (!GTK_SHEET_IN_SELECTION(sheet) && POSSIBLE_YDRAG(sheet, y, &column))
	{
	    new_cursor = GDK_SB_V_DOUBLE_ARROW;
	    if (new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
	    {
		gdk_cursor_destroy(sheet->cursor_drag);
		sheet->cursor_drag = gdk_cursor_new(GDK_SB_V_DOUBLE_ARROW);
		gdk_window_set_cursor(sheet->row_title_window, sheet->cursor_drag);
	    }
	}
	else
	{
	    new_cursor = GDK_TOP_LEFT_ARROW;
	    if (!GTK_SHEET_IN_YDRAG(sheet) && new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
	    {
		gdk_cursor_destroy(sheet->cursor_drag);
		sheet->cursor_drag = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
		gdk_window_set_cursor(sheet->row_title_window, sheet->cursor_drag);
	    }
	}
    }

    new_cursor = GDK_PLUS;
    if (!POSSIBLE_DRAG(sheet, x, y, &row, &column) && !GTK_SHEET_IN_DRAG(sheet) &&
	!POSSIBLE_RESIZE(sheet, x, y, &row, &column) && !GTK_SHEET_IN_RESIZE(sheet) &&
	event->window == sheet->sheet_window &&
	new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
    {
	gdk_cursor_destroy(sheet->cursor_drag);
	sheet->cursor_drag = gdk_cursor_new(GDK_PLUS);
	gdk_window_set_cursor(sheet->sheet_window, sheet->cursor_drag);
    }

    new_cursor = GDK_TOP_LEFT_ARROW;
    if (!(POSSIBLE_RESIZE(sheet, x, y, &row, &column) || GTK_SHEET_IN_RESIZE(sheet)) &&
	(POSSIBLE_DRAG(sheet, x, y, &row, &column) || GTK_SHEET_IN_DRAG(sheet)) &&
	event->window == sheet->sheet_window &&
	new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
    {
	gdk_cursor_destroy(sheet->cursor_drag);
	sheet->cursor_drag = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
	gdk_window_set_cursor(sheet->sheet_window, sheet->cursor_drag);
    }

    new_cursor = GDK_SIZING;
    if (!GTK_SHEET_IN_DRAG(sheet) &&
	(POSSIBLE_RESIZE(sheet, x, y, &row, &column) || GTK_SHEET_IN_RESIZE(sheet)) &&
	event->window == sheet->sheet_window &&
	new_cursor != gdk_cursor_get_cursor_type(sheet->cursor_drag))
    {
	gdk_cursor_destroy(sheet->cursor_drag);
	sheet->cursor_drag = gdk_cursor_new(GDK_SIZING);
	gdk_window_set_cursor(sheet->sheet_window, sheet->cursor_drag);
    }

    gdk_window_get_pointer(gtk_widget_get_window(widget), &x, &y, &mods);
    if (!(mods & GDK_BUTTON1_MASK))
	return (FALSE);

    if (GTK_SHEET_IN_XDRAG(sheet))
    {
	if (event->is_hint ||
	    event->window != gtk_widget_get_window(widget))
	{
	    gtk_widget_get_pointer(widget, &x, NULL);
	}
	else
	    x = event->x;

	new_column_width(sheet, sheet->drag_cell.col, &x);
	if (x != sheet->x_drag)
	{
	    draw_xor_vline(sheet);
	    sheet->x_drag = x;
	    draw_xor_vline(sheet);
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_YDRAG(sheet))
    {
	if (event->is_hint ||
	    event->window != gtk_widget_get_window(widget))
	{
	    gtk_widget_get_pointer(widget, NULL, &y);
	}
	else
	    y = event->y;

	new_row_height(sheet, sheet->drag_cell.row, &y);
	if (y != sheet->y_drag)
	{
	    draw_xor_hline(sheet);
	    sheet->y_drag = y;
	    draw_xor_hline(sheet);
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_DRAG(sheet))
    {
	GtkSheetRange aux, visr;

	gint current_row = MIN(sheet->maxrow, _gtk_sheet_row_from_ypixel(sheet, y));
	gint current_col = MIN(sheet->maxcol, _gtk_sheet_column_from_xpixel(sheet, x));

	row = current_row - sheet->drag_cell.row;
	column = current_col - sheet->drag_cell.col;

	if (sheet->state == GTK_SHEET_ROW_SELECTED)
	    column = 0;
	if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
	    row = 0;
	sheet->x_drag = x;
	sheet->y_drag = y;
	aux = sheet->range;

	/* beware of invisible columns/rows */
	if (!_gtk_sheet_get_visible_range(sheet, &visr))
	    return (TRUE);

	if (_RECT_IN_RANGE(aux.row0 + row, aux.rowi + row,
		aux.col0 + column, aux.coli + column, &visr))
	{
	    aux = sheet->drag_range;

#if 1
	    /* The following code doesn't behave properly when there
	       are invisible columns in the sheet. For a proper user experience
	       it should
	       1. _gtk_sheet_count_visible(drag_range)
	       2. try to move the outer edge into the given direction
	       3. find and add enough visible columns backwards
	       Beware: above algo will modify width/height of the drag area
	       */
	    if (row > 0)
	    {
		gint nrow0 = _gtk_sheet_first_visible_rowidx(sheet, sheet->range.row0 + row);
		gint nrowi = _gtk_sheet_first_visible_rowidx(sheet, sheet->range.rowi + row);
		if (nrow0 >= 0 && nrowi >= 0)
		{
		    sheet->drag_range.row0 = nrow0;
		    sheet->drag_range.rowi = nrowi;
		}
	    }
	    else
	    {
		gint nrow0 = _gtk_sheet_last_visible_rowidx(sheet, sheet->range.row0 + row);
		gint nrowi = _gtk_sheet_last_visible_rowidx(sheet, sheet->range.rowi + row);
		if (nrow0 >= 0 && nrowi >= 0)
		{
		    sheet->drag_range.row0 = nrow0;
		    sheet->drag_range.rowi = nrowi;
		}
	    }
	    if (column > 0)
	    {
		gint ncol0 = _gtk_sheet_first_visible_colidx(sheet, sheet->range.col0 + column);
		gint ncoli = _gtk_sheet_first_visible_colidx(sheet, sheet->range.coli + column);
		if (ncol0 >= 0 && ncoli >= 0)
		{
		    sheet->drag_range.col0 = ncol0;
		    sheet->drag_range.coli = ncoli;
		}
	    }
	    else
	    {
		gint ncol0 = _gtk_sheet_last_visible_colidx(sheet, sheet->range.col0 + column);
		gint ncoli = _gtk_sheet_last_visible_colidx(sheet, sheet->range.coli + column);
		if (ncol0 >= 0 && ncoli >= 0)
		{
		    sheet->drag_range.col0 = ncol0;
		    sheet->drag_range.coli = ncoli;
		}
	    }
#endif

	    if (aux.row0 != sheet->drag_range.row0 ||
		aux.col0 != sheet->drag_range.col0)
	    {
		draw_xor_rectangle(sheet, aux);
		draw_xor_rectangle(sheet, sheet->drag_range);
	    }
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_RESIZE(sheet))
    {
	GtkSheetRange aux, visr;
	gint current_col, current_row, col_threshold, row_threshold;

	g_assert(0 <= sheet->drag_cell.row && sheet->drag_cell.row <= sheet->maxrow);
	g_assert(0 <= sheet->drag_cell.col && sheet->drag_cell.col <= sheet->maxcol);

	current_row = MIN(sheet->maxrow, _gtk_sheet_row_from_ypixel(sheet, y));
	current_col = MIN(sheet->maxcol, _gtk_sheet_column_from_xpixel(sheet, x));

	row    = current_row - sheet->drag_cell.row;
	column = current_col - sheet->drag_cell.col;

#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_motion: RESIZE row %d col %d cr %d cc %d",
	    row, column, current_row, current_col);
#endif

	/*use half of column width resp. row height as threshold to expand selection*/
	row_threshold = _gtk_sheet_row_top_ypixel(sheet, current_row);
	if (current_row >= 0)
	    row_threshold += (ROWPTR(sheet, current_row)->height) / 2;

	if (current_row > sheet->drag_range.row0 && y < row_threshold)
	    current_row = _gtk_sheet_last_visible_rowidx(sheet, current_row - 1);
	else if (current_row < sheet->drag_range.row0 && y < row_threshold)
	    current_row = _gtk_sheet_first_visible_rowidx(sheet, current_row + 1);

	col_threshold = _gtk_sheet_column_left_xpixel(sheet, current_col);
	if (current_col >= 0)
	    col_threshold += (COLPTR(sheet, current_col)->width) / 2;

	if (current_col > sheet->drag_range.col0 && x < col_threshold)
	    current_col = _gtk_sheet_last_visible_colidx(sheet, current_col - 1);
	else if (current_col < sheet->drag_range.col0 && x > col_threshold)
	    current_col = _gtk_sheet_first_visible_colidx(sheet, current_col + 1);

	sheet->x_drag = x;
	sheet->y_drag = y;
	aux = sheet->range;

	/* beware of invisible columns/rows */
	if (!_gtk_sheet_get_visible_range(sheet, &visr))
	    return (TRUE);

#if GTK_SHEET_DEBUG_SELECTION > 0
	g_debug("gtk_sheet_motion: RESIZE th %d row %d col %d cr %d cc %d",
	    row_threshold, row, column, current_row, current_col);
#endif

	if (_POINT_IN_RANGE(current_row, current_col, &visr))
	{
	    aux = sheet->drag_range;
	    sheet->drag_range = sheet->range;

	    if (sheet->state != GTK_SHEET_COLUMN_SELECTED)
	    {
		if (current_row >= sheet->drag_range.row0)
		{
		    sheet->drag_range.rowi = current_row;
		}
		else if (current_row < sheet->drag_range.row0)
		{
		    sheet->drag_range.rowi = sheet->drag_range.row0;
		    sheet->drag_range.row0 = current_row;
		}
	    }

	    if (sheet->state != GTK_SHEET_ROW_SELECTED)
	    {
		if (current_col >= sheet->drag_range.col0)
		{
		    sheet->drag_range.coli = current_col;
		}
		else if (current_col < sheet->drag_range.col0)
		{
		    sheet->drag_range.coli = sheet->drag_range.col0;
		    sheet->drag_range.col0 = current_col;
		}
	    }

	    if (_RANGE_NEQ_RANGE(&aux, &sheet->drag_range)
	    {
		draw_xor_rectangle(sheet, aux);
		draw_xor_rectangle(sheet, sheet->drag_range);
	    }
	}
	return (TRUE);
    }

    gtk_sheet_get_pixel_info(sheet, NULL, x, y, &row, &column);

    if (sheet->state == GTK_SHEET_NORMAL
	&& row == sheet->active_cell.row && column == sheet->active_cell.col)
    {
	return (TRUE);
    }

    if (GTK_SHEET_IN_SELECTION(sheet) && (mods & GDK_BUTTON1_MASK))
    {
	GtkSheetRange visr;

	if (!_gtk_sheet_get_visible_range(sheet, &visr))
	    return (TRUE);

	if (_POINT_IN_RANGE(
	    row >= 0 ? row : _gtk_sheet_first_visible_rowidx(sheet, 0),
	    column >= 0 ? column : _gtk_sheet_first_visible_colidx(sheet, 0), 
	    &visr))
	{
	    gtk_sheet_extend_selection(sheet, row, column);
	}
    }

    return (TRUE);
}

/* _HUNT_() statement macros find visible row/col into hunting direction */

#define _HUNT_VISIBLE_LEFT(col) \
	while (col > 0 \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) ) \
		  col--; \
	if (col < 0) col = 0; \
	while (col < sheet->maxcol \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) ) \
		  col++; \
	if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) ) \
		col = -1;

#define _HUNT_VISIBLE_RIGHT(col) \
	while (col < sheet->maxcol \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) ) \
		col++; \
    if (col > sheet->maxcol) col = sheet->maxcol; \
	while (col > 0 \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) ) \
		col--; \
	if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) ) \
		col = -1;

#define _HUNT_VISIBLE_UP(row) \
	while (row > 0 \
	  && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row--; \
	if (row < 0) row = 0; \
	while (row < sheet->maxrow \
	  && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row++; \
	if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row = -1;

#define _HUNT_VISIBLE_DOWN(row) \
	while (row < sheet->maxrow \
	  && ((row < 0) || !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) ) \
	       row++; \
	if (row > sheet->maxrow) row = sheet->maxrow; \
	while (row > 0 \
	  && !GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row--; \
	if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) row = -1;


#define _HUNT_FOCUS_LEFT(col) \
	while (col > 0 \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) \
		  || !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col)))) ) \
		  col--; \
	if (col < 0) col = 0; \
	while (col < sheet->maxcol \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) \
		  || !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col)))) ) \
		  col++; \
	if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) \
		|| !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col))) ) \
		col = -1;

#define _HUNT_FOCUS_RIGHT(col) \
	while (col < sheet->maxcol \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) \
		  || !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col)))) ) \
		col++; \
        if (col > sheet->maxcol) col = sheet->maxcol; \
	while (col > 0 \
	  && (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) \
		  || !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col)))) ) \
		col--; \
	if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) \
		|| !gtk_widget_get_can_focus(GTK_WIDGET(COLPTR(sheet, col))) ) \
		col = -1;

#define _HUNT_FOCUS_UP(row) \
	while (row > 0 \
	  && (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) \
	      || !GTK_SHEET_ROW_CAN_FOCUS(ROWPTR(sheet, row)))) \
	      row--; \
	if (row < 0) row = 0; \
	while (row < sheet->maxrow \
	  && (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) \
	      || !GTK_SHEET_ROW_CAN_FOCUS(ROWPTR(sheet, row)))) \
	    row++; \
	if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) \
	    || !GTK_SHEET_ROW_CAN_FOCUS(ROWPTR(sheet, row))) \
	    row = -1;

#define _HUNT_FOCUS_DOWN(row) \
	while (row < sheet->maxrow \
	  && (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) \
	      || !GTK_SHEET_ROW_CAN_FOCUS(ROWPTR(sheet, row)))) \
	       row++; \
	if (row > sheet->maxrow) row = sheet->maxrow; \
	while (row > 0 \
	  && (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) \
	      || !GTK_SHEET_ROW_CAN_FOCUS(ROWPTR(sheet, row)))) \
	    row--; \
	if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) \
	    || !GTK_SHEET_ROW_CAN_FOCUS(ROWPTR(sheet, row))) \
	    row = -1;


static gint
_gtk_sheet_move_query(GtkSheet *sheet, gint row, gint column, 
    gboolean need_focus)
{
    gint row_move = FALSE, column_move = FALSE;
    gint row_align = -1, col_align = -1;  /* undef. */
    guint height, width;
    gint new_row = row;
    gint new_col = column;
    GtkSheetRange visr;

#if GTK_SHEET_DEBUG_MOVE > 0
    g_debug("gtk_sheet_move_query: row %d column %d view row %d-%d col %d-%d",
	row, column,
	MIN_VIEW_ROW(sheet), MAX_VIEW_ROW(sheet),
	MIN_VIEW_COLUMN(sheet), MAX_VIEW_COLUMN(sheet));
#endif

    height = sheet->sheet_window_height;
    width = sheet->sheet_window_width;

    /* beware of invisible columns/rows */
    if (!_gtk_sheet_get_visible_range(sheet, &visr))
	return (0);

#if GTK_SHEET_DEBUG_MOVE > 0
    g_debug("gtk_sheet_move_query: state %d visr row %d-%d col %d-%d",
	sheet->state, visr.row0, visr.rowi, visr.col0, visr.coli);
#endif

    if (row >= MAX_VIEW_ROW(sheet)
	&& row <= visr.rowi
	&& sheet->state != GTK_SHEET_COLUMN_SELECTED)
    {
#if GTK_SHEET_DEBUG_MOVE > 0
	g_debug("gtk_sheet_move_query: row %d > maxvr %d visr.rowi %d", 
	    row, MAX_VIEW_ROW(sheet), visr.rowi);
#endif
	row_align = 1;  /* bottom */
	if (need_focus)
	{
	    _HUNT_FOCUS_DOWN(new_row);
	}
	else
	{
	    _HUNT_VISIBLE_DOWN(new_row);
	}
	if (new_row >= 0) row_move = TRUE;

	if (MAX_VIEW_ROW(sheet) == sheet->maxrow &&
	    _gtk_sheet_row_bottom_ypixel(sheet, sheet->maxrow) < height)
	{
	    row_move = FALSE;
	    row_align = -1;
	}
    }

    if (row <= MIN_VIEW_ROW(sheet)
	&& row >= visr.row0
	&& sheet->state != GTK_SHEET_COLUMN_SELECTED)
    {
#if GTK_SHEET_DEBUG_MOVE > 0
	g_debug("gtk_sheet_move_query: row %d < minvr %d visr.row0 %d",
	    row, MIN_VIEW_ROW(sheet), visr.row0);
#endif
	row_align = 0;  /* top */
	if (need_focus)
	{
	    _HUNT_FOCUS_UP(new_row);
	}
	else
	{
	    _HUNT_VISIBLE_UP(new_row);
	}
	if (new_row >= 0) row_move = TRUE;
    }

    if (column >= MAX_VIEW_COLUMN(sheet)
	&& column <= visr.coli
	&& sheet->state != GTK_SHEET_ROW_SELECTED)
    {
#if GTK_SHEET_DEBUG_MOVE > 0
	g_debug("gtk_sheet_move_query: col %d > maxvc %d visr.coli %d",
	    column, MAX_VIEW_COLUMN(sheet), visr.coli);
#endif
	col_align = 1;  /* right */
	if (need_focus)
	{
	    _HUNT_FOCUS_RIGHT(new_col);
	}
	else
	{
	    _HUNT_VISIBLE_RIGHT(new_col);
	}
	if (new_col >= 0) column_move = TRUE;

	if (MAX_VIEW_COLUMN(sheet) == sheet->maxcol &&
	    _gtk_sheet_column_right_xpixel(sheet, sheet->maxcol) < width)
	{
	    column_move = FALSE;
	    col_align = -1;
	}
    }

    if (column <= MIN_VIEW_COLUMN(sheet)
	&& column >= visr.col0
	&& sheet->state != GTK_SHEET_ROW_SELECTED)
    {
#if GTK_SHEET_DEBUG_MOVE > 0
	g_debug("gtk_sheet_move_query: col %d < minvc %d visr.col0 %d",
	    column, MIN_VIEW_COLUMN(sheet), visr.col0);
#endif
	col_align = 0;  /* left */
	if (need_focus)
	{
	    _HUNT_FOCUS_LEFT(new_col);
	}
	else
	{
	    _HUNT_VISIBLE_LEFT(new_col);
	}
	if (new_col >= 0) column_move = TRUE;
    }

#if GTK_SHEET_DEBUG_MOVE > 0
    g_debug("gtk_sheet_move_query: rowMv %d colMv %d newR %d newC %d",
	row_move, column_move, new_row, new_col);
#endif

    if (row_move || column_move)
    {
	gtk_sheet_moveto(sheet, new_row, new_col, row_align, col_align);
    }

    return (row_move || column_move);
}

static void
gtk_sheet_extend_selection(GtkSheet *sheet, gint row, gint column)
{
    GtkSheetRange range;
    gint state;
    gint r, c;

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_extend_selection: row %d column %d", row, column);
#endif

    if (sheet->selection_mode == GTK_SELECTION_SINGLE)
	return;
    if (row == sheet->selection_cell.row && column == sheet->selection_cell.col)
	return;

    if (sheet->active_cell.row < 0 || sheet->active_cell.row > sheet->maxrow)
	return;
    if (sheet->active_cell.col < 0 || sheet->active_cell.col > sheet->maxcol)
	return;

    _gtk_sheet_move_query(sheet, row, column, FALSE);
    gtk_widget_grab_focus(GTK_WIDGET(sheet));

    if (GTK_SHEET_IN_DRAG(sheet))
	return;

    state = sheet->state;

    switch(sheet->state)
    {
	case GTK_SHEET_ROW_SELECTED:
	    column = sheet->maxcol;
	    break;

	case GTK_SHEET_COLUMN_SELECTED:
	    row = sheet->maxrow;
	    break;

	case GTK_SHEET_NORMAL:
	    r = sheet->active_cell.row;
	    c = sheet->active_cell.col;

	    sheet->range.col0 = c;
	    sheet->range.row0 = r;
	    sheet->range.coli = c;
	    sheet->range.rowi = r;

	    gdk_draw_pixmap(sheet->sheet_window,
		gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[GTK_STATE_NORMAL],
		sheet->pixmap,
		_gtk_sheet_column_left_xpixel(sheet, c) - 1,
		_gtk_sheet_row_top_ypixel(sheet, r) - 1,
		_gtk_sheet_column_left_xpixel(sheet, c) - 1,
		_gtk_sheet_row_top_ypixel(sheet, r) - 1,
		COLPTR(sheet, c)->width + 4,
		sheet->row[r].height + 4);

	    sheet->state = GTK_SHEET_RANGE_SELECTED;
	    gtk_sheet_range_draw_selection(sheet, sheet->range);
	    /* FALLTHROUGH */

	case GTK_SHEET_RANGE_SELECTED:
	    sheet->state = GTK_SHEET_RANGE_SELECTED;
	    /* FALLTHROUGH */
    }

    sheet->selection_cell.row = row;
    sheet->selection_cell.col = column;

    range.col0 = MIN(column, sheet->active_cell.col);
    range.coli = MAX(column, sheet->active_cell.col);

    range.row0 = MIN(row, sheet->active_cell.row);
    range.rowi = MAX(row, sheet->active_cell.row);

    range.coli = MIN(range.coli, sheet->maxcol);
    range.rowi = MIN(range.rowi, sheet->maxrow);

    if (range.row0 != sheet->range.row0 || range.rowi != sheet->range.rowi ||
	range.col0 != sheet->range.col0 || range.coli != sheet->range.coli ||
	state == GTK_SHEET_NORMAL)
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
    gboolean stop_emission = FALSE;
    GtkSheet *sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug("gtk_sheet_entry_key_press_handler: called, key %s",
	gtk_accelerator_name(key->keyval, key->state));
#endif

    if (!_gtk_sheet_binding_filter(sheet, key))
    {
#if GTK_SHEET_DEBUG_KEYPRESS > 0
	g_debug("gtk_sheet_entry_key_press_handler: filtered binding");
#endif
	return (FALSE);
    }

#if 1
    /* process enter-event
       - detect wether Return or Enter was pressed 
       - detect wether the application wants it to be handled by the sheet 
       - if unwanted: execute appropriate application handler 
       - use a new signal for this ? 
       */
    if ((key->keyval == GDK_KEY_Return) || (key->keyval == GDK_KEY_KP_Enter))
    {
#if GTK_SHEET_DEBUG_ENTER_PRESSED > 0
	g_debug("gtk_sheet_entry_key_press_handler: enter-pressed: invoke");
#endif
	_gtkextra_signal_emit(GTK_OBJECT(sheet),
	    sheet_signals[ENTER_PRESSED],
	    key,
	    &stop_emission);
#if GTK_SHEET_DEBUG_ENTER_PRESSED > 0
	g_debug("gtk_sheet_entry_key_press_handler: enter-pressed: returns %d", stop_emission);
#endif
    }
#endif

    if (!stop_emission)
    {
	/* intercept item_entry processing if there exists a key binding on the sheet */

	if (gtk_bindings_activate_event(GTK_OBJECT(sheet), key))
	{
	    stop_emission = TRUE;
	}
	else
	{
	    g_signal_emit_by_name(GTK_OBJECT(widget),
		"key_press_event", key,
		&stop_emission);
	}
    }

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug("gtk_sheet_entry_key_press_handler: done, key %s stop %d",
	gtk_accelerator_name(key->keyval, key->state), stop_emission);
#endif

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
    gint state;
    gboolean extend_selection = FALSE;
    gboolean in_selection = FALSE;

    sheet = GTK_SHEET(widget);

    extend_selection = (key->state & GDK_SHIFT_MASK)
	|| key->keyval == GDK_KEY_Shift_L
	|| key->keyval == GDK_KEY_Shift_R;

    state = sheet->state;
    in_selection = GTK_SHEET_IN_SELECTION(sheet);
    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug("gtk_sheet_key_press_handler: key %s",
	gtk_accelerator_name(key->keyval, key->state));
#endif

    /* if there is a key_binding, use implementation from _gtk_sheet_move_cursor() */

    if (_gtk_sheet_binding_filter(sheet, key)
	&& gtk_bindings_activate_event(GTK_OBJECT(sheet), key))
    {
#if GTK_SHEET_DEBUG_KEYPRESS > 0
	g_debug("gtk_sheet_key_press_handler: done %s (binding)",
	    gtk_accelerator_name(key->keyval, key->state));
#endif
	return (TRUE);
    }

    return (FALSE);
}


/**
 * _gtk_sheet_move_cursor: 
 * @sheet:  the sheet 
 * @step:   type of movement
 * @count:  number of steps to move
 * @extend_selection: TRUE if the move should extend the 
 * selection default handler for move-cursor signal 
 */
static void _gtk_sheet_move_cursor(GtkSheet *sheet,
    GtkMovementStep step,
    gint count,
    gboolean extend_selection)
{
    gint row, col;
    gboolean veto = TRUE;

#if GTK_SHEET_DEBUG_SIGNALS > 0
    g_debug("SIGNAL _gtk_sheet_move_cursor %p step %d count %d extend %d",
	sheet, step, count, extend_selection);
#endif

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    switch(step)
    {
	case GTK_MOVEMENT_PAGES:
	    count = count *
		(MAX_VIEW_ROW(sheet) - MIN_VIEW_ROW(sheet) - GTK_SHEET_PAGE_OVERLAP);
	    /* FALLTHROUGH */

	case GTK_MOVEMENT_DISPLAY_LINES:
	    if (count < 0) /* Move Up */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.row > 0)
		    {
			row = sheet->selection_cell.row + count;
			_HUNT_VISIBLE_UP(row);
			gtk_sheet_extend_selection(sheet, row, sheet->selection_cell.col);
		    }
		    return;
		}

		if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
		    row = MIN_VIEW_ROW(sheet);
		if (sheet->state == GTK_SHEET_ROW_SELECTED)
		    col = MIN_VIEW_COLUMN(sheet);

		if (row > 0)
		{
		    row = row + count;
		    _HUNT_FOCUS_UP(row);
		}
	    }
	    else if (count > 0) /* Move Down */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.row < sheet->maxrow)
		    {
			row = sheet->selection_cell.row + count;
			_HUNT_VISIBLE_DOWN(row);
			gtk_sheet_extend_selection(sheet, row, sheet->selection_cell.col);
		    }
		    return;
		}

		if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
		    row = MIN_VIEW_ROW(sheet) - 1;
		if (sheet->state == GTK_SHEET_ROW_SELECTED)
		    col = MIN_VIEW_COLUMN(sheet);

		if (row < sheet->maxrow)
		{
		    row = row + count;
		    _HUNT_FOCUS_DOWN(row);
		}
	    }
	    gtk_sheet_click_cell(sheet, row, col, &veto);
	    break;

	case GTK_MOVEMENT_HORIZONTAL_PAGES:
	    count = count *
		(MAX_VIEW_COLUMN(sheet) - MIN_VIEW_COLUMN(sheet) - GTK_SHEET_PAGE_OVERLAP);
	    /* FALLTHROUGH */

	case GTK_MOVEMENT_VISUAL_POSITIONS:
	    if (count < 0)  /* Move Left */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.col > 0)
		    {
			col = sheet->selection_cell.col + count;
			_HUNT_VISIBLE_LEFT(col);
			gtk_sheet_extend_selection(sheet, sheet->selection_cell.row, col);
		    }
		    return;
		}

		if (sheet->state == GTK_SHEET_ROW_SELECTED)
		    col = MIN_VIEW_COLUMN(sheet) - 1;
		if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
		    row = MIN_VIEW_ROW(sheet);

		if (col > 0)
		{
		    col = col + count;
		    _HUNT_FOCUS_LEFT(col);
		}
	    }
	    else if (count > 0) /* Move Right */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.col < sheet->maxcol)
		    {
			col = sheet->selection_cell.col + count;
			_HUNT_VISIBLE_RIGHT(col);
			gtk_sheet_extend_selection(sheet, sheet->selection_cell.row, col);
		    }
		    return;
		}

		if (sheet->state == GTK_SHEET_ROW_SELECTED)
		    col = MIN_VIEW_COLUMN(sheet) - 1;
		if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
		    row = MIN_VIEW_ROW(sheet);

		if (col < sheet->maxcol)
		{
		    col = col + count;
		    _HUNT_FOCUS_RIGHT(col);
		}
	    }
	    gtk_sheet_click_cell(sheet, row, col, &veto);
	    break;

	case GTK_MOVEMENT_BUFFER_ENDS:
	    if (count < 0)  /* Topmost Row */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.row > 0)
		    {
			row = 0;
			_HUNT_VISIBLE_UP(row);
			gtk_sheet_extend_selection(sheet, row, sheet->selection_cell.col);
		    }
		    return;
		}
		row = 0;
		_HUNT_FOCUS_UP(row);
	    }
	    else if (count > 0)  /* Last Row */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.row < sheet->maxrow)
		    {
			row = sheet->maxrow;
			_HUNT_VISIBLE_DOWN(row);
			gtk_sheet_extend_selection(sheet, row, sheet->selection_cell.col);
		    }
		    return;
		}
		row = sheet->maxrow;
		_HUNT_FOCUS_DOWN(row);
	    }
	    gtk_sheet_click_cell(sheet, row, col, &veto);
	    break;

	case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
	    if (count < 0)  /* First Column */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.col > 0)
		    {
			col = 0;
			_HUNT_VISIBLE_LEFT(col);
			gtk_sheet_extend_selection(sheet, sheet->selection_cell.row, col);
		    }
		    return;
		}
		col = 0;
		_HUNT_FOCUS_LEFT(col);
	    }
	    else if (count > 0)  /* Last Column */
	    {
		if (extend_selection)
		{
		    if (sheet->state == GTK_STATE_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cell.col < sheet->maxcol)
		    {
			col = sheet->maxcol;
			_HUNT_VISIBLE_RIGHT(col);
			gtk_sheet_extend_selection(sheet, sheet->selection_cell.row, col);
		    }
		    return;
		}
		col = sheet->maxcol;
		_HUNT_FOCUS_RIGHT(col);
	    }
	    gtk_sheet_click_cell(sheet, row, col, &veto);
	    break;

	case GTK_MOVEMENT_LOGICAL_POSITIONS:
	    if (sheet->state == GTK_SHEET_ROW_SELECTED)
		col = MIN_VIEW_COLUMN(sheet) - 1;
	    if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
		row = MIN_VIEW_ROW(sheet);

	    if ((count == GTK_DIR_LEFT)  /* Tab horizontal backward */
		|| (count == GTK_DIR_TAB_BACKWARD))  /* Tab horizontal backward */
	    {
		gint old_col = col;

		if (col > 0)  /* move left within line */
		{
		    col = col - 1;
		    _HUNT_FOCUS_LEFT(col);
		}
		if (col == old_col && row > 0) /* wrap at eol */
		{
		    col = sheet->maxcol;
		    _HUNT_FOCUS_LEFT(col);

		    row = row - 1;
		    _HUNT_FOCUS_UP(row);
		}
	    }
	    else if ((count == GTK_DIR_RIGHT)  /* Tab horizontal forward */
		|| (count == GTK_DIR_TAB_FORWARD))
	    {
		gint old_col = col;

		if (col < sheet->maxcol)  /* move right within line */
		{
		    col = col + 1;
		    _HUNT_FOCUS_RIGHT(col);
		}
		if (col == old_col && row < sheet->maxrow) /* wrap at eol */
		{
		    col = 0;
		    _HUNT_FOCUS_RIGHT(col);

		    row = row + 1;
		    _HUNT_FOCUS_DOWN(row);
		}
	    }
	    else if (count == GTK_DIR_UP)  /* Tab vertical backward */
	    {
		gint old_row = row;

		if (row > 0)  /* move up within column */
		{
		    row = row - 1;
		    _HUNT_FOCUS_UP(row);
		}
		if (row == old_row && col > 0) /* wrap at eol */
		{
		    row = sheet->maxrow;
		    _HUNT_FOCUS_UP(row);

		    col = col - 1;
		    _HUNT_FOCUS_LEFT(col);
		}
	    }
	    else if (count == GTK_DIR_DOWN)  /* Tab vertical forward */
	    {
		gint old_row = row;

		if (row < sheet->maxrow)  /* move down within column */
		{
		    row = row + 1;
		    _HUNT_FOCUS_DOWN(row);
		}
		if (row == old_row && col < sheet->maxcol) /* wrap at eol */
		{
		    row = 0;
		    _HUNT_FOCUS_DOWN(row);

		    col = col + 1;
		    _HUNT_FOCUS_RIGHT(col);
		}
	    }
	    gtk_sheet_click_cell(sheet, row, col, &veto);
	    break;

	default:
	    break;
    }

    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);

#if GTK_SHEET_DEBUG_SIGNALS > 0
    g_debug("SIGNAL _gtk_sheet_move_cursor %p done", sheet);
#endif
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
gtk_sheet_size_request_handler(GtkWidget *widget, GtkRequisition *requisition)
{
    GtkSheet *sheet;
    GList *children;
    GtkSheetChild *child;
    GtkRequisition child_requisition;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));
    g_return_if_fail(requisition != NULL);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("gtk_sheet_size_request_handler: called");
#endif

    sheet = GTK_SHEET(widget);

    requisition->width = 3 * GTK_SHEET_COLUMN_DEFAULT_WIDTH;
    requisition->height = 3 * _gtk_sheet_row_default_height(widget);

    /* compute the size of the column title area */
    if (sheet->column_titles_visible)
	requisition->height += sheet->column_title_area.height;

    /* compute the size of the row title area */
    if (sheet->row_titles_visible)
	requisition->width += sheet->row_title_area.width;

    _gtk_sheet_recalc_view_range(sheet);

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
gtk_sheet_size_allocate_handler(GtkWidget *widget, GtkAllocation *allocation)
{
    GtkSheet *sheet;
    GtkAllocation sheet_allocation;
    gint border_width;
    gboolean modified;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));
    g_return_if_fail(allocation != NULL);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("gtk_sheet_size_allocate_handler: called (%d, %d, %d, %d)",
	allocation->x, allocation->y, allocation->width, allocation->height);
#endif

    sheet = GTK_SHEET(widget);

    gtk_widget_set_allocation(widget, allocation);
    border_width = gtk_container_get_border_width(GTK_CONTAINER(widget));

    if (gtk_widget_get_realized(widget))
    {
	gdk_window_move_resize(gtk_widget_get_window(widget),
	    allocation->x + border_width,
	    allocation->y + border_width,
	    allocation->width - 2 * border_width,
	    allocation->height - 2 * border_width);
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

    modified =
	(sheet->sheet_window_width != sheet_allocation.width) ||
	(sheet->sheet_window_height != sheet_allocation.height);

    sheet->sheet_window_width = sheet_allocation.width;
    sheet->sheet_window_height = sheet_allocation.height;

    if (gtk_widget_get_realized(widget))
    {
	gdk_window_move_resize(sheet->sheet_window,
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
	gdk_window_move_resize(sheet->column_title_window,
	    sheet->column_title_area.x,
	    sheet->column_title_area.y,
	    sheet->column_title_area.width,
	    sheet->column_title_area.height);
    }

    /* column button allocation */
    _gtk_sheet_column_buttons_size_allocate(sheet);

    /* position the window which holds the row title buttons */
    sheet->row_title_area.x = 0;
    sheet->row_title_area.y = 0;
    if (sheet->column_titles_visible)
	sheet->row_title_area.y = sheet->column_title_area.height;
    sheet->row_title_area.height = sheet_allocation.height - sheet->row_title_area.y;

    if (gtk_widget_get_realized(widget) && sheet->row_titles_visible)
    {
	gdk_window_move_resize(sheet->row_title_window,
	    sheet->row_title_area.x,
	    sheet->row_title_area.y,
	    sheet->row_title_area.width,
	    sheet->row_title_area.height);
    }

    /* row button allocation */
    size_allocate_row_title_buttons(sheet);

    if (gtk_sheet_autoresize(sheet) &&
	(modified || (GTK_SHEET_FLAGS(sheet) & GTK_SHEET_IN_AUTORESIZE_PENDING)))
    {
	/* autoresize here, because window was changed -> max col_width */
	gtk_sheet_autoresize_all(sheet);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_AUTORESIZE_PENDING);
    }

    _gtk_sheet_recalc_view_range(sheet);

    /* re-scale backing pixmap */
    gtk_sheet_make_backing_pixmap(sheet, 0, 0);
    gtk_sheet_position_children(sheet);

    /* set the scrollbars adjustments */
    _gtk_sheet_scrollbar_adjust(sheet);
}

static gboolean gtk_sheet_focus(GtkWidget *widget,
    GtkDirectionType  direction)
{
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);
    GtkSheet *sheet = GTK_SHEET(widget);

    if (!gtk_widget_is_sensitive(GTK_WIDGET(sheet))) {
	g_debug("gtk_sheet_focus: X"); 
	return(FALSE);
    }

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug("gtk_sheet_focus: %p %d", widget, direction); 
#endif

    if (!gtk_widget_has_focus (widget))
    {
	gtk_widget_grab_focus (widget);
    }

    gint row = sheet->active_cell.row;
    gint col = sheet->active_cell.col;

    if (row < 0 || col < 0)  /* not in sheet */
    {
	_gtk_sheet_move_cursor(sheet, GTK_MOVEMENT_VISUAL_POSITIONS, 1, FALSE);
	return(TRUE);
    }

    gboolean veto;

    gtk_sheet_click_cell(sheet, row, col, &veto);
    if (!veto) return(FALSE);
	
    return(TRUE);
}


static void
size_allocate_row_title_buttons(GtkSheet *sheet)
{
    gint i, y, height;
    GdkRectangle *rta = &sheet->row_title_area;

    if (!sheet->row_titles_visible)
	return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("size_allocate_row_title_buttons: called");
#endif

    height = sheet->sheet_window_height;
    y = 0;

    if (sheet->column_titles_visible)
    {
	height -= sheet->column_title_area.height;
	y = sheet->column_title_area.height;
    }

    /* if neccessary, resize the row title window */
    if (rta->height != height || rta->y != y)
    {
	rta->y = y;
	rta->height = height;
	gdk_window_move_resize(sheet->row_title_window,
	    rta->x,
	    rta->y,
	    rta->width,
	    rta->height);
    }

    /* if the right edge of the sheet is visible, clear it */
    if (MAX_VIEW_ROW(sheet) >= sheet->maxrow)
    {
	/* this one causes flicker */

	gdk_window_clear_area(sheet->row_title_window,
	    0, 0,
	    rta->width,
	    rta->height);
    }

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;

    for (i = MIN_VIEW_ROW(sheet); i <= MAX_VIEW_ROW(sheet) && i <= sheet->maxrow; i++) _gtk_sheet_draw_button(sheet, i, -1);
}

/**
 * gtk_sheet_recalc_top_ypixels:
 * @sheet:  the #GtkSheet 
 *  
 * recalculate topmost pixel of all rows
 */
void
_gtk_sheet_recalc_top_ypixels(GtkSheet *sheet)
{
    gint i, cy;

    if (sheet->column_titles_visible)
	cy = sheet->column_title_area.height;
    else
	cy = 0;

    for (i = 0; i <= sheet->maxrow; i++)
    {
	sheet->row[i].top_ypixel = cy;
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    cy += sheet->row[i].height;
    }
}

/**
 * _gtk_sheet_recalc_left_xpixels:
 * @sheet:  the #GtkSheet 
 *  
 * recalculate left pixel index of all columns
 */
void
_gtk_sheet_recalc_left_xpixels(GtkSheet *sheet)
{
    gint i, cx;

    if (sheet->row_titles_visible)
	cx = sheet->row_title_area.width;
    else
	cx = 0;

    for (i = 0; i <= sheet->maxcol; i++)
    {
	COLPTR(sheet, i)->left_xpixel = cx;
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i)))
	    cx += COLPTR(sheet, i)->width;
    }
}

/**
 * _gtk_sheet_reset_text_column:
 * @sheet:  the #GtkSheet
 * @start_column: left most column to start from 
 *  
 * reset left/right text column index to initial state
 */
void
_gtk_sheet_reset_text_column(GtkSheet *sheet, gint start_column)
{
#if GTK_SHEET_OPTIMIZE_COLUMN_DRAW>0
    int i;

    g_assert(start_column >= -1);

    for (i = start_column + 1; i <= sheet->maxcol; i++)  /* for the fresh columns */
    {
	GtkSheetColumn *colptr = COLPTR(sheet, i);

	colptr->left_text_column = i;
	colptr->right_text_column = i;
    }
#endif
}

static void
_get_entry_window_size(GtkEntry *entry,
    gint     *x,
    gint     *y,
    gint     *width,
    gint     *height)
{
    GtkRequisition requisition;
    GtkAllocation allocation;
    GtkWidget *widget = GTK_WIDGET(entry);

    /* GtkEntry->is_cell_renderer is a GSEAL()ed structure member.
       It will only be set to TRUE when calling the GtkEntry's GtkCellEditable
       interface function gtk_entry_cell_editable_init().
     
       This should never be the case in GtkSheet or GtkItemEntry.
     
       Anyhow, if the above statement wasn't true, solutions could be:
       - ask the Gtk maintainers to add the function
         GtkEntry::get_widget_window_size() to the public GtkEntry interface
       - last resort work-around:
         use the sealed member anyhow GtkEntry->GSEAL(is_cell_renderer)
       */
#if 0
#    define ENTRY_IS_CELL_RENDERER  entry->is_cell_renderer
#else
#    define ENTRY_IS_CELL_RENDERER  FALSE
#endif

    gtk_widget_get_child_requisition(widget, &requisition);
    gtk_widget_get_allocation(widget, &allocation);

    if (x)
	*x = allocation.x;

    if (y)
    {
	if (ENTRY_IS_CELL_RENDERER)
	    *y = allocation.y;
	else
	    *y = allocation.y + (allocation.height - requisition.height) / 2;
    }

    if (width)
	*width = allocation.width;

    if (height)
    {
	if (ENTRY_IS_CELL_RENDERER)
	    *height = allocation.height;
	else
	    *height = requisition.height;
    }
}



/**
 * _gtk_sheet_entry_size_allocate:
 * @sheet:  the #GtkSheet
 *  
 * size allocation handler for the sheet entry
 */
void
_gtk_sheet_entry_size_allocate(GtkSheet *sheet)
{
    GtkAllocation shentry_allocation;
    gint row, col;
    gint size, entry_max_size, column_width, row_height;
    guint text_width, text_height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;
    if (!gtk_widget_get_mapped(GTK_WIDGET(sheet)))
	return;
    if (sheet->maxrow < 0 || sheet->maxcol < 0)
	return;
    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_entry_size_allocate: called");
#endif

    GtkWidget *entry_widget = gtk_sheet_get_entry(sheet);

#if 0
    /* the entry setup must be done before the text assigment,
       otherwise max_length may cause text assignment to fail
       */
    _gtk_sheet_entry_setup(sheet,
	sheet->active_cell.row, sheet->active_cell.col,
	entry_widget);
#endif

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet,
	sheet->active_cell.row, sheet->active_cell.col,
	&attributes);

    if (gtk_widget_get_realized(sheet->sheet_entry))
    {
	gtk_widget_size_request(sheet->sheet_entry, NULL);
    }

    if (GTK_IS_ITEM_ENTRY(entry_widget))
    {
	entry_max_size = GTK_ITEM_ENTRY(entry_widget)->text_max_size;
    }
    else
	entry_max_size = 0;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

    text_width = 0;
    text_height = 0;
    {
	gchar *text = gtk_sheet_get_entry_text(sheet);

	if (text && text[0])
	{
	    _get_string_extent(sheet, 
		 (0 <= col && col <= sheet->maxcol) ? COLPTR(sheet, col) : NULL,
		attributes.font_desc, text, &text_width, &text_height);
	}

	g_free(text);
    }

    if (0 <= col && col <= sheet->maxcol)
	column_width = COLPTR(sheet, col)->width;
    else
	column_width = GTK_SHEET_COLUMN_DEFAULT_WIDTH;

    if (0 <= row && row <= sheet->maxrow)
	row_height = sheet->row[row].height;
    else
	row_height = GTK_SHEET_ROW_DEFAULT_HEIGHT;

    size = MIN(text_width, entry_max_size);
    size = MAX(size, column_width - 2 * CELLOFFSET);

    shentry_allocation.x = _gtk_sheet_column_left_xpixel(sheet, col);
    shentry_allocation.y = _gtk_sheet_row_top_ypixel(sheet, row);
    shentry_allocation.width = column_width;
    shentry_allocation.height = row_height;

    if (GTK_IS_ITEM_ENTRY(sheet->sheet_entry))
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("_gtk_sheet_entry_size_allocate: is_item_entry");
#endif

	shentry_allocation.height -= 2 * CELLOFFSET;
	shentry_allocation.y += CELLOFFSET;

	if (gtk_sheet_clip_text(sheet))
	    shentry_allocation.width = column_width - 2 * CELLOFFSET;
	else  /* text extends multiple cells */
	    shentry_allocation.width = size;

	switch(GTK_ITEM_ENTRY(entry_widget)->justification)
	{
	    case GTK_JUSTIFY_CENTER:
		shentry_allocation.x += (column_width) / 2 - size / 2;
		break;

	    case GTK_JUSTIFY_RIGHT:
		shentry_allocation.x += column_width - size;
		break;

	    case GTK_JUSTIFY_LEFT:
	    case GTK_JUSTIFY_FILL:
		shentry_allocation.x += CELLOFFSET;
		break;
	}

	/* vertical justification */
	{
	    gint x, y, width, height;

	    _get_entry_window_size(GTK_ENTRY(entry_widget), &x, &y, &width, &height);

#if GTK_SHEET_DEBUG_SIZE>0
	    g_debug("_gtk_sheet_entry_size_allocate: get_widget_window_size (%d, %d, %d, %d)",
		x, y, width, height);
#endif

	    switch(sheet->vjust)
	    {
		case GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT:
		case GTK_SHEET_VERTICAL_JUSTIFICATION_TOP:
		    shentry_allocation.height = height;
		    break;

		case GTK_SHEET_VERTICAL_JUSTIFICATION_MIDDLE:
		    shentry_allocation.height = shentry_allocation.height / 2;
		    break;

		case GTK_SHEET_VERTICAL_JUSTIFICATION_BOTTOM:
		    break;
	    }
	}
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(sheet->sheet_entry)
	     || GTK_IS_TEXT_VIEW(sheet->sheet_entry) )
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("_gtk_sheet_entry_size_allocate: is_text_view");
#endif

	shentry_allocation.height -= 2 * CELLOFFSET;
	shentry_allocation.y += CELLOFFSET;
	shentry_allocation.x += CELLOFFSET;

	if (gtk_sheet_clip_text(sheet))
	    shentry_allocation.width = column_width - 2 * CELLOFFSET;
	else  /* text extends multiple cells */
	    shentry_allocation.width = size;
    }
    else
    {
	shentry_allocation.x += 2;
	shentry_allocation.y += 2;
	shentry_allocation.width -= MIN(shentry_allocation.width, 3);
	shentry_allocation.height -= MIN(shentry_allocation.height, 3);
    }

#if 0
    /* the following works with most widgets, but looks funny */
    gtk_widget_set_size_request(sheet->sheet_entry,
				shentry_allocation.width, shentry_allocation.height);
#endif

#if GTK_SHEET_DEBUG_SIZE >0
    g_debug("_gtk_sheet_entry_size_allocate: allocate (%d,%d,%d,%d)",
	shentry_allocation.x, shentry_allocation.y, shentry_allocation.width, shentry_allocation.height);
#endif

    gtk_widget_size_allocate(sheet->sheet_entry, &shentry_allocation);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_entry_size_allocate: returned (%d,%d,%d,%d)",
	shentry_allocation.x, shentry_allocation.y, shentry_allocation.width, shentry_allocation.height);
#endif
}

#if GTK_SHEET_DEBUG_FINALIZE > 0
/* obj_ref debug code */
static void weak_notify(gpointer data, GObject *o)
{
    gchar *msg = data;  /* assume a string was passed as data */

    g_debug("weak_notify: %p finalized (%s)", o, msg ? msg : "");
}
#endif

static void
gtk_sheet_entry_set_max_size(GtkSheet *sheet)
{
    gint i;
    gint size = 0;
    gint sizel = 0, sizer = 0;
    gint row, col;
    GtkJustification justification;

    row = sheet->active_cell.row;
    col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("gtk_sheet_entry_set_max_size: called");
#endif

    if (!GTK_IS_ITEM_ENTRY(sheet->sheet_entry) || gtk_sheet_clip_text(sheet))
	return;

    justification = GTK_ITEM_ENTRY(sheet->sheet_entry)->justification;

    switch(justification)
    {
	case GTK_JUSTIFY_FILL:
	case GTK_JUSTIFY_LEFT:
	    for (i = col + 1; i <= MAX_VIEW_COLUMN(sheet) && i <= sheet->maxcol; i++)
	    {
		if (gtk_sheet_cell_get_text(sheet, row, i))
		    break;

		size += COLPTR(sheet, i)->width;
	    }
	    size = MIN(size, sheet->sheet_window_width - _gtk_sheet_column_left_xpixel(sheet, col));
	    break;

	case GTK_JUSTIFY_RIGHT:
	    for (i = col - 1; i >= MIN_VIEW_COLUMN(sheet); i--)
	    {
		if (gtk_sheet_cell_get_text(sheet, row, i))
		    break;

		if (i < 0 || i > sheet->maxcol)
		    continue;
		size += COLPTR(sheet, i)->width;
	    }
	    break;

	case GTK_JUSTIFY_CENTER:
	    for (i = col + 1; i <= MAX_VIEW_COLUMN(sheet) && i <= sheet->maxcol; i++)
	    {
		/* if (gtk_sheet_cell_get_text(sheet, row, i)) break; */

		sizer += COLPTR(sheet, i)->width;
	    }
	    for (i = col - 1; i >= MIN_VIEW_COLUMN(sheet); i--)
	    {
		if (gtk_sheet_cell_get_text(sheet, row, i))
		    break;

		if (i < 0 || i > sheet->maxcol)
		    continue;
		sizel += COLPTR(sheet, i)->width;
	    }
	    size = 2 * MIN(sizel, sizer);
	    break;
    }

    if (size != 0)
	size += COLPTR(sheet, col)->width;
    GTK_ITEM_ENTRY(sheet->sheet_entry)->text_max_size = size;
}

static gboolean sheet_entry_focus_in_handler(GtkWidget *widget,
    GdkEventFocus *event, gpointer user_data)
{
    gboolean retval = FALSE;
    g_signal_emit(GTK_OBJECT(widget),
	sheet_signals[ENTRY_FOCUS_IN], 0, event, &retval);
    return (retval);
}

static gboolean sheet_entry_focus_out_handler(GtkWidget *widget,
    GdkEventFocus *event, gpointer user_data)
{
    gboolean retval = FALSE;
    g_signal_emit(GTK_OBJECT(widget),
	sheet_signals[ENTRY_FOCUS_OUT], 0, event, &retval);
    return (retval);
}

static void sheet_entry_populate_popup_handler(GtkWidget *widget,
    GtkMenu *menu, gpointer user_data)
{
#if GTK_SHEET_DEBUG_SIGNALS > 0
    g_debug("sheet_entry_populate_popup_handler: menu %p", menu);
#endif

    g_signal_emit(GTK_OBJECT(widget),
	sheet_signals[ENTRY_POPULATE_POPUP], 0, menu);
}

static void
create_sheet_entry(GtkSheet *sheet, GType new_entry_type)
{
    GtkWidget *widget;
    GtkWidget * entry, *new_entry;
    GtkStyle *style;

    widget = GTK_WIDGET(sheet);

#if GTK_SHEET_DEBUG_ENTRY > 0
    g_debug("create_sheet_entry: called");
#endif

    style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(sheet)));

    if (sheet->sheet_entry)
    {
	/* avoids warnings */
	g_object_ref(sheet->sheet_entry);
	gtk_widget_unparent(sheet->sheet_entry);
#if GTK_SHEET_DEBUG_ENTRY > 0
	g_debug("create_sheet_entry: destroying old entry %p", sheet->sheet_entry);
#endif
	gtk_widget_destroy(sheet->sheet_entry);
	sheet->sheet_entry = NULL;
    }

    if (new_entry_type == G_TYPE_NONE) new_entry_type = G_TYPE_ITEM_ENTRY;

#if GTK_SHEET_DEBUG_ENTRY > 0
	g_debug("create_sheet_entry: new_entry type %s", 
	    g_type_name(new_entry_type));
#endif

    new_entry = gtk_widget_new(new_entry_type, NULL);

#if GTK_SHEET_DEBUG_ENTRY > 0
	g_debug("create_sheet_entry: got new_entry %p", new_entry);
#endif

    /* connect focus signal propagation handlers */
    g_signal_connect_swapped(new_entry, "focus-in-event",
	G_CALLBACK(sheet_entry_focus_in_handler), sheet);
    g_signal_connect_swapped(new_entry, "focus-out-event",
	G_CALLBACK(sheet_entry_focus_out_handler), sheet);

    if (GTK_IS_ENTRY(new_entry) 
	|| GTK_IS_DATA_TEXT_VIEW(new_entry) 
	|| GTK_IS_TEXT_VIEW(new_entry))
    {
	g_signal_connect_swapped(new_entry, "populate-popup",
	    G_CALLBACK(sheet_entry_populate_popup_handler), sheet);
    }

    sheet->installed_entry_type = new_entry_type;
    sheet->sheet_entry = new_entry;
    entry = gtk_sheet_get_entry(sheet);

    if (!entry)  /* this was an unsupported entry type */
    {
	g_warning("Unsupported entry type - widget must contain an GtkEditable or GtkTextView");
	gtk_widget_destroy(new_entry);

	new_entry = gtk_item_entry_new();
	sheet->sheet_entry = new_entry;
	sheet->installed_entry_type = G_TYPE_ITEM_ENTRY;
    }
    g_object_ref_sink(sheet->sheet_entry);

#if GTK_SHEET_DEBUG_FINALIZE > 0
    g_object_weak_ref(G_OBJECT(sheet->sheet_entry), weak_notify, "Sheet-Entry");
#endif

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
	gtk_widget_size_request(sheet->sheet_entry, NULL);

#if GTK_SHEET_DEBUG_ENTRY > 0
	g_debug("create_sheet_entry: sheet was realized");
#endif
	gtk_widget_set_parent_window(sheet->sheet_entry, sheet->sheet_window);
	gtk_widget_set_parent(sheet->sheet_entry, GTK_WIDGET(sheet));
	gtk_widget_realize(sheet->sheet_entry);
    }

    g_signal_connect_swapped(GTK_OBJECT(entry), "key_press_event",
	(void *)gtk_sheet_entry_key_press_handler,
	GTK_OBJECT(sheet));

    gtk_widget_show(sheet->sheet_entry);
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
    g_return_val_if_fail(sheet, GTK_SHEET_ENTRY_TYPE_DEFAULT);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), GTK_SHEET_ENTRY_TYPE_DEFAULT);

    return (sheet->entry_type);
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
 * Returns: (transfer none) a #GtkWidget or NULL
 */
GtkWidget *
gtk_sheet_get_entry(GtkSheet *sheet)
{
    GtkWidget *parent;
    GtkWidget *entry = NULL;
    GtkTableChild *table_child;
    GtkBoxChild *box_child;
    GList *children = NULL;

    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (!sheet->sheet_entry)   /* PR#102114 */
	return(NULL);
    
    if (GTK_IS_EDITABLE(sheet->sheet_entry))
	return (sheet->sheet_entry);
    if (GTK_IS_DATA_TEXT_VIEW(sheet->sheet_entry))
	return (sheet->sheet_entry);
    if (GTK_IS_TEXT_VIEW(sheet->sheet_entry))
	return (sheet->sheet_entry);

    parent = GTK_WIDGET(sheet->sheet_entry);

    if (GTK_IS_TABLE(parent))
	children = GTK_TABLE(parent)->children;
    if (GTK_IS_BOX(parent))
	children = GTK_BOX(parent)->children;

    if (!children)
	return (NULL);

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

	if (GTK_IS_EDITABLE(entry))
	    return (entry);
	if (GTK_IS_DATA_TEXT_VIEW(entry))
	    return (entry);
	if (GTK_IS_TEXT_VIEW(entry))
	    return (entry);

	children = children->next;
    }

    return (NULL);
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
 * Returns: (transfer none) a #GtkWidget or NULL
 */
GtkWidget *
gtk_sheet_get_entry_widget(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);
    g_return_val_if_fail(sheet->sheet_entry != NULL, NULL);

    return (sheet->sheet_entry);
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

    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (!sheet->sheet_entry)   /* PR#102114 */
	return(NULL);

    entry = gtk_sheet_get_entry(sheet);
    g_return_val_if_fail(entry != NULL, NULL);

    if (GTK_IS_EDITABLE(entry))
    {
	text = gtk_editable_get_chars(GTK_EDITABLE(entry), 0, -1);
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry) 
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextIter start, end;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	text = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
    }
    else
    {
	g_warning("gtk_sheet_get_entry_text: no GTK_EDITABLE, don't know how to get the text.");
    }
    return (text);
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

    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail(entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
	gint position = 0;
	gtk_editable_delete_text(GTK_EDITABLE(entry), 0, -1);
	gtk_editable_insert_text(GTK_EDITABLE(entry), text, -1, &position);
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry) 
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
	GtkTextIter iter;

	gtk_text_buffer_set_text(buffer, text, -1);

	gtk_text_buffer_get_start_iter(buffer, &iter);
	gtk_text_buffer_place_cursor(buffer, &iter);
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

    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail(entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
	gtk_editable_set_editable(GTK_EDITABLE(entry), editable);
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry)
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	gtk_text_view_set_editable(GTK_TEXT_VIEW(entry), editable);
    }
    else
    {
	g_warning("gtk_sheet_set_entry_editable: no GTK_EDITABLE, don't know how to set editable.");
    }
}

/**
 * gtk_sheet_entry_select_region:
 * @sheet: a #GtkSheet 
 * @start_pos: start of region 
 * @end_pos: end of region 
 *
 * Selects a region of text.
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 */
void gtk_sheet_entry_select_region(GtkSheet *sheet, gint start_pos, gint end_pos)
{
    GtkWidget *entry = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail(entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
	gtk_editable_select_region(GTK_EDITABLE(entry), start_pos, end_pos);
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry)
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
	GtkTextIter start, end;

	gtk_text_buffer_get_iter_at_offset(buffer, &start, start_pos);
	gtk_text_buffer_get_iter_at_offset(buffer, &end, end_pos);
	gtk_text_buffer_select_range(buffer, &start, &end);
    }
    else
    {
	g_warning("gtk_sheet_entry_select_region: no GTK_EDITABLE, don't know how to select region.");
    }
}

/**
 * gtk_sheet_entry_signal_connect_changed:
 * @sheet: a #GtkSheet 
 * @handler: (scope notified) the signal handler
 *
 * Connect a handler to the sheet_entry "changed" signal. The 
 * user_data argument of the handler will be filled with the 
 * #GtkSheet. 
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

    if (!sheet->sheet_entry)   /* PR#102114 */
	return(handler_id);

    entry = gtk_sheet_get_entry(sheet);
    g_return_val_if_fail(entry != NULL, handler_id);

    if (GTK_IS_EDITABLE(entry))
    {
	handler_id = g_signal_connect(G_OBJECT(entry),
	    "changed", handler, GTK_OBJECT(sheet));
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry) 
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));

	handler_id = g_signal_connect(G_OBJECT(buffer),
	    "changed", handler, GTK_OBJECT(sheet));
    }
    else
    {
	g_warning("gtk_sheet_entry_signal_connect_changed: no GTK_EDITABLE, don't know how to get editable.");
    }
    return (handler_id);
}

/**
 * gtk_sheet_entry_signal_disconnect_by_func:
 * @sheet: a #GtkSheet 
 * @handler: (scope call) the signal handler
 *
 * Disconnect a handler from the sheet_entry "changed" signal
 *  
 * This function is mainly used to synchronize a second entry 
 * widget with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 */
void gtk_sheet_entry_signal_disconnect_by_func(GtkSheet *sheet, GCallback handler)
{
    GtkWidget *entry = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail(entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
	g_signal_handlers_disconnect_by_func(G_OBJECT(entry),
	    handler, GTK_OBJECT(sheet));
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry)
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));

	g_signal_handlers_disconnect_by_func(G_OBJECT(buffer),
	    handler, GTK_OBJECT(sheet));
    }
    else
    {
	g_warning("gtk_sheet_entry_signal_disconnect_by_func: no GTK_EDITABLE, don't know how to get editable.");
    }
}



/* BUTTONS */
static void
row_button_set(GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow)
	return;
    if (sheet->row[row].button.state == GTK_STATE_ACTIVE)
	return;

    sheet->row[row].button.state = GTK_STATE_ACTIVE;
    _gtk_sheet_draw_button(sheet, row, -1);
}

static void
row_button_release(GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow)
	return;
    if (sheet->row[row].button.state == GTK_STATE_NORMAL)
	return;

    sheet->row[row].button.state = GTK_STATE_NORMAL;
    _gtk_sheet_draw_button(sheet, row, -1);
}

/**
 * _gtk_sheet_draw_button: 
 * @sheet:  the #GtkSheet
 * @row:    row index
 * @col:    column index 
 *  
 * draw a sheet button
 * if row == -1 draw a column button
 * if col == -1 draw a row button
 * if both == -1 draw the sheet button
 * 
 */
void
_gtk_sheet_draw_button(GtkSheet *sheet, gint row, gint col)
{
    GdkWindow *window = NULL;
    GtkShadowType shadow_type;
    guint width = 0, height = 0;
    gint x = 0, y = 0;
    gint index = 0;
    guint text_width = 0, text_height = 0;
    GtkSheetButton *button = NULL;
    GtkSheetChild *child = NULL;
    GdkRectangle allocation;
    gboolean sensitive = FALSE;
    gint state = 0;
    gchar * label, label_buf[10];
    PangoAlignment pango_alignment = PANGO_ALIGN_LEFT;
    GtkSheetArea area = ON_SHEET_BUTTON_AREA;
    PangoFontDescription *font_desc =
	gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc;
    PangoRectangle extent;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if ((row == -1) && (col == -1))
	return;

#if GTK_SHEET_DEBUG_DRAW_BUTTON > 0
    g_debug("_gtk_sheet_draw_button: row %d col %d", row, col);
#endif

    if (row >= 0)
    {
	if (row > sheet->maxrow)
	    return;
	if (!sheet->row_titles_visible)
	    return;
	if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	    return;
	if (row < MIN_VIEW_ROW(sheet))
	    return;
	if (row > MAX_VIEW_ROW(sheet))
	    return;
    }

    if (col >= 0)
    {
	if (col > sheet->maxcol)
	    return;
	if (!sheet->column_titles_visible)
	    return;
	if (!GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)))
	    return;
	if (col < MIN_VIEW_COLUMN(sheet))
	    return;
	if (col > MAX_VIEW_COLUMN(sheet))
	    return;
    }

    if (row == -1)
    {
	window = sheet->column_title_window;
	button = &COLPTR(sheet, col)->button;
	index = col;
	x = _gtk_sheet_column_left_xpixel(sheet, col) + CELL_SPACING;
	if (sheet->row_titles_visible)
	    x -= sheet->row_title_area.width;
	y = 0;
	width = COLPTR(sheet, col)->width;
	height = sheet->column_title_area.height;
	sensitive = GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, col));
	area = ON_COLUMN_TITLES_AREA;
    }
    else if (col == -1)
    {
	window = sheet->row_title_window;
	button = &sheet->row[row].button;
	index = row;
	x = 0;
	y = _gtk_sheet_row_top_ypixel(sheet, row) + CELL_SPACING;
	if (sheet->column_titles_visible)
	    y -= sheet->column_title_area.height;
	width = sheet->row_title_area.width;
	height = sheet->row[row].height;
	sensitive = GTK_SHEET_ROW_IS_SENSITIVE(ROWPTR(sheet, row));
	area = ON_ROW_TITLES_AREA;
    }

    allocation.x = x;
    allocation.y = y;
    allocation.width = width;
    allocation.height = height;

    gdk_window_clear_area(window, x, y, width, height);

    state = button->state;
    if (!sensitive)
	state = GTK_STATE_INSENSITIVE;

    if (state == GTK_STATE_ACTIVE)
	shadow_type = GTK_SHADOW_IN;
    else
	shadow_type = GTK_SHADOW_OUT;

    if (state != GTK_STATE_NORMAL && state != GTK_STATE_INSENSITIVE)
	gtk_paint_box(gtk_widget_get_style(sheet->button), window,
	    button->state, shadow_type,
	    &allocation, GTK_WIDGET(sheet->button),
	    "table-heading", x, y, width, height);
    else
	gtk_paint_box(gtk_widget_get_style(sheet->button), window,
	    GTK_STATE_NORMAL, GTK_SHADOW_OUT,
	    &allocation, GTK_WIDGET(sheet->button),
	    "table-heading", x, y, width, height);

    if (button->label_visible)
    {
	PangoLayout *layout = NULL;
	gint real_x = x, real_y;

	text_height =
	    _gtk_sheet_row_default_height(GTK_WIDGET(sheet)) - 2 * CELLOFFSET;

	gdk_gc_set_clip_rectangle(
	    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[button->state],
	    &allocation);
	gdk_gc_set_clip_rectangle(
	    gtk_widget_get_style(GTK_WIDGET(sheet))->white_gc,
	    &allocation);

	y += 2 * gtk_widget_get_style(sheet->button)->ythickness;

	real_y = y;
	label = button->label;

	if (!label || !label[0])   /* revert to auto-generated numeric label */
	{
	    sprintf(label_buf, "%d", index);
	    label = label_buf;
	}

	layout = gtk_widget_create_pango_layout(GTK_WIDGET(sheet), label);
	pango_layout_set_font_description(layout, font_desc);

	pango_layout_get_pixel_extents(layout, NULL, &extent);
	text_width = extent.width;

	switch(button->justification)
	{
	    case GTK_JUSTIFY_LEFT:
		real_x = x + CELLOFFSET;
		pango_alignment = PANGO_ALIGN_LEFT;
		break;

	    case GTK_JUSTIFY_RIGHT:
		real_x = x + width - text_width - CELLOFFSET;
		pango_alignment = PANGO_ALIGN_RIGHT;
		break;

	    case GTK_JUSTIFY_FILL:
		pango_layout_set_justify(layout, TRUE);
		/* FALLTHROUGH */

	    case GTK_JUSTIFY_CENTER:
		real_x = x + (width - text_width) / 2;
		pango_alignment = PANGO_ALIGN_CENTER;

	    default:
		break;
	}
	pango_layout_set_alignment(layout, pango_alignment);
	gtk_paint_layout(gtk_widget_get_style(GTK_WIDGET(sheet)),
	    window,
	    state,
	    FALSE,
	    &allocation,
	    GTK_WIDGET(sheet),
	    "label",
	    real_x, real_y,
	    layout);
	g_object_unref(G_OBJECT(layout));

	gdk_gc_set_clip_rectangle(
	    gtk_widget_get_style(GTK_WIDGET(sheet))->fg_gc[button->state],
	    NULL);
	gdk_gc_set_clip_rectangle(
	    gtk_widget_get_style(GTK_WIDGET(sheet))->white_gc,
	    NULL);
    }

    gtk_sheet_draw_tooltip_marker(sheet, area, row, col);

    if ((child = button->child) && (child->widget))
    {
	GtkRequisition requisition;

	child->x = allocation.x;
	child->y = allocation.y;

	gtk_widget_get_requisition(child->widget, &requisition);

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

/**
 * _gtk_sheet_scrollbar_adjust:
 * 
 * @param sheet  the #GtkSheet
 *  
 * recalculate scrollbar adjustments and emit changed signals 
 */
void
_gtk_sheet_scrollbar_adjust(GtkSheet *sheet)
{
#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
    g_debug("_gtk_sheet_scrollbar_adjust: called");
#endif

    if (sheet->vadjustment)
    {
	GtkAdjustment *va = sheet->vadjustment;

	gint upper = _gtk_sheet_height(sheet) + 80;
	gint page_size = sheet->sheet_window_height;

	gtk_adjustment_configure(va,
	    gtk_adjustment_get_value(va),  /* value */
	    0.0,  /* lower */
	    upper,  /* upper */
	    _gtk_sheet_row_default_height(GTK_WIDGET(sheet)), /* step_increment */
	    sheet->sheet_window_height / 2, /* page_increment */
	    page_size  /* page_size */
	    );

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	g_debug("_gtk_sheet_scrollbar_adjust: va PS %d PI %g SI %g L %g U %d V %g VO %d",
	    page_size, 
	    gtk_adjustment_get_page_increment(va), 
	    gtk_adjustment_get_step_increment(va),
	    gtk_adjustment_get_lower(va), 
	    upper, 
	    gtk_adjustment_get_value(va), 
	    sheet->voffset);
#endif

	if (upper <= page_size)  /* whole sheet fits into window? */
	{
#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	    g_debug("_gtk_sheet_scrollbar_adjust: reset V to 0");
#endif
	    gtk_adjustment_set_value(va, 0);
	    gtk_adjustment_value_changed(va);
	}
	/* can produce smudge effects - 23.3.13/fp 
	else if (va->value >= va->upper - va->page_size)
	{
	    va->value = va->upper - va->page_size;
#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	    g_debug("_gtk_sheet_scrollbar_adjust: reset to V %g VO %d", 
		va->value, sheet->voffset);
#endif
	    g_signal_emit_by_name(GTK_OBJECT(va), "value_changed");
	} 
	*/ 
	
	gtk_adjustment_changed(va);
    }

    if (sheet->hadjustment)
    {
	GtkAdjustment *ha = sheet->hadjustment;

	/* gint upper = _gtk_sheet_width(sheet) * 3 / 2; */
	gint upper = _gtk_sheet_width(sheet) + 80;
	gint page_size = sheet->sheet_window_width;

	gtk_adjustment_configure(ha,
	    gtk_adjustment_get_value(ha),  /* value */
	    0.0,  /* lower */
	    upper,  /* upper */
	    GTK_SHEET_COLUMN_DEFAULT_WIDTH, /* step_increment */
	    sheet->sheet_window_width / 2, /* page_increment */
	    page_size  /* page_size */
	    );

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	g_debug("_gtk_sheet_scrollbar_adjust: ha PS %d PI %g SI %g L %g U %d V %g HO %d",
	    page_size, 
	    gtk_adjustment_get_page_increment(ha), 
	    gtk_adjustment_get_step_increment(ha),
	    gtk_adjustment_get_lower(ha), 
	    upper, 
	    gtk_adjustment_get_value(ha), sheet->hoffset);
#endif

	if (upper <= page_size)  /* whole sheet fits into window? */
	{
#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	    g_debug("_gtk_sheet_scrollbar_adjust: reset V to 0");
#endif
	    gtk_adjustment_set_value(ha, 0);
	    gtk_adjustment_value_changed(ha);
	}
	/* can produce smudge effects - 23.3.13/fp
	else if (ha->value >= ha->upper - ha->page_size)
	{
	    ha->value = ha->upper - ha->page_size;
#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	    g_debug("_gtk_sheet_scrollbar_adjust: reset to V %g HO %d", 
		va->value, sheet->hoffset);
#endif
	    g_signal_emit_by_name(GTK_OBJECT(ha), "value_changed");
	} 
	*/ 
	
	gtk_adjustment_changed(ha);
    }
}


/*
 * _vadjustment_changed_handler:
 * 
 * this is the #GtkSheet vertical adjustment "changed" signal handler
 * 
 * @param adjustment the #GtkAdjustment that received the signal
 * @param data the #GtkSheet passed on signal creation
 */
static void
_vadjustment_changed_handler(GtkAdjustment *adjustment,
    gpointer data)
{
    GtkSheet *sheet;

    g_return_if_fail(adjustment != NULL);
    g_return_if_fail(data != NULL);

    sheet = GTK_SHEET(data);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 1
    g_debug("_vadjustment_changed_handler: called: O VA %g VO %d",
	sheet->old_vadjustment, sheet->voffset);
#endif
}

/*
 * _hadjustment_changed_handler:
 * 
 * this is the #GtkSheet horizontal adjustment "change" handler
 * 
 * @param adjustment the #GtkAdjustment that received the signal
 * @param data       the #GtkSheet passed on signal creation
 */
static void
_hadjustment_changed_handler(GtkAdjustment *adjustment, gpointer data)
{
    GtkSheet *sheet;

    g_return_if_fail(adjustment != NULL);
    g_return_if_fail(data != NULL);

    sheet = GTK_SHEET(data);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 1
    g_debug("_hadjustment_changed_handler: called: O VA %g VO %d",
	sheet->old_vadjustment, sheet->voffset);
#endif
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
_vadjustment_value_changed_handler(GtkAdjustment *adjustment, gpointer data)
{
    GtkSheet *sheet;
    gint old_value;

    g_return_if_fail(adjustment != NULL);
    g_return_if_fail(data != NULL);
    g_return_if_fail(GTK_IS_SHEET(data));

    sheet = GTK_SHEET(data);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
    g_debug("_vadjustment_value_changed_handler: called: O VA %g VO %d",
	sheet->old_vadjustment, sheet->voffset);
#endif

    if (GTK_SHEET_IS_FROZEN(sheet))
	return;

#if 0
    if (sheet->column_titles_visible)
	row = _gtk_sheet_row_from_ypixel(sheet, sheet->column_title_area.height + CELL_SPACING);
    else
	row = _gtk_sheet_row_from_ypixel(sheet, CELL_SPACING);

    old_value = -sheet->voffset;

    for (i = 0; i < sheet->maxrow; i++)  /* all but the last row */
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, i)))
	    y += sheet->row[i].height;
	if (y > gtk_adjustment_get_value(adjustment))
	    break;
    }
    if (0 <= i && i <= sheet->maxrow)
	y -= sheet->row[i].height;
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
    if (gtk_adjustment_get_value(adjustment) > sheet->old_vadjustment && sheet->old_vadjustment > 0. &&
	0 <= new_row && new_row <= sheet->maxrow &&
	sheet->row[new_row].height > gtk_adjustment_get_step_increment(sheet->vadjustment))
    {
	new_row += 1;
	y = y + sheet->row[row].height;
    }
#endif

    /* Negative old_adjustment enforces the redraw, otherwise avoid spureous redraw */
    if (sheet->old_vadjustment >= 0. && row == new_row)
    {
	sheet->old_vadjustment = gtk_adjustment_get_value(sheet->vadjustment);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
	g_debug("_vadjustment_value_changed_handler: return 1: vv %g",
	    sheet->old_vadjustment);
#endif
	return;
    }

    sheet->old_vadjustment = gtk_adjustment_get_value(sheet->vadjustment);
    gtk_adjustment_set_value(adjustment, y);

    if (new_row < 0 || new_row > sheet->maxrow)
    {
	gtk_adjustment_set_step_increment(sheet->vadjustment, 
	    GTK_SHEET_ROW_DEFAULT_HEIGHT);
    }
    else if (new_row == 0)
    {
	gtk_adjustment_set_step_increment(sheet->vadjustment, 
	    sheet->row[0].height);
    }
    else
    {
	gtk_adjustment_set_step_increment(sheet->vadjustment,
	    MIN(sheet->row[new_row].height, sheet->row[new_row - 1].height));
    }

    value = gtk_adjustment_get_value(adjustment);
    gtk_adjustment_set_value(sheet->vadjustment, value);

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
#else
    /* Negative old_adjustment enforces the redraw, otherwise avoid spureous redraw */
    old_value = sheet->old_vadjustment;
    sheet->old_vadjustment = gtk_adjustment_get_value(sheet->vadjustment);

    if (old_value >= 0. && sheet->voffset == -1 *  gtk_adjustment_get_value(adjustment))
	return;

    gdouble value = gtk_adjustment_get_value(adjustment);
    gtk_adjustment_set_value(sheet->vadjustment, value);
    sheet->voffset = -value;
#endif

    _gtk_sheet_recalc_view_range(sheet);

    if (gtk_widget_get_realized(sheet->sheet_entry) &&
	sheet->state == GTK_SHEET_NORMAL &&
	sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0 &&
	!gtk_sheet_cell_isvisible(sheet, sheet->active_cell.row, sheet->active_cell.col))
    {
	gchar *text = gtk_sheet_get_entry_text(sheet);

	if (!text || !text[0])
	{
	    gtk_sheet_cell_clear(sheet,
		sheet->active_cell.row, sheet->active_cell.col);
	}
	g_free(text);

	gtk_widget_unmap(sheet->sheet_entry);
    }


    gtk_sheet_position_children(sheet);

    size_allocate_global_button(sheet);
    size_allocate_row_title_buttons(sheet);

    _gtk_sheet_range_draw(sheet, NULL, TRUE);
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
_hadjustment_value_changed_handler(GtkAdjustment *adjustment, gpointer data)
{
    GtkSheet *sheet;
    gint old_value;

    g_return_if_fail(adjustment != NULL);
    g_return_if_fail(data != NULL);
    g_return_if_fail(GTK_IS_SHEET(data));

    sheet = GTK_SHEET(data);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 0
    g_debug("_hadjustment_value_changed_handler: called: O HA %g HO %d",
	sheet->old_hadjustment, sheet->hoffset);
#endif

    if (GTK_SHEET_IS_FROZEN(sheet))
	return;

#if 0
    if (sheet->row_titles_visible) col = _gtk_sheet_column_from_xpixel(sheet, sheet->row_title_area.width + CELL_SPACING);
    else
    col = _gtk_sheet_column_from_xpixel(sheet, CELL_SPACING);

    old_value = -sheet->hoffset;

    for (i = 0; i < sheet->maxcol; i++)  /* all but the last column */
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, i))) x += COLPTR(sheet, i)->width;
	if (x > adjustment->value) break;
    }
    if (0 <= i && i <= sheet->maxcol) x -= COLPTR(sheet, i)->width;
    new_col = i;

    x = MAX(x, 0);

#if 0
    if (adjustment->value > sheet->old_hadjustment && sheet->old_hadjustment > 0 &&
	0 <= new_col && new_col <= sheet->maxcol &&
	COLPTR(sheet, new_col)->width > sheet->hadjustment->step_increment)
    {
	/* This avoids embarrassing twitching */
	if (col == new_col && col != sheet->maxcol &&
	    adjustment->value - sheet->old_hadjustment >=
	    sheet->hadjustment->step_increment &&
	    new_col + 1 != MIN_VISIBLE_COLUMN(sheet))
	{
	    new_col+=1;
	    x=x+COLPTR(sheet, col)->width;
	}
    }
#else
    if (adjustment->value > sheet->old_hadjustment && sheet->old_hadjustment > 0 &&
	0 <= new_col && new_col <= sheet->maxcol &&
	COLPTR(sheet, new_col)->width > sheet->hadjustment->step_increment)
    {
	new_col += 1;
	x = x + COLPTR(sheet, col)->width;
    }
#endif

    /* Negative old_adjustment enforces the redraw, otherwise avoid spureous redraw */
    if (sheet->old_hadjustment >= 0. && new_col == col)
    {
	sheet->old_hadjustment = sheet->hadjustment->value;
	return;
    }

    sheet->old_hadjustment = sheet->hadjustment->value;
    adjustment->value = x;

    if (new_col < 0 || new_col > sheet->maxcol)
    {
	sheet->hadjustment->step_increment = GTK_SHEET_COLUMN_DEFAULT_WIDTH;
    }
    else if (new_col == 0)
    {
	sheet->hadjustment->step_increment = COLPTR(sheet, 0)->width;
    }
    else
    {
	sheet->hadjustment->step_increment =
	MIN(COLPTR(sheet, new_col)->width, COLPTR(sheet, new_col - 1)->width);
    }

    sheet->hadjustment->value = adjustment->value;
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
#else
    /* Negative old_adjustment enforces the redraw, otherwise avoid spureous redraw */
    old_value = sheet->old_hadjustment;
    sheet->old_hadjustment = gtk_adjustment_get_value(sheet->hadjustment);

    if (old_value >= 0. && sheet->hoffset == -1 *  gtk_adjustment_get_value(adjustment))
	return;

    gdouble value = gtk_adjustment_get_value(adjustment);
    gtk_adjustment_set_value(sheet->hadjustment, value);
    sheet->hoffset = -value;
#endif

    _gtk_sheet_recalc_view_range(sheet);

    if (gtk_widget_get_realized(sheet->sheet_entry) &&
	sheet->state == GTK_SHEET_NORMAL &&
	sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0 &&
	!gtk_sheet_cell_isvisible(sheet, sheet->active_cell.row, sheet->active_cell.col))
    {
	gchar *text = gtk_sheet_get_entry_text(sheet);

	if (!text || !text[0])
	{
	    gtk_sheet_cell_clear(sheet,
		sheet->active_cell.row, sheet->active_cell.col);
	}
	g_free(text);

	gtk_widget_unmap(sheet->sheet_entry);
    }

    gtk_sheet_position_children(sheet);

    size_allocate_global_button(sheet);
    _gtk_sheet_column_buttons_size_allocate(sheet);

    _gtk_sheet_range_draw(sheet, NULL, TRUE);
}


/* COLUMN RESIZING */
static void
draw_xor_vline(GtkSheet *sheet)
{
    GtkWidget *widget;

    g_return_if_fail(sheet != NULL);

    widget = GTK_WIDGET(sheet);

    gdk_draw_line(gtk_widget_get_window(widget), sheet->xor_gc,
	sheet->x_drag,
	sheet->column_title_area.height,
	sheet->x_drag,
	sheet->sheet_window_height + 1);
}

/* ROW RESIZING */
static void
draw_xor_hline(GtkSheet *sheet)
{
    GtkWidget *widget;

    g_return_if_fail(sheet != NULL);

    widget = GTK_WIDGET(sheet);

    gdk_draw_line(gtk_widget_get_window(widget), sheet->xor_gc,
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

    if (range.col0 < 0 || range.coli < 0 || range.row0 < 0 || range.rowi < 0)
	return;  /* PR#203012 */

    area.x = _gtk_sheet_column_left_xpixel(sheet, range.col0);
    area.y = _gtk_sheet_row_top_ypixel(sheet, range.row0);
    area.width = _gtk_sheet_column_left_xpixel(sheet, range.coli) - area.x +
	COLPTR(sheet, range.coli)->width;
    area.height = _gtk_sheet_row_top_ypixel(sheet, range.rowi) - area.y +
	sheet->row[range.rowi].height;

    clip_area.x = sheet->row_title_area.width;
    clip_area.y = sheet->column_title_area.height;
    clip_area.width = sheet->sheet_window_width;
    clip_area.height = sheet->sheet_window_height;

    if (!sheet->row_titles_visible)
	clip_area.x = 0;
    if (!sheet->column_titles_visible)
	clip_area.y = 0;

    if (area.x < 0)
    {
	area.width = area.width + area.x;
	area.x = 0;
    }
    if (area.width > clip_area.width)
	area.width = clip_area.width + 10;
    if (area.y < 0)
    {
	area.height = area.height + area.y;
	area.y = 0;
    }
    if (area.height > clip_area.height)
	area.height = clip_area.height + 10;

    clip_area.x--;
    clip_area.y--;
    clip_area.width += 3;
    clip_area.height += 3;

    gdk_gc_get_values(sheet->xor_gc, &values);

    gdk_gc_set_clip_rectangle(sheet->xor_gc, &clip_area);

    for (i = -1; i <= 1; ++i) gdk_draw_rectangle(sheet->sheet_window,
	    sheet->xor_gc,
	    FALSE,
	    area.x + i, area.y + i,
	    area.width - 2 * i, area.height - 2 * i);


    gdk_gc_set_clip_rectangle(sheet->xor_gc, NULL);

    gdk_gc_set_foreground(sheet->xor_gc, &values.foreground);
}


/* this function returns the new width of the column being resized given
 * the column and x position of the cursor; the x cursor position is passed
 * in as a pointer and automaticaly corrected if it's beyond min/max limits */

static guint
new_column_width(GtkSheet *sheet, gint col, gint *x)
{
    gint cx, width;
    GtkRequisition requisition;

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("new_column_width: called");
#endif

    cx = *x;

    requisition.width = COLPTR(sheet, col)->requisition;

    /* you can't shrink a column to less than its minimum width */
    if (cx < _gtk_sheet_column_left_xpixel(sheet, col) + requisition.width)
    {
	*x = cx = _gtk_sheet_column_left_xpixel(sheet, col) + requisition.width;
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
    width = cx - _gtk_sheet_column_left_xpixel(sheet, col);
    if (width < requisition.width)
	width = requisition.width;

    COLPTR(sheet, col)->width = width;
    _gtk_sheet_recalc_left_xpixels(sheet);
    _gtk_sheet_recalc_view_range(sheet);

    _gtk_sheet_column_buttons_size_allocate(sheet);

    return (width);
}

/* this function returns the new height of the row being resized given
 * the row and y position of the cursor; the y cursor position is passed
 * in as a pointer and automaticaly corrected if it's beyond min/max limits */

static guint
new_row_height(GtkSheet *sheet, gint row, gint *y)
{
    GtkRequisition requisition;
    gint cy, height;

    cy = *y;

    requisition.height = sheet->row[row].requisition;

    /* you can't shrink a row to less than its minimum height */
    if (cy < _gtk_sheet_row_top_ypixel(sheet, row) + requisition.height)
    {
	*y = cy = _gtk_sheet_row_top_ypixel(sheet, row) + requisition.height;
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
    height = (cy - _gtk_sheet_row_top_ypixel(sheet, row));
    if (height < requisition.height)
	height = requisition.height;

    sheet->row[row].height = height;
    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_view_range(sheet);

    size_allocate_row_title_buttons(sheet);

    return (height);
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
gtk_sheet_set_row_height(GtkSheet *sheet, gint row, guint height)
{
    guint min_height;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow)
	return;

    gtk_sheet_row_size_request(sheet, row, &min_height);
    if (height < min_height)
	return;

    sheet->row[row].height = height;

    _gtk_sheet_recalc_top_ypixels(sheet);

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) && !GTK_SHEET_IS_FROZEN(sheet))
    {
	size_allocate_row_title_buttons(sheet);
	_gtk_sheet_scrollbar_adjust(sheet);
	_gtk_sheet_entry_size_allocate(sheet);
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
    }

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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    AddColumns(sheet, sheet->maxcol + 1, ncols);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    _gtk_sheet_scrollbar_adjust(sheet);

    if (sheet->state == GTK_SHEET_ROW_SELECTED)
	sheet->range.coli += ncols;

    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    AddRows(sheet, sheet->maxrow + 1, nrows);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
	sheet->range.rowi += nrows;

    _gtk_sheet_scrollbar_adjust(sheet);

    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    gtk_sheet_real_unselect_range(sheet, NULL);

    InsertRow(sheet, row, nrows);

    children = sheet->children;
    while (children)
    {
	child = (GtkSheetChild *)children->data;

	if (child->attached_to_cell)
	    if (child->row >= row)
		child->row += nrows;

	children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
	sheet->range.rowi += nrows;

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    gtk_sheet_real_unselect_range(sheet, NULL);

    InsertColumn(sheet, col, ncols);

    children = sheet->children;
    while (children)
    {
	child = (GtkSheetChild *)children->data;

	if (child->attached_to_cell)
	    if (child->col >= col)
		child->col += ncols;

	children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if (sheet->state == GTK_SHEET_ROW_SELECTED)
	sheet->range.coli += ncols;

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    nrows = MIN(nrows, sheet->maxrow - row + 1);

    act_row = sheet->active_cell.row;
    act_col = sheet->active_cell.col;

    _gtk_sheet_hide_active_cell(sheet);
    gtk_sheet_real_unselect_range(sheet, NULL);

    DeleteRow(sheet, row, nrows);

    children = sheet->children;
    while (children)
    {
	child = (GtkSheetChild *)children->data;

	if (child->attached_to_cell &&
	    child->row >= row && child->row < row + nrows &&
	    gtk_widget_get_realized(child->widget))
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

	if (child->attached_to_cell && child->row > row)
	    child->row -= nrows;
	children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    sheet->active_cell.row = -1;
    sheet->active_cell.col = -1;

/* if(sheet->state == GTK_SHEET_ROW_SELECTED)
*/

    act_row = MIN(act_row, sheet->maxrow);
    act_row = MAX(act_row, 0);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);

    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    ncols = MIN(ncols, sheet->maxcol - col + 1);

    _gtk_sheet_hide_active_cell(sheet);
    gtk_sheet_real_unselect_range(sheet, NULL);

    DeleteColumn(sheet, col, ncols);

    children = sheet->children;
    while (children)
    {
	child = (GtkSheetChild *)children->data;

	if (child->attached_to_cell &&
	    child->col >= col && child->col < col + ncols &&
	    gtk_widget_get_realized(child->widget))
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

	if (child->attached_to_cell && child->col > col)
	    child->col -= ncols;
	children = children->next;
    }

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    act_row = sheet->active_cell.row;
    act_col = sheet->active_cell.col;

    sheet->active_cell.row = -1;
    sheet->active_cell.col = -1;

/* if(sheet->state == GTK_SHEET_COLUMN_SELECTED)
*/

    act_col = MIN(act_col, sheet->maxcol);
    act_col = MAX(act_col, 0);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);

    /* 22.06.13/fp - want same behaviour as gtk_sheet_delete_rows()
       gtk_sheet_click_cell(sheet, act_row, act_col, &veto); */
    gtk_sheet_activate_cell(sheet, sheet->active_cell.row, sheet->active_cell.col);
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
gtk_sheet_range_set_background(GtkSheet *sheet,
    const GtkSheetRange *urange,
    const GdkColor *color)
{
    gint i, j;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

#if GTK_SHEET_DEBUG_COLORS > 0
    g_debug("gtk_sheet_range_set_background: %s row %d-%d col %d-%d)",
	gdk_color_to_string(color), range.row0, range.rowi, range.col0, range.coli);
#endif

    for (i = range.row0; i <= range.rowi; i++) for (j = range.col0; j <= range.coli; j++)
    {
	GtkSheetCellAttr attributes;
	gtk_sheet_get_attributes(sheet, i, j, &attributes);

	if (color != NULL)
	    attributes.background = *color;
	else
	    attributes.background = sheet->bg_color;
	
	gdk_colormap_alloc_color(gdk_colormap_get_system(), &attributes.background, FALSE, TRUE);

	gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
gtk_sheet_range_set_foreground(GtkSheet *sheet,
    const GtkSheetRange *urange,
    const GdkColor *color)
{
    gint i, j;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

#if GTK_SHEET_DEBUG_COLORS > 0
    g_debug("gtk_sheet_range_set_foreground: %s row %d-%d col %d-%d)",
	gdk_color_to_string(color), range.row0, range.rowi, range.col0, range.coli);
#endif

    for (i = range.row0; i <= range.rowi; i++) for (j = range.col0; j <= range.coli; j++)
    {
	GtkSheetCellAttr attributes;
	gtk_sheet_get_attributes(sheet, i, j, &attributes);

	if (color != NULL)
	    attributes.foreground = *color;
	else
	    gdk_color_black(gdk_colormap_get_system(), &attributes.foreground);

	gdk_colormap_alloc_color(gdk_colormap_get_system(), &attributes.foreground, FALSE, TRUE);

	gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (i = range.row0; i <= range.rowi; i++)
    {
	for (j = range.col0; j <= range.coli; j++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, i, j, &attributes);
	    attributes.justification = just;
	    gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
	}
    }

    range.col0 = sheet->view.col0;
    range.coli = sheet->view.coli;

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (i = range.row0; i <= range.rowi; i++)
    {
	for (j = range.col0; j <= range.coli; j++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, i, j, &attributes);
	    attributes.is_editable = editable;
	    gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
	}
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (i = range.row0; i <= range.rowi; i++)
    {
	for (j = range.col0; j <= range.coli; j++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, i, j, &attributes);
	    attributes.is_visible = visible;
	    gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
	}
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (i = range.row0; i <= range.rowi; i++)
    {
	for (j = range.col0; j <= range.coli; j++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, i, j, &attributes);
	    attributes.border.mask = mask;
	    attributes.border.width = width;
	    attributes.border.line_style = line_style;
	    attributes.border.cap_style = GDK_CAP_NOT_LAST;
	    attributes.border.join_style = GDK_JOIN_MITER;
	    gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
	}
    }

    range.row0--;
    range.col0--;
    range.rowi++;
    range.coli++;

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
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
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (i = range.row0; i <= range.rowi; i++)
    {
	for (j = range.col0; j <= range.coli; j++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, i, j, &attributes);
	    attributes.border.color = *color;
	    gtk_sheet_set_cell_attributes(sheet, i, j, attributes);
	}
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
}

/**
 * gtk_sheet_range_set_font:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange where we set font_desc.
 * @font_desc: (transfer none) a #PangoFontDescription.
 *
 * Set font_desc for the given range.
 */
void
gtk_sheet_range_set_font(GtkSheet *sheet,
    const GtkSheetRange *urange,
    PangoFontDescription *font_desc)
{
    gint i, j;
    gint font_height;
    GtkSheetRange range;
    PangoContext *context;
    PangoFontMetrics *metrics;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    gtk_sheet_freeze(sheet);

    context = gtk_widget_get_pango_context(GTK_WIDGET(sheet));
    metrics = pango_context_get_metrics(context,
	font_desc,
	pango_context_get_language(context));
    font_height = pango_font_metrics_get_descent(metrics) +
	pango_font_metrics_get_ascent(metrics);
    font_height = PANGO_PIXELS(font_height) + 2 * CELLOFFSET;

    for (i = range.row0; i <= range.rowi; i++)
    {
	for (j = range.col0; j <= range.coli; j++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, i, j, &attributes);
	    attributes.font_desc = pango_font_description_copy(font_desc);  /* copy */
	    attributes.do_font_desc_free = TRUE;

	    if (font_height > sheet->row[i].height)
	    {
		sheet->row[i].height = font_height;
		_gtk_sheet_recalc_top_ypixels(sheet);
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

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;

    CheckCellData(sheet, row, col);

    cell = sheet->data[row][col];

    if (!cell->attributes)
	cell->attributes = g_new(GtkSheetCellAttr, 1);

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

    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

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
	return (FALSE);
    }

    *attributes = *(cell->attributes);

    if (COLPTR(sheet, col)->justification != GTK_SHEET_COLUMN_DEFAULT_JUSTIFICATION)
    {
	attributes->justification = COLPTR(sheet, col)->justification;
    }

    return (TRUE);
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
	attributes->justification = GTK_SHEET_COLUMN_DEFAULT_JUSTIFICATION;
    else
	attributes->justification = COLPTR(sheet, col)->justification;

    attributes->border.width = 0;
    attributes->border.line_style = GDK_LINE_SOLID;
    attributes->border.cap_style = GDK_CAP_NOT_LAST;
    attributes->border.join_style = GDK_JOIN_MITER;
    attributes->border.mask = 0;
    attributes->border.color = gtk_widget_get_style(GTK_WIDGET(sheet))->black;
    attributes->is_editable = TRUE;
    attributes->is_visible = TRUE;
    attributes->font = gtk_widget_get_style(GTK_WIDGET(sheet))->private_font;

    attributes->font_desc = 
	gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc;  /* no copy */
    attributes->do_font_desc_free = FALSE;
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
    gint c;
    GtkSheetColumn *newobj;

    g_assert(ncols >= 0);
    g_assert(position >= 0 && position <= sheet->maxcol + 1);

    if (ncols > 0)
    {
	sheet->column = (GtkSheetColumn **)g_realloc(sheet->column,
	    (sheet->maxcol + 1 + ncols)* sizeof(GtkSheetColumn *));

	for (c = sheet->maxcol; c >= position; c--)  /* make space */
	{
	    sheet->column[c + ncols] = sheet->column[c];
	    sheet->column[c] = NULL;
	}

	for (c = 0; c < ncols; c++)
	{
	    gint newidx = position + c;

	    newobj = g_object_new(G_TYPE_SHEET_COLUMN, NULL);

#if GTK_SHEET_DEBUG_FINALIZE > 0
	    g_object_weak_ref(G_OBJECT(newobj), weak_notify, "Sheet-Column");
#endif

	    newobj->sheet = sheet;
	    sheet->column[newidx] = newobj;

	    gtk_widget_set_parent(GTK_WIDGET(newobj), GTK_WIDGET(sheet));

#if 0
	    g_debug("Setting Column %p Parent to GtkSheet %p - got %p",
		    newobj, sheet, gtk_widget_get_parent(GTK_WIDGET(newobj)));
#endif

	    g_object_ref_sink(newobj);
	}

	sheet->maxcol += ncols;

	_gtk_sheet_reset_text_column(sheet, sheet->maxcol - ncols);
	_gtk_sheet_recalc_left_xpixels(sheet);
    }
}

static void
InsertColumn(GtkSheet *sheet, gint position, gint ncols)
{
    gint r, c;

    g_assert(ncols >= 0);
    g_assert(position >= 0);

    AddColumns(sheet, position, ncols);

    _gtk_sheet_reset_text_column(sheet, sheet->maxcol - ncols);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (position <= sheet->maxalloccol)  /* adjust allocated cells */
    {
	GrowSheet(sheet, 0, ncols);

	for (r = 0; r <= sheet->maxallocrow; r++)
	{
	    for (c = sheet->maxalloccol; c >= position + ncols; c--)
	    {
		gtk_sheet_real_cell_clear(sheet, r, c, TRUE);

		sheet->data[r][c] = sheet->data[r][c - ncols];
		if (sheet->data[r][c])
		    sheet->data[r][c]->col = c;
		sheet->data[r][c - ncols] = NULL;
	    }
	}
    }
}

static void
DeleteColumn(GtkSheet *sheet, gint position, gint ncols)
{
    gint c, r;

    g_assert(ncols >= 0);
    g_assert(position >= 0);

    ncols = MIN(ncols, sheet->maxcol - position + 1);

    if (ncols <= 0 || position > sheet->maxcol)
	return;

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("DeleteColumn: pos %d ncols %d mxr %d mxc %d mxar %d mxac %d ", 
	position, ncols, 
	sheet->maxrow, sheet->maxcol, sheet->maxallocrow, sheet->maxalloccol);
#endif

    for (c = position; c < position + ncols; c++)  /* dispose columns */
    {
	sheet->column[c]->sheet = NULL;

	g_object_unref(sheet->column[c]);
	sheet->column[c] = NULL;
    }

    for (c = position; c <= sheet->maxcol - ncols; c++)  /* shift columns into position*/
    {
	sheet->column[c] = sheet->column[c + ncols];
    }

    for (c = sheet->maxcol - ncols + 1; c <= sheet->maxcol; c++)  /* clear tail */
    {
	sheet->column[c] = NULL;
    }

    /* to be done: shrink pointer pool via realloc */

    if (position <= sheet->maxalloccol)
    {
	for (c = position; c <= sheet->maxcol - ncols; c++)  /* shift column data */
	{
	    if (c > sheet->maxalloccol) break;
	    
	    for (r = 0; r <= sheet->maxallocrow; r++)
	    {
		gtk_sheet_real_cell_clear(sheet, r, c, TRUE);

		if (c + ncols <= sheet->maxalloccol)
		{
		    sheet->data[r][c] = sheet->data[r][c + ncols];
		    sheet->data[r][c + ncols] = NULL;
		    if (sheet->data[r][c])
			sheet->data[r][c]->col = c;
		}
	    }
	}

	for (c = sheet->maxcol - ncols + 1; c <= sheet->maxcol; c++)  /* clear tail */
	{
	    if (c > sheet->maxalloccol) break;
	    
	    for (r = 0; r <= sheet->maxallocrow; r++)
	    {
		gtk_sheet_real_cell_clear(sheet, r, c, TRUE);
	    }
	}

	sheet->maxalloccol -= MIN(ncols, sheet->maxalloccol - position + 1);
	sheet->maxalloccol = MIN(sheet->maxalloccol, sheet->maxcol);
    }

    sheet->maxcol -= ncols;

    _gtk_sheet_range_fixup(sheet, &sheet->view);
    _gtk_sheet_range_fixup(sheet, &sheet->range);

    _gtk_sheet_reset_text_column(sheet, position);
    _gtk_sheet_recalc_left_xpixels(sheet);
}

static void
AddRows(GtkSheet *sheet, gint position, gint nrows)
{
    gint r;

    g_assert(nrows >= 0);
    g_assert(position >= 0 && position <= sheet->maxrow + 1);

    if (nrows > 0)
    {
	sheet->row = (GtkSheetRow *)g_realloc(sheet->row,
	    (sheet->maxrow + 1 + nrows) * sizeof(GtkSheetRow));

	for (r = sheet->maxrow; r >= position; r--)  /* make space */
	{
	    sheet->row[r + nrows] = sheet->row[r];
	    _gtk_sheet_row_init(&sheet->row[r]);
	}

	for (r = 0; r < nrows; r++)
	{
	    gint newidx = position + r;

	    _gtk_sheet_row_init(&sheet->row[newidx]);

	    sheet->row[newidx].requisition = sheet->row[newidx].height =
		_gtk_sheet_row_default_height(GTK_WIDGET(sheet));
	}
	sheet->maxrow += nrows;

	_gtk_sheet_recalc_top_ypixels(sheet);
    }
}

static void
InsertRow(GtkSheet *sheet, gint row, gint nrows)
{
    gint r, c;

    AddRows(sheet, row, nrows);

    _gtk_sheet_recalc_top_ypixels(sheet);

    if (row <= sheet->maxallocrow)  /* adjust allocated cells */
    {
	GrowSheet(sheet, nrows, 0);  /* append rows at end */

	for (r = sheet->maxallocrow; r >= row + nrows; r--)  /* swap new rows into position */
	{
	    GtkSheetCell **auxdata = sheet->data[r];

	    sheet->data[r] = sheet->data[r - nrows];
	    sheet->data[r - nrows] = auxdata;

	    /* new cells have no data yet to update */

	    GtkSheetCell **pp = sheet->data[r];  /* update row in existing cells */
	    for (c = 0; c <= sheet->maxalloccol; c++, pp++)
	    {
		if (*pp)
		    (*pp)->row = r;
	    }
	}
    }
}

static void
DeleteRow(GtkSheet *sheet, gint position, gint nrows)
{
    gint r, c;

    g_assert(nrows >= 0);
    g_assert(position >= 0);

    nrows = MIN(nrows, sheet->maxrow - position + 1);

    if (nrows <= 0 || position > sheet->maxrow)
	return;

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("DeleteRow: pos %d nrows %d mxr %d mxc %d mxar %d mxac %d ", 
	position, nrows, 
	sheet->maxrow, sheet->maxcol, sheet->maxallocrow, sheet->maxalloccol);
#endif

    for (r = position; r < position + nrows; r++)  /* dispose row data */
    {
	gtk_sheet_row_finalize(&sheet->row[r]);
    }

    for (r = position; r <= sheet->maxrow - nrows; r++)  /* shift rows into position*/
    {
	sheet->row[r] = sheet->row[r + nrows];
    }

    for (r = sheet->maxrow - nrows + 1; r <= sheet->maxrow; r++)  /* clear tail */
    {
	_gtk_sheet_row_init(&sheet->row[r]);
    }

    /* to be done: shrink pointer pool via realloc */

    if (position <= sheet->maxallocrow)
    {
	for (r = position; r <= sheet->maxrow - nrows; r++)  /* shift row data */
	{
	    if (r > sheet->maxallocrow) break;

	    for (c = 0; c <= sheet->maxalloccol; c++)  /* dispose cell data */
	    {
		gtk_sheet_real_cell_clear(sheet, r, c, TRUE);
	    }

	    if (sheet->data[r])  /* dispose row data pointer array */
	    {
		g_free(sheet->data[r]);
		sheet->data[r] = NULL;
	    }

	    if (r + nrows <= sheet->maxallocrow)  /* shift tail down */
	    {
		sheet->data[r] = sheet->data[r + nrows];
		sheet->data[r + nrows] = NULL;

		GtkSheetCell **pp = sheet->data[r];  /* update row in existing cells */
		for (c = 0; c <= sheet->maxalloccol; c++, pp++)
		{
		    if (*pp)
			(*pp)->row = r;
		}
	    }
	}

	for (r = sheet->maxrow - nrows + 1; r <= sheet->maxrow; r++)  /* clear tail */
	{
	    if (r > sheet->maxallocrow) break;

	    for (c = 0; c <= sheet->maxalloccol; c++)  /* dispose cell data */
	    {
		gtk_sheet_real_cell_clear(sheet, r, c, TRUE);
	    }

	    if (sheet->data[r])  /* dispose row data pointer array */
	    {
		g_free(sheet->data[r]);
		sheet->data[r] = NULL;
	    }
	}

	sheet->maxallocrow -= MIN(nrows, sheet->maxallocrow - position + 1);
	sheet->maxallocrow = MIN(sheet->maxallocrow, sheet->maxrow);
    }

    sheet->maxrow -= nrows;

    _gtk_sheet_range_fixup(sheet, &sheet->view);
    _gtk_sheet_range_fixup(sheet, &sheet->range);

    _gtk_sheet_recalc_top_ypixels(sheet);
}

static gint
GrowSheet(GtkSheet *tbl, gint newrows, gint newcols)
{
    gint r, c;
    gint inirow, inicol;

    inirow = tbl->maxallocrow + 1;
    inicol = tbl->maxalloccol + 1;

    tbl->maxalloccol = tbl->maxalloccol + newcols;
    tbl->maxallocrow = tbl->maxallocrow + newrows;

    if (newrows > 0)
    {
	tbl->data = (GtkSheetCell ***)
	g_realloc(tbl->data, (tbl->maxallocrow + 1)*sizeof(GtkSheetCell **)+sizeof(double));

	for (r = inirow; r <= tbl->maxallocrow; r++)
	{
	    tbl->data[r] = (GtkSheetCell **)
		g_malloc((tbl->maxcol + 1)*sizeof(GtkSheetCell *)+sizeof(double));

	    for (c = 0; c < inicol; c++)
	    {
		tbl->data[r][c] = NULL;
	    }
	}

    }

    if (newcols > 0)
    {
	for (r = 0; r <= tbl->maxallocrow; r++)
	{
	    tbl->data[r] = (GtkSheetCell **)
		g_realloc(tbl->data[r], (tbl->maxalloccol + 1)*sizeof(GtkSheetCell *)+sizeof(double));
	    for (c = inicol; c <= tbl->maxalloccol; c++)
	    {
		tbl->data[r][c] = NULL;
	    }
	}
    }

    return (0);
}

static void
CheckBounds(GtkSheet *sheet, gint row, gint col)
{
    gint newrows = 0, newcols = 0;

    if (col > sheet->maxalloccol)
	newcols = col - sheet->maxalloccol;
    if (row > sheet->maxallocrow)
	newrows = row - sheet->maxallocrow;

    if (newrows > 0 || newcols > 0)
	GrowSheet(sheet, newrows, newcols);
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

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

    CheckBounds(sheet, row, col);

    cell = &sheet->data[row][col];

    if (!(*cell))
	(*cell) = gtk_sheet_cell_new();

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
 * Returns:  (transfer none) TRUE means that the cell is currently allocated.
 */
GtkSheetChild *
gtk_sheet_put(GtkSheet *sheet, GtkWidget *child, gint x, gint y)
{
    GtkRequisition child_requisition;
    GtkSheetChild *child_info;

    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);
    g_return_val_if_fail(child != NULL, NULL);
    g_return_val_if_fail(gtk_widget_get_parent(child) == NULL, NULL);

#if GTK_SHEET_DEBUG_CHILDREN > 0
    g_debug("gtk_sheet_put: %p %s child %p", 
	sheet, gtk_widget_get_name(sheet), child);
#endif

    child_info = g_new(GtkSheetChild, 1);
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
    g_object_ref(child);
    gtk_widget_set_parent(child, GTK_WIDGET(sheet));

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

    return (child_info);
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
gtk_sheet_attach_floating(GtkSheet *sheet,
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
gtk_sheet_attach_default(GtkSheet *sheet,
    GtkWidget *widget,
    gint row, gint col)
{
    if (row < 0 || col < 0)
    {
	gtk_sheet_button_attach(sheet, widget, row, col);
	return;
    }

    gtk_sheet_attach(sheet, widget, row, col, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
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
gtk_sheet_attach(GtkSheet *sheet,
    GtkWidget *widget,
    gint row, gint col,
    gint xoptions,
    gint yoptions,
    gint xpadding,
    gint ypadding)
{
    GdkRectangle area;
    GtkSheetChild *child = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));
    g_return_if_fail(widget != NULL);

    if (row < 0 || col < 0)
    {
	gtk_sheet_button_attach(sheet, widget, row, col);
	return;
    }

#if GTK_SHEET_DEBUG_CHILDREN > 0
    g_debug("gtk_sheet_attach: %p %s widget %p", 
	sheet, gtk_widget_get_name(sheet), widget);
#endif

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
    g_object_ref(child->widget);
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
gtk_sheet_button_attach(GtkSheet *sheet,
    GtkWidget *widget,
    gint row, gint col)
{
    GtkSheetButton *button;
    GtkSheetChild *child;
    GtkRequisition button_requisition;

    if (row >= 0 && col >= 0)
	return;
    if (row < 0 && col < 0)
	return;

#if GTK_SHEET_DEBUG_CHILDREN > 0
    g_debug("gtk_sheet_button_attach: called");
#endif

    child = g_new(GtkSheetChild, 1);
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
	button = &COLPTR(sheet, col)->button;
	button->child = child;
    }
    else
    {
	button = &sheet->row[row].button;
	button->child = child;
    }

    sheet->children = g_list_append(sheet->children, child);
    g_object_ref(child);

    _gtk_sheet_button_size_request(sheet, button, &button_requisition);

    if (row == -1)
    {
	if (button_requisition.height > sheet->column_title_area.height)
	    sheet->column_title_area.height = button_requisition.height;
	if (button_requisition.width > COLPTR(sheet, col)->width)
	    COLPTR(sheet, col)->width = button_requisition.width;
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

    if (row == -1)
	_gtk_sheet_column_buttons_size_allocate(sheet);
    if (col == -1)
	size_allocate_row_title_buttons(sheet);
}

static void
label_size_request(GtkSheet *sheet, gchar *label, GtkRequisition *req)
{
    gchar *words;
    gchar word[1000];
    gint n = 0;
    gint row_height = _gtk_sheet_row_default_height(GTK_WIDGET(sheet)) - 2 * CELLOFFSET + 2;

    req->height = 0;
    req->width = 0;
    words = label;

    while (words && *words != '\0')
    {
	if (*words == '\n' || *(words + 1) == '\0')
	{
	    guint text_width, text_height;

	    req->height += row_height;

	    word[n] = '\0';

	    _get_string_extent(sheet, NULL,
		gtk_widget_get_style(GTK_WIDGET(sheet))->font_desc, 
		word, &text_width, &text_height);
	    req->width = MAX(req->width, text_width);
	    n = 0;
	}
	else
	{
	    word[n++] = *words;
	}
	words++;
    }

    if (n > 0)
	req->height -= 2;
}

/**
 * _gtk_sheet_button_size_request:
 * @sheet:  the #GtkSheet
 * @button: the #GtkSheetButton requested
 * @button_requisition: the requisition
 *  
 * size request handler for all sheet buttons
 */
void
_gtk_sheet_button_size_request(GtkSheet *sheet,
    GtkSheetButton *button,
    GtkRequisition *button_requisition)
{
    GtkRequisition requisition;
    GtkRequisition label_requisition;

    if (gtk_sheet_autoresize(sheet) && button->label && button->label[0])
    {
	label_size_request(sheet, button->label, &label_requisition);
	label_requisition.width += 2 * CELLOFFSET;
	label_requisition.height += 2 * CELLOFFSET;
    }
    else
    {
	label_requisition.height = _gtk_sheet_row_default_height(GTK_WIDGET(sheet));
	label_requisition.width = GTK_SHEET_COLUMN_MIN_WIDTH;
    }

    if (button->child)
    {
	gtk_widget_size_request(button->child->widget, &requisition);
	requisition.width += 2 * button->child->xpadding;
	requisition.height += 2 * button->child->ypadding;
	requisition.width += 2 * gtk_widget_get_style(sheet->button)->xthickness;
	requisition.height += 2 * gtk_widget_get_style(sheet->button)->ythickness;
    }
    else
    {
	requisition.height = _gtk_sheet_row_default_height(GTK_WIDGET(sheet));
	requisition.width = GTK_SHEET_COLUMN_MIN_WIDTH;
    }

    *button_requisition = requisition;
    button_requisition->width = MAX(requisition.width, label_requisition.width);
    button_requisition->height = MAX(requisition.height, label_requisition.height);

}

static void
gtk_sheet_row_size_request(GtkSheet *sheet,
    gint row,
    guint *requisition)
{
    GtkRequisition button_requisition;
    GList *children;

    _gtk_sheet_button_size_request(sheet, &sheet->row[row].button, &button_requisition);

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
	    child->row = _gtk_sheet_row_from_ypixel(sheet, y);
	    child->col = _gtk_sheet_column_from_xpixel(sheet, x);
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

    /* PR#99118 - we cannot reposition all children while scrolling because
       with many offside rows it will consume lots of CPU and incredibly
       slow down scrolling.
     
       If we do not reposition all childs, we have to hide them whenn going
       off screen, in order not to stray around. We're using unmap/map here
       in order to leave hide/show available for the application
       */

    if (child->attached_to_cell)
    {
	if ((child->row < MIN_VIEW_ROW(sheet) || child->row > MAX_VIEW_ROW(sheet))
	    || (child->col < MIN_VIEW_COLUMN(sheet) || child->col > MAX_VIEW_COLUMN(sheet))
	    )
	{
	    gtk_widget_unmap(child->widget);
	    return;
	}
	if (gtk_widget_get_realized(child->widget) 
	    && !gtk_widget_get_mapped(child->widget))
	{
	    gtk_widget_map(child->widget); 
	}
    }

    gtk_widget_get_child_requisition(child->widget, &child_requisition);

    if (sheet->column_titles_visible)
	yoffset = sheet->column_title_area.height;

    if (sheet->row_titles_visible)
	xoffset = sheet->row_title_area.width;

    if (child->attached_to_cell)
    {
	gtk_sheet_get_cell_area(sheet, child->row, child->col, &area);

	child->x = area.x + child->xpadding;
	child->y = area.y + child->ypadding;

	if (!child->floating)
	{
	    if (child_requisition.width + 2 * child->xpadding <= COLPTR(sheet, child->col)->width)
	    {
		if (child->xfill)
		{
		    child_requisition.width = child_allocation.width = COLPTR(sheet, child->col)->width - 2 * child->xpadding;
		}
		else
		{
		    if (child->xexpand)
		    {
			child->x = area.x + COLPTR(sheet, child->col)->width / 2 -
			    child_requisition.width / 2;
		    }
		    child_allocation.width = child_requisition.width;
		}
	    }
	    else
	    {
		if (!child->xshrink)
		{
#if GTK_SHEET_DEBUG_SIZE > 0
		    g_debug("gtk_sheet_position_child[%d]: set width %d",
			child->col, child_requisition.width + 2 * child->xpadding);
#endif
		    gtk_sheet_set_column_width(sheet,
			child->col, child_requisition.width + 2 * child->xpadding);
		}
		child_allocation.width = COLPTR(sheet, child->col)->width - 2 * child->xpadding;
	    }

	    if (child_requisition.height + 2 * child->ypadding <= sheet->row[child->row].height)
	    {
		if (child->yfill)
		{
		    child_requisition.height = child_allocation.height = sheet->row[child->row].height - 2 * child->ypadding;
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
		child_allocation.height = sheet->row[child->row].height - 2 * child->ypadding;
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
    else  /* not attached_to_cell */
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
gtk_sheet_forall_handler(GtkContainer *container,
    gboolean      include_internals,
    GtkCallback   callback,
    gpointer      callback_data)
{
    GtkSheet *sheet;
    GtkSheetChild *child;
    GList *children;

    g_return_if_fail(GTK_IS_SHEET(container));
    g_return_if_fail(callback != NULL);

    sheet = GTK_SHEET(container);
    children = sheet->children;

#if GTK_SHEET_DEBUG_CHILDREN > 1
    g_debug("gtk_sheet_forall_handler: Sheet <%s>", 
	gtk_widget_get_name(GTK_WIDGET(sheet)));
#endif

    while (children)
    {
	child = children->data;
	children = children->next;

#if GTK_SHEET_DEBUG_CHILDREN > 1
	g_debug("gtk_sheet_forall_handler: L1 %p", child->widget);
#endif

	if (G_IS_OBJECT(child->widget) 
	    && GTK_IS_WIDGET(child->widget)
	    )
	{
#if GTK_SHEET_DEBUG_CHILDREN > 1
	    g_debug("gtk_sheet_forall_handler: L2 %p", child->widget);
#endif
	    (*callback)(child->widget, callback_data); 
	}
    }

#if GTK_SHEET_DEBUG_CHILDREN > 1
    g_debug("gtk_sheet_forall_handler: B1 %p %d", 
	sheet->button, GTK_IS_WIDGET(sheet->button));
#endif

    if (sheet->button 
	&& G_IS_OBJECT(sheet->button) 
	&& GTK_IS_WIDGET(sheet->button)
	)
    {
#if GTK_SHEET_DEBUG_CHILDREN > 1
	g_debug("gtk_sheet_forall_handler: B2 %p", sheet->button);
#endif
	g_object_ref(sheet->button);
	(*callback)(sheet->button, callback_data);
	g_object_unref(sheet->button);
    }

#if GTK_SHEET_DEBUG_CHILDREN > 1
    g_debug("gtk_sheet_forall_handler: C1 %p %d", 
	sheet->sheet_entry, GTK_IS_WIDGET(sheet->sheet_entry));
#endif

    if (sheet->sheet_entry 
	&& G_IS_OBJECT(sheet->sheet_entry) 
	&& GTK_IS_WIDGET(sheet->sheet_entry)
	)
    {
#if GTK_SHEET_DEBUG_CHILDREN > 1
	g_debug("gtk_sheet_forall_handler: C2 %p IsObject %d IsWidget %d", 
	    sheet->sheet_entry,
	    G_IS_OBJECT(sheet->sheet_entry),
	    GTK_IS_WIDGET(sheet->sheet_entry));
#endif
	g_object_ref(sheet->sheet_entry);
	(*callback)(sheet->sheet_entry, callback_data);
	g_object_unref(sheet->sheet_entry);
    }
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

	if (child->col != -1 && child->row != -1)
	    gtk_sheet_position_child(sheet, child);

	if (child->row == -1)
	{
	    if (child->col < MIN_VIEW_COLUMN(sheet) ||
		child->col > MAX_VIEW_COLUMN(sheet))
		_gtk_sheet_child_hide(child);
	    else
		_gtk_sheet_child_show(child);
	}
	if (child->col == -1)
	{
	    if (child->row < MIN_VIEW_ROW(sheet) ||
		child->row > MAX_VIEW_ROW(sheet))
		_gtk_sheet_child_hide(child);
	    else
		_gtk_sheet_child_show(child);
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
gtk_sheet_remove_handler(GtkContainer *container, GtkWidget *widget)
{
    GtkSheet *sheet;
    GList *children;
    GtkSheetChild *child = NULL;

    g_return_if_fail(container != NULL);
    g_return_if_fail(GTK_IS_SHEET(container));

    sheet = GTK_SHEET(container);

    children = sheet->children;

    while (children)
    {
	child = (GtkSheetChild *)children->data;

	if (child->widget == widget)
	    break;

	children = children->next;
    }

    if (children)
    {
	if (child->row == -1)
	    sheet->row[child->col].button.child = NULL;

	if (child->col == -1)
	    COLPTR(sheet, child->row)->button.child = NULL;

#if GTK_SHEET_DEBUG_CHILDREN > 0
	g_debug("gtk_sheet_remove_handler: %p %s widget %p", 
	    sheet, gtk_widget_get_name(sheet), widget);
#endif

	gtk_widget_unparent(widget);
	if (G_IS_OBJECT(child->widget))
	    g_object_unref(child->widget); 
	child->widget = NULL;

	sheet->children = g_list_remove_link(sheet->children, children);
	g_list_free_1(children);
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
const GtkSheetChild *
gtk_sheet_get_child_at(GtkSheet *sheet, gint row, gint col)
{
    GList *children;

    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    children = sheet->children;

    while (children)
    {
	const GtkSheetChild *child = (GtkSheetChild *)children->data;

	if (child->attached_to_cell)
	{
	    if (child->row == row && child->col == col)
		return (child);
	}

	children = children->next;
    }
    return (NULL);
}

/**
 * _gtk_sheet_child_hide:
 * @child:  the child
 * 
 * gtk_widget_hide(child)
 */
void
_gtk_sheet_child_hide(GtkSheetChild *child)
{
    g_return_if_fail(child != NULL);
    gtk_widget_hide(child->widget);
}

/**
 * _gtk_sheet_child_show:
 * @child:  the child
 * 
 * gtk_widget_show(child)
 */
void
_gtk_sheet_child_show(GtkSheetChild *child)
{
    g_return_if_fail(child != NULL);
    gtk_widget_show(child->widget);
}
