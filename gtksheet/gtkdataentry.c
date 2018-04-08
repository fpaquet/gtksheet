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

#define __GTKSHEET_H_INSIDE__

#include "gtksheet-compat.h"
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
#define GTK_DATA_ENTRY_DEBUG_VLIST  0  /* debug field validation */
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

#define GTK_DATA_ENTRY_VLIST_IGNORE "Ignore("
#define GTK_DATA_ENTRY_VLIST_ACCEPT "Accept("
#define GTK_DATA_ENTRY_VLIST_REJECT "Reject("
#define GTK_DATA_ENTRY_VLIST_MAP "Map("
#define GTK_DATA_ENTRY_VLIST_SEP "|"
#define GTK_DATA_ENTRY_VLIST_END ")"

/**
 * dealloc_string_ptr
 * 
 * free allocated memory and set pointer to NULL
 * 
 * @p:      address of pointer
 */
static void dealloc_generic_ptr(void **p)
{
    if (*p) 
    {
        g_free(*p);
        *p = NULL;
    }
}

/**
 * utf8_to_unichar - convert utf8 string to unichar array
 * 
 * Convert a string from UTF-8 to a unichar fixed width representation. 
 * A trailing 0 character will be added to the string after the converted text.
 * 
 * similar to g_utf8_to_ucs4()
 * 
 * @str:        a UTF-8 encoded string
 * @len:        the maximum length of str to use, in bytes. If 
 *             len < 0, then the string is nul-terminated.
 * @items_written:
 *                   location to store number of characters written or NULL.
 *                   The value here stored does not include the
 *                   trailing 0 character.
 * 
 * Returns a pointer to a newly allocated unicode string. This 
 * value must be freed with g_free(). If an error occurs, NULL 
 * will be returned. 
 */
static gunichar *utf8_to_unichar (
    const gchar *str,
    glong len,
    glong *items_written)
{
    if (len < 0 ) len = strlen(str);
    const gchar *endp = str + len;

    if (len <= 0) return NULL;
    
    gint nc = g_utf8_strlen(str, -1);
    gunichar *result = (gunichar *) g_malloc0_n(nc+1, sizeof(gunichar));
    gunichar *dst = result;

    while (str < endp)
    {
        gunichar ch = g_utf8_get_char(str);
        *dst++ = ch;
        str = g_utf8_next_char(str);
    }

    if (items_written)
        *items_written = (dst - result);

    *dst = (gunichar) 0; /* terminator */
    return result;
}

/**
 * Extract character list from string. 
 *  
 * the search starts at str and ends at the first occurance 
 * of enstr. 
 * 
 * @str: starting point of search for enstr
 * @endpp: pointer to store end of haystack or NULL
 * @pat: definition keyword to search for, including leading 
 *         quote
 * @endstr: list termination string
 * 
 * Returns: pointer to validation list or NULL
 */
static gchar *get_vlist(
    const gchar *str, 
    const gchar **endpp, 
    gchar *pat, 
    gchar *endstr)
{
    gchar *p = g_strstr_len(str, -1, pat);
#if GTK_DATA_ENTRY_DEBUG_VLIST>0
    g_debug("get_vlist p %s", p);
#endif

    if (p)
    {
        p += strlen(pat);
        gchar *endp = g_strstr_len(p, -1, endstr);
        if (endpp) *endpp = endp;  /* save end of list pointer */
        
        if (!endp || endp <= p)  /* length > 0 */
        {
            return NULL;
        }

        GString *result = g_string_sized_new (endp - p + 1);

        while (p < endp)
        {
            if (*p == '\\')
            {
                switch(*++p)
                {
                    case 'a': g_string_append_c (result, '\a'); break;
                    case 'b': g_string_append_c (result, '\b'); break;
                    case 'f': g_string_append_c (result, '\f'); break;
                    case 'n': g_string_append_c (result, '\n'); break;
                    case 'r': g_string_append_c (result, '\r'); break;
                    case 't': g_string_append_c (result, '\t'); break;
                    case 'v': g_string_append_c (result, '\v'); break;
                    case '\\': g_string_append_c (result, '\\'); break;
                    default:
                        g_string_append_c (result, '\\'); 
                        continue;
                }
                p++;
                continue;
            }

            gchar buf[7];
            gunichar ch = g_utf8_get_char (p);
            gint buflen = g_unichar_to_utf8 (ch, buf);
            g_string_append_len (result, buf, buflen);

            p = g_utf8_next_char (p);
        }
        g_string_append_c (result, '\0');

        return g_string_free (result, FALSE);
    }
    return NULL;
}

