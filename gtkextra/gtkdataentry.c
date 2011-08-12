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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gtk/gtk.h>
#include "gtkextra-compat.h"
#include "gtkdataentry.h"

/**
 * SECTION: gtkdataentry
 * @short_description: a data entry widget, based on GtkEntry
 *
 * GtkDataEntry provides additional features:
 * 
 * - description (property): no functionality, a place for 
 *   private information that cannot be put anywhere else
 *  
 * - datatype (property): no functionality, a datatype hint for 
 *   the application because any widget content is text
 *  
 * - dataformat (property): a formatting string that controls 
 *   what you see when the widget doesn't contain input focus
 *  
 * The main reason for this widget is to provide a formatting  
 * entry widget for numeric data like integer, float, money 
 * which also supports NULL values (GtkSpinButton is nice but 
 * doesn't support empty field values). Handling of Null values 
 * is supported by all SQL database systems. 
 */

enum
{
    PROP_0,
    PROP_DATA_ENTRY_DESCRIPTION,
    PROP_DATA_ENTRY_DATATYPE,
    PROP_DATA_ENTRY_DATAFORMAT,
} GTK_DATA_ENTRY_PROPERTIES;

enum
{
    LAST_SIGNAL
} GTK_DATA_ENTRY_SIGNALS;

static void gtk_data_entry_class_init(GtkDataEntryClass *klass);
static void gtk_data_entry_init(GtkDataEntry *data);

static GtkEntryClass *parent_class = NULL;

/**
 * gtk_data_entry_set_description: 
 * @data_entry:  a #GtkDataEntry
 * @description:  the description or NULL 
 *  
 * Sets the GtkDataEntry description. 
 */
void gtk_data_entry_set_description(GtkDataEntry *data_entry, 
                                    const gchar *description)
{
    g_return_if_fail (data_entry != NULL);
    g_return_if_fail (GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->description) g_free(data_entry->description);
    data_entry->description = g_strdup(description);
}

/**
 * gtk_data_entry_set_data_type: 
 * @data_entry:  a #GtkDataEntry
 * @data_type:  the data type or NULL 
 *  
 * Sets the GtkDataEntry data type for application use. 
 */
void gtk_data_entry_set_data_type(GtkDataEntry *data_entry, 
                                  const gchar *data_type)
{
    g_return_if_fail (data_entry != NULL);
    g_return_if_fail (GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->data_type) g_free(data_entry->data_type);
    data_entry->data_type = g_strdup(data_type);
}

/**
 * gtk_data_entry_set_data_format: 
 * @data_entry:  a #GtkDataEntry
 * @data_format:  the data type or NULL 
 *  
 * Sets the GtkDataEntry data type for application use. 
 */
void gtk_data_entry_set_data_format(GtkDataEntry *data_entry, 
                                    const gchar *data_format)
{
    g_return_if_fail (data_entry != NULL);
    g_return_if_fail (GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->data_format) g_free(data_entry->data_format);
    data_entry->data_format = g_strdup(data_format);
}




GType
    gtk_data_entry_get_type(void)
{
    static GType data_entry_type = 0;

    if (!data_entry_type)
    {
        static const GInterfaceInfo interface_info = {
            (GInterfaceInitFunc) NULL,
            (GInterfaceFinalizeFunc) NULL,
            (gpointer) NULL
        };

        data_entry_type = g_type_register_static_simple(
                                                       gtk_entry_get_type(),
                                                       "GtkDataEntry",
                                                       sizeof (GtkDataEntryClass),
                                                       (GClassInitFunc) gtk_data_entry_class_init,
                                                       sizeof (GtkDataEntry),
                                                       (GInstanceInitFunc) gtk_data_entry_init,
                                                       0);

        g_type_add_interface_static (data_entry_type, 
                                     GTK_TYPE_BUILDABLE, 
                                     &interface_info);
    }
    return(data_entry_type);
}


