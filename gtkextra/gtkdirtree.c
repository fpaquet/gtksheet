/* gtkdirtree - gtkdirtree widget for gtk+
 * Copyright 1999-2001  Adrian E. Feiguin <feiguin@ifir.edu.ar>
 *
 * check_dir - taken from gtk+ (gtkfilesel.c)
 * Copyright (C) 2001 Chris Kuklewicz <chrisk at mit.edu>
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

#include "config.h"
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef G_OS_WIN32
#define STRICT
#include <windows.h> /* GetLogicalDriveStrings */

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#define S_ISLNK(m) (0)
#elif defined(_WIN32)
#define S_ISLNK(m) (0)
#endif
#endif

#include <string.h>
#include "gtkdirtree.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

static void destroy_tree(gpointer data);
static void expand_tree(GtkCTree *ctree,GtkCTreeNode *parent_node, gpointer data);
static gboolean accept_dirname (char *dirname, gboolean show_hidden);
gboolean check_dir_extra (gchar *dir_name, struct stat *result, gboolean *stat_subdirs);

/* XPM */
static char * dennied_xpm[] = {
"16 16 9 1",
" 	c None",
".	c #000000",
"X	c #EFE8EF",
"o	c #FFF8CF",
"O	c #FFF890",
"+	c #CFC860",
"@	c #FF0000",
"#	c #FFC890",
"x	c #909000",
"                ",
"  xxxxx.        ",
" xXooOO .       ",
"x++++@@@@xxxx   ",
"xoo@@@@@@@@oO+. ",
"xoO@@OOOO@@O#+. ",
"xo@@OOOO@@@@O+. ",
"xo@@OOO@@O@@#+. ",
"xo@@OO@@O#@@O+. ",
"xo@@O@@O#O@@#+. ",
"xo@@@@O#O#@@#+. ",
"xo#@@O#O#@@##+. ",
"x++@@@@@@@@+++. ",
" ....@@@@...... ",
"                ",
"                "};

static char * folder_xpm[] = {
"16 16 8 1",
" 	c None",
".	c #909000",
"+	c #000000",
"@	c #EFE8EF",
"#	c #FFF8CF",
"$	c #FFF890",
"%	c #CFC860",
"&	c #FFC890",
"                ",
"  .....+        ",
" .@##$$.+       ",
".%%%%%%%......  ",
".###########$%+ ",
".#$$$$$$$$$$&%+ ",
".#$$$$$$$&$&$%+ ",
".#$$$$$$$$&$&%+ ",
".#$$$$$&$&$&$%+ ",
".#$$$$$$&$&$&%+ ",
".#$$$&$&$&$&&%+ ",
".#&$&$&$&$&&&%+ ",
".%%%%%%%%%%%%%+ ",
" ++++++++++++++ ",
"                ",
"                "};

/* XPM */
static char * ofolder_xpm[] = {
"16 16 12 1",
" 	c None",
".	c #808080",
"+	c #E0E0D0",
"@	c #4F484F",
"#	c #909000",
"$	c #FFF8EF",
"%	c #CFC860",
"&	c #003090",
"*	c #7F7800",
"=	c #FFC890",
"-	c #FFF890",
";	c #2F3000",
"        .       ",
"       .+@      ",
"   ###.$$+@     ",
"  #%%.$$$$+@    ",
"  #%.$$$&$$+@** ",
"  #.+++&+&+++@* ",
"############++@ ",
"#$$$$$$$$$=%#++@",
"#$-------=-=#+; ",
" #---=--=-==%#; ",
" #-----=-=-==#; ",
" #-=--=-=-=-=#; ",
"  #=-=-=-=-==#; ",
"  ############; ",
"   ;;;;;;;;;;;  ",
"                "};



/* XPM */
static char * mypc_xpm[] = {
"18 18 7 1",
" 	c None",
".	c #808080",
"+	c #C0C0C0",
"@	c #000000",
"#	c #FFFFFF",
"$	c #0000FF",
"%	c #FFFF00",
"                  ",
"                  ",
"                  ",
"     .........    ",
"    .++++++++.@   ",
"   .########..@   ",
"   .#$$$$$$#..@   ",
"   .#$%$$$$#..@   ",
"   .#$$$$$$#..@   ",
"   .#$$$$$$#..@   ",
"   .########.@.   ",
"   .@@@@@@@@@.+@  ",
"  .#########.+.@  ",
"  ..@.@.@.@.@..@  ",
" .+@+@+@+@+@+@@   ",
".###########+@    ",
"@@@@@@@@@@@@@     ",
"                  "};


