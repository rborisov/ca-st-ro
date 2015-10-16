#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>

#include "db_utils.h"
#include "mpd_utils.h"

void get_random_song(char *str, char *path)
{
    struct mpd_entity *entity;
    int listened0 = 65000,
        skipnum, numberofsongs = 0;

    struct mpd_stats *stats = mpd_run_stats(mpd.conn);
    if (stats == NULL)
        return;
    numberofsongs = mpd_stats_get_number_of_songs(stats);
    mpd_stats_free(stats);
    skipnum = rand() % numberofsongs;

    syslog(LOG_DEBUG, "%s: path %s; number of songs: %i skip: %i\n",
            __func__, path, numberofsongs, skipnum);
    if (!mpd_send_list_all_meta(mpd.conn, ""))//path))
    {
        syslog(LOG_ERR, "%s: error: mpd_send_list_meta %s\n", __func__, path);
        return;
    }

    while((entity = mpd_recv_entity(mpd.conn)) != NULL)
    {
        const struct mpd_song *song;
        if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG)
        {
            if (skipnum-- > 0)
                continue;

            int listened;
            song = mpd_entity_get_song(entity);
            listened = db_get_song_numplayed(mpd_get_title(song),
                    mpd_get_artist(song));
            if (listened < listened0) {
                listened0 = listened;
                syslog(LOG_DEBUG, "listened: %i ", listened);
                int probability = 50 +
                    db_get_song_rating(mpd_get_title(song),
                            mpd_get_artist(song));
                syslog(LOG_DEBUG, "probability: %i ", probability);
                bool Yes = (rand() % 100) < probability;
                if (Yes) {
                    sprintf(str, "%s", mpd_song_get_uri(song));
                    syslog(LOG_DEBUG, "uri: %s ", str);
                    syslog(LOG_DEBUG, "title: %s ", mpd_get_title(song));
                    syslog(LOG_DEBUG, "artist: %s", mpd_get_artist(song));
                }
            }
        }
        mpd_entity_free(entity);
    }
}

void mpd_poll()
{
    switch (mpd.conn_state) {
        case MPD_DISCONNECTED:
            syslog(LOG_INFO, "%s - MPD Connecting...\n", __func__);
            mpd.conn = mpd_connection_new(NULL, NULL, 3000);
            if (mpd.conn == NULL) {
                syslog(LOG_ERR, "%s - Out of memory.", __func__);
                mpd.conn_state = MPD_FAILURE;
                return;
            }
            if (mpd_connection_get_error(mpd.conn) != MPD_ERROR_SUCCESS) {
                syslog(LOG_ERR, "%s - MPD connection: %s\n", __func__,
                        mpd_connection_get_error_message(mpd.conn));
                mpd.conn_state = MPD_FAILURE;
                return;
            }
            syslog(LOG_INFO, "%s - MPD connected.\n", __func__);
            mpd_connection_set_timeout(mpd.conn, 10000);
            mpd.conn_state = MPD_CONNECTED;
            break;
        case MPD_FAILURE:
        case MPD_DISCONNECT:
        case MPD_RECONNECT:
            syslog(LOG_ERR, "%s - MPD (dis)reconnect or failure\n", __func__);
            if(mpd.conn != NULL)
                mpd_connection_free(mpd.conn);
            mpd.conn = NULL;
            mpd.conn_state = MPD_DISCONNECTED;
            break;
        case MPD_CONNECTED:
            mpd_put_state();
            break;
        default:
            syslog(LOG_INFO, "%s - mpd.conn_state %i\n", __func__, mpd.conn_state);
    }
}

