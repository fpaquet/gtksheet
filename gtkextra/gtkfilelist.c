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

/**
 * SECTION: gtkfilelist
 * @short_description: A file list widget fot GTK
 *
 * It is a GtkIconList subclass that displays the contents of a given directory using fancy icons for different types of files
 */

#include "config.h"
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef HAVE_FNMATCH_H
#include <fnmatch.h>
#else
static int
fnmatch(const char* pattern, const char* s, int m)
{
  /* very simple replacement, should work here for now */
  int slen = strlen(s);
  int plen = strlen(pattern);

  if (strstr(pattern, "*.*")) {
    return 0;
  } else if (*pattern == '*') {
    return ((strstr(s, pattern+1) == s + slen - (plen - 1)) ? 0 : 1);
  } else if ('*' == pattern[plen]) {
    return ((0 == strncmp(s, pattern, plen - 1)) ? 0 : 1);
  } else if (NULL == strstr(pattern, "*") && NULL == strstr(pattern, "?")) {
    return ((0 == strcmp(s, pattern)) ? 0 : 1);
  } else {
    g_print("fnmatch(%s,%s,%d) ???\n", pattern, s, m);
    return 0;
  }
}
#endif

#include "gtkfilelist.h"
#include "gtkfileicons.h"

#ifdef G_OS_WIN32
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#define S_ISLNK(m) (0)
#elif defined(_WIN32)
#define S_ISLNK(m) (0)
#endif
#define lstat(f,s) stat(f,s)
#endif /* G_OS_WIN32 */

static void gtk_file_list_class_init          (GtkFileListClass *klass);
static void gtk_file_list_init                (GtkFileList      *file_list);
static void gtk_file_list_destroy             (GtkObject      *object);
static void gtk_file_list_realize             (GtkWidget *widget);
static gint sort_list			      (gpointer a, gpointer b);
extern gboolean check_dir_extra (gchar *dir_name, struct stat *result, gboolean *stat_subdirs);


static GtkIconListClass *parent_class = NULL;

static GtkFileListType default_types[] = {
                              {"*.cpp",GTK_FILE_LIST_CPP},
                              {"*.c",GTK_FILE_LIST_C},
                              {"*.h",GTK_FILE_LIST_H},
                              {"*.for",GTK_FILE_LIST_F},
                              {"*.f",GTK_FILE_LIST_F},
                              {"*.f77",GTK_FILE_LIST_F},
                              {"*.f90",GTK_FILE_LIST_F},
                              {"*.jar",GTK_FILE_LIST_JAVA},
                              {"*.java",GTK_FILE_LIST_JAVA},
                              {"*.j",GTK_FILE_LIST_JAVA},
                              {"*.mp3",GTK_FILE_LIST_SOUND},
                              {"*.wav",GTK_FILE_LIST_SOUND},
                              {"*.ps",GTK_FILE_LIST_PS},
                              {"*.pdf",GTK_FILE_LIST_PDF},
                              {"*.doc",GTK_FILE_LIST_DOC},
                              {"*.txt",GTK_FILE_LIST_TEXT},
                              {"README",GTK_FILE_LIST_TEXT},
                              {"TODO",GTK_FILE_LIST_TEXT},
                              {"NEWS",GTK_FILE_LIST_TEXT},
                              {"AUTHORS",GTK_FILE_LIST_TEXT},
                              {"COPYING",GTK_FILE_LIST_TEXT},
                              {"INSTALL",GTK_FILE_LIST_TEXT},
                              {"BUGS",GTK_FILE_LIST_TEXT},
                              {"*.xpm",GTK_FILE_LIST_IMG},
                              {"*.jpg",GTK_FILE_LIST_IMG},
                              {"*.jpeg",GTK_FILE_LIST_IMG},
                              {"*.gif",GTK_FILE_LIST_IMG},
                              {"*.tif",GTK_FILE_LIST_IMG},
                              {"*.tiff",GTK_FILE_LIST_IMG},
                              {"*.png",GTK_FILE_LIST_IMG},
                              {"*.tar",GTK_FILE_LIST_ARCH},
                              {"*.tar.bz2",GTK_FILE_LIST_ARCH},
                              {"*.bz2",GTK_FILE_LIST_ARCH},
                              {"*.zip",GTK_FILE_LIST_ARCH},
                              {"*.tar.gz",GTK_FILE_LIST_PKG},
                              {"*.tgz",GTK_FILE_LIST_PKG},
                              {"*.rpm",GTK_FILE_LIST_RPM},
                              {"*.deb",GTK_FILE_LIST_DEB},
                              {"*.htm",GTK_FILE_LIST_HTML},
                              {"*.html",GTK_FILE_LIST_HTML},
                              {"*.mpeg",GTK_FILE_LIST_MOVIE},
                              {"*.mpeg1",GTK_FILE_LIST_MOVIE},
                              {"*.mpeg2",GTK_FILE_LIST_MOVIE},
                              {"*.mpg",GTK_FILE_LIST_MOVIE},
                              {"*.mpg1",GTK_FILE_LIST_MOVIE},
                              {"*.mpg2",GTK_FILE_LIST_MOVIE},
                              {"*.avi",GTK_FILE_LIST_MOVIE},
                              {"*.mov",GTK_FILE_LIST_MOVIE},
                              {"core",GTK_FILE_LIST_CORE}, 
                              {NULL, 0}, 
                             };


