/* gtkplotpc - gtkplot print context - a renderer for printing functions
 * Copyright 1999-2001  Adrian E. Feiguin <feiguin@ifir.edu.ar>
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
 * SECTION: gtkplotcairo
 * @short_description: Cairo drawing backend.
 *
 * Subclass of #GtkPlotPC used for screen drawing.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <gtk/gtk.h>
#include <cairo.h>

#include "gtkplotpc.h"
#include "gtkplotcairo.h"
#include "gtkplot.h"
#include "gtkpsfont.h"
#include "gtkplotcanvas.h"
#include <pango/pango.h>

static void gtk_plot_cairo_init                       (GtkPlotCairo *pc);
static void gtk_plot_cairo_class_init                 (GtkPlotCairoClass *klass);
static void gtk_plot_cairo_finalize                   (GObject *object);
static gboolean gtk_plot_cairo_real_init              (GtkPlotPC *pc);
static void gtk_plot_cairo_set_viewport               (GtkPlotPC *pc,
                                                       gdouble w, gdouble h);
static void gtk_plot_cairo_leave                      (GtkPlotPC *pc);
static void gtk_plot_cairo_gsave                      (GtkPlotPC *pc);
static void gtk_plot_cairo_grestore                   (GtkPlotPC *pc);
static void gtk_plot_cairo_clip                       (GtkPlotPC *pc,
                                                       const GdkRectangle *area);
static void gtk_plot_cairo_clip_mask                  (GtkPlotPC *pc,
                                                       gdouble x,
                                                       gdouble y,
                                                       const GdkBitmap *mask);
static void gtk_plot_cairo_set_color                   (GtkPlotPC *pc,
                                                        const GdkColor *color);
static void gtk_plot_cairo_set_lineattr           (GtkPlotPC *pc,
                                                   gfloat line_width,
                                                   GdkLineStyle line_style,
                                                   GdkCapStyle cap_style,
                                                   GdkJoinStyle join_style);
static void gtk_plot_cairo_set_dash                    (GtkPlotPC *pc,
                                                        gdouble offset_,
                                                        gdouble *values,
                                                        gint num_values);
static void gtk_plot_cairo_draw_point                  (GtkPlotPC *pc,
                                                        gdouble x, gdouble y);
static void gtk_plot_cairo_draw_line                   (GtkPlotPC *pc,
                                                        gdouble x1, gdouble y1,
                                                        gdouble x2, gdouble y2);
static void gtk_plot_cairo_draw_lines                  (GtkPlotPC *pc,
                                                        GtkPlotPoint *points,
                                                        gint numpoints);
static void gtk_plot_cairo_draw_rectangle              (GtkPlotPC *pc,
                                                        gint filled,
                                                        gdouble x, gdouble y,
                                                        gdouble width, 
                                                        gdouble height);
static void gtk_plot_cairo_draw_polygon                (GtkPlotPC *pc,
                                                        gint filled,
                                                        GtkPlotPoint *points,
                                                        gint numpoints);
static void gtk_plot_cairo_draw_circle                 (GtkPlotPC *pc,
                                                        gint filled,
                                                        gdouble x, gdouble y,
                                                        gdouble size);
static void gtk_plot_cairo_draw_ellipse                (GtkPlotPC *pc,
                                                        gint filled,
                                                        gdouble x, gdouble y,
                                                        gdouble width, 
                                                        gdouble height);
static void gtk_plot_cairo_set_font                    (GtkPlotPC *pc,
                                                        GtkPSFont *psfont,
                                                        gint height);
static void gtk_plot_cairo_draw_string                (GtkPlotPC *pc,
                                                       gint x, gint y,
                                                       gint angle,
                                                       const GdkColor *fg,
                                                       const GdkColor *bg,
                                                       gboolean transparent,
                                                       gint border,
                                                       gint border_space,
                                                       gint border_width,
                                                       gint shadow_width,
                                                       const gchar *font,
                                                       gint height,
                                                       GtkJustification just,
                                                       const gchar *text);
static void gtk_plot_cairo_draw_pixmap                (GtkPlotPC *pc,
                                                      GdkPixmap *pixmap,
                                                      GdkBitmap *mask,
                                                      gint xsrc, gint ysrc,
                                                      gint xdest, gint ydest,
                                                      gint width, gint height,
                                                      gdouble scale_x, 
                                                      gdouble scale_y);

extern inline gint roundint                         (gdouble x);

static GtkPlotPCClass *parent_class = NULL;

GType
gtk_plot_cairo_get_type (void)
{
  static GType pc_type = 0;

  if (!pc_type)
    {
      pc_type = g_type_register_static_simple (
		gtk_plot_pc_get_type(),
		"GtkPlotCairo",
		sizeof (GtkPlotCairoClass),
		(GClassInitFunc) gtk_plot_cairo_class_init,
		sizeof (GtkPlotCairo),
		(GInstanceInitFunc) gtk_plot_cairo_init,
		0);

    }
  return pc_type;
}

static void
gtk_plot_cairo_init (GtkPlotCairo *pc)
{
  pc->cairo = NULL;
  /*
    CairoWindowAttr attributes;
    gint attributes_mask;
    CairoScreen *screen;

    attributes.window_type = CAIRO_WINDOW_CHILD;
    attributes.title = NULL;
    attributes.wclass = CAIRO_INPUT_OUTPUT;
    attributes.visual = cairo_visual_get_system ();
    attributes.colormap = cairo_colormap_get_system ();
    attributes.event_mask = 0;
    attributes_mask = CAIRO_WA_VISUAL | CAIRO_WA_COLORMAP;
  */

  /*
    pc->window = cairo_window_new (NULL, &attributes, attributes_mask);

    screen = cairo_screen_get_default ();
  */
  /*
    pc->context = cairo_pango_context_get ();
  */
  /*
    pango_context_set_base_dir (pc->context,
    pc->text_direction == GTK_TEXT_DIR_LTR ?
    PANGO_DIRECTION_LTR : PANGO_DIRECTION_RTL);
  */
  
  /*
    pango_context_set_language (pc->context, gtk_get_default_language ());
    pc->layout = pango_layout_new(pc->context);
  */
}


