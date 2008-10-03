/* gtkdirtree - gtkdirtree widget for gtk+
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


#ifndef __GTK_DIR_TREE_H__
#define __GTK_DIR_TREE_H__


#include <gdk/gdk.h>
#include <gtk/gtk.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_DIR_TREE                  (gtk_dir_tree_get_type ())
#define GTK_DIR_TREE(obj)                  (GTK_CHECK_CAST ((obj), GTK_TYPE_DIR_TREE, GtkDirTree))
#define GTK_DIR_TREE_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_DIR_TREE, GtkDirTreeClass))
#define GTK_IS_DIR_TREE(obj)               (GTK_CHECK_TYPE ((obj), GTK_TYPE_DIR_TREE))
#define GTK_IS_DIR_TREE_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DIR_TREE))

typedef struct _GtkDirTree       GtkDirTree;
typedef struct _GtkDirTreeNode   GtkDirTreeNode;
typedef struct _GtkDirTreeClass  GtkDirTreeClass;


struct _GtkDirTreeNode
{
  gboolean scanned;
  gchar *path;
};


struct _GtkDirTree
{
  GtkCTree ctree;

  gchar *local_hostname;

  gboolean show_hidden;

  GdkPixmap *my_pc;
  GdkPixmap *folder;
  GdkPixmap *ofolder;
  GdkPixmap *dennied;

  GdkBitmap *my_pc_mask;
  GdkBitmap *folder_mask;
  GdkBitmap *ofolder_mask;
  GdkBitmap *dennied_mask;
};

struct _GtkDirTreeClass
{
  GtkCTreeClass parent_class;

};


GtkType    gtk_dir_tree_get_type       (void);
GtkWidget* gtk_dir_tree_new            (void);

gint       gtk_dir_tree_open_dir(GtkDirTree *dir_tree, const gchar *path);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_DIR_TREE_H__ */


