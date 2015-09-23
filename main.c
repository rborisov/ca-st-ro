#include <gtk/gtk.h>
#include <locale.h>
#include <getopt.h>
#include <libgen.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libintl.h>

#include "mpd_utils.h"
#include "gtk_utils.h"

static void mpd_idle(gpointer data)
{
	mpd_poll();
//	printf("-");
}

static void player_idle(gpointer data)
{
    gtk_poll();
//	printf("+");
}

static void streamripper_idle(gpointer data)
{
//	printf("*");
}

int main (int argc, char *argv[])
{
    gchar *path;
    //char *lang;
    int opt;
    int longopt_index;
    //setlocale (LC_ALL, lang);
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
    gtk_app_init();
    gtk_widget_show (main_window);
	gtk_idle_add(player_idle, NULL);
	gtk_idle_add(mpd_idle, NULL);
	gtk_idle_add(streamripper_idle, NULL);
    gdk_threads_enter ();
    gtk_main ();
    gdk_threads_leave ();

cleanup:
    

    return 0;
}