static void
gtk_plot_cairo_class_init (GtkPlotCairoClass *klass)
{
  GtkObjectClass *object_class;
  GObjectClass *gobject_class;
  GtkPlotPCClass *pc_class;
  GtkPlotCairoClass *cairo_class;

  parent_class = g_type_class_ref (gtk_plot_pc_get_type ());

  object_class = (GtkObjectClass *) klass;
  gobject_class = (GObjectClass *) klass;

  pc_class = (GtkPlotPCClass *) klass;
  cairo_class = (GtkPlotCairoClass *) klass;

  gobject_class->finalize = gtk_plot_cairo_finalize;

  pc_class->init = gtk_plot_cairo_real_init;
  pc_class->leave = gtk_plot_cairo_leave;
  pc_class->set_viewport = gtk_plot_cairo_set_viewport;
  pc_class->gsave = gtk_plot_cairo_gsave;
  pc_class->grestore = gtk_plot_cairo_grestore;
  pc_class->clip = gtk_plot_cairo_clip;
  pc_class->clip_mask = gtk_plot_cairo_clip_mask;
  pc_class->set_color = gtk_plot_cairo_set_color;
  pc_class->set_dash = gtk_plot_cairo_set_dash;
  pc_class->set_lineattr = gtk_plot_cairo_set_lineattr;
  pc_class->draw_point = gtk_plot_cairo_draw_point;
  pc_class->draw_line = gtk_plot_cairo_draw_line;
  pc_class->draw_lines = gtk_plot_cairo_draw_lines;
  pc_class->draw_rectangle = gtk_plot_cairo_draw_rectangle;
  pc_class->draw_polygon = gtk_plot_cairo_draw_polygon;
  pc_class->draw_circle = gtk_plot_cairo_draw_circle;
  pc_class->draw_ellipse = gtk_plot_cairo_draw_ellipse;
  pc_class->set_font = gtk_plot_cairo_set_font;
  pc_class->draw_string = gtk_plot_cairo_draw_string;
  pc_class->draw_pixmap = gtk_plot_cairo_draw_pixmap;
}

/**
 * gtk_plot_cairo_new:
 * @cairo:
 *
 *
 *
 * Return value:
 */
GtkObject *
gtk_plot_cairo_new (cairo_t *cairo)
{
  GtkObject *object;

  object = g_object_new(gtk_plot_cairo_get_type(), NULL);
  gtk_plot_cairo_construct(GTK_PLOT_CAIRO(object), cairo, NULL);
  GTK_PLOT_CAIRO(object)->destroy_cairo = FALSE;

  return (object);
}

/**
 * gtk_plot_cairo_new_with_drawable:
 * @drawable:
 *
 *
 *
 * Return value:
 */
GtkObject *
gtk_plot_cairo_new_with_drawable (GdkDrawable *drawable)
{
  GtkObject *object;
  cairo_t *cairo = NULL;

  object = g_object_new(gtk_plot_cairo_get_type(), NULL);
  if(drawable) cairo = gdk_cairo_create(drawable);
  gtk_plot_cairo_construct(GTK_PLOT_CAIRO(object), cairo, NULL);
  GTK_PLOT_CAIRO(object)->destroy_cairo = TRUE;

  return (object);
}

