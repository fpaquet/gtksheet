/* gtkcolorcombo - toggle_combo widget for gtk+
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
 * SECTION: gtktogglecombo
 * @short_description: A toggle combo widget for GTK.
 *
 * A GtkToggleCombo is a GtkCombo which will remain 'pressed-in' when clicked. 
 * Clicking it again will cause the toggle combo to return to it's normal state.
 * A toggle combo is created by calling gtk_toggle_combo_new().
 * The number of rows/columns may be find out by calling gtk_toggle_combo_get_nrows() or gtk_toggle_combo_get_ncols().
 * A combo row may be selected with gtk_toggle_combo_select().The current selection is returned by gtk_toggle_combo_get_selection().
 */

/**
 * GtkToggleCombo:
 *
 * The GtkToggleCombo struct contains only private data.
 * It should only be accessed through the functions described below.
 */


#include <string.h>
#include <stdio.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtksignal.h>
#include <gdk/gdkkeysyms.h>
#include "gtkcombobutton.h"
#include "gtktogglecombo.h"
#include "gtkextra-marshal.h"


/* SIGNALS */
enum {
   CHANGED,
   LAST_SIGNAL
};

static gint toggle_combo_signals[LAST_SIGNAL] = {0};


static void         gtk_toggle_combo_class_init      (GtkToggleComboClass *klass);
static void         gtk_toggle_combo_init            (GtkToggleCombo      *toggle_combo);
static void         gtk_toggle_combo_destroy         (GtkObject     *toggle_combo);
static void         gtk_toggle_combo_create_buttons  (GtkWidget *widget);


static GtkComboButtonClass *parent_class = NULL;

static void
gtk_toggle_combo_class_init (GtkToggleComboClass * klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  parent_class = g_type_class_ref (gtk_hbox_get_type ());
  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  object_class->destroy = gtk_toggle_combo_destroy;

  /**
   * GtkToggleCombo::changed:
   * @toggle_combo: the #GtkToggleCombo object that received the signal
   * @row: row number
   * @col: column number
   *
   * Emmited when the @row,@col are selected in #GtkToggleCombo
   */ 
  toggle_combo_signals[CHANGED]=g_signal_new("changed",
                                 G_TYPE_FROM_CLASS(object_class),
                                 G_SIGNAL_RUN_FIRST,
                                 G_STRUCT_OFFSET(GtkToggleComboClass, changed),
				 NULL, NULL,
                                 gtkextra_VOID__INT_INT,
                                 G_TYPE_NONE,  
                                 2, G_TYPE_INT, G_TYPE_INT);

  klass->changed = NULL;
}

