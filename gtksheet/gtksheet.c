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
 * #GtkSheet is a matrix widget for GTK+. It consists of a
 * scrollable grid of cells where you can put text or other
 * #GtkWidget's in. Cells are organized in rows (#GtkSheetRow) 
 * and columns (#GtkSheetColumn). Cell contents can be edited 
 * interactively through a configurable item entry
 * (#GtkDataEntry, #GtkEntry, #GtkDataTextView, #GtkTextView,
 * #GtkItemEntry). A #GtkSheet is also a container subclass,
 * allowing you to display buttons, curves, pixmaps and any
 * other widget in it. You can also set many attributes as:
 * border, text justification, and more. The testgtksheet
 * program shows how easy is to create a spreadsheet-like GUI
 * using this widget set.
 * 
 * # CSS nodes
 *
 * |[<!-- language="plain" --> 
 * sheet[.view][.cell[:selected]]
 * ├── <entry>
 * │    ╰── selection
 * ╰── <button>[.top][.left]
 * ]|
 *
 * GtkSheet has a main CSS node with name "sheet". It has
 * subnodes for the sheet item entry named "entry" and row/cell
 * headings named "button". The GtkSheet generally has
 * the CSS class ".view" attached. The inner cell area has class
 * ".cell" attached. Sheet cells can have optional ":selected"
 * state.
 * 
 * #GtkSheetColumn titles are Gtk widgets, typically containing
 * a #GtkToggleButton with all it's decendants. Column titles
 * can be styled via "sheet button.top" selector.
 * 
 * Row title buttons are just frames, being styled via "sheet
 * .button.left" selector, optionally having ":active" state
 * when selected.
 * 
 * See embedded "gtksheet.css" or "testgtksheet.css" for CSS
 * styling examples. The #GtkSheet will load it's embedded
 * "gtksheet.css" with application priority upon initialization.
 * The testgtksheet application will try to load the file
 * "testgtksheet.css" upon startup, which must be available in
 * the current directory.
 *
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gobject/gvaluecollector.h>
#include <pango/pango.h>

#define __GTKSHEET_H_INSIDE__

#include "gtksheet-compat.h"
#include "gtkdataentry.h"
#include "gtkdatatextview.h"
#include "gtksheet.h"
#include "pango-markup.h"
#include "gtkdataformat.h"
#include "gtksheet-marshal.h"
#include "gtksheettypebuiltins.h"

#undef GTK_SHEET_DEBUG

#ifdef DEBUG
#   undef GTK_SHEET_DEBUG
#define GTK_SHEET_DEBUG  1  /* define to activate debug output */
#endif

#ifdef GTK_SHEET_DEBUG
#   define GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS  1

#   define GTK_SHEET_DEBUG_ADJUSTMENT  0
#   define GTK_SHEET_DEBUG_ALLOCATION  0    /* 0,1,2 */
#   define GTK_SHEET_DEBUG_BUILDER   0
#   define GTK_SHEET_DEBUG_CELL_ACTIVATION  0
#   define GTK_SHEET_DEBUG_CHILDREN  0
#   define GTK_SHEET_DEBUG_CAIRO  0
#   define GTK_SHEET_DEBUG_CLICK  0
#   define GTK_SHEET_DEBUG_COLORS  0
#   define GTK_SHEET_DEBUG_DRAW  0
#   define GTK_SHEET_DEBUG_DRAW_BACKGROUND  0
#   define GTK_SHEET_DEBUG_DRAW_BUTTON  0
#   define GTK_SHEET_DEBUG_DRAW_INVALIDATE  0
#   define GTK_SHEET_DEBUG_DRAW_LABEL  0
#   define GTK_SHEET_DEBUG_ENTER_PRESSED   0
#   define GTK_SHEET_DEBUG_ENTRY   0
#   define GTK_SHEET_DEBUG_EXPOSE   0
#   define GTK_SHEET_DEBUG_FINALIZE  0
#   define GTK_SHEET_DEBUG_FONT_METRICS  0
#   define GTK_SHEET_DEBUG_FREEZE   0
#   define GTK_SHEET_DEBUG_KEYPRESS   0
#   define GTK_SHEET_DEBUG_MOUSE  0    /* 0,1 */
#   define GTK_SHEET_DEBUG_MOVE  0
#   define GTK_SHEET_DEBUG_MOTION  0
#   define GTK_SHEET_DEBUG_PIXEL_INFO  0
#   define GTK_SHEET_DEBUG_PROPERTIES  0
#   define GTK_SHEET_DEBUG_REALIZE  0
#   define GTK_SHEET_DEBUG_SELECTION  0
#   define GTK_SHEET_DEBUG_SIGNALS   0
#   define GTK_SHEET_DEBUG_SIZE  0
#   define GTK_SHEET_DEBUG_SCROLL  0
#   define GTK_SHEET_DEBUG_SET_CELL_TIMER  0
#   define GTK_SHEET_DEBUG_SET_CELL_TEXT  0
#   define GTK_SHEET_DEBUG_MARKUP  0

#   define GTK_SHEET_ENABLE_DEBUG_MACROS
#   undef GTK_SHEET_ENABLE_DEBUG_MACROS
#endif

#include "gtksheetdebug.h"

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
    PROP_GTK_SHEET_HADJUSTMENT,  /* gtk_sheet_set_hadjustment() */
    PROP_GTK_SHEET_VADJUSTMENT,  /* gtk_sheet_set_vadjustment() */
    PROP_GTK_SHEET_HSCROLL_POLICY,  /* GtkScrollable interface */
    PROP_GTK_SHEET_VSCROLL_POLICY,  /* GtkScrollable interface */
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

#define GTK_SHEET_FLAGS(sheet) \
    (GTK_SHEET (sheet)->flags)

#define GTK_SHEET_SET_FLAGS(sheet,flag) \
    (GTK_SHEET_FLAGS (sheet) |= (flag))

#define GTK_SHEET_UNSET_FLAGS(sheet,flag) \
    (GTK_SHEET_FLAGS (sheet) &= ~(flag))

#define GTK_SHEET_IS_FROZEN(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IS_FROZEN)

#define GTK_SHEET_IN_XDRAG(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_XDRAG)

#define GTK_SHEET_IN_YDRAG(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_YDRAG)

#define GTK_SHEET_IN_DRAG(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_DRAG)

#define GTK_SHEET_IN_SELECTION(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_SELECTION)

#define GTK_SHEET_IN_RESIZE(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_RESIZE)

#define GTK_SHEET_IN_CLIP(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_CLIP)

#define GTK_SHEET_REDRAW_PENDING(sheet) \
    (GTK_SHEET_FLAGS (sheet) & GTK_SHEET_IN_REDRAW_PENDING)

/* data access macros - no frontend update! */

#define COLPTR(sheet, colidx) (sheet->column[colidx])
#define ROWPTR(sheet, rowidx) (&sheet->row[rowidx])

#define MIN_VIEW_ROW(sheet) \
    (sheet->view.row0)

/* beware: MAX_VISIBLE_ROW() can be maxrow+1 */
#define MAX_VIEW_ROW(sheet) \
    (sheet->view.rowi)

#define MIN_VIEW_COLUMN(sheet) \
    (sheet->view.col0)

/* beware: MAX_VISIBLE_COLUMN() can be maxcol+1 */
#define MAX_VIEW_COLUMN(sheet) \
    (sheet->view.coli)

/* widget flag tests and settings */

#define GTK_SHEET_ROW_IS_VISIBLE(rowptr) \
    ((rowptr)->is_visible)

#define GTK_SHEET_ROW_SET_VISIBLE(rowptr, value) \
    ((rowptr)->is_visible = (value))

#define GTK_SHEET_ROW_IS_SENSITIVE(rowptr) \
    ((rowptr)->is_sensitive)

#define GTK_SHEET_ROW_SET_SENSITIVE(rowptr, value) \
    ((rowptr)->is_sensitive = (value))

#define GTK_SHEET_ROW_CAN_FOCUS(rowptr) \
    GTK_SHEET_ROW_IS_SENSITIVE(rowptr)

/* cell is in range or on range border */

#define CELL_IN_RANGE(row, col, range) \
    (range.row0 <= row && row <= range.rowi  \
    && range.col0 <= col && col <= range.coli)

/* cell is on range border */
#define CELL_ON_RANGE_BORDER(row, col, range) \
    (row == range.row0 || row == range.rowi \
    || col == range.col0 || col == range.coli)

/* some constants */

/* unrealized maximum width */
#define COLUMN_UNREALIZED_MAX_WIDTH 512

/* maximized: free space left for other columns */
#define COLUMN_REMNANT_PIXELS  48

/* maximized: divisor for window width */
#define COLUMN_WIN_WIDTH_DIVISOR  3

/* unrealized maximum height */
#define ROW_UNREALIZED_MAX_HEIGHT 128

/* maximized: free space left for other rows */
#define ROW_REMNANT_PIXELS  32

/* maximized: divisor for window height */
#define ROW_WIN_HEIGHT_DIVISOR  3

/* default entry type for GTK_SHEET_ENTRY_TYPE_DEFAULT, a GType */
#define DEFAULT_ENTRY_TYPE GTK_TYPE_DATA_ENTRY

#define COLUMN_MAX_WIDTH(sheet) \
    (sheet->sheet_window_width < COLUMN_REMNANT_PIXELS ? \
    COLUMN_UNREALIZED_MAX_WIDTH : \
    sheet->sheet_window_width / COLUMN_WIN_WIDTH_DIVISOR )

#if 0
#   define ROW_MAX_HEIGHT(sheet) \
    (sheet->sheet_window_height < ROW_REMNANT_PIXELS ? \
    ROW_UNREALIZED_MAX_HEIGHT : \
    sheet->sheet_window_height - ROW_REMNANT_PIXELS )
#else
#   define ROW_MAX_HEIGHT(sheet) \
    (sheet->sheet_window_height < ROW_REMNANT_PIXELS ? \
    ROW_UNREALIZED_MAX_HEIGHT : \
    sheet->sheet_window_height / ROW_WIN_HEIGHT_DIVISOR )
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

#define SET_ACTIVE_CELL(r, c) \
    { \
	/*g_debug("%s(%d): SET_ACTIVE_CELL(%d, %d)", \
            __FUNCTION__, __LINE__, (r), (c));*/ \
	sheet->active_cell.row = (r); \
        sheet->active_cell.col = (c); \
    }

#define SET_SELECTION_PIN(r, c) \
    { \
        /*g_debug("%s(%d): SET_SELECTION_PIN(%d, %d)", \
            __FUNCTION__, __LINE__, (r), (c));*/ \
        sheet->selection_pin.row = (r); \
        sheet->selection_pin.col = (c); \
    }

#define SET_SELECTION_CURSOR(r, c) \
    { \
        /*g_debug("%s(%d): SET_SELECTION_CURSOR(%d, %d)", \
            __FUNCTION__, __LINE__, (r), (c));*/ \
        sheet->selection_cursor.row = (r); \
        sheet->selection_cursor.col = (c); \
    }

const guint gtksheet_major_version = GTKSHEET_MAJOR_VERSION;
const guint gtksheet_minor_version = GTKSHEET_MINOR_VERSION;
const guint gtksheet_micro_version = GTKSHEET_MICRO_VERSION;

gchar * 
gtksheet_check_version (
    guint required_major, guint required_minor, guint required_micro)
{
  if (required_major > GTKSHEET_MAJOR_VERSION)
    return "GtkExtra version too old (major mismatch)";
  if (required_major < GTKSHEET_MAJOR_VERSION)
    return "GtkExtra version too new (major mismatch)";
  if (required_minor > GTKSHEET_MINOR_VERSION)
    return "GtkExtra version too old (minor mismatch)";
  if (required_minor < GTKSHEET_MINOR_VERSION)
    return "GtkExtra version too new (minor mismatch)";
  if (required_micro > GTKSHEET_MICRO_VERSION)
    return "GtkExtra version too old (micro mismatch)";
  return NULL;
}

/*
void
_gtksheet_signal_test( 
    GObject *object, guint signal_id, gint arg1, gint arg2,
    gboolean *default_ret)
{
  gboolean result;
  GValue ret = { 0, };
  GValue instance_and_param[3] = { { 0, }, {0, }, {0, } };

  g_value_init(instance_and_param + 0, G_OBJECT_TYPE(object));
  g_value_set_instance(instance_and_param + 0, G_OBJECT(object));

  g_value_init(instance_and_param + 1, G_TYPE_INT);
  g_value_set_int(instance_and_param + 1, arg1);

  g_value_init(instance_and_param + 2, G_TYPE_INT);
  g_value_set_int(instance_and_param + 2, arg2);

  g_value_init(&ret, G_TYPE_BOOLEANEAN);
  g_value_set_boolean(&ret, *default_ret);

  g_signal_emitv(instance_and_param, signal_id, 0, &ret);
  *default_ret = g_value_get_boolean(&ret);

  g_value_unset(instance_and_param + 0);
  g_value_unset(instance_and_param + 1);
  g_value_unset(instance_and_param + 2);
}
*/

void
_gtksheet_signal_emit(GObject *object, guint signal_id, ...)
{
  gboolean *result;
  GValue ret = { 0, };
  GValue instance_and_params [10] = { {0, }, };
  va_list var_args;
  GSignalQuery query;
  gchar *error;
  int i;

  va_start (var_args, signal_id);

  g_value_init(instance_and_params + 0, G_OBJECT_TYPE(object));
  g_value_set_instance (instance_and_params + 0, G_OBJECT(object));

  g_signal_query(signal_id, &query);

  for (i = 0; i < query.n_params; i++)
    {
      gboolean static_scope = query.param_types[i]&~G_SIGNAL_TYPE_STATIC_SCOPE;
      g_value_init(instance_and_params + i + 1, query.param_types[i]);


      G_VALUE_COLLECT (instance_and_params + i + 1,
                       var_args,
                       static_scope ? G_VALUE_NOCOPY_CONTENTS : 0,
                       &error);

      if (error)
        {
          g_warning ("%s: %s", G_STRLOC, error);
          g_free (error);
          while (i-- > 0)
            g_value_unset (instance_and_params + i);

          va_end (var_args);
          return;
        }
  

    }

  g_value_init(&ret, query.return_type);
  result = va_arg(var_args,gboolean *);
  g_value_set_boolean(&ret, *result);    
  g_signal_emitv(instance_and_params, signal_id, 0, &ret);
  *result = g_value_get_boolean(&ret);    
  g_value_unset (&ret);

  for (i = 0; i < query.n_params; i++)
    g_value_unset (instance_and_params + 1 + i);
  g_value_unset (instance_and_params + 0);

  va_end (var_args);
}

/* defaults */

#define GTK_SHEET_ROW_DEFAULT_HEIGHT 24

#define GTK_SHEET_DEFAULT_FONT_ASCENT  12
#define GTK_SHEET_DEFAULT_FONT_DESCENT  12

#define GTK_SHEET_DEFAULT_BG_COLOR      "lightgray"
#define GTK_SHEET_DEFAULT_GRID_COLOR  "gray"
#define GTK_SHEET_DEFAULT_TM_COLOR  "red"   /* tooltip marker */

/* enable focus hunt when no active cell is set when sheet is shown */
#define GTK_SHEET_ENABLE_FOCUS_ON_SHOW  0

/* size of tooltip marker, pixels */
#define GTK_SHEET_DEFAULT_TM_SIZE  4

/* number of rows to stay visible with PageUp/Dn */
#define GTK_SHEET_PAGE_OVERLAP 1  

#define CELL_SPACING 1
#define DRAG_WIDTH 6
#define CLICK_CELL_MOTION_TOLERANCE 5  /* pixel */
#define TIMEOUT_SCROLL 20
#define TIMEOUT_FLASH 200
#define TIME_INTERVAL 8
#define MINROWS 0
#define MINCOLS 0
#define MAXLENGTH 30

#define CELLOFFSET 4  /* inner border from grid to text */

#define CELLOFFSET_NF  (CELLOFFSET)    /* no frame, GtkTextView */
#define CELLOFFSET_WF  (CELLOFFSET-2)  /* w/frame, GtkEntry */

#define DEFAULT_SELECTION_BORDER_WIDTH  3
#define DEFAULT_CORNER_EXTENT  2

static gdouble selection_border_width
= DEFAULT_SELECTION_BORDER_WIDTH;
static gdouble selection_border_offset = 0;  /* + to outside, - to inner */

/* beware: keep corner within border! */
static gdouble selection_corner_size
= DEFAULT_SELECTION_BORDER_WIDTH + DEFAULT_CORNER_EXTENT;
static gdouble selection_corner_offset = -DEFAULT_CORNER_EXTENT;

static gdouble selection_bb_offset = 1;  /* border/background*/

static GdkRGBA color_black;
static GdkRGBA color_white;

#ifdef GTK_SHEET_DEBUG
#   define GTK_SHEET_DEBUG_COLOR  "green"

static GdkRGBA debug_color;

#if GTK_SHEET_DEBUG_EXPOSE > 0
static void _debug_cairo_clip_extent(gchar *where, cairo_t *cr)
{
    double x1, y1, x2, y2;
    cairo_clip_extents (cr, &x1, &y1, &x2, &y2);
    g_debug(
        "%s: _debug_cairo_clip_extent: x1 %0.1f y1 %0.1f x2 %0.1f y2 %0.1f",
        where, x1, y1, x2, y2);
}
#endif

#ifdef GTK_SHEET_DEBUG
static void _debug_color_rect(
    gchar *where, cairo_t *cr, double x, double y, double w, double h)
{
    g_debug(
        "%s: _debug_color_rect: x %0.1f y %0.1f w %0.1f h %0.1f",
        where, x, y, w, h);

    cairo_save(cr);  // debug color

    cairo_set_line_width(cr, 1.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
    gdk_cairo_set_source_rgba(cr, &debug_color);
    cairo_rectangle(cr, x, y, w, h);
    cairo_stroke(cr);

    cairo_restore(cr);  // debug color
}
#endif

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

static void g_debug_style_context_list_classes(
    gchar *hint, GtkStyleContext *context)
{
    GList *l = gtk_style_context_list_classes (context);
    while (l)
    {
	g_debug("%s: style_context class <%s>", hint, (gchar *) l->data);
	l = l->next;
    }
    g_list_free(l);
}

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
    PangoFontDescription *font_desc = NULL;
    GtkStyleContext *style_context = gtk_widget_get_style_context(widget);

    GtkStateFlags flags = gtk_widget_get_state_flags(widget);

    gtk_style_context_get(style_context, 
        flags,
        GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    if (!font_desc)
	return (GTK_SHEET_ROW_DEFAULT_HEIGHT);

    PangoContext *context = gtk_widget_get_pango_context(widget);

    PangoFontMetrics *metrics = pango_context_get_metrics(context,
	font_desc, pango_context_get_language(context));
    guint val = pango_font_metrics_get_descent(metrics) +
	pango_font_metrics_get_ascent(metrics);

    pango_font_metrics_unref(metrics);
    pango_font_description_free(font_desc);

    return (PANGO_PIXELS(val) + 2 * CELLOFFSET);
}

static inline guint
_default_font_ascent(GtkWidget *widget)
{
    PangoFontDescription *font_desc = NULL;
    GtkStyleContext *style_context = gtk_widget_get_style_context(widget);

    gtk_style_context_get (style_context, GTK_STATE_FLAG_NORMAL,
                       GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    if (!font_desc)
	return (GTK_SHEET_DEFAULT_FONT_ASCENT);

    PangoContext *context = gtk_widget_get_pango_context(widget);

    PangoFontMetrics *metrics = pango_context_get_metrics(context,
	font_desc, pango_context_get_language(context));
    guint val = pango_font_metrics_get_ascent(metrics);

    pango_font_metrics_unref(metrics);
    pango_font_description_free(font_desc);

    return (PANGO_PIXELS(val));
}

/* _find_invisible_char:
   derived from gtkentry.c - find_invisible_char() */

static gunichar
_find_invisible_char (GtkWidget *widget)
{
  gunichar invisible_chars [] = {
    0,
    0x25cf, /* BLACK CIRCLE */
    0x2022, /* BULLET */
    0x2731, /* HEAVY ASTERISK */
    0x273a  /* SIXTEEN POINTED ASTERISK */
  };

  gtk_widget_style_get(widget,
      "invisible-char", &invisible_chars[0],
      NULL);

  PangoLayout *layout = gtk_widget_create_pango_layout(
      widget, NULL);

  PangoAttrList *attr_list = pango_attr_list_new();
  pango_attr_list_insert(
      attr_list, pango_attr_fallback_new (FALSE));

  pango_layout_set_attributes (layout, attr_list);
  pango_attr_list_unref (attr_list);

  gint i;
  for (i = (invisible_chars[0] != 0 ? 0 : 1);
        i < G_N_ELEMENTS (invisible_chars); i++)
  {
      gchar text[7] = { 0, };

      gint len = g_unichar_to_utf8(invisible_chars[i], text);
      pango_layout_set_text(layout, text, len);

      gint count = pango_layout_get_unknown_glyphs_count(layout);

      if (count == 0)
      {
          g_object_unref(layout);
          return invisible_chars[i];
      }
  }

  g_object_unref(layout);

  return '*';
}

static gchar *_make_secret_text(GtkSheet *sheet, const gchar *text)
{
    gunichar invisible_char = _find_invisible_char(
        GTK_WIDGET(sheet));
    gchar char_str[7];
    gint char_len = g_unichar_to_utf8(invisible_char, char_str);

    guint length = g_utf8_strlen(text, -1);
    GString *str = g_string_sized_new(length * 2);

    /*
     * Add hidden characters for each character in the text
     * buffer. If there is a password hint, then keep that
     * character visible.
     */

    gint i;
    for (i = 0; i < length; ++i)
    {
        g_string_append_len(str, char_str, char_len);
    }

    return g_string_free(str, FALSE);
}

static void _get_string_extent(
    GtkSheet *sheet, GtkSheetColumn *colptr,
    PangoFontDescription *font_desc, 
    const gchar *text, gboolean is_markup,
    guint *width, guint *height)
{
    PangoRectangle extent;
    PangoLayout *layout;

    if (!text) {
        *width = *height = 0;
        return;
    }

    layout = gtk_widget_create_pango_layout(
        GTK_WIDGET(sheet), NULL);

    if (colptr && colptr->is_secret) 
    {
        text = _make_secret_text(sheet, text);
    }

    if (is_markup)
        pango_layout_set_markup(layout, text, -1);
    else
        pango_layout_set_text(layout, text, -1);

    pango_layout_set_font_description(layout, font_desc);

    if (colptr && !gtk_sheet_autoresize_columns(sheet))
    {
        switch(colptr->wrap_mode)
        {
            case GTK_WRAP_NONE: 
                break;

            case GTK_WRAP_CHAR:
		pango_layout_set_width(
                    layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(
                    layout, PANGO_WRAP_CHAR);
		break;

	    case GTK_WRAP_WORD:
		pango_layout_set_width(
                    layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(
                    layout, PANGO_WRAP_WORD);
		break;

	    case GTK_WRAP_WORD_CHAR:
		pango_layout_set_width(
                    layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(
                    layout, PANGO_WRAP_WORD_CHAR);
		break;
	}
    }

    pango_layout_get_pixel_extents(layout, NULL, &extent);

#if GTK_SHEET_DEBUG_FONT_METRICS > 0
    {
	PangoContext *context = gtk_widget_get_pango_context(
            GTK_WIDGET(sheet));
	PangoFontMetrics *metrics = pango_context_get_metrics(
	    context, font_desc, 
            pango_context_get_language(context));

	gint ascent = 
            pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
	gint descent =
            pango_font_metrics_get_descent(metrics) / PANGO_SCALE;
	gint spacing = 
            pango_layout_get_spacing(layout) / PANGO_SCALE;

	pango_font_metrics_unref(metrics);

	g_debug("_get_string_extent(%s): ext (%d, %d, %d, %d) asc %d desc %d spac %d",
	    text,
	    extent.x, extent.y,
	    extent.width, extent.height,
	    ascent, descent, spacing);
    }
#endif

    g_object_unref(G_OBJECT(layout));

    if (colptr && colptr->is_secret)
    {
        g_free((gchar *) text);
    }

    if (width)
	*width = extent.width;
    if (height)
	*height = extent.height;
}

static inline guint
_default_font_descent(GtkWidget *widget)
{
    PangoFontDescription *font_desc = NULL;
    GtkStyleContext *style_context = gtk_widget_get_style_context(widget);

    gtk_style_context_get (style_context, GTK_STATE_FLAG_NORMAL,
                       GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    if (!font_desc)
	return (GTK_SHEET_DEFAULT_FONT_DESCENT);

    PangoContext *context = gtk_widget_get_pango_context(widget);

    PangoFontMetrics *metrics = pango_context_get_metrics(context,
	font_desc, pango_context_get_language(context));
    guint val =  pango_font_metrics_get_descent(metrics);

    pango_font_metrics_unref(metrics);
    pango_font_description_free(font_desc);

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
            {
                return (i);
            }
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

/**
 * _gtk_sheet_invalidate_region: 
 * Wrapper for gdk_window_invalidate_region
 * 
 * - invalidates sheet_window
 * - give coordinates relative to sheet_window
 * 
 * @param sheet  the sheet
 * @param x      x
 * @param y      y
 * @param width  width
 * @param height height
 */
static void _gtk_sheet_invalidate_region(
    GtkSheet *sheet, 
    gint x, gint y, gint width, gint height)
{
#if GTK_SHEET_DEBUG_DRAW_INVALIDATE > 0
    g_debug("_gtk_sheet_invalidate_region %d %d %d %d",
        x, y, width, height);
#endif
    cairo_rectangle_int_t rect = { x, y, width, height };
    cairo_region_t *crg = cairo_region_create_rectangle(&rect);
    gdk_window_invalidate_region(sheet->sheet_window, crg, FALSE);
    cairo_region_destroy(crg);
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

extern void _gtksheet_signal_emit(GObject *object, guint signal_id, ...);

/* Prototypes (static) */

static void gtk_sheet_class_init(GtkSheetClass *klass);
static void gtk_sheet_init(GtkSheet *sheet);
static void gtk_sheet_destroy_handler(GtkWidget *widget);
static void gtk_sheet_finalize_handler(GObject *object);
static void gtk_sheet_style_set_handler(GtkWidget *widget,
    GtkStyle  *previous_style);
static void gtk_sheet_realize_handler(GtkWidget *widget);
static void gtk_sheet_unrealize_handler(GtkWidget *widget);
static void gtk_sheet_map_handler(GtkWidget *widget);
static void gtk_sheet_unmap_handler(GtkWidget *widget);

static gboolean gtk_sheet_draw(GtkWidget *widget, cairo_t *cr);

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

static void gtk_sheet_size_request(GtkWidget *widget,
    GtkRequisition *requisition);
static void gtk_sheet_get_preferred_width (GtkWidget *widget,
    gint *minimal_width, gint *natural_width);
static void gtk_sheet_get_preferred_height (GtkWidget *widget,
    gint *minimal_height, gint *natural_height);
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
static void _cell_draw_background(GtkSheet *sheet, gint row, gint column,
    cairo_t *swin_cr);

/* draw cell border */
static void _cell_draw_border(GtkSheet *sheet,
    gint row, gint column, gint mask);

/* draw cell contents */
static void _cell_draw_label(GtkSheet *sheet, gint row, gint column, 
    cairo_t *swin_cr);

/* highlight the visible part of the selected range */
static void gtk_sheet_range_draw_selection(
    GtkSheet *sheet, GtkSheetRange range, cairo_t *cr);

/* Selection */

static gint _gtk_sheet_move_query(
    GtkSheet *sheet, gint row, gint column, gboolean need_focus);
static void gtk_sheet_real_select_range(
    GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_real_unselect_range(
    GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_extend_selection(
    GtkSheet *sheet, gint row, gint column);
static void gtk_sheet_new_selection(
    GtkSheet *sheet, GtkSheetRange *range);
static void gtk_sheet_draw_border(
    GtkSheet *sheet, GtkSheetRange range, cairo_t *cr);
static void gtk_sheet_draw_corners(
    GtkSheet *sheet, GtkSheetRange range, cairo_t *cr);

/* Active Cell handling */

static void gtk_sheet_entry_changed_handler(
    GtkWidget *widget, gpointer data);
static gboolean gtk_sheet_activate_cell(
    GtkSheet *sheet, gint row, gint col);
static void gtk_sheet_draw_active_cell(GtkSheet *sheet, cairo_t *cr);
static void gtk_sheet_show_active_cell(GtkSheet *sheet);
void gtk_sheet_click_cell(
    GtkSheet *sheet, gint row, gint column, gboolean *veto);

/* Backing Pixmap */

static void gtk_sheet_make_bsurf(
    GtkSheet *sheet, guint width, guint height);
static void gtk_sheet_draw_backing_pixmap(
    GtkSheet *sheet, GtkSheetRange range, cairo_t *cr);

/* Scrollbars */

static void _vadjustment_changed_handler(
    GtkAdjustment *adjustment, gpointer data);
static void _hadjustment_changed_handler(
    GtkAdjustment *adjustment, gpointer data);
static void _vadjustment_value_changed_handler(
    GtkAdjustment *adjustment, gpointer data);
static void _hadjustment_value_changed_handler(
    GtkAdjustment *adjustment, gpointer data);

static void draw_xor_vline(GtkSheet *sheet, gboolean draw);
static void draw_xor_hline(GtkSheet *sheet, gboolean draw);
static void gtk_sheet_draw_flashing_range(
    GtkSheet *sheet, cairo_t *xor_cr);
static guint new_column_width(GtkSheet *sheet, gint column, gint *x);
static guint new_row_height(GtkSheet *sheet, gint row, gint *y);

/* Sheet Entry */

static void create_sheet_entry(GtkSheet *sheet, GType new_entry_type);
//static void gtk_sheet_entry_set_max_size(GtkSheet *sheet);

/* Sheet button gadgets */

static void _gtk_sheet_row_buttons_size_allocate(GtkSheet *sheet);

static void row_button_set(GtkSheet *sheet, gint row);
static void row_button_release(GtkSheet *sheet, gint row);

/* global sheet button */
static void _global_sheet_button_create(GtkSheet *sheet);
static void _global_sheet_button_size_allocate(GtkSheet *sheet);

/* Attributes routines */

static void gtk_sheet_set_cell_attributes(GtkSheet *sheet,
    gint row, gint col, GtkSheetCellAttr attributes);
static void init_attributes(GtkSheet *sheet,
    gint col, GtkSheetCellAttr *attributes);

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
static void gtk_sheet_remove_handler(
    GtkContainer *container, GtkWidget *widget);
static void gtk_sheet_realize_child(
    GtkSheet *sheet, GtkSheetChild *child);
static void gtk_sheet_position_child(
    GtkSheet *sheet, GtkSheetChild *child);
static void gtk_sheet_row_size_request(
    GtkSheet *sheet, gint row, guint *requisition);

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

    g_return_if_fail(GTK_IS_SHEET(sheet));
    g_return_if_fail(GTK_IS_SHEET_COLUMN(child));

    gtk_sheet_add_column(sheet, 1);
    int col = gtk_sheet_get_columns_count(sheet) - 1;

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
    g_debug("%s(%d): %p %s child %s %p name %s m %d r %d v %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
        G_OBJECT_TYPE_NAME(child), child, 
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

    DEBUG_WIDGET_SET_PARENT(child, sheet);
    gtk_widget_set_parent(GTK_WIDGET(child), GTK_WIDGET(sheet));

    if (gtk_widget_get_realized(GTK_WIDGET(sheet))
        && GTK_IS_SHEET_COLUMN(child))
    {
        _gtk_sheet_column_realize(child, sheet);

        _gtk_sheet_column_check_windows(child, sheet);
    }

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
    const gchar *name = gtk_widget_get_name(GTK_WIDGET(child));

#if GTK_SHEET_DEBUG_BUILDER > 0
    g_debug("gtk_sheet_buildable_add_child %p: %s type %s", child,
	name ? name : "NULL",
	type ? type : "NULL");
#endif

    GtkSheet *sheet = GTK_SHEET(buildable);
    GtkSheetColumn *newcol = GTK_SHEET_COLUMN(child);

#if GTK_SHEET_DEBUG_BUILDER > 0
    {
	gchar *strval;

	g_object_get(G_OBJECT(newcol), "label", &strval, NULL);
        g_debug("gtk_sheet_buildable_add_child: label=%s",
            strval ? strval : "NULL");

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

	sheet_type = g_type_register_static(gtk_container_get_type(),
	    "GtkSheet",
	    &sheet_info,
	    0);

	static const GInterfaceInfo interface_info = {
	    (GInterfaceInitFunc)gtk_sheet_buildable_init,
	    (GInterfaceFinalizeFunc)NULL,
	    (gpointer)NULL 
	};

	g_type_add_interface_static(sheet_type, GTK_TYPE_BUILDABLE,
	    &interface_info);

	static const GInterfaceInfo scrollable_info = {
		(GInterfaceInitFunc) NULL,
		(GInterfaceFinalizeFunc) NULL,
		(gpointer) NULL
	    };

        g_type_add_interface_static(
            sheet_type, GTK_TYPE_SCROLLABLE, &scrollable_info);
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
    if (entry_type == GTK_TYPE_ENTRY)
	return (GTK_SHEET_ENTRY_TYPE_GTK_ENTRY);

    else if (entry_type == GTK_TYPE_TEXT_VIEW)
	return (GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW);

    else if (entry_type == GTK_TYPE_DATA_ENTRY)
	return (GTK_SHEET_ENTRY_TYPE_GTK_DATA_ENTRY);

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
	case GTK_SHEET_ENTRY_TYPE_GTK_ENTRY:
	    return (GTK_TYPE_ENTRY);

	case GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW:
	    return (GTK_TYPE_TEXT_VIEW);

	case GTK_SHEET_ENTRY_TYPE_GTK_DATA_ENTRY:
	    return (GTK_TYPE_DATA_ENTRY);

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
    guint property_id, const GValue *value, GParamSpec   *pspec)
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

	case PROP_GTK_SHEET_HADJUSTMENT:
	    gtk_sheet_set_hadjustment(sheet, g_value_get_object (value));
	    break;

	case PROP_GTK_SHEET_VADJUSTMENT:
	    gtk_sheet_set_vadjustment(sheet, g_value_get_object (value));
	    break;

	case PROP_GTK_SHEET_HSCROLL_POLICY:
	    if (sheet->hscroll_policy != g_value_get_enum (value))
	    {
		sheet->hscroll_policy = g_value_get_enum (value);
		gtk_widget_queue_resize (GTK_WIDGET (sheet));
		g_object_notify_by_pspec (object, pspec);
	      }
	    break;

	case PROP_GTK_SHEET_VSCROLL_POLICY:
	    if (sheet->vscroll_policy != g_value_get_enum (value))
	      {
		sheet->vscroll_policy = g_value_get_enum (value);
		gtk_widget_queue_resize (GTK_WIDGET (sheet));
		g_object_notify_by_pspec (object, pspec);
	      }
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
	    g_value_set_enum(
                value, _gtk_sheet_entry_type_from_gtype(sheet->entry_type));
	    break;

	case PROP_GTK_SHEET_VJUST:
	    g_value_set_enum(value, sheet->vjust);
	    break;

	case PROP_GTK_SHEET_HADJUSTMENT:
	    g_value_set_object(value, sheet->hadjustment);
	    break;

	case PROP_GTK_SHEET_VADJUSTMENT:
	    g_value_set_object(value, sheet->vadjustment);
	    break;

	case PROP_GTK_SHEET_HSCROLL_POLICY:
	    g_value_set_enum (value, sheet->hscroll_policy);
	    break;

	case PROP_GTK_SHEET_VSCROLL_POLICY:
	    g_value_set_enum (value, sheet->vscroll_policy);
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

    /* GtkScrollable implementation */
    g_object_class_override_property (gobject_class, 
	PROP_GTK_SHEET_HADJUSTMENT, "hadjustment");
    g_object_class_override_property (gobject_class, 
	PROP_GTK_SHEET_VADJUSTMENT, "vadjustment");
    g_object_class_override_property (gobject_class, 
	PROP_GTK_SHEET_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property (gobject_class, 
	PROP_GTK_SHEET_VSCROLL_POLICY, "vscroll-policy");

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
	GDK_TYPE_RGBA,
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
	GDK_TYPE_RGBA,
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

static void gtk_sheet_debug_select_range(GtkSheet *sheet,
    GtkSheetRange *range)
{
    g_debug("SIGNAL select-range %p {%d, %d, %d, %d}", sheet,
	range->row0, range->col0, range->rowi, range->coli);
}

static void gtk_sheet_debug_clip_range(GtkSheet *sheet,
    GtkSheetRange *range)
{
    g_debug("SIGNAL clip-range %p {%d, %d, %d, %d}", sheet,
	range->row0, range->col0, range->rowi, range->coli);
}

static void gtk_sheet_debug_resize_range(GtkSheet *sheet,
    GtkSheetRange *old_range, GtkSheetRange *new_range)
{
    g_debug("SIGNAL resize-range %p {%d, %d, %d, %d} -> {%d, %d, %d, %d}", 
        sheet,
	old_range->row0, old_range->col0,
        old_range->rowi, old_range->coli,
	new_range->row0, new_range->col0,
        new_range->rowi, new_range->coli);
}

static void gtk_sheet_debug_move_range(GtkSheet *sheet,
    GtkSheetRange *old_range, GtkSheetRange *new_range)
{
    g_debug("SIGNAL move-range %p {%d, %d, %d, %d} -> {%d, %d, %d, %d}",
        sheet,
	old_range->row0, old_range->col0,
        old_range->rowi, old_range->coli,
	new_range->row0, new_range->col0,
        new_range->rowi, new_range->coli);
}

static gboolean gtk_sheet_debug_traverse(GtkSheet *sheet,
    gint row, gint column, gint *new_row, gint *new_column)
{
    g_debug("SIGNAL traverse %p row %d col %d nrow %d ncol %d",
        sheet,
	row, column, *new_row, *new_column);
    return (TRUE);
}

static gboolean gtk_sheet_debug_deactivate(GtkSheet *sheet,
    gint row, gint column)
{
    g_debug("SIGNAL deactivate %p row %d col %d", sheet, row, column);
    return (TRUE);
}

static gboolean gtk_sheet_debug_activate(GtkSheet *sheet,
    gint row, gint column)
{
    g_debug("SIGNAL activate %p row %d col %d", sheet, row, column);
    return (TRUE);
}

static void gtk_sheet_debug_set_cell(GtkSheet *sheet,
    gint row, gint column)
{
    g_debug("SIGNAL set-cell %p row %d col %d", sheet, row, column);
}

static void gtk_sheet_debug_clear_cell(GtkSheet *sheet,
    gint row, gint column)
{
    g_debug("SIGNAL clear-cell %p row %d col %d", sheet, row, column);
}

#if 0
static void gtk_sheet_debug_changed(GtkSheet *sheet,
    gint row, gint column)
{
    g_debug("SIGNAL changed %p row %d col %d", sheet, row, column);
}
#endif

static void gtk_sheet_debug_new_column_width(GtkSheet *sheet,
    gint col, guint width)
{
    g_debug("SIGNAL new-column-width %p col %d width %d",
        sheet, col, width);
}

static void gtk_sheet_debug_new_row_height(GtkSheet *sheet,
    gint row, guint height)
{
    g_debug("SIGNAL new-row-height %p row %d height %d",
        sheet, row, height);
}

static gboolean gtk_sheet_debug_focus_in_event(GtkSheet *sheet,
    GdkEventFocus *event)
{
    g_debug("SIGNAL focus-in-event %p event %p", sheet, event);
    return (FALSE);
}

static gboolean gtk_sheet_debug_focus_out_event(GtkSheet *sheet,
    GdkEventFocus *event)
{
    g_debug("SIGNAL focus-out-event %p event %p", sheet, event);
    return (FALSE);
}

#endif

static void
_gtk_sheet_class_init_signals(GObjectClass *gobject_class,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, select_row),
	NULL, NULL,
	gtksheet_VOID__INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, select_column),
	NULL, NULL,
	gtksheet_VOID__INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, select_range),
	NULL, NULL,
	gtksheet_VOID__BOXED,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, clip_range),
	NULL, NULL,
	gtksheet_VOID__BOXED,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, resize_range),
	NULL, NULL,
	gtksheet_VOID__BOXED_BOXED,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, move_range),
	NULL, NULL,
	gtksheet_VOID__BOXED_BOXED,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, traverse),
	NULL, NULL,
	gtksheet_BOOLEAN__INT_INT_POINTER_POINTER,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, deactivate),
	NULL, NULL,
	gtksheet_BOOLEAN__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, activate),
	NULL, NULL,
	gtksheet_BOOLEAN__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, set_cell),
	NULL, NULL,
	gtksheet_VOID__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, clear_cell),
	NULL, NULL,
	gtksheet_VOID__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, changed),
	NULL, NULL,
	gtksheet_VOID__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, changed),
	NULL, NULL,
	gtksheet_VOID__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, changed),
	NULL, NULL,
	gtksheet_VOID__INT_INT,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, focus_in_event),
	NULL, NULL,
	gtksheet_BOOLEAN__BOXED,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	G_STRUCT_OFFSET(GtkSheetClass, focus_out_event),
	NULL, NULL,
	gtksheet_BOOLEAN__BOXED,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	0,
	NULL, NULL,
	gtksheet_VOID__OBJECT,
	G_TYPE_NONE, 1,
	gtk_menu_get_type());


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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
	G_STRUCT_OFFSET(GtkSheetClass, move_cursor),
	NULL, NULL,
	gtksheet_VOID__ENUM_INT_BOOLEAN,
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
	G_TYPE_FROM_CLASS(gobject_class),
	G_SIGNAL_RUN_LAST,
	0,
	NULL, NULL,
	gtksheet_BOOLEAN__BOXED,
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
 * gtk_sheet_style_updated - style update handler
 *
 * @param widget
 */
