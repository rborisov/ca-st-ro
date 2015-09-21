#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

static void cb_prev_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer   data)
{
    return;
}

static void cb_next_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer  data)
{
    return;
}

static void cb_play_button_clicked (G_GNUC_UNUSED GtkWidget *widget,
        G_GNUC_UNUSED gpointer   data)
{
    return;
}

static gboolean cb_gimmix_key_press (G_GNUC_UNUSED GtkWidget *widget,
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

void gtk_app_init(void)
{
    GtkWidget    *main_window;
    GtkBuilder  *xml = NULL;

    main_window = GTK_WIDGET (gtk_builder_get_object (xml, "main_window"));
    g_signal_connect (G_OBJECT(main_window), "key-press-event", G_CALLBACK(cb_gimmix_key_press), NULL);
}
