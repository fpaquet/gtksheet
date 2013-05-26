/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Tristan Van Berkom.
 *
 * This library is free software; you can redistribute it and/or it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Authors:
 *   Tristan Van Berkom <tvb@gnome.org>
 *   Fredy Paquet <fredy@opag.ch> (adopted for GtkSheet)
 */

#include "config.h"

#include <gladeui/glade.h>
#include <glib/gi18n-lib.h>
#include <gdk/gdkkeysyms.h>

#include "glade-gtksheet-editor.h"
#include "gtkextra/gtksheet.h"

#undef GTK_SHEET_EDITOR_DEBUG

#ifdef DEBUG
#    define GTK_SHEET_EDITOR_DEBUG 1  /* define to activate debug output */
#endif

#ifdef GTK_SHEET_EDITOR_DEBUG
#    define GTK_SHEET_EDITOR_DEBUG_CHILDREN 0
#    define GTK_SHEET_EDITOR_DEBUG_LAYOUT 0
#endif

static void glade_sheet_editor_finalize(GObject *object);
static void glade_sheet_editor_editable_init(GladeEditableIface *iface);
static void glade_sheet_editor_realize(GtkWidget *widget);
static void glade_sheet_editor_grab_focus(GtkWidget *widget);

G_DEFINE_TYPE_WITH_CODE (
    GladeSheetEditor, glade_sheet_editor, GTK_TYPE_HBOX,
    G_IMPLEMENT_INTERFACE (GLADE_TYPE_EDITABLE, glade_sheet_editor_editable_init)
    );

static void
    glade_sheet_editor_class_init (GladeSheetEditorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    object_class->finalize = glade_sheet_editor_finalize;

    widget_class->realize = glade_sheet_editor_realize;
    widget_class->grab_focus = glade_sheet_editor_grab_focus;
}

static void
    glade_sheet_editor_init (GladeSheetEditor *self)
{
}

static void
    project_changed (GladeProject      *project,
                 GladeCommand      *command,
                 gboolean           execute,
                 GladeSheetEditor *view_editor)
{
    if (!GTK_WIDGET_MAPPED (view_editor)) return;

    /* Reload on all commands */
    glade_editable_load (GLADE_EDITABLE (view_editor), view_editor->loaded_widget);
}

static void
    project_finalized (GladeSheetEditor *view_editor, GladeProject *where_project_was)
{
    view_editor->loaded_widget = NULL;

    glade_editable_load (GLADE_EDITABLE (view_editor), NULL);
}

static GladeWidget *
    get_model_widget (GladeWidget *view)
{
    return NULL;
}

static void
    glade_sheet_editor_load (GladeEditable *editable, GladeWidget   *widget)
{
    GladeSheetEditor *view_editor = GLADE_SHEET_EDITOR (editable);
    GladeWidget *model_widget;

    /* Since we watch the project*/
    if (view_editor->loaded_widget)
    {
        g_signal_handlers_disconnect_by_func (G_OBJECT (view_editor->loaded_widget->project),
                                              G_CALLBACK (project_changed), view_editor);

        /* The widget could die unexpectedly... */
        g_object_weak_unref (G_OBJECT (view_editor->loaded_widget->project),
                             (GWeakNotify)project_finalized,
                             view_editor);
    }

    /* Mark our widget... */
    view_editor->loaded_widget = widget;

    if (view_editor->loaded_widget)
    {
        /* This fires for undo/redo */
        g_signal_connect (G_OBJECT (view_editor->loaded_widget->project), "changed",
                          G_CALLBACK (project_changed), view_editor);

        /* The widget/project could die unexpectedly... */
        g_object_weak_ref (G_OBJECT (view_editor->loaded_widget->project),
                           (GWeakNotify) project_finalized,
                           view_editor);
    }

    /* load the embedded editable... */
    if (view_editor->embed)
        glade_editable_load (GLADE_EDITABLE (view_editor->embed), widget);

    if (view_editor->embed_list_store && view_editor->embed_tree_store)
    {
        gtk_widget_hide (view_editor->no_model_message);
        gtk_widget_hide (view_editor->embed_list_store);
        gtk_widget_hide (view_editor->embed_tree_store);
        glade_editable_load (GLADE_EDITABLE (view_editor->embed_list_store), NULL);
        glade_editable_load (GLADE_EDITABLE (view_editor->embed_tree_store), NULL);

        /* Finalize safe code here... */
        if (widget && (model_widget = get_model_widget (widget)))
        {
            if (GTK_IS_LIST_STORE (model_widget->object))
            {
                gtk_widget_show (view_editor->embed_list_store);
                glade_editable_load (GLADE_EDITABLE (view_editor->embed_list_store), model_widget);
            }
            else if (GTK_IS_TREE_STORE (model_widget->object))
            {
                gtk_widget_show (view_editor->embed_tree_store);
                glade_editable_load (GLADE_EDITABLE (view_editor->embed_tree_store), model_widget);
            }
            else
                gtk_widget_show (view_editor->no_model_message);
        }
        else
            gtk_widget_show (view_editor->no_model_message);
    }
}

