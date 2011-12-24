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
#include <gtk/gtk.h>
#include "gtkextra-compat.h"
#include "gtkdataformat.h"

/**
 * SECTION: gtkdataformat
 * @short_description: a data formatting library
 *
 * the widget property 'dataformat' may contain formatting 
 * instructions for the field contents. Any unrecognized 
 * formatting instruction is silently skipped. 
 *  
 * the library can be easily extended by adding more 
 * instructions to the list above.
 * 
 */

#define NUM_SEPARATOR   '\''
#define MAX_NUM_STRLEN  64
#define NULL_TEXT_REP   ""
#define INVALID_DATA   "?"

#define SIGNIFICANT_DIGITS  16

#if 0

#define IS_NUM_CHAR(c) \
        (  (('0' <= (c)) && ((c) <= '9')) \
        || ((c) == '.') \
        || ((c) == '+') || ((c) == '-') \
        || ((c) == 'e') || ((c) == 'E') )
        
static BOOL is_numeric(char *str)
{
    for(;*str;str++)
    {
        if (!IS_NUM_CHAR(*str)) return(FALSE);
    }
    return(TRUE);
}
#endif
             
static gchar *replace_dot_with_comma(gchar *cp)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *dst;
        
    for(dst=buf;*cp; cp++)
    {
        switch(*cp)
        {
            case '.': *dst++ = ','; break;
            default: *dst++ = *cp; break;
        }
    }   
    *dst = '\0';
    return(buf);
}

static gchar *insert_num_seps(gchar *cp)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *dot, c;
    gint pos;
        
    dot = strchr(cp, '.');
    if (dot)
        pos = dot - cp;
    else
        pos = strlen(cp);
    
    for(dot=buf;;)
    {
        if (!*cp) break;
        c = *dot++ = *cp++;
        --pos;
        if ((pos > 0) && !(pos % 3)
        && (c != '-') && (c != '+'))
            *dot++ = NUM_SEPARATOR;
    }
    *dot++ = '\0';
    return(buf);
}

#if 0
static char *remove_num_seps(gchar *src)
{
    static gchar buf[MAX_NUM_STRLEN];
    gchar *dst = buf;
    gboolean found=FALSE;
    gint i=0,l = strlen(src);
    
#if 1
    if ((l > 1) && (src[l-1] == '-'))    /* trailing minus sign */
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
#endif

    for(;i<l;i++)
    {
        if (src[i] == NUM_SEPARATOR)
            found=TRUE;
        else
            *dst++ = src[i];
    }
    *dst = '\0';
    
    if (found) return(buf);
    return(NULL);
}
#endif

static gchar *format_double(gdouble d, 
    gint comma_digits, gboolean do_numseps, gboolean do_comma)
{
    static gchar str_buf[MAX_NUM_STRLEN], *cp;

    if (comma_digits >= 0)
        sprintf(str_buf, "%.*f", comma_digits, d);
    else
        sprintf(str_buf, "%.*g", SIGNIFICANT_DIGITS, d);
    
    cp = str_buf; 

    if (do_numseps) cp = insert_num_seps(str_buf);  
    if (do_comma) cp = replace_dot_with_comma(cp);

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
 * 
 * @str:        the string to be formatted
 * @dataformat: formatting instructions
 * 
 * format string data according to dataformat
 *  
 * empty string -  no formatting
 *  
 * int8 - singed 8-bit integer value, optional '-' sign 
 * int16 signed 16-bit integer, optional '-' sign 
 * int32 - signed 32-bit integer, optional '-' sign
 *  
 * money - double value, 2 decimal digits, 1000-separators 
 * float,N - double value,  N decimal digits, 1000-separators 
 * bit - boolean value [0,1] 
 *  
 * 
 * Returns: a pointer to an internal static buffer, with the 
 * formatted data 
 */
gchar *gtk_data_format(gchar *str, gchar *dataformat)
{
    if (!str || !dataformat || !dataformat[0]) return(str);

    switch (dataformat[0])
    {
        case 'i':
            if (strcmp(dataformat, "int8") == 0)
            {
                gint i;
                if (sscanf(str, "%d", &i) == 1) return(format_int(i, 1));
                return(INVALID_DATA);
            }
            else if (strcmp(dataformat, "int16") == 0)
            {
                gint i;
                if (sscanf(str, "%d", &i) == 1) return(format_int(i, 2));
                return(INVALID_DATA);
            }
            else if (strcmp(dataformat, "int32") == 0)
            {
                gint i;
                if (sscanf(str, "%d", &i) == 1) return(format_int(i, 4));
                return(INVALID_DATA);
            }
            break;

        case 'm':
            if (strcmp(dataformat, "money") == 0)
            {
                gdouble d;

                if (sscanf(str, "%lg", &d) == 1) 
                    return(format_double(d, 2, TRUE, FALSE));

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

                    if (sscanf(str, "%lg", &d) == 1) 
                        return(format_double(d, precision, TRUE, FALSE));

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
    return(str);
}