static void gtk_sheet_style_updated(GtkWidget *widget)
{
    //g_debug("gtk_sheet_style_updated");

    _gtk_sheet_range_draw(GTK_SHEET(widget), NULL, FALSE);
    GTK_WIDGET_CLASS (sheet_parent_class)->style_updated(widget);
}

/*
 * gtk_sheet_screen_changed - style update handler
 * 
 * this signal is used by inspector to force immediate 
 * screen updates.
 *  
 * @param widget
 * @param previous_screen
 */
static void gtk_sheet_screen_changed(GtkWidget *widget,
    GdkScreen *previous_screen)
{
    //g_debug("gtk_sheet_style_updated");

    _gtk_sheet_range_draw(GTK_SHEET(widget), NULL, TRUE);
    GTK_WIDGET_CLASS (sheet_parent_class)->style_updated(widget);
}

/*
 * gtk_sheet_class_init - GtkSheet class initialisation
 *
 * @param klass
 */

static void
gtk_sheet_class_init(GtkSheetClass *klass)
{
    GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
    GtkContainerClass *container_class = (GtkContainerClass *)klass;
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

    sheet_parent_class = g_type_class_peek_parent(klass);

    _gtk_sheet_class_init_signals(gobject_class, widget_class);
    _gtk_sheet_class_init_bindings(klass);

    container_class->add = NULL;
    container_class->remove = gtk_sheet_remove_handler;
    container_class->forall = gtk_sheet_forall_handler;

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
    widget_class->draw = gtk_sheet_draw;
    widget_class->get_preferred_width = gtk_sheet_get_preferred_width;
    widget_class->get_preferred_height = gtk_sheet_get_preferred_height;
    widget_class->size_allocate = gtk_sheet_size_allocate_handler;
    widget_class->focus = gtk_sheet_focus;
    widget_class->focus_in_event = NULL;
    widget_class->focus_out_event = NULL;
    widget_class->destroy = gtk_sheet_destroy_handler;
    widget_class->style_updated = gtk_sheet_style_updated;
    widget_class->screen_changed = gtk_sheet_screen_changed;

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

    gtk_widget_class_set_css_name (widget_class, "sheet");
}

static gboolean has_css_parsing_errors = FALSE;

static void
_gtk_sheet_css_parsing_error(
    GtkCssProvider *provider,
    GtkCssSection *section,
    GError *error,
    gpointer user_data)
{
    if (!error) return;

    gchar *file_name = (gchar *) user_data;
    guint line = gtk_css_section_get_start_line(section) + 1;
    guint pos = gtk_css_section_get_start_position(section) + 1;

    if (error->code != G_FILE_ERROR_ACCES)
    {
        g_warning("CSS Error %d: %s:%d:%d:%s",
            error->code, 
            file_name ? file_name : "",
            line, 
            pos,
            error->message);
    }
    has_css_parsing_errors = TRUE;
}

/* 
https://bugzilla.redhat.com/show_bug.cgi?id=1404656 
 
==23841== Invalid read of size 8
==23841==    at 0x5B16B17: _gtk_widget_get_toplevel (gtkwidgetprivate.h:381)
==23841==    by 0x5B16B17: gtk_widget_get_scale_factor (gtkwidget.c:10887)
==23841==    by 0x5911048: gtk_css_widget_node_get_style_provider (gtkcsswidgetnode.c:246)
==23841==    by 0x58F653B: gtk_css_node_get_style_provider_or_null (gtkcssnode.c:121)
==23841==    by 0x58F653B: gtk_css_node_invalidate_style_provider (gtkcssnode.c:1316)
==23841==    by 0x74B6677: g_closure_invoke (gclosure.c:804)
==23841==    by 0x74C7FCC: signal_emit_unlocked_R (gsignal.c:3635)
==23841==    by 0x74CFF30: g_signal_emit_valist (gsignal.c:3391)
==23841==    by 0x74D0191: g_signal_emit (gsignal.c:3447)
==23841==    by 0x5A6CDA4: _gtk_style_cascade_add_provider (gtkstylecascade.c:380)
==23841==    by 0x504DCD8: gtk_sheet_init (gtksheet.c:3567)
==23841==    by 0x74D720A: g_type_create_instance (gtype.c:1866)
==23841==    by 0x74BB65C: g_object_new_internal (gobject.c:1783)
==23841==    by 0x74BD14C: g_object_newv (gobject.c:1930)
==23841==  Address 0xd1 is not stack'd, malloc'd or (recently) free'd
*/ 

static gboolean style_provider_was_set = FALSE; /* Bug id 1404656 */

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

#ifdef GTK_SHEET_DEBUG
    gdk_rgba_parse(&debug_color, GTK_SHEET_DEBUG_COLOR);
    g_debug("debug_color: initialized to r %g g %g b %g a %g", 
	debug_color.red, debug_color.green, debug_color.blue, debug_color.alpha);
#endif

    gdk_rgba_parse(&color_black, "black");
    gdk_rgba_parse(&color_white, "white");

    gdk_rgba_parse(&sheet->bg_color, GTK_SHEET_DEFAULT_BG_COLOR);

    gdk_rgba_parse(&sheet->grid_color, GTK_SHEET_DEFAULT_GRID_COLOR);
    sheet->show_grid = FALSE;

    gdk_rgba_parse(&sheet->tm_color, GTK_SHEET_DEFAULT_TM_COLOR);
    sheet->deprecated_bg_color_used = FALSE;

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

    sheet->min_vis_col_index = -1;
    sheet->max_vis_col_index = -1;
    sheet->min_vis_row_index = -1;
    sheet->max_vis_row_index = -1;

    sheet->view.row0 = -1;
    sheet->view.col0 = -1;
    sheet->view.rowi = -1;
    sheet->view.coli = -1;

    sheet->data = NULL;

    sheet->maxalloc_row_array = -1;
    sheet->maxallocrow = -1;
    sheet->maxalloccol = -1;

    SET_ACTIVE_CELL(-1, -1);

    sheet->sheet_entry = NULL;
    sheet->entry_type = G_TYPE_NONE;
    sheet->installed_entry_type = G_TYPE_NONE;

    SET_SELECTION_PIN(-1, -1);
    SET_SELECTION_CURSOR(-1, -1);

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

    sheet->bsurf = NULL;
    sheet->bsurf_width = 0;
    sheet->bsurf_height = 0;
    sheet->bsurf_cr = NULL;

    sheet->hoffset = 0;
    sheet->voffset = 0;

    sheet->old_hadjustment = 0;
    sheet->old_vadjustment = 0;

    sheet->hadjustment = sheet->vadjustment = NULL;
    sheet->hscroll_policy = sheet->vscroll_policy = 0;

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

    {
        gtk_style_context_add_class(
            gtk_widget_get_style_context(GTK_WIDGET(sheet)),
            GTK_STYLE_CLASS_VIEW);

        if (!style_provider_was_set)
        {
	    GError *error = NULL;
	    GtkCssProvider *provider = gtk_css_provider_new();

	    /* for predefined colors,
	       see gtk-contained.css section "GTK NAMED COLORS" near eof */

	    g_signal_connect(provider, "parsing-error", 
		G_CALLBACK(_gtk_sheet_css_parsing_error),
                GTK_SHEET_CSS_FILE_NAME);

            gtk_css_provider_load_from_path(provider,
		GTK_SHEET_CSS_FILE_NAME, &error);

	    if (error || has_css_parsing_errors)
	    {
                g_debug("%s(%d): fallback to CSS resource://"
                    GTK_SHEET_CSS_RESOURCE_NAME,
                    __FUNCTION__, __LINE__);

                g_object_unref(provider);
                provider = gtk_css_provider_new();

                g_signal_connect(provider, "parsing-error", 
                    G_CALLBACK(_gtk_sheet_css_parsing_error),
                    "resource://" GTK_SHEET_CSS_RESOURCE_NAME);

		has_css_parsing_errors = FALSE;

		gtk_css_provider_load_from_resource (
		    provider, GTK_SHEET_CSS_RESOURCE_NAME);

		if (has_css_parsing_errors) 
                    g_assert(FALSE);
	    }

	    GdkDisplay *display = gdk_display_get_default ();
	    GdkScreen *screen = gdk_display_get_default_screen (display);

            gtk_style_context_add_provider_for_screen(
                GDK_SCREEN(screen), 
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION - 10);

            style_provider_was_set = TRUE;

	    g_object_unref(provider);
        }
    }
}


static void
_gtk_sheet_row_init(GtkSheetRow *row)
{
    row->name = NULL;
    row->height = GTK_SHEET_ROW_DEFAULT_HEIGHT;
    row->requisition = GTK_SHEET_ROW_DEFAULT_HEIGHT;
    row->top_ypixel = 0;
    row->max_extent_height = 0;

    row->button.state = GTK_STATE_FLAG_NORMAL;
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
#if 0
    g_debug("%s(%d): rows %d columns %d",
        __FUNCTION__, __LINE__,
	rows, columns
	);
#endif
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
    _global_sheet_button_create(sheet);

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
 * Changes the default entry type of the #GtkSheet. The old
 * sheet entry widget is dropped and a new entry widget created.
 * Beware: You will will have to reconnect all your signal
 * handlers after changing default entry type.
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

    /* save new default type, no matter wether it failed */
    sheet->entry_type = entry_type;

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
 * 
 * Deprecated:4.2.0: Use CSS instead
 */
void
gtk_sheet_show_grid(GtkSheet *sheet, gboolean show)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (show == sheet->show_grid) return;

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
 *  
 * Deprecated:4.2.0: Use CSS instead
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
 * @color: a #GdkRGBA structure
 *
 * Sets the background color of the #GtkSheet.
 * If pass NULL, the sheet will be reset to the default color.
 * 
 * Deprecated:4.1.3: Use #gtk_sheet_set_css_class() instead
 */
void
gtk_sheet_set_background(GtkSheet *sheet, GdkRGBA *color)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!color)
    {
	gdk_rgba_parse(&sheet->bg_color, GTK_SHEET_DEFAULT_BG_COLOR);
        sheet->deprecated_bg_color_used = FALSE;
    }
    else
    {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
        g_warning("%s(%d): deprecated_bg_color_used %s %s",
            __FUNCTION__, __LINE__,
            G_OBJECT_TYPE_NAME(sheet),
            gtk_widget_get_name(GTK_WIDGET(sheet)));
#endif
	sheet->bg_color = *color;
        sheet->deprecated_bg_color_used = TRUE;
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
}

/**
 * gtk_sheet_set_css_class:
 * @sheet: a #GtkSheet
 * @css_class: (nullable): a CSS class name
 *
 * Sets the background color of the #GtkSheet.
 * If pass NULL, the sheet will be reset to the default color.
 */
void
gtk_sheet_set_css_class(GtkSheet *sheet, gchar *css_class)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
        GTK_WIDGET(sheet));

    if (sheet->css_class)
    {
        gtk_style_context_remove_class(sheet_context,
            sheet->css_class);

        g_free(sheet->css_class);
        sheet->css_class = NULL;
    }

    sheet->css_class = g_strdup(css_class);

    if (sheet->css_class)
        gtk_style_context_add_class(sheet_context,
            sheet->css_class);

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
}

/**
 * gtk_sheet_set_grid:
 * @sheet: a #GtkSheet
 * @color: a #GdkRGBA structure
 *
 * Set the grid color.
 * If pass NULL, the grid will be reset to the default color. 
 *  
 * Deprecated:4.2.0: Use CSS instead
 */
void
gtk_sheet_set_grid(GtkSheet *sheet, GdkRGBA *color)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!color)
    {
	gdk_rgba_parse(&sheet->grid_color, GTK_SHEET_DEFAULT_GRID_COLOR);
    }
    else
    {
	sheet->grid_color = *color;
    }

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
 * recalc maximum cell text width without column title button
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
    
    for (row = 0; row <= sheet->maxrow; row++)
    {
	GtkSheetRow *rowptr = ROWPTR(sheet, row);

	if (GTK_SHEET_ROW_IS_VISIBLE(rowptr))
	{
	    GtkSheetCell *cell = sheet->data[row][col];

	    if (cell && cell->text && cell->text[0])
	    {
		GtkSheetCellAttr attributes;

		gtk_sheet_get_attributes(sheet,
                    row, col, &attributes);

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

    for (col = 0; col <= sheet->maxcol; col++)
    {
	GtkSheetColumn *colptr = COLPTR(sheet, col);

	if (GTK_SHEET_COLUMN_IS_VISIBLE(colptr))
	{
	    GtkSheetCell *cell = sheet->data[row][col];

	    if (cell && cell->text && cell->text[0])
	    {
		GtkSheetCellAttr attributes;

		gtk_sheet_get_attributes(sheet,
                    row, col, &attributes);

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
 * @text:   formatted cell text
 * 
 * update cell extent and propagate to max row/column extent
 */
static void _gtk_sheet_update_extent(GtkSheet *sheet,
    GtkSheetCell *cell, gint row, gint col,
    gchar *text)
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

    if (!text || !text[0])
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

    if (attributes.deprecated_font_desc_used)
    {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
        g_warning(
            "%s(%d): deprecated_font_desc_used %s %s row %d col %d",
            __FUNCTION__, __LINE__,
            G_OBJECT_TYPE_NAME(sheet),
            gtk_widget_get_name(GTK_WIDGET(sheet)),
            row, col);
#endif
    }

    _get_string_extent(sheet, colptr,
	attributes.font_desc, 
        text, cell->is_markup,
        &text_width, &text_height);

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

/* return font, font_desc must be freed after use */
static PangoFontDescription *
_style_context_get_font(GtkStyleContext *style_context)
{
    PangoFontDescription *font_desc;

    gtk_style_context_save(style_context);
    gtk_style_context_set_state(style_context, GTK_STATE_FLAG_NORMAL);
    GtkStateFlags flags = gtk_style_context_get_state(style_context);

    gtk_style_context_get(
        style_context, 
        flags,
        GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    gtk_style_context_restore(style_context);
    return(font_desc);
}

/* return sum of margin,border,padding width */
static gint
_style_context_get_gap_width(GtkStyleContext *style_context)
{
    GtkBorder border;
    gint width = 0;

    gtk_style_context_save(style_context);
    gtk_style_context_set_state(style_context, GTK_STATE_FLAG_NORMAL);
    GtkStateFlags flags = gtk_style_context_get_state(style_context);

    gtk_style_context_get_border(style_context, flags, &border);
    width += border.left + border.right;

    gtk_style_context_get_padding(style_context, flags, &border);
    width += border.left + border.right;

    gtk_style_context_get_margin(style_context, flags, &border);
    width += border.left + border.right;

    gtk_style_context_restore(style_context);
    return(width);
}

static guint
_get_button_width(GtkSheet *sheet, GtkWidget *widget)
{
    /* Note: calling gtk_widget_get_preferred_size()
       will not work because columns outside of visible
       area are switched to visible FALSE
       */
    /* Note: calling gtk_widget_get_preferred_size()
       will not work because columns outside of visible
       area are switched to visible FALSE
       */

    gint width = 0; /* return value */

    if (!GTK_IS_BUTTON(widget)) return(width);

    GtkButton *button = GTK_BUTTON(widget);
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

    GtkStyleContext *style_context
        = gtk_widget_get_style_context(widget);

    /* Note: button image has precedence over label */

    if (child && GTK_IS_IMAGE(child))
    {
        GdkPixbuf *p = gtk_image_get_pixbuf(GTK_IMAGE(child));
        if (p)
        {
            gint pixbuf_width = gdk_pixbuf_get_width(p);

#if GTK_SHEET_DEBUG_SIZE > 0
            g_debug("%s[%d] FIXME image pixbuf_width %d", 
                __FUNCTION__, __LINE__, pixbuf_width);
#endif
            width += pixbuf_width;
        }
    }
    else /* check button label width */
    {
        const gchar *label = gtk_button_get_label(button);

        if (label)
        {
            PangoFontDescription *font_desc
                = _style_context_get_font(style_context);

            guint label_width, label_height;
            _get_string_extent(sheet, NULL, font_desc,
                label, FALSE, &label_width, &label_height);

            width += label_width;

            pango_font_description_free(font_desc);
        }
    }

    /* button widget gap */
    {
        gint widget_gap
            = _style_context_get_gap_width(style_context);

#if GTK_SHEET_DEBUG_SIZE > 0
        g_debug("%s[%d] FIXME widget_gap %d", 
            __FUNCTION__, __LINE__, widget_gap);
#endif
        width += widget_gap;
    }

    /* button child gap */
    if (child)
    {
        GtkStyleContext *style_context
            = gtk_widget_get_style_context(child);

        guint child_gap
            = _style_context_get_gap_width(style_context);

#if GTK_SHEET_DEBUG_SIZE > 0
        g_debug("%s[%d] FIXME child_gap %d", 
            __FUNCTION__, __LINE__, child_gap);
#endif
        width += child_gap;
    }

    return(width);
}

static gboolean
_gtk_sheet_autoresize_column_internal(GtkSheet *sheet, gint col)
{
    gint new_width;
    GtkSheetColumn *colptr;

    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (col < 0 || col > sheet->maxalloccol || col > sheet->maxcol)
	return(FALSE);

    colptr = COLPTR(sheet, col);

    if (!GTK_SHEET_COLUMN_IS_VISIBLE(colptr))
	return(FALSE);

    new_width = COLUMN_EXTENT_TO_WIDTH(colptr->max_extent_width);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug(
        "%s[%d]: FIXME col %d %s max_extent_width %d new_width %d",
        __FUNCTION__, __LINE__, col,
        gtk_widget_get_name(GTK_WIDGET(colptr)), 
        colptr->max_extent_width,
        new_width);
#endif
    if (colptr->col_button)
    {
        guint w = _get_button_width(sheet, colptr->col_button);

        if (w > new_width)
            new_width = w;
    }

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("%s(%d): called col %d w %d new w %d",
        __FUNCTION__, __LINE__, col, colptr->width, new_width);
#endif

    if (new_width != colptr->width)
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("%s(%d): col %d set width %d",
            __FUNCTION__, __LINE__, col, new_width);
#endif
	gtk_sheet_set_column_width(sheet, col, new_width);
	GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_REDRAW_PENDING);

        return(TRUE);
    }

    return(FALSE);
}

static gboolean
_gtk_sheet_autoresize_row_internal(GtkSheet *sheet, gint row)
{
    gint new_height;
    GtkSheetRow *rowptr;

    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (row < 0 || row > sheet->maxallocrow || row > sheet->maxrow)
	return(FALSE);

    rowptr = ROWPTR(sheet, row);

    if (!GTK_SHEET_ROW_IS_VISIBLE(rowptr))
	return(FALSE);

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

        return(TRUE);
    }

    return(FALSE);
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

	for (row = 0; row <= sheet->maxrow; row++)
	{
	    GtkSheetRow *rowobj = ROWPTR(sheet, row);
	    g_assert(rowobj);
	    
	    if (GTK_SHEET_ROW_IS_VISIBLE(rowobj))
	    {
		_gtk_sheet_autoresize_row_internal(sheet, row);
	    }
	}
    }
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

    gtk_sheet_autoresize_all(sheet);
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

    gtk_sheet_autoresize_all(sheet);
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

    gtk_sheet_autoresize_all(sheet);
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

#if 0
    /* disabled since ages */
    GtkWidget *label;

    if (gtk_bin_get_child(GTK_BIN(sheet->button)))
	label = gtk_bin_get_child(GTK_BIN(sheet->button));

    gtk_label_set_text(GTK_LABEL(label), title);
#endif

    _gtk_sheet_global_sheet_button_show(sheet);
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
	_global_sheet_button_size_allocate(sheet);
    }

    if (sheet->row_titles_visible)
    {
	_gtk_sheet_row_buttons_size_allocate(sheet);
    }

    if (sheet->column_titles_visible)
    {
	_gtk_sheet_column_buttons_size_allocate(sheet);
    }

    /* send value_changed or redraw directly */

    if (sheet->vadjustment)
    {
	g_signal_emit_by_name(G_OBJECT(sheet->vadjustment), "value_changed");
	done = TRUE;
    }

    if (sheet->hadjustment)
    {
	g_signal_emit_by_name(G_OBJECT(sheet->hadjustment), "value_changed");
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
	    _gtk_sheet_row_buttons_size_allocate(sheet);
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

    if (sheet->state == GTK_SHEET_NORMAL)
    {
	if (sheet->sheet_entry && gtk_widget_get_mapped(sheet->sheet_entry))
	{
	    gtk_sheet_activate_cell(
                sheet, sheet->active_cell.row, sheet->active_cell.col);
	    /*
	    gtk_sheet_entry_signal_connect_changed(
                sheet, gtk_sheet_entry_changed_handler);
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
 * 
 * Row titles with will be increased automatically, when one of
 * the properties #GtkSheet:autoresize-cols or
 * #GtkSheet:autoresize-rows is true. It will not be reduced
 * automatically, you can use this function to reset it.
 */
void
gtk_sheet_set_row_titles_width(GtkSheet *sheet, guint width)
{
    if (width < GTK_SHEET_COLUMN_MIN_WIDTH) return;

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
 * Show row titles.
 */
void
gtk_sheet_show_row_titles(GtkSheet *sheet)
{
    if (sheet->row_titles_visible) return;

    sheet->row_titles_visible = TRUE;  /* must be set first */

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (GTK_SHEET_IS_FROZEN(sheet)) return;

    gdk_window_show(sheet->row_title_window);

    gdk_window_move_resize(sheet->row_title_window,
	sheet->row_title_area.x,
	sheet->row_title_area.y,
	sheet->row_title_area.width,
	sheet->row_title_area.height);

    _gtk_sheet_global_sheet_button_show(sheet);

    gint row;
    for (row = MIN_VIEW_ROW(sheet);
          row <= MAX_VIEW_ROW(sheet) && row <= sheet->maxrow;
          row++)
    {
	if (row < 0) continue;

	GtkSheetChild *child = sheet->row[row].button.child;
	if (child) _gtk_sheet_child_show(child);
    }

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);
}


/**
 * gtk_sheet_hide_row_titles:
 * @sheet: a #GtkSheet
 *
 * Hide row titles.
 */
void
gtk_sheet_hide_row_titles(GtkSheet *sheet)
{
    if (!sheet->row_titles_visible) return;

    sheet->row_titles_visible = FALSE;  /* must be set first */

    _gtk_sheet_recalc_top_ypixels(sheet);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (GTK_SHEET_IS_FROZEN(sheet)) return;

    if (sheet->row_title_window)
	gdk_window_hide(sheet->row_title_window);

    _gtk_sheet_global_sheet_button_hide(sheet);

    gint row;
    for (row = MIN_VIEW_ROW(sheet);
          row <= MAX_VIEW_ROW(sheet) && row <= sheet->maxrow;
          row++)
    {
	if (row < 0) continue;

        GtkSheetChild *child = sheet->row[row].button.child;
	if (child) _gtk_sheet_child_hide(child);
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
gtk_sheet_row_button_add_label(
    GtkSheet *sheet, gint row, const gchar *label)
{
    GtkRequisition req;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow) return;

    GtkSheetButton *button = &sheet->row[row].button;

    if (button->label)
	g_free(button->label);

    button->label = g_strdup(label);

#define ENABLE_TOGGLE_AUTORESIZE_FLAGS 0
    /* ENABLE_TOGGLE_AUTORESIZE_FLAGS causes major performance issues
       when row buttons visible and you have >1500 rows
       timing 44s (code enabled)
       timing  2s (code disabled)
       */

#if ENABLE_TOGGLE_AUTORESIZE_FLAGS>0
    gboolean aux_c, aux_r;
    aux_c = gtk_sheet_autoresize_columns(sheet);
    aux_r = gtk_sheet_autoresize_rows(sheet);
    gtk_sheet_set_autoresize(sheet, FALSE);
    gtk_sheet_set_autoresize_rows(sheet, TRUE);
#endif

    _gtk_sheet_button_size_request(sheet, button, &req);

#if ENABLE_TOGGLE_AUTORESIZE_FLAGS>0
    gtk_sheet_set_autoresize_columns(sheet, aux_c);
    gtk_sheet_set_autoresize_rows(sheet, aux_r);
#endif

    if (req.height > sheet->row[row].height)
	gtk_sheet_set_row_height(sheet, row, req.height);

    if (req.width > sheet->row_title_area.width)
    {
	gtk_sheet_set_row_titles_width(sheet, req.width);
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	_gtk_sheet_draw_button(sheet, row, -1, NULL);
    }
    g_signal_emit(G_OBJECT(sheet), sheet_signals[CHANGED], 0, row, -1);
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
	_gtk_sheet_draw_button(sheet, row, -1, NULL);
	g_signal_emit(
            G_OBJECT(sheet), sheet_signals[CHANGED], 0, row, -1);
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
	_gtk_sheet_draw_button(sheet, row, -1, NULL);
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
	    g_signal_emit_by_name(G_OBJECT(sheet->vadjustment), "value_changed");
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
	    g_signal_emit_by_name(G_OBJECT(sheet->hadjustment), "value_changed");
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
	sheet->row[row].button.state |= GTK_STATE_FLAG_INSENSITIVE;
    else
	sheet->row[row].button.state &= ~GTK_STATE_FLAG_INSENSITIVE;

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)) 
	&& !GTK_SHEET_IS_FROZEN(sheet)) {
	_gtk_sheet_draw_button(sheet, row, -1, NULL);
    }
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
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (row < 0 || row > sheet->maxrow) return;

    GtkSheetRow *rowobj = ROWPTR(sheet, row);

    if (GTK_SHEET_ROW_IS_VISIBLE(rowobj) == visible) return;

    gint act_row = sheet->active_cell.row;

    if (act_row == row) /* hide active column -> disable active cell */
    {
	_gtk_sheet_hide_active_cell(sheet);

        SET_ACTIVE_CELL(-1, -1);
    }

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
	gboolean veto = _gtk_sheet_deactivate_cell(sheet);
	if (!veto) return;
    }

    sheet->state = GTK_SHEET_ROW_SELECTED;

    sheet->range.row0 = row;
    sheet->range.col0 = 0;
    sheet->range.rowi = row;
    sheet->range.coli = sheet->maxcol;

    SET_SELECTION_PIN(row, 0);

    g_signal_emit(G_OBJECT(sheet), sheet_signals[SELECT_ROW], 0, row);
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
	gboolean veto = _gtk_sheet_deactivate_cell(sheet);
	if (!veto) return;
    }

    sheet->state = GTK_SHEET_COLUMN_SELECTED;

    sheet->range.row0 = 0;
    sheet->range.col0 = column;
    sheet->range.rowi = sheet->maxrow;
    sheet->range.coli = column;

    SET_SELECTION_PIN(0, column);

    g_signal_emit(G_OBJECT(sheet), sheet_signals[SELECT_COLUMN], 0, column);
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
    sheet->clip_timer = g_timeout_add_full(
        0, TIMEOUT_FLASH, gtk_sheet_flash, sheet, NULL);

    g_signal_emit(G_OBJECT(sheet), sheet_signals[CLIP_RANGE],
        0, &sheet->clip_range);
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

    if (sheet->clip_timer)
    {
        g_source_remove(sheet->clip_timer);
        sheet->clip_timer = 0;
    }

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

#if 0
static cairo_t *_create_main_win_xor_cr(GtkSheet *sheet)
{
    cairo_t *xor_cr = gdk_cairo_create(gtk_widget_get_window(GTK_WIDGET(sheet)));
    gdk_cairo_set_source_rgba(xor_cr, &color_white);
    gdk_cairo_set_source_rgba(xor_cr, &debug_color);
    //gdk_cairo_set_source_window(xor_cr, sheet->sheet_window, 0, 0);
    //cairo_set_operator(xor_cr, CAIRO_OPERATOR_XOR);  /* ex GDK_INVERT */
    return(xor_cr);
}
#endif

static void _get_visible_index(
    GtkSheet *sheet,
    gint act_row, gint act_col, 
    gint *vis_row, gint *min_vis_row, gint *max_vis_row,
    gint *vis_col, gint *min_vis_col, gint *max_vis_col)
{
    *vis_row = *min_vis_row = *max_vis_row = -1;
    *vis_col = *min_vis_col = *max_vis_col = -1;

#if 0
    g_debug("_get_visible_index: r/c %d/%d", act_row, act_col);
#endif

    if (act_row < 0 || act_col < 0) return;
    if (act_row > sheet->maxrow || act_col > sheet->maxcol) return;

    *vis_row = ROWPTR(sheet, act_row)->vis_row_index;
    *min_vis_row = sheet->min_vis_row_index;
    *max_vis_row = sheet->max_vis_row_index;

    *vis_col = COLPTR(sheet, act_col)->vis_col_index;
    *min_vis_col = sheet->min_vis_col_index;
    *max_vis_col = sheet->max_vis_col_index;

#if 0
    g_debug("_get_visible_index: row v/min/max %d/%d/%d col v/min/max %d/%d/%d", 
        *vis_row, *min_vis_row, *max_vis_row,
        *vis_col, *min_vis_col, *max_vis_col);
#endif
}

#define GTK_SHEET_CSS_POSITION_PREFIX "position_"
/**
 * _cell_style_context_add_position_subclass:
 * add CSS position subclasses to context.
 * 
 * @param sheet   the sheet
 * @param context style context of the cell
 * @param row     active row or -1
 * @param col     active col or -1
 */
static void _cell_style_context_add_position_subclass(
    GtkSheet *sheet,
    GtkStyleContext *context,
    gint row, gint col)
{
    /* remove all known position sub-classes */
    GList *class_list = gtk_style_context_list_classes(context);
    GList *p;

    for (p=class_list; p; p=p->next)
    {
        if (strncmp(p->data, 
            GTK_SHEET_CSS_POSITION_PREFIX, 
            strlen(GTK_SHEET_CSS_POSITION_PREFIX)) == 0)
        {
            gtk_style_context_remove_class(context, p->data);
        }
    }

    g_list_free(class_list);

    gint vis_row, min_vis_row, max_vis_row;
    gint vis_col, min_vis_col, max_vis_col;

    /* TBD - take care of visibility */

    _get_visible_index(sheet, 
        row, col, 
        &vis_row, &min_vis_row, &max_vis_row,
        &vis_col, &min_vis_col, &max_vis_col);

    if ((vis_row < 0) || (vis_col < 0)) {
        g_warning("no vis_row/vis_col");
        return;
    }

    /* add column sub-classes */

    if (sheet->active_cell.col >= 0
        && col == sheet->active_cell.col)
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "col_active");
    }

    if (vis_col>=0 && (vis_col == min_vis_col))
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "col_first");
    }
    if (vis_col>=0 && (vis_col == max_vis_col))
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "col_last");
    }

    if (vis_col % 2)
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "col_odd");
    }
    else
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "col_even");
    }

    /* add row sub-classes */

    if (sheet->active_cell.row >= 0
        && row == sheet->active_cell.row)
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "row_active");
    }

    if (vis_row>=0 && (vis_row == min_vis_row))
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "row_first");
    }
    if (vis_row>=0 && (vis_row == max_vis_row))
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "row_last");
    }

    if (vis_row % 2)
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "row_odd");
    }
    else
    {
        gtk_style_context_add_class(context,
            GTK_SHEET_CSS_POSITION_PREFIX "row_even");
    }
}

/** 
 * _cairo_clip_sheet_area: 
 * set cairo clipping area to visible part of the sheet.
 * 
 * Note that row/column title windows are overlapping sheet_window.
 * 
 * Use this to prevent overwriting the title areas.
 * Always necessary when copying bsurf to window.
 * 
 * Note that clipping area needs to be restored properly.
 * Failure to do so will result in missing objects, like global sheet button.
 * 
 * @param sheet  the sheet
 * @param cr     cairo context
 */
static void
_cairo_clip_sheet_area(GtkSheet *sheet, cairo_t *cr)
{
    GdkRectangle clip_area;

    g_assert(cr);

    clip_area.x = _gtk_sheet_column_left_xpixel(
        sheet, MIN_VIEW_COLUMN(sheet));

    clip_area.y = _gtk_sheet_row_top_ypixel(
        sheet, MIN_VIEW_ROW(sheet));

    clip_area.width = sheet->sheet_window_width;
    clip_area.height = sheet->sheet_window_height;

    clip_area.x = MAX(clip_area.x, 
	sheet->row_titles_visible ? sheet->row_title_area.width : 0);

    clip_area.y = MAX(clip_area.y, 
	sheet->column_titles_visible ? sheet->column_title_area.height : 0);

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("_cairo_clip_sheet_area %d %d %d %d", 
	clip_area.x, clip_area.y, clip_area.width, clip_area.height);
#endif

    gdk_cairo_rectangle(cr, &clip_area);
    cairo_clip (cr);
}

/** 
 * _cairo_save_and_set_sel_color: 
 * saves cairo context and set up cairo context for selection 
 * drawing 
 *  
 * ex GDK_INVERT is emulated by using sel_color.
 *  
 * Context must be popped with cairo_restore() after usage.
 * 
 * @param sheet  the sheet
 * @param cr     the cairo context
 * 
 * @return 
 */
static void _cairo_save_and_set_sel_color(GtkSheet *sheet, cairo_t *cr)
{
    g_assert(cr);
    //cairo_t *xor_cr = gdk_cairo_create(sheet->sheet_window);

#if GTK_SHEET_DEBUG_EXPOSE > 1
    _debug_cairo_clip_extent(__FUNCTION__, cr);
#endif

    cairo_save(cr);  // _cairo_save_and_set_sel_color,  _cairo_clip_sheet_area

    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
        GTK_WIDGET(sheet));

    _cairo_clip_sheet_area(sheet, cr);

    gtk_style_context_save(sheet_context);

    gtk_style_context_add_class (
        sheet_context, GTK_STYLE_CLASS_VIEW);
    gtk_style_context_add_class (
        sheet_context, GTK_STYLE_CLASS_CELL);

    GdkRGBA sel_color;
    gtk_style_context_get_background_color(
        sheet_context, GTK_STATE_FLAG_SELECTED, &sel_color);
    sel_color.alpha = 0.5;

#if GTK_SHEET_DEBUG_EXPOSE > 1
    g_debug("%s(%d): sel_color %s",
        __FUNCTION__, __LINE__, gdk_rgba_to_string(&sel_color));
#endif

    gtk_style_context_restore(sheet_context);

    gdk_cairo_set_source_rgba(cr, &sel_color);
}

static void 
_get_flash_rectangle(
    GtkSheet *sheet, gint *x, gint *y, gint *width, gint *height)
{
    GtkSheetRange *flash_range = &sheet->clip_range;

    /* get flashing range */
    *x = _gtk_sheet_column_left_xpixel(sheet, flash_range->col0);
    *y = _gtk_sheet_row_top_ypixel(sheet, flash_range->row0);

    *width = _gtk_sheet_column_left_xpixel(sheet, flash_range->coli) - *x
        + COLPTR(sheet, flash_range->coli)->width;
    *height = _gtk_sheet_row_top_ypixel(sheet, flash_range->rowi) - *y
        + ROWPTR(sheet, flash_range->rowi)->height;

    /* shrink flashing rect */
    *x += 0;
    *y += 0;
    *width -= 2;
    *height -= 2;
}

