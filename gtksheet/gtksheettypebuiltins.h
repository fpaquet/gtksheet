


#ifndef __GTKSHEET_TYPE_BUILTINS_H__
#define __GTKSHEET_TYPE_BUILTINS_H__

#include <glib-object.h>

G_BEGIN_DECLS
/* enumerations from "gtksheet.h" */
GType gtk_sheet_attr_type_get_type (void);
#define GTK_TYPE_SHEET_ATTR_TYPE (gtk_sheet_attr_type_get_type())
GType gtk_sheet_state_get_type (void);
#define GTK_TYPE_SHEET_STATE (gtk_sheet_state_get_type())
GType gtk_sheet_entry_type_get_type (void);
#define GTK_TYPE_SHEET_ENTRY_TYPE (gtk_sheet_entry_type_get_type())
GType gtk_sheet_vertical_justification_get_type (void);
#define GTK_TYPE_SHEET_VERTICAL_JUSTIFICATION (gtk_sheet_vertical_justification_get_type())
GType gtk_sheet_traverse_type_get_type (void);
#define GTK_TYPE_SHEET_TRAVERSE_TYPE (gtk_sheet_traverse_type_get_type())
G_END_DECLS

#endif /* __GTKSHEET_TYPE_BUILTINS_H__ */