static void gtk_dir_tree_class_init          (GtkDirTreeClass *klass);
static void gtk_dir_tree_init                (GtkDirTree      *dir_tree);


static GtkCTreeClass *parent_class = NULL;


GtkType
gtk_dir_tree_get_type (void)
{
  static GtkType dir_tree_type = 0;
  
  if (!dir_tree_type)
    {
      GtkTypeInfo dir_tree_info =
      {
	"GtkDirTree",
	sizeof (GtkDirTree),
	sizeof (GtkDirTreeClass),
	(GtkClassInitFunc) gtk_dir_tree_class_init,
	(GtkObjectInitFunc) gtk_dir_tree_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      dir_tree_type = gtk_type_unique (gtk_ctree_get_type(), &dir_tree_info);
    }
  
  return dir_tree_type;
}

GtkWidget*
gtk_dir_tree_new (void)
{
  GtkWidget *widget;

  widget = gtk_widget_new (gtk_dir_tree_get_type(), NULL);

  return widget;
}

static void
gtk_dir_tree_class_init (GtkDirTreeClass *klass)
{
  GtkWidgetClass *widget_class;
  GtkObjectClass *object_class;
  
  parent_class = gtk_type_class (gtk_ctree_get_type ());
  widget_class = (GtkWidgetClass*) klass;
  object_class = (GtkObjectClass *) klass;

}

static void
gtk_dir_tree_init (GtkDirTree *dir_tree)
{
  GtkCTreeNode *root_node,*mypc_node,*node;
  GtkDirTreeNode *dirnode;
  gchar *root_text=G_DIR_SEPARATOR_S,*node_text="dummy";
  gchar localhost[MAXHOSTNAMELEN];
  GtkWidget *widget;
  GdkColormap *colormap;
#ifdef G_OS_WIN32
  gchar  drives[128];
  gchar* drive;
#endif

  widget = GTK_WIDGET(dir_tree);
  colormap = gdk_colormap_get_system();

  dir_tree->show_hidden = TRUE;

  /* Get the local hostname. */
#ifndef G_OS_WIN32
  if ((gethostname (localhost, MAXHOSTNAMELEN) != 0) &&
      (getdomainname (localhost, MAXHOSTNAMELEN) != 0))
    strcpy (localhost, "LocalHost");
#else
    strcpy (localhost, "My PC");
#endif

  dir_tree->local_hostname = g_strdup(localhost);
  g_object_set(G_OBJECT(widget), "n_columns", 1, "tree_column", 0, NULL);


  gtk_clist_set_row_height (GTK_CLIST (dir_tree), 18);

  dir_tree->my_pc=gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap,
                                                       &dir_tree->my_pc_mask,
                                                       NULL,mypc_xpm);
  dir_tree->folder=gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap,
                                                       &dir_tree->folder_mask,
                                                       NULL,folder_xpm);
  dir_tree->ofolder=gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap,
                                                        &dir_tree->ofolder_mask,
                                                        NULL,ofolder_xpm);
  dir_tree->dennied=gdk_pixmap_colormap_create_from_xpm_d(NULL, colormap,
                                                        &dir_tree->dennied_mask,
                                                        NULL,dennied_xpm);

  gtk_clist_set_column_auto_resize(GTK_CLIST(dir_tree),0,TRUE);
  gtk_clist_set_selection_mode(GTK_CLIST(dir_tree),GTK_SELECTION_SINGLE);
  gtk_ctree_set_line_style(GTK_CTREE(dir_tree),GTK_CTREE_LINES_DOTTED);
  
  gtk_signal_connect(GTK_OBJECT(dir_tree),"tree_expand",GTK_SIGNAL_FUNC(expand_tree), NULL);

  mypc_node=gtk_ctree_insert_node(GTK_CTREE(dir_tree),NULL,NULL,&dir_tree->local_hostname,4,dir_tree->my_pc,dir_tree->my_pc_mask,dir_tree->my_pc,dir_tree->my_pc_mask,FALSE,FALSE);

  dirnode=g_malloc0(sizeof(GtkDirTreeNode));
  dirnode->path = dir_tree->local_hostname;
  gtk_ctree_node_set_row_data_full(GTK_CTREE(dir_tree),mypc_node,dirnode,destroy_tree);

