/* gtkdataformat - data formatter
 * Copyright 2011  Fredy Paquet <fredy@opag.ch>
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

#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION) && !defined (__GTKEXTRA_H_INSIDE__)
    #error "Only <gtkextra/gtkextra.h> can be included directly."
#endif

#ifndef __GTK_DATA_FORMAT_H__
#define __GTK_DATA_FORMAT_H__

G_BEGIN_DECLS

gchar *gtk_data_format(const gchar *str, const gchar *dataformat);
gchar *gtk_data_format_remove(const gchar *str, const gchar *dataformat);

G_END_DECLS

#endif /* __GTK_DATA_FORMAT_H__ */
