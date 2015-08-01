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

#define __GTKEXTRA_H_INSIDE__

#include "gtkextra-compat.h"
#include "gtkdataentry.h"
#include "gtkdataformat.h"

/** 
 * SECTION: gtkdataentry 
 * @short_description: a data entry widget, based on GtkEntry
 *
 * GtkDataEntry provides additional properties: 
 * 
 * - #GtkDataEntry:description - no functionality, a place for 
 *   private information that cannot be put anywhere else
 * 
 * - #GtkDataEntry:datatype - no functionality, a datatype hint
 *   for the application because any widget content is text
 *
 * - #GtkDataEntry:dataformat - a formatting instruction that 
 *   controls what you see when the widget doesn't have input
 *   focus
 *
 * - #GtkDataEntry:text - set the contents of the widget. if the
 *   widget doesn't have input focus text will be formatted
 *   according to the data_format
 *
 * - #GtkDataEntry:max-length-bytes - set the maximum byte 
 *   length for the contents of the widget.
 *
 * The main reason for this widget is to provide a formatting  
 * entry widget for numeric data like integer, float, money 
 * which also supports NULL values (GtkSpinButton is nice but 
 * doesn't support empty field values). Handling of Null values 
 * is supported by all SQL database systems. The Null values are
 * represented by empty field contents. 
 *
 * When editing field contents, all data formatting is removed 
 * before the focus enters the widget. As soon as the focus 
 * leaves the widget, data will be nicely formatted again. 
 *
 * As an additional feature, the minus sign (negative numbers) 
 * can be entered at the end of the data. As soon as you leave 
 * the field it will be placed properly in front of the number. 
 *
 * See also: gtk_data_format()
 *
 */

#undef GTK_DATA_ENTRY_DEBUG

#ifdef DEBUG
#define GTK_DATA_ENTRY_DEBUG  0  /* define to activate debug output */
#endif

#if GTK_DATA_ENTRY_DEBUG
#define GTK_DATA_ENTRY_DEBUG_SIGNAL  0  /* debug signal handlers */
#endif


enum
{
    PROP_0,
    PROP_DATA_ENTRY_DATATYPE,
    PROP_DATA_ENTRY_DATAFORMAT,
    PROP_DATA_ENTRY_DESCRIPTION,
    PROP_DATA_ENTRY_TEXT,
    PROP_DATA_ENTRY_MAX_LENGTH_BYTES,
} GTK_DATA_ENTRY_PROPERTIES;

enum
{
    LAST_SIGNAL
} GTK_DATA_ENTRY_SIGNALS;

static void gtk_data_entry_class_init(GtkDataEntryClass *klass);
static void gtk_data_entry_init(GtkDataEntry *data);

static GtkEntryClass *parent_class = NULL;

/**
 * gtk_data_entry_get_description:
 * @data_entry: a #GtkDataEntry
 *
 * Retrieves the #GtkDataEntry description.
 *
 * Returns: a pointer to the contents of the widget as a
 *      string. This string points to internally allocated
 *      storage in the widget and must not be freed, modified or
 *      stored.
 **/
const gchar *
gtk_data_entry_get_description(GtkDataEntry *data_entry)
{
    g_return_val_if_fail(GTK_IS_DATA_ENTRY(data_entry), NULL);
    return data_entry->description;
}

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
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->description)
	g_free(data_entry->description);
    data_entry->description = g_strdup(description);
}

/**
 * gtk_data_entry_get_data_type:
 * @data_entry: a #GtkDataEntry
 *
 * Retrieves the #GtkDataEntry data_type. 
 *  
 * Returns: a pointer to the contents of the widget as a
 *      string. This string points to internally allocated
 *      storage in the widget and must not be freed, modified or
 *      stored.
 **/
const gchar *
gtk_data_entry_get_data_type(GtkDataEntry *data_entry)
{
    g_return_val_if_fail(GTK_IS_DATA_ENTRY(data_entry), NULL);
    return data_entry->data_type;
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
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->data_type)
	g_free(data_entry->data_type);
    data_entry->data_type = g_strdup(data_type);
}

/**
 * gtk_data_entry_get_data_format:
 * @data_entry: a #GtkDataEntry
 *
 * Retrieves the #GtkDataEntry data_format. 
 *
 * See gtk_data_format() for details. 
 *
 * Returns: a pointer to the contents of the widget as a
 *      string. This string points to internally allocated
 *      storage in the widget and must not be freed, modified or
 *      stored.
 **/
