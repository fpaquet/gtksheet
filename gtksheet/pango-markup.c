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
 */

/**
 * SECTION: pango-markup
 * @short_description: Pango markup serializer for
 *                   #GtkTextBuffer
 *  
 * This module provides a serializer for #GtkTextBuffer to 
 * generate pango markup language.
 */

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <glib.h>
#include <config.h>
#include <gtk/gtk.h>

typedef struct {
    GtkTextBuffer *textbuffer;
    const GtkTextIter *start, *end;
    GString *output;
    GHashTable *tag_codes;
} WriterContext;

/* Initialize the writer context */
static WriterContext *writer_context_new(void)
{
    WriterContext *ctx = g_slice_new0(WriterContext);

    ctx->output = g_string_new("");
    ctx->tag_codes = g_hash_table_new_full(
        g_direct_hash, g_direct_equal, NULL, g_free);

    return ctx;
}

/* Free the writer context */
static void writer_context_free(WriterContext *ctx)
{
    g_hash_table_unref(ctx->tag_codes);
    g_slice_free(WriterContext, ctx);
}

static gchar *
get_color_from_rgba_color(GdkRGBA *color)
{
    gchar *colorcode;

    if (color->alpha == 1.0)
    {
        colorcode = g_strdup_printf(
            "#%02x%02x%02x", 
            (guint)(255 * color->red), 
            (guint)(255 * color->green), 
            (guint)(255 * color->blue));
    }
    else
    {
        colorcode = g_strdup_printf(
            "#%02x%02x%02x%02x", 
            (guint)(255 * color->red), 
            (guint)(255 * color->green), 
            (guint)(255 * color->blue), 
            (guint)(255 * color->alpha));
    }

    return colorcode;
}

/* Generate pango markup for tag
   add it to the context's hashtable of tags */

