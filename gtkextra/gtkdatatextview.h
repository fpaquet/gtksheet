/* gtkdatatextview - data textview widget, based on GtkTextView
 * Copyright 2013  Fredy Paquet <fredy@opag.ch>
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

#ifndef __GTK_DATA_TEXT_VIEW_H__
#define __GTK_DATA_TEXT_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_DATA_TEXT_VIEW              (gtk_data_text_view_get_type ())
#define GTK_DATA_TEXT_VIEW(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DATA_TEXT_VIEW, GtkDataTextView))
#define GTK_DATA_TEXT_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_DATA_TEXT_VIEW, GtkDataTextViewClass))
#define GTK_IS_DATA_TEXT_VIEW(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_DATA_TEXT_VIEW))
#define GTK_IS_DATA_TEXT_VIEW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DATA_TEXT_VIEW))
#define GTK_DATA_TEXT_VIEW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_DATA_TEXT_VIEW, GtkDataTextViewClass))

typedef struct _GtkDataTextView        GtkDataTextView;
typedef struct _GtkDataTextViewClass  GtkDataTextViewClass;

/**
 * GtkDataTextView:
 *
 * The GtkDataTextView struct contains only private data. It should
 * only be accessed through the functions described below. 
 */
struct _GtkDataTextView
{
    /*< private >*/
    GtkTextView textview;

    gchar *description;         /* column description */
    gint max_length;   /* maximum length in characters */
    gint max_length_bytes;   /* maximum length in bytes */
};

struct _GtkDataTextViewClass
{
    GtkTextViewClass parent_class;
};

GType gtk_data_text_view_get_type(void) G_GNUC_CONST;
GtkDataTextView *gtk_data_text_view_new(void);

const gchar *
    gtk_data_text_view_get_description(GtkDataTextView *data_text_view);

void gtk_data_text_view_set_description(GtkDataTextView *data_text_view, 
                                    const gchar *description);

gint gtk_data_text_view_get_max_length(GtkDataTextView *data_text_view);

void gtk_data_text_view_set_max_length(GtkDataTextView *data_text_view, 
                                    gint max_length);

gint gtk_data_text_view_get_max_length_bytes(GtkDataTextView *data_text_view);

void gtk_data_text_view_set_max_length_bytes(GtkDataTextView *data_text_view, 
                                    gint max_length_bytes);

G_END_DECLS

#endif /* __GTK_DATA_TEXT_VIEW_H__ */