const gchar *
gtk_data_entry_get_data_format(GtkDataEntry *data_entry)
{
    g_return_val_if_fail(GTK_IS_DATA_ENTRY(data_entry), NULL);
    return data_entry->data_format;
}

/**
 * gtk_data_entry_set_data_format: 
 * @data_entry:  a #GtkDataEntry
 * @data_format:  the data type or NULL 
 *  
 * Sets the GtkDataEntry data type for application use. The 
 * display will not be refreshed upon change. 
 *
 * See gtk_data_format() for details. 
 *
 */
void gtk_data_entry_set_data_format(GtkDataEntry *data_entry,
    const gchar *data_format)
{
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->data_format)
	g_free(data_entry->data_format);
    data_entry->data_format = g_strdup(data_format);
}

/**
 * gtk_data_entry_get_text:
 * @data_entry: a #GtkDataEntry
 *
 * Retrieves the #GtkDataEntry contents. All formatting will be 
 * removed. 
 *  
 * Returns: a pointer to the contents of the widget as a
 *      string. This string points to internally allocated
 *      storage in the widget and must not be freed, modified or
 *      stored.
 **/
const gchar *
gtk_data_entry_get_text(GtkDataEntry *data_entry)
{
    gchar *text;
    g_return_val_if_fail(GTK_IS_DATA_ENTRY(data_entry), NULL);

    text = (gchar *)gtk_entry_get_text(GTK_ENTRY(data_entry));

/* if (!gtk_widget_has_focus(GTK_WIDGET(data_entry)  -- PR#205431 */
	text = gtk_data_format_remove(text, data_entry->data_format);

#if GTK_DATA_ENTRY_DEBUG
    g_debug("gtk_data_entry_get_text: %s", text ? text : "");
#endif

    return text;
}

/**
 * gtk_data_entry_set_text: 
 * @data_entry:  a #GtkDataEntry
 * @text:  the contents to be set
 *  
 * Sets the GtkDataEntry contents. The contents will be 
 * formatted due to the current data_format. 
 */
void gtk_data_entry_set_text(GtkDataEntry *data_entry,
    const gchar *text)
{
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    if (!gtk_widget_has_focus(GTK_WIDGET(data_entry)))
    {
	text = gtk_data_format(text, data_entry->data_format);
    }

#if GTK_DATA_ENTRY_DEBUG
    g_debug("gtk_data_entry_set_text: %s", text ? text : "");
#endif

    gtk_entry_set_text(GTK_ENTRY(data_entry), text);
}

/**
 * gtk_data_entry_get_max_length_bytes:
 * @data_entry: a #GtkDataEntry
 *
 * Retrieves the maximum byte length for the contents of  
 * #GtkDataEntry. 
 *
 * Returns: maximum byte length or 0. 
 *  
 * Since: 3.0.6 
 **/
gint
gtk_data_entry_get_max_length_bytes(GtkDataEntry *data_entry)
{
    g_return_val_if_fail(GTK_IS_DATA_ENTRY(data_entry), 0);
    return data_entry->max_length_bytes;
}

/**
 * gtk_data_entry_set_max_length_bytes: 
 * @data_entry:  a #GtkDataEntry
 * @max_length_bytes:  maximum byte length or 0
 *
 * Sets the maximum byte length for the contents of the 
 * #GtkDataEntry. Existing content will not be truncted. 
 *  
 * Since: 3.0.6 
 */
void gtk_data_entry_set_max_length_bytes(GtkDataEntry *data_entry,
    gint max_length_bytes)
{
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    if (max_length_bytes < 0) 
	max_length_bytes = 0;

    if (max_length_bytes > GTK_ENTRY_BUFFER_MAX_SIZE) 
	max_length_bytes = GTK_ENTRY_BUFFER_MAX_SIZE;

    data_entry->max_length_bytes = max_length_bytes;
}





GType
gtk_data_entry_get_type(void)
{
    static GType data_entry_type = 0;

    if (!data_entry_type)
    {
	static const GInterfaceInfo interface_info = {
	    (GInterfaceInitFunc)NULL,
	    (GInterfaceFinalizeFunc)NULL,
	    (gpointer)NULL
	};

	data_entry_type = g_type_register_static_simple(
	    gtk_entry_get_type(),
	    "GtkDataEntry",
	    sizeof(GtkDataEntryClass),
	    (GClassInitFunc)gtk_data_entry_class_init,
	    sizeof(GtkDataEntry),
	    (GInstanceInitFunc)gtk_data_entry_init,
	    0);

	g_type_add_interface_static(data_entry_type,
	    GTK_TYPE_BUILDABLE,
	    &interface_info);
    }
    return (data_entry_type);
}


