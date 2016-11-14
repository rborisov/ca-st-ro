/* 
 *  *  A threaded server
 *   *  by Martin Broadhurst (www.martinbroadhurst.com)
 *    *  Compile with -pthread
 *     */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#define PORT    "9213" /* Port to listen on */
#define BACKLOG     10  /* Passed to listen() */

void *handle(void *pnewsock)
{
    int sock = *(int*)pnewsock;
    char buffer[128];
    ssize_t n;
    bzero(buffer, 128);
    n = read(sock, buffer, 127);
    if (n < 0) 
        printf("ERROR reading from socket\n");
    printf("> %s\n", buffer);
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
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        perror("getaddrinfo");
        goto ERROR;
    }

    /* Create the socket */
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == -1) {
        perror("socket");
        goto ERROR;
    }

    /* Enable the socket to reuse the address */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1) {
        perror("setsockopt");
        goto ERROR;
    }

    /* Bind to the address */
    if (bind(sock, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        goto ERROR;
    }

    freeaddrinfo(res);

    /* Listen */
    if (listen(sock, BACKLOG) == -1) {
        perror("listen");
        goto ERROR;
    }

    /* Main loop */
    while (1) {
        socklen_t size = sizeof(struct sockaddr_in);
        struct sockaddr_in their_addr;
        int newsock = accept(sock, (struct sockaddr*)&their_addr, &size);
        if (newsock == -1) {
            perror("accept");
        }
        else {
            printf("Got a connection from %s on port %d\n", 
                    inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));
            /* Make a safe copy of newsock */
            int *safesock = malloc(sizeof(int));
            if (safesock) {
                *safesock = newsock;
                if (pthread_create(&thread, NULL, handle, safesock) != 0) {
                    fprintf(stderr, "Failed to create thread\n");
                }
            }
            else {
                perror("malloc");
            }
        }
    }
ERROR:
    close(sock);
}

int main()
{
    pthread_t main_thread;
    if (pthread_create(&main_thread, NULL, mainthread, NULL) != 0) {
        printf("main thread error\n");
        return -1;
    }
    while (1) {
        sleep(5);
        printf("-");
    }
    return 0;
}
