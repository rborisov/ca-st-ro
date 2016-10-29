#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "socket_utils.h"
#include "mpd_utils.h"

static int mnt_sockfd;

int mntsrv_init()
{
    int res = 0;
    struct sockaddr_in serv_addr;
    
    mnt_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (mnt_sockfd < 0) {
        printf("ERROR opening socket\n");
        return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(_MNTPORT_);
    if (bind(mnt_sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0) {
        printf("ERROR on binding\n");
        res = -1;
        goto done;
    }
    listen(mnt_sockfd, 5);
done:
    return res;
}

void mntsrv_poll()
{
    socklen_t clilen;
    int newsockfd, n;
    struct sockaddr_in cli_addr;
    char buffer[3*128];

    printf("_");
    clilen = sizeof(cli_addr);
/*    newsockfd = accept(mnt_sockfd,
            (struct sockaddr *)&cli_addr,
            &clilen);
    if (newsockfd < 0)
        printf("ERROR on accept\n");
    bzero(buffer,256);
    n = read(newsockfd, buffer, 255);
    if (n < 0) printf("ERROR reading from socket\n");
    printf("Here is the message: %s\n",buffer);*/
    
    close(newsockfd);
}

void mntsrv_close()
{
    close(mnt_sockfd);
}