static gint
gtk_sheet_flash(gpointer data)
{
    GtkSheet *sheet = GTK_SHEET(data);
#if 0
    g_debug("%s(%d) called %s %p", 
        __FUNCTION__, __LINE__, G_OBJECT_TYPE_NAME(sheet), sheet);
#endif

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return (TRUE);
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return (TRUE);
    if (!gtk_sheet_range_isvisible(sheet, sheet->clip_range)) return (TRUE);
    if (GTK_SHEET_IN_XDRAG(sheet)) return (TRUE);
    if (GTK_SHEET_IN_YDRAG(sheet)) return (TRUE);

    // FIXME - invalidate_region is missing here?

#if GTK_CHECK_VERSION(3,6,0) == 0
    GDK_THREADS_ENTER();
#endif

    /* get flashing range */
    gint x, y, width, height;
    _get_flash_rectangle(sheet, &x, &y, &width, &height);

    // use swin_cr to blit sheet->bsurf into sheet_window
    cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);
    cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);

    cairo_save(swin_cr);  // _cairo_clip_sheet_area
    _cairo_clip_sheet_area(sheet, swin_cr);

    /* clear background */

    // blit
    cairo_rectangle(swin_cr, x, y, 1, height);  /* left edge */
    cairo_fill(swin_cr);

    // blit
    cairo_rectangle(swin_cr, x, y, width, 1); /* top edge */
    cairo_fill(swin_cr);

    // blit
    cairo_rectangle(swin_cr, x, y + height, width, 1); /* bottom edge */
    cairo_fill(swin_cr);

    // blit
    cairo_rectangle(swin_cr, x + width, y, 1, height); /* right edge */
    cairo_fill(swin_cr);

    /* advancing line anchor */
    sheet->interval = sheet->interval + 1;
    if (sheet->interval >= TIME_INTERVAL) sheet->interval = 0;

    cairo_t *xor_cr = gdk_cairo_create(sheet->sheet_window); /* FIXME, to be removed */
    _cairo_save_and_set_sel_color(sheet, xor_cr);

    double dashes[] = { 4.0, 4.0 };
    cairo_set_dash(xor_cr, dashes, 2, (double) sheet->interval);

    gtk_sheet_draw_flashing_range(sheet, xor_cr);

    cairo_set_dash(xor_cr, dashes, 2, (double) 0);

#if GTK_CHECK_VERSION(3,6,0) == 0
    GDK_THREADS_LEAVE();
#endif

    cairo_restore(xor_cr);  // _cairo_save_and_set_sel_color
    cairo_destroy(xor_cr);  /* FIXME, to be removed */

    cairo_restore(swin_cr); // _cairo_clip_sheet_area
    cairo_destroy(swin_cr);  /* FIXME */

    return (TRUE);
}

static void
gtk_sheet_draw_flashing_range(
    GtkSheet *sheet, cairo_t *xor_cr)
{
    if (!gtk_sheet_range_isvisible(sheet, sheet->clip_range))
	return;

#if 0
    g_debug("%s(%d): called", __FUNCTION__, __LINE__);
#endif

    // obsolete, already done in _cairo_save_and_set_sel_color()
    //_cairo_clip_sheet_area(sheet, xor_cr);

    /* get flashing range */
    gint x, y, width, height;
    _get_flash_rectangle(sheet, &x, &y, &width, &height);

    cairo_set_line_width(xor_cr, 1.0);
    cairo_set_line_cap(xor_cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(xor_cr, CAIRO_LINE_JOIN_MITER);

    // draw dash line
    cairo_rectangle(xor_cr, x + 0.5, y + 0.5, width, height);
    cairo_stroke(xor_cr);

    cairo_set_line_width(xor_cr, 1.0);
    cairo_set_line_cap(xor_cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(xor_cr, CAIRO_LINE_JOIN_MITER);
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
 * Get sheet's ranges in a #GtkSheetRange structure.
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
 * Returns: (transfer none): a #GtkAdjustment
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
 * Returns: (transfer none): a #GtkAdjustment
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
	    G_OBJECT(sheet->vadjustment),
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

	g_signal_connect(G_OBJECT(sheet->vadjustment), "changed",
	    (void *)_vadjustment_changed_handler,
	    (gpointer)sheet);
	g_signal_connect(G_OBJECT(sheet->vadjustment), "value_changed",
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
	    G_OBJECT(sheet->hadjustment),
	    G_SIGNAL_MATCH_DATA,
	    0, 0, NULL, NULL, sheet);
	g_object_unref(G_OBJECT(sheet->hadjustment));
    }

    sheet->hadjustment = adjustment;

    if (sheet->hadjustment)
    {
	g_object_ref(G_OBJECT(sheet->hadjustment));
	g_object_ref_sink(G_OBJECT(sheet->hadjustment));
	g_object_unref(G_OBJECT(sheet->hadjustment));

	g_signal_connect(G_OBJECT(sheet->hadjustment), "changed",
	    (void *)_hadjustment_changed_handler,
	    (gpointer)sheet);
	g_signal_connect(G_OBJECT(sheet->hadjustment), "value_changed",
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

    if (sheet->css_class)
    {
        g_free(sheet->css_class);
        sheet->css_class = NULL;
    }

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
 * this is the #GtkSheet object class "destroy" handler
 * 
 * @param object
 */
static void
gtk_sheet_destroy_handler(GtkWidget *widget)
{
    GtkSheet *sheet;
    GList *children;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    sheet = GTK_SHEET(widget);

    /* destroy the entry */
    if (sheet->sheet_entry && GTK_IS_WIDGET(sheet->sheet_entry))
    {
	gtk_widget_destroy(sheet->sheet_entry);
	sheet->sheet_entry = NULL;
    }

    /* destroy the global sheet button */
    if (sheet->button && GTK_IS_WIDGET(sheet->button))
    {
#if GTK_SHEET_DEBUG_REALIZE > 0
	g_debug("gtk_sheet_destroy: destroying old entry %p",
            sheet->button);
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
	    G_OBJECT(sheet->hadjustment),
	    G_SIGNAL_MATCH_DATA,
	    0, 0, NULL, NULL, sheet);
	g_object_unref(G_OBJECT(sheet->hadjustment));
	sheet->hadjustment = NULL;
    }
    if (sheet->vadjustment)
    {
	g_signal_handlers_disconnect_matched(
	    G_OBJECT(sheet->vadjustment),
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
	    gtk_sheet_remove_handler(
                GTK_CONTAINER(sheet), child->widget);
	children = sheet->children;
    }
    sheet->children = NULL;

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_DESTROYED);

    if (GTK_WIDGET_CLASS(sheet_parent_class)->destroy)
	(*GTK_WIDGET_CLASS(sheet_parent_class)->destroy)(widget);
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
    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    if (GTK_WIDGET_CLASS(sheet_parent_class)->style_set)
	(*GTK_WIDGET_CLASS(sheet_parent_class)->style_set)(widget, previous_style);

    if (gtk_widget_get_realized(widget))
    {
	GtkStyleContext *style_context = gtk_widget_get_style_context(widget);
	gtk_style_context_set_background(style_context, gtk_widget_get_window(widget));
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
    g_debug("%s(%d) called %s %p %s visible %d entry %s %p", 
        __FUNCTION__, __LINE__, 
        G_OBJECT_TYPE_NAME(sheet), sheet,
        gtk_widget_get_name(GTK_WIDGET(sheet)), 
        gtk_widget_get_visible(GTK_WIDGET(sheet)),
        G_OBJECT_TYPE_NAME(sheet->sheet_entry), sheet->sheet_entry);
#endif

    gtk_widget_set_realized_true(GTK_WIDGET(sheet));

    gtk_widget_get_allocation(widget, &allocation);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;

    attributes.visual = gtk_widget_get_visual(widget);

    attributes.event_mask = gtk_widget_get_events(widget);
    attributes.event_mask |= (
			      GDK_EXPOSURE_MASK |
			      GDK_BUTTON_PRESS_MASK |
			      GDK_BUTTON_RELEASE_MASK |
			      GDK_SCROLL_MASK |
			      GDK_KEY_PRESS_MASK |
			      GDK_POINTER_MOTION_MASK |
			      GDK_POINTER_MOTION_HINT_MASK);

    attributes_mask = GDK_WA_X | GDK_WA_Y |
	    GDK_WA_VISUAL | GDK_WA_CURSOR;

    attributes.cursor = gdk_cursor_new(GDK_TOP_LEFT_ARROW);

    /* main window -
       behind sheet_window, row_title_window and column_title_window
       */
    gtk_widget_set_window(widget,
	gdk_window_new(gtk_widget_get_parent_window(widget),
	    &attributes, attributes_mask));

    gdk_window_set_user_data(gtk_widget_get_window(widget), sheet);

    GtkStyleContext *style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_set_background(style_context, gtk_widget_get_window(widget));

    /* column-title window */
    attributes.x = attributes.y = 0;
    if (sheet->row_titles_visible)
	attributes.x = sheet->row_title_area.width;
    attributes.width = sheet->column_title_area.width;
    attributes.height = sheet->column_title_area.height;

    sheet->column_title_window = gdk_window_new(
	gtk_widget_get_window(widget),
	&attributes, attributes_mask);
    gdk_window_set_user_data(sheet->column_title_window, sheet);

    //GtkStyleContext *style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_set_background(
	style_context, sheet->column_title_window);

    /* row-title window */
    attributes.x = attributes.y = 0;
    if (sheet->column_titles_visible)
	attributes.y = sheet->column_title_area.height;
    attributes.width = sheet->row_title_area.width;
    attributes.height = sheet->row_title_area.height;

    sheet->row_title_window = gdk_window_new(
	gtk_widget_get_window(widget),
	&attributes, attributes_mask);
    gdk_window_set_user_data(sheet->row_title_window, sheet);

    //GtkStyleContext *style_context = gtk_widget_get_style_context(widget);
    gtk_style_context_set_background(
	style_context, sheet->row_title_window);

    /* sheet-window */
    attributes.cursor = gdk_cursor_new(GDK_PLUS);

    attributes.x = attributes.y = 0;
    attributes.width = sheet->sheet_window_width;
    attributes.height = sheet->sheet_window_height;

    sheet->sheet_window = gdk_window_new(
	gtk_widget_get_window(widget),
	&attributes, attributes_mask);
    gdk_window_set_user_data(sheet->sheet_window, sheet);

    gdk_window_set_background_rgba(sheet->sheet_window, &color_white);
    gdk_window_show(sheet->sheet_window);

    /* backing surface */
    gtk_sheet_make_bsurf(sheet, 0, 0);  /* use default size */

    /* reparent global sheet entry */
    if (gtk_widget_get_parent(sheet->sheet_entry))
    {
	g_object_ref(sheet->sheet_entry);
	gtk_widget_unparent(sheet->sheet_entry);
    }
    DEBUG_WIDGET_SET_PARENT_WIN(
        sheet->sheet_entry, sheet->sheet_window);
    gtk_widget_set_parent_window(
        sheet->sheet_entry, sheet->sheet_window);

    DEBUG_WIDGET_SET_PARENT(sheet->sheet_entry, sheet);
    gtk_widget_set_parent(sheet->sheet_entry, GTK_WIDGET(sheet));

    /* reparent global sheet button */
    if (sheet->button && gtk_widget_get_parent(sheet->button))
    {
	g_object_ref(sheet->button);
	gtk_widget_unparent(sheet->button);
    }
    DEBUG_WIDGET_SET_PARENT_WIN(
        sheet->button, sheet->sheet_window);
    gtk_widget_set_parent_window(sheet->button, sheet->sheet_window);

    DEBUG_WIDGET_SET_PARENT(sheet->button, sheet);
    gtk_widget_set_parent(sheet->button, GTK_WIDGET(sheet));

/*
    gtk_sheet_activate_cell(
      sheet, sheet->active_cell.row, sheet->active_cell.col);
*/

    if (!sheet->cursor_drag)
	sheet->cursor_drag = gdk_cursor_new(GDK_PLUS);

    if (sheet->column_titles_visible)
	gdk_window_show(sheet->column_title_window);
    if (sheet->row_titles_visible)
	gdk_window_show(sheet->row_title_window);

    _gtk_sheet_row_buttons_size_allocate(sheet);
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

    gint col;
    for (col=0; col<sheet->maxcol; col++)
    {
        g_assert(0 <= col && col <= sheet->maxcol);
        GtkSheetColumn *colobj = COLPTR(sheet, col);

        _gtk_sheet_column_realize(colobj, sheet);
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

    //g_debug("global_button_press_handler called");

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

/** 
 * _global_sheet_button_create: 
 * create global sheet button 
 * 
 * @param sheet  the sheet
 */
static void
_global_sheet_button_create(GtkSheet *sheet)
{
    sheet->button = gtk_button_new_with_label(" ");

    //g_debug("_global_sheet_button_create called");

    g_signal_connect(G_OBJECT(sheet->button), "button-press-event",
	G_CALLBACK(global_button_press_handler),
	(gpointer)sheet);
}

/** 
 * _global_sheet_button_size_allocate: 
 * allocate global sheet button size
 * 
 * set to title area size
 * 
 * @param sheet  the sheet
 */
static void
_global_sheet_button_size_allocate(GtkSheet *sheet)
{
    GtkAllocation allocation;

    if (!sheet->column_titles_visible) return;
    if (!sheet->row_titles_visible) return;

    //g_debug("_global_sheet_button_size_allocate called");

    gtk_widget_get_preferred_size(sheet->button, NULL, NULL);

    allocation.x = 0;
    allocation.y = 0;
    allocation.width = sheet->row_title_area.width;
    allocation.height = sheet->column_title_area.height;

    gtk_widget_size_allocate(sheet->button, &allocation);
}

/** 
 * _gtk_sheet_global_sheet_button_show: 
 * show global sheet button
 * 
 * - allocate size
 * - show widget
 * - map widget
 * 
 * @param sheet  the sheet
 */
void
_gtk_sheet_global_sheet_button_show(GtkSheet *sheet)
{
    if (!sheet->column_titles_visible) return;
    if (!sheet->row_titles_visible) return;

    //g_debug("_global_sheet_button_show called");

    _global_sheet_button_size_allocate(sheet);
    
    gtk_widget_show(sheet->button);

    if (gtk_widget_get_visible(sheet->button)
        && !gtk_widget_get_mapped(sheet->button))
    {
        gtk_widget_map(sheet->button);
    }
}

/** 
 * _gtk_sheet_global_sheet_button_hide: 
 * hide global sheet button
 * 
 * - if visible, hide widget
 * 
 * @param sheet  the sheet
 */
void
_gtk_sheet_global_sheet_button_hide(GtkSheet *sheet)
{
    //g_debug("_global_sheet_button_hide called");

    if (gtk_widget_get_visible(sheet->button))
	gtk_widget_hide(sheet->button);
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

    g_object_unref(G_OBJECT(sheet->cursor_drag));
    sheet->cursor_drag = NULL;

    gdk_window_destroy(sheet->sheet_window);
    sheet->sheet_window = NULL;

    gdk_window_destroy(sheet->column_title_window);
    sheet->column_title_window = NULL;

    gdk_window_destroy(sheet->row_title_window);
    sheet->row_title_window = NULL;

    if (sheet->bsurf)
    {
	cairo_destroy(sheet->bsurf_cr);
	sheet->bsurf_cr = NULL;

	cairo_surface_destroy(sheet->bsurf);
	sheet->bsurf_width = sheet->bsurf_height = 0;
	sheet->bsurf = NULL;
    }

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

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_EXPOSE > 0
    g_debug("%s(%d): called %s %p %s visible %d",
        __FUNCTION__, __LINE__,
        G_OBJECT_TYPE_NAME(sheet), sheet,
        gtk_widget_get_name(GTK_WIDGET(sheet)), 
        gtk_widget_get_visible(GTK_WIDGET(sheet)));
#endif

    if (!gtk_widget_get_mapped(widget))
    {
	gtk_widget_set_mapped_true(GTK_WIDGET(sheet));

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
	    _gtk_sheet_row_buttons_size_allocate(sheet);
	    gdk_window_show(sheet->row_title_window);
	}

#if 0
	/* this will be done by gtk_sheet_activate_cell() below,
           it causes trouble when there is no active cell
           in the sheet, because sheet_entry will start to
           process events */

	if (!gtk_widget_get_mapped (sheet->sheet_entry))
	{
	    gtk_widget_show (sheet->sheet_entry);
	    gtk_widget_map (sheet->sheet_entry);
	}
#endif

        _gtk_sheet_global_sheet_button_show(sheet);

	if ((WdChild = gtk_bin_get_child(GTK_BIN(sheet->button))))
	{
	    if (gtk_widget_get_visible(WdChild)
                && !gtk_widget_get_mapped(WdChild))
	    {
                // FIXME - size allocate missing here?
		gtk_widget_map(WdChild);
	    }
	}

#if GTK_SHEET_DEBUG_EXPOSE > 0
	g_debug("%s(%d): calling _gtk_sheet_recalc_view_range",
            __FUNCTION__, __LINE__);
#endif

	_gtk_sheet_recalc_view_range(sheet);
	_gtk_sheet_range_draw(sheet, NULL, TRUE);

        /* this will cause selection to be reset
           whenever sheet gets unmapped+mapped or hidden+shown
	    */
	gtk_sheet_activate_cell(
            sheet,  sheet->active_cell.row,  sheet->active_cell.col);

        /* map sheet columns is handled by
           _gtk_sheet_position_children() */

        _gtk_sheet_position_children(sheet);
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
    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));

    GtkSheet *sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("%s(%d): called", __FUNCTION__, __LINE__);
#endif

    if (gtk_widget_get_mapped(widget))
    {
	gtk_widget_set_mapped_false(GTK_WIDGET(sheet));

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

        /* unmap sheet columns */
        gint col;
        for (col=0; col<=sheet->maxcol; col++)
        {
            GtkSheetColumn *colobj = COLPTR(sheet, col);
            GtkWidget *colwidget = GTK_WIDGET(colobj);

	    if (gtk_widget_get_visible(colwidget)
                && gtk_widget_get_mapped(colwidget))
	    {
		gtk_widget_unmap(colwidget);
	    }
        }

        GList *children = sheet->children;
	while (children)
	{
            GtkSheetChild *child = children->data;
	    children = children->next;

	    if (gtk_widget_get_visible(child->widget)
                && gtk_widget_get_mapped(child->widget))
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


		    p[0].x = _gtk_sheet_column_left_xpixel(sheet, col) + COLPTR(sheet, col)->width
			- GTK_SHEET_DEFAULT_TM_SIZE - 1;
		    p[0].y = _gtk_sheet_row_top_ypixel(sheet, row) + 1;

		    p[1].x = p[0].x + GTK_SHEET_DEFAULT_TM_SIZE;
		    p[1].y = p[0].y;

		    p[2].x = p[1].x;
		    p[2].y = p[1].y + GTK_SHEET_DEFAULT_TM_SIZE;

		    /* draw cell tooltip marker */
		    gdk_cairo_set_source_rgba(sheet->bsurf_cr, &sheet->tm_color);
		    cairo_move_to(sheet->bsurf_cr, p[0].x - 0.5, p[0].y - 0.5);
		    cairo_line_to(sheet->bsurf_cr, p[1].x - 0.5, p[1].y - 0.5);
		    cairo_line_to(sheet->bsurf_cr, p[2].x - 0.5, p[2].y - 0.5);
		    //cairo_line_to(sheet->bsurf_cr, (double) p[0].x - 0.5, (double) p[0].y - 0.5);
		    cairo_fill(sheet->bsurf_cr);
		}
	    }
	    break;

	case ON_ROW_TITLES_AREA:
	    if (0 <= row && row <= sheet->maxrow)
	    {
		GtkSheetRow *rowp = ROWPTR(sheet, row);

		if (rowp->tooltip_markup || rowp->tooltip_text)
		{
		    GdkPoint p[3];

		    cairo_t *twin_cr = gdk_cairo_create(sheet->row_title_window);

		    gdk_cairo_set_source_rgba(twin_cr, &sheet->tm_color);

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
		    cairo_move_to(twin_cr, (double) p[0].x, (double) p[0].y);
		    cairo_line_to(twin_cr, (double) p[1].x, (double) p[1].y);
		    cairo_line_to(twin_cr, (double) p[2].x, (double) p[2].y);
		    //cairo_line_to(twin_cr, (double) p[0].x, (double) p[0].y);
		    cairo_fill(twin_cr);
		    cairo_destroy(twin_cr);
		}
	    }
	    break;

	case ON_COLUMN_TITLES_AREA:
	    if (0 <= col && col <= sheet->maxcol)
	    {
		GtkSheetColumn *column = COLPTR(sheet, col);

		if (gtk_widget_get_has_tooltip(GTK_WIDGET(column)))
		{
		    GdkPoint p[3];

		    cairo_t *twin_cr = gdk_cairo_create(sheet->column_title_window);

		    gdk_cairo_set_source_rgba(twin_cr, &sheet->tm_color);

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
		    cairo_move_to(twin_cr, (double) p[0].x, (double) p[0].y);
		    cairo_line_to(twin_cr, (double) p[1].x, (double) p[1].y);
		    cairo_line_to(twin_cr, (double) p[2].x, (double) p[2].y);
		    //cairo_line_to(twin_cr, (double) p[0].x, (double) p[0].y);
		    cairo_fill(twin_cr);
		    cairo_destroy(twin_cr);
		}
	    }
	    break;

	default:
	    return;
    }
}

static void
_cell_draw_background(GtkSheet *sheet, gint row, gint col, 
    cairo_t *swin_cr)
{
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

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    area.x = _gtk_sheet_column_left_xpixel(sheet, col);
    area.y = _gtk_sheet_row_top_ypixel(sheet, row);
    area.width = COLPTR(sheet, col)->width;
    area.height = ROWPTR(sheet, row)->height;

#if GTK_SHEET_DEBUG_DRAW_BACKGROUND>0
    g_debug("_cell_draw_background(%d,%d): cellbg x %d y %d w %d h %d %s",
	row, col,
	area.x, area.y, area.width, area.height,
	gdk_rgba_to_string(&attributes.background));
#endif

    if (sheet->deprecated_bg_color_used
        || attributes.deprecated_bg_color_used)
    {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
        g_warning("%s(%d): deprecated_bg_color_used %s %s row %d col %d",
            __FUNCTION__, __LINE__,
            G_OBJECT_TYPE_NAME(sheet),
            gtk_widget_get_name(GTK_WIDGET(sheet)),
            row, col);
#endif

        /* fill cell background */

        gdk_cairo_set_source_rgba(
            sheet->bsurf_cr, &attributes.background);
        gdk_cairo_rectangle(sheet->bsurf_cr, &area);
        cairo_fill(sheet->bsurf_cr);
    }
    else
    {
        GtkStyleContext *sheet_context = gtk_widget_get_style_context(
            GTK_WIDGET(sheet));

        gtk_style_context_save(sheet_context);

        gtk_style_context_add_class(sheet_context,
            GTK_STYLE_CLASS_CELL);

        if (attributes.css_class)
        {
            gtk_style_context_add_class(sheet_context,
                attributes.css_class);
        }

        _cell_style_context_add_position_subclass(sheet, 
            sheet_context, row, col);

        gtk_render_background(sheet_context, sheet->bsurf_cr,
            area.x, area.y, area.width, area.height);

        gtk_render_frame(sheet_context, sheet->bsurf_cr,
            area.x, area.y, area.width, area.height);

        gtk_style_context_restore(sheet_context);
    }

#if 0
    /* debug */
    cairo_t *cr = gdk_cairo_create(sheet->sheet_window);
    _debug_color_rect("here", cr, 10, 20, 1300, 40);
    cairo_destroy(cr);
#endif

    if (sheet->show_grid)
    {
#if GTK_SHEET_DEBUG_DRAW_BACKGROUND>0
#if 1
	g_debug("_cell_draw_background(%d,%d): grid x %d y %d w %d h %d %s",
		row, col,
		area.x, area.y, area.width, area.height,
		gdk_rgba_to_string(&sheet->grid_color));
#endif
#endif

	/* draw grid rectangle */
	cairo_save(sheet->bsurf_cr);  // grid rectangle color

	cairo_set_line_width(sheet->bsurf_cr, 1.0);
	cairo_set_line_cap(sheet->bsurf_cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_line_join(sheet->bsurf_cr, CAIRO_LINE_JOIN_MITER);
	cairo_set_dash(sheet->bsurf_cr, NULL, 0, 0.0);

	gdk_cairo_set_source_rgba(sheet->bsurf_cr, &sheet->grid_color);
	cairo_rectangle(sheet->bsurf_cr, 
	    area.x - 0.5, area.y - 0.5, area.width + 1.0, area.height + 1.0);
	cairo_stroke(sheet->bsurf_cr);

	cairo_restore(sheet->bsurf_cr);  // grid rectangle color
    }

    gtk_sheet_draw_tooltip_marker(sheet, ON_CELL_AREA, row, col);
}

static void
_cell_draw_border(GtkSheet *sheet, gint row, gint col, gint mask)
{
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

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    area.x = _gtk_sheet_column_left_xpixel(sheet, col);
    area.y = _gtk_sheet_row_top_ypixel(sheet, row);
    area.width = COLPTR(sheet, col)->width;
    area.height = sheet->row[row].height;

    cairo_save(sheet->bsurf_cr);  // cell border attributes

    width = attributes.border.width;
    cairo_set_line_width(sheet->bsurf_cr, (double) attributes.border.width);
    cairo_set_line_cap(sheet->bsurf_cr, attributes.border.cap_style);
    cairo_set_line_join(sheet->bsurf_cr, attributes.border.join_style);
    cairo_set_dash(sheet->bsurf_cr, NULL, 0, 0.0);

    gdk_cairo_set_source_rgba(sheet->bsurf_cr, &attributes.border.color);

    if (width > 0)
    {
	double w2 = width / 2.0;

	if (attributes.border.mask & GTK_SHEET_LEFT_BORDER & mask)
        {
            cairo_move_to(sheet->bsurf_cr, 
		area.x - 1.0 + w2, 
		area.y - 0.5);
	    cairo_line_to(sheet->bsurf_cr, 
		area.x - 1.0 + w2, 
		area.y + area.height - 0.5);
	    cairo_stroke(sheet->bsurf_cr);
	}

	if (attributes.border.mask & GTK_SHEET_RIGHT_BORDER & mask)
        {
	    cairo_move_to(sheet->bsurf_cr, 
		area.x + area.width - w2, 
		area.y - 0.5 );
	    cairo_line_to(sheet->bsurf_cr, 
		area.x + area.width - w2,
		area.y + area.height - 0.5);
	    cairo_stroke(sheet->bsurf_cr);
	}

	if (attributes.border.mask & GTK_SHEET_TOP_BORDER & mask)
        {
	    cairo_move_to(sheet->bsurf_cr, 
		area.x - 0.5, 
		area.y - 1.0 + w2);
	    cairo_line_to(sheet->bsurf_cr, 
		area.x + area.width - 0.5,
		area.y - 1.0 + w2);
	    cairo_stroke(sheet->bsurf_cr);
	}

	if (attributes.border.mask & GTK_SHEET_BOTTOM_BORDER & mask)
        {
	    cairo_move_to(sheet->bsurf_cr, 
		area.x - 0.5, 
		area.y + area.height - w2);
	    cairo_line_to(sheet->bsurf_cr, 
		area.x + area.width - 0.5,
		area.y + area.height - w2);
	    cairo_stroke(sheet->bsurf_cr);
	}
    }
    cairo_restore(sheet->bsurf_cr);  // cell border attributes
}


static void
_cell_draw_label(GtkSheet *sheet, gint row, gint col, cairo_t *swin_cr)
{
    GdkRectangle area, clip_area;
    gint i;
    gint text_width, y;
    gint xoffset = 0;
    gint size, sizel, sizer;
    PangoRectangle rect;
    gint ascent, descent, spacing, y_pos;

    g_return_if_fail(sheet != NULL);

    /* bail now if we aren't drawable yet */
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;

    if (row < 0 || row > sheet->maxallocrow) return;
    if (col < 0 || col > sheet->maxalloccol) return;

    if (!sheet->data[row]) return;
    if (!sheet->data[row][col]) return;
    if (!sheet->data[row][col]->text || !sheet->data[row][col]->text[0])
	return;

    if (row < 0 || row > sheet->maxrow) return;
    if (col < 0 || col > sheet->maxcol) return;

    GtkSheetColumn *colptr = COLPTR(sheet, col);

    if (!GTK_SHEET_COLUMN_IS_VISIBLE(colptr)) return;
    if (!GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row))) return;

    gchar *label = sheet->data[row][col]->text;
    gchar *dataformat = gtk_sheet_column_get_format(sheet, col);

    if (dataformat)
	label = gtk_data_format(label, dataformat);

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);

    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
	GTK_WIDGET(sheet));

    gtk_style_context_save(sheet_context);

    gtk_style_context_add_class(
	sheet_context, GTK_STYLE_CLASS_CELL);

    if (attributes.css_class)
    {
	gtk_style_context_add_class(sheet_context,
	    attributes.css_class);
    }

    _cell_style_context_add_position_subclass(sheet, 
        sheet_context, row, col);

    PangoContext *pango_context = gtk_widget_get_pango_context(
	GTK_WIDGET(sheet));

    area.x = _gtk_sheet_column_left_xpixel(sheet, col);
    area.y = _gtk_sheet_row_top_ypixel(sheet, row);
    area.width = colptr->width;
    area.height = ROWPTR(sheet, row)->height;

    clip_area = area;

    PangoLayout *layout = gtk_widget_create_pango_layout(
	GTK_WIDGET(sheet), NULL);

    if (colptr->is_secret) 
    {
        label = _make_secret_text(sheet, label);
    }

    if (sheet->data[row][col]->is_markup)
        pango_layout_set_markup(layout, label, -1);
    else
        pango_layout_set_text(layout, label, -1);

    g_debug("area x/y %d %d w/h %d %d Label\n%s", 
        area.x, area.y, area.width, area.height, label);

    PangoFontDescription *font_desc = NULL;
    gboolean font_desc_needs_free = FALSE;

    if (attributes.deprecated_font_desc_used)
    {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
        g_warning("%s(%d): deprecated_font_desc_used %s %s row %d col %d",
            __FUNCTION__, __LINE__,
            G_OBJECT_TYPE_NAME(sheet),
            gtk_widget_get_name(GTK_WIDGET(sheet)),
            row, col);
#endif

	font_desc = attributes.font_desc;
    }
    else
    {
        gtk_style_context_get(sheet_context, GTK_STATE_FLAG_NORMAL,
	    GTK_STYLE_PROPERTY_FONT, &font_desc, 
	    NULL);
        font_desc_needs_free = TRUE;
    }

    pango_layout_set_font_description(layout, font_desc);

    if (!gtk_sheet_autoresize_columns(sheet))
    {
	switch(colptr->wrap_mode)
	{
	    case GTK_WRAP_NONE: 
		break;

	    case GTK_WRAP_CHAR:
		pango_layout_set_width(
                    layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_CHAR);
		break;

	    case GTK_WRAP_WORD:
		pango_layout_set_width(
                    layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD);
		break;

	    case GTK_WRAP_WORD_CHAR:
		pango_layout_set_width(
                    layout, colptr->width * PANGO_SCALE);
		pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
		break;
	}
    }

    pango_layout_get_pixel_extents(layout, NULL, &rect);

    PangoFontMetrics *metrics = pango_context_get_metrics(
	pango_context,
	font_desc,
	pango_context_get_language(pango_context));

    ascent = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
    descent = pango_font_metrics_get_descent(metrics) / PANGO_SCALE;
    spacing = pango_layout_get_spacing(layout) / PANGO_SCALE;

    pango_font_metrics_unref(metrics);
    if (font_desc_needs_free) pango_font_description_free(font_desc);

    /* Align primarily for locale's ascent/descent */

    /* vertical cell text justification */
    {
	/* column->vjust overrides sheet->vjust */
	GtkSheetVerticalJustification vjust = colptr->vjust;

	if (vjust == GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT)
	    vjust = sheet->vjust;

	switch(vjust)
	{
	    case GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT:
            case GTK_SHEET_VERTICAL_JUSTIFICATION_TOP:
		y_pos = CELLOFFSET;
		break;

            case GTK_SHEET_VERTICAL_JUSTIFICATION_MIDDLE:
                y_pos = (area.height/2) - (rect.height/2);
                break;

            case GTK_SHEET_VERTICAL_JUSTIFICATION_BOTTOM:
                y_pos = area.height - rect.height - CELLOFFSET;
                break;
	}

	y = area.y + y_pos;
    }

    text_width = rect.width;

    switch(attributes.justification)
    {
	case GTK_JUSTIFY_RIGHT:
	    size = area.width;  /* start with col size */
	    area.x += area.width;  /* anchor clip_area at right */

	    /* text extends multiple cells? */
	    if (!gtk_sheet_clip_text(sheet))
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

	    /* text extends multiple cells? */
	    if (!gtk_sheet_clip_text(sheet))
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

	    /* text extends multiple cells? */
	    if (!gtk_sheet_clip_text(sheet))
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

    cairo_save(sheet->bsurf_cr);  // label clip

#if GTK_SHEET_DEBUG_DRAW_LABEL>0
    g_debug("_cell_draw_label(%d,%d): clip (%d, %d, %d, %d) %s",
	    row, col,
	    clip_area.x, clip_area.y, clip_area.width, clip_area.height,
        label);
#endif

    gdk_cairo_rectangle(sheet->bsurf_cr, &clip_area);

#if GTK_SHEET_DEBUG_EXPOSE > 0
#if 0
    gdk_cairo_set_source_rgba(sheet->bsurf_cr, &debug_color);
    cairo_stroke_preserve(sheet->bsurf_cr);
#endif
#endif

    cairo_clip(sheet->bsurf_cr);

    if (attributes.deprecated_fg_color_used)
    {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
        g_warning("%s(%d): deprecated_fg_color_used %s %s row %d col %d",
            __FUNCTION__, __LINE__,
            G_OBJECT_TYPE_NAME(sheet),
            gtk_widget_get_name(GTK_WIDGET(sheet)),
            row, col);
#endif

        gdk_cairo_set_source_rgba(
            sheet->bsurf_cr, &attributes.foreground);

#if GTK_SHEET_DEBUG_DRAW_LABEL>0
        // debug code
        gdk_cairo_rectangle(sheet->bsurf_cr, &area);
        cairo_move_to(sheet->bsurf_cr, area.x, area.y);
        cairo_line_to(sheet->bsurf_cr,
            area.x + area.width, area.y + area.height);
        cairo_stroke(sheet->bsurf_cr);
#endif

#if GTK_SHEET_DEBUG_DRAW_LABEL>0
        g_debug("_cell_draw_label(%d,%d): x %d y %d rgba %s",
            row, col,
            area.x + xoffset + CELLOFFSET, y,
            gdk_rgba_to_string (&attributes.foreground));
#endif

        cairo_move_to(sheet->bsurf_cr,
            area.x + xoffset + CELLOFFSET, y);
        pango_cairo_show_layout(sheet->bsurf_cr, layout);
    }
    else
    {
        gtk_render_layout(sheet_context,
            sheet->bsurf_cr,
	    area.x + xoffset + CELLOFFSET, y, layout);
    }

    cairo_restore(sheet->bsurf_cr);  // label clip

    g_object_unref(G_OBJECT(layout));    /* dispose pango layout */

    if (colptr->is_secret)
    {
        g_free(label);
    }

    gtk_style_context_restore(sheet_context);

    // 19.01.20/fp - invalidate seams not to be necessary here
    //gdk_window_invalidate_rect(sheet->sheet_window, &clip_area, FALSE);
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
    g_debug("%s(%d): called drawable %d realized %d mapped %d",
        __FUNCTION__, __LINE__,
        gtk_widget_is_drawable(GTK_WIDGET(sheet)),
        gtk_widget_get_realized(GTK_WIDGET(sheet)),
        gtk_widget_get_mapped(GTK_WIDGET(sheet)) );
#endif

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_mapped(GTK_WIDGET(sheet))) return;

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
    g_debug("%s(%d): row %d - %d col %d - %d",
        __FUNCTION__, __LINE__,
        drawing_range.row0, drawing_range.rowi, 
        drawing_range.col0, drawing_range.coli);
#endif

    if (drawing_range.row0 > drawing_range.rowi)
	return;
    if (drawing_range.col0 > drawing_range.coli)
	return;

    /* beware to free cairo context upon return */
    cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);

/*  
   gdk_draw_rectangle (sheet->bsurf,
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
	    cairo_save(sheet->bsurf_cr);  // bgcolor

	    gdk_cairo_set_source_rgba(sheet->bsurf_cr, &sheet->bg_color);
	    gdk_cairo_rectangle(sheet->bsurf_cr, &area);
	    cairo_fill(sheet->bsurf_cr);

	    // double screen update?
	    //cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);
	    //gdk_cairo_rectangle(swin_cr, &area);
	    //cairo_fill(swin_cr);

	    cairo_restore(sheet->bsurf_cr);  // bgcolor
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
	    cairo_save(sheet->bsurf_cr);  // bgcolor

	    gdk_cairo_set_source_rgba(sheet->bsurf_cr, &sheet->bg_color);
	    gdk_cairo_rectangle(sheet->bsurf_cr, &area);
	    cairo_fill(sheet->bsurf_cr);

	    // double screen update?
	    //cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);
	    //gdk_cairo_rectangle(swin_cr, &area);
	    //cairo_fill(swin_cr);

	    cairo_restore(sheet->bsurf_cr);  // bgcolor
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
	    _cell_draw_background(sheet, row, col, swin_cr);
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
		_cell_draw_label(sheet, row, col, swin_cr);
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
    gtk_sheet_draw_backing_pixmap(sheet, drawing_range, NULL);

    if (sheet->state != GTK_SHEET_NORMAL &&
	gtk_sheet_range_isvisible(sheet, sheet->range))
    {
	gtk_sheet_range_draw_selection(sheet, drawing_range, NULL);
    }

    if (activate_active_cell
        && sheet->state == GTK_SHEET_NORMAL
        && sheet->active_cell.row >= drawing_range.row0
        && sheet->active_cell.row <= drawing_range.rowi
        && sheet->active_cell.col >= drawing_range.col0
        && sheet->active_cell.col <= drawing_range.coli)
    {
	gtk_sheet_show_active_cell(sheet);
    }

    cairo_destroy(swin_cr);
}

/** 
 * _selection_border_add_gap_for_bg: 
 * add a gap between selection border and background.
 * 
 * @param row    row
 * @param col    column
 * @param range  selection range
 * @param x      selection range pixel dimensions
 * @param y      selection range pixel dimensions
 * @param width  selection range pixel dimensions
 * @param height selection range pixel dimensions
 */
