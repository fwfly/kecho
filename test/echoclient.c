#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

struct thread_info {
    pthread_t thread_id;
    int thread_num;
    char *argv_string;
};

static void *request_worker(void *argv)
{
    struct thread_info *info = argv;
    int clientfd;
    struct addrinfo hints, *listp, *p;
    char *msg;
    char buf[4096];

    msg = info->argv_string;

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
            return;
        }
    }
    freeaddrinfo(listp);

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
}

int main(int argc, char **argv)
{
    char *cnum_threads = argv[1], *msg = argv[2];
    int num_threads = atoi(cnum_threads);
    struct thread_info *tinfo;
    pthread_attr_t attr;
    void *res;

    pthread_attr_init(&attr);
    tinfo = calloc(num_threads, sizeof(struct thread_info));
    for (int num = 0; num < num_threads; num++) {
        tinfo[num].thread_num = num + 1;
        tinfo[num].argv_string = msg;
        pthread_create(&tinfo[num].thread_id, &attr, &request_worker,
                       &tinfo[num]);
    }

    pthread_attr_destroy(&attr);

    for (int num = 0; num < num_threads; num++) {
        pthread_join(tinfo[num].thread_id, &res);
        free(res);
    }
    free(tinfo);
    return 0;
}