static void
gtk_data_entry_set_property(GObject *object,
    guint prop_id,
    const GValue *value,
    GParamSpec *pspec)
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY(object);

    switch(prop_id)
    {
	case PROP_DATA_ENTRY_DATATYPE:
	    {
		const gchar *data_type = g_value_get_string(value);

		if (!gtk_widget_get_realized(GTK_WIDGET(data_entry)))
		{
		    if (data_entry->data_type)
			g_free(data_entry->data_type);
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
		    if (data_entry->data_format)
			g_free(data_entry->data_format);
		    data_entry->data_format = g_strdup(data_format);
		}
		else
		{
		    gtk_data_entry_set_data_format(data_entry, data_format);
		}
	    }
	    break;

	case PROP_DATA_ENTRY_DESCRIPTION:
	    {
		const gchar *description = g_value_get_string(value);

		if (!gtk_widget_get_realized(GTK_WIDGET(data_entry)))
		{
		    if (data_entry->description)
			g_free(data_entry->description);
		    data_entry->description = g_strdup(description);
		}
		else
		{
		    gtk_data_entry_set_description(data_entry, description);
		}
	    }
	    break;

	case PROP_DATA_ENTRY_TEXT:
	    gtk_data_entry_set_text(data_entry, g_value_get_string(value));
	    break;

	case PROP_DATA_ENTRY_MAX_LENGTH_BYTES:
	    gtk_data_entry_set_max_length_bytes(data_entry, g_value_get_int(value));
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	    break;
    }
}

static void
gtk_data_entry_get_property(GObject *object,
    guint prop_id,
    GValue *value,
    GParamSpec *pspec)
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY(object);

    switch(prop_id)
    {
	case PROP_DATA_ENTRY_DATATYPE:
	    g_value_set_string(value, data_entry->data_type);
	    break;

	case PROP_DATA_ENTRY_DATAFORMAT:
	    g_value_set_string(value, data_entry->data_format);
	    break;

	case PROP_DATA_ENTRY_DESCRIPTION:
	    g_value_set_string(value, data_entry->description);
	    break;

	case PROP_DATA_ENTRY_TEXT:
	    g_value_set_string(value, gtk_data_entry_get_text(data_entry));
	    break;

	case PROP_DATA_ENTRY_MAX_LENGTH_BYTES:
	    g_value_set_int(value, gtk_data_entry_get_max_length_bytes(data_entry));
	    break;

	default:
	    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	    break;
    }
}

static gint
gtk_data_entry_focus_in(GtkWidget *widget, GdkEventFocus *event)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));
    gchar *data_format = GTK_DATA_ENTRY(widget)->data_format;

#if GTK_DATA_ENTRY_DEBUG
    g_debug("gtk_data_entry_focus_in: called");
#endif
    if (data_format && data_format[0])
    {
	text = gtk_data_format_remove(text, data_format);
	gtk_entry_set_text(GTK_ENTRY(widget), text);
    }

    return ((*GTK_WIDGET_CLASS(parent_class)->focus_in_event)(widget, event));
}

static gint
gtk_data_entry_focus_out(GtkWidget *widget, GdkEventFocus *event)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));
    gchar *data_format = GTK_DATA_ENTRY(widget)->data_format;

#if GTK_DATA_ENTRY_DEBUG
    g_debug("gtk_data_entry_focus_out: called");
#endif

    if (data_format && data_format[0])
    {
	text = gtk_data_format(text, data_format);
	gtk_entry_set_text(GTK_ENTRY(widget), text);
    }

    return ((*GTK_WIDGET_CLASS(parent_class)->focus_out_event)(widget, event));
}

static void
gtk_data_entry_class_init(GtkDataEntryClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
#if 0
    GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass);
    GtkEntryClass *entry_class = GTK_ENTRY_CLASS (klass);
