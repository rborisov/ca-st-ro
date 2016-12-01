#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <syslog.h>

#include "mpd_utils.h"
#include "config.h"

char nullstr[1] = "";
char titlebuf[128] = "";
char artistbuf[128] = "";
char albumbuf[128] = "";

bool mpd_is_in_queue(const char *uri)
{
    bool res = false;
    struct mpd_entity *entity;
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
    if (!mpd_send_list_queue_meta(conn)) {
        syslog(LOG_ERR, "%s: %s", __func__, mpd_connection_get_error_message(conn));
        mpd_connection_clear_error(conn);
        goto DONE;
    }
    while((entity = mpd_recv_entity(conn)) != NULL) {
        const struct mpd_song *song;
        if(mpd_entity_get_type(entity) == MPD_ENTITY_TYPE_SONG && !res) {
            song = mpd_entity_get_song(entity);
            if (strcmp(mpd_song_get_uri(song), uri) == 0) {
                syslog(LOG_INFO, "%s: %s is already in the queue", __func__, uri);
                res = true;
            }
        }
        mpd_entity_free(entity);
    }

DONE:
    if(conn != NULL)
        mpd_connection_free(conn);
    return res;
}

int mpd_crop()
{
    int res = 1;
    struct mpd_status *status = mpd_run_status(mpd.conn);
    if (status == 0) {
        res = 0;
        goto ERROR;
    }
    int length = mpd_status_get_queue_length(status) - 1;

    if (length < 0) {
        syslog(LOG_INFO, "%s: A playlist longer than 1 song in length is required to crop.\n", __func__);
    } else if (mpd_status_get_state(status) == MPD_STATE_PLAY ||
            mpd_status_get_state(status) == MPD_STATE_PAUSE) {
        if (!mpd_command_list_begin(mpd.conn, false)) {
            syslog(LOG_ERR, "%s: mpd_command_list_begin failed\n", __func__);
            res = 0;
            goto DONE;
        }

        for (; length >= 0; --length)
            if (length != mpd_status_get_song_pos(status))
                mpd_send_delete(mpd.conn, length);

        if (!mpd_command_list_end(mpd.conn) || !mpd_response_finish(mpd.conn)) {
            syslog(LOG_ERR, "%s: mpd_command_list_end || mpd_response_finish failed\n", __func__);
        }
        res = 0;
    } else {
        syslog(LOG_INFO, "%s: You need to be playing to crop the playlist\n", __func__);
    }
DONE:
    mpd_status_free(status);
ERROR:
    return res;
}

int mpd_list_artists()
{
    int num = 0;

    mpd_search_db_tags(mpd.conn, MPD_TAG_ARTIST);
    if (!mpd_search_commit(mpd.conn)) {
        syslog(LOG_DEBUG, "%s: search_commit error\n", __func__);
        return 0;
    }

    struct mpd_pair *pair;
    while ((pair = mpd_recv_pair_tag(mpd.conn, MPD_TAG_ARTIST)) != NULL) {
        syslog(LOG_DEBUG, "%s: %s\n", __func__, pair->value);
        mpd_return_pair(mpd.conn, pair);
        num++;
    }

    if (!mpd_response_finish(mpd.conn)) {
        syslog(LOG_DEBUG, "%s: error\n", __func__);
        return 0;
    }

    syslog(LOG_DEBUG, "%s: found %i\n", __func__, num);
    return num;
}

char* mpd_get_current_title()
{
    struct mpd_song *song;

    if (mpd.conn_state == MPD_CONNECTED) {
        song = mpd_run_current_song(mpd.conn);
        if(song == NULL) {
            printf("song == NULL\n");
            return NULL;
        }
        sprintf(titlebuf, "%s", mpd_get_title(song));
        mpd_song_free(song);
    }

    mpd_response_finish(mpd.conn);

    return titlebuf;
}

char* mpd_get_current_artist()
{   
    struct mpd_song *song;

    if (mpd.conn_state == MPD_CONNECTED) {
        song = mpd_run_current_song(mpd.conn);
        if(song == NULL) {
            printf("song == NULL\n");
            return NULL;
        }
        sprintf(artistbuf, "%s", mpd_get_artist(song));
        mpd_song_free(song);
    }

    mpd_response_finish(mpd.conn);

    return artistbuf;
}