static void _selection_border_add_gap_for_bg(
    gint row, gint col, 
    GtkSheetRange *range,
    gint *x, gint *y, gint *width, gint *height)
{
    gdouble dlt;  /* consider borders */
    dlt = -MIN(selection_border_offset, 0);
    dlt += selection_bb_offset + 1.5;

    if (col == range->col0) /* left */
    {
        *x += dlt;
        *width -= dlt - 1.0;
    }
    if (col == range->coli) /* right */
    {
        *width -= dlt;
    }

    if (row == range->row0) /* top */
    {
        *y += dlt;
        *height -= dlt - 1.0;
    }
    if (row == range->rowi) /* bottom */
    {
        *height -= dlt - 0.5;
    }
}

/** 
 * gtk_sheet_range_draw_selection: 
 * draw sheet selection including border and corners
 * 
 * @param sheet  the sheet
 * @param range  the selected range
 * @param cr     cairo drawing context or NULL
 */
static void
gtk_sheet_range_draw_selection(
    GtkSheet *sheet, 
    GtkSheetRange range,
    cairo_t *cr)
{
    gint x, y, width, height;
    gint row, col;

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

    cairo_t *my_cr = cr;

    if (!my_cr)
    {
        // when not in expose - create my own cr and destroy at end
        my_cr = gdk_cairo_create(sheet->sheet_window);
    }

#if GTK_SHEET_DEBUG_EXPOSE > 1
    _debug_cairo_clip_extent("gtk_sheet_range_draw_selection", my_cr);
#endif

    range.col0 = MAX(sheet->range.col0, range.col0);
    range.coli = MIN(sheet->range.coli, range.coli);
    range.row0 = MAX(sheet->range.row0, range.row0);
    range.rowi = MIN(sheet->range.rowi, range.rowi);

    range.col0 = MAX(range.col0, MIN_VIEW_COLUMN(sheet));
    range.coli = MIN(range.coli, MAX_VIEW_COLUMN(sheet));
    range.row0 = MAX(range.row0, MIN_VIEW_ROW(sheet));
    range.rowi = MIN(range.rowi, MAX_VIEW_ROW(sheet));

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("%s(%d): range r %d-%d c %d-%d",
        __FUNCTION__, __LINE__,
	range.row0, range.rowi, range.col0, range.coli);
#endif

    _cairo_save_and_set_sel_color(sheet, my_cr);

    for (row = range.row0; row <= range.rowi; row++)
    {
	if (row > sheet->maxrow)
	    break;

	for (col = range.col0; col <= range.coli; col++)
	{
	    if (col > sheet->maxcol)
		break;

	    if (gtk_sheet_cell_get_state(sheet, row, col) == GTK_STATE_SELECTED
                && GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))
                && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	    {
		row_button_set(sheet, row);
		_gtk_sheet_column_button_set(sheet, col);

		x = _gtk_sheet_column_left_xpixel(sheet, col);
		y = _gtk_sheet_row_top_ypixel(sheet, row);
		width = COLPTR(sheet, col)->width;
		height = ROWPTR(sheet, row)->height;

                _selection_border_add_gap_for_bg(
                    row, col, &range, &x, &y, &width, &height);

                if (row != sheet->active_cell.row || col != sheet->active_cell.col)
		{
		    // xor
		    cairo_rectangle(my_cr, x, y, width, height);
		    cairo_fill(my_cr);
		}
	    }
	}
    }

    cairo_restore(my_cr);  // _cairo_save_and_set_sel_color
    if (!cr) cairo_destroy(my_cr);

    gtk_sheet_draw_border(sheet, sheet->range, cr);
}

static void
gtk_sheet_draw_backing_pixmap(
    GtkSheet *sheet, 
    GtkSheetRange range, 
    cairo_t *cr)
{
    gint x, y, width, height;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    range.row0 = MAX(range.row0, 0);
    range.col0 = MAX(range.col0, 0);
    range.rowi = MIN(range.rowi, sheet->maxrow);
    range.coli = MIN(range.coli, sheet->maxcol);

    g_assert(range.row0 >= 0);
    g_assert(range.rowi <= sheet->maxrow);
    g_assert(range.col0 >= 0);
    g_assert(range.coli <= sheet->maxcol);

    if (range.rowi < range.row0) return;
    if (range.coli < range.col0) return;

    cairo_t *my_cr = cr;

    if (!my_cr)
    {
        // when not in expose - create my own cr and destroy at end
        my_cr = gdk_cairo_create(sheet->sheet_window);
    }

#if GTK_SHEET_DEBUG_EXPOSE > 0
    _debug_cairo_clip_extent("gtk_sheet_draw_backing_pixmap", my_cr);
#endif

    x = _gtk_sheet_column_left_xpixel(sheet, range.col0);
    y = _gtk_sheet_row_top_ypixel(sheet, range.row0);

    width = _gtk_sheet_column_left_xpixel(sheet, range.coli) - x;
    if (0 <= range.coli && range.coli <= sheet->maxcol
        && GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, range.coli)))
	width += COLPTR(sheet, range.coli)->width;

    height = _gtk_sheet_row_top_ypixel(sheet, range.rowi) - y;
    if (0 <= range.rowi && range.rowi <= sheet->maxrow
        && GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, range.rowi)))
	height += sheet->row[range.rowi].height;

    double d;  /* consider borders and decorations */
    d = 1 + selection_border_offset + selection_border_width;
    x -= d;
    y -= d;
    width += d * 2.0;
    height += d * 2.0;

#if GTK_SHEET_DEBUG_EXPOSE > 0
    g_debug("gtk_sheet_draw_backing_pixmap: x %d y %d w %d h %d",
	x, y, width, height);
#endif

    cairo_save(my_cr); // _cairo_clip_sheet_area
    _cairo_clip_sheet_area(sheet, my_cr);

    cairo_set_source_surface(my_cr, sheet->bsurf, 0, 0);
    cairo_rectangle(my_cr, x, y, width, height);
    cairo_fill(my_cr);

    cairo_restore(my_cr); // _cairo_clip_sheet_area

#if 0
    _debug_color_rect(
        "gtk_sheet_draw_backing_pixmap", cr, x, y, width, height);
#endif

    if (!cr)
    {
        _gtk_sheet_invalidate_region(sheet, x, y, width, height);
        cairo_destroy(my_cr);
    }
}

static inline void
gtk_sheet_cell_init(GtkSheetCell *cell)
{
    cell->extent.x = cell->extent.y = 0;
    cell->extent.width = cell->extent.height = 0;

    cell->row = cell->col = -1;

    cell->attributes = NULL;
    cell->text = cell->link = NULL;
    cell->is_markup = FALSE;

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

	if (G_IS_OBJECT(sheet) && G_OBJECT(sheet)->ref_count > 0)
	    g_signal_emit(G_OBJECT(sheet), sheet_signals[CLEAR_CELL], 0,
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
	    pango_font_description_free(attributes->font_desc);
	    attributes->font_desc = NULL;
	}

        if (attributes->css_class)
        {
            g_free(attributes->css_class);
            cell->attributes->css_class = NULL;
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

#if GTK_SHEET_DEBUG_MARKUP>1
static void _debug_show_tag(GtkTextTag *tag, gpointer data)
{
    g_debug("Tag %p", tag);

    gboolean val;
    g_object_get(tag, "background-full-height-set", &val, NULL);
    if (val) g_debug("background-full-height-set");
    g_object_get(tag, "background-set", &val, NULL);
    if (val) g_debug("background-set");
    g_object_get(tag, "editable-set", &val, NULL);
    if (val) g_debug("editable-set");
    g_object_get(tag, "fallback-set", &val, NULL);
    if (val) g_debug("fallback-set");
    g_object_get(tag, "family-set", &val, NULL);
    if (val) g_debug("family-set");
    g_object_get(tag, "font-features-set", &val, NULL);
    if (val) g_debug("font-features-set");
    g_object_get(tag, "foreground-set", &val, NULL);
    if (val) g_debug("foreground-set");
    g_object_get(tag, "indent-set", &val, NULL);
    if (val) g_debug("indent-set");
    g_object_get(tag, "invisible-set", &val, NULL);
    if (val) g_debug("invisible-set");
    g_object_get(tag, "justification-set", &val, NULL);
    if (val) g_debug("justification-set");
    g_object_get(tag, "language-set", &val, NULL);
    if (val) g_debug("language-set");
    g_object_get(tag, "left-margin-set", &val, NULL);
    if (val) g_debug("left-margin-set");
    g_object_get(tag, "letter-spacing-set", &val, NULL);
    if (val) g_debug("letter-spacing-set");
    g_object_get(tag, "paragraph-background-set", &val, NULL);
    if (val) g_debug("paragraph-background-set");
    g_object_get(tag, "pixels-above-lines-set", &val, NULL);
    if (val) g_debug("pixels-above-lines-set");
    g_object_get(tag, "pixels-below-lines-set", &val, NULL);
    if (val) g_debug("pixels-below-lines-set");
    g_object_get(tag, "pixels-inside-wrap-set", &val, NULL);
    if (val) g_debug("pixels-inside-wrap-set");
    g_object_get(tag, "right-margin-set", &val, NULL);
    if (val) g_debug("right-margin-set");
    g_object_get(tag, "rise-set", &val, NULL);
    if (val) g_debug("rise-set");
    g_object_get(tag, "scale-set", &val, NULL);
    if (val) g_debug("scale-set");
    g_object_get(tag, "size-set", &val, NULL);
    if (val) g_debug("size-set");
    g_object_get(tag, "stretch-set", &val, NULL);
    if (val) g_debug("stretch-set");
    g_object_get(tag, "strikethrough-rgba-set", &val, NULL);
    if (val) g_debug("strikethrough-rgba-set");
    g_object_get(tag, "strikethrough-set", &val, NULL);
    if (val) g_debug("strikethrough-set");
    g_object_get(tag, "style-set", &val, NULL);
    if (val) g_debug("style-set");
    g_object_get(tag, "tabs-set", &val, NULL);
    if (val) g_debug("tabs-set");
    g_object_get(tag, "underline-rgba-set", &val, NULL);
    if (val) g_debug("underline-rgba-set");
    g_object_get(tag, "underline-set", &val, NULL);
    if (val) g_debug("underline-set");
    g_object_get(tag, "variant-set", &val, NULL);
    if (val) g_debug("variant-set");
    g_object_get(tag, "weight-set", &val, NULL);
    if (val) g_debug("weight-set");
    g_object_get(tag, "wrap-mode-set", &val, NULL);
    if (val) g_debug("wrap-mode-set");
}
#endif

void _collect_tags(GtkTextTag *tag, gpointer data)
{
    GSList **l = data;
    *l = g_slist_append (*l, tag);
}

void _tag_table_remove(gpointer data, gpointer user_data)
{
    GtkTextTag *tag = data;
    GtkTextTagTable *tag_table = user_data;
    gtk_text_tag_table_remove(tag_table, tag);
}

/*
 * _gtk_text_buffer_clear_tag_table:
 * 
 * remove all tags from text buffer tag table.
 * 
 * @param buffer the #GtkTextBuffer
 */
static void _gtk_text_buffer_clear_tag_table(GtkTextBuffer *buffer)
{
    //g_debug("%s(%d)", __FUNCTION__, __LINE__);

    GtkTextTagTable *tag_table
        = gtk_text_buffer_get_tag_table(buffer);

    GSList *tmp_list = NULL;

    gtk_text_tag_table_foreach(
        tag_table, _collect_tags, &tmp_list);

    g_slist_foreach(tmp_list, _tag_table_remove, tag_table);
    g_slist_free(tmp_list);
}

/**
 * _gtk_sheet_set_entry_text_internal:
 * @sheet: a #GtkSheet 
 * @text: the text to be set or NULL 
 * @is_markup: pass TRUE when text contains pango markup
 *
 * Set the text in the sheet_entry (and active cell).
 *  
 * This function is mainly used to synchronize the text of a 
 * second entry with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface. 
 * 
 * Beware that you must have a sheet entry capable of handling
 * pango markup, when using markup.
 */
static void _gtk_sheet_set_entry_text_internal(
    GtkSheet *sheet, const gchar *text, gboolean is_markup)
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
	gtk_editable_insert_text(GTK_EDITABLE(entry),
            text, -1, &position);
    }
    else if (GTK_IS_DATA_TEXT_VIEW(entry) 
             || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(entry));

        if (is_markup)
        {
            GtkTextIter start_iter, end_iter;

            _gtk_text_buffer_clear_tag_table(buffer);

#if GTK_SHEET_DEBUG_MARKUP>1
            GtkTextTagTable *tag_table
                = gtk_text_buffer_get_tag_table(buffer);

            gtk_text_buffer_get_bounds(buffer,
                &start_iter, &end_iter);

            g_debug(
                "%s(%d): tag_table after clear %d %d cell_is_markup %d next tag %d", 
                __FUNCTION__, __LINE__, 
                (tag_table != NULL),
                gtk_text_tag_table_get_size(tag_table),
                is_markup,
                gtk_text_iter_forward_to_tag_toggle(
                    &start_iter, NULL));

            gtk_text_tag_table_foreach (
                tag_table, _debug_show_tag , NULL);
#endif
            gtk_text_buffer_get_bounds(buffer,
                &start_iter, &end_iter);

            gtk_text_buffer_delete(buffer,
                &start_iter, &end_iter);

            gtk_text_buffer_insert_markup(buffer,
                &start_iter, text, -1);
        }
        else
        {
            gtk_text_buffer_set_text(buffer, text, -1);

            /* FIXME - testing of growing tag table
            GtkTextIter start_iter, end_iter;

            g_debug("clearing tags");

            gtk_text_buffer_get_bounds(buffer,
                &start_iter, &end_iter);
            gtk_text_buffer_remove_all_tags(buffer,
                &start_iter, &end_iter);
            */
        }

        gtk_text_buffer_set_modified (buffer, FALSE);

	GtkTextIter iter;
	gtk_text_buffer_get_start_iter(buffer, &iter);
	gtk_text_buffer_place_cursor(buffer, &iter);
    }
    else
    {
	g_warning("%s(%d): no GTK_EDITABLE, don't know how to set the text.",
            __FUNCTION__, __LINE__);
    }
}

/**
 * _gtk_sheet_set_cell_internal:
 * @sheet: a #GtkSheet.
 * @row: row_number
 * @col: column number
 * @justification: a #GtkJustification :GTK_JUSTIFY_LEFT, RIGHT, CENTER
 * @text: cell text 
 * @is_markup: pass TRUE when text contains pango markup
 *
 * Set cell contents and allocate memory if needed.
 */
static void
_gtk_sheet_set_cell_internal(GtkSheet *sheet,
    gint row, gint col,
    const gchar *text,
    const gboolean is_markup)
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

    if (cell->text)
    {
	g_free(cell->text);
	cell->text = NULL;
    }

    gchar *dataformat = NULL;

    if (text)
    {
	dataformat = gtk_sheet_column_get_format(sheet, col);

	if (dataformat)
	    text = gtk_data_format_remove(text, dataformat);

#if GTK_SHEET_DEBUG_SET_CELL_TEXT > 0
    	g_debug("%s(%d): %p r %d c %d ar %d ac %d <%s> markup %d", 
            __FUNCTION__, __LINE__,
	    sheet, row, col, 
            sheet->active_cell.row, sheet->active_cell.col, 
            text, is_markup);
#endif

	cell->text = g_strdup(text);
    }

    cell->is_markup = is_markup;

#if 0 && GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st2: %0.6f", g_timer_elapsed(tm, NULL));
#endif

    {
        gchar *label = cell->text;

        if (dataformat)
            label = gtk_data_format(label, dataformat);

        _gtk_sheet_update_extent(sheet, cell, row, col, label);
    }

#if GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st3: %0.6f", g_timer_elapsed(tm, NULL));
#endif
    
    if (attributes.is_visible)
    {
	gboolean need_draw = TRUE;
        
        /* PR#104553
           if sheet entry editor is active on the cell
           being modified, we need to update it's contents
	   */
	if (row == sheet->active_cell.row && col == sheet->active_cell.col)
	{
#if GTK_SHEET_DEBUG_SET_CELL_TEXT > 0
	    g_debug(
                "%s(%d): %p: update sheet entry",
                __FUNCTION__, __LINE__, sheet);
#endif
            _gtk_sheet_set_entry_text_internal(sheet,
                text, is_markup);
	}

	if (gtk_sheet_autoresize(sheet))  /* handle immediate resize */
	{
	    if (cell->text && cell->text[0])
	    {
		if (gtk_sheet_autoresize_columns(sheet)
                    && _gtk_sheet_autoresize_column_internal(sheet, col))
                {
                        need_draw = FALSE;
                }

		if (gtk_sheet_autoresize_rows(sheet)
                    && _gtk_sheet_autoresize_row_internal(sheet, row))
                {
                        need_draw = FALSE;
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

    g_signal_emit(G_OBJECT(sheet), sheet_signals[CHANGED], 0, row, col);

#if GTK_SHEET_DEBUG_SET_CELL_TIMER > 0
    g_debug("st9: %0.6f", g_timer_elapsed(tm, NULL));
    g_timer_destroy(tm);
#endif
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
gtk_sheet_set_cell_text(
    GtkSheet *sheet, gint row, gint col, const gchar *text)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

    _gtk_sheet_set_cell_internal(sheet, row, col, text, FALSE);
}

/**
 * gtk_sheet_set_cell_markup:
 * @sheet: a #GtkSheet.
 * @row: row_number
 * @col: column number
 * @markup: text with pango markup
 *  
 * Beware that when using markup, you need to set 
 * #GtkDataTextView, #GtkTextView or another widget supporting 
 * pango markup as sheet entry editor. 
 *  
 * Using markup will only work with readonly cells. Reading back
 * modified text out of #GtkDataTextView and #GtkTextView 
 * widgets will not return any pango markup in the returned 
 * text.  
 *  
 * Set cell contents and allocate memory if needed. No 
 * justifcation is made. attributes and links remain unchanged.
 */
void
gtk_sheet_set_cell_markup(
    GtkSheet *sheet, gint row, gint col, const gchar *markup)
{
    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (col > sheet->maxcol || row > sheet->maxrow)
	return;
    if (col < 0 || row < 0)
	return;

    _gtk_sheet_set_cell_internal(sheet, row, col, markup, TRUE);
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
gtk_sheet_set_cell(GtkSheet *sheet,
    gint row, gint col,
    GtkJustification justification,
    const gchar *text)
{
    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, row, col, &attributes);
    attributes.justification = justification;
    g_debug("gtk_sheet_set_cell");
    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
    
    _gtk_sheet_set_cell_internal(sheet, row, col, text, FALSE);
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

	if (G_IS_OBJECT(sheet) && G_OBJECT(sheet)->ref_count > 0)
	    g_signal_emit(G_OBJECT(sheet),
                sheet_signals[CLEAR_CELL], 0, row, column);
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

#if GTK_SHEET_DEBUG_ALLOCATION > 1
	g_debug("gtk_sheet_real_cell_clear: freeing %d %d",
            row, column); 
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
    gint row, col;
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

    for (row = clear.row0; row <= clear.rowi; row++)
    {
	for (col = clear.col0; col <= clear.coli; col++)
	{
	    gtk_sheet_real_cell_clear(sheet, row, col, delete);
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
gtk_sheet_cell_get_text(GtkSheet *sheet, gint row, gint col)
{
    g_return_val_if_fail(sheet != NULL, NULL);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), NULL);

    if (col > sheet->maxcol || row > sheet->maxrow) return (NULL);
    if (col < 0 || row < 0) return (NULL);

    if (row > sheet->maxallocrow || col > sheet->maxalloccol)
	return (NULL);

    if (!sheet->data[row]) return (NULL);
    if (!sheet->data[row][col]) return (NULL);
    if (!sheet->data[row][col]->text) return (NULL);
    if (!sheet->data[row][col]->text[0]) return (NULL);

    return (sheet->data[row][col]->text);
}

/**
 * gtk_sheet_cell_is_markup:
 * @sheet: a #GtkSheet
 * @row: row number
 * @col: column number
 *
 * Get cell markup flag, set when using
 * #gtk_sheet_set_cell_markup. 
 * 
 * Cells which are not initialized with any content,
 * have markup flag FALSE.
 */
gboolean
gtk_sheet_cell_is_markup(GtkSheet *sheet, gint row, gint col)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (col > sheet->maxcol || row > sheet->maxrow) return (FALSE);
    if (col < 0 || row < 0) return (FALSE);
    if (row > sheet->maxallocrow || col > sheet->maxalloccol)
	return (FALSE);
    if (!sheet->data[row]) return (FALSE);
    if (!sheet->data[row][col]) return (FALSE);

    return (sheet->data[row][col]->is_markup);
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
 * Returns: (transfer none): pointer linked to the cell
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

#if GTK_SHEET_DEBUG_PIXEL_INFO > 0
    g_debug("gtk_sheet_get_pixel_info: window %p", window);
#endif

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
    g_debug("gtk_sheet_get_pixel_info: x %d y %d row %d col %d",
            x, y, trow, tcol);
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
    g_debug("%s(%d): row %d col %d inSel %s inResize %s inDrag %s",
        __FUNCTION__, __LINE__, 
	row, col,
	GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
	);
#endif

    if (!gtk_widget_get_can_focus(GTK_WIDGET(sheet)))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("%s(%d): row %d col %d abort: sheet, can-focus false",
            __FUNCTION__, __LINE__, row, col);
#endif
	return (FALSE);
    }

    if (col >= 0 && !gtk_widget_get_can_focus(
        GTK_WIDGET(COLPTR(sheet, col))))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("%s(%d): row %d col %d abort: sheet column, can-focus false",
            __FUNCTION__, __LINE__, row, col);
#endif
	return (FALSE);
    }

    if (col >= 0 && !GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)))
    {
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("%s(%d): row %d col %d abort: sheet column, visible false",
            __FUNCTION__, __LINE__, row, col);
#endif
	return (FALSE);
    }

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
#if 0
	gint old_row = sheet->active_cell.row;
	gint old_col = sheet->active_cell.col;
#endif

	if (!_gtk_sheet_deactivate_cell(sheet))
	{
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	    g_debug("%s(%d): abort: deactivation false",
                __FUNCTION__, __LINE__);
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

    SET_ACTIVE_CELL(row, col);

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

/**
 * _gtk_sheet_get_entry_text_internal:
 * @sheet: a #GtkSheet
 * @row: sheet row where entry is attached
 * @col: sheet column where entry is attached
 * @is_markup: set to TRUE if text has pango markup
 * @is_modified: set to TRUE if text buffer was modified
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
 * 
 * is_modified is only evaluated for entry widgets that support
 * a GtkTextBuffer. it returns TRUE for all other entry widgets. 
 * 
 * is_markup is an attempt to determine whether markup was set.
 * It is not really useable because GtkTextView doesn't fully
 * support returning pango markup at time of implementation.
 */
static gchar *_gtk_sheet_get_entry_text_internal(
    GtkSheet *sheet,
    gint row, gint col,
    gboolean *is_markup,
    gboolean *is_modified)
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
        *is_markup = FALSE;
        *is_modified = TRUE;
    }
    else if (GTK_IS_DATA_TEXT_VIEW(entry)
             || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(entry));

        gboolean cell_is_markup = gtk_sheet_cell_is_markup(
            sheet, row, col);

	GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);

        if (cell_is_markup)
        {
            gsize length;
            text = serialize_pango_markup(
                buffer, buffer, &start, &end, &length, NULL);

#if GTK_SHEET_DEBUG_MARKUP>0
            g_debug("%s(%d): serialized, markup r/c %d/%d |%s|",
                __FUNCTION__, __LINE__, row, col, text);
#endif
        }
        else
        {
            text = gtk_text_buffer_get_text(
                buffer, &start, &end, TRUE);

#if GTK_SHEET_DEBUG_MARKUP>0
            g_debug("%s(%d): text, no markup r/c %d/%d |%s|",
                __FUNCTION__, __LINE__, row, col, text);
#endif
        }

#if GTK_SHEET_DEBUG_MARKUP>1
        GtkTextTagTable *tag_table = gtk_text_buffer_get_tag_table(
            buffer);

        g_debug("tag_table %d %d cell_is_markup %d", 
            (tag_table != NULL),
            gtk_text_tag_table_get_size(tag_table),
            cell_is_markup);

        gtk_text_tag_table_foreach(
            tag_table, _debug_show_tag , NULL);
#endif

        *is_markup = cell_is_markup;
        *is_modified = gtk_text_buffer_get_modified(buffer);
    }
    else
    {
	g_warning("%s(%d): no GTK_EDITABLE, don't know how to get the text.",
            __FUNCTION__, __LINE__);
    }
    return (text);
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
    g_return_if_fail(data != NULL);
    g_return_if_fail(GTK_IS_SHEET(data));

#if 0
    g_debug("%s(%d): called", __FUNCTION__, __LINE__);
#endif

    GtkSheet *sheet = GTK_SHEET(data);

    if (!gtk_widget_get_visible(gtk_sheet_get_entry_widget(sheet)))
	return;

    if (sheet->state != GTK_SHEET_NORMAL)
	return;

    gint old_row = sheet->active_cell.row;
    gint old_col = sheet->active_cell.col;

    if (old_row < 0 || old_col < 0) return;

    /* why do we temporarily reset here? */
    SET_ACTIVE_CELL(-1, -1);

    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    gboolean is_markup, is_modified;
    gchar *text = _gtk_sheet_get_entry_text_internal(sheet,
        old_row, old_col,
        &is_markup, &is_modified);

    GtkSheetCellAttr attributes;
    gtk_sheet_get_attributes(sheet, old_row, old_col, &attributes);

    GtkSheetColumn *colptr = COLPTR(sheet, old_col);

    gboolean editable = !(gtk_sheet_locked(sheet)
	|| !attributes.is_editable
	|| colptr->is_readonly);

#if GTK_SHEET_DEBUG_ENTRY>0
    g_debug(
        "%s(%d): r/c %d/%d markup %d modified %d editable %d <%s> ", 
        __FUNCTION__, __LINE__, old_row, old_col,
        is_markup, is_modified, editable,
        text);
#endif

    // Beware: Ctrl-X (cut) doesn't set the text buffer 
    // if (editable && is_modified)

    if (editable)
    {
        _gtk_sheet_set_cell_internal(
            sheet, old_row, old_col,
            text, is_markup);
    }

    g_free(text);

    if (sheet->freeze_count == 0)
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

    SET_ACTIVE_CELL(old_row, old_col);
}

/**
 * _gtk_sheet_redraw_pending:
 * 
 * process a pending redraw when sheet flag GTK_SHEET_IN_REDRAW_PENDING is set.
 * 
 * @param sheet  the #GtkSheet
 */
void
_gtk_sheet_redraw_pending(GtkSheet *sheet)
{
#if 0
    g_debug("%s(%d) pending %d", 
        __FUNCTION__, __LINE__, 
        GTK_SHEET_REDRAW_PENDING(sheet));
#endif

    if (GTK_SHEET_REDRAW_PENDING(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(
            sheet, GTK_SHEET_IN_REDRAW_PENDING);
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
}

/** 
 * _gtk_sheet_deactivate_cell: 
 * deactivate active cell 
 * 
 * if there is an active cell:
 * - disconnect gtk_sheet_entry_changed_handler
 * - hide sheet entry
 * - emit DEACTIVATE signal
 * - opt. initiate pending redraw
 * 
 * @param sheet  the sheet
 * 
 * @return TRUE on success
 */
gboolean
_gtk_sheet_deactivate_cell(GtkSheet *sheet)
{
    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    gint old_row = sheet->active_cell.row;
    gint old_col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("%s(%d): called, row %d col %d",
        __FUNCTION__, __LINE__, old_row, old_col);
#endif

    _gtk_sheet_redraw_pending(sheet);

    if (old_row < 0 || old_row > sheet->maxrow) return (TRUE);
    if (old_col < 0 || old_col > sheet->maxcol) return (TRUE);

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return (FALSE);
    if (sheet->state != GTK_SHEET_NORMAL) return (FALSE);

    gtk_sheet_entry_signal_disconnect_by_func(sheet,
	G_CALLBACK(gtk_sheet_entry_changed_handler));

    _gtk_sheet_hide_active_cell(sheet);

    /* reset before signal emission, to prevent recursion */
    SET_ACTIVE_CELL(-1, -1);

    /* beware: DEACTIVATE handler may call
       gtk_sheet_set_active_cell()
       */

    gboolean veto = TRUE;
    _gtksheet_signal_emit(G_OBJECT(sheet), sheet_signals[DEACTIVATE],
	old_row, old_col, &veto);

    if (!veto)
    {
        //FIXME - forgot to restore gtk_sheet_entry_changed_handler?
        SET_ACTIVE_CELL(old_row, old_col);
	return (FALSE);
    }

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("%s(%d): done row %d col %d",
        __FUNCTION__, __LINE__, old_row, old_col);
#endif

    return (TRUE);
}

/** 
 * _gtk_sheet_hide_active_cell: 
 * hide active cell: 
 * 
 * if there is an active cell
 * - redraw cell
 * - update row/column button state
 * - unmap sheet entry
 * - draw background
 * - hide sheet entry
 * - do not modify active_cell
 * 
 * @param sheet  the #GtkSheet
 */
void
_gtk_sheet_hide_active_cell(GtkSheet *sheet)
{
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_visible(sheet->sheet_entry)) return;

    gint old_row = sheet->active_cell.row;
    gint old_col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: called row %d col %d",
        old_row, old_col);
#endif

    if (old_row < 0 || old_row > sheet->maxrow) return;
    if (old_col < 0 || old_col > sheet->maxcol) return;

    if (sheet->freeze_count == 0)
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IS_FROZEN);

#if 0
    /* transferring entry text to the cell gives problems when gtk_sheet_change_entry()
       is called during TRAVERSE signal emission. The text of the already changed
       entry gets written into the deactivated cell */
    {
	char *text = gtk_sheet_get_entry_text(sheet);

	/* todo: compare with existing text, only notify if text changes */
	gtk_sheet_set_cell_text(sheet, old_row, old_col, text);

	g_free(text);
    }
#endif

#if 0
    /* why shoud we first set the cursor to the cell we want hide ? */
    g_signal_emit(G_OBJECT(sheet),sheet_signals[SET_CELL],
        0, old_row, old_col);
#endif

    if (!GTK_SHEET_IS_FROZEN(sheet))
    {
	GtkSheetRange range;

	range.row0 = range.rowi = old_row;
	range.col0 = range.coli = old_col;

	_gtk_sheet_range_draw(sheet, &range, FALSE);  /* do not reactivate active cell!!! */
    }
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: _gtk_sheet_column_button_release");
#endif

    _gtk_sheet_column_button_release(sheet, old_col);
    row_button_release(sheet, old_row);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("_gtk_sheet_hide_active_cell: gtk_widget_unmap");
#endif
    gtk_widget_unmap(sheet->sheet_entry);

    // use swin_cr to blit sheet->bsurf into sheet_window
    cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);
    cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);

    cairo_save(swin_cr); // _cairo_clip_sheet_area
    _cairo_clip_sheet_area(sheet, swin_cr);

    // blit
    cairo_rectangle(swin_cr, 
	_gtk_sheet_column_left_xpixel(sheet, old_col) - 1, 
	_gtk_sheet_row_top_ypixel(sheet, old_row) - 1, 
	COLPTR(sheet, old_col)->width + 4, 
	ROWPTR(sheet, old_row)->height + 4);
    cairo_fill(swin_cr);

    cairo_save(swin_cr); // _cairo_clip_sheet_area
    cairo_destroy(swin_cr);

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

/** 
 * gtk_sheet_activate_cell: 
 * activate cell if possible 
 * 
 * - set sheet range to active cell
 * - set row/column button state
 * - show active cell (grabs focus)
 * - connect sheet entry changed signal
 * - emit ACTIVATE signal
 * 
 * @param sheet  the sheet
 * @param row    row index
 * @param col    column index
 * 
 * @return TRUE on success
 */
static gboolean
gtk_sheet_activate_cell(GtkSheet *sheet, gint row, gint col)
{
    gboolean veto = TRUE;

    g_return_val_if_fail(sheet != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(sheet), FALSE);

    if (GTK_SHEET_FLAGS(sheet) & GTK_SHEET_IS_DESTROYED) return(FALSE); /* PR#102114 */

    if (row < 0 || col < 0) return (FALSE);
    if (row > sheet->maxrow || col > sheet->maxcol) return (FALSE);

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
   _gtksheet_signal_emit(G_OBJECT(sheet),sheet_signals[ACTIVATE], row, col, &veto);
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

    SET_ACTIVE_CELL(row, col);

    SET_SELECTION_PIN(row, col);
    SET_SELECTION_CURSOR(row, col);

    row_button_set(sheet, row);
    _gtk_sheet_column_button_set(sheet, col);

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
    gtk_sheet_show_active_cell(sheet);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("gtk_sheet_activate_cell: signal setup");
#endif

    gtk_sheet_entry_signal_connect_changed(
        sheet, G_CALLBACK(gtk_sheet_entry_changed_handler));

    _gtksheet_signal_emit(
        G_OBJECT(sheet), sheet_signals[ACTIVATE], row, col, &veto);

    return (TRUE);
}

#if 0
/* 06.02.22/fp _gtk_sheet_entry_preselect is obsoleted
    by GtkSetting "gtk-entry-select-on-focus" */
 
static void _gtk_sheet_entry_preselect(GtkSheet *sheet, GtkWidget *sheet_entry)
{
    gboolean select_on_focus;
    gboolean editable = TRUE;  /* incomplete - refer to gtkitementry::gtk_entry_grab_focus() */
    gboolean in_click = FALSE;  /* incomplete - refer to gtkitementry::gtk_entry_grab_focus() */

#if GTK_SHEET_DEBUG_MOVE > 0
    g_debug(
        "%s(%d): called has_focus %d is_focus %d has_grab %d",
        __FUNCTION__, __LINE__,
        gtk_widget_has_focus(sheet_entry),
        gtk_widget_is_focus(sheet_entry),
        gtk_widget_has_grab(sheet_entry)
        );
#endif

    g_object_get(G_OBJECT(gtk_settings_get_default()),
	"gtk-entry-select-on-focus",
	&select_on_focus,
	NULL);

    if (select_on_focus && editable && !in_click
	&& gtk_widget_has_focus(sheet_entry))
    {
	gtk_sheet_entry_select_region(sheet, 0, -1);
    }
}
#endif

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
static void _gtk_sheet_entry_setup(
    GtkSheet *sheet, gint row, gint col,
    GtkWidget *entry_widget)
{
    GtkJustification justification = GTK_JUSTIFY_LEFT;
    gboolean editable;
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

    if (GTK_IS_DATA_ENTRY(entry_widget))
    {
	GtkDataEntry *data_entry = GTK_DATA_ENTRY(entry_widget);
	GtkEntry *entry = GTK_ENTRY(entry_widget);

	/* 5.8.2010/fp - the code below has no effect in GTK 2.18.9,
	   a right justified editable will pop to left justification
	   as soon as something gets selected, and will
	   pop back to right aligment, as soon as the
	   cursor ist moved. When this happens, the
	   justification value in the editable is correct. */

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
	g_debug("_gtk_sheet_entry_setup: GtkDataEntry justification %d", justification);
#endif
	switch(justification)
	{
	    case GTK_JUSTIFY_LEFT:
		gtk_entry_set_alignment (entry, 0.0);
		break;

	    case GTK_JUSTIFY_RIGHT:
		gtk_entry_set_alignment (entry, 1.0);
		break;
 
	    case GTK_JUSTIFY_CENTER:
	    case GTK_JUSTIFY_FILL:
		gtk_entry_set_alignment (entry, 0.5);
		break;
	}
	gtk_data_entry_set_max_length_bytes(
            data_entry, colptr->max_length_bytes);
	gtk_entry_set_max_length(entry, colptr->max_length);
        gtk_entry_set_has_frame(entry, FALSE);
	gtk_entry_set_visibility(entry, !colptr->is_secret);

        gtk_entry_set_width_chars(entry, 0);
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
        gtk_entry_set_has_frame(entry, FALSE);
	gtk_entry_set_visibility(entry, !colptr->is_secret);

        gtk_entry_set_width_chars(entry, 0);
    }
    else if (GTK_IS_SPIN_BUTTON(entry_widget))
    {
	GtkEntry *entry = GTK_ENTRY(entry_widget);

        gtk_entry_set_width_chars(entry, 0);
    }

    if (gtk_widget_get_realized(entry_widget))
    {
        GtkStyleContext *entry_context = gtk_widget_get_style_context(
            GTK_WIDGET(sheet->sheet_entry));

        GList *class_list = gtk_style_context_list_classes(
            entry_context);

	/* remove all classes except flat */

        GList *p;
        for (p=class_list; p; p=p->next)
        {
            if (strcmp(p->data, "flat") != 0)
                gtk_style_context_remove_class(entry_context, p->data);
        }

        g_list_free(class_list);

        if (attributes.css_class)
        {
            gtk_style_context_add_class(entry_context,
                attributes.css_class);
        }
        else if (sheet->css_class)
        {
            gtk_style_context_add_class(entry_context,
                sheet->css_class);
        }

        _cell_style_context_add_position_subclass(sheet, 
            entry_context, row, col);
    }
}

/** 
 * gtk_sheet_show_active_cell: 
 * show active cell 
 * 
 * - transfer cell visibility to sheet entry
 * - setup sheet entry
 * - update text in sheet entry
 * - allocate size of sheet entry
 * - draw active cell
 * - preselect sheet entry contents
 * 
 * @param sheet  the sheet
 */