GType
gtk_file_list_get_type (void)
{
  static GType file_list_type = 0;
  
  if (!file_list_type)
    {
      file_list_type = g_type_register_static_simple(
		gtk_icon_list_get_type(),
		"GtkFileList",
		sizeof (GtkFileListClass),
	        (GClassInitFunc) gtk_file_list_class_init,
		sizeof (GtkFileList),
		(GInstanceInitFunc) gtk_file_list_init,
		0);
    }
  
  return file_list_type;
}

/**
 * gtk_file_list_new:
 * @icon_width: the width of the icon
 * @mode:GTK_FILE_LIST_SORT_NAME , GTK_FILE_LIST_SORT_TYPE
 * @path: the path to the files to be be opened in #GtkFileList widget.
 * 
 * Create a newfile list widget.
 * Remark for the 2nd open file window you must use: gtk_icon_file_selection_show_tree(GTK_ICON_FILESEL(filesel), TRUE);
 *  
 * Returns: the newly-created #GtkFileList widget.
 */
GtkWidget*
gtk_file_list_new (guint icon_width, gint mode, const gchar *path)
{
  GtkWidget *widget;
  GtkFileList *file_list;

  widget = gtk_widget_new (gtk_file_list_get_type(), NULL);

  file_list = GTK_FILE_LIST(widget);

  gtk_file_list_construct(file_list, icon_width, mode, path);

  return widget;
}

/**
 * gtk_file_list_construct:
 * @file_list: #GtkFileList widget
 * @icon_width: the width of the icon
 * @mode:GTK_FILE_LIST_SORT_NAME , GTK_FILE_LIST_SORT_TYPE
 * @path: the path to the files to be be opened in #GtkFileList widget.
 * 
 * Initializes newfile list widget with specified values.
 */
void
gtk_file_list_construct(GtkFileList *file_list, 
                        guint icon_width, gint mode, const gchar *path)
{
  GtkIconList *icon_list;

  icon_list = GTK_ICON_LIST(file_list);

  icon_list->mode = mode;
  icon_list->icon_width = icon_width;
  icon_list->selection_mode = GTK_SELECTION_SINGLE;

  if(path)
     file_list->path = g_strdup(path);
  else
     file_list->path = g_strdup(G_DIR_SEPARATOR_S);

}

static void
gtk_file_list_class_init (GtkFileListClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;
  
  widget_class = (GtkWidgetClass*) klass;
  object_class = (GtkObjectClass*) klass;
  parent_class = g_type_class_ref (gtk_icon_list_get_type ());

  widget_class->realize = gtk_file_list_realize; 
  object_class->destroy = gtk_file_list_destroy; 
}

