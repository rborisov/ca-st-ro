#ifndef __GTK_UTILS_H__
#define __GTK_UTILS_H__

#include <libintl.h>
 
#define UI_FILE "player.glade"
#define APPNAME "player"
#define PREFIX "../assets"
#define VERSION "0.1"

#define _(String) gettext (String)

GtkWidget    *main_window;

void gtk_poll(void);
void gtk_app_init(void);

#endif
