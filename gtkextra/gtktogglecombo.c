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

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <gtk/gtkarrow.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktable.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkeventbox.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkframe.h>
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

  parent_class = gtk_type_class (gtk_hbox_get_type ());
  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  object_class->destroy = gtk_toggle_combo_destroy;

  toggle_combo_signals[CHANGED]=gtk_signal_new("changed",
                                      GTK_RUN_FIRST,
                                      GTK_CLASS_TYPE(object_class),
                                      GTK_SIGNAL_OFFSET(GtkToggleComboClass,
                                      changed),
                                      gtkextra_VOID__INT_INT,
                                      GTK_TYPE_NONE,  
                                      2, GTK_TYPE_INT, GTK_TYPE_INT);

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
      if(GTK_WIDGET_HAS_FOCUS(toggle_combo->button[i][j])){
            focus_row=i;
            focus_col=j;
      }
      if(toggle_combo->button[i][j]->state==GTK_STATE_ACTIVE){
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
      
      gtk_signal_emit (GTK_OBJECT(toggle_combo), toggle_combo_signals[CHANGED],
                       new_row, new_col);

  }

  if(!new_selection && row >= 0 && column >= 0){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_combo->button[row][column]), TRUE);
          gtk_widget_queue_draw(toggle_combo->button[row][column]);

          gtk_signal_emit (GTK_OBJECT(toggle_combo), 
                           toggle_combo_signals[CHANGED],
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

        gtk_widget_set_usize(toggle_combo->button[i][j], 24, 24);
        gtk_widget_show(toggle_combo->button[i][j]); 
        gtk_signal_connect (GTK_OBJECT (toggle_combo->button[i][j]), "toggled",
		            (GtkSignalFunc) gtk_toggle_combo_update, 
                            toggle_combo);

    }
  }

  gtk_container_add(GTK_CONTAINER(GTK_COMBO_BUTTON(toggle_combo)->frame), 
                    toggle_combo->table);
  gtk_widget_show(toggle_combo->table);

  gtk_signal_connect (GTK_OBJECT (combo->button), "clicked",
	              (GtkSignalFunc) gtk_toggle_combo_update, 
                      toggle_combo);

  gtk_toggle_combo_update(NULL, toggle_combo);

}


GtkType
gtk_toggle_combo_get_type ()
{
  static GtkType toggle_combo_type = 0;

  if (!toggle_combo_type)
    {
      GtkTypeInfo toggle_combo_info =
      {
	"GtkToggleCombo",
	sizeof (GtkToggleCombo),
	sizeof (GtkToggleComboClass),
	(GtkClassInitFunc) gtk_toggle_combo_class_init,
	(GtkObjectInitFunc) gtk_toggle_combo_init,
	NULL,
	NULL,
	(GtkClassInitFunc) NULL,
      };
      toggle_combo_type = gtk_type_unique (gtk_combo_button_get_type (), &toggle_combo_info);
    }
  return toggle_combo_type;
}

GtkWidget *
gtk_toggle_combo_new (gint nrows, gint ncols)
{
  GtkWidget *widget;

  widget = gtk_type_new (gtk_toggle_combo_get_type ());

  gtk_toggle_combo_construct(GTK_TOGGLE_COMBO(widget), nrows, ncols);

  return(widget);
}

void
gtk_toggle_combo_construct(GtkToggleCombo *toggle_combo, gint nrows, gint ncols)
{
  toggle_combo->default_flag = FALSE;

  toggle_combo->nrows = nrows;
  toggle_combo->ncols = ncols;

  gtk_toggle_combo_create_buttons(GTK_WIDGET(toggle_combo));
}

gint
gtk_toggle_combo_get_nrows(GtkToggleCombo *combo)
{
  return (combo->nrows);
}

gint
gtk_toggle_combo_get_ncols(GtkToggleCombo *combo)
{
  return (combo->ncols);
}

void
gtk_toggle_combo_get_selection(GtkToggleCombo *combo, gint *row, gint *col)
{
  *col = combo->column;
  *row = combo->row;
}

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

  gtk_signal_emit (GTK_OBJECT(toggle_combo), 
                   toggle_combo_signals[CHANGED],
                   new_row, new_col);
}