static void
    glade_sheet_editor_set_show_name (GladeEditable *editable, gboolean show_name)
{
    GladeSheetEditor *view_editor = GLADE_SHEET_EDITOR (editable);
    glade_editable_set_show_name (GLADE_EDITABLE (view_editor->embed), show_name);
}

static void
    glade_sheet_editor_editable_init (GladeEditableIface *iface)
{
    iface->load = glade_sheet_editor_load;
    iface->set_show_name = glade_sheet_editor_set_show_name;
}

static void
    glade_sheet_editor_finalize (GObject *object)
{
    GladeSheetEditor *view_editor = GLADE_SHEET_EDITOR (object);

    view_editor->embed_tree_store = NULL;
    view_editor->embed_list_store = NULL;
    view_editor->embed            = NULL;

    glade_editable_load (GLADE_EDITABLE (object), NULL);

    G_OBJECT_CLASS (glade_sheet_editor_parent_class)->finalize (object);
}


static void
    glade_sheet_editor_realize (GtkWidget *widget)
{
    GladeSheetEditor *view_editor = GLADE_SHEET_EDITOR (widget);

    GTK_WIDGET_CLASS (glade_sheet_editor_parent_class)->realize (widget);

    glade_editable_load (GLADE_EDITABLE (view_editor), view_editor->loaded_widget);
}

static void
    glade_sheet_editor_grab_focus (GtkWidget *widget)
{
    GladeSheetEditor *view_editor = GLADE_SHEET_EDITOR (widget);

    gtk_widget_grab_focus (view_editor->embed);
}

