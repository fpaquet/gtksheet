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

/* References
   https://en.wikipedia.org/wiki/Decimal_mark
   https://en.wikipedia.org/wiki/Indian_numbering_system
   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <gtk/gtk.h>

#define __GTKSHEET_H_INSIDE__

#include "gtksheet-compat.h"
#include "gtkdataformat.h"

#ifdef DEBUG
#  define GTK_DATA_FORMAT_DEBUG  0  /* define to activate debug output */
#endif

/**
 * SECTION: gtkdataformat
 * @short_description: a data formatting library
 *
 * the widget property 'dataformat' may contain formatting
 * instructions for the field contents. Any unrecognized
 * formatting instruction is silently skipped.
 *
 * The formatting process should always be reversible. Thus
 * formatting can be applied when input focus leaves a field and
 * removed again when the focus enters a field, without the need
 * of an additional content buffer.
 *
 * the library can be easily extended by adding more
 * instructions to the list above.
 *
 */

#define DEFAULT_DECIMAL_POINT   "."  /* default radix char */
#define DEFAULT_THOUSANDS_SEP   "'"  /* default thousands grouping char */
#define DEFAULT_GROUPING        "\3"    /* default grouping */

#define MAX_NUM_STRLEN  64
#define NULL_TEXT_REP   ""
#define INVALID_DATA   "?"

#define SIGNIFICANT_DIGITS  16

/* Cached locale data */
static gchar *radix_str = NULL;
static gchar *thousands_c = NULL;
static guchar *grouping = NULL;

static void _cache_localedata_utf8(gboolean recheck)
{
    if (radix_str && ! recheck) return;

    struct lconv *lc = localeconv();
    GError *err = NULL;

    gchar *r = (lc && lc->decimal_point) ?
	lc->decimal_point : DEFAULT_DECIMAL_POINT;

    if (radix_str) { g_free(radix_str); radix_str = NULL; }
    radix_str = g_locale_to_utf8(r, strlen(r), NULL, NULL, &err);

    if (!radix_str && err) {
        g_warning("_get_localedata_utf8: failed to convert decimal_point <%s> to UTF8", r);
        radix_str = g_strdup(r);
    }

    gchar *tc = (lc && lc->thousands_sep) ?
	lc->thousands_sep : DEFAULT_THOUSANDS_SEP;

    if (thousands_c) { g_free(thousands_c); thousands_c = NULL; }
    thousands_c = g_locale_to_utf8(tc, strlen(r), NULL, NULL, &err);

    if (!thousands_c && err) {
        g_warning("_get_localedata_utf8: failed to convert thousands_setp <%s> to UTF8", tc);
        thousands_c = g_strdup(tc);
    }

    guchar *gp = (guchar *) (
      (lc && lc->grouping && lc->grouping[0]) ? 
        lc->grouping : DEFAULT_GROUPING);

    if (grouping) { g_free(grouping); grouping = NULL; }
    grouping = (guchar *) g_strdup((gchar *) gp);

#if GTK_DATA_FORMAT_DEBUG>0
    g_debug("_cache_localedata_utf8: <%s> <%s>", radix_str, thousands_c);
#endif
}

static gchar *insert_thousands_seps(const gchar *cp)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *radix_cp, c, *dst;
    const gchar *src;
    gint pos;  /* position of radix_str */
    gint tpos;  /* position of next thousands_sep */

    _cache_localedata_utf8(FALSE);

    gint thousands_len = strlen(thousands_c);
    guchar *grp_ptr = grouping;
    gint grp_size = *grp_ptr++;
    gint len = strlen(cp);

    if (len == 0) return("");

    radix_cp = strstr(cp, radix_str);
    if (radix_cp)
        pos = (radix_cp - cp) - len;
    else
        pos = 0;

    tpos = grp_size;
    if (*grp_ptr) grp_size = *grp_ptr++;

#if GTK_DATA_FORMAT_DEBUG>0
    g_debug("its: start grp_size %d pos %d cp <%s>", 
            grp_size, pos, cp);
#endif

    /* reverse copy, inserting thousands_c on the fly */
    src = &cp[len];  /* also copy terminator '\0' */
    dst = &buf[MAX_NUM_STRLEN-1];  /* end of buffer */

    for (;(src >= cp) && (dst > buf);pos++) {
        c = *dst-- = *src--;

#if GTK_DATA_FORMAT_DEBUG>0
        g_debug("its: loop grp_size %d pos %d c %c", grp_size, pos, c);
        g_debug("its: dst = <%s>", dst+1);
#endif
        if ((pos > 0)   /* left of radix_str */
            && (pos == tpos)   /* position at grouping */
            && (src >= cp)  /* not at beginning of number */
            && (*src != '-') && (*src != '+')  /* skip sign */
            )
        {
            /* note: use unterminated copy */
            strncpy(dst - thousands_len + 1, thousands_c, thousands_len);
            dst -= thousands_len;

            tpos += grp_size;
            if (*grp_ptr) grp_size = *grp_ptr++;
        }
    }
#if GTK_DATA_FORMAT_DEBUG>0
    g_debug("its: result = <%s>", dst+1);
#endif
    return(dst+1);
}

