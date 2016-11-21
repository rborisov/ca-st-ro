#ifndef __UTILS_H__
#define __UTILS_H__

#include "mpd_utils.h"

void utils_init();
void utils_close();
//void get_random_song(char *, char *);
void mpd_poll();
char* get_current_album();
int mpd_db_update_current_song_rating(int);
int mpd_db_get_current_song_rating();
void rm_current_songfile();

#endif
