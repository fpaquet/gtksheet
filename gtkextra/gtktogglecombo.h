/* gtkcolorcombo - color combo widget for gtk+
 * Copyright 1999-2001  Adrian E. Feiguin <feiguin@ifir.edu.ar>
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


#ifndef __GTK_TOGGLE_COMBO_H__
#define __GTK_TOGGLE_COMBO_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gtkcombobutton.h"

#define GTK_TOGGLE_COMBO(obj)			GTK_CHECK_CAST (obj, gtk_toggle_combo_get_type (), GtkToggleCombo)
#define GTK_TOGGLE_COMBO_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, gtk_toggle_combo_get_type (), GtkToggleComboClass)
#define GTK_IS_TOGGLE_COMBO(obj)       GTK_CHECK_TYPE (obj, gtk_toggle_combo_get_type ())

typedef struct _GtkToggleCombo		GtkToggleCombo;
typedef struct _GtkToggleComboClass	GtkToggleComboClass;

/* you should access only the entry and list fields directly */
struct _GtkToggleCombo {
	GtkComboButton toggle_combo;

        gint default_flag:1;

        gint nrows;
        gint ncols;
        gint row;
        gint column;

        GtkWidget ***button;
	GtkWidget *table;
        GtkWidget *custom_button;
};

struct _GtkToggleComboClass {
	GtkComboButtonClass parent_class;

        void (*changed) (GtkToggleCombo *toggle_combo, gint row, gint col); 
};

GtkType      gtk_toggle_combo_get_type          (void);

GtkWidget*   gtk_toggle_combo_new     		(gint nrows, gint ncols);
void	     gtk_toggle_combo_construct   	(GtkToggleCombo *combo,
                                                 gint nrows, gint ncols);

gint	     gtk_toggle_combo_get_nrows		(GtkToggleCombo *combo);
gint	     gtk_toggle_combo_get_ncols		(GtkToggleCombo *combo);

void	     gtk_toggle_combo_select		(GtkToggleCombo *combo,
						 gint row, gint col);
void	     gtk_toggle_combo_get_selection	(GtkToggleCombo *combo,
						 gint *row, gint *col);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_TOGGLE_COMBO_H__ */


