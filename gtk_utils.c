#include <pthread.h>
#include <glib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <syslog.h>

#include "config.h"
#include "gtk_utils.h"
#include "mpd_utils.h"
#include "db_utils.h"
#include "socket_utils.h"
#include "utils.h"
//#include "memory_utils.h"

#define black_vertical_rectangle '|'

GtkBuilder  *xml = NULL;
pthread_t notification_thread;
GMutex mutex_interface;

static void gtk_setlabel(char* lbl, char* str);
static void gtk_setlabel_int(char* lbl, int val)
{
    gchar *str;
    str = g_strdup_printf ("%d", val);
    gtk_setlabel(lbl, str);
    g_free(str);
}

void ui_show_speed(gchar *message)
{
#if 1
    gtk_setlabel("lbl_gps_speed", message);
#else
    char a[2] = {0,0};
    GtkWidget *label[3];
    int len = strlen(message);
    
    label[0] = GTK_WIDGET (gtk_builder_get_object (xml, "lab_gps_speed_hundreds"));
    gtk_widget_set_visible(GTK_WIDGET(label[0]), FALSE);
    label[1] = GTK_WIDGET (gtk_builder_get_object (xml, "lab_gps_speed_tens"));
    gtk_widget_set_visible(GTK_WIDGET(label[1]), FALSE);
    label[2] = GTK_WIDGET (gtk_builder_get_object (xml, "lab_gps_speed_units"));

    switch (len) {
        case 3:
            a[0] = message[0];
            gtk_label_set_text (GTK_LABEL (label[0]), a);
            gtk_widget_set_visible(GTK_WIDGET(label[0]), TRUE);
            message++;
        case 2:
            a[0] = message[0];
            gtk_label_set_text (GTK_LABEL (label[1]), a);
            gtk_widget_set_visible(GTK_WIDGET(label[1]), TRUE);
            message++;
        case 1:
            gtk_label_set_text (GTK_LABEL (label[2]), message);
            break;
        default:
            syslog(LOG_ERR, "%s: speed %s, length %d", __func__, message, len);
    }
#endif
}

static void show_notification_after(gchar *message)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    sleep(5);
    gtk_setlabel("lbl_notify", message);
}

void ui_show_notification(gchar *message)
{
    pthread_cancel(notification_thread);
    gtk_setlabel("lbl_notify", message);
    pthread_create(&notification_thread, NULL, show_notification_after, "\0");
}

static void ui_queue_update(int songpos, int queue_length)
{
    gchar *str;
    str = g_strdup_printf ("%d(%d)", songpos, queue_length);
    gtk_setlabel("lbl_queue_length", str);
    g_free(str);
}

static void cb_prev_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer   data)
{
    printf("%s\n", __func__);
    mpd_prev();
    return;
}

static void cb_next_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer  data)
{
    char rating[10];
    sprintf(rating, "%d", mpd_db_update_current_song_rating(-1));
    ui_show_notification(rating);
    mpd_next();
    return;
}

static void cb_play_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer   data)
{
    printf("%s\n", __func__);
    mpd_toggle_play();
    ui_show_notification("play");
    return;
}

static void cb_vol_inc_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
    char volmessage[100/VOLUMESTEP] = "\0";
    int num;
    printf("%s\n", __func__);
    mpd_change_volume(VOLUMESTEP);
    num = mpd.volume/VOLUMESTEP;
    if (num) {
        memset(volmessage, black_vertical_rectangle, num);
        ui_show_notification(volmessage);
    }
    return;
}
static void cb_vol_dec_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
    char volmessage[100/VOLUMESTEP] = "\0";
    int num;
    printf("%s\n", __func__);
    mpd_change_volume(-VOLUMESTEP);
    num = mpd.volume/VOLUMESTEP;
    if (num) {
        memset(volmessage, black_vertical_rectangle, num);
        ui_show_notification(volmessage);
    }
    return;
}
static void cb_like_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
    char rating[10];
    int rat = mpd_db_update_current_song_rating(5);
    sprintf(rating, "%d", rat);
    ui_show_notification(rating);
    gtk_setlabel_int("lbl_rating", rat);

    return;
}
static void cb_radio_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
    printf("%s TODO: WRONG BUTTON\n", __func__);
    rm_current_songfile();
    return;
}
static void cb_crop_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
    printf("%s %i\n", __func__, mpd_get_queue_length());
    mpd_crop();
    printf("%s %i\n", __func__, mpd_get_queue_length());
    return;
}
static void cb_dislike_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
    char rating[10];
    sprintf(rating, "%d", mpd_db_update_current_song_rating(-5));
    ui_show_notification(rating);

    mpd_next();
    return;
}

