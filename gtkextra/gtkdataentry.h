/* gtkdataentry - data entry widget, based on GtkEntry
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

#ifndef __GTK_DATA_ENTRY_H__
#define __GTK_DATA_ENTRY_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_DATA_ENTRY              (gtk_data_entry_get_type ())
#define GTK_DATA_ENTRY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATA_ENTRY, GtkDataEntry))
#define GTK_DATA_ENTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATA_ENTRY, GtkDataEntryClass))
#define GTK_IS_DATA_ENTRY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATA_ENTRY))
#define GTK_IS_DATA_ENTRY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATA_ENTRY))
#define GTK_DATA_ENTRY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATA_ENTRY, GtkDataEntryClass))

typedef struct _GtkDataEntry        GtkDataEntry;
typedef struct _GtkDataEntryClass  GtkDataEntryClass;

/**
 * GtkDataEntry:
 *
 * The GtkDataEntry struct contains only private data. It should
 * only be accessed through the functions described below. 
 */
struct _GtkDataEntry
{
    /*< private >*/
    GtkEntry entry;

    gchar *data_type;           /* data type for application use */
    gchar *data_format;        /* cell content formatting template */
    gchar *description;         /* column description */
    gint max_length_bytes;   /* maximum length in bytes */
};

struct _GtkDataEntryClass
{
    GtkEntryClass parent_class;
};

GType gtk_data_entry_get_type(void) G_GNUC_CONST;
GtkDataEntry *gtk_data_entry_new(void);

const gchar *
    gtk_data_entry_get_description(GtkDataEntry *data_entry);

void gtk_data_entry_set_description(GtkDataEntry *data_entry, 
                                    const gchar *description);

const gchar*
    gtk_data_entry_get_data_type(GtkDataEntry *data_entry);

void gtk_data_entry_set_data_type(GtkDataEntry *data_entry, 
                                  const gchar *data_type);

const gchar *
gtk_data_entry_get_data_format(GtkDataEntry *data_entry);

void gtk_data_entry_set_data_format(GtkDataEntry *data_entry, 
                                    const gchar *data_format);

const gchar*
    gtk_data_entry_get_text(GtkDataEntry *data_entry);

void gtk_data_entry_set_text(GtkDataEntry *data_entry, 
                                    const gchar *text);

gint gtk_data_entry_get_max_length_bytes(GtkDataEntry *data_entry);

void gtk_data_entry_set_max_length_bytes(GtkDataEntry *data_entry, 
                                    gint max_length_bytes);

G_END_DECLS

#endif /* __GTK_DATA_ENTRY_H__ */