/**
 * gtk_plot_cairo_construct:
 * @pc: A #GtkPlotCairo.
 * @cairo:
 * @context:
 *
 *
 */
void
gtk_plot_cairo_construct(GtkPlotCairo *pc,
                         cairo_t *cairo,
                         PangoContext *context)
{

  gtk_plot_cairo_set_cairo(pc, cairo);

  pc->context = context;
 
  if (pc->context) 
    {
      g_object_ref(G_OBJECT(pc->context));
    }
  if (pc->layout) 
    {
      g_object_ref(G_OBJECT(pc->layout));
    }
  else 
    { 
      if(pc->cairo) pc->layout = pango_cairo_create_layout(cairo);
    }
}


static void
gtk_plot_cairo_finalize (GObject *object)
{
  GtkPlotCairo *pc = GTK_PLOT_CAIRO(object);

  if(pc->destroy_cairo && pc->cairo) cairo_destroy(pc->cairo);
  GTK_PLOT_CAIRO(object)->cairo = NULL;

  if(pc->layout)
    g_object_unref(G_OBJECT(pc->layout));
  pc->layout = NULL;

  if(pc->context)
    g_object_unref(G_OBJECT(pc->context));
  pc->context = NULL;
}

static gboolean 
gtk_plot_cairo_real_init (GtkPlotPC *pc)
{
  return TRUE;
}

static void
gtk_plot_cairo_leave (GtkPlotPC *pc)
{
}

/**
 * gtk_plot_cairo_set_cairo:
 * @pc: a #GtkPlotCairo
 * @cairo:
 *
 *
 */
void
gtk_plot_cairo_set_cairo(GtkPlotCairo *pc,
                         cairo_t *cairo)
{
  if(pc->destroy_cairo && pc->cairo && cairo) cairo_destroy(pc->cairo);
  pc->cairo = cairo;
}

static void 
gtk_plot_cairo_set_viewport               (GtkPlotPC *pc, gdouble w, gdouble h)
{
}

static void 
gtk_plot_cairo_gsave                                  (GtkPlotPC *pc)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;
  cairo_save(cairo);
}

static void 
gtk_plot_cairo_grestore                                  (GtkPlotPC *pc)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;
  cairo_restore(cairo);
}

static void 
gtk_plot_cairo_clip (GtkPlotPC *pc,
                     const GdkRectangle *area)
{
  /* discard CairoRectangle* const: 
   * cairo_gc_set_clip_rectangle should have a const arg.
   * I've checked the code and it doesn't change it or keep it. murrayc.
   */
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo; /* Shortcut */
  if (!cairo)
    return;
  cairo_reset_clip(cairo);
  cairo_new_path(cairo);
  if (area) {
      cairo_move_to(cairo, area->x, area->y);
      cairo_line_to(cairo, area->x+area->width, area->y);
      cairo_line_to(cairo, area->x+area->width, area->y+area->height);
      cairo_line_to(cairo, area->x, area->y+area->height);
      cairo_close_path(cairo);
      cairo_clip(cairo);
  }
}

static void 
gtk_plot_cairo_clip_mask                              (GtkPlotPC *pc,
                                                       gdouble x,
                                                       gdouble y,
                                                       const GdkBitmap *mask)
{
  /* TBD: Currently no support for clip mask */
  return;
}

static void 
gtk_plot_cairo_set_color                               (GtkPlotPC *pc,
                                                        const GdkColor *color)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo; /* Shortcut */
  if (!cairo)
    return;
  cairo_set_source_rgba(cairo,
                        1.0/65535 * color->red,
                        1.0/65535 * color->green,
                        1.0/65535 * color->blue,
                        1.0);  // TBD fix alpha
}

static void 
gtk_plot_cairo_set_dash                               (GtkPlotPC *pc,
                                                       gdouble offset,
                                                       gdouble *values,
                                                       gint num_values)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo; /* Shortcut */
  if (!cairo)
    return;
  gchar list[] = {'\0','\1','\2','\3','\4','\5','\6','\7'};
  double dash[1000];
  gint i;

  if(num_values == 0){
    return;
  }

  /* TBD - Fix this */
  for(i = 0; i < num_values; i++){
    gint value;
    value = values[i];
    dash[i] = list[value];
  }  

  cairo_set_dash(cairo, dash, num_values, 0);
}

