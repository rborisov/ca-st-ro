#include <gtk/gtk.h>
#include <locale.h>
#include <getopt.h>
#include <libgen.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libintl.h>

#define UI_FILE "player.glade"
#define APPNAME "player"
#define PREFIX "../assets"
#define VERSION "0.1"

#define _(String) gettext (String)

GtkWidget    *main_window;
GtkBuilder  *xml = NULL;

static void mpd_idle(gpointer data)
{
	mpd_poll();
	printf("-");
}

static void player_idle(gpointer data)
{
	printf("+");
}

static void streamripper_idle(gpointer data)
{
	printf("*");
}

int main (int argc, char *argv[])
{
    gchar *path;
    char *lang;
    int opt;
    int longopt_index;
    GdkColor color;
    gdk_color_parse("black", &color);

    setlocale (LC_ALL, lang);
    static struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "h:v", long_options, &longopt_index)) > 0)
    {
        switch (opt)
        {
            case 'v':
                printf ("%s %s\n%s\n", g_ascii_strdown(APPNAME,strlen(APPNAME)),
                        VERSION,
                        "Copyright 2015 Roman Borisov");
                goto cleanup;
                break;
            case 'h':
            default:
                fprintf(stderr, "usage: %s [options]\n", basename(argv[0]));
                fprintf(stderr, "  -h, --help           display this help\n");
                fprintf(stderr, "  -v, --version            version information\n");
                return 1;
        }
    }

    gdk_threads_init ();

    gtk_init (&argc, &argv);

    path = g_strdup_printf ("%s/%s", PREFIX, UI_FILE);
    xml = gtk_builder_new ();
    GError* error = NULL;
    if (!gtk_builder_add_from_file (xml, path, &error)) {
        g_error (_("Failed to initialize interface: %s"), error->message);
        g_error_free(error);
        goto cleanup;
    }
    g_free (path);
                        
    gtk_builder_connect_signals (xml, NULL);

    main_window = GTK_WIDGET (gtk_builder_get_object (xml, "window1"));
    gtk_widget_modify_bg(GTK_WIDGET(main_window), GTK_STATE_NORMAL, &color);
    gtk_widget_show (main_window);

    /*
     * button color
     */
    GtkWidget *button;
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button1"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button2"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button3"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button4"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button5"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button6"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button7"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button8"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "button9"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);


/*
 * background
 * */
    GdkPixmap *background;
    GdkPixbuf *pixbuf;
    GtkStyle *style;

    path = g_strdup_printf ("%s/bkbg.jpg", PREFIX);
    pixbuf = gdk_pixbuf_new_from_file (path,&error);
    if (error != NULL) {
        if (error->domain == GDK_PIXBUF_ERROR) {
            g_print ("Pixbuf Related Error:\n");
        }
        if (error->domain == G_FILE_ERROR) {
            g_print ("File Error: Check file permissions and state:\n");
        }

        g_printerr ("%s\n", error[0].message);
    } else {
        gdk_pixbuf_render_pixmap_and_mask (pixbuf, &background, NULL, 0);
        style = gtk_style_new ();
        style->bg_pixmap[0] = background;
        gtk_widget_set_style (GTK_WIDGET(main_window), GTK_STYLE(style));
    }
/*
 * - background
 */
	gtk_idle_add(player_idle, NULL);
	gtk_idle_add(mpd_idle, NULL);
	gtk_idle_add(streamripper_idle, NULL);
    gdk_threads_enter ();
    gtk_main ();
    gdk_threads_leave ();

cleanup:
    

    return 0;
}