static void
gtk_file_list_destroy(GtkObject *object)
{
  GtkIconList *icon_list;
  GtkFileList *file_list;
  GtkIconListItem *item;
  GtkFileListItem *file_item;
  GList *list;

  file_list = GTK_FILE_LIST(object);
  icon_list = GTK_ICON_LIST(object);

  list = icon_list->icons;
  while(list){
    item = (GtkIconListItem *)list->data;
    file_item = (GtkFileListItem *)item->link;
    if(file_item->file_name) g_free(file_item->file_name);
    file_item->file_name = NULL;
    g_free(item->link);
    item->link = NULL;
    list = list->next;
  }

  list = file_list->types;
  while(list){
    GtkFileListType *type;
    type = (GtkFileListType *)list->data;
    if(type->extension) g_free(type->extension);
    type->extension = NULL;
    file_list->types = g_list_remove_link(file_list->types, list);
    g_list_free_1(list);
    list = file_list->types;
  }
  file_list->types = NULL;

  list = file_list->pixmaps;
  while(list){
    file_list->pixmaps = g_list_remove_link(file_list->pixmaps, list);
    gtk_widget_destroy(GTK_WIDGET(list->data));
    g_list_free_1(list);
    list = file_list->pixmaps;
  }
  file_list->pixmaps = NULL;


  g_free(GTK_FILE_LIST(object)->path); 
  GTK_FILE_LIST(object)->path = NULL; 

  g_free(GTK_FILE_LIST(object)->filter); 
  GTK_FILE_LIST(object)->filter = NULL; 

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
  (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);

}

static void
gtk_file_list_init (GtkFileList *file_list)
{
  GtkFileListType *types;
  gchar **pixmap_data[GTK_FILE_LIST_CORE+1];
  gint i;

  file_list->path = NULL;
  file_list->show_folders = TRUE;
  file_list->show_hidden = TRUE;
  file_list->sort_mode = GTK_FILE_LIST_SORT_TYPE;
  file_list->filter = NULL;

  GTK_ICON_LIST(file_list)->text_space = 150;

  GTK_ICON_LIST(file_list)->compare_func = (GCompareFunc)sort_list;

  pixmap_data[GTK_FILE_LIST_FILE] = file_xpm;
  pixmap_data[GTK_FILE_LIST_FOLDER] = folder_xpm;
  pixmap_data[GTK_FILE_LIST_EXEC] = exec_xpm;
  pixmap_data[GTK_FILE_LIST_CPP] = cpp_xpm;
  pixmap_data[GTK_FILE_LIST_C] = c_xpm;
  pixmap_data[GTK_FILE_LIST_H] = h_xpm;
  pixmap_data[GTK_FILE_LIST_F] = f_xpm;
  pixmap_data[GTK_FILE_LIST_JAVA] = java_xpm;
  pixmap_data[GTK_FILE_LIST_DOC] = doc_xpm;
  pixmap_data[GTK_FILE_LIST_SOUND] = sound_xpm;
  pixmap_data[GTK_FILE_LIST_PS] = ps_xpm;
  pixmap_data[GTK_FILE_LIST_PDF] = pdf_xpm;
  pixmap_data[GTK_FILE_LIST_TEXT] = text_xpm;
  pixmap_data[GTK_FILE_LIST_IMG] = image_xpm;
  pixmap_data[GTK_FILE_LIST_ARCH] = arch_xpm;
  pixmap_data[GTK_FILE_LIST_PKG] = package_xpm;
  pixmap_data[GTK_FILE_LIST_DEB] = deb_xpm;
  pixmap_data[GTK_FILE_LIST_RPM] = rpm_xpm;
  pixmap_data[GTK_FILE_LIST_HTML] = html_xpm;
  pixmap_data[GTK_FILE_LIST_MOVIE] = movie_xpm;
  pixmap_data[GTK_FILE_LIST_CAT] = cat_xpm;
  pixmap_data[GTK_FILE_LIST_CORE] = core_xpm;

  file_list->ntypes = 0;

  for(i = 0; i <= GTK_FILE_LIST_CORE; i++)
    gtk_file_list_add_type(file_list, (const gchar **)pixmap_data[i]);

  types = default_types;
  while(types->extension){
    gtk_file_list_add_type_filter(file_list, types->type, types->extension);
    types++;
  }
  
}