static void update_validation_lists(GtkDataEntry *data_entry)
{
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    dealloc_generic_ptr((void *) &data_entry->vlist_ignore);
    dealloc_generic_ptr((void *) &data_entry->vlist_accept);
    dealloc_generic_ptr((void *) &data_entry->vlist_reject);
    dealloc_generic_ptr((void *) &data_entry->map_from);
    dealloc_generic_ptr((void *) &data_entry->map_to);

    gchar *desc = data_entry->description;

    if (desc)
    {
        data_entry->vlist_ignore = get_vlist(
            desc, NULL, 
            GTK_DATA_ENTRY_VLIST_IGNORE,
            GTK_DATA_ENTRY_VLIST_END);

        data_entry->vlist_accept = get_vlist(
            desc, NULL, 
            GTK_DATA_ENTRY_VLIST_ACCEPT,
            GTK_DATA_ENTRY_VLIST_END);

        data_entry->vlist_reject = get_vlist(
            desc, NULL, 
            GTK_DATA_ENTRY_VLIST_REJECT,
            GTK_DATA_ENTRY_VLIST_END);

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
        g_debug("Got Ignore list <%s>", data_entry->vlist_ignore);
        g_debug("Got Accept list <%s>", data_entry->vlist_accept);
        g_debug("Got Reject list <%s>", data_entry->vlist_reject);
#endif

        const gchar *map_from_endp = NULL;
        data_entry->map_from = get_vlist(
            desc, &map_from_endp,
            GTK_DATA_ENTRY_VLIST_MAP,
            GTK_DATA_ENTRY_VLIST_SEP);

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
        g_debug("Got Map From <%s>", data_entry->map_from);
#endif

        if (data_entry->map_from)
        {
            const gchar *map_to_endp = NULL;
            gchar *map_to = get_vlist(
                map_from_endp, &map_to_endp,
                GTK_DATA_ENTRY_VLIST_SEP,
                GTK_DATA_ENTRY_VLIST_END);

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
            g_debug("Got Map From <%s>", data_entry->map_from);
#endif

            if (map_to)
            {
                long och = 0;
                data_entry->map_to = utf8_to_unichar(
                    map_to, -1, &och);

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
                g_debug("Got Map To <%s> %ld unichars", map_to, och);
#endif
                g_free(map_to);
            }
            else  /* junk map_from */
            {
                dealloc_generic_ptr((void *) &data_entry->map_from);
#if GTK_DATA_ENTRY_DEBUG_VLIST>0
                g_debug("No Map To - Map From junked");
#endif
            }
        }
    }
}

/**
 * gtk_data_entry_set_description: 
 * @data_entry:  a #GtkDataEntry
 * @description:  the description or NULL 
 *  
 * Sets the GtkDataEntry description. 
 *  
 * Basically, you can use the description for your own 
 * programming purpose. The #GtkDataEntry recognizes the 
 * keywords below, which have a special meaning. 
 *  
 * - Ignore(vString) - silently ignore all characters in vString 
 *  
 * - Map(vString1|vString) - map each character in vString1 to 
 *   the corresponding character in vString2 with the same index
 *  
 * - Accept(vString) - accept only characters in vString, sound
 *   bell for invalid characters
 *  
 * - Reject(vString) - ignore all characters in vString, sound 
 *   bell for invalid characters
 *  
 * Evaluation order: Ignore, Map, not Accept or Reject 
 * 
 * vString's are a literate character lists. They may contain
 * valid utf-8 character sequences and the following
 * escape sequences: \a, \b, \f, \n, \r, \t, \v, \\. vStrings
 * are not quoted like a normal C string, they are delimited by 
 * round brackets or the pipe symbol, see above. 
 */
