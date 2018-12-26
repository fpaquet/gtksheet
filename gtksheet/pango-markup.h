/* serialize-pango-markup - serializer for GtkTextBuffer
 * Copyright 2018  Fredy Paquet <fredy@opag.ch>
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
 * 
 * Based on Osxcart rtf-serialize.c from P. F. Chimento, 2009
 * https://github.com/ptomato/osxcart
 */

#ifndef __SERIALIZE_PANGO_MARKUP_H__
#define __SERIALIZE_PANGO_MARKUP_H__

#include <glib.h>
#include <gtk/gtk.h>

guint8 *serialize_pango_markup(
    GtkTextBuffer *register_buffer,
    GtkTextBuffer *content_buffer,
    const GtkTextIter *start,
    const GtkTextIter *end,
    gsize *length,
    gpointer user_data);

#endif /* __SERIALIZE_PANGO_MARKUP_H__ */
