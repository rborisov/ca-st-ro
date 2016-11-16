#ifndef __GTK_UTILS_H__
#define __GTK_UTILS_H__

#include <libintl.h>
//#include <libnotify/notify.h>
#include <gtk/gtk.h>
#include "config.h"
#include <mpd/client.h>

#define UI_FILE "player.glade"
#define APPNAME "player"
/*#define PREFIX "../assets"*/
#define VERSION "0.1"

#define _(String) gettext (String)

struct t_gtk {
    GtkWidget    *main_window;
//    NotifyNotification * TrackNotify;
    int song_id;
    unsigned queue_version;
    int conn_state;
    char bg_file[128];
    enum mpd_state state;
} gtk;

void gtk_poll(void);
void gtk_app_init(void);
void ui_show_notification(gchar *message);

#endif