GtkWidget *
    glade_sheet_editor_new (GladeWidgetAdaptor *adaptor, GladeEditable *embed)
{
    GladeSheetEditor *view_editor;
    GtkWidget *vbox, *separator;
    gchar *str;

    g_return_val_if_fail (GLADE_IS_WIDGET_ADAPTOR (adaptor), NULL);
    g_return_val_if_fail (GLADE_IS_EDITABLE (embed), NULL);

    view_editor = g_object_new (GLADE_TYPE_SHEET_EDITOR, NULL);
    view_editor->embed = GTK_WIDGET (embed);

    /* Pack the parent on the left... */
    gtk_box_pack_start (GTK_BOX (view_editor), GTK_WIDGET (embed), TRUE, TRUE, 8);

    separator = gtk_vseparator_new ();
    gtk_box_pack_start (GTK_BOX (view_editor), separator, FALSE, FALSE, 0);

    /* ...and the vbox with datastore/label on the right */
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (view_editor), vbox, TRUE, TRUE, 8);

    str = g_strdup_printf ("<b>%s</b>", _("XXX Choose a Data Model and define some\n"
                                          "columns in the data store first"));
    view_editor->no_model_message = gtk_label_new (str);
    gtk_label_set_use_markup (GTK_LABEL (view_editor->no_model_message), TRUE);
    gtk_label_set_justify (GTK_LABEL (view_editor->no_model_message), GTK_JUSTIFY_CENTER);

    g_free (str);

    gtk_box_pack_start (GTK_BOX (vbox), view_editor->no_model_message, TRUE, TRUE, 0);

    view_editor->embed_list_store = (GtkWidget *) glade_widget_adaptor_create_editable(
        glade_widget_adaptor_get_by_type (GTK_TYPE_LIST_STORE), GLADE_PAGE_GENERAL);
    glade_editable_set_show_name (GLADE_EDITABLE (view_editor->embed_list_store), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), view_editor->embed_list_store, TRUE, TRUE, 0);

    view_editor->embed_tree_store = (GtkWidget *)glade_widget_adaptor_create_editable(
        glade_widget_adaptor_get_by_type (GTK_TYPE_TREE_STORE), GLADE_PAGE_GENERAL);
    glade_editable_set_show_name (GLADE_EDITABLE (view_editor->embed_tree_store), FALSE);
    gtk_box_pack_start (GTK_BOX (vbox), view_editor->embed_tree_store, TRUE, TRUE, 0);

    gtk_widget_show_all (GTK_WIDGET (view_editor));

    return GTK_WIDGET (view_editor);
}



static gboolean
    glade_gtk_sheet_layout_move_child (GladeBaseEditor *editor, 
                                       GladeWidget *gparent,
                                       GladeWidget *gchild,
                                       gpointer data)
{   
    GObject *parent = glade_widget_get_object (gparent);
    GObject *child  = glade_widget_get_object (gchild);
    GList list   = { 0,};

#if GTK_SHEET_EDITOR_DEBUG_LAYOUT > 0
    g_debug("glade_gtk_sheet_layout_move_child: called");
#endif

    if (GTK_IS_SHEET(parent) && !GTK_IS_SHEET_COLUMN (child)) return FALSE;
    if (GTK_IS_CELL_LAYOUT(parent) && !GTK_IS_CELL_RENDERER (child)) return FALSE;
    if (GTK_IS_CELL_RENDERER(parent)) return FALSE;

    if (gparent != glade_widget_get_parent (gchild))
    {
        list.data = gchild;
        glade_command_dnd(&list, gparent, NULL);
    }

    return TRUE;
}

static void
    glade_gtk_sheet_layout_child_selected (GladeBaseEditor *editor,
                                           GladeWidget *gchild,
                                           gpointer data)
{
    GObject *child = glade_widget_get_object (gchild);

#if GTK_SHEET_EDITOR_DEBUG_LAYOUT > 0
    g_debug("glade_gtk_sheet_layout_child_selected: called");
#endif

    glade_base_editor_add_label (editor, GTK_IS_SHEET_COLUMN (child) ? 
                                 _("GtkSheet Column") : _("Unknown"));

    glade_base_editor_add_default_properties (editor, gchild);

    glade_base_editor_add_label (editor, GTK_IS_SHEET_COLUMN (child) ? 
                                 _("Properties") : _("Properties and Attributes"));

    glade_base_editor_add_editable (editor, gchild, GLADE_PAGE_GENERAL);

    if (GTK_IS_CELL_RENDERER (child))
    {
        glade_base_editor_add_label (editor, _("Common Properties and Attributes"));
        glade_base_editor_add_editable (editor, gchild, GLADE_PAGE_COMMON);
    }
}


static gchar *
    glade_gtk_sheet_layout_get_display_name (GladeBaseEditor *editor,
                                         GladeWidget *gchild,
                                         gpointer user_data)
{
    GObject *child = glade_widget_get_object (gchild);
    gchar *name;

    if (GTK_IS_TREE_VIEW_COLUMN (child))
        glade_widget_property_get (gchild, "title", &name);
    else
        name = gchild->name;

#if GTK_SHEET_EDITOR_DEBUG_LAYOUT > 0
    g_debug("glade_gtk_sheet_layout_get_display_name: called <%s>", name ? name : "NULL");
#endif

    return g_strdup (name);
}


