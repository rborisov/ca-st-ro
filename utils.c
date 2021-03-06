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
struct memstruct rndstr;
pthread_t rndthread;
bool rnd_completed = true;
pthread_mutex_t rnd_mutex;
static int skip_multiplier = 1;

void utils_init()
{
    memory_init((void *)&albumstr);
    memory_init((void *)&rndstr);
}
void utils_close()
{
    memory_clean((void *)&albumstr);
    memory_clean((void *)&rndstr);
}

void rm_current_songfile()
{
    struct mpd_song *song = NULL;
    char *songuri = NULL;
    char filepath[255];
    song = mpd_run_current_song(mpd.conn);
    if (song != NULL) {
        songuri = mpd_song_get_uri(song);
        sprintf(filepath, "%s/%s", MUSICPATH, songuri);
        syslog(LOG_DEBUG, "%s: lets delete %s forever", __func__, filepath);
        if (unlink(filepath) != 0)
            syslog(LOG_DEBUG, "%s: cant", __func__);
        mpd_song_free(song);
    }
    mpd_response_finish(mpd.conn);

    return;
}

void get_random_song(char *path)
{
    struct mpd_entity *entity;
    int listened0 = 65000,
        skipnum, numberofsongs = 0;
    bool Done = false;
    bool First = true;
    char *str = NULL;
    int rndprb = (rand() % 100);

    rnd_completed = false;

//    pthread_mutex_lock(&rnd_mutex);
    struct mpd_connection *conn = mpd_connection_new(NULL, NULL, 3000);
    if (conn == NULL) {
        syslog(LOG_ERR, "%s - Out of memory.", __func__);
        goto DONE;
    }
    if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
        syslog(LOG_ERR, "%s - MPD connection: %s\n", __func__,
                mpd_connection_get_error_message(conn));
        goto DONE;
    }

    struct mpd_stats *stats = mpd_run_stats(conn);
    if (stats == NULL)
        goto DONE;
    numberofsongs = mpd_stats_get_number_of_songs(stats);
    mpd_stats_free(stats);
    skipnum = (rand() % numberofsongs)/skip_multiplier;

    syslog(LOG_DEBUG, "%s: path: %s; number of songs: %i skip: %i\n",
            __func__, path, numberofsongs, skipnum);
    if (!mpd_send_list_all_meta(conn, ""))//path))
    {
        syslog(LOG_ERR, "%s: error: mpd_send_list_meta %s\n", __func__, path);
        goto DONE;
    }
    while((entity = mpd_recv_entity(conn)) != NULL)
    {
        if (mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG)
        {
            if ((skipnum-- > 0) || Done)
                continue;
            const struct mpd_song *song = mpd_entity_get_song(entity);

            if (First) {
                str = mpd_song_get_uri(song);
                memory_write(str, 1, strlen(str), (void *)&rndstr);
                str = rndstr.memory;
                First = false;
            }

            int whenplayed = db_get_song_played(mpd_get_title(song),
                    mpd_get_artist(song));
            /*
             * quite sofisticated formula to calculate the probability of song 
             * to be playing
             */
            int np = db_get_song_numplayed(mpd_get_title(song), mpd_get_artist(song));
            int probability = 30 + whenplayed/10 + 
                db_get_song_rating(mpd_get_title(song), mpd_get_artist(song)) * 10;

            if ((np == 0) || (rndprb < probability)) {
                str = mpd_song_get_uri(song);
                if (!mpd_is_in_queue(str)) {
                    memory_write(str, 1, strlen(str), (void *)&rndstr);
                    str = rndstr.memory;
                    syslog(LOG_DEBUG, "%s: rnd: %d np: %d probability: %i; when: %d; uri: %s", 
                            __func__, rndprb, np, probability, whenplayed, str);
                    Done = true;
                }
            }
        }
        mpd_entity_free(entity);
    }
    if (str != NULL && conn != NULL) {
        syslog(LOG_DEBUG, "%s: add to queue: %s", __func__, str);
        if (!mpd_run_add(conn, str)) {
            syslog(LOG_ERR, "%s: %s", __func__, mpd_connection_get_error_message(conn));
        }
    }
DONE:
    mpd_response_finish(conn);
    if(conn != NULL)
        mpd_connection_free(conn);
//    pthread_mutex_unlock(&rnd_mutex);
    rnd_completed = true;

    return;
}

char* get_current_album()
{
    const struct mpd_song *song;
    char *str = NULL;
    if (mpd.conn_state == MPD_CONNECTED) {
        song = mpd_run_current_song(mpd.conn);
        if(song != NULL) {
            str = (char *)mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
            if (str == NULL) {
                str = db_get_song_album(mpd_song_get_tag(song, MPD_TAG_TITLE, 0),
                        mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
                syslog(LOG_DEBUG, "%s: no MPD_TAG_ALBUM\n", __func__);
            }
            if (str) {
                memory_write(str, 1, strlen(str), (void *)&albumstr);
                str = albumstr.memory;
                syslog(LOG_DEBUG, "%s: %s\n", __func__, albumstr.memory);
            }

            mpd_song_free(song);
        }

        mpd_response_finish(mpd.conn);
    }

    return str;
}

int mpd_db_get_current_song_rating()
{
    int rating = 0;
    struct mpd_song *song;

    song = mpd_run_current_song(mpd.conn);
    if(song != NULL) {
        rating = db_get_song_rating(mpd_get_title(song), mpd_get_artist(song));
        mpd_song_free(song);
    }
    mpd_response_finish(mpd.conn);
    return rating;
}

int mpd_db_update_current_song_rating(int increase)
{
    int rating = 0;
    struct mpd_song *song;

    song = mpd_run_current_song(mpd.conn);
    if(song != NULL) {
        rating = db_update_song_rating(mpd_get_title(song),
                mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), increase);
        mpd_song_free(song);
    }
    mpd_response_finish(mpd.conn);

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
            if (mpd.song_pos+QUEUETAIL >= mpd.queue_len &&
                    rnd_completed)
            {
                syslog(LOG_DEBUG, "%s: queue is empty %i(%i)\n", __func__, mpd.song_pos, mpd.queue_len);
                int res = pthread_create(&rndthread, NULL, get_random_song, "");
                if (res) {
                    syslog(LOG_ERR, "%s: pthread_create returns %d", __func__, res);
                }
            }
            break;
        default:
            syslog(LOG_INFO, "%s - mpd.conn_state %i\n", __func__, mpd.conn_state);
    }
}