static gboolean cb_key_press (G_GNUC_UNUSED GtkWidget *widget,
        GdkEventKey *event,
        G_GNUC_UNUSED gpointer userdata)
{
    gboolean result = FALSE;

    if (event->type == GDK_KEY_PRESS) {
        switch (event->keyval) {
            case GDK_KEY_b: /* NEXT */
                cb_next_button_clicked (NULL, NULL);
                result = TRUE;
                break;
            case GDK_KEY_c: /* PLAY/PAUSE */
                cb_play_button_clicked (NULL, NULL);
                result = TRUE;
                break;
            case GDK_KEY_z: /* PREV */
                cb_prev_button_clicked (NULL, NULL);
                result = TRUE;
                break;
        }
    }
    return result;
}

void gtk_win_bg(char* file)
{
    GdkPixmap *background;
    GdkPixbuf *pixbuf, *pixbuf1;
    GtkStyle *style;
    gint width, height;
    gchar *path;
    GError* error = NULL;

    if (file) {
        path = g_strdup_printf ("%s/%s", IMAGEPATH, file);
    } else {
        path = g_strdup_printf ("%s/bkbg.jpg", PREFIX);
    }
    gtk_window_get_size(gtk.main_window, &width, &height);
    printf("bg_image: %s; window size: %ix%i\n", path, width, height);
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
        pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, width, GDK_INTERP_BILINEAR); //GDK_INTERP_TILES); //GDK_INTERP_NEAREST);
        gdk_pixbuf_render_pixmap_and_mask (pixbuf, &background, NULL, 0);
        style = gtk_style_new ();
        style->bg_pixmap[0] = background;
        gtk_widget_set_style (GTK_WIDGET(gtk.main_window), GTK_STYLE(style));
    }
    g_free(path);
}

void gtk_app_init(void)
{
    GdkColor color;
    GError* error = NULL;
    gchar *path;

    //memory_init();

    gdk_color_parse("black", &color);

    path = g_strdup_printf ("%s/%s", PREFIX, UI_FILE);
    xml = gtk_builder_new ();
    if (!gtk_builder_add_from_file (xml, path, &error)) {
        g_error (_("Failed to initialize interface: %s"), error->message);
        g_error_free(error);
        goto cleanup;
    }
    g_free (path);

    gtk_builder_connect_signals (xml, NULL);

    gtk.main_window = GTK_WIDGET (gtk_builder_get_object (xml, "window1"));
    gtk_window_maximize(gtk.main_window);
    gtk_widget_modify_bg(GTK_WIDGET(gtk.main_window), GTK_STATE_NORMAL, &color);
    
    g_signal_connect (G_OBJECT(gtk.main_window), "key-press-event", G_CALLBACK(cb_key_press), NULL);

    GtkWidget *button;
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_prev"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_prev_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_play"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_play_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_next"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_next_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_vol_inc"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_vol_inc_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_vol_dec"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_vol_dec_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_like"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_like_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_radio"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_radio_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_crop"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_crop_button_clicked), NULL);
    button = GTK_WIDGET (gtk_builder_get_object (xml, "btn_dislike"));
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(GTK_WIDGET(button), GTK_STATE_PRELIGHT, &color);
    g_signal_connect(button, "clicked", G_CALLBACK(cb_dislike_button_clicked), NULL);

    gtk_win_bg(NULL);

    gtk.song_id = -555; //magic number to show song at beginning
    gtk.queue_version = 0;

    gtk.conn_state = -555;

    gtk.state = -555;
cleanup:
    return;
}

void gtk_setlabel(char* lbl, char* str)
{
    g_mutex_lock(&mutex_interface);
    GtkWidget *label = NULL;
    label = GTK_WIDGET(gtk_builder_get_object (xml, lbl));
    gtk_label_set(GTK_LABEL(label), str);
    g_mutex_unlock(&mutex_interface);
}

