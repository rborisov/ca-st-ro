/*
wiki.archlinux.org/index.php/Desktop_notifications

Dependency: libnotify

gcc -Wall -o notifications notify.c `pkg-config --libs --cflags glib-2.0 gtk+-2.0 libxml-2.0` -lnotify

hello_world.c
*/

#include <libnotify/notify.h>

int main () {
    notify_init ("Hello world!");
    NotifyNotification * Hello = notify_notification_new ("Hello world", "This is an example notification.", "dialog-information");
    notify_notification_show (Hello, NULL);
    g_object_unref(G_OBJECT(Hello));
    notify_uninit();
    return 0;
}
