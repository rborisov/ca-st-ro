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
    pthread_mutex_lock(&rnd_mutex);

    struct mpd_entity *entity;
    int listened0 = 65000,
        skipnum, numberofsongs = 0;
    bool Done = false;
    char *str = NULL;

    rnd_completed = false;

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
    skipnum = rand() % numberofsongs;

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
            int whenplayed = db_get_song_played(mpd_get_title(song),
                    mpd_get_artist(song));
            /*
             * quite sofisticated formula to calculate the probability of song to be playing
             */
            int probability = 50 + whenplayed/10 + 
                db_get_song_rating(mpd_get_title(song), mpd_get_artist(song)) * 10;

            bool Yes = (rand() % 100) < probability;
            if (Yes) {
                str = mpd_song_get_uri(song);
                memory_write(str, 1, strlen(str), (void *)&rndstr);
                str = rndstr.memory;
                syslog(LOG_DEBUG, "%s: probability: %i; when: %d; uri: %s", __func__, probability, whenplayed, str);
                Done = true;
            }
        }
        mpd_entity_free(entity);
    }
DONE:
    if (Done) {
        syslog(LOG_DEBUG, "%s: add to queue: %s", __func__, str);
        if (!mpd_run_add(conn, str)) {
            syslog(LOG_ERR, "%s: %s", __func__, mpd_connection_get_error_message(conn));
        }
    }
    if(conn != NULL)
        mpd_connection_free(conn);

    rnd_completed = true;
    pthread_mutex_unlock(&rnd_mutex);

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
            if (mpd.song_pos+QUEUETAIL >= mpd.queue_len)
            {
                syslog(LOG_DEBUG, "%s: queue is empty %i(%i)\n", __func__, mpd.song_pos, mpd.queue_len);
                if (rnd_completed) {
                    int res = pthread_create(&rndthread, NULL, get_random_song, "");
                    if (res) {
                        syslog(LOG_ERR, "%s: pthread_create returns %s", __func__, res);
                    }
                }
            }
            break;
        default:
            syslog(LOG_INFO, "%s - mpd.conn_state %i\n", __func__, mpd.conn_state);
    }
}

