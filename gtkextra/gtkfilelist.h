/* gtkfilelist - gtkfilelist widget for gtk+
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


#ifndef __GTK_FILE_LIST_H__
#define __GTK_FILE_LIST_H__


#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "gtkiconlist.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_FILE_LIST                  (gtk_file_list_get_type ())
#define GTK_FILE_LIST(obj)                  (GTK_CHECK_CAST ((obj), GTK_TYPE_FILE_LIST, GtkFileList))
#define GTK_FILE_LIST_CLASS(klass)          (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_FILE_LIST, GtkFileListClass))
#define GTK_IS_FILE_LIST(obj)               (GTK_CHECK_TYPE ((obj), GTK_TYPE_FILE_LIST))
#define GTK_IS_FILE_LIST_CLASS(klass)       (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FILE_LIST))

enum 
{
      GTK_FILE_LIST_FOLDER,
      GTK_FILE_LIST_FILE,
      GTK_FILE_LIST_HTML,
      GTK_FILE_LIST_TEXT,
      GTK_FILE_LIST_DOC,
      GTK_FILE_LIST_PS,
      GTK_FILE_LIST_PDF,
      GTK_FILE_LIST_C,
      GTK_FILE_LIST_CPP,
      GTK_FILE_LIST_H,
      GTK_FILE_LIST_F,
      GTK_FILE_LIST_JAVA,
      GTK_FILE_LIST_EXEC,
      GTK_FILE_LIST_IMG,
      GTK_FILE_LIST_ARCH,
      GTK_FILE_LIST_PKG,
      GTK_FILE_LIST_DEB,
      GTK_FILE_LIST_RPM,
      GTK_FILE_LIST_CAT,
      GTK_FILE_LIST_SOUND,
      GTK_FILE_LIST_MOVIE,
      GTK_FILE_LIST_CORE,
};

enum
{
      GTK_FILE_LIST_SORT_NAME,
      GTK_FILE_LIST_SORT_TYPE,
};

typedef struct _GtkFileList       GtkFileList;
typedef struct _GtkFileListItem   GtkFileListItem;
typedef struct _GtkFileListType   GtkFileListType;
typedef struct _GtkFileListClass  GtkFileListClass;

struct _GtkFileListType
{
  gchar *extension;
  gint type;
};

struct _GtkFileListItem
{
  gchar *file_name;
  gint type;
  gint is_dir;
  gint is_link;
};

struct _GtkFileList
{
  GtkIconList iconlist;

  gint sort_mode;
  gchar *filter;

  gboolean show_folders;
  gboolean show_hidden;

  gchar *path;

  GList *pixmaps;

  GList *types;
  gint ntypes;
};

struct _GtkFileListClass
{
  GtkIconListClass parent_class;

};


GtkType    	gtk_file_list_get_type       (void);
GtkWidget* 	gtk_file_list_new            (guint icon_width, 
                                              gint mode,
                                              const gchar *path);
void 		gtk_file_list_construct      (GtkFileList *file_list,
                                              guint icon_width,
                                              gint mode,
                                              const gchar *path);
void 		gtk_file_list_set_filter     (GtkFileList *file_list, 
                                              const gchar *filter);
gboolean 	gtk_file_list_open_dir       (GtkFileList *file_list, 
                                              const gchar *path);
const gchar*    gtk_file_list_get_path       (GtkFileList *file_list);
const gchar*    gtk_file_list_get_filename   (GtkFileList *file_list);
gint            gtk_file_list_get_filetype   (GtkFileList *file_list);
gint            gtk_file_list_add_type       (GtkFileList *file_list,
					      const gchar **pixmap_data);
gint            gtk_file_list_add_type_with_pixmap
                                             (GtkFileList *file_list,
                                              GdkPixmap *pixmap,
                                              GdkBitmap *mask);
void            gtk_file_list_add_type_filter(GtkFileList *file_list,
					      gint type,
					      const gchar *filter);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_FILE_LIST_H__ */