static void
gtk_sheet_show_active_cell(GtkSheet *sheet)
{
    gchar *text = NULL;
    gboolean is_visible = TRUE;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    gint act_row = sheet->active_cell.row;
    gint act_col = sheet->active_cell.col;

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("%s(%d): %s %p called row %d col %d inSel %s inResize %s inDrag %s",
        __FUNCTION__, __LINE__,
        G_OBJECT_TYPE_NAME(sheet), sheet,
        act_row, act_col,
	GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
	);
#endif

    if (act_row < 0 || act_col < 0) return;
    if (act_row > sheet->maxrow || act_col > sheet->maxcol) return;

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (sheet->state != GTK_SHEET_NORMAL) return;
    if (GTK_SHEET_IN_SELECTION(sheet)) return;
    if (!sheet->sheet_entry)  return;  /* PR#102114 */

    /* we should send a ENTRY_CHANGE_REQUEST signal here */

    if ((act_row <= sheet->maxallocrow)
        && (act_col <= sheet->maxalloccol)
	&& sheet->data[act_row])
    {
	GtkSheetCell *cell = sheet->data[act_row][act_col];
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

    gtk_widget_set_visible(
        GTK_WIDGET(sheet->sheet_entry), is_visible);

    if (GTK_IS_ENTRY(entry_widget))
    {
	gtk_entry_set_visibility(
            GTK_ENTRY(entry_widget), is_visible);
    }

    /* update text  */

    if (!text)
	text = g_strdup("");

    gboolean is_markup, is_modified;
    gchar *old_text = _gtk_sheet_get_entry_text_internal(sheet,
        act_row, act_col,
        &is_markup, &is_modified);

    /* the entry setup must be done before the text assigment,
       otherwise max_length may cause text assignment to fail
       */
    _gtk_sheet_entry_setup(sheet, act_row, act_col, entry_widget);

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
    g_debug("%s(%d): old_text <%s> text <%s>",
        __FUNCTION__, __LINE__, old_text, text);
#endif

    if (!old_text
        || (old_text[0] != text[0])
        || strcmp(old_text, text) != 0)
    {
        _gtk_sheet_set_entry_text_internal(sheet, text, is_markup);
    }

    /* we should send an ENTRY_CONFIGURATION signal here */

    //gtk_sheet_entry_set_max_size(sheet);
    _gtk_sheet_entry_size_allocate(sheet);

    gtk_widget_map(sheet->sheet_entry);
    gtk_sheet_draw_active_cell(sheet, NULL);

#if 0
    /* 06.02.22/fp _gtk_sheet_entry_preselect is obsoleted
        by GtkSetting "gtk-entry-select-on-focus" */

    _gtk_sheet_entry_preselect(sheet, sheet->sheet_entry);
#endif

    if (gtk_widget_get_mapped(GTK_WIDGET(sheet))) 
    { // 23.02.20/fp 233370 06.02.22/fp 309343
#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 0
        g_debug("%s(%d): mapped  has_focus %d is_focus %d has_grab %d",
            __FUNCTION__, __LINE__,
            gtk_widget_has_focus(entry_widget),
            gtk_widget_is_focus(entry_widget),
            gtk_widget_has_grab(entry_widget)
            );
#endif
        if (!gtk_widget_is_focus(entry_widget)) // maybe gtk_widget_has_focus?
        {
            gtk_widget_grab_focus(entry_widget);
        }
    }

    g_free(text);
    g_free(old_text);
}

/** 
 * gtk_sheet_draw_active_cell: 
 * draw active cell, but do not activate sheet entry
 * 
 * - set row/column buttons
 * - draw backing pixmap
 * - draw borders
 * 
 * @param sheet  the sheet
 * @param cr     cairo context or NULL
 */
static void
gtk_sheet_draw_active_cell(GtkSheet *sheet, cairo_t *cr)
{
    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    gint act_row = sheet->active_cell.row;
    gint act_col = sheet->active_cell.col;

    if (act_row < 0 || act_row > sheet->maxrow) return;
    if (act_col < 0 || act_col > sheet->maxcol) return;

    if (!gtk_sheet_cell_isvisible(sheet, act_row, act_col)) return;

    row_button_set(sheet, act_row);
    _gtk_sheet_column_button_set(sheet, act_col);

    gtk_sheet_draw_backing_pixmap(sheet, sheet->range, cr);
    gtk_sheet_draw_border(sheet, sheet->range, cr);
}


static void
gtk_sheet_make_bsurf(GtkSheet *sheet, guint width, guint height)
{
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    if (width == 0 && height == 0)
    {
	width = sheet->sheet_window_width + 80;
	height = sheet->sheet_window_height + 80;
    }

    if (sheet->bsurf 
	&& ((sheet->bsurf_width != width) || (sheet->bsurf_height != height)) )
    {
#if GTK_SHEET_DEBUG_EXPOSE > 0
	g_debug("gtk_sheet_make_bsurf: resize");
#endif
	cairo_destroy(sheet->bsurf_cr);
	sheet->bsurf_cr = NULL;

	cairo_surface_destroy(sheet->bsurf);
	sheet->bsurf = NULL;
    }

    if (!sheet->bsurf)
    {
	/* allocate */
	sheet->bsurf = gdk_window_create_similar_surface(sheet->sheet_window,
	    CAIRO_CONTENT_COLOR_ALPHA, width, height);

#if GTK_SHEET_DEBUG_EXPOSE > 0
	g_debug("gtk_sheet_make_bsurf: type %d", 
	    cairo_surface_get_type(sheet->bsurf));
#endif

	sheet->bsurf_width = width;
	sheet->bsurf_height = height;
	sheet->bsurf_cr = cairo_create(sheet->bsurf);

	if (!GTK_SHEET_IS_FROZEN(sheet))
	    _gtk_sheet_range_draw(sheet, NULL, TRUE);
    }
}

/** 
 * gtk_sheet_new_selection: 
 * compute and draw new selection 
 * 
 * @param sheet  the sheet
 * @param range  the new range or NULL 
 *  
 * passing NULL reverts to sheet->range 
 */
static void
gtk_sheet_new_selection(GtkSheet *sheet, GtkSheetRange *range)
{
    gint row, col;
    gint cell_state;
    gint x, y, width, height;
    GtkSheetRange new_range;

    g_return_if_fail(sheet != NULL);

    if (range == NULL)
	range = &sheet->range;

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_new_selection: row %d-%d col %d-%d",
        range->row0, range->rowi, range->col0, range->coli);
#endif

    _gtk_sheet_deactivate_cell(sheet);

    // use swin_cr to blit sheet->bsurf into sheet_window
    cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);
    cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);

    cairo_save(swin_cr); // _cairo_clip_sheet_area
    _cairo_clip_sheet_area(sheet, swin_cr);

    cairo_t *xor_cr = gdk_cairo_create(sheet->sheet_window); /* FIXME, to be removed */
    _cairo_save_and_set_sel_color(sheet, xor_cr);

    new_range = *range;  /* copy new range */

    /* union range and sheet->range - affected envelope */
    range->row0 = MIN(range->row0, sheet->range.row0);
    range->rowi = MAX(range->rowi, sheet->range.rowi);
    range->col0 = MIN(range->col0, sheet->range.col0);
    range->coli = MAX(range->coli, sheet->range.coli);

    /* restrict range to visible area - visible part of envelope */
    range->row0 = MAX(range->row0, MIN_VIEW_ROW(sheet));
    range->rowi = MIN(range->rowi, MAX_VIEW_ROW(sheet));
    range->col0 = MAX(range->col0, MIN_VIEW_COLUMN(sheet));
    range->coli = MIN(range->coli, MAX_VIEW_COLUMN(sheet));

#if 1
    /* loop over envelope
       - for visible selected cells not within new_range
       - clear unselected cells to bsurf 

       - beware: border of new_range will be scrambled
       */
    for (row = range->row0; row <= range->rowi; row++)
    {
	for (col = range->col0; col <= range->coli; col++)
	{
	    cell_state = gtk_sheet_cell_get_state(sheet, row, col);
	    gboolean in_new_range = CELL_IN_RANGE(row, col, new_range);

	    if (cell_state == GTK_STATE_SELECTED 
		&& !in_new_range 
		&& GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) 
		&& GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	    {
		x = _gtk_sheet_column_left_xpixel(sheet, col);
		y = _gtk_sheet_row_top_ypixel(sheet, row);
		width = COLPTR(sheet, col)->width;
		height = ROWPTR(sheet, row)->height;

                double d;
                d = selection_border_width - selection_border_offset;
                x -= d;
                y -= d;
                d = MAX(d, selection_corner_size - selection_corner_offset);
                width += d*2;
                height += d*2;

                // blit
		cairo_rectangle(swin_cr, x, y, width, height);
#if GTK_SHEET_DEBUG_CAIRO > 0
                g_debug("cairo_fill: blit enabled (3)");
#endif
		cairo_fill(swin_cr);

                _gtk_sheet_invalidate_region(
                    sheet, x-1, y-1, width+2, height+2);
	    }
	}
    }
#else
    g_debug("FIXME clear unselected turned off for debug");
#endif

#if  1
    /* loop over envelope
       - for visible selected cells within new_range (except active cell)
       - draw selected background
       */
    for (row = range->row0; row <= range->rowi; row++)
    {
	for (col = range->col0; col <= range->coli; col++)
	{
	    cell_state = gtk_sheet_cell_get_state(sheet, row, col);
	    gboolean in_new_range = CELL_IN_RANGE(row, col, new_range);

	    if (in_new_range 
		&& GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col)) 
		&& GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)) 
		//&& (row != sheet->active_cell.row || col != sheet->active_cell.col)
                )
	    {
		x = _gtk_sheet_column_left_xpixel(sheet, col);
		y = _gtk_sheet_row_top_ypixel(sheet, row);
		width = COLPTR(sheet, col)->width;
		height = ROWPTR(sheet, row)->height;

                _selection_border_add_gap_for_bg(
                    row, col, &new_range, &x, &y, &width, &height);

                // xor
		cairo_rectangle(xor_cr, x, y, width, height);
#if GTK_SHEET_DEBUG_CAIRO > 0
                g_debug("cairo_fill: xor disabled (4) %d %d %d %d",
                    x, y, width, height);
#endif
                cairo_fill(xor_cr); 

#if 0
                _debug_color_rect(
                    "gtk_sheet_new_selection invalidate gap", 
                    xor_cr, x, y, width, height);
#endif
                _gtk_sheet_invalidate_region(sheet, x, y, width, height);
            }
        }
    }
#else
    g_debug("FIXME selection bg drawing turned off for debug");
#endif

    gtk_sheet_draw_border(sheet, new_range, NULL);

    *range = new_range;  /* restore new_range */

    cairo_restore(xor_cr);   // _cairo_save_and_set_sel_color
    cairo_destroy(xor_cr);  /* FIXME, to be removed */

    cairo_save(swin_cr); // _cairo_clip_sheet_area
    cairo_destroy(swin_cr);  /* FIXME */
}

/** 
 * gtk_sheet_draw_border: 
 * draw selection border 
 * 
 * Called without a cairo context, 
 * _gtk_sheet_invalidate_region() will be called.
 * 
 * @param sheet     the sheet
 * @param new_range selected range
 * @param cr        cairo context or NULL
 */
static void
gtk_sheet_draw_border(
    GtkSheet *sheet, 
    GtkSheetRange new_range,
    cairo_t *cr)
{
    GdkRectangle clip_area, area;

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

#if GTK_SHEET_DEBUG_CELL_ACTIVATION > 1
    g_debug("gtk_sheet_draw_border: clip_area (%d, %d, %d, %d)",
	clip_area.x, clip_area.y, clip_area.width, clip_area.height);
#endif

    cairo_t *my_cr = cr;

    if (!my_cr)
    {
        // when not in expose - create my own cr and destroy at end
        my_cr = gdk_cairo_create(sheet->sheet_window);
    }

    _cairo_save_and_set_sel_color(sheet, my_cr);
    
    cairo_set_line_cap(my_cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(my_cr, CAIRO_LINE_JOIN_MITER);
    cairo_set_dash(my_cr, NULL, 0, 0.0);

    cairo_set_line_width(my_cr, selection_border_width);

    gdouble d,x,y,w,h;

    d =  1.0 - selection_border_offset - selection_border_width/2.0;
    x = area.x + d;
    y = area.y + d;
    w = area.width - 1.0 - d * 2.0;
    h = area.height - d * 2.0;

    cairo_rectangle(my_cr, x, y, w, h);
    cairo_stroke(my_cr);  /* draw border */

    x -= selection_border_width/2.0;
    y -= selection_border_width/2.0;
    w += selection_border_width;
    h += selection_border_width;

#if 0
    _debug_color_rect(
        "gtk_sheet_draw_border invalidate", my_cr, x, y, w, h);
#endif

    if (!cr)
    {
        _gtk_sheet_invalidate_region(sheet, x, y, w, h);
    }

    cairo_restore(my_cr);  // _cairo_save_and_set_sel_color
    if (!cr) cairo_destroy(my_cr);

    gtk_sheet_draw_corners(sheet, new_range, cr);
}

/** 
 * gtk_sheet_draw_corners: 
 * draw corners of selection border 
 * 
 * @param sheet   the sheet
 * @param range   the selection range
 * @param cr cairo drawing context
 */
static void
gtk_sheet_draw_corners(
    GtkSheet *sheet, GtkSheetRange range,
    cairo_t *cr)
{
    gint x, y;

    /* bewae: corner must stay within border!*/

    cairo_t *my_cr = cr;

    if (!my_cr)
    {
        // when not in expose - create my own cr and destroy at end
        my_cr = gdk_cairo_create(sheet->sheet_window);
    }

    /* top right */
    if (sheet->state == GTK_SHEET_COLUMN_SELECTED)
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.coli) +
	    COLPTR(sheet, range.coli)->width;
	y = _gtk_sheet_row_top_ypixel(sheet, range.row0);

        x += selection_corner_offset - 2;
        y += 0;

        //_cairo_clip_sheet_area(sheet, swin_cr); // not needed here

        cairo_set_source_surface(my_cr, sheet->bsurf, 0, 0);

        //gdk_cairo_set_source_rgba(my_cr, &debug_color);
	cairo_rectangle(my_cr, 
	    x-1, y-1, selection_corner_size+2, selection_corner_size+2);
	cairo_fill(my_cr);

        _cairo_save_and_set_sel_color(sheet, my_cr);

	cairo_rectangle(my_cr, 
	    x, y, selection_corner_size, selection_corner_size);
	cairo_fill(my_cr);

        cairo_restore(my_cr);  // _cairo_save_and_set_sel_color

        if (!cr)
        {
            _gtk_sheet_invalidate_region(sheet,
                x-1, y-1, selection_corner_size+2, selection_corner_size+2);
        }
    }

    /* bottom left */
    if (sheet->state == GTK_SHEET_ROW_SELECTED)
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.col0);
	y = _gtk_sheet_row_top_ypixel(sheet, range.rowi) +
            ROWPTR(sheet, range.rowi)->height;

        x += -1;
        y += selection_corner_offset - 1;

        cairo_set_source_surface(my_cr, sheet->bsurf, 0, 0);
        //gdk_cairo_set_source_rgba(my_cr, &debug_color);
	cairo_rectangle(my_cr, 
	    x-1, y-1, selection_corner_size+2, selection_corner_size+2);
	cairo_fill(my_cr);

        _cairo_save_and_set_sel_color(sheet, my_cr);

	cairo_rectangle(my_cr, 
	    x, y, selection_corner_size, selection_corner_size);
	cairo_fill(my_cr);

        cairo_restore(my_cr);  // _cairo_save_and_set_sel_color

        if (!cr)
        {
            _gtk_sheet_invalidate_region(sheet,
                x-1, y-1, selection_corner_size+2, selection_corner_size+2);
        }
    }

    /* bottom right */
    if (gtk_sheet_cell_isvisible(sheet, range.rowi, range.coli))
    {
	x = _gtk_sheet_column_left_xpixel(sheet, range.coli) +
	    COLPTR(sheet, range.coli)->width;
	y = _gtk_sheet_row_top_ypixel(sheet, range.rowi) +
            ROWPTR(sheet, range.rowi)->height;

        x += selection_corner_offset - 2;
        y += selection_corner_offset - 1;

        cairo_set_source_surface(my_cr, sheet->bsurf, 0, 0);
        //gdk_cairo_set_source_rgba(my_cr, &debug_color);
	cairo_rectangle(my_cr, 
	    x-1, y-1, selection_corner_size+2, selection_corner_size+2);
	cairo_fill(my_cr);

        _cairo_save_and_set_sel_color(sheet, my_cr);

	cairo_rectangle(my_cr, 
	    x, y, selection_corner_size, selection_corner_size);
	cairo_fill(my_cr);

        cairo_restore(my_cr);  // _cairo_save_and_set_sel_color

        if (!cr)
        {
            _gtk_sheet_invalidate_region(sheet,
                x-1, y-1, selection_corner_size+2, selection_corner_size+2);
        }
    }

    if (!cr) cairo_destroy(my_cr);
}


static void
gtk_sheet_real_select_range(GtkSheet *sheet, GtkSheetRange *range)
{
    gint state;

    g_return_if_fail(sheet != NULL);

    if (range == NULL)
	range = &sheet->range;

    if (range->row0 < 0 || range->rowi < 0) return;
    if (range->col0 < 0 || range->coli < 0) return;

#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("gtk_sheet_real_select_range: {%d, %d, %d, %d}",
	range->row0, range->col0, range->rowi, range->coli);
#endif

    state = sheet->state;

    if (state == GTK_SHEET_COLUMN_SELECTED
        || state == GTK_SHEET_RANGE_SELECTED)
    {
        gint col;

	for (col = sheet->range.col0; col < range->col0; col++)
            _gtk_sheet_column_button_release(sheet, col);

	for (col = range->coli + 1; col <= sheet->range.coli; col++)
            _gtk_sheet_column_button_release(sheet, col);

	for (col = range->col0; col <= range->coli; col++)
	{
	    _gtk_sheet_column_button_set(sheet, col);
	}
    }

    if (state == GTK_SHEET_ROW_SELECTED
        || state == GTK_SHEET_RANGE_SELECTED)
    {
        gint row;

	for (row = sheet->range.row0; row < range->row0; row++)
            row_button_release(sheet, row);

	for (row = range->rowi + 1; row <= sheet->range.rowi; row++)
            row_button_release(sheet, row);

	for (row = range->row0; row <= range->rowi; row++)
	{
	    row_button_set(sheet, row);
	}
    }

    if (range->coli != sheet->range.coli
        || range->col0 != sheet->range.col0
        || range->rowi != sheet->range.rowi
        || range->row0 != sheet->range.row0)
    {
	gtk_sheet_new_selection(sheet, range);

	sheet->range.col0 = range->col0;
	sheet->range.coli = range->coli;
	sheet->range.row0 = range->row0;
	sheet->range.rowi = range->rowi;
    }
    else
    {
	gtk_sheet_draw_backing_pixmap(sheet, sheet->range, NULL);
	gtk_sheet_range_draw_selection(sheet, sheet->range, NULL);
    }

    g_signal_emit(G_OBJECT(sheet), sheet_signals[SELECT_RANGE],
        0, range);
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

    if (new_range.row0 < 0 || new_range.rowi < 0) return;
    if (new_range.col0 < 0 || new_range.coli < 0) return;

    if (sheet->state != GTK_SHEET_NORMAL)
    {
	/* this will clear sheet->range */
	gtk_sheet_real_unselect_range(sheet, NULL);
    }
    else
    {
	gboolean veto = TRUE;
	veto = _gtk_sheet_deactivate_cell(sheet);
	if (!veto) return;
    }

    sheet->range.row0 = new_range.row0;
    sheet->range.rowi = new_range.rowi;
    sheet->range.col0 = new_range.col0;
    sheet->range.coli = new_range.coli;

    SET_SELECTION_PIN(new_range.row0, new_range.col0);
    SET_SELECTION_CURSOR(new_range.rowi, new_range.coli);

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

    gtk_sheet_activate_cell(
        sheet, sheet->active_cell.row, sheet->active_cell.col);
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
	gtk_sheet_draw_backing_pixmap(sheet, *range, NULL);
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
    _gtk_sheet_position_children(sheet);

    /* reset range */
    range->row0 = range->rowi = range->col0 = range->coli = -1;
}


/*
 * gtk_sheet_draw:
 * 
 * this is the #GtkSheet widget class "expose-event" signal handler
 * 
 * @param widget the #GtkSheet
 * @param event  the GdkEventExpose which triggered this signal
 * @param cr cairo context or NULL
 * 
 * @return TRUE to stop other handlers from being invoked for the event. FALSE to propagate the event further.
 */
static gboolean
gtk_sheet_draw(GtkWidget *widget, cairo_t *cr)
{
    GtkSheet *sheet;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);

#if GTK_SHEET_DEBUG_EXPOSE > 1
    _debug_cairo_clip_extent("gtk_sheet_draw", cr);
#endif

    sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_EXPOSE > 1
    gdk_cairo_set_source_rgba(cr, &debug_color);
    cairo_rectangle(cr, 100, 200, 1300, 40);
    cairo_fill(cr);
#endif

    if (gtk_widget_is_drawable(widget))
    {
#if GTK_SHEET_DEBUG_EXPOSE > 0
        GTimeVal timeval;
        g_get_current_time (&timeval);
        gchar *isotm = g_time_val_to_iso8601(&timeval);
	g_debug("gtk_sheet_draw: called %s", isotm);
        g_free(isotm);
#endif

	if (gtk_cairo_should_draw_window(cr, sheet->row_title_window)
            && sheet->row_titles_visible)
	{
#if GTK_SHEET_DEBUG_EXPOSE > 0
	    g_debug("gtk_sheet_draw: row buttons");
#endif
            gint row;
	    for (row = MIN_VIEW_ROW(sheet);
                  row <= MAX_VIEW_ROW(sheet) && row <= sheet->maxrow;
                  row++)
	    {
		_gtk_sheet_draw_button(sheet, row, -1, cr);
	    }
	}


	if (gtk_cairo_should_draw_window(cr, sheet->column_title_window)
            && sheet->column_titles_visible)
	{
#if GTK_SHEET_DEBUG_EXPOSE > 0
            g_debug("%s(%d): column buttons", 
                __FUNCTION__, __LINE__);
#endif
            gint col;
	    for (col = MIN_VIEW_COLUMN(sheet);
                  col <= MAX_VIEW_COLUMN(sheet) && col <= sheet->maxcol;
                  col++)
	    {
		_gtk_sheet_draw_button(sheet, -1, col, cr);
	    }
	}

	if (gtk_cairo_should_draw_window(cr, sheet->sheet_window))
	{
	    GtkSheetRange range;

	    _gtk_sheet_get_visible_range(sheet, &range);

#if GTK_SHEET_DEBUG_EXPOSE > 0
	    g_debug("gtk_sheet_draw: bsurf (%d,%d) (%d,%d)",
		range.row0, range.col0, range.rowi, range.coli);
#endif

	    gtk_sheet_draw_backing_pixmap(sheet, range, cr);

	    if (sheet->state != GTK_SHEET_NORMAL)
	    {
		if (gtk_sheet_range_isvisible(sheet, sheet->range))
		    gtk_sheet_draw_backing_pixmap(sheet, sheet->range, cr);

		if (GTK_SHEET_IN_RESIZE(sheet) || GTK_SHEET_IN_DRAG(sheet))
		    gtk_sheet_draw_backing_pixmap(sheet, sheet->drag_range, cr);

		if (gtk_sheet_range_isvisible(sheet, sheet->range))
		    gtk_sheet_range_draw_selection(sheet, sheet->range, cr);

		if (GTK_SHEET_IN_RESIZE(sheet) || GTK_SHEET_IN_DRAG(sheet))
                    gtk_sheet_draw_border(sheet, sheet->drag_range, NULL);
	    }

	    if ((!GTK_SHEET_IN_XDRAG(sheet))
                && (!GTK_SHEET_IN_YDRAG(sheet)))
	    {
		if (sheet->state == GTK_SHEET_NORMAL)
		{
		    gtk_sheet_draw_active_cell(sheet, cr);
		}
	    }
	}
    }

    if (sheet->row_titles_visible && sheet->column_titles_visible
        && gtk_widget_get_visible(sheet->button))
    {
        /* fix for
           **: GtkButton is drawn without a current allocation.
           **: GtkLabel is drawn without a current allocation.
           */
        _global_sheet_button_size_allocate(sheet);
    }

    if (sheet->sheet_entry
        && gtk_widget_get_visible(sheet->sheet_entry))
    {
        if (!gtk_widget_get_mapped(sheet->sheet_entry))
        {
            //g_debug("FIXME - map in draw");
            //gtk_widget_map(sheet->sheet_entry);
        }
        // FIXME - beware: gtk_sheet_draw() must not activate sheet entry
        // FIXME - it causes draw loop
        // FIXME - it prevents cursor off sheet in testgtksheet
        // FIXME - it doesn't allow input to sheet entry
        //gtk_sheet_show_active_cell(sheet);
    }

    if (sheet->state != GTK_SHEET_NORMAL
        && GTK_SHEET_IN_SELECTION(sheet))
    {
        gtk_widget_grab_focus(GTK_WIDGET(sheet));
    }

    /* propagation re-enabled - 28.04.18/fp #219937
       */
    if (GTK_WIDGET_CLASS(sheet_parent_class)->draw)
    	(*GTK_WIDGET_CLASS(sheet_parent_class)->draw)(widget, cr);

    return (GDK_EVENT_PROPAGATE);
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

    sheet = GTK_SHEET(widget);
    GdkWindow *main_window = gtk_widget_get_window(widget);  // for coordinates

    gdk_window_get_device_position(
	main_window, event->device, NULL, NULL, &mods);

    if (!(mods & GDK_BUTTON1_MASK)) 
    {
	return (TRUE);
    }

    /* press on resize windows */
    if (event->window == sheet->column_title_window 
	&& gtk_sheet_columns_resizable(sheet))
    {
	gdk_window_get_device_position(main_window, event->device, 
	    &sheet->x_drag, NULL, NULL);

	if (POSSIBLE_XDRAG(sheet, sheet->x_drag, &sheet->drag_cell.col))
	{
	    guint req;
	    if (event->type == GDK_2BUTTON_PRESS)
	    {
		_gtk_sheet_autoresize_column_internal(
                    sheet, sheet->drag_cell.col);

		GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
		return (TRUE);
	    }
	    _gtk_sheet_column_size_request(sheet, sheet->drag_cell.col, &req);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
	    gdk_device_grab (event->device, event->window,
		GDK_OWNERSHIP_WINDOW, FALSE,
		GDK_POINTER_MOTION_MASK | 
		GDK_BUTTON1_MOTION_MASK | 
		GDK_BUTTON_RELEASE_MASK,
		NULL, event->time);
	    draw_xor_vline(sheet, TRUE);
	    return (TRUE);
	}
    }

    if (event->window == sheet->row_title_window 
	&& gtk_sheet_rows_resizable(sheet))
    {
	gdk_window_get_device_position(main_window, event->device,
	    NULL, &sheet->y_drag, NULL);

	if (POSSIBLE_YDRAG(sheet, sheet->y_drag, &sheet->drag_cell.row))
	{
	    guint req;
	    gtk_sheet_row_size_request(sheet, sheet->drag_cell.row, &req);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_YDRAG);
	    gdk_device_grab (event->device, event->window,
		GDK_OWNERSHIP_WINDOW, FALSE,
		GDK_POINTER_MOTION_MASK | 
		GDK_BUTTON1_MOTION_MASK | 
		GDK_BUTTON_RELEASE_MASK,
		NULL, event->time);

	    draw_xor_hline(sheet, TRUE);
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

	gdk_window_get_device_position(
	    main_window, event->device, &x, &y, NULL);

	gtk_sheet_get_pixel_info(sheet, NULL, x, y, &row, &column);
	if (row < 0 && column < 0) return(FALSE);  /* chain up to global button press handler*/

#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug("%s(%d): pointer grab (%d,%d) r %d c %d", 
	    __FUNCTION__, __LINE__,
	    x, y, row, column);
#endif
	gdk_device_grab (event->device, event->window,
	    GDK_OWNERSHIP_WINDOW, FALSE,
	    GDK_POINTER_MOTION_MASK | 
	    GDK_BUTTON1_MOTION_MASK | 
	    GDK_BUTTON_RELEASE_MASK,
	    NULL, event->time);
	gtk_grab_add(GTK_WIDGET(sheet));

	sheet->timer = g_timeout_add_full(0, TIMEOUT_SCROLL, 
	    _gtk_sheet_scroll_to_pointer, sheet, NULL);

#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug("%s(%d): grab focus", __FUNCTION__, __LINE__);
#endif
	gtk_widget_grab_focus(GTK_WIDGET(sheet));

	if (sheet->selection_mode != GTK_SELECTION_SINGLE 
	    && gdk_cursor_get_cursor_type(sheet->cursor_drag) == GDK_SIZING 
	    && !GTK_SHEET_IN_SELECTION(sheet) 
	    && !GTK_SHEET_IN_RESIZE(sheet))
	{
	    if (sheet->state == GTK_SHEET_NORMAL)
	    {
		gint old_row = sheet->active_cell.row;  /* PR#203012 */
		gint old_col = sheet->active_cell.col;  /* PR#203012 */

		if (!_gtk_sheet_deactivate_cell(sheet)) return (FALSE);

                SET_SELECTION_PIN(old_row, old_col);

		sheet->drag_range = sheet->range;

                sheet->state = GTK_SHEET_RANGE_SELECTED;
		gtk_sheet_select_range(sheet, &sheet->drag_range);
	    }

	    sheet->x_drag = x;
	    sheet->y_drag = y;

	    if (row > sheet->range.rowi) row--;
	    if (column > sheet->range.coli) column--;

	    sheet->drag_cell.row = row;
	    sheet->drag_cell.col = column;
	    sheet->drag_range = sheet->range;

            gtk_sheet_draw_border(sheet, sheet->drag_range, NULL);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_RESIZE);
	}
	else if (gdk_cursor_get_cursor_type(sheet->cursor_drag) == GDK_TOP_LEFT_ARROW 
		 && !GTK_SHEET_IN_SELECTION(sheet) 
		 && !GTK_SHEET_IN_DRAG(sheet))
	{
	    if (sheet->state == GTK_SHEET_NORMAL)
	    {
		gint old_row = sheet->active_cell.row;  /* PR#203012 */
		gint old_col = sheet->active_cell.col;  /* PR#203012 */

		if (!_gtk_sheet_deactivate_cell(sheet)) return (FALSE);

                SET_SELECTION_PIN(old_row, old_col);

		sheet->drag_range = sheet->range;

                sheet->state = GTK_SHEET_RANGE_SELECTED;
		gtk_sheet_select_range(sheet, &sheet->drag_range);
	    }
	    sheet->x_drag = x;
	    sheet->y_drag = y;

	    if (row < sheet->range.row0) row++;
	    if (row > sheet->range.rowi) row--;
	    if (column < sheet->range.col0) column++;
	    if (column > sheet->range.coli) column--;

	    sheet->drag_cell.row = row;
	    sheet->drag_cell.col = column;
	    sheet->drag_range = sheet->range;

            gtk_sheet_draw_border(sheet, sheet->drag_range, NULL);
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_DRAG);
	}
	else
	{
#if GTK_SHEET_DEBUG_MOUSE > 0
	    g_debug(
		"%s(%d): SET_SELECTION_PIN r/c %d/%d", 
		__FUNCTION__, __LINE__, 
		row, column);
#endif

	    if (!_gtk_sheet_deactivate_cell(sheet)) return (FALSE);

	    SET_SELECTION_PIN(row, column);

	    sheet->x_drag = x;
	    sheet->y_drag = y;

#if 0
#if GTK_SHEET_DEBUG_MOUSE > 0
	    g_debug("%s(%d): on click cell", __FUNCTION__, __LINE__);
#endif
	    /* 303340 09.01.21/fp - added gdk_device_ungrab()
	       gtk_sheet_click_cell emits a traverse signal which
	       allows handlers to show popup windows. The grab prevents
	       popup windows from working properly because mouse device
	       is grabbed.
	    */
	    gdk_device_ungrab(event->device, event->time);

	    gint old_active_row = sheet->active_cell.row;
	    gint old_active_col = sheet->active_cell.col;

	    gtk_sheet_click_cell(sheet, row, column, &veto);

	    if (veto)
            {
		/* 303340~0049875
		   Selection mode is started on button press here.
		   While executing traverse handler, mouse button might
		   have been already released in another window. So we
		   need to reset selection in gtk_sheet_motion_handler()
		   when no button is active.
		   */

#if GTK_SHEET_DEBUG_MOUSE > 1
		g_debug("%s(%d): active cell: oldr %d oldc %d newr %d newc %d",
		    __FUNCTION__, __LINE__,
		    old_active_row, old_active_col,
		    sheet->active_cell.row, sheet->active_cell.col);
#endif

#if GTK_SHEET_DEBUG_MOUSE > 0
		g_debug("%s(%d): set inSel mapped %d visible %d is_focus %d is_sensitive %d has_grab %d", 
		    __FUNCTION__, __LINE__,
		    gtk_widget_get_mapped(GTK_WIDGET(sheet)),
		    gtk_widget_get_visible(GTK_WIDGET(sheet)),
		    gtk_widget_is_focus(GTK_WIDGET(sheet)),
		    gtk_widget_is_sensitive(GTK_WIDGET(sheet)),
		    gtk_widget_has_grab(GTK_WIDGET(sheet))
		    );
#endif

		/* the condition below sovles only part of the problem.
		   It helps keeping selection OFF when traverse handler
		   issues a move to the originating cell.
		   
		   The selection is still extended when the traverse
		   handler issues a move to another cell.
		*/
		if ((old_active_row != sheet->active_cell.row)
		    || (old_active_col != sheet->active_cell.col))
		{
		    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
		}
            }
#endif
	}
	return(TRUE);
    }

    if (event->window == sheet->column_title_window)
    {
	gdk_window_get_device_position(
	    main_window, event->device, &x, &y, NULL);

	column = _gtk_sheet_column_from_xpixel(sheet, x);
	if (column < 0 || column > sheet->maxcol)
	    return (FALSE);

	if (GTK_SHEET_COLUMN_IS_SENSITIVE(COLPTR(sheet, column)))
	{
	    gtk_sheet_click_cell(sheet, -1, column, &veto);
	    gtk_grab_add(GTK_WIDGET(sheet));
	    sheet->timer = g_timeout_add_full(0, TIMEOUT_SCROLL,
                _gtk_sheet_scroll_to_pointer, sheet, NULL);
            gtk_widget_grab_focus(GTK_WIDGET(sheet));

            GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	}
    }

    if (event->window == sheet->row_title_window)
    {
	gdk_window_get_device_position(main_window, event->device, &x, &y, NULL);

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

#if GTK_CHECK_VERSION(3,6,0) == 0
    GDK_THREADS_ENTER();
#endif

#if GTK_SHEET_DEBUG_SCROLL > 0
    g_debug("%s(%d): window %p", __FUNCTION__, __LINE__, sheet->sheet_window);
#endif

    if (!sheet->sheet_window) return FALSE;   /* 16.01.22/fp 308726 */

    //gtk_widget_get_pointer(GTK_WIDGET(sheet), &x, &y);
    GdkWindow *window = sheet->sheet_window;
    GdkDisplay *display = gdk_window_get_display(window);
    GdkDeviceManager *dev_manager = gdk_display_get_device_manager(display);
    GdkDevice *dev_pointer = gdk_device_manager_get_client_pointer (dev_manager);
    gdk_window_get_device_position (window, dev_pointer, &x, &y, NULL);

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
            gtk_sheet_draw_border(sheet, sheet->drag_range, NULL);
    }

#if GTK_CHECK_VERSION(3,6,0) == 0
    GDK_THREADS_LEAVE();
#endif
    return (TRUE);
}

