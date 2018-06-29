/* GtkSheet widget for Gtk+.
 * Copyright (C) 2018 F. Paquet <mailbox@opag.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Library General Public License for more details.
 *
 */

#ifndef GTK_SHEET_DEBUG_MACROS_DEFINED
#define GTK_SHEET_DEBUG_MACROS_DEFINED

#if 0 || !defined(GTK_SHEET_ENABLE_DEBUG_MACROS)
# define DEBUG_WIDGET_SET_PARENT(widget, parent)
#else
# define DEBUG_WIDGET_SET_PARENT(widget, parent) \
    g_debug( \
        "%s(%d) FIXME gtk_widget_set_parent of %s %p to %s %p",  \
        __FUNCTION__, __LINE__,  \
        G_OBJECT_TYPE_NAME(widget), widget, \
        G_OBJECT_TYPE_NAME (parent), parent)
#endif

#if 1 || +defined(GTK_SHEET_ENABLE_DEBUG_MACROS)
# define DEBUG_WIDGET_SET_PARENT_WIN(widget, parent)
#else
# define DEBUG_WIDGET_SET_PARENT_WIN(widget, parent) \
    g_debug( \
        "%s(%d) FIXME gtk_widget_set_parent_window of %s %p to %s %p",  \
        __FUNCTION__, __LINE__,  \
        G_OBJECT_TYPE_NAME(widget), widget, \
        G_OBJECT_TYPE_NAME(parent), parent)
#endif

#if 0 || !defined(GTK_SHEET_ENABLE_DEBUG_MACROS)
# define DEBUG_WIDGET_CONTAINER_ADD(parent, widget)
#else
# define DEBUG_WIDGET_CONTAINER_ADD(parent, widget) \
    g_debug( \
        "%s(%d) FIXME gtk_container_add of %s %p to %s %p",  \
        __FUNCTION__, __LINE__,  \
        G_OBJECT_TYPE_NAME(widget), widget, \
        G_OBJECT_TYPE_NAME (parent), parent)
#endif

#if 0 || !defined(GTK_SHEET_ENABLE_DEBUG_MACROS)
# define DEBUG_WIDGET_CONTAINER_REMOVE(parent, widget)
#else
# define DEBUG_WIDGET_CONTAINER_REMOVE(parent, widget) \
    g_debug( \
        "%s(%d) FIXME gtk_container_remove of %s %p from %s %p",  \
        __FUNCTION__, __LINE__,  \
        G_OBJECT_TYPE_NAME(widget), widget, \
        G_OBJECT_TYPE_NAME (parent), parent)
#endif

#endif
