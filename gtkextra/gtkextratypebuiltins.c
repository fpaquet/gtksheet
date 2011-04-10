


#define GTKEXTRA_ENABLE_BROKEN
#include "gtkextra.h"

/* enumerations from "gtkiconlist.h" */
GType
gtk_icon_list_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_ICON_LIST_ICON, "GTK_ICON_LIST_ICON", "icon" },
      { GTK_ICON_LIST_TEXT_RIGHT, "GTK_ICON_LIST_TEXT_RIGHT", "text-right" },
      { GTK_ICON_LIST_TEXT_BELOW, "GTK_ICON_LIST_TEXT_BELOW", "text-below" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkIconListMode", values);
  }
  return etype;
}

/* enumerations from "gtkplot.h" */
GType
gtk_plot_scale_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_SCALE_LINEAR, "GTK_PLOT_SCALE_LINEAR", "linear" },
      { GTK_PLOT_SCALE_LOG10, "GTK_PLOT_SCALE_LOG10", "log10" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotScale", values);
  }
  return etype;
}
GType
gtk_plot_symbol_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_SYMBOL_NONE, "GTK_PLOT_SYMBOL_NONE", "none" },
      { GTK_PLOT_SYMBOL_SQUARE, "GTK_PLOT_SYMBOL_SQUARE", "square" },
      { GTK_PLOT_SYMBOL_CIRCLE, "GTK_PLOT_SYMBOL_CIRCLE", "circle" },
      { GTK_PLOT_SYMBOL_UP_TRIANGLE, "GTK_PLOT_SYMBOL_UP_TRIANGLE", "up-triangle" },
      { GTK_PLOT_SYMBOL_DOWN_TRIANGLE, "GTK_PLOT_SYMBOL_DOWN_TRIANGLE", "down-triangle" },
      { GTK_PLOT_SYMBOL_RIGHT_TRIANGLE, "GTK_PLOT_SYMBOL_RIGHT_TRIANGLE", "right-triangle" },
      { GTK_PLOT_SYMBOL_LEFT_TRIANGLE, "GTK_PLOT_SYMBOL_LEFT_TRIANGLE", "left-triangle" },
      { GTK_PLOT_SYMBOL_DIAMOND, "GTK_PLOT_SYMBOL_DIAMOND", "diamond" },
      { GTK_PLOT_SYMBOL_PLUS, "GTK_PLOT_SYMBOL_PLUS", "plus" },
      { GTK_PLOT_SYMBOL_CROSS, "GTK_PLOT_SYMBOL_CROSS", "cross" },
      { GTK_PLOT_SYMBOL_STAR, "GTK_PLOT_SYMBOL_STAR", "star" },
      { GTK_PLOT_SYMBOL_DOT, "GTK_PLOT_SYMBOL_DOT", "dot" },
      { GTK_PLOT_SYMBOL_IMPULSE, "GTK_PLOT_SYMBOL_IMPULSE", "impulse" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotSymbolType", values);
  }
  return etype;
}
GType
gtk_plot_symbol_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_SYMBOL_EMPTY, "GTK_PLOT_SYMBOL_EMPTY", "empty" },
      { GTK_PLOT_SYMBOL_FILLED, "GTK_PLOT_SYMBOL_FILLED", "filled" },
      { GTK_PLOT_SYMBOL_OPAQUE, "GTK_PLOT_SYMBOL_OPAQUE", "opaque" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotSymbolStyle", values);
  }
  return etype;
}
GType
gtk_plot_border_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_BORDER_NONE, "GTK_PLOT_BORDER_NONE", "none" },
      { GTK_PLOT_BORDER_LINE, "GTK_PLOT_BORDER_LINE", "line" },
      { GTK_PLOT_BORDER_SHADOW, "GTK_PLOT_BORDER_SHADOW", "shadow" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotBorderStyle", values);
  }
  return etype;
}
GType
gtk_plot_line_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_LINE_NONE, "GTK_PLOT_LINE_NONE", "none" },
      { GTK_PLOT_LINE_SOLID, "GTK_PLOT_LINE_SOLID", "solid" },
      { GTK_PLOT_LINE_DOTTED, "GTK_PLOT_LINE_DOTTED", "dotted" },
      { GTK_PLOT_LINE_DASHED, "GTK_PLOT_LINE_DASHED", "dashed" },
      { GTK_PLOT_LINE_DOT_DASH, "GTK_PLOT_LINE_DOT_DASH", "dot-dash" },
      { GTK_PLOT_LINE_DOT_DOT_DASH, "GTK_PLOT_LINE_DOT_DOT_DASH", "dot-dot-dash" },
      { GTK_PLOT_LINE_DOT_DASH_DASH, "GTK_PLOT_LINE_DOT_DASH_DASH", "dot-dash-dash" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotLineStyle", values);
  }
  return etype;
}
GType
gtk_plot_connector_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_CONNECT_NONE, "GTK_PLOT_CONNECT_NONE", "none" },
      { GTK_PLOT_CONNECT_STRAIGHT, "GTK_PLOT_CONNECT_STRAIGHT", "straight" },
      { GTK_PLOT_CONNECT_SPLINE, "GTK_PLOT_CONNECT_SPLINE", "spline" },
      { GTK_PLOT_CONNECT_HV_STEP, "GTK_PLOT_CONNECT_HV_STEP", "hv-step" },
      { GTK_PLOT_CONNECT_VH_STEP, "GTK_PLOT_CONNECT_VH_STEP", "vh-step" },
      { GTK_PLOT_CONNECT_MIDDLE_STEP, "GTK_PLOT_CONNECT_MIDDLE_STEP", "middle-step" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotConnector", values);
  }
  return etype;
}
GType
gtk_plot_label_pos_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GTK_PLOT_LABEL_NONE, "GTK_PLOT_LABEL_NONE", "none" },
      { GTK_PLOT_LABEL_IN, "GTK_PLOT_LABEL_IN", "in" },
      { GTK_PLOT_LABEL_OUT, "GTK_PLOT_LABEL_OUT", "out" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GtkPlotLabelPos", values);
  }
  return etype;
}
GType
gtk_plot_error_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_ERROR_DIV_ZERO, "GTK_PLOT_ERROR_DIV_ZERO", "div-zero" },
      { GTK_PLOT_ERROR_LOG_NEG, "GTK_PLOT_ERROR_LOG_NEG", "log-neg" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotError", values);
  }
  return etype;
}
GType
gtk_plot_orientation_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_AXIS_X, "GTK_PLOT_AXIS_X", "x" },
      { GTK_PLOT_AXIS_Y, "GTK_PLOT_AXIS_Y", "y" },
      { GTK_PLOT_AXIS_Z, "GTK_PLOT_AXIS_Z", "z" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotOrientation", values);
  }
  return etype;
}
GType
gtk_plot_axis_pos_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_AXIS_LEFT, "GTK_PLOT_AXIS_LEFT", "left" },
      { GTK_PLOT_AXIS_RIGHT, "GTK_PLOT_AXIS_RIGHT", "right" },
      { GTK_PLOT_AXIS_TOP, "GTK_PLOT_AXIS_TOP", "top" },
      { GTK_PLOT_AXIS_BOTTOM, "GTK_PLOT_AXIS_BOTTOM", "bottom" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotAxisPos", values);
  }
  return etype;
}
GType
gtk_plot_label_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_LABEL_FLOAT, "GTK_PLOT_LABEL_FLOAT", "float" },
      { GTK_PLOT_LABEL_EXP, "GTK_PLOT_LABEL_EXP", "exp" },
      { GTK_PLOT_LABEL_POW, "GTK_PLOT_LABEL_POW", "pow" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotLabelStyle", values);
  }
  return etype;
}
GType
gtk_plot_ticks_pos_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GTK_PLOT_TICKS_NONE, "GTK_PLOT_TICKS_NONE", "none" },
      { GTK_PLOT_TICKS_IN, "GTK_PLOT_TICKS_IN", "in" },
      { GTK_PLOT_TICKS_OUT, "GTK_PLOT_TICKS_OUT", "out" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GtkPlotTicksPos", values);
  }
  return etype;
}

