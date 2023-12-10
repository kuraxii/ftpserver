/*********************************************
 * @FileName: ftp_server.h
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 14:41:20 2023
 *********************************************/



#include <arpa/inet.h>
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


#ifndef __FTP_SERVER_H
#define __FTP_SERVER_H
#define MAX_AUTH_FAILS 3;

typedef struct Socket
{
    int socketFd;
    FILE* fpIn;
    FILE* fpOut;
    struct sockaddr_in addr; // sddr port
    socklen_t len;
    int isConn;

} Socket;

int sockinit(char *ipaddr, unsigned short port); // 初始化服务端套接字




#endif
