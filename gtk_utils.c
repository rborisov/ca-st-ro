#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "gtk_utils.h"
#include "mpd_utils.h"

//GtkWidget *main_window;
GtkBuilder  *xml = NULL;

static void cb_prev_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer   data)
{
    printf("%s\n", __func__);
    return;
}

static void cb_next_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer  data)
{
    printf("%s\n", __func__);
    return;
}

static void cb_play_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer   data)
{
    printf("%s\n", __func__);
    return;
}

static void cb_vol_inc_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
        printf("%s\n", __func__);
            return;
}
static void cb_vol_dec_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
        printf("%s\n", __func__);
            return;
}
static void cb_like_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
        printf("%s\n", __func__);
            return;
}
static void cb_radio_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
        printf("%s\n", __func__);
            return;
}
static void cb_crop_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
        printf("%s\n", __func__);
            return;
}
static void cb_dislike_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
                G_GNUC_UNUSED gpointer   data)
{
        printf("%s\n", __func__);
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
//    GtkWidget *main_window;
    GdkPixmap *background;
    GdkPixbuf *pixbuf;
    GtkStyle *style;
    gchar *path;
    GError* error = NULL;

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
}

void gtk_app_init(void)
{
//    GtkWidget    *main_window;
//    GtkBuilder  *xml = NULL;
    GdkColor color;
    GError* error = NULL;
    gchar *path;

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

    main_window = GTK_WIDGET (gtk_builder_get_object (xml, "window1"));
    gtk_widget_modify_bg(GTK_WIDGET(main_window), GTK_STATE_NORMAL, &color);
    
    g_signal_connect (G_OBJECT(main_window), "key-press-event", G_CALLBACK(cb_key_press), NULL);

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
cleanup:
    return;
}

void gtk_poll(void)
{
    GtkWidget *label = NULL;
    label = GTK_WIDGET (gtk_builder_get_object (xml, "lbl_track"));
    gtk_label_set (GTK_LABEL (label), "track1");
}