static gchar *remove_thousands_seps(const gchar *src)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *dst = buf;
    gboolean found=FALSE;
    gint i=0, l = strlen(src);

    _cache_localedata_utf8(FALSE);

    gint thousands_len = strlen(thousands_c);

    if (!src) return((gchar *) src);
    if (l >= MAX_NUM_STRLEN) return((gchar *) src);

    if ((l > 1) && (src[l-1] == '-'))    /* handle trailing minus sign */
    {
        if (src[0] == '-')
        {
            ++i;
            --l;
        }
        else
        {
            *dst++ = '-';
            --l;
        }
        found=TRUE;
    }

    while (i<l)
    {
        if ((src[i] == thousands_c[0])
            && (strncmp(&src[i], thousands_c, thousands_len) == 0))
        {
            i += thousands_len;
            found=TRUE;
        }
        else
            *dst++ = src[i++];  /* beware: minor risc to hit a UTF-8 radix_str */
    }
    *dst = '\0';

    if (found) return(buf);
    return((gchar *) src);
}

static gchar *format_double(gdouble d,
    gint comma_digits, gboolean do_numseps)
{
    static gchar str_buf[MAX_NUM_STRLEN], *cp;

    if (comma_digits >= 0)
        sprintf(str_buf, "%.*f", comma_digits, d);
    else
        sprintf(str_buf, "%.*g", SIGNIFICANT_DIGITS, d);

    cp = str_buf;

    if (do_numseps) cp = insert_thousands_seps(str_buf);

    return(cp);
}

static gchar *format_int(gint i, gint num_bytes)
{
    static gchar str_buf[MAX_NUM_STRLEN];
    sprintf(str_buf, "%d", i);
    return(str_buf);
}


/**
 * gtk_data_format:
 * @str:        the string to be formatted
 * @dataformat: formatting instructions
 *
 * format @str according to @dataformat.
 *
 * formatting instructions:
 *
 * '' (the empty string) does no formatting at all.
 *
 * 'int8' is formatted as a singed 8-bit integer value with
 * optional '-' sign.
 *
 * 'int16' is formatted as a signed 16-bit integer with optional
 * '-' sign.
 *
 * 'int32' is formatted as a signed 32-bit integer with optional
 * '-' sign.
 *
 * 'money' is formatted as a double float value with 2 decimal
 * digits and 1000s-separators
 *
 * 'float,N' is formatted as a double float value with N decimal
 * digits and 1000s-separators
 *
 * 'bit' is formatted as a boolean value [0,1].
 *
 *
 * Returns: a pointer to an internal static buffer, with the
 * formatted data
 */
gchar *gtk_data_format(const gchar *str, const gchar *dataformat)
{
    if (!str || !str[0] || !dataformat || !dataformat[0]) return((gchar *) str);

    switch (dataformat[0])
    {
        case 'i':
            if (strcmp(dataformat, "int8") == 0)
            {
                gint i;
                str = remove_thousands_seps(str);
                if (sscanf(str, "%d", &i) == 1) return(format_int(i, 1));
                return(INVALID_DATA);
            }
            else if (strcmp(dataformat, "int16") == 0)
            {
                gint i;
                str = remove_thousands_seps(str);
                if (sscanf(str, "%d", &i) == 1) return(format_int(i, 2));
                return(INVALID_DATA);
            }
            else if (strcmp(dataformat, "int32") == 0)
            {
                gint i;
                str = remove_thousands_seps(str);
                if (sscanf(str, "%d", &i) == 1) return(format_int(i, 4));
                return(INVALID_DATA);
            }
            break;

        case 'm':
            if (strcmp(dataformat, "money") == 0)
            {
                gdouble d;

                str = remove_thousands_seps(str);

                if (sscanf(str, "%lg", &d) == 1)
                    return(format_double(d, 2, TRUE));

                return(INVALID_DATA);
            }
            break;

        case 'f':
            if (strncmp(dataformat, "float,", 6) == 0)
            {
                gint precision;

                if (sscanf(&dataformat[6], "%d", &precision) == 1)
                {
                    gdouble d;

                    str = remove_thousands_seps(str);

                    if (sscanf(str, "%lg", &d) == 1)
                        return(format_double(d, precision, TRUE));

                    return(INVALID_DATA);
                }
            }
            break;

        case 'b':
            if (strcmp(dataformat, "bit") == 0)
            {
                if (strcmp(str, "1") == 0) return(format_int(1, 1));
                else if (strcmp(str, "0") == 0) return(format_int(0, 1));
                else if (strcmp(str, "true") == 0) return(format_int(1, 1));
                else if (strcmp(str, "false") == 0) return(format_int(0, 1));
                return(INVALID_DATA);
            }
            break;

        default: break;
    }
    return((gchar *) str);
}

/**
 * gtk_data_format_remove:
 * @str:        the string to be unformatted
 * @dataformat: formatting instructions
 *
 * reverse the effect of #gtk_data_format, i.e. remove all
 * formatting characters, apply trailing dash
 *
 * Returns: a pointer to an internal static buffer, with the
 * unformatted data
 */
gchar *gtk_data_format_remove(const gchar *str, const gchar *dataformat)
{
    if (!str || !dataformat || !dataformat[0]) return((gchar *) str);

    switch (dataformat[0])
    {
        case 'i':
            if (strcmp(dataformat, "int8") == 0)
            {
                str = remove_thousands_seps(str);
            }
            else if (strcmp(dataformat, "int16") == 0)
            {
                str = remove_thousands_seps(str);
            }
            else if (strcmp(dataformat, "int32") == 0)
            {
                str = remove_thousands_seps(str);
            }
            break;

        case 'm':
            if (strcmp(dataformat, "money") == 0)
            {
                str = remove_thousands_seps(str);
            }
            break;

        case 'f':
            if (strncmp(dataformat, "float,", 6) == 0)
            {
                gint precision;

                if (sscanf(&dataformat[6], "%d", &precision) == 1)
                {
                    str = remove_thousands_seps(str);
                }
            }
            break;

        default: break;
    }
    return((gchar *) str);
}

