/* GdkPixbuf library - Orientation functions.
 *
 * Copyright (C) 2001 Free Software Foundation, Inc.
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

/**
 * gdk_pixbuf_flip:
 *  + pixbuf: a #GdkPixbuf
 *  + horizontal: Whether to flip in horizontal direction.  If  + horizontal is
 * #TRUE, then flip around in horizontal direction.
 *  + vertical: Whether to flip in vertical direction.  If  + vertical is #TRUE,
 * then flip in vertical direction.  (NOT IMPLEMENTED YET!)
 *  + in_place: If in_place is #TRUE, the original  + pixbuf is modified.
 *
 * Flips  + pixbuf around horizontally and/or vertically.
 *
 * Return value: The original  + pixbuf is modified and returned, if
 * in_place is #TRUE.  If in_place is #FALSE, a newly created pixbuf
 * with a reference count of 1 is returned.
 **/
GdkPixbuf *
gdk_pixbuf_flip (GdkPixbuf *pixbuf,
		 gboolean horizontal,
		 gboolean vertical,
		 gboolean in_place)
{
	GdkPixbuf *dest;
	gint src_has_alpha;
	gint width, height, src_rs;
	gint dst_rs;

	guchar *src_pix, *src_p;
	guchar *dst_pix, *dst_p;

	gint i, j;
	gint alpha;

	g_warning ("%s is not finished.\n", __PRETTY_FUNCTION__);
	
	g_return_if_fail (pixbuf != NULL);

	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	src_has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

	alpha = src_has_alpha ? 4 : 3;
	
	src_rs = gdk_pixbuf_get_rowstride (pixbuf);
	src_pix = gdk_pixbuf_get_pixels (pixbuf);

	if (in_place==TRUE) {
		dest = pixbuf;
		dst_rs = gdk_pixbuf_get_rowstride (pixbuf);
		dst_pix = gdk_pixbuf_get_pixels (pixbuf);
	} else {
		dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, src_has_alpha, 8, width, height);
		dst_rs = gdk_pixbuf_get_rowstride (dest);
		dst_pix = gdk_pixbuf_get_pixels (dest);
	}

	if (horizontal==TRUE) {
		for (i = 0; i &lt; height; i) {
			src_p = src_pix + (src_rs * i);
			dst_p = dst_pix + (dst_rs * i);
			
			for (j = 0; j &lt; width; j) {
				*(dst_p) = *(src_p);
				*(dst_p) = *(src_p);
				*(dst_p) = *(src_p);
				if (src_has_alpha)
					*(dst_p) = *(src_p);
				dst_p = dst_p - (alpha + 3);
			}
		}
	}
	
	return dest;
}



/**
 * gdk_pixbuf_rotate:
 *  + pixbuf: a #GdkPixbuf
 *  + angle: angle modulo 90-degree (ie 90, 180, 270 degrees angle)
 *  + in_place: If in_place is #TRUE, the original data is modified
 * and the same value as  + pixbuf is returned.
 *
 * Rotates  + pixbuf by 90-degree multiples in the counter-clockwise
 * direction.
 *
 * Return value: The original  + pixbuf is modified and returned, if
 * in_place is #TRUE.
 * If in_place is false, we try to allocate a new #GdkPixbuf and return
 * NULL on failure; on success we write the flipped data there.
 **/

GdkPixbuf *
gdk_pixbuf_rotate (GdkPixbuf *pixbuf,
		   gint angle,
		   gboolean in_place)
{
	GdkPixbuf *dest;
	gint src_has_alpha;
	
	gint src_w, src_h, src_rs;
	gint dst_w, dst_h, dst_rs;

	guchar *src_pix, *src_p;
	guchar *dst_pix, *dst_p;

	gint i, j;
	gint alpha;

	g_warning ("%s is not finished.\n", __PRETTY_FUNCTION__);

	g_return_val_if_fail (pixbuf != NULL, NULL);
	g_return_val_if_fail (angle % 90 == 0, NULL);

	src_w = gdk_pixbuf_get_width (pixbuf);
	src_h = gdk_pixbuf_get_height (pixbuf);

	src_has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	src_rs = gdk_pixbuf_get_rowstride (pixbuf);
	src_pix = gdk_pixbuf_get_pixels (pixbuf);

	alpha = (src_has_alpha ? 4 : 3);

	/* NOT FINISHED! */
				
	if (in_place==TRUE) {
		dest = pixbuf;
		dst_rs = gdk_pixbuf_get_rowstride (pixbuf);
		dst_pix = gdk_pixbuf_get_pixels (pixbuf);
	} else {
		dest = gdk_pixbuf_new (GDK_COLORSPACE_RGB, src_has_alpha, 8, dst_w, dst_h);
		dst_rs = gdk_pixbuf_get_rowstride (dest);
		dst_pix = gdk_pixbuf_get_pixels (dest);
	}
	return dest;
}





