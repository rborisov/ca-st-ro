#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <math.h>
#include <gps.h>

#include "gps_utils.h"
#include "gtk_utils.h"

pthread_t gps_thread;
#define _WAIT_GPS_ 10
#define _MIN_MOVE_ 1

static void fix(struct gps_data_t *gpsdata)
{
    double move;
    static double moved = 0;
    static double time_saved = 0;
    static double lat_saved = 0, lon_saved = 0;
    static bool first = true;
    int speedkph;
    char speedstr[4] = "\0";
    char movedstr[6] = "\0";
    static int speed_saved = 0;

    if ((gpsdata->fix.time == time_saved) || gpsdata->fix.mode < MODE_2D)
        return;

    if (!first) {
        move = earth_distance(gpsdata->fix.latitude, gpsdata->fix.longitude,
                lat_saved, lon_saved);
        if (move < _MIN_MOVE_)
            return;
        moved += move;
        snprintf(movedstr, 5, "%f", moved/1000);

        syslog(LOG_INFO, "%s: %s km, %f m", __func__, movedstr, moved);
//        send_to_server("[moved]", movedstr);
    }

    if (first) {
        first = false;
    }

    lat_saved = gpsdata->fix.latitude;
    lon_saved = gpsdata->fix.longitude;
    time_saved = gpsdata->fix.time;

    speedkph = round(gpsdata->fix.speed*3.6);

    if ((speedkph >= 0) &&
            (speedkph < 300) &&
            (speedkph != speed_saved)) {
        sprintf(speedstr, "%d", speedkph);
        
        ui_show_speed(speedstr);
        
        speed_saved = speedkph;
    }
}

void gpsthread(void)
{
    int rc;
    struct gps_data_t gps_data;

    while ((rc = gps_open("localhost", "2947", &gps_data)) == -1) {
        syslog(LOG_ERR, "%s: code: %d, reason: %s\n", __func__, rc,
                gps_errstr(rc));
        
        sleep(_WAIT_GPS_);
    }

    syslog(LOG_INFO, "%s: gps_open passed: %s", __func__, gps_errstr(rc));

    while (1) {
        gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);
        gps_mainloop(&gps_data, 5000000, fix);
        
        syslog(LOG_ERR, "%s: timeout reached", __func__);
        
        gps_stream(&gps_data, WATCH_DISABLE, NULL);
        gps_close(&gps_data);
        
        sleep(_WAIT_GPS_);
    }
}

int gpsutils_init()
{
    syslog(LOG_DEBUG, "%s: >", __func__);
    if (pthread_create(&gps_thread, NULL, gpsthread, NULL) != 0) {
        syslog(LOG_ERR, "%s: main thread error", __func__);
        return 0;
    }
    syslog(LOG_DEBUG, "%s: <", __func__);
    return 1;
}