static void
gtk_toggle_combo_destroy (GtkObject * toggle_combo)
{
  gint i,j;

  GtkToggleCombo *combo;
  combo=GTK_TOGGLE_COMBO(toggle_combo);

  if(combo && combo->button) 
   for(i=0; i<combo->nrows; i++)
    for(j=0; j<combo->ncols; j++)
      if(combo->button[i][j]){
        gtk_widget_destroy(combo->button[i][j]);
        combo->button[i][j] = NULL;
      }
 
  if(GTK_TOGGLE_COMBO(toggle_combo)->table){
    gtk_widget_destroy (GTK_TOGGLE_COMBO(toggle_combo)->table);
    GTK_TOGGLE_COMBO(toggle_combo)->table = NULL;
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (toggle_combo);
}



static void
gtk_toggle_combo_update (GtkWidget * widget, GtkToggleCombo * toggle_combo)
{
  gint i,j;
  gint focus_row = -1, focus_col = -1;
  gint new_row = -1, new_col = -1;
  gint new_selection=FALSE;
  gint row,column;

  row = toggle_combo->row;
  column = toggle_combo->column;

  for(i=0 ; i<toggle_combo->nrows; i++)
    for(j=0; j<toggle_combo->ncols; j++){    
      if(gtk_widget_has_focus(toggle_combo->button[i][j])){
            focus_row=i;
            focus_col=j;
      }
      if(gtk_widget_get_state(toggle_combo->button[i][j])==GTK_STATE_ACTIVE){
        if(i != row || j != column){
            new_selection=TRUE;
            new_row=i;
            new_col=j;
        }
      }
    }

  if(!new_selection && focus_row >= 0 && focus_col >= 0){
     if(focus_row != row && focus_col != column){
       new_selection = TRUE;
       new_row=focus_row;
       new_col=focus_col;
     }
  }

  if(new_selection){
      if(row >= 0 && column >= 0){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_combo->button[row][column]), FALSE);
          gtk_widget_queue_draw(toggle_combo->button[row][column]);
      }
      toggle_combo->row = new_row;
      toggle_combo->column = new_col;
      
      g_signal_emit (GTK_OBJECT(toggle_combo), toggle_combo_signals[CHANGED], 0,
                       new_row, new_col);

  }

  if(!new_selection && row >= 0 && column >= 0){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_combo->button[row][column]), TRUE);
          gtk_widget_queue_draw(toggle_combo->button[row][column]);

          g_signal_emit (GTK_OBJECT(toggle_combo), 
                           toggle_combo_signals[CHANGED], 0,
                           row, column);

  }



  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_COMBO_BUTTON(toggle_combo)->arrow), FALSE);

  gtk_grab_remove(GTK_COMBO_BUTTON(toggle_combo)->popwin);
  gdk_pointer_ungrab(GDK_CURRENT_TIME);
  gtk_widget_hide(GTK_COMBO_BUTTON(toggle_combo)->popwin);
  return;
}

static void
gtk_toggle_combo_init (GtkToggleCombo * toggle_combo)
{
  toggle_combo->row = -1;
  toggle_combo->column = -1;
}

static void
gtk_toggle_combo_create_buttons(GtkWidget *widget)
{
  GtkToggleCombo *toggle_combo;
  GtkComboButton *combo;
  gint i,j;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_TOGGLE_COMBO (widget));

  toggle_combo = GTK_TOGGLE_COMBO(widget);
  combo = GTK_COMBO_BUTTON(widget);

  toggle_combo->table = gtk_table_new (toggle_combo->nrows, toggle_combo->ncols, TRUE);

  toggle_combo->button = (GtkWidget ***)g_malloc(toggle_combo->nrows*sizeof(GtkWidget **));

  for(i = 0; i < toggle_combo->nrows; i++){

    toggle_combo->button[i] = (GtkWidget **)g_malloc(toggle_combo->ncols*sizeof(GtkWidget *));

    for(j = 0; j < toggle_combo->ncols; j++){

        toggle_combo->button[i][j] = gtk_toggle_button_new();
        gtk_button_set_relief(GTK_BUTTON(toggle_combo->button[i][j]),
                              GTK_RELIEF_NONE);
        gtk_table_attach (GTK_TABLE(toggle_combo->table), 
                          toggle_combo->button[i][j],
                          j, j+1, i, i+1, GTK_SHRINK, GTK_SHRINK, 0, 0);

        gtk_widget_set_size_request(toggle_combo->button[i][j], 24, 24);
        gtk_widget_show(toggle_combo->button[i][j]); 
        g_signal_connect (GTK_OBJECT (toggle_combo->button[i][j]), "toggled",
		            (void *) gtk_toggle_combo_update, 
                            toggle_combo);

    }
  }

  gtk_container_add(GTK_CONTAINER(GTK_COMBO_BUTTON(toggle_combo)->frame), 
                    toggle_combo->table);
  gtk_widget_show(toggle_combo->table);

  g_signal_connect (GTK_OBJECT (combo->button), "clicked",
	              (void *) gtk_toggle_combo_update, 
                      toggle_combo);

  gtk_toggle_combo_update(NULL, toggle_combo);

}