static void 
convert_tag_to_code(GtkTextTag *tag, WriterContext *ctx)
{
    GString *code = g_string_new("");
	
    //const gchar *name;
    //g_object_get(tag, "name", &name, NULL);

    gboolean val;

	g_object_get(tag, "background-set", &val, NULL);
	if(val)
	{
        GdkRGBA *color_rgba;
        g_object_get(tag, "background-rgba", &color_rgba, NULL);

        gchar *cs = get_color_from_rgba_color(color_rgba);

        g_string_append_printf(code, " bgcolor=\"%s\"", cs);
        g_free(cs);
	}
	
    g_object_get(tag, "fallback-set", &val, NULL);
    if(val)
    {
        g_object_get(tag, "fallback", &val, NULL);
        if(val)
            g_string_append(code, " fallback=\"true\"");
        else
            g_string_append(code, " fallback=\"false\"");
    }
    
	g_object_get(tag, "family-set", &val, NULL);
    if(val)
	{
	    const gchar *family;
	    g_object_get(tag, "family", &family, NULL);
        g_string_append_printf(code, " font_family=\"%s\"", family);
    }
	
    g_object_get(tag, "font-features-set", &val, NULL);
    if(val)
    {
        const gchar *font_features;
        g_object_get(tag, "font-features", &font_features, NULL);
        g_string_append_printf(code, " font_features=\"%s\"", font_features);
    }

	g_object_get(tag, "foreground-set", &val, NULL);
	if(val)
	{
        GdkRGBA *color_rgba;
        g_object_get(tag, "foreground-rgba", &color_rgba, NULL);

	    gchar *cs = get_color_from_rgba_color(color_rgba);

	    g_string_append_printf(code, " fgcolor=\"%s\"", cs);
        g_free(cs);
	}
	
	g_object_get(tag, "language-set", &val, NULL);
	if(val)
	{
		gchar *isocode;
		g_object_get(tag, "language", &isocode, NULL);
		g_string_append_printf(code, " lang=\"%s\"", isocode);
	}
	
    g_object_get(tag, "letter-spacing-set", &val, NULL);
    if(val)
    {
        gint letter_spacing;
        g_object_get(tag, "letter-spacing", &letter_spacing, NULL);
        g_string_append_printf(
            code, " letter_spacing=\"%d\"", letter_spacing);
    }
    
    g_object_get(tag, "rise-set", &val, NULL);
    if(val)
    {
        gint rise;
        g_object_get(tag, "rise", &rise, NULL);
        g_string_append_printf(code, " rise=\"%d\"", rise);
    }
    
    g_object_get(tag, "scale-set", &val, NULL);
    if(val)
    {
        gdouble factor;
        g_object_get(tag, "scale", &factor, NULL);

        if (factor < (PANGO_SCALE_XX_SMALL+PANGO_SCALE_X_SMALL)/2.0)
            g_string_append(code, " font_size=\"xx-small\"");
        else if (factor < (PANGO_SCALE_X_SMALL+PANGO_SCALE_SMALL)/2.0)
            g_string_append(code, " font_size=\"x-small\"");
        else if (factor < (PANGO_SCALE_SMALL+PANGO_SCALE_MEDIUM)/2.0)
            g_string_append(code, " font_size=\"small\"");
        else if (factor < (PANGO_SCALE_MEDIUM+PANGO_SCALE_LARGE)/2.0)
            g_string_append(code, " font_size=\"medium\"");
        else if (factor < (PANGO_SCALE_LARGE+PANGO_SCALE_X_LARGE)/2.0)
            g_string_append(code, " font_size=\"large\"");
        else if (factor < (PANGO_SCALE_X_LARGE+PANGO_SCALE_XX_LARGE)/2.0)
            g_string_append(code, " font_size=\"x-large\"");
        else
            g_string_append(code, " font_size=\"xx-large\"");
    }
    
    g_object_get(tag, "size-set", &val, NULL);
    if(val)
    {
        gint size;
        g_object_get(tag, "size", &size, NULL);
        g_string_append_printf(code, " font_size=\"%d\"", size);
    }
    
    g_object_get(tag, "stretch-set", &val, NULL);
    if(val)
    {
        PangoStretch stretch;
        g_object_get(tag, "stretch", &stretch, NULL);

        switch(stretch)
        {
            case PANGO_STRETCH_ULTRA_CONDENSED:
                g_string_append(code, " font_stretch=\"ultracondensed\"");
                break;
            case PANGO_STRETCH_EXTRA_CONDENSED:
                g_string_append(code, " font_stretch=\"extracondensed\"");
                break;
            case PANGO_STRETCH_CONDENSED:
                g_string_append(code, " font_stretch=\"condensed\"");
                break;
            case PANGO_STRETCH_SEMI_CONDENSED:
                g_string_append(code, " font_stretch=\"semicondensed\"");
                break;
            case PANGO_STRETCH_NORMAL:
                g_string_append(code, " font_stretch=\"normal\"");
                break;
            case PANGO_STRETCH_SEMI_EXPANDED:
                g_string_append(code, " font_stretch=\"semiexpanded\"");
                break;
            case PANGO_STRETCH_EXPANDED:
                g_string_append(code, " font_stretch=\"expanded\"");
                break;
            case PANGO_STRETCH_EXTRA_EXPANDED:
                g_string_append(code, " font_stretch=\"extraexpanded\"");
                break;
            case PANGO_STRETCH_ULTRA_EXPANDED:
                g_string_append(code, " font_stretch=\"ultraexpanded\"");
                break;
        }
    }
    
    g_object_get(tag, "strikethrough-set", &val, NULL);
    if(val)
    {
        g_object_get(tag, "strikethrough", &val, NULL);
        if(val)
            g_string_append(code, " strikethrough=\"true\"");
        else
            g_string_append(code, " strikethrough=\"false\"");
    }
    
    g_object_get(tag, "strikethrough-rgba-set", &val, NULL);
    if(val)
    {
        GdkRGBA *color_rgba;
        g_object_get(tag, "strikethrough-rgba", &color_rgba, NULL);

	    gchar *cs = get_color_from_rgba_color(color_rgba);

	    g_string_append_printf(
            code, " strikethrough_color=\"%s\"", cs);
        g_free(cs);
    }
        
    g_object_get(tag, "style-set", &val, NULL);
    if(val)
    {
        PangoStyle style;
        g_object_get(tag, "style", &style, NULL);

        switch(style)
        {
            case PANGO_STYLE_NORMAL:
                g_string_append(code, " font_style=\"normal\"");
                break;

            case PANGO_STYLE_OBLIQUE:
                g_string_append(code, " font_style=\"oblique'");
                break;

            case PANGO_STYLE_ITALIC:
                g_string_append(code, " font_style=\"italic\"");
                break;
        }
    }
    
    g_object_get(tag, "underline-set", &val, NULL);
    if(val)
    {
        PangoUnderline underline;
        g_object_get(tag, "underline", &underline, NULL);

        switch(underline)
        {
            case PANGO_UNDERLINE_NONE:
                g_string_append(code, " underline=\"none\"");
                break;

            case PANGO_UNDERLINE_SINGLE:
                g_string_append(code, " underline=\"single\"");
                break;

            case PANGO_UNDERLINE_LOW:
                g_string_append(code, " underline=\"low\"");
                break;

            case PANGO_UNDERLINE_DOUBLE:
                g_string_append(code, " underline=\"double\"");
                break;

            case PANGO_UNDERLINE_ERROR:
                g_string_append(code, " underline=\"error\"");
                break;
        }
    }
    
    g_object_get(tag, "underline-rgba-set", &val, NULL);
    if(val)
    {
        GdkRGBA *color_rgba;
        g_object_get(tag, "underline-rgba", &color_rgba, NULL);

	    gchar *cs = get_color_from_rgba_color(color_rgba);

	    g_string_append_printf(
            code, " underline_color=\"%s\"", cs);
        g_free(cs);
    }
    
    g_object_get(tag, "variant-set", &val, NULL);
    if(val)
    {
        PangoVariant variant;
        g_object_get(tag, "variant", &variant, NULL);

        switch(variant)
        {
            case PANGO_VARIANT_NORMAL:
                g_string_append(code, " font_variant=\"normal\"");
                break;

            case PANGO_VARIANT_SMALL_CAPS:
                g_string_append(code, " font_variant=\"smallcaps\"");
                break;
        }
    }
    
    g_object_get(tag, "weight-set", &val, NULL);
    if(val)
    {
        gint weight;
        g_object_get(tag, "weight", &weight, NULL);

        switch(weight)
        {
            case PANGO_WEIGHT_THIN:
                break;
            case PANGO_WEIGHT_ULTRALIGHT:
                g_string_append(code, " font_weight=\"ultralight\"");
                break;
            case PANGO_WEIGHT_LIGHT:
                g_string_append(code, " font_weight=\"light\"");
                break;
            case PANGO_WEIGHT_NORMAL:
                g_string_append(code, " font_weight=\"normal\"");
                break;
            case PANGO_WEIGHT_BOLD:
                g_string_append(code, " font_weight=\"bold\"");
                break;
            case PANGO_WEIGHT_ULTRABOLD:
                g_string_append(code, " font_weight=\"ultrabold\"");
                break;
            case PANGO_WEIGHT_HEAVY:
                g_string_append(code, " font_weight=\"heavy\"");
                break;

            case PANGO_WEIGHT_SEMILIGHT:
            case PANGO_WEIGHT_BOOK:
            case PANGO_WEIGHT_MEDIUM:
            case PANGO_WEIGHT_SEMIBOLD:
            case PANGO_WEIGHT_ULTRAHEAVY:
            default:
                g_string_append_printf(code, " font_weight=\"%d\"", weight);
                break;
        }
    }
    
    g_hash_table_insert(
        ctx->tag_codes, tag, g_string_free(code, FALSE));
}