static gint
sort_list(gpointer a, gpointer b)
{
  GtkFileList *file_list;
  GtkIconListItem *itema;
  GtkIconListItem *itemb;
  GtkFileListItem *filea;
  GtkFileListItem *fileb;
  gint compare_value;

  itema = (GtkIconListItem *)a;
  itemb = (GtkIconListItem *)b;
  filea = (GtkFileListItem *)itema->link;
  fileb = (GtkFileListItem *)itemb->link;
  file_list = GTK_FILE_LIST(gtk_widget_get_parent(itema->entry));
  if(!file_list) return 0;

  switch(file_list->sort_mode){
    case GTK_FILE_LIST_SORT_TYPE:
       compare_value = filea->type - fileb->type;
       if(compare_value == 0)
              compare_value = strcmp(itema->label, itemb->label);
       break;
    case GTK_FILE_LIST_SORT_NAME:
    default:
       compare_value = strcmp(itema->label, itemb->label);
       if(filea->type == GTK_FILE_LIST_FOLDER ||
          fileb->type == GTK_FILE_LIST_FOLDER){ 
              compare_value = filea->type - fileb->type;
              if(compare_value == 0)
                 compare_value = strcmp(itema->label, itemb->label);
       }
       break;
  }

  return compare_value;
}

static void
gtk_file_list_realize(GtkWidget *widget)
{
  GtkFileList *file_list;

  GTK_WIDGET_CLASS(parent_class)->realize (widget);

  file_list = GTK_FILE_LIST(widget);
  gtk_file_list_open_dir(file_list, file_list->path); 
}

/**
 * gtk_file_list_open_dir:
 * @file_list: a #GtkFileList widget
 * @dir_path: path of directory to be opened in #GtkFileList widget
 * 
 * Opens the content of specified directory in #GtkFileList widget.
 *  
 * Returns: TRUE(succes) or FALSE(failure)
 */
