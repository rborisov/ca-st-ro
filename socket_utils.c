#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <syslog.h>

#include "socket_utils.h"
#include "gtk_utils.h"

#define BACKLOG     10
pthread_t main_thread;

enum message_type
{
//    gps_speed,
    notification_new_file,
    notification_deleted_file,
    notification_mnt_point,
    unknown
};

enum message_type get_type_message(const char *buffer, char **message)
{
    enum message_type type = unknown;
    char *start = buffer;
    char *end = buffer;
    *message = start;
    while (*buffer) {
        if (*buffer == '[') start = buffer;
        else if (*buffer == ']') end = buffer;
        if (start < end && *start) {
            *end = 0;
            if (strcmp(start+1, "newfile") == 0) type = notification_new_file;
            else if (strcmp(start+1, "mntpoint") == 0) type = notification_mnt_point;
//            else if (strcmp(start+1, "speed") == 0) type = gps_speed;
            *message = end+1;
            *end = ']';
            break;
        }
        buffer++;
    }
    return type;
}


void *handle(void *pnewsock)
{
    int type, sock = *(int*)pnewsock;
    char *message, buffer[128];
    ssize_t n;
    bzero(buffer, 128);
    n = read(sock, buffer, 127);
    if (n < 0) 
        syslog(LOG_ERR, "%s: ERROR reading from socket", __func__);
    type = get_type_message(buffer, &message);
//    syslog(LOG_DEBUG, "%s: %s >> %d, %s\n", __func__, buffer, type, message);
    switch (type) {
/*        case gps_speed:
            ui_show_speed(message);
            break;*/
        case notification_new_file:
            sprintf(buffer, "new: %s", message);
            ui_show_notification(buffer);
            break;
        case notification_deleted_file:
            sprintf(buffer, "del: %s", message);
            ui_show_notification(buffer);
            break;
        case notification_mnt_point:
            sprintf(buffer, "mnt: %s", message);
            ui_show_notification(buffer);
            break;
        case unknown:
        default:
            ui_show_notification(message);
    }
//    ui_show_notification(buffer);
    free(pnewsock);

    return NULL;
}

void mainthread(void)
{
    int sock;
    pthread_t thread;
    struct addrinfo hints, *res;
    int reuseaddr = 1; /* True */

    /* Get the address info */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(NULL, SERVERPORT, &hints, &res) != 0) {
        syslog(LOG_ERR, "%s: getaddrinfo", __func__);
        goto ERROR;
    }

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        syslog(LOG_ERR, "%s: socket", __func__);
        goto ERROR;
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        syslog(LOG_ERR, "%s: setsockopt", __func__);
        goto ERROR;
    }

    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        syslog(LOG_ERR, "%s: bind", __func__);
        goto ERROR;
    }

    freeaddrinfo(res);
    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        syslog(LOG_ERR, "%s: listen", __func__);
        goto ERROR;
    }

    /* Main loop */
    while (1) {
        socklen_t size = sizeof(struct sockaddr_in);
        struct sockaddr_in their_addr;
        int newsock = accept(sock, (struct sockaddr*)&their_addr, &size);
        if (newsock == -1) {
            syslog(LOG_ERR, "%s: accept", __func__);
        }
        else {
            /*printf("Got a connection from %s on port %d\n", 
              inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));*/
            /* Make a safe copy of newsock */
            int *safesock = malloc(sizeof(int));
            if (safesock) {
                *safesock = newsock;
                if (pthread_create(&thread, NULL, handle, safesock) != 0) {
                    syslog(LOG_ERR, "%s: Failed to create thread", __func__);
                }
            }
            else {
                syslog(LOG_ERR, "%s: malloc", __func__);
            }
        }
    }
ERROR:
    close(sock);
}

int srv_init()
{
    syslog(LOG_DEBUG, "%s: >", __func__);
    if (pthread_create(&main_thread, NULL, mainthread, NULL) != 0) {
        syslog(LOG_ERR, "%s: main thread error", __func__);
        return 0;
    }
    syslog(LOG_DEBUG, "%s: <", __func__);
    return 1;
}

void srv_close()
{

}