static void gtk_plot_cairo_set_lineattr           (GtkPlotPC *pc,
                                                   gfloat line_width,
                                                   GdkLineStyle line_style,
                                                   GdkCapStyle cap_style,
                                                   GdkJoinStyle join_style)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo; /* Shortcut */

  if (!cairo)
    return;
  if (line_style == GDK_LINE_SOLID)
      cairo_set_dash(cairo,
                     NULL, 0, 0);

  if (line_width == 0)
      line_width = 0.5;   // What should be the minimum line width?
  cairo_set_line_width(cairo,
                       line_width);

  if(cap_style == GDK_CAP_NOT_LAST || cap_style == GDK_CAP_PROJECTING) 
    cairo_set_line_cap(cairo, CAIRO_LINE_CAP_SQUARE);
  if(cap_style == GDK_CAP_BUTT) cairo_set_line_cap(cairo, CAIRO_LINE_CAP_BUTT);
  if(cap_style == GDK_CAP_ROUND) cairo_set_line_cap(cairo, CAIRO_LINE_CAP_ROUND);

  cairo_set_line_join(cairo,
                     (cairo_line_join_t)join_style);
}

static void 
gtk_plot_cairo_draw_point                              (GtkPlotPC *pc,
                                                        gdouble x, gdouble y)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo; /* Shortcut */
  if (!cairo)
    return;
  /* Move and draw to same point, like in the postscript backend */
  cairo_move_to(cairo, x,y);
  cairo_line_to(cairo, x,y);
  cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_draw_line                               (GtkPlotPC *pc,
                                                        gdouble x1, gdouble y1,
                                                        gdouble x2, gdouble y2)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;
  cairo_move_to(cairo, x1,y1);
  cairo_line_to(cairo, x2,y2);
  cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_draw_lines                              (GtkPlotPC *pc,
                                                        GtkPlotPoint *points,
                                                        gint numpoints)
{
  gint i;
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;

  cairo_move_to(cairo, points[0].x,points[0].y);
  for(i = 1; i < numpoints; i++){
    cairo_line_to(cairo, points[i].x, points[i].y);
  }
  cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_draw_rectangle                          (GtkPlotPC *pc,
                                                        gint filled,
                                                        gdouble x, gdouble y,
                                                        gdouble width, gdouble height)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if(!cairo)
    return;

  cairo_move_to(cairo, x, y);
  cairo_line_to(cairo, x+width, y);
  cairo_line_to(cairo, x+width, y+height);
  cairo_line_to(cairo, x, y+height);
  cairo_close_path(cairo);
  if (filled)
    cairo_fill(cairo);
  else
    cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_draw_polygon                            (GtkPlotPC *pc,
                                                        gint filled,
                                                        GtkPlotPoint *points,
                                                        gint numpoints)
{
  gint i;
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;

  cairo_move_to(cairo, points[0].x,points[0].y);
  for(i = 1; i < numpoints; i++)
    cairo_line_to(cairo, points[i].x, points[i].y);
  cairo_close_path(cairo);
  if (filled)
    cairo_fill(cairo);
  else
    cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_draw_circle                             (GtkPlotPC *pc,
                                                        gint filled,
                                                        gdouble x, gdouble y,
                                                        gdouble size)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;

  cairo_arc (cairo, x, y, size/2, 0., 2 * M_PI);
  if (filled)
    cairo_fill(cairo);
  else
    cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_draw_ellipse                            (GtkPlotPC *pc,
                                                        gint filled,
                                                        gdouble x, gdouble y,
                                                        gdouble width, gdouble height)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  if (!cairo)
    return;

  cairo_save(cairo);
  cairo_translate(cairo, x+width/2.0,y+height/2.0);
  cairo_scale (cairo, 1. / (height / 2.), 1. / (width / 2.));
  cairo_arc (cairo, 0., 0., 1., 0., 2 * M_PI);
  cairo_restore(cairo);

  if (filled)
    cairo_fill(cairo);
  else
    cairo_stroke(cairo);
}

static void 
gtk_plot_cairo_set_font                                (GtkPlotPC *pc,
                                                        GtkPSFont *psfont,
                                                        gint height)
{
}

/* subfunction of gtk_plot_cairo_draw_string(). */
static gint
drawstring(GtkPlotPC *pc,
           gint angle,
           gint dx, gint dy,
           GtkPSFont *psfont, gint height,
           const gchar *text)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  PangoLayout *layout = GTK_PLOT_CAIRO(pc)->layout;
  PangoFontDescription *font;
  PangoRectangle rect;
  PangoFontMap *map;
  gint ret_value;
  gint dpi_cairo, dpi_screen;
  GdkScreen *screen = gdk_screen_get_default();

  if(!text || strlen(text) == 0) return 0;
  cairo_save(cairo);

  map = pango_cairo_font_map_get_default();
  dpi_cairo = pango_cairo_font_map_get_resolution(PANGO_CAIRO_FONT_MAP(map));
  dpi_screen = gdk_screen_get_resolution(screen);
  if (dpi_screen != -1)
    height *= (double)dpi_screen/(double)dpi_cairo;
  font = gtk_psfont_get_font_description(psfont, height);
  pango_layout_set_font_description(GTK_PLOT_CAIRO(pc)->layout, font);
  pango_layout_set_text(GTK_PLOT_CAIRO(pc)->layout, text, strlen(text));
  pango_layout_get_extents(GTK_PLOT_CAIRO(pc)->layout, NULL, &rect);

  if (psfont->i18n_latinfamily && psfont->vertical) {
    /* vertical-writing CJK postscript fonts. */
    return rect.height;
  } 
  else 
    {
      /* horizontal writing */
      if(angle == 90)
//        cairo_translate(cairo, dx, dy-PANGO_PIXELS(rect.width));
        cairo_translate(cairo, dx, dy);
      else if(angle == 270)
        cairo_translate(cairo, dx+PANGO_PIXELS(rect.height), dy);
      else if(angle == 180)
        cairo_translate(cairo, dx-PANGO_PIXELS(rect.width), dy);
      else
        cairo_translate(cairo, dx, dy);
    }
  cairo_rotate(cairo, -angle * G_PI / 180);
  pango_cairo_update_layout(cairo, layout);
  pango_cairo_show_layout(cairo, layout);
  cairo_restore(cairo);
  pango_font_description_free(font);
  ret_value = (angle == 0 || angle == 180) ? rect.width : rect.height;
  return PANGO_PIXELS(rect.width);
}

