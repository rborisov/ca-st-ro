#include <pthread.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "db_utils.h"
#include "memory_utils.h"
#include "mpd_utils.h"

struct memstruct albumstr;
pthread_t rndthread;
bool rnd_completed = true;

void utils_init()
{
    memory_init((void *)&albumstr);
}
void utils_close()
{
    memory_clean((void *)&albumstr);
}

void rm_current_songfile()
{
    struct mpd_song *song = NULL;
    char *songuri = NULL;
    char filepath[255];
    song = mpd_run_current_song(mpd.conn);
    if (song == NULL) {
        syslog(LOG_DEBUG, "%s: no current song", __func__);
        return;
    }
    songuri = mpd_song_get_uri(song);
    sprintf(filepath, "%s/%s", MUSICPATH, songuri);
    syslog(LOG_DEBUG, "%s: lets delete %s forever", __func__, filepath);
    if (unlink(filepath) != 0)
        syslog(LOG_DEBUG, "%s: cant", __func__);
    mpd_song_free(song);

    return;
}

void get_random_song(char *path)
{
    struct mpd_entity *entity;
    int listened0 = 65000,
        skipnum, numberofsongs = 0;
    bool Done = false;
    char *str = NULL;

    struct mpd_connection *conn = mpd.conn; //mpd_connection_new(NULL, NULL, 3000);
    if (conn == NULL) {
        syslog(LOG_ERR, "%s - Out of memory.", __func__);
        goto DONE;
    }
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
        syslog(LOG_ERR, "%s - MPD connection: %s\n", __func__,
                mpd_connection_get_error_message(conn));
        goto DONE;
    }

    struct mpd_stats *stats = mpd_run_stats(mpd.conn);
    if (stats == NULL)
        goto DONE;
    numberofsongs = mpd_stats_get_number_of_songs(stats);
    mpd_stats_free(stats);
    skipnum = rand() % numberofsongs;

    syslog(LOG_DEBUG, "%s: path: %s; number of songs: %i skip: %i\n",
            __func__, path, numberofsongs, skipnum);
    if (!mpd_send_list_all_meta(mpd.conn, ""))//path))
    {
        syslog(LOG_ERR, "%s: error: mpd_send_list_meta %s\n", __func__, path);
        goto DONE;
    }
    while((entity = mpd_recv_entity(mpd.conn)) != NULL)
    {
        const struct mpd_song *song;
        if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG)
        {
            if ((skipnum-- > 0) || Done)
                continue;

            int listened, whenplayed;
            song = mpd_entity_get_song(entity);
            listened = db_get_song_numplayed(mpd_get_title(song),
                    mpd_get_artist(song));
            whenplayed = db_get_song_played(mpd_get_title(song),
                    mpd_get_artist(song));
            if (listened < listened0) {
                listened0 = listened;
                int probability = 50 - listened +
                    db_get_song_rating(mpd_get_title(song),
                            mpd_get_artist(song));
                bool Yes = (rand() % 100) < probability;
                if (Yes) {
                    str = mpd_song_get_uri(song);
                    syslog(LOG_DEBUG, "%s: probability: %i; uri: %s ", __func__, probability, str);
                    Done = true;
                }
            }
        }
        mpd_entity_free(entity);
    }
DONE:
    if (Done && strlen(str) > 5) {
        if (!mpd_run_add(mpd.conn, str)) {
            syslog(LOG_ERR, "%s: %s", __func__, mpd_connection_get_error_message(mpd.conn));
        }
    }
//    if(conn != NULL)
//        mpd_connection_free(conn);

    return;
}

char* get_current_album()
{
    const struct mpd_song *song;
    char *str = NULL;
    if (mpd.conn_state == MPD_CONNECTED) {
        song = mpd_run_current_song(mpd.conn);
        if(song == NULL) {
            printf("song == NULL\n");
            return NULL;
        }
        str = (char *)mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
        if (str == NULL) {
            str = db_get_song_album(mpd_song_get_tag(song, MPD_TAG_TITLE, 0),
                    mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
            printf("%s: no MPD_TAG_ALBUM\n", __func__);
        }
        if (str) {
            memory_write(str, 1, strlen(str), (void *)&albumstr);
            str = albumstr.memory;
            printf("%s: %s\n", __func__, albumstr.memory);
        }

        mpd_song_free(song);
    }

    return str;
}

int mpd_db_get_current_song_rating()
{
    int rating;
    struct mpd_song *song;

    song = mpd_run_current_song(mpd.conn);
    if(song == NULL)
        return 0;

    rating = db_get_song_rating(mpd_get_title(song), mpd_get_artist(song));

    mpd_song_free(song);
    return rating;
}

int mpd_db_update_current_song_rating(int increase)
{
    int rating = 0;
    struct mpd_song *song;

    song = mpd_run_current_song(mpd.conn);
    if(song == NULL)
        return 0;

    rating = db_update_song_rating(mpd_get_title(song),
            mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), increase);

    mpd_song_free(song);

    return rating;
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
            if (mpd.song_pos+1 >= mpd.queue_len)
            {
                char str[128] = "";
                syslog(LOG_DEBUG, "%s: queue is empty %i(%i)\n", __func__, mpd.song_pos, mpd.queue_len);
                get_random_song("");
/*                get_random_song(str, "");
                if (strlen(str) > 5)
                    if (!mpd_run_add(mpd.conn, str)) {
                        syslog(LOG_ERR, "%s: %s", __func__, mpd_connection_get_error_message(mpd.conn));
                    }*/
            }

            break;
        default:
            syslog(LOG_INFO, "%s - mpd.conn_state %i\n", __func__, mpd.conn_state);
    }
}