void
gtk_sheet_click_cell(GtkSheet *sheet, gint row, gint col, gboolean *veto)
{
    *veto = TRUE;

#if GTK_SHEET_DEBUG_CLICK > 0
    g_debug("%s(%d): called, r %d c %d %p",
        __FUNCTION__, __LINE__, row, col, sheet);
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

#if 0
    /* this causes major trouble with GtkSetting "gtk-entry-select-on-focus"
       it always preselects GtkEntry contents when clicking a cell
       it prevents ordinary mouse selection in sheet entry
       06.02.22/fp 309343 */
    g_debug("%s(%d): calling gtk_widget_grab_focus(sheet)", 
    __FUNCTION__, __LINE__);
    gtk_widget_grab_focus(GTK_WIDGET(sheet));
#endif

    _gtksheet_signal_emit(G_OBJECT(sheet), sheet_signals[TRAVERSE],
	sheet->active_cell.row, sheet->active_cell.col,
	&row, &col, veto);

    if (!*veto)
    {
	if (sheet->state == GTK_SHEET_NORMAL) return;

	gtk_sheet_activate_cell(
            sheet, sheet->active_cell.row, sheet->active_cell.col);
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

        /* if any range is selected, clear it */

	if (sheet->state != GTK_SHEET_NORMAL)
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
	    g_debug("%s(%d): row %d col %d VETO: sheet, can-focus false", 
		__FUNCTION__, __LINE__,
		row, col);
#endif
	    *veto = FALSE;
	    return;
	}

        if (!gtk_widget_get_can_focus(GTK_WIDGET(colp)))
	{
#if GTK_SHEET_DEBUG_CLICK > 0
	    g_debug("%s(%d): row %d col %d VETO: sheet column, can-focus false", 
		__FUNCTION__, __LINE__,
		row, col);
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
	    g_debug("%s(%d): row %d col %d ar %d ac %d", 
                __FUNCTION__, __LINE__, 
                row, col, sheet->active_cell.row, sheet->active_cell.col);
#endif
            /* deactivate only on change */
            if (row != sheet->active_cell.row || col != sheet->active_cell.col)
            {
#if GTK_SHEET_DEBUG_CLICK > 0
                g_debug("%s(%d): row %d col %d calling deactivate, inSel %s inResize %s inDrag %s", 
                    __FUNCTION__, __LINE__, 
		    row, col,
		    GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
		    GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
		    GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
		    );
#endif
                if (!_gtk_sheet_deactivate_cell(sheet))
                {
#if GTK_SHEET_DEBUG_CLICK > 0
                    g_debug("%s(%d): row %d col %d VETO: deactivate false",
                        __FUNCTION__, __LINE__, row, col);
#endif
                    *veto = FALSE;
                    return;
                }
            }
#if GTK_SHEET_DEBUG_CLICK > 0
	    g_debug("%s(%d): row %d col %d back from deactivate, inSel %s inResize %s inDrag %s", 
		__FUNCTION__, __LINE__,
		row, col,
		GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
		GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
		GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
		);
#endif
	}

	/* auto switch column entry_type */
	/* not sure wether to move this code to gtk_sheet_show_active_cell() */
	{
	    GType installed_entry_type = sheet->installed_entry_type;
	    GType wanted_type =
		(colp->entry_type != G_TYPE_NONE) ? colp->entry_type : sheet->entry_type;

            if (wanted_type == G_TYPE_NONE) // 06.02.22/fp 309343
                wanted_type = DEFAULT_ENTRY_TYPE; /* fallback to default entry type */

#if GTK_SHEET_DEBUG_CLICK > 0
            g_debug(
                "%s(%d): installed_entry_type %d %s wanted_type %d %s colp->entry_type %d %s sheet->entry_type %d %s", 
                __FUNCTION__, __LINE__,
                installed_entry_type, g_type_name(installed_entry_type),
                wanted_type, g_type_name(wanted_type),
                colp->entry_type, g_type_name(colp->entry_type),
                sheet->entry_type, g_type_name(sheet->entry_type)
                );
#endif

	    if (installed_entry_type != wanted_type)
	    {
		if (sheet->state == GTK_SHEET_NORMAL)
                {
                    _gtk_sheet_hide_active_cell(sheet);
                }

                // create_sheet_entry() will not activate sheet entry
                // because active_cell is -1
                // experimentally setting it before call 
                // to create_sheet_entry() will only work partially,
                // if entry_type != wanted_type
                //sheet->active_cell.row = row;
                //sheet->active_cell.col = col;

		create_sheet_entry(
                    sheet, wanted_type ? wanted_type : G_TYPE_NONE);
	    }
	}

	/* DEACTIVATE handler might have called gtk_sheet_set_active_cell(),
	   so wie leave it, if it was changed
	   */
#if 0
        /* restore modified active cell (disabled, see above) */
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

        SET_ACTIVE_CELL(row, col);

        SET_SELECTION_PIN(row, col);
        SET_SELECTION_CURSOR(row, col);

	sheet->range.row0 = row;
	sheet->range.col0 = col;
	sheet->range.rowi = row;
	sheet->range.coli = col;

	sheet->state = GTK_SHEET_NORMAL;

        GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
#if 0
        /* turned off 06.02.22/fp 309343 */
        g_debug(
            "%s(%d): calling gtk_sheet_activate_cell r %d c %d ar %d ac %d",
            __FUNCTION__, __LINE__,
            sheet->selection_pin.row, sheet->selection_pin.col,
            sheet->active_cell.row, sheet->active_cell.col);

        gtk_sheet_activate_cell(
            sheet, sheet->active_cell.row, sheet->active_cell.col);
#endif        
	return;
    }

    g_assert_not_reached();
    gtk_sheet_activate_cell(
        sheet, sheet->active_cell.row, sheet->active_cell.col);
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
gtk_sheet_button_release_handler(
    GtkWidget *widget, GdkEventButton *event)
{
    GtkSheet *sheet;
    gint x, y;

    sheet = GTK_SHEET(widget);
    GdkWindow *main_window = gtk_widget_get_window(widget);  // for coordinates

#if GTK_SHEET_DEBUG_MOUSE > 0
    g_debug("%s(%d): col %d inSel %s inResize %s inDrag %s",
	__FUNCTION__, __LINE__,
	sheet->drag_cell.col, 
	GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
	);
#endif

    /* release on resize windows */
    if (GTK_SHEET_IN_XDRAG(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_XDRAG);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gdk_window_get_device_position (
            main_window, event->device, &x, NULL, NULL);
	gdk_device_ungrab (event->device, event->time);
	draw_xor_vline(sheet, FALSE);

#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("%s(%d): col %d set width %d",
	    __FUNCTION__, __LINE__,
	    sheet->drag_cell.col, 
            new_column_width(sheet, sheet->drag_cell.col, &x));
#endif
	gtk_sheet_set_column_width(sheet, sheet->drag_cell.col, 
            new_column_width(sheet, sheet->drag_cell.col, &x));

	sheet->old_hadjustment = -1.;

	if (sheet->hadjustment)
	{
	    g_signal_emit_by_name(G_OBJECT(sheet->hadjustment),  "value_changed");
	}
	return (TRUE);
    }

    else if (GTK_SHEET_IN_YDRAG(sheet))
    {
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_YDRAG);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gdk_window_get_device_position (
            main_window, event->device, NULL, &y, NULL);
	gdk_device_ungrab (event->device, event->time);
	draw_xor_hline(sheet, TRUE);

	gtk_sheet_set_row_height(sheet, sheet->drag_cell.row,
	    new_row_height(sheet, sheet->drag_cell.row, &y));

	sheet->old_vadjustment = -1.;
	if (sheet->vadjustment)
	{
	    g_signal_emit_by_name(G_OBJECT(sheet->vadjustment), "value_changed");
	}
	return (TRUE);
    }

    else if (GTK_SHEET_IN_DRAG(sheet))  /* selection being moved */
    {
	GtkSheetRange old_range;
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_DRAG);
	gdk_device_ungrab (event->device, event->time);

	gtk_sheet_real_unselect_range(sheet, NULL);

        /* move active cell - with drag area */

        /* move selection pin - with drag area */
        SET_SELECTION_PIN(
            sheet->selection_pin.row +
            (sheet->drag_range.row0 - sheet->range.row0),
            sheet->selection_pin.col +
            (sheet->drag_range.col0 - sheet->range.col0));
        /* move selection cursor - with drag area */
        SET_SELECTION_CURSOR(
            sheet->selection_cursor.row +
            (sheet->drag_range.row0 - sheet->range.row0),
            sheet->selection_cursor.col +
	    (sheet->drag_range.col0 - sheet->range.col0));

        old_range = sheet->range;
	sheet->range = sheet->drag_range;
	sheet->drag_range = old_range;

	g_signal_emit(G_OBJECT(sheet), sheet_signals[MOVE_RANGE], 0,
	    &sheet->drag_range, &sheet->range);
	gtk_sheet_select_range(sheet, &sheet->range);
    }

    else if (GTK_SHEET_IN_RESIZE(sheet)) /* selection being resized*/
    {
	GtkSheetRange old_range;
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_RESIZE);
	gdk_device_ungrab (event->device, event->time);

	gtk_sheet_real_unselect_range(sheet, NULL);

        /* set active_cell - to drag end point  */

        /* set selection cursor - to drag end point  */
        SET_SELECTION_CURSOR(
            sheet->drag_cell.row, sheet->drag_cell.col);

	old_range = sheet->range;
	sheet->range = sheet->drag_range;
	sheet->drag_range = old_range;

	if (sheet->state == GTK_SHEET_NORMAL)
        {
            sheet->state = GTK_SHEET_RANGE_SELECTED;
        }
	g_signal_emit(G_OBJECT(sheet), sheet_signals[RESIZE_RANGE], 0,
	    &sheet->drag_range, &sheet->range);
	gtk_sheet_select_range(sheet, &sheet->range);
    }

    else if (!GTK_SHEET_IN_SELECTION(sheet)  /* not selecting */
	&& !GTK_SHEET_IN_DRAG(sheet) /* not moving selection */
	&& !GTK_SHEET_IN_RESIZE(sheet) /* not resizing selection */ )
    {
	gdk_window_get_device_position(
	    main_window, event->device, &x, &y, NULL);

	gint row, column;
	gtk_sheet_get_pixel_info(sheet, NULL, x, y, &row, &column);

#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug(
	    "%s(%d): click cell r/c %d %d ar/ac %d %d", 
	    __FUNCTION__, __LINE__, 
	    row, column,
	    sheet->active_cell.row, sheet->active_cell.col);
#endif

	gdk_device_ungrab(event->device, event->time);

	gboolean veto;
	gtk_sheet_click_cell(
	    sheet, row, column, &veto);

	if (veto)
	{
	    gtk_sheet_activate_cell(
		sheet, sheet->active_cell.row, sheet->active_cell.col);
	}
    }

    if (sheet->state == GTK_SHEET_NORMAL  /* nothing selected */
        && GTK_SHEET_IN_SELECTION(sheet))
    {
#if GTK_SHEET_DEBUG_MOUSE > 0
	    g_debug(
		"%s(%d): UNSET_FLAGS(IN_SELECTION) PIN r %d c %d ar %d ac %d",
		__FUNCTION__, __LINE__,
                sheet->selection_pin.row, sheet->selection_pin.col,
                sheet->active_cell.row, sheet->active_cell.col
                );
#endif

	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	gdk_device_ungrab (event->device, event->time);

	gtk_sheet_activate_cell(
            sheet, sheet->active_cell.row, sheet->active_cell.col);
    }

    gdk_device_ungrab(event->device, event->time);

    if (sheet->timer)
    {
        g_source_remove(sheet->timer);
        sheet->timer = 0;
    }
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
    GdkWindow *main_window = gtk_widget_get_window(widget);  // for coordinates

    /* selections on the sheet */
    x = event->x;
    y = event->y;

#if GTK_SHEET_DEBUG_MOTION > 0
    g_debug("%s(%d): (%d,%d) inSel %s inResize %s inDrag %s state mod %u but %u", 
	__FUNCTION__, __LINE__,
	x, y,
	GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No",
	event->state & GDK_MODIFIER_MASK,
	event->state & (GDK_BUTTON1_MASK
			|GDK_BUTTON2_MASK
			|GDK_BUTTON3_MASK
			|GDK_BUTTON4_MASK
			|GDK_BUTTON5_MASK)
	);
#endif

    if (event->window == sheet->column_title_window 
	&& gtk_sheet_columns_resizable(sheet))
    {
	gdk_window_get_device_position(
            main_window, event->device, &x, &y, NULL);

	if (!GTK_SHEET_IN_SELECTION(sheet)
            && POSSIBLE_XDRAG(sheet, x, &column))
	{
	    new_cursor = GDK_SB_H_DOUBLE_ARROW;
	    if (new_cursor != gdk_cursor_get_cursor_type(
                sheet->cursor_drag))
	    {
		g_object_unref(G_OBJECT(sheet->cursor_drag));

		sheet->cursor_drag = gdk_cursor_new(
                    GDK_SB_H_DOUBLE_ARROW);
		gdk_window_set_cursor(
                    sheet->column_title_window, sheet->cursor_drag);
	    }
	}
	else
	{
	    new_cursor = GDK_TOP_LEFT_ARROW;

	    if (!GTK_SHEET_IN_XDRAG(sheet)
                && new_cursor != gdk_cursor_get_cursor_type(
                    sheet->cursor_drag))
	    {
		g_object_unref(G_OBJECT(sheet->cursor_drag));

		sheet->cursor_drag = gdk_cursor_new(
                    GDK_TOP_LEFT_ARROW);
		gdk_window_set_cursor(
                    sheet->column_title_window, sheet->cursor_drag);
	    }
	}
    }

    if (event->window == sheet->row_title_window 
        && gtk_sheet_rows_resizable(sheet))
    {
	gdk_window_get_device_position (
            main_window, event->device, &x, &y, NULL);

	if (!GTK_SHEET_IN_SELECTION(sheet)
            && POSSIBLE_YDRAG(sheet, y, &column))
	{
	    new_cursor = GDK_SB_V_DOUBLE_ARROW;
	    if (new_cursor != gdk_cursor_get_cursor_type(
                sheet->cursor_drag))
	    {
		g_object_unref(G_OBJECT(sheet->cursor_drag));

		sheet->cursor_drag = gdk_cursor_new(
                    GDK_SB_V_DOUBLE_ARROW);
		gdk_window_set_cursor(
                    sheet->row_title_window, sheet->cursor_drag);
	    }
	}
	else
	{
	    new_cursor = GDK_TOP_LEFT_ARROW;

	    if (!GTK_SHEET_IN_YDRAG(sheet)
                && new_cursor != gdk_cursor_get_cursor_type(
                    sheet->cursor_drag))
	    {
		g_object_unref(G_OBJECT(sheet->cursor_drag));

		sheet->cursor_drag = gdk_cursor_new(
                    GDK_TOP_LEFT_ARROW);
		gdk_window_set_cursor(
                    sheet->row_title_window, sheet->cursor_drag);
	    }
	}
    }

    new_cursor = GDK_PLUS;
    if (!POSSIBLE_DRAG(sheet, x, y, &row, &column)
        && !GTK_SHEET_IN_DRAG(sheet)
        && !POSSIBLE_RESIZE(sheet, x, y, &row, &column)
        && !GTK_SHEET_IN_RESIZE(sheet)
        && event->window == sheet->sheet_window
        && new_cursor != gdk_cursor_get_cursor_type(
            sheet->cursor_drag))
    {
	g_object_unref(G_OBJECT(sheet->cursor_drag));

	sheet->cursor_drag = gdk_cursor_new(GDK_PLUS);
	gdk_window_set_cursor(
            sheet->sheet_window, sheet->cursor_drag);
    }

    new_cursor = GDK_TOP_LEFT_ARROW;
    if (!(POSSIBLE_RESIZE(sheet, x, y, &row, &column)
          || GTK_SHEET_IN_RESIZE(sheet))
        && (POSSIBLE_DRAG(sheet, x, y, &row, &column)
            || GTK_SHEET_IN_DRAG(sheet))
        && event->window == sheet->sheet_window
        && new_cursor != gdk_cursor_get_cursor_type(
            sheet->cursor_drag))
    {
	g_object_unref(G_OBJECT(sheet->cursor_drag));

	sheet->cursor_drag = gdk_cursor_new(GDK_TOP_LEFT_ARROW);
	gdk_window_set_cursor(
            sheet->sheet_window, sheet->cursor_drag);
    }

    new_cursor = GDK_SIZING;
    if (!GTK_SHEET_IN_DRAG(sheet)
        && (POSSIBLE_RESIZE(sheet, x, y, &row, &column)
            || GTK_SHEET_IN_RESIZE(sheet))
        && event->window == sheet->sheet_window
        && new_cursor != gdk_cursor_get_cursor_type(
            sheet->cursor_drag))
    {
	g_object_unref(G_OBJECT(sheet->cursor_drag));

	sheet->cursor_drag = gdk_cursor_new(GDK_SIZING);
	gdk_window_set_cursor(
            sheet->sheet_window, sheet->cursor_drag);
    }

    gdk_window_get_device_position (
        main_window, event->device, &x, &y, &mods);

#if GTK_SHEET_DEBUG_MOUSE > 0
    g_debug("%s(%d): check mouse button1", __FUNCTION__, __LINE__);
#endif

    if (mods & GDK_BUTTON1_MASK)
    {
#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug(
	    "%s(%d): check mouse button1 inSel %s inResize %s inDrag %s", 
	    __FUNCTION__, __LINE__,
	    GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	    GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	    GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
	    );
#endif

#if 0
	/* 303340~0049875
	   When selection mode was activated in button_press_handler on
	   main window, find 'veto' is true after call to gtk_sheet_click_cell().
	   While executing traverse handler, mouse button might have been
	   already released in another window. So we need to reset selection
	   mode here.
	   */
	if (GTK_SHEET_IN_SELECTION(sheet))
	{
	    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	}

	return (FALSE);
#else
	gint dx = x - sheet->x_drag;
	gint dy = y - sheet->y_drag;

#if GTK_SHEET_DEBUG_MOUSE > 0
	g_debug(
	    "%s(%d): check dx/dy %d/%d inSel %s inResize %s inDrag %s",
	    __FUNCTION__, __LINE__,
	    dx, dy,
	    GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	    GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	    GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
	    );
#endif

	if (!GTK_SHEET_IN_SELECTION(sheet)  /* not selecting */
	    && !GTK_SHEET_IN_DRAG(sheet) /* not moving selection */
	    && !GTK_SHEET_IN_RESIZE(sheet) /* not resizing selection */ 
	    && (abs(dx) > CLICK_CELL_MOTION_TOLERANCE
		|| abs(dy) > CLICK_CELL_MOTION_TOLERANCE))
	{
#if GTK_SHEET_DEBUG_MOUSE > 0
	    g_debug(
		"%s(%d): SET_FLAGS(IN_SELECTION)",
		__FUNCTION__, __LINE__);
#endif
	    GTK_SHEET_SET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);
	}
#endif
    }

    if (GTK_SHEET_IN_XDRAG(sheet))
    {
	if (event->is_hint || event->window != main_window)
	{
	    gdk_window_get_device_position(
                main_window, event->device, &x, NULL, NULL);
	}
	else
	    x = event->x;

	new_column_width(sheet, sheet->drag_cell.col, &x);

	if (x != sheet->x_drag)
	{
	    draw_xor_vline(sheet, FALSE);
	    sheet->x_drag = x;
	    draw_xor_vline(sheet, TRUE);
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_YDRAG(sheet))
    {
	if (event->is_hint || event->window != main_window)
	{
	    gdk_window_get_device_position(
                main_window, event->device, NULL, &y, NULL);
	}
	else
	    y = event->y;

	new_row_height(sheet, sheet->drag_cell.row, &y);

	if (y != sheet->y_drag)
	{
	    draw_xor_hline(sheet, FALSE);
	    sheet->y_drag = y;
	    draw_xor_hline(sheet, TRUE);
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_DRAG(sheet))
    {
	GtkSheetRange aux, visr;

	gint current_row = MIN(
            sheet->maxrow, _gtk_sheet_row_from_ypixel(sheet, y));
	gint current_col = MIN(
            sheet->maxcol, _gtk_sheet_column_from_xpixel(sheet, x));

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
		gint nrow0 = _gtk_sheet_first_visible_rowidx(
                    sheet, sheet->range.row0 + row);
		gint nrowi = _gtk_sheet_first_visible_rowidx(
                    sheet, sheet->range.rowi + row);

		if (nrow0 >= 0 && nrowi >= 0)
		{
		    sheet->drag_range.row0 = nrow0;
		    sheet->drag_range.rowi = nrowi;
		}
	    }
	    else
	    {
		gint nrow0 = _gtk_sheet_last_visible_rowidx(
                    sheet, sheet->range.row0 + row);
		gint nrowi = _gtk_sheet_last_visible_rowidx(
                    sheet, sheet->range.rowi + row);

		if (nrow0 >= 0 && nrowi >= 0)
		{
		    sheet->drag_range.row0 = nrow0;
		    sheet->drag_range.rowi = nrowi;
		}
	    }

	    if (column > 0)
	    {
		gint ncol0 = _gtk_sheet_first_visible_colidx(
                    sheet, sheet->range.col0 + column);
		gint ncoli = _gtk_sheet_first_visible_colidx(
                    sheet, sheet->range.coli + column);

		if (ncol0 >= 0 && ncoli >= 0)
		{
		    sheet->drag_range.col0 = ncol0;
		    sheet->drag_range.coli = ncoli;
		}
	    }
	    else
	    {
		gint ncol0 = _gtk_sheet_last_visible_colidx(
                    sheet, sheet->range.col0 + column);
		gint ncoli = _gtk_sheet_last_visible_colidx(
                    sheet, sheet->range.coli + column);

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
                gtk_sheet_draw_border(sheet, sheet->drag_range, NULL);
	    }
	}
	return (TRUE);
    }

    if (GTK_SHEET_IN_RESIZE(sheet))
    {
	GtkSheetRange aux, visr;
	gint current_col, current_row, col_threshold, row_threshold;

	g_assert(0 <= sheet->drag_cell.row
            && sheet->drag_cell.row <= sheet->maxrow);
	g_assert(0 <= sheet->drag_cell.col
            && sheet->drag_cell.col <= sheet->maxcol);

	current_row = MIN(
            sheet->maxrow, _gtk_sheet_row_from_ypixel(sheet, y));
	current_col = MIN(
            sheet->maxcol, _gtk_sheet_column_from_xpixel(sheet, x));

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
	    current_row = _gtk_sheet_last_visible_rowidx(
                sheet, current_row - 1);
	else if (current_row < sheet->drag_range.row0
                 && y < row_threshold)
	    current_row = _gtk_sheet_first_visible_rowidx(
                sheet, current_row + 1);

	col_threshold = _gtk_sheet_column_left_xpixel(sheet, current_col);
	if (current_col >= 0)
	    col_threshold += (COLPTR(sheet, current_col)->width) / 2;

	if (current_col > sheet->drag_range.col0 && x < col_threshold)
	    current_col = _gtk_sheet_last_visible_colidx(
                sheet, current_col - 1);
	else if (current_col < sheet->drag_range.col0 && x > col_threshold)
	    current_col = _gtk_sheet_first_visible_colidx(
                sheet, current_col + 1);

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
                gtk_sheet_draw_border(sheet, sheet->drag_range, NULL);
	    }
	}
	return (TRUE);
    }

    gtk_sheet_get_pixel_info(sheet, NULL, x, y, &row, &column);

    if (sheet->state == GTK_SHEET_NORMAL
	&& row == sheet->active_cell.row
        && column == sheet->active_cell.col)
    {
	return (TRUE);
    }

    if (GTK_SHEET_IN_SELECTION(sheet)
        && (mods & GDK_BUTTON1_MASK))
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
#if GTK_SHEET_DEBUG_SELECTION > 0
    g_debug("%s(%d): row %d column %d inSel %s inResize %s inDrag %s", 
	__FUNCTION__, __LINE__,
	row, column,
	GTK_SHEET_IN_SELECTION(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_RESIZE(sheet) ? "Yes" : "No",
	GTK_SHEET_IN_DRAG(sheet) ? "Yes" : "No"
	);
#endif

    if (sheet->selection_mode == GTK_SELECTION_SINGLE) return;

    /* compare current row/col with selection cursor */
    if (row == sheet->selection_cursor.row
        && column == sheet->selection_cursor.col)
	return;

    gint old_state = sheet->state;

    _gtk_sheet_move_query(sheet, row, column, FALSE);
    gtk_widget_grab_focus(GTK_WIDGET(sheet));

    if (gtk_widget_get_realized(GTK_WIDGET(sheet))
        && old_state == GTK_SHEET_NORMAL)
    {
        gboolean veto = _gtk_sheet_deactivate_cell(sheet);
        // there are 2 possible reasons for veto
        // no active_cell or DEACTIVATE signal returns veto
        if (!veto) return;
        // deactivate will set active_cell := -1
    }

    if (GTK_SHEET_IN_DRAG(sheet)) return;

    // use swin_cr to blit sheet->bsurf into sheet_window
    cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);
    cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);

    switch(old_state)
    {
	case GTK_SHEET_ROW_SELECTED:
	    column = sheet->maxcol;
	    break;

	case GTK_SHEET_COLUMN_SELECTED:
	    row = sheet->maxrow;
	    break;

        case GTK_SHEET_NORMAL:
            sheet->state = GTK_SHEET_RANGE_SELECTED;
	    gtk_sheet_range_draw_selection(sheet, sheet->range, NULL);
	    /* FALLTHROUGH */

	case GTK_SHEET_RANGE_SELECTED:
	    sheet->state = GTK_SHEET_RANGE_SELECTED;
	    /* FALLTHROUGH */
    }

    cairo_destroy(swin_cr);

    /* set selection cursor - to current row/col */
    SET_SELECTION_CURSOR(row, column);

    GtkSheetRange range;
    range.col0 = MIN(column, sheet->selection_pin.col);
    range.coli = MAX(column, sheet->selection_pin.col);

    range.row0 = MIN(row, sheet->selection_pin.row);
    range.rowi = MAX(row, sheet->selection_pin.row);

    /* limits */
    range.coli = MIN(range.coli, sheet->maxcol);
    range.rowi = MIN(range.rowi, sheet->maxrow);

    if (range.row0 != sheet->range.row0
        || range.rowi != sheet->range.rowi
        || range.col0 != sheet->range.col0
        || range.coli != sheet->range.coli
        )
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
gtk_sheet_entry_key_press_handler(
    GtkWidget *widget, GdkEventKey *key, gpointer user_data)
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
    if ((key->keyval == GDK_KEY_Return)
        || (key->keyval == GDK_KEY_KP_Enter))
    {
#if GTK_SHEET_DEBUG_ENTER_PRESSED > 0
	g_debug("gtk_sheet_entry_key_press_handler: enter-pressed: invoke");
#endif
	_gtksheet_signal_emit(G_OBJECT(sheet),
	    sheet_signals[ENTER_PRESSED],
	    key,
	    &stop_emission);

#if GTK_SHEET_DEBUG_ENTER_PRESSED > 0
	g_debug("gtk_sheet_entry_key_press_handler: enter-pressed: returns %d",
            stop_emission);
#endif
    }
#endif

    if (!stop_emission)
    {
	/* intercept item_entry processing if there exists a key binding on the sheet */

	if (gtk_bindings_activate_event(G_OBJECT(sheet), key))
	{
	    stop_emission = TRUE;
	}
	else
	{
	    g_signal_emit_by_name(G_OBJECT(widget),
		"key_press_event", key,
		&stop_emission);
	}
    }

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug(
        "gtk_sheet_entry_key_press_handler: done, key %s stop %d",
	gtk_accelerator_name(key->keyval, key->state),
        stop_emission);
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
    GtkSheet *sheet = GTK_SHEET(widget);

    GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_SELECTION);

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug("gtk_sheet_key_press_handler: key %s",
	gtk_accelerator_name(key->keyval, key->state));
#endif

    /* if there is a key_binding,
       use implementation from _gtk_sheet_move_cursor() */

    if (_gtk_sheet_binding_filter(sheet, key)
	&& gtk_bindings_activate_event(G_OBJECT(sheet), key))
    {
#if GTK_SHEET_DEBUG_KEYPRESS > 0
	g_debug("gtk_sheet_key_press_handler: done %s (binding)",
	    gtk_accelerator_name(key->keyval, key->state));
#endif
	return (TRUE);  /* stop emission */
    }

    return (FALSE);  /* propagate emission */
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
    gboolean veto = TRUE;

#if GTK_SHEET_DEBUG_SIGNALS > 0
    g_debug("SIGNAL _gtk_sheet_move_cursor %p step %d count %d extend %d",
	sheet, step, count, extend_selection);