gboolean 
gtk_file_list_open_dir(GtkFileList *file_list, const gchar *dir_path)
{
  DIR *dir;
  struct dirent *dirent;
  struct stat fileinfo, linkinfo; 
  GtkIconList *icon_list;
  GtkIconListItem *item;
  GtkFileListItem *file_item;
  GtkFileListType *file_type;
  GtkWidget *widget;
  gchar *full_name, *file;
  gchar *real_path;
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  GdkPixmap *l_pixmap;
  GdkBitmap *l_mask;
  GtkImage *icon;
  gint width, height;
  GdkGC *gc;
  GList *types, *files, *list;
  gint type;
  gboolean show_file;
  gboolean stat_subdirs=TRUE;
  gchar root[5], root1[5], root2[5], root3[5], root4[5];
  gchar *aux_path;

  widget = GTK_WIDGET(file_list);
  icon_list = GTK_ICON_LIST(widget);

  /* GET ABSOLUTE PATH */

  sprintf(root,"%s",G_DIR_SEPARATOR_S);
  sprintf(root1,"%s.",G_DIR_SEPARATOR_S);
  sprintf(root2,"%s..",G_DIR_SEPARATOR_S);
  sprintf(root3,"%s..%s",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  sprintf(root4,"%s.%s",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  if(dir_path){
     gint length;

     aux_path = g_strdup(dir_path);
     length = strlen(aux_path);

     if(strcmp(aux_path + length - 2, root1) == 0){
        if(length == 2) {
           g_free(aux_path);
           aux_path = g_strdup(root);
        } else {
           aux_path[length - 2] = '\0';
        }
     } else if(strcmp(aux_path + length - 3, root2) == 0){
        if(length == 3) {
           g_free(aux_path);
           aux_path = g_strdup(root);
        } else {
           gint i = length - 4;
           while(i >= 0){
              if(aux_path[i] == root[0]){ 
                   aux_path[i] = '\0';
                   break;
              }
              i--;
           }
        }
     } else if(strcmp(aux_path + length - 4, root3) == 0){
        if(length == 4) {
           g_free(aux_path);
           aux_path = g_strdup(root);
        } else {
           gint i = length - 5;
           while(i >= 0){
              if(aux_path[i] == root[0]){ 
                   aux_path[i] = '\0';
                   break;
              }
              i--;
           }
        }
     } else if(strcmp(aux_path + length - 3, root4) == 0){
        if(length == 3) {
           g_free(aux_path);
           aux_path = g_strdup(root);
        } else {
           aux_path[length - 3] = '\0';
        }
     } else if(strcmp(aux_path + length - 1, root) == 0){
       aux_path[length - 1] = '\0';
     }

     if(strlen(aux_path) == 0)
     {
       g_free(aux_path);
       aux_path = g_strdup(G_DIR_SEPARATOR_S);
     }
  }
  else
     aux_path = g_strdup(G_DIR_SEPARATOR_S);

  real_path = g_strdup(aux_path);
  g_free(aux_path);

  if((dir = opendir(real_path)) == NULL){
    g_warning("Can not open folder: %s",real_path);
    g_free(real_path);
    return FALSE;
  }

  if(!check_dir_extra(real_path,&fileinfo,&stat_subdirs)){
    closedir(dir);
    g_warning("Can not stat folder: %s",real_path);
    g_free(real_path);
    return FALSE;
  }
  
  if(file_list->path) 
      g_free(file_list->path);

  file_list->path = real_path;

  /* UPDATE ICON LIST */

  gtk_icon_list_freeze(icon_list);

  files = icon_list->icons;
  while(files){
    item = (GtkIconListItem *)files->data;
    file_item = (GtkFileListItem *)item->link;
    g_free(file_item->file_name);
    g_free(item->link);
    item->link = NULL;
    files = files->next;
  }

  gtk_icon_list_clear(icon_list);

/*
  gdk_threads_enter();
*/

  files = NULL;
  while((dirent=readdir(dir))!=NULL){
        full_name = g_strconcat(real_path,G_DIR_SEPARATOR_S,dirent->d_name,NULL);
        file = dirent->d_name;
        show_file = TRUE;
        if(stat_subdirs){
          if(0==stat(full_name, &fileinfo)) {
            if(S_ISDIR(fileinfo.st_mode))
            {
              type = GTK_FILE_LIST_FOLDER;
            }
            else
            {
              type = GTK_FILE_LIST_FILE;
              if(fileinfo.st_mode & 0111)
                type = GTK_FILE_LIST_EXEC;
            }
          }
          else {
            /* silently skip instead of warn. Chris Kuklewicz chrisk@mit.edu */
            /* g_warning("Can not stat path: %s",full_name); */
            continue;
          }
        } 
        else {
          type = GTK_FILE_LIST_FOLDER;
        }

        if(file_list->show_folders && type == GTK_FILE_LIST_FOLDER)
           show_file = TRUE;

        if(file[0] == '.'){
           if(file_list->show_hidden == TRUE)
             show_file = TRUE;
           else
             show_file = FALSE;
           if(file_list->show_folders && (strcmp(file,".") == 0 || strcmp(file,"..") == 0))
               show_file = TRUE;
        }
        if(strcmp(full_name, root1) == 0 || strcmp(full_name, root2) == 0)
               show_file = FALSE; 
        if(!file_list->show_folders && type == GTK_FILE_LIST_FOLDER)
             show_file = FALSE;
        if(file_list->filter && fnmatch(file_list->filter, file, (1 << 4)) != 0)
        {
           if(file_list->show_folders && type == GTK_FILE_LIST_FOLDER && show_file == TRUE)
             show_file = TRUE;
           else 
             show_file = FALSE;
        }
        

        if(show_file){
            file_item = (GtkFileListItem *)g_new(GtkFileListItem, 1);
            file_item->file_name = g_strdup(file);
            file_item->type = type;
  
            file_item->is_link = FALSE;
            if (stat_subdirs){
              if(0!=lstat(full_name, &linkinfo))
                g_warning("Can not resolve link: %s",full_name);
              if(S_ISLNK(linkinfo.st_mode))
                file_item->is_link = TRUE;
            }
            files = g_list_append(files, file_item);
        }

        g_free(full_name);
  }

  list = files;
  while(list){
       file_item = (GtkFileListItem *)list->data;
       type = file_item->type;

       types = file_list->types;
       while(types){
         file_type = (GtkFileListType *)types->data;
         if(fnmatch((gchar *)file_type->extension, file_item->file_name, (1 << 4)) == 0){
           type = file_type->type; 
           break;
         }
         types = types->next;
       }

       icon = (g_list_nth_data(file_list->pixmaps, type));
       file_item->type = type;

       gtk_image_get_pixmap(icon, &l_pixmap, &l_mask);
       gdk_pixmap_ref(l_pixmap);
       gdk_bitmap_ref(l_mask);
       item = gtk_icon_list_add_from_pixmap(icon_list,
                        l_pixmap,
                        l_mask,
                        file_item->file_name, file_item);

       if(file_item->is_link){
               pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL, 
                           gdk_colormap_get_system(),
                           &mask, NULL,
                           symlink_xpm);

       	       gtk_image_get_pixmap((GtkImage *)(item->pixmap), &l_pixmap, &l_mask);
               gdk_window_get_size(l_pixmap,
                            &width, &height);

               gc = gdk_gc_new(pixmap);
               gdk_draw_pixmap(l_pixmap,
                               gc,
                               pixmap,
                               0, 0,
                               width - 7, height - 7,
                               7, 7); 
               gdk_gc_unref(gc);
               gc = gdk_gc_new(mask);
               gdk_draw_pixmap(l_mask,
                               gc,
                               mask,
                               0, 0,
                               width - 7, height - 7,
                               7, 7); 
               gdk_gc_unref(gc);
  
               gdk_pixmap_unref(pixmap);
               gdk_bitmap_unref(mask);
       }

       list = list->next;
  }

  closedir(dir);
/*
  gdk_threads_leave();
*/
  gtk_icon_list_thaw(icon_list);

  g_list_free(files);
  return TRUE;
}

/**
 * gtk_file_list_set_filter:
 * @file_list: the #GtkFileList widget.
 * @filter: filter applied to files 
 * 
 * Sets a filter for files displayed in Open window. 
 */
void
gtk_file_list_set_filter(GtkFileList *filelist, const gchar *filter)
{
  filelist->filter = g_strdup(filter);
  gtk_file_list_open_dir(filelist, filelist->path);
}

/**
 * gtk_file_list_get_path:
 * @file_list: the #GtkFileList widget.
 * 
 * Get the path of the files shown in filelist
 *  
 * Returns: the path of files.
 */
const gchar *
gtk_file_list_get_path(GtkFileList *file_list)
{
  return(file_list->path);
}

/**
 * gtk_file_list_get_filename:
 * @file_list: the #GtkFileList widget.
 * 
 * Get the path of the files shown in filelist
 *  
 * Returns: the filename
 */
const gchar *
gtk_file_list_get_filename(GtkFileList *file_list)
{
  GList *list;
  gpointer selection=NULL;
  gchar *file;

  file=NULL;
  list = GTK_ICON_LIST(file_list)->selection;               

  if(list) selection=list->data;

  if(selection){
       selection = ((GtkIconListItem *)selection)->link;
       file = ((GtkFileListItem *)selection)->file_name;
  }
  return file;
}

/**
 * gtk_file_list_get_filetype:
 * @file_list: the #GtkFileList widget.
 *       GTK_FILE_LIST_FILE,
 *       GTK_FILE_LIST_HTML,
 *       GTK_FILE_LIST_TEXT,
 *       GTK_FILE_LIST_DOC,
 *       GTK_FILE_LIST_PDF,
 *       GTK_FILE_LIST_C,
 *       GTK_FILE_LIST_CPP,
 *       GTK_FILE_LIST_H,
 *       GTK_FILE_LIST_F,
 *       GTK_FILE_LIST_JAVA,
 *       GTK_FILE_LIST_EXEC,
 *       GTK_FILE_LIST_IMG,
 *       GTK_FILE_LIST_ARCH,
 *       GTK_FILE_LIST_PKG,
 *       GTK_FILE_LIST_DEB,
 *       GTK_FILE_LIST_RPM,
 *       GTK_FILE_LIST_CAT,
 *       GTK_FILE_LIST_SOUND,
 *       GTK_FILE_LIST_MOVIE,
 *       GTK_FILE_LIST_CORE)
 * 
 * Get the #GtkFileListType of selected file in File List.
 *  
 * Returns: The #GtkFileListType (   GTK_FILE_LIST_FOLDER,
 */
gint 
gtk_file_list_get_filetype(GtkFileList *file_list)
{
  GList *list;
  gpointer selection=NULL;
  gint file = -1;

  list = GTK_ICON_LIST(file_list)->selection;               

  if(list) selection=list->data;

  if(selection){
       selection = ((GtkIconListItem *)selection)->link;
       file = ((GtkFileListItem *)selection)->type;
  }
  return file;
}


/**
 * gtk_file_list_add_type:
 * @file_list: the #GtkFileList widget.
 * @pixmap_data: Pointer to a string containing the XPM data.(last argument of gdk_pixmap_create_from_xpm_d() ).
 * 
 * Add a new file type in a #GtkFileList structure. 
 *  
 * Returns: the identification number given to the added 
 * filetype 
 */
gint
gtk_file_list_add_type       (GtkFileList *file_list,
                              const gchar **pixmap_data)
{
  GdkPixmap *pixmap;
  GdkBitmap *mask;
  gint type;

  pixmap = gdk_pixmap_colormap_create_from_xpm_d(NULL,
                                                 gdk_colormap_get_system(),
                                                 &mask, NULL,
                                                 (gchar **)pixmap_data);

  type = gtk_file_list_add_type_with_pixmap(file_list, pixmap, mask);
  gdk_pixmap_unref(pixmap);
  gdk_bitmap_unref(mask);

  return type;
}

/**
 * gtk_file_list_add_type_with_pixmap:
 * @file_list: the #GtkFileList widget.
 * @pixmap: a #GdkPixmap object.
 * @mask: a #GdkBitmap mask.
 * 
 * Add a new file type in a #GtkFileList structure. 
 *  
 * Returns: the identification number given to the added 
 * filetype 
 */
gint
gtk_file_list_add_type_with_pixmap (GtkFileList *file_list,
                              GdkPixmap *pixmap, GdkBitmap *mask)
{
  GtkWidget *wpixmap;
  wpixmap = gtk_image_new_from_pixmap(pixmap, mask);

  file_list->pixmaps = g_list_append(file_list->pixmaps, wpixmap);
  file_list->ntypes++;
  return (file_list->ntypes - 1);
}

/**
 * gtk_file_list_add_type_filter:
 * @file_list: the #GtkFileList widget.
 * @type: file type.
 * @filter: filter for specified type.
 * 
 * Add a filter for specified file type in a #GtkFileList structure. 
 */
void
gtk_file_list_add_type_filter(GtkFileList *file_list,
                              gint type,
                              const gchar *filter)
{
  GtkFileListType *type_item;

  type_item = g_new0(GtkFileListType,1);
  type_item->type = type;
  type_item->extension = g_strdup(filter);

  file_list->types = g_list_append(file_list->types, type_item);
  return;
}
