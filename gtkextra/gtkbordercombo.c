/* gtkbordercombo - border_combo widget for gtk+
 * Copyright 1999-2001 Adrian E. Feiguin <feiguin@ifir.edu.ar>
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
 * SECTION: gtkbordercombo
 * @short_description: A border combo widget
 *
 * It is a #GtkComboBox subclass with a variety of border styles in the popdown window arranged in a table of togglebuttons.
 */

/**
 * GtkBorderCombo:
 *
 * The GtkBorderCombo struct contains only private data.
 * It should only be accessed through the functions described below.
 */

#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "gtkcombobutton.h"
#include "gtkbordercombo.h"
#include "gtkextra-marshal.h"

/* SIGNALS */
enum {
   CHANGED,
   LAST_SIGNAL
};

static gint border_combo_signals[LAST_SIGNAL] = {0};


static char *xpm_border[]={
"15 15 2 1",
"      c None",
"X     c #000000000000",
"               ",
" X X X X X X X ",
"               ",
" X     X     X ",
"               ",
" X     X     X ",
"               ", 
" X X X X X X X ",
"               ",
" X     X     X ",
"               ",
" X     X     X ",
"               ",
" X X X X X X X ",
"               "};

static char *full   =" XXXXXXXXXXXXX ";
static char *dotted =" X X X X X X X ";
static char *side111=" X     X     X ";
static char *side000="               ";
static char *side101=" X           X ";
static char *side010="       X       ";
static char *side100=" X             ";
static char *side001="             X ";

static void         gtk_border_combo_class_init      (GtkBorderComboClass *klass);
static void         gtk_border_combo_init            (GtkBorderCombo      *border_combo);
static void         gtk_border_combo_destroy         (GtkObject     *border_combo);
static void         gtk_border_combo_realize         (GtkWidget *widget);
static GtkWidget*   create_border_pixmap             (GtkBorderCombo *border_combo, 
                                                      gchar *border[18]);

static GtkComboButtonClass *parent_class = NULL;

static void
gtk_border_combo_class_init (GtkBorderComboClass * klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;


  parent_class = g_type_class_ref (gtk_hbox_get_type ());
  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  object_class->destroy = gtk_border_combo_destroy;
  
  widget_class->realize = gtk_border_combo_realize;

  /**
   * GtkBorderCombo::changed:
   * @border_combo: the #GtkBorderCombo object that received the signal
   * @selection: the number of selected item
   *
   * Emmited when the GtkBorderCombo's state is changed
   */ 
  border_combo_signals[CHANGED]=g_signal_new("changed",
 			  G_OBJECT_CLASS_TYPE (object_class),
                          G_SIGNAL_RUN_FIRST,
                          G_STRUCT_OFFSET(GtkBorderComboClass, changed),
			  NULL, NULL,
                          gtkextra_VOID__INT,
                          G_TYPE_NONE, 1, G_TYPE_INT);

  klass->changed = NULL;
                                                        
}

