#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define _MAXSTRING_ 128
#define _HOST_ "localhost"
#define _PORT_ 9213

void send_to_server(const char *type, const char *message)
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        syslog(LOG_ERR, "%s: ERROR opening socket\n", __func__);
        goto done;
    }
    server = gethostbyname(_HOST_);
    if (server == NULL) {
        syslog(LOG_ERR, "%s: ERROR, no such host\n", __func__);
        goto done;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(_PORT_);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        syslog(LOG_ERR, "%s: ERROR connecting\n", __func__);
        goto done;
    }
    n = write(sockfd, type, strlen(type));
    if (n) {
        n = write(sockfd, message, strlen(message));
    }
    if (n < 0)
        syslog(LOG_ERR, "%s: ERROR writing to socket\n", __func__);
done:
    close(sockfd);
    return;
}

void main(void)
{
    while (1) {
        send_to_server("[speed]", "20");
        sleep(2);
    }
}