/* enumerations from "gtkplot3d.h" */
GType
gtk_plot_plane_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_PLANE_XY, "GTK_PLOT_PLANE_XY", "xy" },
      { GTK_PLOT_PLANE_YX, "GTK_PLOT_PLANE_YX", "yx" },
      { GTK_PLOT_PLANE_XZ, "GTK_PLOT_PLANE_XZ", "xz" },
      { GTK_PLOT_PLANE_ZX, "GTK_PLOT_PLANE_ZX", "zx" },
      { GTK_PLOT_PLANE_YZ, "GTK_PLOT_PLANE_YZ", "yz" },
      { GTK_PLOT_PLANE_ZY, "GTK_PLOT_PLANE_ZY", "zy" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotPlane", values);
  }
  return etype;
}
GType
gtk_plot_side_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GTK_PLOT_SIDE_XY, "GTK_PLOT_SIDE_XY", "xy" },
      { GTK_PLOT_SIDE_XZ, "GTK_PLOT_SIDE_XZ", "xz" },
      { GTK_PLOT_SIDE_YX, "GTK_PLOT_SIDE_YX", "yx" },
      { GTK_PLOT_SIDE_YZ, "GTK_PLOT_SIDE_YZ", "yz" },
      { GTK_PLOT_SIDE_ZX, "GTK_PLOT_SIDE_ZX", "zx" },
      { GTK_PLOT_SIDE_ZY, "GTK_PLOT_SIDE_ZY", "zy" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GtkPlotSide", values);
  }
  return etype;
}