void gtk_data_entry_set_description(GtkDataEntry *data_entry,
    const gchar *description)
{
    g_return_if_fail(data_entry != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(data_entry));

    if (data_entry->description)
	g_free(data_entry->description);

    data_entry->description = g_strdup(description);
    update_validation_lists(data_entry);
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
                    update_validation_lists(data_entry);
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

 /*
 * gtk_data_entry_finalize_handler:
 * 
 * this is the #GtkDataEntry object class "finalize" signal handler
 * 
 * @param object the #GtkDataEntry
 */
static void
gtk_data_entry_finalize_handler(GObject *object)
{
    g_return_if_fail(object != NULL);
    g_return_if_fail(GTK_IS_DATA_ENTRY(object));

    GtkDataEntry *data_entry = GTK_DATA_ENTRY(object);

    dealloc_generic_ptr((void *) &(data_entry->data_type));
    dealloc_generic_ptr((void *) &(data_entry->data_format));
    dealloc_generic_ptr((void *) &(data_entry->description));

    dealloc_generic_ptr((void *) &(data_entry->vlist_ignore));
    dealloc_generic_ptr((void *) &(data_entry->vlist_accept));
    dealloc_generic_ptr((void *) &data_entry->vlist_reject);
    dealloc_generic_ptr((void *) &data_entry->map_from);
    dealloc_generic_ptr((void *) &data_entry->map_to);

    /* not sure if this is needed 22.06.17/fp
    data_entry_parent_class = g_type_class_peek_parent(klass);

    if (G_OBJECT_CLASS(data_entry_parent_class)->finalize)
        (*G_OBJECT_CLASS(data_entry_parent_class)->finalize)(object); 
    */ 
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

    gobject_class->finalize = gtk_data_entry_finalize_handler;

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

/**
 * process_vlists
 * 
 * handle all vlists
 * 
 * vlist_ignore -> chars are silently ignored
 * vlist_accept -> chars are accepted, beep, precedence over vlist_reject
 * vlist_reject -> chars are rejected, beep 
 * vlist_map_from -> vlist_map_to 
 * 
 * @param data_entry the #GtkDataEntry
 * @param str        the text to be inserted
 * @param length     the length of the text in bytes, or -1
 * 
 * @return sanitized text or NULL
 */
static gchar *
process_vlists(GtkDataEntry *data_entry,
    const gchar *str, 
    gint length)
{
    if (length < 0) length = strlen(str);

    GString *result = g_string_sized_new(length + 1);
    const gchar *p = str;
    const gchar *endp = str + length;
    gboolean modified = FALSE;
    gboolean beep = FALSE;

    gchar *vlist_ignore = data_entry->vlist_ignore;
    gchar *vlist_accept = data_entry->vlist_accept;
    gchar *vlist_reject = data_entry->vlist_reject;
    gchar *map_from = data_entry->map_from;

    while (p < endp)
    {
        gunichar ch = g_utf8_get_char(p);
        gchar buf[7];

        if (vlist_ignore && g_utf8_strchr(vlist_ignore, -1, ch))
        {
            modified = TRUE;
            p = g_utf8_next_char(p);
            continue;
        }

        if (map_from)
        {
            gchar *pos = g_utf8_strchr(map_from, -1, ch);
            if (pos)  /* found mapping entry */
            {
                gint idx = g_utf8_strlen(map_from, pos - map_from);
#if GTK_DATA_ENTRY_DEBUG_VLIST>0
                g_debug("Map: found at idx %d", idx);
#endif
                gunichar nchar = data_entry->map_to[idx];
                if (nchar)  /* transliteration exists */
                {
                    ch = nchar;
                    if (ch) modified = TRUE;
                }
            }
        }

        if ( (vlist_accept && !g_utf8_strchr(vlist_accept, -1, ch))
            || (vlist_reject && g_utf8_strchr(vlist_reject, -1, ch)) )
        {
            modified = TRUE;
            beep = TRUE;
        }
        else
        {
            gint buflen = g_unichar_to_utf8(ch, buf);
            g_string_append_len(result, buf, buflen);
        }
        p = g_utf8_next_char(p);
    }
    g_string_append_c(result, '\0');

    if (modified)
    {
        if (beep) gdk_beep();
        return g_string_free(result, FALSE);
    }

    g_string_free(result, TRUE);
    return NULL;
}

/**
 * _gtk_data_entry_insert_text_handler
 * 
 * Details see #gtk_editable_insert_text
 * 
 * @editable:  a #GtkEditable
 * @new_text:  the text to append
 * @new_text_length: the length of the text in bytes, or -1 
 * @position:  location of the position text will be inserted 
 *          at.
 * @user_data: user data
 */
static void _gtk_data_entry_insert_text_handler(GtkEditable *editable,
    gchar *new_text, gint  new_text_length,
    gpointer position, gpointer user_data)   
{
    gchar *my_text = new_text;
    if (new_text_length < 0) new_text_length = strlen(my_text);

    GtkDataEntry *data_entry = GTK_DATA_ENTRY(editable);

    gchar *sanitized_str = process_vlists(
        data_entry, new_text, new_text_length);

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
    g_debug("IT nl %d T<%s> S<%s> ", 
        new_text_length, new_text, sanitized_str);
#endif

    if (sanitized_str)
    {
        my_text = sanitized_str;
        new_text_length = strlen(my_text);
    }

    /* check max_length_bytes */

    gint max_len_bytes = data_entry->max_length_bytes;

    gchar *bc_text = NULL;  /* text assembled with byte length check */

    if (max_len_bytes > 0) 
    {
        const gchar *old_text = gtk_data_entry_get_text(data_entry);
        gint old_length = strlen(old_text);

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
        g_debug("_gtk_data_entry_insert_text_handler: o %d m %d n %d", 
            old_length, max_len_bytes, new_text_length);
#endif
            
        if (old_length + new_text_length > max_len_bytes)
        {
            gdk_beep();

            /* assemble a fitting text fragment */
            GString *bc_wrk_str = g_string_sized_new(new_text_length + 1);
            const gchar *p = my_text;
            const gchar *endp = my_text + new_text_length;
            gint assembly_len = 0;

            while (p < endp)
            {
                gunichar ch = g_utf8_get_char(p);

                gchar buf[7];
                gint buflen = g_unichar_to_utf8(ch, buf);

                assembly_len += buflen;

                if (old_length + assembly_len > max_len_bytes) 
                    break;
                
                g_string_append_len(bc_wrk_str, buf, buflen);
                p = g_utf8_next_char(p);
            }
            g_string_append_c(bc_wrk_str, '\0');
            
            bc_text = g_string_free(bc_wrk_str, FALSE);
            my_text = bc_text;
            new_text_length = strlen(my_text);
        }
    }

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
    g_debug("Re-invoking insert handler");
#endif

    g_signal_handlers_block_by_func(G_OBJECT(editable),
        G_CALLBACK(_gtk_data_entry_insert_text_handler),
        user_data);
    gtk_editable_insert_text(editable, 
        my_text, new_text_length, 
        position);
    g_signal_handlers_unblock_by_func(G_OBJECT(editable),
        G_CALLBACK(_gtk_data_entry_insert_text_handler),
        user_data);
    g_signal_stop_emission_by_name(G_OBJECT(editable), "insert_text");

#if GTK_DATA_ENTRY_DEBUG_VLIST>0
    g_debug("Back from insert handler");
#endif

    /* cleanup */
    if (sanitized_str) g_free(sanitized_str);
    if (bc_text) g_free(bc_text);
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

    data_entry->vlist_ignore = NULL;
    data_entry->vlist_accept = NULL;
    data_entry->vlist_reject = NULL;
    data_entry->map_from = NULL;
    data_entry->map_to = NULL;

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