#ifndef G_OS_WIN32
  root_node=gtk_ctree_insert_node(GTK_CTREE(dir_tree),mypc_node,NULL,&root_text,4,dir_tree->folder,dir_tree->folder_mask,dir_tree->ofolder,dir_tree->ofolder_mask,FALSE,FALSE);

  dirnode=g_malloc0(sizeof(GtkDirTreeNode));
  dirnode->path=g_strdup(G_DIR_SEPARATOR_S);
  gtk_ctree_node_set_row_data_full(GTK_CTREE(dir_tree),root_node,dirnode,destroy_tree);
  node=gtk_ctree_insert_node(GTK_CTREE(dir_tree),root_node,NULL,&node_text,4,NULL,NULL,NULL,NULL,TRUE,TRUE);
  gtk_ctree_expand(GTK_CTREE(dir_tree),mypc_node);

  gtk_ctree_select(GTK_CTREE(dir_tree),root_node);
#else
  /* On Windoze there isn't one unique root directory "/" but instead
   * there are logical drives a:, c: ... . Insert them into the dir_tree.
   */

  /* Get the Drives string */
  GetLogicalDriveStrings(sizeof(drives), drives);
  drive = drives;
  /* add an entry for every existing drive */
  while (*drive != '\0')
  {
    root_node=gtk_ctree_insert_node(GTK_CTREE(dir_tree),mypc_node,NULL,&drive,4,dir_tree->folder,dir_tree->folder_mask,dir_tree->ofolder,dir_tree->ofolder_mask,FALSE,FALSE);

    dirnode = g_malloc0(sizeof(GtkDirTreeNode));
    dirnode->path = g_strdup(drive);
    gtk_ctree_node_set_row_data_full(GTK_CTREE(dir_tree),root_node,dirnode,destroy_tree);
    node=gtk_ctree_insert_node(GTK_CTREE(dir_tree),root_node,NULL,&node_text, 4,NULL,NULL,NULL,NULL,TRUE,TRUE);
    drive += (strlen(drive) + 1);
  }

  gtk_ctree_expand(GTK_CTREE(dir_tree),mypc_node);
  gtk_ctree_select(GTK_CTREE(dir_tree),mypc_node);
#endif

}

static gboolean accept_dirname (char *dirname, gboolean show_hidden)
{
    if(dirname[0]!='.') return TRUE;
    if (show_hidden)
	if (strcmp(dirname, ".") && strcmp(dirname, ".."))
	    return TRUE;
    return FALSE;
}

static gboolean check_for_subdir(gchar *path, gboolean show_hidden)
{
  DIR *dir;
  struct dirent *dirent;
  struct stat statbuf;
  gchar *npath;

  if((dir=opendir(path))!=NULL)
  {
    while((dirent=readdir(dir))!=NULL)
    {
	if(accept_dirname(dirent->d_name,show_hidden))
      {
#ifndef G_OS_WIN32
        npath=g_strconcat(path,dirent->d_name,G_DIR_SEPARATOR_S,NULL);
#else
        /* the M$ stat is somewhat pedantic ... */
        npath=g_strconcat(path,dirent->d_name,NULL);
#endif
        if(0!=stat(npath,&statbuf))
        {
          g_free(npath);
          continue;
        }
        g_free(npath);
        if(S_ISDIR(statbuf.st_mode))
        {
          closedir(dir);
          return TRUE;
        }
      }
    }
    closedir(dir);
  }
/*
  else
    g_warning("check_for_subdir: opendir(%s) failed", path);
*/

  return FALSE;
}

static void 
destroy_tree(gpointer data)
{

  GtkDirTreeNode *node;
  node = data;
  g_free(node->path);
  g_free(node);
}