/* enumerations from "gtkplotbar.h" */
GType
gtk_plot_bar_units_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_BAR_POINTS, "GTK_PLOT_BAR_POINTS", "points" },
      { GTK_PLOT_BAR_RELATIVE, "GTK_PLOT_BAR_RELATIVE", "relative" },
      { GTK_PLOT_BAR_ABSOLUTE, "GTK_PLOT_BAR_ABSOLUTE", "absolute" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotBarUnits", values);
  }
  return etype;
}

/* enumerations from "gtkplotcanvas.h" */
GType
gtk_plot_canvas_action_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_CANVAS_ACTION_INACTIVE, "GTK_PLOT_CANVAS_ACTION_INACTIVE", "inactive" },
      { GTK_PLOT_CANVAS_ACTION_SELECTION, "GTK_PLOT_CANVAS_ACTION_SELECTION", "selection" },
      { GTK_PLOT_CANVAS_ACTION_DRAG, "GTK_PLOT_CANVAS_ACTION_DRAG", "drag" },
      { GTK_PLOT_CANVAS_ACTION_RESIZE, "GTK_PLOT_CANVAS_ACTION_RESIZE", "resize" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotCanvasAction", values);
  }
  return etype;
}
GType
gtk_plot_canvas_flag_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GTK_PLOT_CANVAS_FROZEN, "GTK_PLOT_CANVAS_FROZEN", "frozen" },
      { GTK_PLOT_CANVAS_CAN_MOVE, "GTK_PLOT_CANVAS_CAN_MOVE", "can-move" },
      { GTK_PLOT_CANVAS_CAN_RESIZE, "GTK_PLOT_CANVAS_CAN_RESIZE", "can-resize" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GtkPlotCanvasFlag", values);
  }
  return etype;
}
GType
gtk_plot_canvas_selection_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_CANVAS_SELECT_NONE, "GTK_PLOT_CANVAS_SELECT_NONE", "none" },
      { GTK_PLOT_CANVAS_SELECT_MARKERS, "GTK_PLOT_CANVAS_SELECT_MARKERS", "markers" },
      { GTK_PLOT_CANVAS_SELECT_TARGET, "GTK_PLOT_CANVAS_SELECT_TARGET", "target" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotCanvasSelection", values);
  }
  return etype;
}
GType
gtk_plot_canvas_selection_mode_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_CANVAS_SELECT_CLICK_1, "GTK_PLOT_CANVAS_SELECT_CLICK_1", "1" },
      { GTK_PLOT_CANVAS_SELECT_CLICK_2, "GTK_PLOT_CANVAS_SELECT_CLICK_2", "2" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotCanvasSelectionMode", values);
  }
  return etype;
}
GType
gtk_plot_canvas_pos_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_CANVAS_OUT, "GTK_PLOT_CANVAS_OUT", "out" },
      { GTK_PLOT_CANVAS_IN, "GTK_PLOT_CANVAS_IN", "in" },
      { GTK_PLOT_CANVAS_LEFT, "GTK_PLOT_CANVAS_LEFT", "left" },
      { GTK_PLOT_CANVAS_RIGHT, "GTK_PLOT_CANVAS_RIGHT", "right" },
      { GTK_PLOT_CANVAS_TOP, "GTK_PLOT_CANVAS_TOP", "top" },
      { GTK_PLOT_CANVAS_BOTTOM, "GTK_PLOT_CANVAS_BOTTOM", "bottom" },
      { GTK_PLOT_CANVAS_TOP_LEFT, "GTK_PLOT_CANVAS_TOP_LEFT", "top-left" },
      { GTK_PLOT_CANVAS_TOP_RIGHT, "GTK_PLOT_CANVAS_TOP_RIGHT", "top-right" },
      { GTK_PLOT_CANVAS_BOTTOM_LEFT, "GTK_PLOT_CANVAS_BOTTOM_LEFT", "bottom-left" },
      { GTK_PLOT_CANVAS_BOTTOM_RIGHT, "GTK_PLOT_CANVAS_BOTTOM_RIGHT", "bottom-right" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotCanvasPos", values);
  }
  return etype;
}

