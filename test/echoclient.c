#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
    int clientfd;
    struct addrinfo hints, *listp, *p;
    char buf[4096];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;

    getaddrinfo("localhost", "12345", &hints, &listp);

    for (p = listp; p; p = p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) <
            0)
            continue;

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break;

        if (close(clientfd) < 0) {
            fprintf(stderr, "open_clientfd: close failed: %s\n",
                    strerror(errno));
            return -1;
        }
    }
    freeaddrinfo(listp);

    char *msg = argv[1];

    printf("%d\n", strlen(msg));
    printf("%s\n", msg);
    // Write a simple string
    send(clientfd, msg, strlen(msg) + 1, 0);

    // recv string
    // To do need a while loop to get all message
    recv(clientfd, buf, 4096, 0);
    printf("%s\n", buf);

    // clost connection
    close(clientfd);
    return 0;
}
