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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <locale.h>
#include <gtk/gtk.h>

#define __GTKEXTRA_H_INSIDE__

#include "gtkextra-compat.h"
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
#define MAX_NUM_STRLEN  64
#define NULL_TEXT_REP   ""
#define INVALID_DATA   "?"

#define SIGNIFICANT_DIGITS  16

static gchar *insert_thousands_seps(const gchar *cp)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *radix_cp, c;
    gint pos;
    struct lconv *lc = localeconv();
    gchar *radix_c = (lc && lc->decimal_point) ? 
	lc->decimal_point : DEFAULT_DECIMAL_POINT;
    gchar *thousands_c = (lc && lc->thousands_sep) ? 
	lc->thousands_sep : DEFAULT_THOUSANDS_SEP;
    gint thousands_len = strlen(thousands_c);

    radix_cp = strstr(cp, radix_c);
    if (radix_cp)
        pos = radix_cp - cp;
    else
        pos = strlen(cp);
    
    for(radix_cp=buf;;)  /* copy inserting thousands_c on the fly */
    {
        if (!*cp) break;

        c = *radix_cp++ = *cp++;
        --pos;
        if ((pos > 0) && !(pos % 3)
        && (c != '-') && (c != '+'))
        {
            strcpy(radix_cp, thousands_c);
            radix_cp += thousands_len;
        }
    }
    *radix_cp++ = '\0';
    return(buf);
}

static gchar *remove_thousands_seps(const gchar *src)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *dst = buf;
    gboolean found=FALSE;
    gint i=0, l = strlen(src);
    struct lconv *lc = localeconv();
    gchar *thousands_c = (lc && lc->thousands_sep) ? 
	lc->thousands_sep : DEFAULT_THOUSANDS_SEP;
    gint thousands_len = strlen(thousands_c);

    if (!src) return((gchar *) src);
    
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
        if ((src[i] == thousands_c[0]) && (strncmp(&src[i], thousands_c, thousands_len) == 0))
        {
            i += thousands_len;
            found=TRUE;
        }
        else
            *dst++ = src[i++];  /* beware: minor risc to hit a UTF-8 radix_c */
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