/* enumerations from "gtkplotcsurface.h" */
GType
gtk_plot_projection_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_PROJECT_NONE, "GTK_PLOT_PROJECT_NONE", "none" },
      { GTK_PLOT_PROJECT_EMPTY, "GTK_PLOT_PROJECT_EMPTY", "empty" },
      { GTK_PLOT_PROJECT_FULL, "GTK_PLOT_PROJECT_FULL", "full" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotProjection", values);
  }
  return etype;
}

/* enumerations from "gtkplotpc.h" */
GType
gtk_plot_page_size_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_LETTER, "GTK_PLOT_LETTER", "letter" },
      { GTK_PLOT_LEGAL, "GTK_PLOT_LEGAL", "legal" },
      { GTK_PLOT_A4, "GTK_PLOT_A4", "a4" },
      { GTK_PLOT_EXECUTIVE, "GTK_PLOT_EXECUTIVE", "executive" },
      { GTK_PLOT_CUSTOM, "GTK_PLOT_CUSTOM", "custom" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotPageSize", values);
  }
  return etype;
}
GType
gtk_plot_page_orientation_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_PORTRAIT, "GTK_PLOT_PORTRAIT", "portrait" },
      { GTK_PLOT_LANDSCAPE, "GTK_PLOT_LANDSCAPE", "landscape" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotPageOrientation", values);
  }
  return etype;
}
GType
gtk_plot_units_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_PSPOINTS, "GTK_PLOT_PSPOINTS", "pspoints" },
      { GTK_PLOT_MM, "GTK_PLOT_MM", "mm" },
      { GTK_PLOT_CM, "GTK_PLOT_CM", "cm" },
      { GTK_PLOT_INCHES, "GTK_PLOT_INCHES", "inches" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotUnits", values);
  }
  return etype;
}

/* enumerations from "gtkplotsegment.h" */
GType
gtk_plot_arrow_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GTK_PLOT_ARROW_NONE, "GTK_PLOT_ARROW_NONE", "none" },
      { GTK_PLOT_ARROW_ORIGIN, "GTK_PLOT_ARROW_ORIGIN", "origin" },
      { GTK_PLOT_ARROW_END, "GTK_PLOT_ARROW_END", "end" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GtkPlotArrow", values);
  }
  return etype;
}

