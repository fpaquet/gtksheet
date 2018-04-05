/* gtkextra-compat - gtk version compatibility stuff
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

#ifndef GTK_EXTRA_COMPAT_H
#define GTK_EXTRA_COMPAT_H

#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(2,20,0)

    /* before V2.20 */

#  define gtk_widget_get_realized GTK_WIDGET_REALIZED
#  define gtk_widget_get_mapped GTK_WIDGET_MAPPED

#  define gtk_widget_get_requisition(widget, requisitionptr) \
        *(requisitionptr) = GTK_WIDGET(widget)->requisition

#   define gtk_widget_set_realized_true(widget)  \
        GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED)
#   define gtk_widget_set_realized_false(widget)  \
        GTK_WIDGET_UNSET_FLAGS(widget, GTK_REALIZED)

#   define gtk_widget_set_mapped_true(widget)  \
        GTK_WIDGET_SET_FLAGS(widget, GTK_MAPPED)
#   define gtk_widget_set_mapped_false(widget)  \
        GTK_WIDGET_UNSET_FLAGS(widget, GTK_MAPPED)

#else

    /* from V2.20 */

#   define gtk_widget_set_realized_true(widget)  \
		gtk_widget_set_realized(widget, TRUE)
#   define gtk_widget_set_realized_false(widget)  \
		gtk_widget_set_realized(widget, FALSE)

#   define gtk_widget_set_mapped_true(widget)  \
		gtk_widget_set_mapped(widget, TRUE)
#   define gtk_widget_set_mapped_false(widget)  \
		gtk_widget_set_mapped(widget, FALSE)

#endif

#if !GTK_CHECK_VERSION(2,22,0)

    /* before V2.22 */

#   define  GDK_KEY_Return   GDK_Return
#   define  GDK_KEY_KP_Enter   GDK_KP_Enter
#   define  GDK_KEY_Escape   GDK_Escape
#   define  GDK_KEY_Tab   GDK_Tab
#   define  GDK_KEY_ISO_Left_Tab   GDK_ISO_Left_Tab
#   define  GDK_KEY_BackSpace   GDK_BackSpace

#   define  GDK_KEY_Up   GDK_Up
#   define  GDK_KEY_Down   GDK_Down
#   define  GDK_KEY_Left   GDK_Left
#   define  GDK_KEY_Right   GDK_Right
#   define  GDK_KEY_Home   GDK_Home
#   define  GDK_KEY_End   GDK_End
#   define  GDK_KEY_Page_Up   GDK_Page_Up
#   define  GDK_KEY_Page_Down   GDK_Page_Down

#   define  GDK_KEY_Control_L   GDK_Control_L
#   define  GDK_KEY_Control_R   GDK_Control_R
#   define  GDK_KEY_Shift_L   GDK_Shift_L
#   define  GDK_KEY_Shift_R   GDK_Shift_R

#endif


#endif /* GTK_EXTRA_COMPAT_H */