#endif

    gint row = sheet->active_cell.row;
    gint col = sheet->active_cell.col;

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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto) break;
		    }
		    if (sheet->selection_cursor.row > 0)
		    {
                        /* move relative to selection cursor */
			row = sheet->selection_cursor.row + count;
			_HUNT_VISIBLE_UP(row);
			gtk_sheet_extend_selection(
			    sheet, row, sheet->selection_cursor.col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.row < sheet->maxrow)
		    {
			row = sheet->selection_cursor.row + count;
			_HUNT_VISIBLE_DOWN(row);
			gtk_sheet_extend_selection(
			    sheet, row, sheet->selection_cursor.col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.col > 0)
		    {
                        /* move relative to selection cursor */
			col = sheet->selection_cursor.col + count;
			_HUNT_VISIBLE_LEFT(col);
			gtk_sheet_extend_selection(
                            sheet, sheet->selection_cursor.row, col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.col < sheet->maxcol)
		    {
                        /* move relative to selection cursor */
			col = sheet->selection_cursor.col + count;
			_HUNT_VISIBLE_RIGHT(col);
			gtk_sheet_extend_selection(
                            sheet, sheet->selection_cursor.row, col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.row > 0)
		    {
			row = 0;
			_HUNT_VISIBLE_UP(row);
			gtk_sheet_extend_selection(
                            sheet, row, sheet->selection_cursor.col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.row < sheet->maxrow)
		    {
			row = sheet->maxrow;
			_HUNT_VISIBLE_DOWN(row);
			gtk_sheet_extend_selection(sheet, row, sheet->selection_cursor.col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.col > 0)
		    {
			col = 0;
			_HUNT_VISIBLE_LEFT(col);
			gtk_sheet_extend_selection(sheet, sheet->selection_cursor.row, col);
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
		    if (sheet->state == GTK_SHEET_NORMAL)
		    {
			gtk_sheet_click_cell(sheet, row, col, &veto);
			if (!veto)
			    break;
		    }
		    if (sheet->selection_cursor.col < sheet->maxcol)
		    {
			col = sheet->maxcol;
			_HUNT_VISIBLE_RIGHT(col);
			gtk_sheet_extend_selection(sheet, sheet->selection_cursor.row, col);
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
 * gtk_sheet_size_request:
 * 
 * this is the #GtkSheet widget class "size-request" signal handler
 * 
 * @param widget the #GtkSheet
 * @param requisition
 *               the #GtkRequisition
 */
static void
gtk_sheet_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    GtkSheet *sheet;
    GList *children;
    GtkSheetChild *child;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));
    g_return_if_fail(requisition != NULL);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("gtk_sheet_size_request: called");
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

	gtk_widget_get_preferred_size(child->widget, NULL, NULL);
    }

    /* fix for
       **: GtkDataEntry is drawn without a current allocation.
       */
    _gtk_sheet_entry_size_allocate(sheet);
}

static void
gtk_sheet_get_preferred_width (GtkWidget *widget,
    gint *minimal_width, gint *natural_width)
{
  GtkRequisition requisition;
  gtk_sheet_size_request(widget, &requisition);

  *minimal_width = *natural_width = requisition.width;
}

static void
gtk_sheet_get_preferred_height (GtkWidget *widget,
    gint *minimal_height, gint *natural_height)
{
  GtkRequisition requisition;
  gtk_sheet_size_request(widget, &requisition);

  *minimal_height = *natural_height = requisition.height;
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
gtk_sheet_size_allocate_handler(
    GtkWidget *widget, GtkAllocation *allocation)
{
    gboolean modified;

    g_return_if_fail(widget != NULL);
    g_return_if_fail(GTK_IS_SHEET(widget));
    g_return_if_fail(allocation != NULL);

    if (!gtk_widget_get_mapped(widget)) return;

    GtkSheet *sheet = GTK_SHEET(widget);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("%s(%d): called %s %p %s mapped %d (%d, %d, %d, %d)",
        __FUNCTION__, __LINE__,
        G_OBJECT_TYPE_NAME(sheet), sheet,
        gtk_widget_get_name(GTK_WIDGET(sheet)), 
        gtk_widget_get_mapped(GTK_WIDGET(sheet)), 
	allocation->x, allocation->y,
        allocation->width, allocation->height);
#endif

    /* main window - allocation and size */

    gtk_widget_set_allocation(widget, allocation);

    gint border_width = gtk_container_get_border_width(
        GTK_CONTAINER(widget));

    if (gtk_widget_get_realized(widget))
    {
        /* note: x,y determines position of all contents */

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

    /* sheet_window - allocation and size */

    GtkAllocation sheet_allocation;
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
        /* note: x,y determines global sheet button position only */

	gdk_window_move_resize(sheet->sheet_window,
	    sheet_allocation.x,
	    sheet_allocation.y,
	    sheet_allocation.width,
	    sheet_allocation.height);
    }

    /* column_title_window - allocation and size */

    sheet->column_title_area.x = 0;
    sheet->column_title_area.y = 0;

    if (sheet->row_titles_visible)
	sheet->column_title_area.x = sheet->row_title_area.width;

    sheet->column_title_area.width = sheet_allocation.width
        - sheet->column_title_area.x;

    if (gtk_widget_get_realized(widget)
        && sheet->column_titles_visible)
    {
	gdk_window_move_resize(sheet->column_title_window,
	    sheet->column_title_area.x,
	    sheet->column_title_area.y,
	    sheet->column_title_area.width,
	    sheet->column_title_area.height);
    }

    /* row_title_window - allocation and size */

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

    _gtk_sheet_recalc_view_range(sheet);

    /* column button allocation */
    _gtk_sheet_column_buttons_size_allocate(sheet);

    /* row button allocation */
    _gtk_sheet_row_buttons_size_allocate(sheet);

    if (gtk_sheet_autoresize(sheet)
        && (modified
            || (GTK_SHEET_FLAGS(sheet) & GTK_SHEET_IN_AUTORESIZE_PENDING)))
    {
	/* autoresize here, because window was changed -> max col_width */
	gtk_sheet_autoresize_all(sheet);
	GTK_SHEET_UNSET_FLAGS(sheet, GTK_SHEET_IN_AUTORESIZE_PENDING);
    }

    /* re-scale backing bsurf */
    gtk_sheet_make_bsurf(sheet, 0, 0);  /* use default size */
    _gtk_sheet_position_children(sheet);

    /* set the scrollbars adjustments */
    _gtk_sheet_scrollbar_adjust(sheet);
}

static gboolean
gtk_sheet_focus(GtkWidget *widget, GtkDirectionType  direction)
{
    g_return_val_if_fail(GTK_IS_SHEET(widget), FALSE);
    GtkSheet *sheet = GTK_SHEET(widget);

    if (!gtk_widget_is_sensitive(GTK_WIDGET(sheet))) {
	g_debug("gtk_sheet_focus: sheet not sensitive"); 
	return(FALSE);
    }

#if GTK_SHEET_DEBUG_KEYPRESS > 0
    g_debug("gtk_sheet_focus: %p %d", widget, direction); 
#endif

    if (!gtk_widget_has_focus (widget))
    {
	gtk_widget_grab_focus (widget);
    }

    gint act_row = sheet->active_cell.row;
    gint act_col = sheet->active_cell.col;

    if (act_row < 0 || act_col < 0)  /* not in sheet */
    {
        /* initiate focus hunt when the sheet is shown */

        /* starting with -1,-1 this will only work on second attempt:
           - first attempt will hunt to row -1 col 0
           - second attempt will hunt to row 0 col 0
         
           if you want to have focus on expose,
           use gtk_sheet_set_active_cell() after sheet creation
           */

#if GTK_SHEET_ENABLE_FOCUS_ON_SHOW > 0
	_gtk_sheet_move_cursor(
            sheet, GTK_MOVEMENT_VISUAL_POSITIONS, 1, FALSE);
#endif
	return(TRUE);
    }

    gboolean veto;

    gtk_sheet_click_cell(sheet, act_row, act_col, &veto);
    if (!veto) return(FALSE);
	
    return(TRUE);
}


static void
_gtk_sheet_row_buttons_size_allocate(GtkSheet *sheet)
{
    gint row, y, height;
    GdkRectangle *rta = &sheet->row_title_area;

    if (!sheet->row_titles_visible)
	return;
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

#if GTK_SHEET_DEBUG_ALLOCATION > 1
    g_debug("size_allocate_row_title_buttons: called");
#endif

    height = sheet->sheet_window_height;
    y = 0;

    if (sheet->column_titles_visible)
    {
        /* negative height can result in pixman invalid rectangle warnings */ 
        if (height >= sheet->column_title_area.height)
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

	cairo_t *rta_cr = gdk_cairo_create(sheet->row_title_window);
	gdk_cairo_set_source_rgba(rta_cr, &sheet->bg_color);
	//cairo_paint(rta_cr);
	cairo_destroy(rta_cr);
    }

    if (!gtk_widget_is_drawable(GTK_WIDGET(sheet)))
	return;

    for (row = MIN_VIEW_ROW(sheet);
          row <= MAX_VIEW_ROW(sheet) && row <= sheet->maxrow;
          row++) 
    {
        _gtk_sheet_draw_button(sheet, row, -1, NULL);
    }
}

/**
 * gtk_sheet_recalc_top_ypixels:
 * 
 * for all rows: recalculate top_ypixel position, vis_row_index 
 * and sheet->min_vis_row_index, sheet->max_vis_row_index 
 * 
 * @param sheet  the #GtkSheet
 */
void
_gtk_sheet_recalc_top_ypixels(GtkSheet *sheet)
{
    gint row, cy;
    gint min_vis_row = -1;
    gint max_vis_row = -1;

    if (sheet->column_titles_visible)
	cy = sheet->column_title_area.height;
    else
	cy = 0;

    for (row = 0; row <= sheet->maxrow; row++)
    {
	GtkSheetRow *rowptr = ROWPTR(sheet, row);

	rowptr->top_ypixel = cy;

	if (GTK_SHEET_ROW_IS_VISIBLE(rowptr))
        {
            cy += rowptr->height;

            ++max_vis_row;
            if (min_vis_row < 0) min_vis_row = max_vis_row;
            rowptr->vis_row_index = max_vis_row;
        }
        else
        {
            rowptr->vis_row_index = -1;
        }
    }
    sheet->min_vis_row_index = min_vis_row;
    sheet->max_vis_row_index = max_vis_row;
}

/**
 * _gtk_sheet_recalc_left_xpixels:
 * 
 * for all columns: 
 * recalculate left_xpixel position, vis_col_index 
 * and sheet->min_vis_col_index, sheet->max_vis_col_index
 *  
 * @param sheet  the #GtkSheet
 */
void
_gtk_sheet_recalc_left_xpixels(GtkSheet *sheet)
{
    gint col, cx;
    gint min_vis_col = -1;
    gint max_vis_col = -1;

    if (sheet->row_titles_visible)
	cx = sheet->row_title_area.width;
    else
	cx = 0;

    for (col = 0; col <= sheet->maxcol; col++)
    {
        GtkSheetColumn *colobj = COLPTR(sheet, col);

	colobj->left_xpixel = cx;

	if (GTK_SHEET_COLUMN_IS_VISIBLE(colobj))
        {
            cx += colobj->width;

            ++max_vis_col;
            if (min_vis_col < 0) min_vis_col = max_vis_col;
            colobj->vis_col_index = max_vis_col;
        }
        else
        {
            colobj->vis_col_index = -1;
        }
    }
    sheet->min_vis_col_index = min_vis_col;
    sheet->max_vis_col_index = max_vis_col;
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
    int col;

    g_assert(start_column >= -1);

    for (col = start_column + 1; col <= sheet->maxcol; col++)  /* for the fresh columns */
    {
	GtkSheetColumn *colptr = COLPTR(sheet, col);

	colptr->left_text_column = col;
	colptr->right_text_column = col;
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
    GtkRequisition minimum_size, natural_size;
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

    gtk_widget_get_preferred_size (widget, &minimum_size, &natural_size);
    gtk_widget_get_allocation(widget, &allocation);

    if (x)
	*x = allocation.x;

    if (y)
    {
	if (ENTRY_IS_CELL_RENDERER)
	    *y = allocation.y;
	else
	    *y = allocation.y + (allocation.height - minimum_size.height) / 2;
    }

    if (width)
	*width = allocation.width;

    if (height)
    {
	if (ENTRY_IS_CELL_RENDERER)
	    *height = allocation.height;
	else
	    *height = minimum_size.height;
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
    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;
    if (!gtk_widget_get_mapped(GTK_WIDGET(sheet))) return;
    if (sheet->maxrow < 0 || sheet->maxcol < 0) return;
    if (!sheet->sheet_entry) return;  /* PR#102114 */

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_entry_size_allocate: called");
#endif

    //GtkWidget *entry_widget = gtk_sheet_get_entry(sheet);

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
        /* shut down warnings */
	gtk_widget_get_preferred_size(sheet->sheet_entry, NULL, NULL);
    }

    gint entry_max_size = 0;
    //if (GTK_IS_ITEM_ENTRY(entry_widget)) 
	//entry_max_size = GTK_ITEM_ENTRY(entry_widget)->text_max_size;

    gint act_row = sheet->active_cell.row;
    gint act_col = sheet->active_cell.col;

    guint text_width = 0;
    guint text_height = 0;

    {
        gboolean is_markup, is_modified;

        gchar *text = _gtk_sheet_get_entry_text_internal(
            sheet, act_row, act_col, &is_markup, &is_modified);

	if (text && text[0])
	{
	    if (attributes.deprecated_font_desc_used)
	    {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
		g_warning("%s(%d): deprecated_font_desc_used %s %s row %d col %d",
		    __FUNCTION__, __LINE__,
		    G_OBJECT_TYPE_NAME(sheet),
		    gtk_widget_get_name(GTK_WIDGET(sheet)),
		    act_row, act_col);
#endif
	    }

	    _get_string_extent(sheet, 
		(0 <= act_col && act_col <= sheet->maxcol)
		? COLPTR(sheet, act_col) : NULL,
		attributes.font_desc, 
                text, is_markup,
                &text_width, &text_height);
	}

	g_free(text);
    }

    gint column_width;

    if (0 <= act_col && act_col <= sheet->maxcol)
	column_width = COLPTR(sheet, act_col)->width;
    else
	column_width = GTK_SHEET_COLUMN_DEFAULT_WIDTH;

    gint row_height;

    if (0 <= act_row && act_row <= sheet->maxrow)
	row_height = sheet->row[act_row].height;
    else
	row_height = GTK_SHEET_ROW_DEFAULT_HEIGHT;

    gint size = MIN(text_width, entry_max_size);
    size = MAX(size, column_width - CELLOFFSET*2 - 1);

    GtkAllocation shentry_allocation;
    shentry_allocation.x = _gtk_sheet_column_left_xpixel(sheet, act_col);
    shentry_allocation.y = _gtk_sheet_row_top_ypixel(sheet, act_row);
    shentry_allocation.width = column_width;
    shentry_allocation.height = row_height;

#define DEBUG_VSHIFT 0  /* vertical entry shift, use 0 or ~10 */

    if ( GTK_IS_DATA_TEXT_VIEW(sheet->sheet_entry)
	     || GTK_IS_TEXT_VIEW(sheet->sheet_entry) )
    {
#if GTK_SHEET_DEBUG_SIZE > 0
	g_debug("_gtk_sheet_entry_size_allocate: is_text_view");
#endif

	shentry_allocation.x += CELLOFFSET_NF;
	shentry_allocation.y += CELLOFFSET_NF;
        shentry_allocation.height -= CELLOFFSET*2 + 1 - DEBUG_VSHIFT;

	if (gtk_sheet_clip_text(sheet))
	    shentry_allocation.width = column_width - 1*2;
	else  /* text extends multiple cells */
	    shentry_allocation.width = size;
    }
    else
    {
	shentry_allocation.x += CELLOFFSET_WF;
	shentry_allocation.y += CELLOFFSET_WF;

	shentry_allocation.width -= MIN(
            shentry_allocation.width, CELLOFFSET_WF*2 + 1);

	shentry_allocation.height -= MIN(
            shentry_allocation.height, CELLOFFSET_WF*2) - DEBUG_VSHIFT;
    }

    /* vertical justification*/
    if (0 <= act_col && act_col <= sheet->maxcol)
    {
        GtkSheetVerticalJustification vjust = COLPTR(sheet, act_col)->vjust;

        if (vjust == GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT)
            vjust = sheet->vjust;

        switch(vjust)
        {
            case GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT:
            case GTK_SHEET_VERTICAL_JUSTIFICATION_TOP:
                gtk_widget_set_valign(sheet->sheet_entry, GTK_ALIGN_START);
                break;

            case GTK_SHEET_VERTICAL_JUSTIFICATION_MIDDLE:
                gtk_widget_set_valign(sheet->sheet_entry, GTK_ALIGN_CENTER);
                break;

            case GTK_SHEET_VERTICAL_JUSTIFICATION_BOTTOM:
                gtk_widget_set_valign(sheet->sheet_entry, GTK_ALIGN_END);
                break;
        }
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

#if 0
    {
        /* the following code shuts down warnings like
           ** attempt to underallocate GtkSheet's child GtkDataEntry 
           but leads to HUGE and UGLY cell editor overlays
         
           it can also be shut down by undef of
           G_ENABLE_CONSISTENCY_CHECKS
           */

        GtkRequisition minimum;

        gtk_widget_get_preferred_size (
            sheet->sheet_entry, &minimum, NULL);

#if GTK_SHEET_DEBUG_SIZE >0
        g_debug("_gtk_sheet_entry_size_allocate: minw %d minh %d)",
            minimum.width, minimum.height);
#endif
        
        /* check minimum, center and resize */
        if (shentry_allocation.width < minimum.width)
        {
            gint diff = minimum.width - shentry_allocation.width;
            shentry_allocation.x -= diff/2;
            shentry_allocation.width = minimum.width;
        }

        if (shentry_allocation.height < minimum.height)
        {
            gint diff = minimum.height - shentry_allocation.height;
            shentry_allocation.y -= diff/2;
            shentry_allocation.height = minimum.height;
        }
    }
#endif

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("%s(%d) %s %p alloc x %d y %d w %d h %d", 
        __FUNCTION__, __LINE__, 
        G_OBJECT_TYPE_NAME (sheet->sheet_entry), sheet->sheet_entry,
        shentry_allocation.x, shentry_allocation.y, 
        shentry_allocation.width, shentry_allocation.height);
#endif

    gtk_widget_size_allocate(sheet->sheet_entry, &shentry_allocation);

#if GTK_SHEET_DEBUG_SIZE > 0
    g_debug("_gtk_sheet_entry_size_allocate: returned (%d,%d,%d,%d)",
	shentry_allocation.x, shentry_allocation.y, 
        shentry_allocation.width, shentry_allocation.height);
#endif

    if (gtk_widget_get_mapped(sheet->sheet_entry))
    {
        GdkRectangle *rta = &sheet->row_title_area;

        /* clip on row_title_area */
        if (sheet->row_titles_visible
            && shentry_allocation.x < rta->width)
        {
            shentry_allocation.x = rta->width;
        }
        gtk_widget_set_clip(sheet->sheet_entry, &shentry_allocation);
    }
}

#if GTK_SHEET_DEBUG_FINALIZE > 0
/* obj_ref debug code */
static void weak_notify(gpointer data, GObject *o)
{
    gchar *msg = data;  /* assume a string was passed as data */

    g_debug("weak_notify: %p finalized (%s)", o, msg ? msg : "");
}
#endif

static gboolean sheet_entry_focus_in_handler(GtkWidget *widget,
    GdkEventFocus *event, gpointer user_data)
{
    gboolean retval = FALSE;
    g_signal_emit(G_OBJECT(widget),
	sheet_signals[ENTRY_FOCUS_IN], 0, event, &retval);
    return (retval);
}

static gboolean sheet_entry_focus_out_handler(GtkWidget *widget,
    GdkEventFocus *event, gpointer user_data)
{
    gboolean retval = FALSE;
    g_signal_emit(G_OBJECT(widget),
	sheet_signals[ENTRY_FOCUS_OUT], 0, event, &retval);
    return (retval);
}

static void sheet_entry_populate_popup_handler(GtkWidget *widget,
    GtkMenu *menu, gpointer user_data)
{
#if GTK_SHEET_DEBUG_SIGNALS > 0
    g_debug("sheet_entry_populate_popup_handler: menu %p", menu);
#endif

    g_signal_emit(G_OBJECT(widget),
	sheet_signals[ENTRY_POPULATE_POPUP], 0, menu);
}

/** 
 * create_sheet_entry: 
 * create or change sheet entry 
 * 
 * - destroy existing sheet entry
 * - create sheet entry widget
 * - connect "focus-in-event" handler
 * - connect "focus-out-event" handler
 * - connect "popuplate-popup" handler
 * - fall back to default entry type on failure
 * - set parent and realize entry widget
 * - connect "key-press-event" handler
 * 
 * Does not connect "changed" event handler, 
 * this will be done in gtk_sheet_activate_cell().
 * 
 * Use or G_TYPE_NONE to connect default entry type.
 * 
 * @param sheet  the sheet
 * @param new_entry_type
 *               the new entry type
 */
static void
create_sheet_entry(
    GtkSheet *sheet, GType new_entry_type)
{
    GtkWidget *entry, *new_entry;

#if GTK_SHEET_DEBUG_ENTRY > 0
    g_debug("%s(%d): entry_type %d %s",
        __FUNCTION__, __LINE__, new_entry_type, g_type_name(new_entry_type));
#endif

    //GtkStyle *style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(sheet)));

    if (sheet->sheet_entry)
    {
	/* avoids warnings */
	g_object_ref(sheet->sheet_entry);
	gtk_widget_unparent(sheet->sheet_entry);

#if GTK_SHEET_DEBUG_ENTRY > 0
        g_debug("%s(%d): destroying old entry %p", 
            __FUNCTION__, __LINE__, sheet->sheet_entry);
#endif
	gtk_widget_destroy(sheet->sheet_entry);
	sheet->sheet_entry = NULL;
    }

    if (new_entry_type == G_TYPE_NONE) 
        new_entry_type = DEFAULT_ENTRY_TYPE; /* fallback to default entry type */

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

#if 0
    /* the next statement kills default sheet entry, deactivated 06.02.22/fp 309343 */
    sheet->entry_type = new_entry_type;
#endif

    entry = gtk_sheet_get_entry(sheet);

    if (!entry)  /* this was an unsupported entry type */
    {
	g_warning("Unsupported entry type - widget must contain an GtkEditable or GtkTextView");
	gtk_widget_destroy(new_entry);

	new_entry = GTK_WIDGET(gtk_data_entry_new());
	sheet->sheet_entry = new_entry;
	sheet->installed_entry_type = DEFAULT_ENTRY_TYPE; /* fallback to default entry type */
    }
    g_object_ref_sink(sheet->sheet_entry);

#if GTK_SHEET_DEBUG_FINALIZE > 0
    g_object_weak_ref(
        G_OBJECT(sheet->sheet_entry), weak_notify, "Sheet-Entry");
#endif

    if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
    {
	gtk_widget_get_preferred_size(sheet->sheet_entry, NULL, NULL);

#if GTK_SHEET_DEBUG_ENTRY > 0
	g_debug("create_sheet_entry: sheet was realized");
#endif
        DEBUG_WIDGET_SET_PARENT_WIN(
            sheet->sheet_entry, sheet->sheet_window);
	gtk_widget_set_parent_window(
            sheet->sheet_entry, sheet->sheet_window);

        DEBUG_WIDGET_SET_PARENT(sheet->sheet_entry, sheet);
	gtk_widget_set_parent(sheet->sheet_entry, GTK_WIDGET(sheet));

	gtk_widget_realize(sheet->sheet_entry);
    }

    g_signal_connect_swapped(G_OBJECT(entry), "key_press_event",
	(void *)gtk_sheet_entry_key_press_handler,
	G_OBJECT(sheet));

#if 0
    /* deactivated 06.02.22/fp 309343 */
    g_debug("%s(%d): calling gtk_sheet_show_active_cell", __FUNCTION__, __LINE__);
    gtk_sheet_show_active_cell(sheet);
#endif
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
 * Returns: (transfer none): a #GtkWidget or NULL
 */
GtkWidget *
gtk_sheet_get_entry(GtkSheet *sheet)
{
    GtkWidget *parent;
    GtkWidget *entry = NULL;

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

    GList *children = NULL;

    if (GTK_IS_CONTAINER(parent))
	children = gtk_container_get_children(GTK_CONTAINER(parent));

    if (!children)
	return (NULL);

    while (children)
    {
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
 * Returns: (transfer none): a #GtkWidget or NULL
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
    gboolean is_markup, is_modified;

    gchar *text = _gtk_sheet_get_entry_text_internal(
        sheet, sheet->active_cell.row, sheet->active_cell.col,
        &is_markup, &is_modified);

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
 * widgets implement the GtkEditable interface. 
 */
void gtk_sheet_set_entry_text(GtkSheet *sheet, const gchar *text)
{
    _gtk_sheet_set_entry_text_internal(sheet, text, FALSE);
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
void gtk_sheet_entry_select_region(
    GtkSheet *sheet, gint start_pos, gint end_pos)
{
    GtkWidget *entry = NULL;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!sheet->sheet_entry)   /* PR#102114 */
	return;

    //g_debug("gtk_sheet_entry_select_region(): called");

    entry = gtk_sheet_get_entry(sheet);
    g_return_if_fail(entry != NULL);

    if (GTK_IS_EDITABLE(entry))
    {
	gboolean res = gtk_editable_get_selection_bounds(
	    GTK_EDITABLE(entry), NULL, NULL);
	//g_debug("gtk_editable_get_selection_bounds() = %d", res);

	if (!res) {
	    gtk_editable_select_region(
		GTK_EDITABLE(entry), start_pos, end_pos);
	}
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
 * @handler: (scope notified): the signal handler
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
gulong gtk_sheet_entry_signal_connect_changed(
    GtkSheet *sheet, GCallback handler)
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
	handler_id = g_signal_connect(
            G_OBJECT(entry), "changed", handler, G_OBJECT(sheet));
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry) 
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(entry));

	handler_id = g_signal_connect(
            G_OBJECT(buffer), "changed", handler, G_OBJECT(sheet));
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
 * @handler: (scope call): the signal handler
 *
 * Disconnect a handler from the sheet_entry "changed" signal
 *  
 * This function is mainly used to synchronize a second entry 
 * widget with the sheet_entry. 
 *  
 * This function is necessary, because not all possible entry 
 * widgets implement the GtkEditable interface yet. 
 */
void gtk_sheet_entry_signal_disconnect_by_func(
    GtkSheet *sheet, GCallback handler)
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
	g_signal_handlers_disconnect_by_func(
            G_OBJECT(entry), handler, G_OBJECT(sheet));
    }
    else if ( GTK_IS_DATA_TEXT_VIEW(entry)
	     || GTK_IS_TEXT_VIEW(entry) )
    {
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(
            GTK_TEXT_VIEW(entry));

	g_signal_handlers_disconnect_by_func(
            G_OBJECT(buffer), handler, G_OBJECT(sheet));
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
    if (row < 0 || row > sheet->maxrow) return;

    GtkSheetButton *button = &sheet->row[row].button;

    if (button->state & GTK_STATE_FLAG_ACTIVE) return;

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("row_button_set: row %d", row);
#endif

    button->state |= GTK_STATE_FLAG_ACTIVE;
    _gtk_sheet_draw_button(sheet, row, -1, NULL);
}

static void
row_button_release(GtkSheet *sheet, gint row)
{
    if (row < 0 || row > sheet->maxrow) return;

    GtkSheetButton *button = &sheet->row[row].button;
 
    if (!(button->state & GTK_STATE_FLAG_ACTIVE)) return;

#if GTK_SHEET_DEBUG_DRAW > 0
    g_debug("row_button_release: row %d", row);
#endif

    button->state &= ~GTK_STATE_FLAG_ACTIVE;
    _gtk_sheet_draw_button(sheet, row, -1, NULL);
}

/** 
 * _gtk_sheet_draw_button: 
 * draw a sheet button 
 *  
 * * if row == -1 draw a column button 
 * * if col == -1 draw a row button 
 * * not used for global sheet button 
 * 
 * @param sheet  the #GtkSheet
 * @param row    row index
 * @param col    column index
 * @param cr     cairo context or NULL
 */
void
_gtk_sheet_draw_button(
    GtkSheet *sheet, gint row, gint col, cairo_t *cr)
{
    GdkWindow *window = NULL;
    guint width = 0, height = 0;
    gint x = 0, y = 0;
    gint index = 0;
    GtkSheetButton *button = NULL;
    GtkSheetChild *child = NULL;
    GdkRectangle allocation;
    gchar * label, label_buf[10];
    PangoAlignment pango_alignment = PANGO_ALIGN_LEFT;
    GtkSheetArea area = ON_SHEET_BUTTON_AREA;
    PangoFontDescription *font_desc = NULL;
    PangoRectangle extent;
    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
        GTK_WIDGET(sheet));

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    if ((row == -1) && (col == -1))  /* not for global sheet button*/
        return;

#if GTK_SHEET_DEBUG_DRAW_BUTTON > 0
    g_debug("%s(%d): row %d col %d", 
        __FUNCTION__, __LINE__, row, col);
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

    if (row == -1)  /* column title button */
    {
	if (!cr) return;
	
	g_assert(0 <= col && col <= sheet->maxcol);
	GtkSheetColumn *colobj = COLPTR(sheet, col);

	if (colobj->col_button)
	{
            if (gtk_widget_get_realized(GTK_WIDGET(sheet))
                 && gtk_widget_get_realized(colobj->col_button))
            {
                gtk_container_propagate_draw(
                    GTK_CONTAINER(colobj), colobj->col_button, cr);
            }
	}

	return;
    }
    else if (col == -1)  /* row title button */
    {
	window = sheet->row_title_window;
	button = &sheet->row[row].button;
	index = row;
	x = 0;
	y = _gtk_sheet_row_top_ypixel(sheet, row) + CELL_SPACING;

        /* move button labels and attached widgets, see below */
        if (!cr && sheet->column_titles_visible)
            y -= sheet->column_title_area.height;

	width = sheet->row_title_area.width-1;
	height = sheet->row[row].height;
	area = ON_ROW_TITLES_AREA;
    }

    allocation.x = x;
    allocation.y = y;
    allocation.width = width;
    allocation.height = height;

    cairo_t *my_cr = cr;

    if (!my_cr)
    {
        // when not in expose - create my own cr and destroy at end
        my_cr = gdk_cairo_create(window);
    }

    //gdk_cairo_rectangle(my_cr, &allocation);
    //gdk_cairo_set_source_rgba(my_cr, &debug_color);
    //cairo_fill(my_cr);

    gtk_style_context_save(sheet_context);

    GtkTextDirection sheet_dir = gtk_widget_get_direction(
	GTK_WIDGET(sheet));

    GtkStateFlags dir_flags = (
	(sheet_dir == GTK_TEXT_DIR_LTR) ? GTK_STATE_FLAG_DIR_LTR :
	(sheet_dir == GTK_TEXT_DIR_RTL) ? GTK_STATE_FLAG_DIR_RTL :
	0);

    GtkStateFlags button_state = (button->state | dir_flags);

    gtk_style_context_set_state(sheet_context, button_state);

    gtk_style_context_add_class(
        sheet_context, GTK_STYLE_CLASS_BUTTON);
    gtk_style_context_add_class(
        sheet_context, GTK_STYLE_CLASS_LEFT);

    gtk_style_context_get(sheet_context, GTK_STATE_FLAG_NORMAL,
        GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    //g_debug_style_context_list_classes("_gtk_sheet_draw_button", sheet_context);

#if GTK_SHEET_DEBUG_DRAW_BUTTON > 0
    g_debug(
        "%s(%d): gtk_render_background: x %d y %d w %d h %d st %d", 
        __FUNCTION__, __LINE__, 
	x, y, width, height,
        button_state);
#endif

    gtk_render_background (sheet_context, my_cr,
	(double) x, (double) y, (double) width, (double) height);

    gtk_render_frame(sheet_context, my_cr,
	(double) x, (double) y, (double) width, (double) height);
 
    if (button->label_visible)
    {
	PangoLayout *layout = NULL;
	gint real_x = x, real_y;

	cairo_save(my_cr);  // button clip

	cairo_rectangle(my_cr, 
	    (double) allocation.x,
	    (double) allocation.y,
	    (double) allocation.width,
	    (double) allocation.height);
	cairo_clip(my_cr);

	GtkBorder button_border;
	gtk_style_context_get_border (
            sheet_context, button_state, &button_border);
	y += button_border.top + button_border.bottom;

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
        guint text_width = extent.width;

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

	gtk_render_layout(sheet_context,
	    my_cr,
	    (gdouble) real_x, 
	    (gdouble) real_y,
	    layout);
	g_object_unref(G_OBJECT(layout));

	cairo_restore(my_cr);  // button clip
    }

    if (!cr) 
    {
        _gtk_sheet_invalidate_region(sheet, x, y, width, height);
        cairo_destroy(my_cr);  /* FIXME*/
    }

    pango_font_description_free(font_desc);

    gtk_style_context_restore(sheet_context);

    gtk_sheet_draw_tooltip_marker(sheet, area, row, col);

    if ((child = button->child) && (child->widget))
    {
	GtkRequisition child_min_size, child_nat_size;

	child->x = allocation.x;
	child->y = allocation.y;

        if (row == -1)  /* column title button */
        {
            /* move attached widgets only, see above */
            if (cr && sheet->row_titles_visible)
                child->x -= sheet->row_title_area.width;
        }
        else if (col == -1)  /* row title button */
        {
            /* move attached widgets only, see above */
            if (cr && sheet->column_titles_visible)
                child->y -= sheet->column_title_area.height;
        }

	gtk_widget_get_preferred_size (
            child->widget, &child_min_size, &child_nat_size);

	child->x += (width - child_min_size.width) / 2;
	child->y += (height - child_min_size.height) / 2;

	allocation.x = child->x;
	allocation.y = child->y;
	allocation.width = child_min_size.width;
	allocation.height = child_min_size.height;

	//gtk_widget_set_state(child->widget, button->state);
	gtk_widget_set_state_flags (child->widget, button_state, FALSE);

	if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
	{
            if (gtk_widget_get_visible(child->widget)
                && !gtk_widget_get_mapped(child->widget))
            {
                gtk_widget_map(child->widget);
            }

            /* set position and size of attached child widget */
            gtk_widget_size_allocate(child->widget, &allocation);

            /* the following statement causes recursive draw
               when button has a visible widget attached
            gtk_widget_queue_draw(child->widget); 
            */ 
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
	    g_signal_emit_by_name(G_OBJECT(va), "value_changed");
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
	    g_signal_emit_by_name(G_OBJECT(ha), "value_changed");
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
    g_return_if_fail(adjustment != NULL);
    g_return_if_fail(data != NULL);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 1
    GtkSheet *sheet = GTK_SHEET(data);

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
    g_return_if_fail(adjustment != NULL);
    g_return_if_fail(data != NULL);

#if GTK_SHEET_DEBUG_ADJUSTMENT > 1
    GtkSheet *sheet = GTK_SHEET(data);

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

    for (row = 0; row < sheet->maxrow; row++)  /* all but the last row */
    {
	if (GTK_SHEET_ROW_IS_VISIBLE(ROWPTR(sheet, row)))
	    y += sheet->row[row].height;
	if (y > gtk_adjustment_get_value(adjustment))
	    break;
    }
    if (0 <= row && row <= sheet->maxrow)
	y -= sheet->row[row].height;
    new_row = row;

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

    if (gtk_widget_get_realized(sheet->sheet_entry)
        && sheet->state == GTK_SHEET_NORMAL
        && sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0
        && !gtk_sheet_cell_isvisible(
            sheet, sheet->active_cell.row, sheet->active_cell.col))
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

    _gtk_sheet_position_children(sheet);

    _global_sheet_button_size_allocate(sheet);
    _gtk_sheet_row_buttons_size_allocate(sheet);

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

    for (col = 0; col < sheet->maxcol; col++)  /* all but the last column */
    {
	if (GTK_SHEET_COLUMN_IS_VISIBLE(COLPTR(sheet, col))) x += COLPTR(sheet, col)->width;
	if (x > adjustment->value) break;
    }
    if (0 <= col && col <= sheet->maxcol) x -= COLPTR(sheet, col)->width;
    new_col = col;

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

    if (gtk_widget_get_realized(sheet->sheet_entry)
        && sheet->state == GTK_SHEET_NORMAL
        && sheet->active_cell.row >= 0 && sheet->active_cell.col >= 0
        && !gtk_sheet_cell_isvisible(
            sheet, sheet->active_cell.row, sheet->active_cell.col))
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

    _gtk_sheet_position_children(sheet);

    _global_sheet_button_size_allocate(sheet);
    _gtk_sheet_column_buttons_size_allocate(sheet);

    _gtk_sheet_range_draw(sheet, NULL, TRUE);
}


/* COLUMN RESIZING */
static void
draw_xor_vline(GtkSheet *sheet, gboolean draw)
{
    g_return_if_fail(sheet != NULL);

    if (draw)
    {
        cairo_t *xor_cr = gdk_cairo_create(sheet->sheet_window); /* FIXME, to be removed */
	_cairo_save_and_set_sel_color(sheet, xor_cr);

	// xor
	cairo_move_to(xor_cr, sheet->x_drag, sheet->column_title_area.height);
	cairo_line_to(xor_cr, sheet->x_drag, sheet->sheet_window_height + 1);
	cairo_stroke(xor_cr);

	cairo_restore(xor_cr);  // _cairo_save_and_set_sel_color
	cairo_destroy(xor_cr);  /* FIXME - to be removed */
    }
    else  // erase
    {
	// use swin_cr to blit sheet->bsurf into sheet_window
	cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);
	cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);

	cairo_move_to(swin_cr, sheet->x_drag, sheet->column_title_area.height);
	cairo_line_to(swin_cr, sheet->x_drag, sheet->sheet_window_height + 1);
	cairo_stroke(swin_cr);

	cairo_destroy(swin_cr);
    }
}

/* ROW RESIZING */
static void
draw_xor_hline(GtkSheet *sheet, gboolean draw)
{
    g_return_if_fail(sheet != NULL);

    if (draw)
    {
        cairo_t *xor_cr = gdk_cairo_create(sheet->sheet_window); /* FIXME, to be removed */
	_cairo_save_and_set_sel_color(sheet, xor_cr);

	// xor
	cairo_move_to(xor_cr, sheet->row_title_area.width, sheet->y_drag);
	cairo_line_to(xor_cr,  sheet->sheet_window_width + 1, sheet->y_drag);
	cairo_stroke(xor_cr);

	cairo_restore(xor_cr);  // _cairo_save_and_set_sel_color
	cairo_destroy(xor_cr);   /* FIXME - to be removed */
    }
    else  // erase
    {
	// use swin_cr to blit sheet->bsurf into sheet_window
	cairo_t *swin_cr = gdk_cairo_create(sheet->sheet_window);
	cairo_set_source_surface(swin_cr, sheet->bsurf, 0, 0);

	cairo_move_to(swin_cr, sheet->row_title_area.width, sheet->y_drag);
	cairo_line_to(swin_cr,  sheet->sheet_window_width + 1, sheet->y_drag);
	cairo_stroke(swin_cr);

	cairo_destroy(swin_cr);
    }
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

    _gtk_sheet_row_buttons_size_allocate(sheet);

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

    if (gtk_widget_get_realized(GTK_WIDGET(sheet))
        && !GTK_SHEET_IS_FROZEN(sheet))
    {
	_gtk_sheet_row_buttons_size_allocate(sheet);
	_gtk_sheet_scrollbar_adjust(sheet);
	_gtk_sheet_entry_size_allocate(sheet);
	_gtk_sheet_range_draw(sheet, NULL, TRUE);
    }

    g_signal_emit(G_OBJECT(sheet), sheet_signals[NEW_ROW_HEIGHT], 0, row, height);
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
#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (1) %p %s - row %d nrows %d  mxr %d mxc %d mxar %d mxac %d data[0] %p",
	__FUNCTION__, __LINE__,
	sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	row, nrows,
	sheet->maxrow, sheet->maxcol,
	sheet->maxallocrow, sheet->maxalloccol,
	sheet->data ? sheet->data[0] : NULL
	);
#endif

    GList *children;
    GtkSheetChild *child;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    nrows = MIN(nrows, sheet->maxrow - row + 1);

    gint act_row = sheet->active_cell.row;

    _gtk_sheet_hide_active_cell(sheet);
    gtk_sheet_real_unselect_range(sheet, NULL);

    DeleteRow(sheet, row, nrows);

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (2) %p %s - row %d nrows %d mxr %d mxc %d mxar %d mxac %d data[0] %p",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	row, nrows,
	sheet->maxrow, sheet->maxcol,
	sheet->maxallocrow, sheet->maxalloccol,
	sheet->data ? sheet->data[0] : NULL
	);
#endif

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

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet))) return;

    /* deactivate active cell properly - added  25.06.18/fp */
    _gtk_sheet_hide_active_cell(sheet);

    SET_ACTIVE_CELL(-1, -1);

/* if(sheet->state == GTK_SHEET_ROW_SELECTED)
*/

    act_row = MIN(act_row, sheet->maxrow);
    act_row = MAX(act_row, 0);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, FALSE, TRUE);

    gtk_sheet_activate_cell(
        sheet, sheet->active_cell.row, sheet->active_cell.col);
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

    gint act_col = sheet->active_cell.col;

    /* deactivate active cell properly - added  25.06.18/fp */
    _gtk_sheet_hide_active_cell(sheet);

    SET_ACTIVE_CELL(-1, -1);

/* if(sheet->state == GTK_SHEET_COLUMN_SELECTED)
*/

    act_col = MIN(act_col, sheet->maxcol);
    act_col = MAX(act_col, 0);

    _gtk_sheet_scrollbar_adjust(sheet);
    _gtk_sheet_redraw_internal(sheet, TRUE, FALSE);

    gtk_sheet_activate_cell(
        sheet, sheet->active_cell.row, sheet->active_cell.col);
}

/**
 * gtk_sheet_range_set_background:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @color: a #GdkRGBA.
 *
 * Set background color of the given range. 
 * 
 * Deprecated:4.1.3: Use #gtk_sheet_range_set_css_class()
 * instead
 */
void
gtk_sheet_range_set_background(GtkSheet *sheet,
    const GtkSheetRange *urange,
    const GdkRGBA *color)
{
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

#if GTK_SHEET_DEBUG_COLORS > 0
    g_debug("%s(%d): %s row %d-%d col %d-%d)",
        __FUNCTION__, __LINE__,
	gdk_rgba_to_string(color), range.row0, range.rowi, range.col0, range.coli);
#endif

    for (row = range.row0; row <= range.rowi; row++)
    {
        for (col = range.col0; col <= range.coli; col++)
        {
            GtkSheetCellAttr attributes;
            gtk_sheet_get_attributes(sheet, row, col, &attributes);
        
            if (color != NULL)
            {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
                g_warning("%s(%d): deprecated_bg_color_used %s %s",
                    __FUNCTION__, __LINE__,
                    G_OBJECT_TYPE_NAME(sheet),
                    gtk_widget_get_name(GTK_WIDGET(sheet)));
#endif
                attributes.background = *color;
                attributes.deprecated_bg_color_used = TRUE;
            }
            else
            {
                attributes.background = sheet->bg_color;
                attributes.deprecated_bg_color_used = FALSE;
            }
        
            gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
        }
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
}

/**
 * gtk_sheet_range_set_foreground:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @color: a #GdkRGBA.
 *
 * Set foreground color of the given range.
 * 
 * Deprecated:4.1.3: Use #gtk_sheet_range_set_css_class()
 * instead
 */
void
gtk_sheet_range_set_foreground(GtkSheet *sheet,
    const GtkSheetRange *urange,
    const GdkRGBA *color)
{
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

#if GTK_SHEET_DEBUG_COLORS > 0
    g_debug("gtk_sheet_range_set_foreground: %s row %d-%d col %d-%d)",
	gdk_rgba_to_string(color), range.row0, range.rowi, range.col0, range.coli);
#endif

    for (row = range.row0; row <= range.rowi; row++) 
        for (col = range.col0; col <= range.coli; col++)
    {
	GtkSheetCellAttr attributes;
	gtk_sheet_get_attributes(sheet, row, col, &attributes);

	if (color != NULL)
        {
#if GTK_SHEET_DEBUG_ENABLE_DEPRECATION_WARNINGS>0
            g_warning("%s(%d): deprecated_fg_color_used %s %s",
                __FUNCTION__, __LINE__,
                G_OBJECT_TYPE_NAME(sheet),
                gtk_widget_get_name(GTK_WIDGET(sheet)));
#endif
            attributes.foreground = *color;
            attributes.deprecated_fg_color_used = TRUE;
        }
	else
        {
            attributes.foreground = color_black;
            attributes.deprecated_fg_color_used = FALSE;
        }

	gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
}

/* Note: 15.09.19/fp
   callgrind shows that pango_context_get_metrics()
   consumes 95% of the time spent in this function.
 
   Looking at testgtksheet Folder 1,
   row height fails to adjust when turned off.
   */
#define GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE 2

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE==2
static gboolean _font_changed(
    GtkSheet *sheet, gchar *old_css_class, gchar *new_css_class)
{
    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
	GTK_WIDGET(sheet));

    /* get old_font_desc */
    gtk_style_context_save(sheet_context);

    gtk_style_context_add_class(
        sheet_context, GTK_STYLE_CLASS_CELL);

    if (old_css_class)
	gtk_style_context_add_class(sheet_context, old_css_class);

    PangoFontDescription *old_font_desc = NULL;
    gtk_style_context_get(sheet_context, GTK_STATE_FLAG_NORMAL,
	GTK_STYLE_PROPERTY_FONT, &old_font_desc, 
	NULL);

    gtk_style_context_restore(sheet_context);

    /* get new_font_desc */
    gtk_style_context_save(sheet_context);

    gtk_style_context_add_class(
        sheet_context, GTK_STYLE_CLASS_CELL);

    if (new_css_class)
	gtk_style_context_add_class(sheet_context, new_css_class);

    PangoFontDescription *new_font_desc = NULL;
    gtk_style_context_get(sheet_context, GTK_STATE_FLAG_NORMAL,
	GTK_STYLE_PROPERTY_FONT, &new_font_desc, 
	NULL);

    gtk_style_context_restore(sheet_context);

    /* compare */
    gboolean is_equal = pango_font_description_equal(
        old_font_desc, new_font_desc);

    /* free */
    pango_font_description_free(old_font_desc);
    pango_font_description_free(new_font_desc);

    return(!is_equal);
}
#endif

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE>0
static gint _get_font_height(GtkSheet *sheet, gchar *css_class)
{
    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
	GTK_WIDGET(sheet));

    gtk_style_context_save(sheet_context);

    gtk_style_context_add_class(
	sheet_context, GTK_STYLE_CLASS_CELL);

    if (css_class)
	gtk_style_context_add_class(sheet_context, css_class);

    PangoFontDescription *font_desc = NULL;
    gtk_style_context_get(sheet_context, GTK_STATE_FLAG_NORMAL,
	GTK_STYLE_PROPERTY_FONT, &font_desc, 
	NULL);
    
    PangoContext *pango_context = gtk_widget_get_pango_context(
	GTK_WIDGET(sheet));

    PangoFontMetrics *metrics = pango_context_get_metrics(
	pango_context,
	font_desc,
	pango_context_get_language(pango_context));

    gint font_height = pango_font_metrics_get_descent(metrics) +
	pango_font_metrics_get_ascent(metrics);

    font_height = PANGO_PIXELS(font_height) + 2 * CELLOFFSET;

    pango_font_metrics_unref(metrics);
    pango_font_description_free(font_desc);

    gtk_style_context_restore(sheet_context);

    return(font_height);
}
#endif

/**
 * gtk_sheet_range_set_css_class:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange.
 * @css_class: (nullable): a CSS class name.
 *
 * Set CSS class of the given range.
 */
void
gtk_sheet_range_set_css_class(GtkSheet *sheet,
    const GtkSheetRange *urange,
    gchar *css_class)
{
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

#if GTK_SHEET_DEBUG_COLORS > 0
    g_debug("%s(%d): %s row %d-%d col %d-%d)",
        __FUNCTION__, __LINE__,
	css_class, 
        range.row0, range.rowi, range.col0, range.coli);
#endif

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE==1
    gint font_height = _get_font_height(sheet, css_class);
#endif

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE==2
    gboolean font_changed = FALSE;
    gint font_height = 0;
#endif

    for (row = range.row0; row <= range.rowi; row++) 
    {
        for (col = range.col0; col <= range.coli; col++)
        {
            GtkSheetCellAttr attributes;
            gtk_sheet_get_attributes(sheet, row, col, &attributes);

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE==2
            if (!font_changed
		&& css_class && attributes.css_class
		&& strcmp(css_class, attributes.css_class) != 0
                && _font_changed(
                    sheet, attributes.css_class, css_class))
            {
                font_changed = TRUE;
                font_height = _get_font_height(sheet, css_class);
            }
#endif
        
            if (attributes.css_class)
            {
                g_free(attributes.css_class);
                attributes.css_class = NULL;
            }
            
            attributes.css_class = g_strdup(css_class);
            
            gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
        }

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE==1
	if (font_height > sheet->row[row].height)
	{
	    sheet->row[row].height = font_height;
	}
#endif

#if GTK_SHEET_RECALC_ROW_HEIGHT_ON_CSS_CHANGE==2
        if (font_changed)
        {

            if (font_height > sheet->row[row].height)
            {
                sheet->row[row].height = font_height;
            }
        }
#endif
    }

    if (font_changed)
	_gtk_sheet_recalc_top_ypixels(sheet);

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
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (row = range.row0; row <= range.rowi; row++)
    {
	for (col = range.col0; col <= range.coli; col++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, row, col, &attributes);
	    attributes.justification = just;
	    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
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
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (row = range.row0; row <= range.rowi; row++)
    {
	for (col = range.col0; col <= range.coli; col++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, row, col, &attributes);
	    attributes.is_editable = editable;
	    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
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
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (row = range.row0; row <= range.rowi; row++)
    {
	for (col = range.col0; col <= range.coli; col++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, row, col, &attributes);
	    attributes.is_visible = visible;
	    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
	}
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
}

/**
 * gtk_sheet_range_set_border:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange where we set border style.
 * @mask: CELL_LEFT_BORDER, CELL_RIGHT_BORDER, 
 *      CELL_TOP_BORDER,CELL_BOTTOM_BORDER
 * @width: width of the border line in pixels
 * @cap_style: see cairo_set_line_cap() 
 * @join_style: see cairo_set_line_join()
 *
 * Set cell border style in the given range.
 */
void
gtk_sheet_range_set_border(GtkSheet *sheet, 
    const GtkSheetRange *urange,  gint mask, guint width,
    cairo_line_cap_t cap_style,
    cairo_line_join_t join_style)
{
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (row = range.row0; row <= range.rowi; row++)
    {
	for (col = range.col0; col <= range.coli; col++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, row, col, &attributes);
	    attributes.border.mask = mask;
	    attributes.border.width = width;
	    attributes.border.cap_style = cap_style;
	    attributes.border.join_style = join_style;
	    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
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
 * @color: a #GdkRGBA.
 *
 * Set border color for the given range.
 */
void
gtk_sheet_range_set_border_color(GtkSheet *sheet,
    const GtkSheetRange *urange,
    const GdkRGBA *color)
{
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    for (row = range.row0; row <= range.rowi; row++)
    {
	for (col = range.col0; col <= range.coli; col++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, row, col, &attributes);
	    attributes.border.color = *color;
	    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
	}
    }

    if (!GTK_SHEET_IS_FROZEN(sheet))
	_gtk_sheet_range_draw(sheet, &range, TRUE);
}

/**
 * gtk_sheet_range_set_font:
 * @sheet: a #GtkSheet.
 * @urange: a #GtkSheetRange where we set font_desc.
 * @font_desc: (transfer none): a #PangoFontDescription.
 *
 * Set font_desc for the given range. 
 * 
 * Deprecated:4.1.3: Use #gtk_sheet_range_set_css_class()
 * instead
 */
void
gtk_sheet_range_set_font(GtkSheet *sheet,
    const GtkSheetRange *urange,
    PangoFontDescription *font_desc)
{
    gint row, col;
    GtkSheetRange range;

    g_return_if_fail(sheet != NULL);
    g_return_if_fail(GTK_IS_SHEET(sheet));

    if (!urange)
	range = sheet->range;
    else
	range = *urange;

    gtk_sheet_freeze(sheet);

    PangoContext *pango_context = gtk_widget_get_pango_context(
	GTK_WIDGET(sheet));

    PangoFontMetrics *metrics = pango_context_get_metrics(
	pango_context,
	font_desc,
	pango_context_get_language(pango_context));

    gint font_height = pango_font_metrics_get_descent(metrics) +
	pango_font_metrics_get_ascent(metrics);

    font_height = PANGO_PIXELS(font_height) + 2 * CELLOFFSET;

    for (row = range.row0; row <= range.rowi; row++)
    {
	for (col = range.col0; col <= range.coli; col++)
	{
	    GtkSheetCellAttr attributes;
	    gtk_sheet_get_attributes(sheet, row, col, &attributes);
	    attributes.font_desc = pango_font_description_copy(
		font_desc);
	    attributes.do_font_desc_free = TRUE;
	    attributes.deprecated_font_desc_used = TRUE;

	    gtk_sheet_set_cell_attributes(sheet, row, col, attributes);
	}

	if (font_height > sheet->row[row].height)
	{
	    sheet->row[row].height = font_height;
	    _gtk_sheet_recalc_top_ypixels(sheet);
	}
    }

    gtk_sheet_thaw(sheet);
    pango_font_metrics_unref(metrics);
}

/**
 * assign cell attributes
 * 
 * cell->attributes will be allocated when necessary.
 * 
 * @param sheet      the sheet
 * @param row        the row
 * @param col        the column
 * @param attributes attributes to assign
 */
static void
gtk_sheet_set_cell_attributes(GtkSheet *sheet,
    gint row, gint col,
    GtkSheetCellAttr attributes)
{
#if 0
    g_debug("%s(%d): row %d col %d mxr %d mxc %d)",
        __FUNCTION__, __LINE__,
	row, col, sheet->maxrow, sheet->maxcol);
#endif

    if (row < 0 || row > sheet->maxrow)
	return;
    if (col < 0 || col > sheet->maxcol)
	return;

    CheckCellData(sheet, row, col);

    GtkSheetCell *cell = sheet->data[row][col];

    if (!cell->attributes)
	cell->attributes = g_new(GtkSheetCellAttr, 1);

    *(cell->attributes) = attributes; /* struct assignment */
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
#if 0
    g_debug("%s(%d): row %d col %d)",
        __FUNCTION__, __LINE__,
	row, col);
#endif

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

    GtkSheetCell *cell = sheet->data[row][col];

    if (!cell->attributes)
    {
	init_attributes(sheet, col, attributes);
	return (FALSE);
    }

    *attributes = *(cell->attributes); /* struct assignment */

    if (COLPTR(sheet, col)->justification
	!= GTK_SHEET_COLUMN_DEFAULT_JUSTIFICATION)
    {
	attributes->justification = COLPTR(sheet, col)->justification;
    }

    return (TRUE);
}

static void
init_attributes(GtkSheet *sheet, gint col, GtkSheetCellAttr *attributes)
{
#if 0
    g_debug("%s(%d): col %d attr %p)",
        __FUNCTION__, __LINE__,
	col, attributes);
#endif

    /* DEFAULT VALUES */
    attributes->foreground = color_black;
    attributes->deprecated_fg_color_used = FALSE;
    attributes->background = sheet->bg_color;
    attributes->deprecated_bg_color_used = FALSE;

    GtkStyleContext *sheet_context = gtk_widget_get_style_context(
        GTK_WIDGET(sheet));

    if (col < 0 || col > sheet->maxcol)
	attributes->justification = GTK_SHEET_COLUMN_DEFAULT_JUSTIFICATION;
    else
        attributes->justification = COLPTR(sheet, col)->justification;

    attributes->border.width = 0;
    attributes->border.cap_style = CAIRO_LINE_CAP_BUTT;
    attributes->border.join_style = CAIRO_LINE_JOIN_MITER;
    attributes->border.mask = 0;
    attributes->border.color = color_black;
    attributes->is_editable = TRUE;
    attributes->is_visible = TRUE;

    GtkStateFlags flags = gtk_widget_get_state_flags(
        GTK_WIDGET(sheet));

    gtk_style_context_get(sheet_context, 
        flags, GTK_STYLE_PROPERTY_FONT, &attributes->font_desc, NULL);
    attributes->do_font_desc_free = TRUE;

    attributes->deprecated_font_desc_used = FALSE;
    attributes->css_class = NULL;
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

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (1) %p %s pos %d ncols %d mxc %d mxac %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	position, ncols,
	sheet->maxcol, sheet->maxalloccol);
#endif

    if (ncols > 0)
    {
	/* realloc produces new uninitialized pointers at end */
	sheet->column = (GtkSheetColumn **)g_realloc(
	    sheet->column,
	    (sheet->maxcol + 1 + ncols)* sizeof(GtkSheetColumn *));

	/* moveright, shift up column pointers */
	for (c = sheet->maxcol; c >= position; c--)
	{
	    sheet->column[c + ncols] = sheet->column[c]; /* copy pointer */
	    sheet->column[c] = NULL; /* clear source pointer */
	}

	for (c = 0; c < ncols; c++) /* allocate and init new columns */
	{
	    gint newidx = position + c;

	    newobj = g_object_new(G_TYPE_SHEET_COLUMN, NULL);

#if GTK_SHEET_DEBUG_FINALIZE > 0
	    g_object_weak_ref(G_OBJECT(newobj), weak_notify, "Sheet-Column");
#endif

	    newobj->sheet = sheet;
	    sheet->column[newidx] = newobj;

            if (gtk_widget_get_realized(GTK_WIDGET(sheet)))
            {
                _gtk_sheet_column_realize(newobj, sheet);

                gtk_widget_set_window(
                    GTK_WIDGET(newobj), sheet->column_title_window);
            }

            DEBUG_WIDGET_SET_PARENT(newobj, sheet);
            gtk_widget_set_parent(
                GTK_WIDGET(newobj), GTK_WIDGET(sheet));

            g_object_ref_sink(newobj);
        }

        gint old_maxcol = sheet->maxcol;

	sheet->maxcol += ncols;

        /* add default titles
           this only works for an empty sheet, otherwise we would
           have to renumber default titles when inserting further
           columns
           */
        if (old_maxcol < 0)
        {
            for (c = 0; c <= sheet->maxcol; c++)
            {
                gint newidx = position + c;

                gchar title[24];
                g_sprintf(title, "%d", newidx);
                gtk_sheet_column_button_add_label(sheet, newidx, title);
            }
        }

	_gtk_sheet_reset_text_column(sheet, sheet->maxcol - ncols);
	_gtk_sheet_recalc_left_xpixels(sheet);
    }

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (2) %p %s pos %d ncols %d mxc %d mxac %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	sheet->maxcol, sheet->maxalloccol);
#endif
}

static void
InsertColumn(GtkSheet *sheet, gint position, gint ncols)
{
    gint r, c;

    g_assert(ncols >= 0);
    g_assert(position >= 0);

#if GTK_SHEET_DEBUG_ALLOCATION > 1
    g_debug("%s(%d): %p %s calling AddColumns",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet))
	);
#endif

    AddColumns(sheet, position, ncols);

    _gtk_sheet_reset_text_column(sheet, sheet->maxcol - ncols);
    _gtk_sheet_recalc_left_xpixels(sheet);

    if (position <= sheet->maxalloccol)  /* adjust allocated data cells */
    {
	GrowSheet(sheet, 0, ncols);  /* append columns at end */

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
    g_debug("%s(%d): (1) %p %s pos %d ncols %d mxc %d mxar %d mxac %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	position, ncols,
	sheet->maxcol, sheet->maxallocrow, sheet->maxalloccol);
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
	/* moveleft, shift down column data  */
	for (c = position; c <= sheet->maxcol - ncols; c++)
	{
	    for (r = 0; r <= sheet->maxallocrow; r++)
	    {
		gtk_sheet_real_cell_clear(sheet, r, c, TRUE);

		if (c + ncols <= sheet->maxalloccol)
		{
		    sheet->data[r][c] = sheet->data[r][c + ncols];
		    sheet->data[r][c + ncols] = NULL;

		    if (sheet->data[r][c])
			sheet->data[r][c]->col = c; /* fix column number */
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
    }

    sheet->maxcol -= ncols;

#define ENABLE_SHEET_DATA_ARRAY_DEALLOCATION 1
    // experimental - deallocate sheet->data array

#if ENABLE_SHEET_DATA_ARRAY_DEALLOCATION > 0
    // experimental - deallocate sheet->data array

    if (sheet->maxcol == -1 && sheet->maxalloccol >= 0)
    {
#if GTK_SHEET_DEBUG_ALLOCATION > 0
	g_debug("%s(%d): %p %s deallocate sheet->data mxac %d",
	    __FUNCTION__, __LINE__,
	    sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	    sheet->maxalloccol);
#endif

	for (r = 0; r <= sheet->maxallocrow; r++)
	{
	    if (sheet->data[r])
	    {
		for (c = 0; c <= sheet->maxalloccol; c++)
		{
		    gtk_sheet_real_cell_clear(sheet, r, c, TRUE);
		}
		g_free(sheet->data[r]);
		sheet->data[r] = NULL;
	    }
	}
	g_free(sheet->data);
	sheet->data = NULL;
	sheet->maxallocrow = -1;
	sheet->maxalloccol = -1;
    }
#endif

    _gtk_sheet_range_fixup(sheet, &sheet->view);
    _gtk_sheet_range_fixup(sheet, &sheet->range);

    _gtk_sheet_reset_text_column(sheet, position);
    _gtk_sheet_recalc_left_xpixels(sheet);

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (2) %p %s mxc %d mxar %d mxac %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	sheet->maxcol, sheet->maxallocrow, sheet->maxalloccol);
#endif
}

static void
AddRows(GtkSheet *sheet, gint position, gint nrows)
{
    gint r;

    g_assert(nrows >= 0);
    g_assert(position >= 0 && position <= sheet->maxrow + 1);
    
#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (1) %p %s pos %d nrows %d mxr %d mxar %d mxara %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	position, nrows,
	sheet->maxrow, sheet->maxallocrow, sheet->maxalloc_row_array);
#endif

    if (nrows > 0)
    {
	if (sheet->maxrow + nrows + 1 > sheet->maxalloc_row_array)
	{
	    sheet->maxalloc_row_array = sheet->maxrow + nrows + 1;
	    // allocation growing policy can be added here
	    // sheet->maxalloc_row_array = sheet->maxrow + nrows + 1 + 1000;
	}

#if GTK_SHEET_DEBUG_ALLOCATION > 1
	g_debug("%s(%d): %p %s realloc %d",
	    __FUNCTION__, __LINE__,
	    sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	    (sheet->maxalloc_row_array) * sizeof(GtkSheetRow));
#endif
	    
	/* realloc produces new uninitialized structs at end */
	sheet->row = (GtkSheetRow *)g_realloc(sheet->row,
	    (sheet->maxalloc_row_array) * sizeof(GtkSheetRow));

	/* thinking aid
	   0123456789
	   rrrrr00000  position=3, nrows=2
	   rrrnnrr000
	*/
#if 0
	// timing 45.55s, 12.38s, 47.06s (with style)
	// timing  0.95s, 10.60s,  1.06s (witout style)

	memmove(
	    &sheet->row[position], 
	    &sheet->row[position + nrows], 
	    nrows * sizeof(GtkSheetRow));
#else
	// timing 44.71s, 12.49s, 47.73s (with style)
	// timing  0.97s, 10.77s,  1.10s (without style)

	for (r = sheet->maxrow; r >= position; r--)  /* moveright, shift up */
	{
	    sheet->row[r + nrows] = sheet->row[r];  /* copy struct */

	    // not needed, will be done in for-loop below
	    //_gtk_sheet_row_init(&sheet->row[r]);  /* clear source struct */
	}
#endif

	gint default_height = _gtk_sheet_row_default_height(GTK_WIDGET(sheet));

	for (r = 0; r < nrows; r++) /* init new rows */
	{
	    gint newidx = position + r;  /* [pos..pos+nrows-1] */

	    _gtk_sheet_row_init(&sheet->row[newidx]);

	    sheet->row[newidx].requisition = sheet->row[newidx].height =
		default_height;
	}

	sheet->maxrow += nrows;

	_gtk_sheet_recalc_top_ypixels(sheet);
    }

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (2) %p %s mxr %d mxar %d mxara %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	sheet->maxrow, sheet->maxallocrow, sheet->maxalloc_row_array);
#endif
}

static void
InsertRow(GtkSheet *sheet, gint position, gint nrows)
{
    gint r, c;

#if GTK_SHEET_DEBUG_ALLOCATION > 1
    g_debug("%s(%d): %p %s calling AddRows",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet))
	);
#endif

    AddRows(sheet, position, nrows);

    _gtk_sheet_recalc_top_ypixels(sheet);

    if (position <= sheet->maxallocrow)  /* adjust allocated cells */
    {
	GrowSheet(sheet, nrows, 0);  /* append rows at end */

	/* swap new rows into position */
	for (r = sheet->maxallocrow; r >= position + nrows; r--)
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

    /* ex1 - example 1
       ---------------
       01234567  row number
       rrrrr00   maxrow=4 maxallocrow=6 (0 means data pointer is NULL)
	 dddd    DeleteRow(position=2, nrows=4)
		 min(nrows, maxrow-position+1) -> nrows=3
       rr000000  result (leave row array intact)
       rr000     result (reallocated row array, not implemented)
       
       ex2 - example 2
       ---------------
       01234567  row number
       rrrrR00   maxrow=4 maxallocrow=6 (0 means data pointer is NULL)
	 dd      DeleteRow(position=2, nrows=2)
		 min(nrows, maxrow-position+1) -> nrows=2
       rrR00000  result (leave row array intact)
       rrR000    result (reallocated row array, not implemented)
    */

    nrows = MIN(nrows, sheet->maxrow - position + 1);

    if (nrows <= 0 || position > sheet->maxrow)
	return;

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (1) %p %s pos %d nrows %d mxr %d mxar %d mxac %d",
	__FUNCTION__, __LINE__,
	sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	position, nrows,
	sheet->maxrow, sheet->maxallocrow, sheet->maxalloccol);
#endif

    for (r = position; r < position + nrows; r++)  /* dispose row data */
    {
	/* ex1: [2..4] ex2: [2..3] - correct */
	gtk_sheet_row_finalize(&sheet->row[r]);
    }

    for (r = position; r <= sheet->maxrow - nrows; r++)  /* shift rows into position*/
    {
	/* ex1: [2..1] ex2: [2..2] - correct */
	sheet->row[r] = sheet->row[r + nrows]; /* copy struct */
	//sheet->row[r + nrows] = NULL; 
	_gtk_sheet_row_init(&sheet->row[r]);  /* clear source struct */
    }

    /* to be done: shrink data pointer pool via realloc */

    if (position <= sheet->maxallocrow)
    {
	for (r = position; r <= sheet->maxrow - nrows; r++)  /* shift row data */
	{
	    /* ex1: [2..1] ex2: [2..2] */
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
		sheet->data[r] = sheet->data[r + nrows]; /* copy pointer */
		sheet->data[r + nrows] = NULL;  /* clear source pointer */

		GtkSheetCell **pp = sheet->data[r];  /* update row in existing cells */
		for (c = 0; c <= sheet->maxalloccol; c++, pp++)
		{
		    if (*pp)
			(*pp)->row = r;
		}
	    }
	}

	/* 308726 15.01.22/fp
	   for performance reasons, we do not actually realloc the
	   data array, but must tell the sheet that we have disposed
	   the cell data
	   */
	if (position + nrows <= sheet->maxallocrow)
	    sheet->maxallocrow -= nrows;
	else
	    sheet->maxallocrow = position;
    }

    sheet->maxrow -= nrows;