char* mpd_get_current_album()
{
    struct mpd_song *song;

    if (mpd.conn_state == MPD_CONNECTED) {
        song = mpd_run_current_song(mpd.conn);
        if(song == NULL) {
            printf("song == NULL\n");
            return NULL;
        }
        sprintf(albumbuf, "%s", mpd_get_album(song));
        mpd_song_free(song);
    }

    mpd_response_finish(mpd.conn);

    return albumbuf;
}

char* mpd_get_artist(struct mpd_song const *song)
{
    char *str;
    str = (char *)mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
    if(str == NULL) {
        str = basename((char *)mpd_song_get_uri(song));
        if  (str == NULL)
            str = nullstr;
    }

    return str;
}

char* mpd_get_album(struct mpd_song const *song)
{
    char *str;
    str = (char *)mpd_song_get_tag(song, MPD_TAG_ALBUM, 0);
    if(str == NULL)
        str = nullstr; //TODO: get album from db

    return str;
}

char* mpd_get_title(struct mpd_song const *song)
{
    char *str;

    str = (char *)mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
    if(str == NULL){
        str = basename((char *)mpd_song_get_uri(song));
        if (str == NULL)
            str = nullstr;
    }

    return str;
}

unsigned mpd_get_queue_length()
{
    unsigned length = 0;
    struct mpd_status *status = mpd_run_status(mpd.conn);
    if (status != NULL) {
        length = mpd_status_get_queue_length(status);
        mpd_status_free(status);
    }
    mpd_response_finish(mpd.conn);
    return length;
}

int mpd_insert (char *song_path )
{
    int res = 0;
    struct mpd_status *status = mpd_run_status(mpd.conn);
    if (status == NULL)
        goto ERROR;
    const unsigned from = mpd_status_get_queue_length(status);
    const int cur_pos = mpd_status_get_song_pos(status);
    mpd_status_free(status);

    if (mpd_run_add(mpd.conn, song_path) != true)
        goto ERROR;

    /* check the new queue length to find out how many songs were
     *        appended  */
    const unsigned end = mpd_get_queue_length();
    if (end == from)
        goto ERROR;

    /* move those songs to right after the current one */
    res = mpd_run_move_range(mpd.conn, from, end, cur_pos + 1);
ERROR:
    return res;
}

void mpd_toggle_play(void)
{
    if (mpd.state == MPD_STATE_STOP)
        mpd_run_play(mpd.conn);
    else
        mpd_run_toggle_pause(mpd.conn);
}

void mpd_next(void)
{
    mpd_run_next(mpd.conn);
}

void mpd_prev(void)
{
    mpd_run_previous(mpd.conn);
}

void mpd_change_volume(int val)
{
    int volume = mpd.volume + val;
    if (volume > 100)
        volume = 100;
    if (volume < 0)
        volume = 0;
    mpd_run_set_volume(mpd.conn, volume);
}

void mpd_put_state(void)
{
    int len;
    unsigned queue_len;
    //    int song_pos, next_song_pos;
    struct mpd_status *status = mpd_run_status(mpd.conn);

    if (!status) {
        syslog(LOG_ERR, "%s mpd_run_status: %s\n", __func__, mpd_connection_get_error_message(mpd.conn));
        mpd.conn_state = MPD_FAILURE;
        goto ERROR;
    }

    mpd.song_pos = mpd_status_get_song_pos(status);
    mpd.next_song_pos = mpd.song_pos+1; //TODO: mpd_status_get_next_song_pos(status);
    mpd.queue_len = mpd_status_get_queue_length(status);
    mpd.volume = mpd_status_get_volume(status);
    mpd.state = mpd_status_get_state(status);
    mpd.repeat = mpd_status_get_repeat(status);
    mpd.single = mpd_status_get_single(status);
    mpd.consume = mpd_status_get_consume(status);
    mpd.random = mpd_status_get_random(status);
    mpd.elapsed_time = mpd_status_get_elapsed_time(status);
    mpd.total_time = mpd_status_get_total_time(status);
    mpd.song_id = mpd_status_get_song_id(status);

    //    printf("%d\n", mpd.song_id);

    mpd_status_free(status);
    mpd_response_finish(mpd.conn);
ERROR:
    return;
}
