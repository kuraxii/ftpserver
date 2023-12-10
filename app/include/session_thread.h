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
void session_welccome(void *);
typedef struct SessionInfo
{
    char userName[20];  // 用户名
    char workFile[255]; // 当前工作目录
    char rootFile[255]; // 根目录
    char cmdBuf[255];   // cmd缓冲区
    
    int isPasv;         // 被动模式
    int isRun;          // 是否运行
    int islogged;       // 是否登录
    int isDataSocketConn;   // 数据socket是否连接
    Socket *dataSocket; // 数据socket
    Socket *cmdSocket;  // 命令socket
    void (*__init)(void *, Socket *);
    void (*__exit)(void *);
    void (*__welccome)(void *);            // 欢迎提示
    int (*__send_by_cmd)(void *, char *);  // 以cmd套接字发送数据
    int (*__recv_by_cmd)(void *);          // 以cmd套接字接收数据
    int (*__send_by_data)(void *, char *); // 以data套接字发送数据
    int (*__recv_by_data)(void *);         // 以data套接字接收数据

} SessionInfo;

void *session_thread(void *arg);

#endif
