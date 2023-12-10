#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
typedef struct
{
    int cfd;
    struct sockaddr_in cli_addr;
    socklen_t addrlen;
} SOCKMSG;

void *thread(void *msg);
int sockinit(char *ipaddr, unsigned short port);

int main(int argc, char *argv[])
{
    int ret;
    pthread_t tid;

    int lfd = sockinit(NULL, 11111);

    SOCKMSG cli_msg;
    bzero(&cli_msg, sizeof(cli_msg));
    cli_msg.addrlen = sizeof(cli_msg.addrlen);

    while (1)
    {
        int cfd = accept(lfd, (struct sockaddr *)&cli_msg.cli_addr, &cli_msg.addrlen);
        if (cfd == -1)
        {
            perror("accept");
            exit(-1);
        }
        cli_msg.cfd = cfd;

        printf("client ip: %s, port: %d connect success\n", inet_ntoa(cli_msg.cli_addr.sin_addr),
               ntohs(cli_msg.cli_addr.sin_port));

        ret = pthread_create(&tid, NULL, thread, (void *)&cli_msg);
        if (ret != 0)
        {
            printf("%s\n", strerror(ret));
            exit(-1);
        }
        ret = pthread_detach(tid);
        if (ret != 0)
        {
            printf("%s\n", strerror(ret));
            exit(-1);
        }
    }
    pthread_exit(NULL);
}

void *thread(void *msg)
{
    SOCKMSG *p = (SOCKMSG *)msg;
    SOCKMSG climsg;
    climsg.cfd = p->cfd;
    climsg.cli_addr = p->cli_addr;
    climsg.addrlen = p->addrlen;
    FILE *fpIn = fdopen(climsg.cfd, "r");
    FILE *fpOut = fdopen(dup(climsg.cfd), "w");
    setlinebuf(fpIn);
    setlinebuf(fpOut);

    int ret;
    char buf[1024] = {0};
    char *err;
    while (1)
    {
        // ret = read(climsg.cfd, buf, sizeof(buf));
        // if (ret == -1)
        // {
        //     perror("read");
        //     pthread_exit(NULL);
        // }
        // if (ret == 0)
        // {
        //     printf("client ip: %s, port: %d disconnect\n", inet_ntoa(climsg.cli_addr.sin_addr),
        //            ntohs(climsg.cli_addr.sin_port));
        //     close(climsg.cfd);
        //     pthread_exit(NULL);
        // }

        bzero(buf, sizeof(buf));
        err = fgets(buf, sizeof(buf), fpIn);
        if (err == NULL)
        {
            close(climsg.cfd);
            pthread_exit(NULL);
        }
        printf("buf: %s\n", buf);
        ret = fputs(buf, fpOut);

        if (ret == EOF)
            perror("fput");

        // write(climsg.cfd, buf, strlen(buf));
    }
}
int sockinit(char *ipaddr, unsigned short port)
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return -1;
    }
    struct sockaddr_in seraddr;
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_addr.s_addr = INADDR_ANY;
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(port);

    int opt = 1, ret;
    ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ret = bind(lfd, (struct sockaddr *)&seraddr, sizeof(seraddr));
    if (ret == -1)
    {
        perror("bind");
        exit(-1);
    }
    ret = listen(lfd, 255);
    if (ret == -1)
    {
        perror("listen");
        exit(-1);
    }
    return lfd;
}