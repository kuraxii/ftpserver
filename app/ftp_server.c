/*********************************************
 * @FileName: ftp_server.c
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 14:41:08 2023
 *********************************************/

#include <session_thread.h>
#include <ftp_server.h>
#include <common.h>
#include <ftp_cmd.h>
#include <arpa/inet.h>
#include <bits/posix2_lim.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <ftp_log.h>



int main()
{
    int ret;
    pthread_t pid;

    int lfd = sockinit(NULL, 2121);
    log_init();

    
    Socket sock;
    sock.len = sizeof(sock.addr);
    while (1)
    {

        int cfd = accept(lfd, (struct sockaddr *)&sock.addr, &sock.len);
        if (cfd == -1)
        {
            perror("accept");
            exit(-1);
        }
        sock.socketFd = cfd;

        ret = pthread_create(&pid, NULL, session_thread, (void *)&sock);
        if (ret != 0)
        {
            printf("%s\n", strerror(ret));
            exit(-1);
        }
        ret = pthread_detach(pid);
        if (ret != 0)
        {
            printf("%s\n", strerror(ret));
            exit(-1);
        }
    }
    pthread_exit(NULL);
}

int sockinit(char *ipaddr, unsigned short port) // 初始化服务端套接字
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
    {
        perror("socket");
        return -1;
    }

    int opt = 1, ret;
    ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret == -1)
    {
        perror("setsockopt");
        exit(-1);
    }

    struct sockaddr_in seraddr;
    bzero(&seraddr, sizeof(seraddr));
    seraddr.sin_addr.s_addr = INADDR_ANY;
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(port);
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