static void 
expand_tree(GtkCTree *ctree,GtkCTreeNode *parent_node, gpointer data)
{
  DIR *dir;
  struct dirent *dirent;
  gchar *path,*text,*dummy="dummy";
  struct stat statbuf;
  GtkCTreeNode *node,*sub_node;
  GtkDirTreeNode *parent_dirnode,*dirnode;
  gboolean has_subdir=FALSE;
  gboolean show_hidden;
  gboolean stat_subdirs=TRUE;
  gboolean can_open_subdir;
  GtkDirTree *dir_tree;
  GtkWidget *widget;

  widget = GTK_WIDGET(ctree);
  dir_tree = GTK_DIR_TREE(widget);
  show_hidden = dir_tree->show_hidden;

  parent_dirnode=gtk_ctree_node_get_row_data(GTK_CTREE(widget),parent_node);

  if(parent_dirnode->path == dir_tree->local_hostname) return;

  if(!parent_dirnode->scanned)
  {
    gtk_clist_freeze(GTK_CLIST(widget));
    node=gtk_ctree_find_by_row_data(GTK_CTREE(widget),parent_node,NULL);
    gtk_ctree_remove_node(GTK_CTREE(widget),node);
    if((dir=opendir(parent_dirnode->path))!=NULL)
    {
      if(!check_dir_extra(parent_dirnode->path,&statbuf,&stat_subdirs))
      {
        closedir(dir);
        gtk_clist_thaw(GTK_CLIST(widget));
        return;
      }
      while((dirent=readdir(dir))!=NULL)
      {
        path=g_strconcat(parent_dirnode->path,dirent->d_name,NULL);
        if (stat_subdirs)
        {
          if(0!=stat(path,&statbuf))
          {
            g_free(path);
            continue;
          }
        }
        if(((!stat_subdirs)&&accept_dirname(dirent->d_name,show_hidden)) ||
           (S_ISDIR(statbuf.st_mode)&&accept_dirname(dirent->d_name,show_hidden)))
        {
          DIR *dir_sub;
          dirnode=g_malloc0(sizeof(GtkDirTreeNode));
          dirnode->path=g_strconcat(path,G_DIR_SEPARATOR_S,NULL);
          text=dirent->d_name;
          if (stat_subdirs)
          {
            if(check_for_subdir(dirnode->path,show_hidden))
               has_subdir=TRUE;
            else
               has_subdir=FALSE;
            dir_sub = opendir(dirnode->path);
            if( dir_sub != NULL )
            {
              closedir(dir_sub);
              can_open_subdir=TRUE;
            }
            else
            {
              can_open_subdir=FALSE;
            }
          }
          else
          {
            has_subdir=TRUE;
            can_open_subdir=TRUE;
          }

          if(can_open_subdir)
          {
             node=gtk_ctree_insert_node(GTK_CTREE(widget),parent_node,NULL,&text,4,dir_tree->folder,dir_tree->folder_mask,dir_tree->ofolder,dir_tree->ofolder_mask,!has_subdir,FALSE);
          }
          else
          {
             node=gtk_ctree_insert_node(GTK_CTREE(widget),parent_node,NULL,&text,4,dir_tree->dennied,dir_tree->dennied_mask,dir_tree->dennied,dir_tree->dennied_mask,!has_subdir,FALSE);
          }

          gtk_ctree_node_set_row_data_full(GTK_CTREE(widget),node,dirnode,destroy_tree);
          if(has_subdir)
                  sub_node=gtk_ctree_insert_node(GTK_CTREE(widget),node,NULL,&dummy,4,NULL,NULL,NULL,NULL,FALSE,FALSE);
        }
        g_free(path);
      }
      closedir(dir);
      gtk_ctree_sort_node(GTK_CTREE(widget),parent_node);
    }
    gtk_clist_thaw(GTK_CLIST(widget));
    parent_dirnode->scanned=TRUE;
  }
}

