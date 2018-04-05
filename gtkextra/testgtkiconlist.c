#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gtkiconlist.h"
#include "icons.h"

void
quit ()
{
  gtk_main_quit();
}

void
clear(GtkWidget *widget, gpointer data)
{
  gtk_icon_list_clear(GTK_ICON_LIST(widget));
}

int main(int argc, char *argv[]){
 GtkWidget *window1;
 GtkWidget *notebook;
 GtkWidget *vbox1;
 GtkWidget *hbox1;
 GtkWidget *table;
 GtkWidget *label1;
 GtkWidget *label2;
 GtkWidget *scrollw1;
 GtkWidget *scrollw2;
 GtkWidget *iconlist1;
 GtkWidget *iconlist2;
 GdkColor color;
 GtkIconListItem *item;
 gchar label[10];
 gint i;
 
 gtk_init(&argc,&argv);

 window1=gtk_window_new(GTK_WINDOW_TOPLEVEL);
 gtk_window_set_title(GTK_WINDOW(window1), "GtkIconList Demo");
 gtk_widget_set_size_request(window1,400,400);
 gtk_container_border_width(GTK_CONTAINER(window1),0);

 g_signal_connect (GTK_OBJECT (window1), "destroy",
		     G_CALLBACK (quit), NULL);

 vbox1=gtk_vbox_new(FALSE,0);
 gtk_container_add(GTK_CONTAINER(window1),vbox1);
 gtk_widget_show(vbox1);

 hbox1=gtk_hbox_new(FALSE,0);
 gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 0);
 gtk_widget_show(hbox1);

 table = gtk_table_new(4, 2, FALSE);
 gtk_box_pack_start(GTK_BOX(hbox1), table, FALSE, FALSE, 0);
 gtk_widget_show(table);

 notebook = gtk_notebook_new();
 gtk_box_pack_start(GTK_BOX(hbox1),notebook, TRUE, TRUE,0);
 gtk_widget_show(notebook);
 

 scrollw1=gtk_scrolled_window_new(NULL, NULL);
 gtk_container_border_width(GTK_CONTAINER(scrollw1),0);
 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw1),
				GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
 label1 = gtk_label_new("Worksheets");
 gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrollw1, label1);
 gtk_widget_show(scrollw1);

 scrollw2=gtk_scrolled_window_new(NULL, NULL);
 gtk_container_border_width(GTK_CONTAINER(scrollw2),0);
 gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw2),
				GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
 label2 = gtk_label_new("Plots");
 gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrollw2, label2);
 gtk_widget_show(scrollw2);


 iconlist1 = gtk_icon_list_new(48, GTK_ICON_LIST_TEXT_BELOW);
 gtk_icon_list_set_selection_mode(GTK_ICON_LIST(iconlist1), GTK_SELECTION_SINGLE);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw1),iconlist1);
 for(i=0; i<20; i++){
  sprintf(label,"Data %d",i); 
  gtk_icon_list_add_from_data(GTK_ICON_LIST(iconlist1), sheet_icon2, label, NULL);
 }

 iconlist2 = gtk_icon_list_new(48, GTK_ICON_LIST_TEXT_RIGHT);
 gtk_icon_list_set_selection_mode(GTK_ICON_LIST(iconlist2), GTK_SELECTION_MULTIPLE);
 gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrollw2),iconlist2);
 for(i=0; i<20; i++){
  GtkIconListItem *new_item;
  sprintf(label,"Plot %d",i); 
  new_item = gtk_icon_list_add_from_data(GTK_ICON_LIST(iconlist2), plot_icon2, label, NULL);
  if(i==0) item = new_item;
 }

 gdk_color_parse("light blue",&color);
 gdk_color_alloc(gdk_colormap_get_system(), &color);
 gtk_icon_list_set_background(GTK_ICON_LIST(iconlist1), &color);

 gtk_widget_show(iconlist1);
 gtk_widget_show(iconlist2);

 gtk_icon_list_set_active_icon(GTK_ICON_LIST(iconlist2), item);
 gtk_widget_show(window1);


/*
 g_signal_connect(GTK_OBJECT(iconlist),"button_press_event", clear, NULL);
*/

 gtk_main();

 return(0);
}