static void
gtk_border_combo_destroy (GtkObject * border_combo)
{
  gint i,j;

  GtkBorderCombo *combo;
  combo=GTK_BORDER_COMBO(border_combo);

  if (combo->button) {
  for(i=0; i<combo->nrows; i++)
   for(j=0; j<combo->ncols; j++){
     if(combo->button[i][j]){
       gtk_widget_destroy(combo->button[i][j]);
       combo->button[i][j] = NULL;
     }
   }
   }
  
  if(GTK_BORDER_COMBO(border_combo)->table){ 
    gtk_widget_destroy (GTK_BORDER_COMBO(border_combo)->table);
    GTK_BORDER_COMBO(border_combo)->table = NULL;
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (border_combo);
}


static void
gtk_border_combo_update (GtkWidget * widget, GtkBorderCombo * border_combo)
{
  gint i,j;
  gint focus_row = -1, focus_col = -1;
  gint new_row = -1, new_col = -1;
  gint new_selection=FALSE;
  gint row,column;
  GdkPixmap *window;
  GdkPixmap *button;
  GtkImage *image;
  GdkBitmap *mask;

  row=border_combo->row;
  column=border_combo->column;

  printf("RRR gtk_border_combo_update\n");

  for(i=0 ; i<border_combo->nrows; i++)
    for(j=0; j<border_combo->ncols; j++){    
      if(gtk_widget_has_focus(border_combo->button[i][j])){
            focus_row=i;
            focus_col=j;
      }
      
      if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (
		border_combo->button[i][j]))) {
        if(i != row || j != column){
            new_selection=TRUE;
            new_row=i;
            new_col=j;
        }
      }
    }

  if(!new_selection && focus_row >= 0 && focus_col >= 0){
     if(focus_row != row || focus_col != column){
       new_selection = TRUE;
       new_row=focus_row;
       new_col=focus_col;
     }
  }

  if(new_selection){
      if(row >= 0 && column >= 0){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(border_combo->button[row][column]), FALSE);
          gtk_widget_queue_draw(border_combo->button[row][column]);
      }
      border_combo->row=new_row;
      border_combo->column=new_col;
      image = GTK_IMAGE( gtk_bin_get_child(GTK_BIN( GTK_COMBO_BUTTON(border_combo)->button)));
      gtk_image_get_pixmap(image, &window, &mask);
      gtk_image_get_pixmap ( 
        GTK_IMAGE(gtk_bin_get_child(GTK_BIN(border_combo->button[new_row][new_col]))),
	&button, &mask);
	
      gdk_draw_drawable(window,
		   gtk_widget_get_style(widget)->fg_gc[GTK_STATE_NORMAL],
		   button,
                   0,0,
                   0,0,16,16);

      gtk_widget_queue_draw(GTK_COMBO_BUTTON(border_combo)->button);
      
      g_signal_emit (GTK_OBJECT(border_combo), 
		border_combo_signals[CHANGED],
		0,
                new_row * border_combo->ncols + new_col);
  }

  if(!new_selection && row >= 0 && column >= 0){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(border_combo->button[row][column]), TRUE);
          gtk_widget_queue_draw(border_combo->button[row][column]);

          g_signal_emit (GTK_OBJECT(border_combo),                     
                     border_combo_signals[CHANGED],
		     0,
                     row * border_combo->ncols + column);
  }


  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_COMBO_BUTTON(border_combo)->arrow), FALSE);

  gtk_grab_remove(GTK_COMBO_BUTTON(border_combo)->popwin);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gtk_widget_hide(GTK_COMBO_BUTTON(border_combo)->popwin);
  return;

}

static void
gtk_border_combo_init (GtkBorderCombo * border_combo)
{
  GtkWidget *widget;

  widget=GTK_WIDGET(border_combo);

  border_combo->nrows = 3;
  border_combo->ncols = 4;
  border_combo->row = -1;
  border_combo->column = -1;

}


static GtkWidget *
create_border_pixmap(GtkBorderCombo *border_combo, gchar *border[18])
{
  GtkWidget *widget;
  GtkWidget *pixmap;
  GdkPixmap *border_pixmap;

  widget=GTK_WIDGET(border_combo);

  border_pixmap=gdk_pixmap_create_from_xpm_d(
                       gtk_widget_get_window(widget),
                       NULL,
                       &(gtk_widget_get_style(widget)->bg[GTK_STATE_NORMAL]),
                       border);    
  pixmap=gtk_image_new_from_pixmap(border_pixmap, NULL);
  return pixmap;  
 
}


GType
gtk_border_combo_get_type ()
{
  static GType border_combo_type = 0;

  if (!border_combo_type)
    {
      border_combo_type = g_type_register_static_simple(
		gtk_combo_button_get_type (),
		"GtkBorderCombo",
		sizeof (GtkBorderComboClass),
        	(GClassInitFunc) gtk_border_combo_class_init,
		sizeof (GtkBorderCombo),
		(GInstanceInitFunc) gtk_border_combo_init,
		0);

    }
  return border_combo_type;
}

GtkWidget *
gtk_border_combo_new ()
{
  return(gtk_widget_new(gtk_border_combo_get_type (), NULL));
}