GType
gtk_toggle_combo_get_type ()
{
  static GType toggle_combo_type = 0;

  if (!toggle_combo_type)
    {
      toggle_combo_type = g_type_register_static_simple (
		gtk_combo_button_get_type (),
		"GtkToggleCombo",sizeof (GtkToggleComboClass),
		(GClassInitFunc) gtk_toggle_combo_class_init,
		sizeof (GtkToggleCombo),
		(GInstanceInitFunc) gtk_toggle_combo_init,
		0);
    }
  return toggle_combo_type;
}

/**
 * gtk_toggle_combo_new:
 * @nrows: number of rows
 * @ncols: number of columns
 * 
 * Creates a new #GtkToggleCombo widget with @nrows rows and @ncols columns.
 *  
 * Returns: the newly-created #GtkToggleCombo widget.
 */
GtkWidget *
gtk_toggle_combo_new (gint nrows, gint ncols)
{
  GtkWidget *widget;

  widget = gtk_widget_new (gtk_toggle_combo_get_type (), NULL);

  gtk_toggle_combo_construct(GTK_TOGGLE_COMBO(widget), nrows, ncols);

  return(widget);
}

/**
 * gtk_toggle_combo_construct:
 * @toggle_combo: a #GtkToggleCombo
 * @nrows: number of rows
 * @ncols: number of columns
 * 
 * Initializes the #GtkToggleCombo with @nrows rows and @ncols columns.
 */
void
gtk_toggle_combo_construct(GtkToggleCombo *toggle_combo, gint nrows, gint ncols)
{
  toggle_combo->default_flag = FALSE;

  toggle_combo->nrows = nrows;
  toggle_combo->ncols = ncols;

  gtk_toggle_combo_create_buttons(GTK_WIDGET(toggle_combo));
}

/**
 * gtk_toggle_combo_get_nrows:
 * @combo: a #GtkToggleCombo
 * 
 * Get the number of rows in #GtkToggleCombo.
 *  
 * Returns: number of rows
 */
gint
gtk_toggle_combo_get_nrows(GtkToggleCombo *combo)
{
  return (combo->nrows);
}

/**
 * gtk_toggle_combo_get_ncols:
 * @combo: a #GtkToggleCombo
 * 
 * Get the number of columns in #GtkToggleCombo.
 *  
 * Returns: number of columns
 */
gint
gtk_toggle_combo_get_ncols(GtkToggleCombo *combo)
{
  return (combo->ncols);
}

/**
 * gtk_toggle_combo_get_selection:
 * @combo: a #GtkToggleCombo
 * @row: number of row
 * @col: number of column
 * 
 * Get the current selection(row,col) in #GtkToggleCombo.
 */
void
gtk_toggle_combo_get_selection(GtkToggleCombo *combo, gint *row, gint *col)
{
  *col = combo->column;
  *row = combo->row;
}

/**
 * gtk_toggle_combo_select:
 * @toggle_combo: a #GtkToggleCombo
 * @new_row: number of row
 * @new_col: number of column
 * 
 * Select the cell(row,col) from #GtkToggleCombo. 
 */
void
gtk_toggle_combo_select (GtkToggleCombo *toggle_combo, 
                         gint new_row, gint new_col)
{
  gint row, column;

  row = toggle_combo->row;
  column = toggle_combo->column;

  if(row >= 0 && column >= 0){
     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_combo->button[row][column]), FALSE);
     gtk_widget_queue_draw(toggle_combo->button[row][column]);
  }

  toggle_combo->row = new_row;
  toggle_combo->column = new_col;

  if(new_row >= 0 && new_col >= 0){
     gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_combo->button[new_row][new_col]), TRUE);
     gtk_widget_queue_draw(toggle_combo->button[new_row][new_col]);
  }

  g_signal_emit (GTK_OBJECT(toggle_combo), 
                   toggle_combo_signals[CHANGED], 0,
                   new_row, new_col);
}
