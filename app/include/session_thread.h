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

typedef struct SessionInfo
{
    char userName[20];    // 用户名
    char workDir[255];    // 当前工作目录
    char rootDir[255];   // 根目录
    char cmdBuf[255];     // cmd缓冲区
    char messsge[255];    // 发送消息缓冲区
    int isPasv;           // 被动模式
    int isPort;           // 主动模式
    int isRun;            // 是否运行
    int islogged;         // 是否登录
    int isDataSocketConn; // 数据socket是否连接
    Socket *dataSocket;   // 数据socket
    Socket *cmdSocket;    // 命令socket
  
} SessionInfo;

void *session_thread(void *arg);

SessionInfo* session_init(SessionInfo *, Socket *);
void session_exit(SessionInfo *);
void welccome(SessionInfo *);
int recv_by_cmd(SessionInfo *);
int send_by_cmd(SessionInfo *, char *);
int send_by_cmd2(SessionInfo *);

#endif