diff --exclude=CVS -ruN gdk-pixbuf/gdk-pixbuf/gdk-pixbuf-orient.h gdk-pixbuf.oka/gdk-pixbuf/gdk-pixbuf-orient.h
--- gdk-pixbuf/gdk-pixbuf/gdk-pixbuf-orient.h	Thu Jan  1 01:00:00 1970
 gdk-pixbuf.oka/gdk-pixbuf/gdk-pixbuf-orient.h	Wed May  2 14:43:53 2001
 +  +  -0,0 1,37  +  + 
/* GdkPixbuf library - Orientation functions.
 *
 * Copyright (C) 2001 Free Software Foundation, Inc.
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

#ifndef GDK_PIXBUF_ORIENT_H
#define GDK_PIXBUF_ORIENT_H

#include "gdk-pixbuf.h"

GdkPixbuf *gdk_pixbuf_flip (GdkPixbuf *pixbuf,
			    gboolean horizontal,
			    gboolean vertical,
			    gboolean in_place);

GdkPixbuf *gdk_pixbuf_rotate (GdkPixbuf *pixbuf,
			      int angle,
			      gboolean in_place);

#endif /* GDK_PIXBUF_ORIENT_H */


diff --exclude=CVS -ruN gdk-pixbuf/gdk-pixbuf/testpixbuf-orient.c gdk-pixbuf.oka/gdk-pixbuf/testpixbuf-orient.c
--- gdk-pixbuf/gdk-pixbuf/testpixbuf-orient.c	Thu Jan  1 01:00:00 1970
 gdk-pixbuf.oka/gdk-pixbuf/testpixbuf-orient.c	Wed May  2 14:53:34 2001
 +  +  -0,0 1,165  +  + 