void gtk_poll(void)
{
    GtkWidget *image0;
    gchar *title = NULL, *artist = NULL, *album = NULL;
    gchar time[11] = "00:00/00:00";
    int minutes_elapsed, minutes_total;
    gchar *str, 
#if 0
          *artist_art, 
#endif
          *album_art = NULL;
   
    ui_queue_update(mpd.song_pos+1, mpd.queue_len);

    if (mpd.conn_state != gtk.conn_state) {
        //change background image while mpd disconnected

        gtk.conn_state = mpd.conn_state;
    }

    if (mpd.state != gtk.state) {
        GError* error = NULL;
        GtkWidget *widget = GTK_WIDGET(gtk_builder_get_object (xml, "playpause"));
        GtkImage *image4 = GTK_IMAGE(widget);
        //TODO: move names to define
        gchar *path = g_strdup_printf ("%s/castro-play.svg", PREFIX);
        gchar *path1 = g_strdup_printf ("%s/castro-pause.svg", PREFIX);
        GdkPixbuf *pixbuf_pause, *pixbuf_play = gdk_pixbuf_new_from_file(path, &error);
        if (error == NULL)
            pixbuf_pause = gdk_pixbuf_new_from_file(path1, &error);
        if (error != NULL) {
            if (error->domain == GDK_PIXBUF_ERROR) {
                g_print ("Pixbuf Related Error:\n");
            }
            if (error->domain == G_FILE_ERROR) {
                g_print ("File Error: Check file permissions and state:\n");
            }
            g_printerr ("%s\n", error[0].message);
        }
        g_free(path);
        g_free(path1);
        //mpd.state changed
        switch (mpd.state) {
            case MPD_STATE_PAUSE:
            case MPD_STATE_STOP:
                if (error == NULL) {
                    gtk_image_set_from_pixbuf(image4, pixbuf_play);
                    ui_show_notification("pause");
                }
                syslog(LOG_INFO, "%s MPD_STATE_PAUSE or STOP\n", __func__);
                break;
            case MPD_STATE_PLAY:
                if (error == NULL) {
                    gtk_image_set_from_pixbuf(image4, pixbuf_pause);
                    ui_show_notification("play");
                }
                syslog(LOG_INFO, "%s MPD_STATE_PLAY\n", __func__);
                break;
            case MPD_STATE_UNKNOWN:
                syslog(LOG_INFO, "%s MPD_STATE_UNKNOWN\n", __func__);
        }
        gtk.state = mpd.state;
    }

    if (mpd.song_id != gtk.song_id) {
        gtk_win_bg(NULL); //clean win bg
        gtk_setlabel("lbl_track", "");
        gtk_setlabel("lbl_artist", "");
        gtk_setlabel("lbl_album", "");
        /*
         * track artist album
         */
        title = mpd_get_current_title();
        if (title) {
            syslog(LOG_INFO, "%s: %s %d %d\n", __func__, title, mpd.song_id, gtk.song_id);
            gtk_setlabel("lbl_track", title);
        }
        artist = mpd_get_current_artist();
        if (artist) {
            syslog(LOG_INFO, "%s: %s", __func__, artist);
            gtk_setlabel("lbl_artist", artist);
#if 0
            artist_art = db_get_artist_art(artist);
#endif
            //impossible to have album without artist
            album = get_current_album();
            if (album) {
                syslog(LOG_INFO, "%s: %s\n", __func__, album);
                gtk_setlabel("lbl_album", album);
                album_art = db_get_album_art(artist, album);
            } else {
                //TODO: start download thread
                gtk_setlabel("lbl_album", "");
            }
            /*
             *          * artist art
             *                   */
#if 0
            image0 = GTK_WIDGET (gtk_builder_get_object (xml, "img_artist"));
            if (artist_art) {
                syslog(LOG_INFO, "%s: art: %s\n", __func__, artist_art);
                artist_art = g_strdup_printf("%s/%s", IMAGEPATH, artist_art);
            } else {
                artist_art = g_strdup_printf("%s/art.png", IMAGEPATH);
            }
            gtk_image_set_from_file(GTK_IMAGE (image0), artist_art);
#endif
            /*
             * album art
             */
            gtk_win_bg(album_art);
        }


        if (title && artist) {
            /*
             * update DB with increased num played
             */
            int np, rating, daysbefore;
            daysbefore = db_get_song_played(title, artist);
            np = db_listen_song(title, artist, album);
            rating = mpd_db_get_current_song_rating();
            syslog(LOG_INFO, "%s: np %d; rt %d; db %d>%d", __func__, np, rating, 
                    daysbefore, db_get_song_played(title, artist));

            gtk_setlabel_int("lbl_np", np);
            gtk_setlabel_int("lbl_rating", rating);
        }

        gtk.song_id = mpd.song_id;
    }

    /*
     * time elapsed / total
     * */
    str = g_strdup_printf ("%02i:%02i/%02i:%02i", mpd.elapsed_time / 60, mpd.elapsed_time % 60,
            mpd.total_time / 60, mpd.total_time % 60);
    gtk_setlabel("lbl_time", str);
    g_free(str);

    /*
     * volume
     * */
    gtk_setlabel_int("lbl_volume", mpd.volume);    
}