#if ENABLE_SHEET_DATA_ARRAY_DEALLOCATION > 0
    // experimental - deallocate sheet->data array

    if (sheet->maxrow == -1 && sheet->maxallocrow >= 0)
    {
#if GTK_SHEET_DEBUG_ALLOCATION > 0
	g_debug("%s(%d): %p %s deallocate sheet->data mxar %d",
	    __FUNCTION__, __LINE__,
	    sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	    sheet->maxallocrow);
#endif

	for (r = 0; r <= sheet->maxallocrow; r++)
	{
	    if (sheet->data[r])
	    {
		for (c = 0; c <= sheet->maxalloccol; c++)
		{
		    gtk_sheet_real_cell_clear(sheet, r, c, TRUE);
		}
		g_free(sheet->data[r]);
		sheet->data[r] = NULL;
	    }
	}
	g_free(sheet->data);
	sheet->data = NULL;
	sheet->maxallocrow = -1;
	sheet->maxalloccol = -1;
    }
#endif

    _gtk_sheet_range_fixup(sheet, &sheet->view);
    _gtk_sheet_range_fixup(sheet, &sheet->range);

    _gtk_sheet_recalc_top_ypixels(sheet);

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug("%s(%d): (2) %p %s mxr %d mxar %d mxac %d",
	__FUNCTION__, __LINE__,
	sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	sheet->maxrow, sheet->maxallocrow, sheet->maxalloccol);
#endif
}

static gint
GrowSheet(GtkSheet *sheet, gint newrows, gint newcols)
{
    // allocation growing policy can be added here
    //if (newrows == 1) newrows = 1000;

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug(
	"%s(%d): (1) %p %s newrows %d newcols %d mxr %d mxc %d mxar %d mxac %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	newrows, newcols,
	sheet->maxrow, sheet->maxcol,
	sheet->maxallocrow, sheet->maxalloccol);
#endif

    gint inirow = sheet->maxallocrow + 1;
    sheet->maxallocrow = sheet->maxallocrow + newrows;

    gint r, c;

    if (newrows > 0)
    {
#if 0
	g_debug("%s(%d): %p %s realloc %d",
	    __FUNCTION__, __LINE__,
	    sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	    (sheet->maxallocrow + 1) * sizeof(GtkSheetCell **) + sizeof(double));
#endif
	sheet->data = (GtkSheetCell ***) g_realloc(
	    sheet->data, 
	    (sheet->maxallocrow + 1) * sizeof(GtkSheetCell **) + sizeof(double));

	for (r = inirow; r <= sheet->maxallocrow; r++)
	{
	    sheet->data[r] = (GtkSheetCell **) g_malloc(
		(sheet->maxalloccol + 1) * sizeof(GtkSheetCell *) + sizeof(double));

	    for (c = 0; c <= sheet->maxalloccol; c++)
	    {
		sheet->data[r][c] = NULL;
	    }
	}
    }

    gint inicol = sheet->maxalloccol + 1;
    sheet->maxalloccol = sheet->maxalloccol + newcols;

    if (newcols > 0)
    {
	for (r = 0; r <= sheet->maxallocrow; r++)
	{
#if 0
	    g_debug("%s(%d): %p %s realloc %d",
		__FUNCTION__, __LINE__,
		sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
		(sheet->maxalloccol + 1) * sizeof(GtkSheetCell *) + sizeof(double));
#endif
	    sheet->data[r] = (GtkSheetCell **) g_realloc(
		sheet->data[r],
		(sheet->maxalloccol + 1) * sizeof(GtkSheetCell *) + sizeof(double));

	    for (c = inicol; c <= sheet->maxalloccol; c++)
	    {
		sheet->data[r][c] = NULL;
	    }
	}
    }

#if GTK_SHEET_DEBUG_ALLOCATION > 0
    g_debug(
	"%s(%d): (2) %p %s mxr %d mxc %d mxar %d mxac %d",
        __FUNCTION__, __LINE__,
        sheet, gtk_widget_get_name(GTK_WIDGET(sheet)),
	sheet->maxrow, sheet->maxcol,
	sheet->maxallocrow, sheet->maxalloccol);
#endif

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

#if 0
    g_debug("%s(%d): row %d col %d mxr %d mxc %d mxar %d mxac %d)",
        __FUNCTION__, __LINE__,
	row, col, 
	sheet->maxrow, sheet->maxcol,
	sheet->maxallocrow, sheet->maxalloccol
	);
#endif

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
 * Returns:  (transfer none): TRUE means that the cell is currently allocated.
 */
GtkSheetChild *
gtk_sheet_put(GtkSheet *sheet, GtkWidget *child, gint x, gint y)
{
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

    DEBUG_WIDGET_SET_PARENT(child, sheet);
    gtk_widget_set_parent(child, GTK_WIDGET(sheet));

    gtk_widget_get_preferred_size(child, NULL, NULL);

    if (gtk_widget_get_visible(GTK_WIDGET(sheet)))
    {
	if (gtk_widget_get_realized(GTK_WIDGET(sheet))
            && (!gtk_widget_get_realized(child)
                || gtk_widget_get_has_window(child)))
        {
            gtk_sheet_realize_child(sheet, child_info);
        }

	if (gtk_widget_get_mapped(GTK_WIDGET(sheet))
            && !gtk_widget_get_mapped(child))
        {
            gtk_widget_map(child);
        }
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

    gtk_sheet_attach(sheet, widget, 
        row, col, 
        GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
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
	if (gtk_widget_get_realized(GTK_WIDGET(sheet))
            && (!gtk_widget_get_realized(widget)
                || gtk_widget_get_has_window(widget)))
        {
            gtk_sheet_realize_child(sheet, child);
        }

	if (gtk_widget_get_mapped(GTK_WIDGET(sheet))
            && !gtk_widget_get_mapped(widget))
        {
            gtk_widget_map(widget);
        }
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

    if (row == -1)  /* attach to column title button */
    {
	g_assert(0 <= col && col <= sheet->maxcol);
	GtkSheetColumn *colobj = COLPTR(sheet, col);

	if (!colobj->col_button)
	{
	    gtk_sheet_column_button_add_label(sheet, col, "");
	}

	GtkWidget *child = gtk_bin_get_child(
            GTK_BIN(colobj->col_button));

        if (child)
	{
	    DEBUG_WIDGET_CONTAINER_REMOVE(
		colobj->col_button, child);
	    gtk_container_remove(
		GTK_CONTAINER(colobj->col_button), child);
	    //gtk_widget_destroy(child);
	}

	if (widget) {
	    DEBUG_WIDGET_CONTAINER_ADD(colobj->col_button, widget);
            gtk_container_add(
                GTK_CONTAINER(colobj->col_button), widget);
	}

	// is this needed?
	//if (gtk_widget_get_realized (GTK_WIDGET(sheet)))
	//  gtk_widget_queue_resize (GTK_WIDGET(sheet));

        return;    
    }
    else  /* attach to row title button */
    {
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
	if (gtk_widget_get_realized(GTK_WIDGET(sheet))
            && (!gtk_widget_get_realized(widget)
                || gtk_widget_get_has_window(widget)))
        {
            gtk_sheet_realize_child(sheet, child);
        }

	if (gtk_widget_get_mapped(GTK_WIDGET(sheet))
            && !gtk_widget_get_mapped(widget))
        {
            gtk_widget_map(widget);
        }
    }

    if (row == -1)
	_gtk_sheet_column_buttons_size_allocate(sheet);

    if (col == -1)
	_gtk_sheet_row_buttons_size_allocate(sheet);
}

static void
label_size_request(GtkSheet *sheet, gchar *label, GtkRequisition *req)
{
    if (!label)
    {
        req->height = req->width = 0;
        return;
    }

    GtkStyleContext *style_context = gtk_widget_get_style_context(
        GTK_WIDGET(sheet));

    GtkStateFlags flags = gtk_widget_get_state_flags(
        GTK_WIDGET(sheet));

    PangoFontDescription *font_desc = NULL;

    gtk_style_context_get(style_context, 
        flags, GTK_STYLE_PROPERTY_FONT, &font_desc, NULL);

    _get_string_extent(sheet, NULL, 
        font_desc, 
        label, FALSE,
        (guint *) &req->width, (guint *) &req->height);

    pango_font_description_free(font_desc);
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
    GtkRequisition minimum_size, natural_size;
    GtkRequisition label_requisition;

    if (gtk_sheet_autoresize(sheet) && button->label && button->label[0])
    {
	label_size_request(sheet, button->label, &label_requisition);
	label_requisition.width += 2 * CELLOFFSET + 2;
	label_requisition.height += 2 * CELLOFFSET;
    }
    else
    {
	label_requisition.height = _gtk_sheet_row_default_height(
            GTK_WIDGET(sheet));

	label_requisition.width = GTK_SHEET_COLUMN_MIN_WIDTH;
    }

    if (button->child)
    {
	GtkStyleContext *style_context = gtk_widget_get_style_context(
            GTK_WIDGET(sheet));

	gtk_widget_get_preferred_size(button->child->widget, 
	    &minimum_size, &natural_size);

	GtkBorder border;  // what border exactly?
	gtk_style_context_get_border(
            style_context, GTK_STATE_FLAG_NORMAL, &border);

	minimum_size.width += 2 * button->child->xpadding;
	minimum_size.height += 2 * button->child->ypadding;
	minimum_size.width += border.left + border.right;
	minimum_size.height += border.top + border.bottom;
    }
    else
    {
	minimum_size.height = _gtk_sheet_row_default_height(GTK_WIDGET(sheet));
	minimum_size.width = GTK_SHEET_COLUMN_MIN_WIDTH;
    }

    *button_requisition = minimum_size;
    button_requisition->width = MAX(minimum_size.width, label_requisition.width);
    button_requisition->height = MAX(minimum_size.height, label_requisition.height);

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

	if (child->attached_to_cell && child->row == row && child->col != -1 && !child->floating && !child->yshrink)
	{
	    GtkRequisition child_min_size, child_nat_size;
	    gtk_widget_get_preferred_size (child->widget, &child_min_size, &child_nat_size);

	    if (child_min_size.height + 2 * child->ypadding > *requisition)
		*requisition = child_min_size.height + 2 * child->ypadding;
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
    GtkAllocation child_allocation;
    gint xoffset = 0;
    gint yoffset = 0;
    GdkRectangle area;

    /* PR#99118 - we cannot reposition all children while scrolling because
       with many offside rows it will consume lots of CPU and incredibly
       slow down scrolling.
     
       If we do not reposition all childs, we have to hide them when going
       off screen, in order not to stray around. We're using unmap/map here
       in order to leave hide/show available for the application
       */

    if (child->attached_to_cell)
    {
	if ((child->row < MIN_VIEW_ROW(sheet)
             || child->row > MAX_VIEW_ROW(sheet))
            || (child->col < MIN_VIEW_COLUMN(sheet)
                || child->col > MAX_VIEW_COLUMN(sheet))
	    )
	{
            // FIXME - causes visible but not mapped warning
	    gtk_widget_hide(child->widget);
	    return;
	}

	if (gtk_widget_get_realized(child->widget) 
	    && !gtk_widget_get_mapped(child->widget))
	{
            // FIXME - causes visible but not mapped warning
	    gtk_widget_show(child->widget); 
	}
    }

    GtkRequisition child_min_size, child_nat_size;
    gtk_widget_get_preferred_size (
        child->widget, &child_min_size, &child_nat_size);

    if (sheet->column_titles_visible)
	yoffset = sheet->column_title_area.height;

    if (sheet->row_titles_visible)            
        xoffset = sheet->row_title_area.width;

    if (child->attached_to_cell)  /* within sheet, not in title areas */
    {
	gtk_sheet_get_cell_area(sheet, child->row, child->col, &area);

	child->x = area.x + child->xpadding;
	child->y = area.y + child->ypadding;

	if (!child->floating)
	{
	    if (child_min_size.width + 2 * child->xpadding <= COLPTR(sheet, child->col)->width)
	    {
		if (child->xfill)
		{
		    child_min_size.width = child_allocation.width = COLPTR(sheet, child->col)->width - 2 * child->xpadding;
		}
		else
		{
		    if (child->xexpand)
		    {
			child->x = area.x + COLPTR(sheet, child->col)->width / 2 -
			    child_min_size.width / 2;
		    }
		    child_allocation.width = child_min_size.width;
		}
	    }
	    else
	    {
		if (!child->xshrink)
		{
#if GTK_SHEET_DEBUG_SIZE > 0
		    g_debug("gtk_sheet_position_child[%d]: set width %d",
			child->col, child_min_size.width + 2 * child->xpadding);
#endif
		    gtk_sheet_set_column_width(sheet,
			child->col, child_min_size.width + 2 * child->xpadding);
		}
		child_allocation.width = COLPTR(sheet, child->col)->width - 2 * child->xpadding;
	    }

	    if (child_min_size.height + 2 * child->ypadding <= sheet->row[child->row].height)
	    {
		if (child->yfill)
		{
		    child_min_size.height = child_allocation.height = sheet->row[child->row].height - 2 * child->ypadding;
		}
		else
		{
		    if (child->yexpand)
		    {
			child->y = area.y + sheet->row[child->row].height / 2 -
			    child_min_size.height / 2;
		    }
		    child_allocation.height = child_min_size.height;
		}
	    }
	    else
	    {
		if (!child->yshrink)
		{
		    gtk_sheet_set_row_height(sheet, child->row, child_min_size.height + 2 * child->ypadding);
		}
		child_allocation.height = sheet->row[child->row].height - 2 * child->ypadding;
	    }
	}
	else
	{
	    child_allocation.width = child_min_size.width;
	    child_allocation.height = child_min_size.height;
	}

	child_allocation.x = child->x + xoffset;
	child_allocation.y = child->y + yoffset;
    }
    else  /* global sheet button */
    {
	//child_allocation.x = child->x + sheet->hoffset + xoffset;
	child_allocation.x = child->x + xoffset;

	//child_allocation.y = child->y + sheet->voffset + yoffset;
	child_allocation.y = child->y + yoffset;

	child_allocation.width = child_min_size.width;
	child_allocation.height = child_min_size.height;
    }

    gtk_widget_size_allocate(child->widget, &child_allocation);

    if (gtk_widget_get_mapped(child->widget))
    {
        GdkRectangle *rta = &sheet->row_title_area;

        /* clip on row_title_area */
        if (sheet->row_titles_visible
            && child_allocation.x < rta->width)
        {
            child_allocation.x = rta->width;
        }
        gtk_widget_set_clip(child->widget, &child_allocation);
    }

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

    if (include_internals)
    {
#if GTK_SHEET_DEBUG_CHILDREN > 1
        g_debug("gtk_sheet_forall_handler: C3 %p internals");
#endif
        gint col;
        for (col=0; col<=sheet->maxcol; col++)
        {
            GtkSheetColumn *colobj = sheet->column[col];
#if GTK_SHEET_DEBUG_CHILDREN > 1
            g_debug("%s(%d): C3 %s %p (%s) IsObject %d IsWidget %d Realized %d Visible %d Mapped %d Window %p Parent %p ParentWin %p", 
                    __FUNCTION__, __LINE__,
                    G_OBJECT_TYPE_NAME (colobj), colobj,
                    gtk_widget_get_name(colobj),
                    G_IS_OBJECT(colobj),
                    GTK_IS_WIDGET(colobj),
                    gtk_widget_get_realized(colobj),
                    gtk_widget_get_visible(colobj),
                    gtk_widget_get_mapped(colobj),
                    gtk_widget_get_window(colobj),
                    gtk_widget_get_parent(colobj),
                    gtk_widget_get_parent_window(colobj));
#endif
            if (!gtk_widget_get_window(GTK_WIDGET(colobj))
                && gtk_widget_get_parent_window(GTK_WIDGET(colobj)))
            {
                /* FIXME - widget window is missing here
                   causes Gdk Warning in gtk_cairo_should_draw_window
                   and later SEGV in draw
                   - when column is made visible
                   - or when column is made invisible
                     causing rightmost visible column getting mapped
                   thus we reset the window
                */
                gtk_widget_set_window(
                    GTK_WIDGET(colobj),
                    gtk_widget_get_parent_window(GTK_WIDGET(colobj)));
            }

            if (G_IS_OBJECT(colobj) && GTK_IS_WIDGET(colobj))
            {
                g_object_ref(colobj);
                (*callback)(GTK_WIDGET(colobj), callback_data);
                g_object_unref(colobj);
            }
        }
    }
}

/**
 * gtk_sheet_position_children:
 * 
 * reposition all sheet children
 * 
 * @param sheet  the #GtkSheet
 */
void
_gtk_sheet_position_children(GtkSheet *sheet)
{
    GList *children = sheet->children;
    GtkSheetChild *child;

#if 0
    g_debug("%s(%d): called %p", __FUNCTION__, __LINE__, sheet);
#endif

    if (!gtk_widget_get_realized(GTK_WIDGET(sheet)))
	return;

    while (children)
    {
	child = (GtkSheetChild *)children->data;

	if (child->col != -1 && child->row != -1)
        {
            gtk_sheet_position_child(sheet, child);

	    if (gtk_widget_get_visible(child->widget)
                && !gtk_widget_get_mapped(child->widget))
            {
                if (!gtk_widget_get_realized(child->widget)
                    || gtk_widget_get_has_window(child->widget))
                {
                    gtk_sheet_realize_child(sheet, child);
                }

                gtk_widget_map(child->widget);
            }
        }
        else if (child->col == -1 && child->row == -1) /* global sheet button */
        {
            if (sheet->column_titles_visible && sheet->row_titles_visible)
                _gtk_sheet_child_show(child);
            else
                _gtk_sheet_child_hide(child);
        }
        else if (child->row == -1)  /* column title button */
	{
	    if (sheet->column_titles_visible
                && MIN_VIEW_COLUMN(sheet) <= child->col
                && child->col <= MAX_VIEW_COLUMN(sheet))
                _gtk_sheet_child_show(child);
	    else
                _gtk_sheet_child_hide(child);
	}
        else if (child->col == -1)  /* row title button */
	{
	    if (sheet->row_titles_visible
                && MIN_VIEW_ROW(sheet) <= child->row
                && child->row <= MAX_VIEW_ROW(sheet))
		_gtk_sheet_child_show(child);
	    else
		_gtk_sheet_child_hide(child);
	}
        children = children->next;
    }

    {
	gint col;
	gint minviewcol = MIN_VIEW_COLUMN(sheet);
	gint maxviewcol = MAX_VIEW_COLUMN(sheet);

	for (col = 0; col <= sheet->maxcol; col++)
	{
	    g_assert(0 <= col && col <= sheet->maxcol);
	    GtkSheetColumn *colobj = COLPTR(sheet, col);

	    if (colobj->col_button)
	    {
                if (sheet->column_titles_visible
                    && colobj->is_visible
		    //&& !gtk_widget_get_visible(GTK_WIDGET(colobj))
		    && minviewcol <= col && col <= maxviewcol
                    && gtk_widget_get_mapped(GTK_WIDGET(sheet)))
		{
                    //gtk_widget_map(GTK_WIDGET(colobj));
                    gtk_widget_show(GTK_WIDGET(colobj->col_button));
                    gtk_widget_show(GTK_WIDGET(colobj));

                    if (!gtk_widget_get_mapped(colobj))
                    {
                        gtk_widget_map(GTK_WIDGET(colobj));
                    }
                }
		else
		{
                    gtk_widget_hide(GTK_WIDGET(colobj->col_button));
                    gtk_widget_hide(GTK_WIDGET(colobj));

                    if (gtk_widget_get_mapped(colobj))
                    {
                        gtk_widget_unmap(GTK_WIDGET(colobj));
                    }
		}
	    }
	}
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
gtk_sheet_remove_handler(
    GtkContainer *container, GtkWidget *widget)
{
    g_return_if_fail(container != NULL);
    g_return_if_fail(GTK_IS_SHEET(container));

    GtkSheet *sheet = GTK_SHEET(container);

    GList *children = sheet->children;

    GtkSheetChild *child = NULL;

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

#if GTK_SHEET_DEBUG_CHILDREN > 0
	g_debug("%s(%d): %p %s widget %s %p", 
            __FUNCTION__, __LINE__, 
	    sheet, gtk_widget_get_name(sheet), 
            G_OBJECT_TYPE_NAME(widget), widget);
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
        {
            DEBUG_WIDGET_SET_PARENT_WIN(
                child->widget, sheet->column_title_window);
            gtk_widget_set_parent_window(
                child->widget, sheet->column_title_window);
        }
	else if (child->col == -1)
        {
            DEBUG_WIDGET_SET_PARENT_WIN(
                child->widget, sheet->row_title_window);
            gtk_widget_set_parent_window(
                child->widget, sheet->row_title_window);
        }
	else
        {
            DEBUG_WIDGET_SET_PARENT_WIN(
                child->widget, sheet->sheet_window);
            gtk_widget_set_parent_window(
                child->widget, sheet->sheet_window);
        }
    }

    DEBUG_WIDGET_SET_PARENT(child->widget, widget);
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
    //g_warning("_gtk_sheet_child_hide %p", child);
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
    //g_warning("_gtk_sheet_child_show %p", child);
    gtk_widget_show(child->widget);
}