gint
gtk_dir_tree_open_dir(GtkDirTree *dir_tree, const gchar *path)
{
  GtkCTreeNode *root_node, *node;
  GtkDirTreeNode *dir_node;
  DIR *dir;
  gchar *c;
  gchar *folder;
  gint nlen;
  gint new_path, new_node;
  gchar *text;
  gchar root[5], root1[5], root2[5], root3[5], root4[5];
  gchar *aux_path = NULL, *real_path = NULL;

  if((dir=opendir(path)) == NULL) return FALSE;
  closedir(dir);

  /* GET ABSOLUTE PATH */

  sprintf(root,"%s",G_DIR_SEPARATOR_S);
  sprintf(root1,"%s.",G_DIR_SEPARATOR_S);
  sprintf(root2,"%s..",G_DIR_SEPARATOR_S);
  sprintf(root3,"%s..%s",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
  sprintf(root4,"%s.%s",G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);

  if(path){
     gint length;

     aux_path = g_strdup(path);
     length = strlen(aux_path);

     if(strcmp(aux_path + length - 2, root1) == 0){
        if(length == 2) {
           g_free(aux_path);
           aux_path = g_strdup(root);
        } else {
           aux_path[length - 1] = '\0';
        }
     } else if(strcmp(aux_path + length - 3, root2) == 0){
        if(length == 3) {
           g_free(aux_path);
           aux_path = g_strdup(root);
        } else {
           gint i = length - 4;
           while(i >= 0){
              if(aux_path[i] == root[0]){
                   aux_path[i+1] = '\0';
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
                   aux_path[i+1] = '\0';
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
           aux_path[length - 2] = '\0';
        }
     }
  }

  if(strlen(aux_path) == 0)
    real_path = g_strdup(G_DIR_SEPARATOR_S);
  else
    real_path = g_strconcat(aux_path,G_DIR_SEPARATOR_S,NULL);

  g_free(aux_path);
 
  c = (gchar *)real_path;
  folder = NULL; 
  nlen = 0;
 
  root_node = gtk_ctree_node_nth(GTK_CTREE(dir_tree), 1);
  gtk_ctree_expand(GTK_CTREE(dir_tree), root_node);
 
  new_path = FALSE;
  new_node = TRUE;

  while(*c != '\0' && *c != '\n' && c != NULL){
   nlen++;
   folder = (char *)g_realloc(folder, (nlen+1)*sizeof(char));
   folder[nlen-1] = *c;
   folder[nlen]='\0';
   if(*c == G_DIR_SEPARATOR){
       if(new_path){
         node = GTK_CTREE_ROW(root_node)->children;
         while(node){
           dir_node = gtk_ctree_node_get_row_data(GTK_CTREE(dir_tree), node);
           text = dir_node->path;
           if(strcmp(text, folder) == 0){
                gtk_ctree_expand(GTK_CTREE(dir_tree), node);
                root_node = node;
                break;
           }
           node = GTK_CTREE_NODE_NEXT(node); 
         }
       }
       else
       {
         new_path = TRUE;
       }
       new_node = FALSE; 
   } else {
       new_node = TRUE;
   }
   c++;
  }

  if(new_node){
     nlen++;
     folder = (char *)g_realloc(folder, (nlen+1)*sizeof(char));
     folder[nlen-1] = G_DIR_SEPARATOR;
     folder[nlen]='\0';
     node = GTK_CTREE_ROW(root_node)->children;
     while(node){
       dir_node = gtk_ctree_node_get_row_data(GTK_CTREE(dir_tree), node);
       text = dir_node->path;
       if(strcmp(text, folder) == 0){
            gtk_ctree_expand(GTK_CTREE(dir_tree), node);
            root_node = node;
            break;
       }
       node = GTK_CTREE_NODE_NEXT(node); 
     }
  }

  g_free(folder);
  if (gtk_ctree_node_is_visible(GTK_CTREE(dir_tree), root_node) !=
    GTK_VISIBILITY_FULL) {
    gtk_widget_map(GTK_WIDGET(dir_tree));
    gtk_ctree_node_moveto(GTK_CTREE(dir_tree), root_node, 0, 0.5, 0.5);
  }
  gtk_ctree_select(GTK_CTREE(dir_tree), root_node);

  g_free(real_path);
  return TRUE;
}

gboolean
check_dir_extra (gchar *dir_name, struct stat *result, gboolean *stat_subdirs)
{
  /* A list of directories that we know only contain other directories.
   * Trying to stat every file in these directories would be very
   * expensive.
   */

  static struct {
    gchar *name;
    gboolean present;
    struct stat statbuf;
  } no_stat_dirs[] = {
    { "/afs", FALSE, { 0 } },
    { "/net", FALSE, { 0 } }
  };

  static const gint n_no_stat_dirs = sizeof(no_stat_dirs) / sizeof(no_stat_dirs[0]);
  static gboolean initialized = FALSE;

  gint i;

  if (!initialized)
    {
      initialized = TRUE;
      for (i = 0; i < n_no_stat_dirs; i++)
        {
          if (stat (no_stat_dirs[i].name, &no_stat_dirs[i].statbuf) == 0)
            no_stat_dirs[i].present = TRUE;
        }
    }

  if(stat(dir_name, result) < 0)
    {
/*      cmpl_errno = errno;*/
      return FALSE;
    }

  *stat_subdirs = TRUE;
  for (i=0; i<n_no_stat_dirs; i++)
    {
      if (no_stat_dirs[i].present &&
          (no_stat_dirs[i].statbuf.st_dev == result->st_dev) &&
          (no_stat_dirs[i].statbuf.st_ino == result->st_ino))
        {
          *stat_subdirs = FALSE;
          break;
        }
    }

  return TRUE;
}