static void
    gtk_data_entry_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY (object);

    switch (prop_id)
    {
        case PROP_DATA_ENTRY_DESCRIPTION:
            {
                const gchar *description = g_value_get_string(value);

                if (!gtk_widget_get_realized(GTK_WIDGET(data_entry)))
                {
                    if (data_entry->description) g_free(data_entry->description);
                    data_entry->description = g_strdup(description);
                }
                else
                {
                    gtk_data_entry_set_description(data_entry, description);
                }
            }
            break;

        case PROP_DATA_ENTRY_DATATYPE:
            {
                const gchar *data_type = g_value_get_string(value);

                if (!gtk_widget_get_realized(GTK_WIDGET(data_entry)))
                {
                    if (data_entry->data_type) g_free(data_entry->data_type);
                    data_entry->data_type = g_strdup(data_type);
                }
                else
                {
                    gtk_data_entry_set_data_type(data_entry, data_type);
                }
            }
            break;

        case PROP_DATA_ENTRY_DATAFORMAT:
            {
                const gchar *data_format = g_value_get_string(value);

                if (!gtk_widget_get_realized(GTK_WIDGET(data_entry)))
                {
                    if (data_entry->data_format) g_free(data_entry->data_format);
                    data_entry->data_format = g_strdup(data_format);
                }
                else
                {
                    gtk_data_entry_set_data_format(data_entry, data_format);
                }
            }
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
    gtk_data_entry_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY (object);

    switch (prop_id)
    {
        case PROP_DATA_ENTRY_DESCRIPTION:
            g_value_set_string(value, data_entry->description);
            break;

        case PROP_DATA_ENTRY_DATATYPE:
            g_value_set_string(value, data_entry->data_type);
            break;

        case PROP_DATA_ENTRY_DATAFORMAT:
            g_value_set_string(value, data_entry->data_format);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}



static void
    gtk_data_entry_class_init(GtkDataEntryClass *class)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
#if 0
    GtkObjectClass *object_class = GTK_OBJECT_CLASS (class);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
    GtkEntryClass *entry_class = GTK_ENTRY_CLASS (class);
#endif

    parent_class = g_type_class_ref (gtk_entry_get_type ());

    gobject_class->set_property = gtk_data_entry_set_property;
    gobject_class->get_property = gtk_data_entry_get_property;

    /**
     * GtkDataEntry:description:
     *
     * Description of the GtkDataEntry, no functionality, a place 
     * for private information that cannot be put anywhere else.
     */
    g_object_class_install_property(gobject_class, 
                                    PROP_DATA_ENTRY_DESCRIPTION, 
                                    g_param_spec_string ("description", 
                                                         "Description",
                                                         "Description of the GtkDataEntry",
                                                         "" /* default value */,
                                                         G_PARAM_READWRITE));

    /**
     * GtkDataEntry:datatype:
     *
     * no functionality, a datatype hint for the application because 
     * any widget content is text. 
     */
    g_object_class_install_property (gobject_class, 
                                     PROP_DATA_ENTRY_DATATYPE, 
                                     g_param_spec_string ("datatype", 
                                                          "Datatype",
                                                          "Application-Datatype",
                                                          "" /* default value */,
                                                          G_PARAM_READWRITE));

    /**
     * GtkDataEntry:dataformat:
     *
     * a formatting string that controls what you see when the 
     * widget doesn't contain input focus. 
     */
    g_object_class_install_property (gobject_class, 
                                     PROP_DATA_ENTRY_DATAFORMAT, 
                                     g_param_spec_string ("dataformat", 
                                                          "Data-Format",
                                                          "A formatting string that controls what you see when the widget doesn't contain input focus",
                                                          "" /* default value */,
                                                          G_PARAM_READWRITE));
}


static void
    gtk_data_entry_init (GtkDataEntry *data_entry)
{
    GtkWidget *widget = GTK_WIDGET(data_entry);

    data_entry->description = NULL;
    data_entry->data_type = NULL;
    data_entry->data_format = NULL;
}

/**
 * gtk_data_entry_new: 
 *  
 * Creates a new GtkDataEntry Widget. 
 *  
 * Returns: the new GtkDataEntry Widget 
 */
GtkDataEntry*
    gtk_data_entry_new (void)
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY(
                                             gtk_widget_new (gtk_data_entry_get_type (), NULL));

    return(data_entry);
}


