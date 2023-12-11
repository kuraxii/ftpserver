/*********************************************
 * @FileName: session_thread.h
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:31:29 2023
 *********************************************/
#include "ftp_server.h"

#ifndef __SESSION_THREAD_H
#define __SESSION_THREAD_H

void set_session_method(void *); // 装入函数

void session_init(void *, Socket *);
void session_exit(void *);
void welccome(void *);
int recv_by_cmd(void *_self);
int send_by_cmd(void *_self, char *response);
typedef struct SessionInfo
{
    char userName[20];  // 用户名
    char workFile[255]; // 当前工作目录
    char rootFile[255]; // 根目录
    char cmdBuf[255];   // cmd缓冲区
    char messsge[255];  // 发送消息缓冲区
    int isPasv;         // 被动模式
    int isRun;          // 是否运行
    int islogged;       // 是否登录
    int isDataSocketConn;   // 数据socket是否连接
    Socket *dataSocket; // 数据socket
    Socket *cmdSocket;  // 命令socket
    void (*__init)(void *, Socket *);
    void (*__exit)(void *);
} SessionInfo;

void *session_thread(void *arg);

#endif
