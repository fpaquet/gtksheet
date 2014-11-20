/* gtkfontcombo - font_combo widget for gtk+
 * Copyright 1999-2001 Adrian E. Feiguin <feiguin@ifir.edu.ar>
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


#ifndef __GTK_FONT_COMBO_H__
#define __GTK_FONT_COMBO_H__

#include "gtkpsfont.h"

G_BEGIN_DECLS

#define GTK_FONT_COMBO_TYPE		(gtk_font_combo_get_type())
#define GTK_FONT_COMBO(obj)		(G_TYPE_CHECK_INSTANCE_CAST (obj, GTK_FONT_COMBO_TYPE, GtkFontCombo))
#define GTK_FONT_COMBO_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST (klass, GTK_FONT_COMBO_TYPE, GtkFontComboClass))
#define GTK_IS_FONT_COMBO(obj)       	(G_TYPE_CHECK_INSTANCE_TYPE (obj, GTK_FONT_COMBO_TYPE))
#define GTK_IS_FONT_COMBO_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE (obj, GTK_FONT_COMBO_TYPE))

typedef struct _GtkFontCombo		GtkFontCombo;
typedef struct _GtkFontComboClass	GtkFontComboClass;

/* you should access only the entry and list fields directly */
struct _GtkFontCombo {
	GtkToolbar toolbar;

	GtkWidget *name_combo;
	GtkWidget *size_combo;
        GtkWidget *bold_button;
	GtkWidget *italic_button;
};

struct _GtkFontComboClass {
	GtkToolbarClass parent_class;

        void (* changed)      (GtkFontCombo *font_combo);
};

GType      gtk_font_combo_get_type              (void);

GtkWidget *gtk_font_combo_new                   (void);

void	   gtk_font_combo_select		(GtkFontCombo *font_combo,
						 const gchar *family,
			                         gboolean bold,
                        			 gboolean italic,
						 gint height);
void	   gtk_font_combo_select_nth		(GtkFontCombo *font_combo,
						 gint n,
			                         gboolean bold,
                        			 gboolean italic,
						 gint height);
gint	   gtk_font_combo_get_font_height	(GtkFontCombo *font_combo);
GtkPSFont  *gtk_font_combo_get_psfont		(GtkFontCombo *font_combo);
PangoFontDescription  *gtk_font_combo_get_font_description (GtkFontCombo *font_combo);

G_END_DECLS

#endif /* __GTK_FONT_COMBO_H__ */