static void
gtk_border_combo_realize(GtkWidget *widget)
{
  GtkComboButton *combo;
  GtkBorderCombo *border_combo;
  GtkWidget *pixmap;
  GtkRequisition requisition;
  GdkPixmap *border_pixmap;
  gint i,j;
  gchar *border[18];

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_BORDER_COMBO (widget));

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  combo=GTK_COMBO_BUTTON(widget);
  border_combo=GTK_BORDER_COMBO(widget);

  border_combo->table = gtk_table_new (border_combo->nrows, border_combo->ncols, TRUE);

  border_combo->button = (GtkWidget ***)g_malloc(border_combo->nrows*sizeof(GtkWidget **));


  for(i = 0; i < border_combo->nrows; i++){

    border_combo->button[i] = (GtkWidget **)g_malloc(border_combo->ncols*sizeof(GtkWidget *));

    for(j = 0; j < border_combo->ncols; j++){

        border_combo->button[i][j] = gtk_toggle_button_new();
        gtk_button_set_relief(GTK_BUTTON(border_combo->button[i][j]),
                              GTK_RELIEF_NONE);
        gtk_table_attach (GTK_TABLE(border_combo->table), 
                          border_combo->button[i][j],
                          j, j+1, i, i+1, GTK_SHRINK, GTK_SHRINK, 0, 0);

        gtk_widget_set_size_request(border_combo->button[i][j], 24, 24);
        gtk_widget_show(border_combo->button[i][j]); 
        g_signal_connect (GTK_OBJECT (border_combo->button[i][j]), 
			"toggled",
		        (void *) gtk_border_combo_update, 
                        border_combo);

    }
  }

  gtk_container_add(GTK_CONTAINER(GTK_COMBO_BUTTON(border_combo)->frame), 
                    border_combo->table);
  gtk_widget_show(border_combo->table);

  if(!gtk_bin_get_child(GTK_BIN(combo->button)) && 
	gtk_widget_get_window(widget)){
       border_pixmap=gdk_pixmap_create_from_xpm_d(
                             gtk_widget_get_window(widget),
                             NULL,
                             &(gtk_widget_get_style(combo->button)->bg[GTK_STATE_NORMAL]),
                             xpm_border);    

       pixmap = gtk_image_new_from_pixmap(border_pixmap, NULL);
       gtk_container_add(GTK_CONTAINER(combo->button), pixmap);
       gtk_widget_show(pixmap);
  }

  GTK_WIDGET_CLASS (parent_class)->size_request (widget, &requisition); 

  /* EMPTY */
  for(i=0; i<18; i++)
        border[i]=xpm_border[i];
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[0][0]), pixmap);
  gtk_widget_show(pixmap);

  /* TOP */
  border[4]=full;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[0][1]), pixmap);
  gtk_widget_show(pixmap);


  /* BOTTOM */
  border[4]=dotted;
  border[16]=full;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[0][2]), pixmap);
  gtk_widget_show(pixmap);


  /* RIGHT */
  border[16]=dotted;
  for(i=5; i<16; i=i+2)
        border[i]=side001;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[0][3]), pixmap);
  gtk_widget_show(pixmap);


  /* LEFT */
  for(i=5; i<16; i=i+2)
        border[i]=side100;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[1][0]), pixmap);
  gtk_widget_show(pixmap);


  /* v101 */
  for(i=5; i<16; i=i+2)
        border[i]=side101;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[1][1]), pixmap);
  gtk_widget_show(pixmap);
 

  /* h101 */
  for(i=5; i<16; i=i+2)
        border[i]=side000;
  border[4]=full;
  border[16]=full;  
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[1][2]), pixmap);
  gtk_widget_show(pixmap);

  /* v111 */
  border[4]=dotted;
  border[16]=dotted;
  for(i=5; i<16; i=i+2)
        border[i]=side111;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[1][3]), pixmap);
  gtk_widget_show(pixmap);  

  /* h111 */
  for(i=5; i<16; i=i+2)
        border[i]=side000;
  border[4]=full;
  border[16]=full;
  border[10]=full;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[2][0]), pixmap);
  gtk_widget_show(pixmap);

  /* CROSS */
  border[4]=dotted;
  border[16]=dotted;
  for(i=5; i<16; i=i+2)
        border[i]=side010;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[2][1]), pixmap);
  gtk_widget_show(pixmap);


  /* sides */
  for(i=5; i<16; i=i+2)
        border[i]=side101;
  border[4]=full;
  border[16]=full;
  border[10]=dotted;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[2][2]), pixmap);
  gtk_widget_show(pixmap);

  /* FULL */
  for(i=5; i<16; i=i+2)
        border[i]=side111;
  border[4]=full;
  border[10]=full;
  border[16]=full;
  pixmap=create_border_pixmap(border_combo, border);
  gtk_container_add(GTK_CONTAINER(border_combo->button[2][3]), pixmap);
  gtk_widget_show(pixmap);

  g_signal_connect (GTK_OBJECT (combo->button), "clicked",
		      (void*) gtk_border_combo_update, 
                       border_combo);
}