static void 
gtk_plot_cairo_draw_string                        (GtkPlotPC *pc,
                                                   gint tx, gint ty,
                                                   gint angle,
                                                   const GdkColor *fg,
                                                   const GdkColor *bg,
                                                   gboolean transparent,
                                                   gint border,
                                                   gint border_space,
                                                   gint border_width,
                                                   gint shadow_width,
                                                   const gchar *font_name,
                                                   gint font_height,
                                                   GtkJustification just,
                                                   const gchar *text)
{
  cairo_t *cairo = GTK_PLOT_CAIRO(pc)->cairo;
  GList *family = NULL;
  gint x0, y0;
  gint old_width, old_height;
  gboolean bold, italic;
  gint fontsize;
  gint ascent, descent;
  gint numf;
  gint width, height;
  gint x, y;
  gint i;
  PangoFontDescription *font = NULL, *latin_font = NULL;
  GtkPSFont *psfont, *base_psfont, *latin_psfont;
  gchar subs[2], insert_char;
  const gchar *aux = text;
  const gchar *lastchar = text;
  const gchar *wtext = text;
  const gchar *xaux = text;
  gchar *new_text; /* Support Tiny C compiler : Original : gchar new_text[strlen(text)+1];*/
  gchar num[4];
  PangoRectangle rect;
  PangoLayout *layout = NULL;
  gint real_x, real_y, real_width, real_height;
  GdkColor real_fg = *fg;
  GdkColor real_bg = *bg;
  gint sign_x = 1, sign_y = 0;
  gint old_tx = tx, old_ty = ty;

  if (!cairo)
    return;

  layout = GTK_PLOT_CAIRO(pc)->layout;
  cairo_save(cairo);
  gtk_plot_cairo_set_color(pc, fg);
/*
  font_name = "sans";
  desc = pango_font_description_from_string(font_name);

  // Since the name does not contain the size yet... Also there is some
  // factor that I have to figure out...
  pango_font_description_set_size (desc, font_height *0.9 * PANGO_SCALE);

  pango_layout_set_font_description(layout, desc);
  pango_layout_set_text(layout, text, -1);
  cairo_save(cairo);
  cairo_translate(cairo, tx, ty);
  cairo_rotate(cairo, angle * G_PI / 180);
  gtk_plot_cairo_set_color(pc, fg);
  pango_cairo_update_layout(cairo, layout);
  PangoFontMetrics *metrics = NULL;

  metrics = pango_context_get_metrics(pango_layout_get_context(layout), desc, gtk_get_default_language());

  pango_layout_get_size (layout, &width, &height);

  ascent = pango_font_metrics_get_ascent(metrics);
  descent = pango_font_metrics_get_descent(metrics);

  if (just == GTK_JUSTIFY_RIGHT) 
    cairo_move_to(cairo, -PANGO_PIXELS(width),
                  -PANGO_PIXELS(ascent)
                  );
  else if (just == GTK_JUSTIFY_CENTER) 
    cairo_move_to(cairo, -PANGO_PIXELS(width)/2.0,
                  -PANGO_PIXELS(ascent)
                  );
  else if (just == GTK_JUSTIFY_LEFT)
    cairo_move_to(cairo, 0,
                  -PANGO_PIXELS(ascent)
                  );
  
  pango_cairo_show_layout(cairo, layout);
  cairo_restore(cairo);
*/


  gtk_plot_text_get_size(text, angle, font_name, font_height, &width, &height, &ascent, &descent);

  if(height == 0 || width == 0) return;

  old_width = width;
  old_height = height;
  if(angle == 90 || angle == 270)
    {
      old_width = height;
      old_height = width;
    }
  switch(angle){
    case 90:
      sign_x = 0;
      sign_y = -1;
      break;
    case 180:
      sign_x = -1;
      sign_y = 0;
      break;
    case 270:
      sign_x = 0;
      sign_y = 1;
      break;
    case 0:
    default:
      sign_x = 1;
      sign_y = 0;
      break;
  }

  switch(just){
    case GTK_JUSTIFY_LEFT:
      switch(angle){
        case 0:
            ty -= ascent;
            break;
        case 90:
            ty -= height;
            tx -= ascent;
            break;
        case 180:
            tx -= width;
            ty -= descent;
            break;
        case 270:
            tx -= descent;
            break;
      }
      old_tx = tx;
      old_ty = ty;
      break;
    case GTK_JUSTIFY_RIGHT:
      switch(angle){
        case 0:
            tx -= width;
            ty -= ascent;
            old_tx -= width;
            old_ty -= ascent;
            break;
        case 90:
            tx -= ascent;
            ty += height;
            old_tx -= ascent;
            break;
        case 180:
            tx += width;
            ty -= descent;
            old_ty -= descent;
            break;
        case 270:
            tx -= descent;
            old_tx -= descent;
            old_ty -= height;
            break;
      }
      break;
    case GTK_JUSTIFY_CENTER:
    default:
      switch(angle){
        case 0:
            tx -= width / 2.;
            ty -= ascent;
            old_tx -= width / 2.;
            old_ty -= ascent;
            break;
        case 90:
            tx -= ascent;
            ty += height / 2.;
            old_tx -= ascent;
            old_ty -= height / 2.;
            break;
        case 180:
            tx += width / 2.;
            ty -= descent;
            old_tx -= width / 2.;
            old_ty -= descent;
            break;
        case 270:
            tx -= descent;
            ty -= height / 2.;
            old_tx -= descent;
            old_ty -= height / 2.;
            break;
      }
  }

  real_x = tx;
  real_y = ty;
  real_width = width;
  real_height = height;

  if(!transparent){
    gtk_plot_cairo_set_color(pc, &real_bg);
    gtk_plot_cairo_draw_rectangle(pc, TRUE, old_tx, old_ty, old_width, old_height);
  }
  gtk_psfont_get_families(&family, &numf);
  base_psfont = psfont = gtk_psfont_get_by_name(font_name);
  font = gtk_psfont_get_font_description(psfont, font_height);
  italic = psfont->italic;
  bold = psfont->bold;
  fontsize = font_height;
  x0 = x = 0;
  y0 = y = 0;

  if (psfont->i18n_latinfamily) {
    latin_psfont = gtk_psfont_get_by_family(psfont->i18n_latinfamily, italic,
                                             bold);
    if(latin_font) pango_font_description_free(latin_font);
    latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
  } else {
    latin_psfont = NULL;
    latin_font = NULL;
  }

  gtk_plot_cairo_set_color(pc, &real_fg);
  aux = text;
  while(aux && *aux != '\0' && *aux != '\n'){
   if(*aux == '\\'){
     aux = g_utf8_next_char(aux);
     switch(*aux){
       case '0': case '1': case '2': case '3':
       case '4': case '5': case '6': case '7': case '9':
           psfont = gtk_psfont_get_by_family((gchar *)g_list_nth_data(family, *aux-'0'), italic, bold);
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           aux = g_utf8_next_char(aux);
           break;
       case '8': case 'g':
           psfont = gtk_psfont_get_by_family("Symbol", italic, bold);
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           aux = g_utf8_next_char(aux);
           break;
       case 'B':
           bold = TRUE;
           psfont = gtk_psfont_get_by_family(psfont->family, italic, bold);
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if(latin_font){
             latin_font = NULL;
           }
           if (psfont->i18n_latinfamily) {
             latin_psfont = gtk_psfont_get_by_family(psfont->i18n_latinfamily,
                                                      italic, bold);
             if(latin_font) pango_font_description_free(latin_font);
             latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
           }
           aux = g_utf8_next_char(aux);
           break;
       case 'x':
           xaux = aux + 1;
           for (i=0; i<3; i++){
            if (xaux[i] >= '0' && xaux[i] <= '9')
              num[i] = xaux[i];
            else
              break;
           }
           if (i < 3){
              aux = g_utf8_next_char(aux);
              break;
           }
           num[3] = '\0';
           insert_char = (gchar)atoi(num);
           subs[0] = insert_char;
           subs[1] = '\0';
           pango_layout_set_font_description(layout, font);
           pango_layout_set_text(layout, subs, 1);
           pango_layout_get_extents(layout, NULL, &rect);
           x += sign_x*PANGO_PIXELS(rect.width);
           y += sign_y*PANGO_PIXELS(rect.width);
           aux += 4;
           lastchar = aux - 1;
           break;
       case 'i':
           italic = TRUE;
           psfont = gtk_psfont_get_by_family(psfont->family, italic, bold);
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if (psfont->i18n_latinfamily) {
             latin_psfont = gtk_psfont_get_by_family(psfont->i18n_latinfamily,
                                                      italic, bold);
             if(latin_font) pango_font_description_free(latin_font);
             latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
           }
           aux = g_utf8_next_char(aux);
           break;
       case 'S': case '^':
           fontsize = (int)((gdouble)fontsize * 0.6 + 0.5);
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if (psfont->i18n_latinfamily) {
             latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
           }
           if(angle == 180)
             y = y0 + fontsize;
           else if(angle == 270)
             x = x0 + sign_y*fontsize;
           aux = g_utf8_next_char(aux);
           break;
       case 's': case '_':
           fontsize = (int)((gdouble)fontsize * 0.6 + 0.5);
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if(angle == 0)
             y = y0 + fontsize;
           else if(angle == 90)
             x = x0 - sign_y*fontsize;
           if (psfont->i18n_latinfamily) {
             latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
           }
           aux = g_utf8_next_char(aux);
           break;
       case '+':
           fontsize += 3;
           y -= sign_x*3;
           x += sign_y*3;
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if (psfont->i18n_latinfamily) {
             latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
           }
           aux = g_utf8_next_char(aux);
           break;
       case '-':
           fontsize -= 3;
           y += sign_x*3;
           x -= sign_y*3;
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if (psfont->i18n_latinfamily) {
             latin_font = gtk_psfont_get_font_description(latin_psfont, fontsize);
           }
           aux = g_utf8_next_char(aux);
           break;
       case 'N':
           psfont = base_psfont;
           fontsize = font_height;
           pango_font_description_free(font);
           font = gtk_psfont_get_font_description(psfont, fontsize);
           if(angle == 0 || angle == 180)
             y = y0;
           else
             x = x0;
           italic = psfont->italic;
           bold = psfont->bold;
           aux = g_utf8_next_char(aux);
           break;
       case 'b':
           if (lastchar) {
             const gchar *aux2 = lastchar;
             gint i = aux2 - g_utf8_prev_char(lastchar);
             pango_layout_set_text(layout, lastchar, i);
             pango_layout_get_extents(layout, NULL, &rect);
             x -= sign_x*PANGO_PIXELS(rect.width);
             y -= sign_y*PANGO_PIXELS(rect.width);

             if (lastchar == wtext)
               lastchar = NULL;
             else
               lastchar = g_utf8_prev_char(lastchar);
           } else {
             pango_layout_set_text(layout, "X", 1);
             pango_layout_get_extents(layout, NULL, &rect);
             x -= sign_x*PANGO_PIXELS(rect.width);
             y -= sign_y*PANGO_PIXELS(rect.width);
           }
           aux = g_utf8_next_char(aux);
           break;
       default:
           if(aux && *aux != '\0' && *aux !='\n'){
             gint new_width = 0;
             new_width = drawstring(pc, angle, tx+x, ty+y,
                             psfont, fontsize, aux);
             x += sign_x * new_width;
             y += sign_y * new_width;
             lastchar = aux;
             aux = g_utf8_next_char(aux);
           }
           break;
     }
   } else {
     gint new_len = 0;
     gint new_width = 0;
     lastchar = aux;
     while(aux && *aux != '\0' && *aux !='\n' && *aux != '\\'){
       xaux = aux;
       new_len += g_utf8_next_char(aux) - xaux;
       xaux++;
       aux = g_utf8_next_char(aux);
     }
     xaux = lastchar;

     new_text = (gchar *) g_new0(gchar , strlen(text)+1); /* Tiny C Compiler support */
     for(i = 0; i < new_len; i++) new_text[i] = *xaux++;
     new_text[new_len] = '\0';
     new_width = drawstring(pc, angle, tx+x, ty+y,
                 psfont, fontsize, new_text);
     x += sign_x * new_width;
     y += sign_y * new_width;
     lastchar = aux;

     g_free (new_text);
   }
  }

  if(latin_font) pango_font_description_free(latin_font);

/* border */

  gtk_plot_cairo_set_color(pc, &real_fg);
  gtk_plot_pc_set_dash(pc, 0, NULL, 0);
  gtk_plot_pc_set_lineattr(pc, border_width, 0, 0, 0);
  switch(border){
    case GTK_PLOT_BORDER_SHADOW:
      gtk_plot_pc_draw_rectangle(pc,
                         TRUE,
                         old_tx - border_space + shadow_width,
                         old_ty + height + border_space,
                         width + 2 * border_space, shadow_width);
      gtk_plot_pc_draw_rectangle(pc,
                         TRUE,
                         old_tx + width + border_space,
                         old_ty - border_space + shadow_width,
                         shadow_width, height + 2 * border_space);
    case GTK_PLOT_BORDER_LINE:
      gtk_plot_pc_draw_rectangle(pc,
                         FALSE,
                         old_tx - border_space, old_ty - border_space,
                         width + 2*border_space, height + 2*border_space);
    case GTK_PLOT_BORDER_NONE:
    default:
        break;
  }

  cairo_restore(cairo);
  return;
}