#endif

    parent_class = g_type_class_ref(gtk_entry_get_type());

    gobject_class->set_property = gtk_data_entry_set_property;
    gobject_class->get_property = gtk_data_entry_get_property;

    widget_class->focus_in_event = gtk_data_entry_focus_in;
    widget_class->focus_out_event = gtk_data_entry_focus_out;

    /**
     * GtkDataEntry:datatype:
     *
     * no functionality, a datatype hint for the application because 
     * any widget content is text. 
     */
    g_object_class_install_property(gobject_class,
	PROP_DATA_ENTRY_DATATYPE,
	g_param_spec_string("datatype",
	    "Datatype",
	    "Data type for application use",
	    "" /* default value */,
	    G_PARAM_READWRITE));

    /**
     * GtkDataEntry:dataformat:
     *
     * a formatting string that controls what you see when the 
     * widget doesn't contain input focus. 
     *
     * See gtk_data_format() for details. 
     *
     */
    g_object_class_install_property(gobject_class,
	PROP_DATA_ENTRY_DATAFORMAT,
	g_param_spec_string("dataformat",
	    "Data format",
	    "A formatting string that controls what you see when the widget doesn't contain input focus",
	    "" /* default value */,
	    G_PARAM_READWRITE));

    /**
     * GtkDataEntry:description:
     *
     * Description of the GtkDataEntry, no functionality, a place 
     * for private information that cannot be put anywhere else.
     */
    g_object_class_install_property(gobject_class,
	PROP_DATA_ENTRY_DESCRIPTION,
	g_param_spec_string("description",
	    "Description",
	    "Description of entry contents",
	    "" /* default value */,
	    G_PARAM_READWRITE));

    /**
     * GtkDataEntry:text:
     *
     * Set the contents of the GtkDataEntry. For details see 
     * #gtk_data_entry_set_text. 
     */
    g_object_class_install_property(gobject_class,
	PROP_DATA_ENTRY_TEXT,
	g_param_spec_string("text",
	    "Text",
	    "The contents of the data_entry",
	    "" /* default value */,
	    G_PARAM_READWRITE));

    /**
     * GtkDataEntry:max-length-bytes:
     *
     * Set the maximum length in bytes for the GtkDataEntry. For 
     * details see #gtk_data_entry_set_max_length_bytes. 
     *
     * Sometimes, systems cannot handle UTF-8 string length
     * correctly, to overcome this problem, you can use the maximum 
     * string length in bytes. When setting both limits, max-length 
     *  and max-length-bytes, both must be fulfilled.
     *  
     * Since: 3.0.6 
     */
    g_object_class_install_property(gobject_class,
	PROP_DATA_ENTRY_MAX_LENGTH_BYTES,
	g_param_spec_int("max-length-bytes",
	    "Maximum bytes length",
	    "The maximum number of bytes for this entry. Zero if no maximum",

	    0, GTK_ENTRY_BUFFER_MAX_SIZE,
	    0 /* default value */,
	    G_PARAM_READWRITE));
}

/* Signal interception */

static void _gtk_data_entry_insert_text_handler(GtkEditable *editable,
    gchar *new_text, gint  new_text_length,
    gpointer position, gpointer user_data)   
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY(editable);
    gint max_len = data_entry->max_length_bytes;

    if (!max_len) return;

    const gchar *old_text = gtk_data_entry_get_text(data_entry);
    gint old_length = strlen(old_text);

    if (new_text_length < 0) new_text_length = strlen(new_text);

#if GTK_DATA_ENTRY_DEBUG_SIGNAL > 0
    g_debug("_gtk_data_entry_insert_text_handler(bytes): cl %d max %d new %d", 
	old_length, max_len, new_text_length);
#endif
	
    if (old_length + new_text_length > max_len)
    {
	gdk_beep();
	g_signal_stop_emission_by_name(editable, "insert-text");
    }
}


static void
gtk_data_entry_init(GtkDataEntry *data_entry)
{
#if 0
    GtkWidget *widget = GTK_WIDGET(data_entry);
#endif

    data_entry->description = NULL;
    data_entry->data_type = NULL;
    data_entry->data_format = NULL;
    data_entry->max_length_bytes = 0;

#if GTK_DATA_ENTRY_DEBUG > 0
    g_debug("gtk_data_entry_init");
#endif

    g_signal_connect(GTK_BUILDABLE(data_entry), "insert-text",
	G_CALLBACK(_gtk_data_entry_insert_text_handler), NULL);
}

/**
 * gtk_data_entry_new: 
 *  
 * Creates a new GtkDataEntry Widget. 
 *  
 * Returns: the new GtkDataEntry Widget 
 */
GtkDataEntry *
gtk_data_entry_new(void)
{
    GtkDataEntry *data_entry = GTK_DATA_ENTRY(
	gtk_widget_new(gtk_data_entry_get_type(), NULL));

    return (data_entry);
}