static void
    glade_gtk_sheet_launch_editor(GObject  *sheet)
{
    GladeWidget *widget = glade_widget_get_from_gobject (sheet);
    GladeBaseEditor *editor;
    GladeEditable *sheet_editor;
    GtkWidget *window;

    sheet_editor = glade_widget_adaptor_create_editable (widget->adaptor, GLADE_PAGE_GENERAL);
    sheet_editor = (GladeEditable *) glade_sheet_editor_new (widget->adaptor, sheet_editor);

    /* Editor */
    editor = glade_base_editor_new (sheet, sheet_editor,
                                    _("Column"), G_TYPE_SHEET_COLUMN,
                                    NULL);

/*
    glade_base_editor_append_types (editor, GTK_TYPE_TREE_VIEW_COLUMN,             
                                    _("Text"), GTK_TYPE_CELL_RENDERER_TEXT,        
                                    _("Accelerator"), GTK_TYPE_CELL_RENDERER_ACCEL,
                                    _("Combo"), GTK_TYPE_CELL_RENDERER_COMBO,      
                                    _("Spin"),  GTK_TYPE_CELL_RENDERER_SPIN,       
                                    _("Pixbuf"), GTK_TYPE_CELL_RENDERER_PIXBUF,    
                                    _("Progress"), GTK_TYPE_CELL_RENDERER_PROGRESS,
                                    _("Toggle"), GTK_TYPE_CELL_RENDERER_TOGGLE,    
                                    NULL);                                         
*/

    g_signal_connect (editor, "get-display-name", 
                      G_CALLBACK (glade_gtk_sheet_layout_get_display_name), NULL);
    g_signal_connect (editor, "child-selected", 
                      G_CALLBACK (glade_gtk_sheet_layout_child_selected), NULL);
    g_signal_connect (editor, "move-child", 
                      G_CALLBACK (glade_gtk_sheet_layout_move_child), NULL);

    gtk_widget_show (GTK_WIDGET (editor));

    window = glade_base_editor_pack_new_window (editor, _("GtkSheet Editor"), NULL);
    gtk_widget_show (window);
}

void
    glade_gtk_sheet_action_activate (GladeWidgetAdaptor *adaptor,
                                     GObject *object,
                                     const gchar *action_path)
{
    if (strcmp (action_path, "launch_editor") == 0)
    {
        glade_gtk_sheet_launch_editor (object);
    }
    else
        GWA_GET_CLASS (GTK_TYPE_CONTAINER)->action_activate (adaptor,
                                                             object,
                                                             action_path);
}


/**
 * Glade-3 - child widget adaptor interface
 *
 */

extern void
    gtk_sheet_buildable_add_child_internal(GtkSheet *sheet, GtkSheetColumn *child, char *name);

/*
static void                                                              
    gtk_sheet_children_callback (GtkWidget *widget, gpointer client_data)
{                                                                        
    GList **children;                                                    
                                                                         
    children = (GList**) client_data;                                    
    *children = g_list_prepend (*children, widget);                      
}                                                                        
*/

GList *
    glade_gtk_sheet_get_children(GladeWidgetAdaptor *adaptor, GtkContainer *container)
{
    GList *children = NULL;
    GtkSheet *sheet;
    gint col;

#if GTK_SHEET_EDITOR_DEBUG_CHILDREN > 0
    g_debug("glade_gtk_sheet_get_children: called");
#endif

    g_return_val_if_fail (GTK_IS_SHEET (container), NULL);

    sheet = GTK_SHEET(container);

/*
    gtk_container_forall (container,                  
                          gtk_sheet_children_callback,
                          &children);                 
*/

    for (col=0; col<=sheet->maxcol; col++)
    {
        children = g_list_append(children, sheet->column[col]);
    }

    /* Is the children list already reversed? */
    return children;
}