static void gtk_plot_cairo_draw_pixmap                (GtkPlotPC *pc,
                                                      GdkPixmap *pixmap,
                                                      GdkBitmap *mask,
                                                      gint xsrc, gint ysrc,
                                                      gint xdest, gint ydest,
                                                      gint width, gint height,
                                                      gdouble scale_x, 
                                                      gdouble scale_y)
{
  if (!GTK_PLOT_CAIRO(pc)->cairo)
    return;

  cairo_surface_t *image_surface = NULL;
  cairo_surface_t *mask_surface = NULL;
  cairo_t *cr;
/* TODO: USE MASK */

  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, (width-xsrc)*scale_x, (height-ysrc)*scale_y);
  cr = cairo_create(image_surface);
  cairo_scale(cr,scale_x,scale_y);
  gdk_cairo_set_source_pixmap(cr,pixmap,xsrc,ysrc);
  cairo_paint(cr);
  cairo_destroy(cr);

/*
  mask_surface = cairo_image_surface_create(CAIRO_CONTENT_COLOR_ALPHA, (width-xsrc)*scale_x, (height-ysrc)*scale_y);
  cr = cairo_create(image_surface);
  cairo_scale(cr,scale_x,scale_y);
  gdk_cairo_set_source_pixmap(cr,mask,xsrc,ysrc);
  cairo_paint(cr);
  cairo_destroy(cr);
*/
  if(mask){
    mask_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, (width-xsrc)*scale_x, (height-ysrc)*scale_y);
    cr = cairo_create(mask_surface);
    cairo_set_source_rgb(cr,0,0,0);
    cairo_scale(cr,scale_x,scale_y);
    gdk_cairo_set_source_pixmap(cr,pixmap,xsrc,ysrc);
    cairo_mask_surface(cr,mask_surface,0,0);
    cairo_fill(cr);
    cairo_destroy(cr);
  }

  cairo_save(GTK_PLOT_CAIRO(pc)->cairo);
/*
  cairo_rectangle(GTK_PLOT_CAIRO(pc)->cairo,xdest,ydest,(width-xsrc)*scale_x,(height-ysrc)*scale_y);
  cairo_clip(GTK_PLOT_CAIRO(pc)->cairo);
*/
/*
  if(mask) cairo_mask_surface(GTK_PLOT_CAIRO(pc)->cairo,mask_surface,xdest,ydest);
*/
  cairo_set_source_surface(GTK_PLOT_CAIRO(pc)->cairo,image_surface,xdest,ydest);
  cairo_paint(GTK_PLOT_CAIRO(pc)->cairo);
  
  cairo_restore(GTK_PLOT_CAIRO(pc)->cairo);
  cairo_surface_destroy(image_surface);
  cairo_surface_destroy(mask_surface);
}