/* enumerations from "gtksheet.h" */
GType
gtk_sheet_attr_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_SHEET_FOREGROUND, "GTK_SHEET_FOREGROUND", "foreground" },
      { GTK_SHEET_BACKGROUND, "GTK_SHEET_BACKGROUND", "background" },
      { GTK_SHEET_FONT, "GTK_SHEET_FONT", "font" },
      { GTK_SHEET_JUSTIFICATION, "GTK_SHEET_JUSTIFICATION", "justification" },
      { GTK_SHEET_BORDER, "GTK_SHEET_BORDER", "border" },
      { GTK_SHEET_BORDER_COLOR, "GTK_SHEET_BORDER_COLOR", "border-color" },
      { GTK_SHEET_IS_EDITABLE, "GTK_SHEET_IS_EDITABLE", "is-editable" },
      { GTK_SHEET_IS_VISIBLE, "GTK_SHEET_IS_VISIBLE", "is-visible" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkSheetAttrType", values);
  }
  return etype;
}
GType
gtk_sheet_state_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_SHEET_NORMAL, "GTK_SHEET_NORMAL", "normal" },
      { GTK_SHEET_ROW_SELECTED, "GTK_SHEET_ROW_SELECTED", "row-selected" },
      { GTK_SHEET_COLUMN_SELECTED, "GTK_SHEET_COLUMN_SELECTED", "column-selected" },
      { GTK_SHEET_RANGE_SELECTED, "GTK_SHEET_RANGE_SELECTED", "range-selected" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkSheetState", values);
  }
  return etype;
}
GType
gtk_sheet_data_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_SHEET_DATA_TYPE_NONE, "GTK_SHEET_DATA_TYPE_NONE", "none" },
      { GTK_SHEET_DATA_TYPE_INT, "GTK_SHEET_DATA_TYPE_INT", "int" },
      { GTK_SHEET_DATA_TYPE_FLOAT, "GTK_SHEET_DATA_TYPE_FLOAT", "float" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkSheetDataType", values);
  }
  return etype;
}
GType
gtk_sheet_entry_type_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_SHEET_ENTRY_TYPE_DEFAULT, "GTK_SHEET_ENTRY_TYPE_DEFAULT", "default" },
      { GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY, "GTK_SHEET_ENTRY_TYPE_GTK_ITEM_ENTRY", "gtk-item-entry" },
      { GTK_SHEET_ENTRY_TYPE_GTK_ENTRY, "GTK_SHEET_ENTRY_TYPE_GTK_ENTRY", "gtk-entry" },
      { GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW, "GTK_SHEET_ENTRY_TYPE_GTK_TEXT_VIEW", "gtk-text-view" },
      { GTK_SHEET_ENTRY_TYPE_GTK_SPIN_BUTTON, "GTK_SHEET_ENTRY_TYPE_GTK_SPIN_BUTTON", "gtk-spin-button" },
      { GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX, "GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX", "gtk-combo-box" },
      { GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX_ENTRY, "GTK_SHEET_ENTRY_TYPE_GTK_COMBO_BOX_ENTRY", "gtk-combo-box-entry" },
      { GTK_SHEET_ENTRY_TYPE_GTK_COMBO, "GTK_SHEET_ENTRY_TYPE_GTK_COMBO", "gtk-combo" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkSheetEntryType", values);
  }
  return etype;
}
GType
gtk_sheet_vertical_justification_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT, "GTK_SHEET_VERTICAL_JUSTIFICATION_DEFAULT", "default" },
      { GTK_SHEET_VERTICAL_JUSTIFICATION_TOP, "GTK_SHEET_VERTICAL_JUSTIFICATION_TOP", "top" },
      { GTK_SHEET_VERTICAL_JUSTIFICATION_MIDDLE, "GTK_SHEET_VERTICAL_JUSTIFICATION_MIDDLE", "middle" },
      { GTK_SHEET_VERTICAL_JUSTIFICATION_BOTTOM, "GTK_SHEET_VERTICAL_JUSTIFICATION_BOTTOM", "bottom" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkSheetVerticalJustification", values);
  }
  return etype;
}

/* enumerations from "gtkplotcanvasline.h" */
GType
gtk_plot_canvas_arrow_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { GTK_PLOT_CANVAS_ARROW_NONE, "GTK_PLOT_CANVAS_ARROW_NONE", "none" },
      { GTK_PLOT_CANVAS_ARROW_ORIGIN, "GTK_PLOT_CANVAS_ARROW_ORIGIN", "origin" },
      { GTK_PLOT_CANVAS_ARROW_END, "GTK_PLOT_CANVAS_ARROW_END", "end" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("GtkPlotCanvasArrow", values);
  }
  return etype;
}

/* enumerations from "gtkplotcanvasplot.h" */
GType
gtk_plot_canvas_plot_pos_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PLOT_CANVAS_PLOT_OUT, "GTK_PLOT_CANVAS_PLOT_OUT", "out" },
      { GTK_PLOT_CANVAS_PLOT_IN_PLOT, "GTK_PLOT_CANVAS_PLOT_IN_PLOT", "in-plot" },
      { GTK_PLOT_CANVAS_PLOT_IN_LEGENDS, "GTK_PLOT_CANVAS_PLOT_IN_LEGENDS", "in-legends" },
      { GTK_PLOT_CANVAS_PLOT_IN_TITLE, "GTK_PLOT_CANVAS_PLOT_IN_TITLE", "in-title" },
      { GTK_PLOT_CANVAS_PLOT_IN_AXIS, "GTK_PLOT_CANVAS_PLOT_IN_AXIS", "in-axis" },
      { GTK_PLOT_CANVAS_PLOT_IN_DATA, "GTK_PLOT_CANVAS_PLOT_IN_DATA", "in-data" },
      { GTK_PLOT_CANVAS_PLOT_IN_GRADIENT, "GTK_PLOT_CANVAS_PLOT_IN_GRADIENT", "in-gradient" },
      { GTK_PLOT_CANVAS_PLOT_IN_MARKER, "GTK_PLOT_CANVAS_PLOT_IN_MARKER", "in-marker" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPlotCanvasPlotPos", values);
  }
  return etype;
}