void
    glade_gtk_sheet_add_child(GladeWidgetAdaptor *adaptor, GObject *object, GObject *child)
{
    GtkSheet *sheet;
    GtkSheetColumn *newcol;
    GladeWidget *child_widget = glade_widget_get_from_gobject (child);
    const gchar *name = glade_widget_get_name(child_widget);

    g_return_if_fail (GTK_IS_SHEET (object));
    g_return_if_fail (GTK_IS_WIDGET (child));

#if GTK_SHEET_EDITOR_DEBUG_CHILDREN > 0
    g_debug("glade_gtk_sheet_add_child: %s %d", 
            name ? name : "NULL",
            GLADE_IS_WIDGET(child)
            );
#endif

    sheet = GTK_SHEET(object);
    newcol = GTK_SHEET_COLUMN(child);

    gtk_sheet_buildable_add_child_internal(sheet, newcol, name);
}

void
    glade_gtk_sheet_remove_child(GladeWidgetAdaptor *adaptor, GObject *object, GObject *child)
{
    gint col;
    GtkSheet *sheet;
    GtkSheetColumn *oldcol;

#if GTK_SHEET_EDITOR_DEBUG_CHILDREN > 0
    g_debug("glade_gtk_sheet_remove_child: called");
#endif

    g_return_if_fail (GTK_IS_SHEET (object));
    g_return_if_fail (GTK_IS_WIDGET (child));

    sheet = GTK_SHEET(object);
    oldcol = GTK_SHEET_COLUMN(child);

    for (col=0; col<=sheet->maxcol; col++)
    {
        if (oldcol == sheet->column[col])
        {
            gtk_sheet_delete_columns(sheet, col, 1);
            return;
        }
    }
    g_warning("glade_gtk_sheet_remove_child: couldn't remove child %p", child);
}

void
    glade_gtk_sheet_replace_child(
        GladeWidgetAdaptor *adaptor,
        GObject *object,
        GObject *old_obj,
        GObject *new_obj)
{
    gint col;
    GtkSheet *sheet;
    GtkSheetColumn *oldcol, *newcol;

#if GTK_SHEET_EDITOR_DEBUG_CHILDREN > 0
    g_debug("glade_gtk_sheet_replace_child: called %p -> %p", old_obj, new_obj);
#endif

    g_return_if_fail (GTK_IS_SHEET (object));
    g_return_if_fail (GTK_IS_WIDGET (old_obj));

    if (GLADE_IS_PLACEHOLDER (new_obj))
    {
        glade_gtk_sheet_remove_child (adaptor, object, old_obj);
        return;
    }

    g_return_if_fail (GTK_IS_WIDGET (new_obj));

    sheet = GTK_SHEET(object);
    oldcol = GTK_SHEET_COLUMN(old_obj);
    newcol = GTK_SHEET_COLUMN(new_obj);

    for (col=0; col<=sheet->maxcol; col++)
    {
        if (oldcol == sheet->column[col])
        {
            g_object_unref(sheet->column[col]);
            sheet->column[col] = newcol;
            return;
        }
    }
    g_warning("glade_gtk_sheet_replace_child: couldn't replace child %p by %p", old_obj, new_obj);
}

#if 0
void glade_gtk_sheet_column_set_property(
    GladeWidgetAdaptor *adaptor,
    GObject *object,
    const gchar *property_name,
    const GValue *value)
{
#ifdef GTK_SHEET_EDITOR_DEBUG
    g_debug("glade_gtk_sheet_column_set_property: called %p %s", object, property_name);
#endif
}

void glade_gtk_sheet_column_get_property(
    GladeWidgetAdaptor *adaptor,
    GObject *object,
    const gchar *property_name,
    GValue *value)
{
#ifdef GTK_SHEET_EDITOR_DEBUG
    g_debug("glade_gtk_sheet_column_get_property: called %p %s", object, property_name);
#endif
}
#endif
