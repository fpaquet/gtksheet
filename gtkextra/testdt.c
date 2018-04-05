#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "gtkplotdt.h"

static gint 
simpleprogressbar(double p)
{
  static int lastprog= -1;
  int prog,i;
  if (p<0) p=0; if (p>1) p=1;
  prog= 70*p;
  if (prog>lastprog) {
    fprintf(stderr,"\r %3d%% ",(int)(p*100));
    for (i=0; i<prog; i++) fprintf(stderr,"*");
    lastprog= prog;
  }

  return TRUE;
}

int main(int argc, char **argv)
{
  GtkPlotDTnode p;
  GtkPlotDT *data;
  GtkPlotDTtriangle *t;
  GList *list;
  char buffer[100];
  FILE *f;

  gtk_init(&argc,&argv);

  if (argc!=2) {
    fprintf(stderr,"\nUsage:\n\ttestgtkplotdt X-Y-FILE\n");
    exit(-1);
  }
  if (!(f=fopen(argv[1],"r"))) {
    fprintf(stderr,"\ncould not open file '%s' for reading\n",argv[1]);
    exit(-2);
  }

  /* init with nodelist size 0 */
  data= GTK_PLOT_DT(gtk_plot_dt_new(0));
  if (!data) exit(-1);
  /* register the progressmeter */
  if (getenv("HAVE_PROGRESSBAR"))
    data->pbar= simpleprogressbar;

  /* read X/Y pairs from f: */
  while (fgets(buffer,100,f)) {
    if (sscanf(buffer,"%lf %lf", &p.x, &p.y)==2) {
      /* add this node */
      gtk_plot_dt_add_node(data,p);
    }
  }
  fclose(f);
  /* start the triangulation */
  gtk_plot_dt_set_subsampling(data,FALSE);
  gtk_plot_dt_triangulate(data);
  fprintf(stderr,"\n");

  list = data->triangles;
  while(list){
    t = (GtkPlotDTtriangle *)list->data;
    printf("%d %d %d\n",t->a,t->b,t->c);
    list = list->next;
  }

  /* clean up data and exit */
  gtk_object_destroy(GTK_OBJECT(data));
  return 0;
}