/* This function is run before processing the actual contents
   of the buffer. It generates pango markup for all of the tags
   in the buffer's tag table, and tells the context which
   portion of the text buffer to serialize. */

static void analyze_buffer(
    WriterContext *ctx, 
    GtkTextBuffer *textbuffer, 
    const GtkTextIter *start, 
    const GtkTextIter *end)
{
    GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table(
        textbuffer); 

    gtk_text_tag_table_foreach(
        tagtable, (GtkTextTagTableForeach)convert_tag_to_code, ctx);

    ctx->textbuffer = textbuffer;
    ctx->start = start;
    ctx->end = end;
}

/* Analyze a segment of text in which there are no tag flips,
   but possibly embedded pictures */

static void write_rtf_text(
    WriterContext *ctx, 
    const GtkTextIter *start, 
    const GtkTextIter *end)
{
    gchar *text = gtk_text_buffer_get_text(
        ctx->textbuffer, start, end, TRUE);

    g_string_append(ctx->output, g_markup_escape_text(text, -1));
    g_free(text);
}

/* write taglist, handle convenience tags */

static void write_taglist(
    WriterContext *ctx, GSList *taglist, gboolean start)
{
    if(!taglist) return;

    guint len = g_slist_length(taglist);
    if(len == 0) return;

    if(!start)
    {
        g_string_append(ctx->output, "</span>");
        return;
    }

    g_string_append(ctx->output, "<span");

    GSList *ptr;
    for(ptr = taglist; ptr; ptr = g_slist_next(ptr))
    {
        g_string_append(
            ctx->output, 
            g_hash_table_lookup(ctx->tag_codes, ptr->data));
    }

    g_string_append(ctx->output, ">");
}