/* testpix-orient.c -- Testing orientation functions
 *
 * Copyright (C) 2001 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	
#include &lt;stdio.h&gt;
#include &lt;png.h&gt;

#include "gdk-pixbuf.h"
#include "gdk-pixbuf-orient.h"

#define FLIPH TRUE
#define FLIPV TRUE

gboolean
save_pixbuf_to_file_as_png (GdkPixbuf *pixbuf, char *filename)
{
	FILE *handle;
  	char *buffer;
	gboolean has_alpha;
	int width, height, depth, rowstride;
  	guchar *pixels;
  	png_structp png_ptr;
  	png_infop info_ptr;
  	png_text text[2];
  	int i;

	g_return_val_if_fail (pixbuf != NULL, FALSE);
	g_return_val_if_fail (filename != NULL, FALSE);
	g_return_val_if_fail (filename[0] != '\0', FALSE);

        handle = fopen (filename, "wb");
        if (handle == NULL)
        	return FALSE;

	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose (handle);
		return FALSE;
	}

	info_ptr = png_create_info_struct (png_ptr);
	if (info_ptr == NULL) {
		png_destroy_write_struct (&amp;png_ptr, (png_infopp)NULL);
		fclose (handle);
	    	return FALSE;
	}

	if (setjmp (png_ptr-&gt;jmpbuf)) {
		png_destroy_write_struct (&amp;png_ptr, &amp;info_ptr);
		fclose (handle);
		return FALSE;
	}

	png_init_io (png_ptr, handle);

        has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);
	width = gdk_pixbuf_get_width (pixbuf);
	height = gdk_pixbuf_get_height (pixbuf);
	depth = gdk_pixbuf_get_bits_per_sample (pixbuf);
	pixels = gdk_pixbuf_get_pixels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);

	png_set_IHDR (png_ptr, info_ptr, width, height,
		      depth, PNG_COLOR_TYPE_RGB_ALPHA,
		      PNG_INTERLACE_NONE,
		      PNG_COMPRESSION_TYPE_DEFAULT,
		      PNG_FILTER_TYPE_DEFAULT);

	/* Some text to go with the png image */
	text[0].key = "Title";
	text[0].text = filename;
	text[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text[1].key = "Software";
	text[1].text = "testpixbuf-orient (gdk-pixbuf)";
	text[1].compression = PNG_TEXT_COMPRESSION_NONE;
	png_set_text (png_ptr, info_ptr, text, 2);

	/* Write header data */
	png_write_info (png_ptr, info_ptr);

	/* if there is no alpha in the data, allocate buffer to expand into */
	if (has_alpha)
		buffer = NULL;
	else
		buffer = g_malloc(4 * width);
	
	/* pump the raster data into libpng, one scan line at a time */	
	for (i = 0; i &lt; height; i) {
		if (has_alpha) {
			png_bytep row_pointer = pixels;
			png_write_row (png_ptr, row_pointer);
		} else {
			/* expand RGB to RGBA using an opaque alpha value */
			int x;
			char *buffer_ptr = buffer;
			char *source_ptr = pixels;
			for (x = 0; x &lt; width; x) {
				*buffer_ptr = *source_ptr;
				*buffer_ptr = *source_ptr;
				*buffer_ptr = *source_ptr;
				*buffer_ptr = 255;
			}
			png_write_row (png_ptr, (png_bytep) buffer);
		}
		pixels = rowstride;
	}
	
	png_write_end (png_ptr, info_ptr);
	png_destroy_write_struct (&amp;png_ptr, &amp;info_ptr);
	
	g_free (buffer);
		
	fclose (handle);
	return TRUE;
}



int
main(int argc, char **argv)
{
	GdkPixbuf *pixbuf, *flippedout_h, *flippedout_v, *flippedout_hv;

	int angle;

	gtk_init (&amp;argc, &amp;argv);
	gdk_rgb_init ();

	if (argc != 2) {
		fprintf (stderr, "Usage: testpixbuf-orient FILE\n");
		exit (1);
	}

	pixbuf = gdk_pixbuf_new_from_file (argv[1]);
	
	if (!pixbuf) {
		fprintf (stderr, "Cannot load %s\n", argv[1]);
		exit(1);
	}

	flippedout_h = gdk_pixbuf_flip(pixbuf, TRUE, FALSE, FALSE);
/*  	flippedout_v = gdk_pixbuf_flip(pixbuf, FALSE, TRUE, FALSE); */

	save_pixbuf_to_file_as_png (pixbuf,"testorient-unflipped-original.png");
	save_pixbuf_to_file_as_png (flippedout_h,"testorient-flipped-horizontal.png");
/*  	save_pixbuf_to_file_as_png (flippedout_v,"testorient-flipped-vertical.png"); */
	
	exit(0);
}


</pre>

<!--X-Body-of-Message-End-->
<!--X-MsgBody-End-->
<!--X-Follow-Ups-->
<hr>
<!--X-Follow-Ups-End-->
<!--X-References-->
<!--X-References-End-->
<!--X-BotPNI-->
<hr>
[<a href="http://mail.gnome.org/archives/eog-list/2001-May/msg00002.html">Date Prev</a>][<a href="http://mail.gnome.org/archives/eog-list/2001-May/msg00003.html">Date Next</a>]   [<a href="http://mail.gnome.org/archives/eog-list/2001-May/msg00002.html">Thread Prev</a>][Thread Next]   
[<a href="http://mail.gnome.org/archives/eog-list/2001-May/thread.html#00001">Thread Index</a>]
[<a href="http://mail.gnome.org/archives/eog-list/2001-May/date.html#00001">Date Index</a>]
[<a href="http://mail.gnome.org/archives/eog-list/2001-May/author.html#00001">Author Index</a>]

<!--X-BotPNI-End-->
<!--X-User-Footer-->
<!--X-User-Footer-End-->
</body></html>
