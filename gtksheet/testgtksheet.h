#ifndef __TESTGTKSHEET_H__
#define __TESTGTKSHEET_H__

/* XPM */
static char * bullet_xpm[] = {
"16 16 26 1",
" 	c #None",
".	c #000000000000",
"X	c #0000E38D0000",
"o	c #0000EBAD0000",
"O	c #0000F7DE0000",
"+	c #0000FFFF0000",
"@	c #0000CF3C0000",
"#	c #0000D75C0000",
"$	c #0000B6DA0000",
"%	c #0000C30B0000",
"&	c #0000A2890000",
"*	c #00009A690000",
"=	c #0000AEBA0000",
"-	c #00008E380000",
";	c #000086170000",
":	c #000079E70000",
">	c #000071C60000",
",	c #000065950000",
"<	c #000059650000",
"1	c #000051440000",
"2	c #000045140000",
"3	c #00003CF30000",
"4	c #000030C20000",
"5	c #000028A20000",
"6	c #00001C710000",
"7	c #000014510000",
"     ......     ",
"    .XooO++.    ",
"  ..@@@#XoO+..  ",
" .$$$$$%@#XO++. ",
" .&&*&&=$%@XO+. ",
".*-;;;-*&=%@XO+.",
".;:>>>:;-&=%#o+.",
".>,<<<,>:-&$@XO.",
".<12321<>;*=%#o.",
".1345431,:-&$@o.",
".2467642<>;&$@X.",
" .57.753<>;*$@. ",
" .467642<>;&$@. ",
"  ..5431,:-&..  ",
"    .21<>;*.    ",
"     ......     "};

/* XPM */
static char * smile_xpm[] = {
"16 16 3 1",
" 	c #None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"     ......     ",
"    .XXXXXX.    ",
"  ..XXXXXXXX..  ",
" .XXXXXXXXXXXX. ",
" .XXX..XX..XXX. ",
".XXXX..XX..XXXX.",
".XXXX..XX..XXXX.",
".XXXXXXXXXXXXXX.",
".XX..XXXXXX..XX.",
".XX..XXXXXX..XX.",
".XXX.XXXXXX.XXX.",
" .XXX.XXXX.XXX. ",
" .XXXX....XXXX. ",
"  ..XXXXXXXX..  ",
"    .XXXXXX.    ",
"     ......     "};


#define N_EXAMPLES 3

G_BEGIN_DECLS

#define TEST_TYPE_MAIN_WINDOW                 (test_main_window_get_type ())
#define TEST_MAIN_WINDOW(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_TYPE_MAIN_WINDOW, TestMainWindow))
#define TEST_MAIN_WINDOW_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_TYPE_MAIN_WINDOW, TestMainWindowClass))
#define TEST_IS_MAIN_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_TYPE_MAIN_WINDOW))
#define TEST_IS_MAIN_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_TYPE_MAIN_WINDOW))
#define TEST_MAIN_WINDOW_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), TEST_TYPE_MAIN_WINDOW, TestMainWindowClass))

typedef struct _TestMainWindow        TestMainWindow;
typedef struct _TestMainWindowClass   TestMainWindowClass;

struct _TestMainWindow
{
    GtkWindow window;

    GtkSheet *sheets[N_EXAMPLES];

    GtkWidget *notebook;
    GtkWidget *bg_color_button;
    GtkWidget *fg_color_button;
    GtkWidget *font_button;
    GtkWidget *toolbar;
    GtkTooltips *tooltips;
    GtkAction *justify_left;
    GtkAction *justify_right;
    GtkAction *justify_center;
    GtkWidget *location; /* Location label */
    GtkWidget *entry;
    GtkWidget *curve;
    GtkWidget *popup;
    GtkWidget *border_combo;
};

struct _TestMainWindowClass
{
  GtkWindowClass parent_class;
};

GType          
test_main_window_get_type (void) G_GNUC_CONST;

/*GtkWidget *
test_main_window_new ();*/


typedef struct {
    TestMainWindow *main_win;
    const gchar *menu_text;
} PopupData;

#endif /* __TESTGTKSHEET_H__ */