static void
write_rtf_paragraphs(WriterContext *ctx)
{
    GtkTextIter start = *(ctx->start), end;

    while(gtk_text_iter_in_range(&start, ctx->start, ctx->end))
    {
        end = start;
        
        /* find next tag */
        gtk_text_iter_forward_to_tag_toggle(&end, NULL);

        GSList *tagstartlist = gtk_text_iter_get_toggled_tags(
            &start, TRUE);

        if(tagstartlist)
        {
            write_taglist(ctx, tagstartlist, TRUE);
            g_slist_free(tagstartlist);
        }
        
        /* Output the actual contents of this section */
        write_rtf_text(ctx, &start, &end);

        GSList *tagendlist = gtk_text_iter_get_toggled_tags(
            &end, FALSE);

        if(tagendlist)
        {
            write_taglist(ctx, tagendlist, FALSE);
            g_slist_free(tagendlist);
        }

        start = end;
    }
}

/* Write the RTF header and assorted front matter */
static gchar *
write_rtf(WriterContext *ctx)
{
#if 0
    /* Preliminary formatting */
    g_string_append_printf(
        ctx->output, "<LANG>%s</LANG>\n", 
        pango_language_to_string(pango_language_get_default()));
#endif
    /* Document body */
    write_rtf_paragraphs(ctx);

    return g_string_free(ctx->output, FALSE);
}

/* This function is called by gtk_text_buffer_serialize(). */

/**
 * serialize_pango_markup: 
 * @register_buffer: (nullable): the #GtkTextBuffer for which 
 *                  the format is registered
 * @content_buffer: the #GtkTextBuffer to serialize
 * @start: start of block of text to serialize
 * @end: end of block of test to serialize
 * @length: Return location for the length of the serialized
 *         data
 * @user_data: user data that was specified when registering 
 *            the format
 * 
 * This function serializes the portion of text between @start
 * and @end in pango markup format. 
 *  
 * It can be registered as #GtkTextBuffer serializer. 
 * 
 * Returns: (nullable): a newly-allocated array of guint8 which 
 * contains the serialized data, or NULL if an error occurred. 
 */
guint8 *
serialize_pango_markup(
    GtkTextBuffer *register_buffer,
    GtkTextBuffer *content_buffer,
    const GtkTextIter *start,
    const GtkTextIter *end,
    gsize *length,
    gpointer user_data)
{
    WriterContext *ctx = writer_context_new();
    
    analyze_buffer(ctx, content_buffer, start, end);

    gchar *contents = write_rtf(ctx);
    *length = strlen(contents);

	writer_context_free(ctx);

	return ((guint8 *)contents);
}
