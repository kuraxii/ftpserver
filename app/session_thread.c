/*********************************************
 * @FileName: session_thread.c
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:31:13 2023
 *********************************************/
#include "log.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <fcntl.h>
#include <ftp_cmd.h>
#include <ftp_log.h>
#include <netinet/in.h>
#include <pthread.h>
#include <session_thread.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void *session_thread(void *arg)
{
    enum CmdNo i;
    int ret;
    Socket *sock = (Socket *)arg;

    FtpCmd *ftpCmd = calloc(1, sizeof(FtpCmd));
    SessionInfo *self = calloc(1, sizeof(SessionInfo));

    set_session_method(self);
    self->__init(self, sock);
    // // 功能测试 默认已登录
    self->islogged = 1;
    set_ftp_cmd_method(ftpCmd);
    ftpCmd->__init(ftpCmd, self);

    welccome(self);

    while (self->isRun)
    {
        ret = recv_by_cmd(self);
        if (ret == -1)
        {
            goto exit;
        }

#if 1
        cmd_structor(ftpCmd);
        for (i = SYST; i < COMMAND_COUNT; i++)
        {
            if (!strcmp(ftpCmd->cmdMap.Map[i], ftpCmd->cmd))
            {
                // 未登录状态 除了接收到USER 否则返回  530 Login first with USER and PASS
                if (!self->islogged && i != USER)
                {

                    send_by_cmd(self, "530 Login first with USER and PASS\r\n");
                    log_info("Operation failed, Not logged in. IP: %s", inet_ntoa(self->cmdSocket->addr.sin_addr));
                    break;
                }
                ftpCmd->cmdMap.cmdFunc[i](ftpCmd);
                break;
            }
        }
#endif
        if (i == COMMAND_COUNT)
        {
            send_by_cmd(self, "502 Comman not recognized\r\n");
            log_info("Operation failed, Invalid command. Command: %s ,IP: %s", ftpCmd->cmd,
                     inet_ntoa(self->cmdSocket->addr.sin_addr));
        }
    }

exit:
    log_info("session thread exit, ip: %s", inet_ntoa(self->cmdSocket->addr.sin_addr));
    self->__exit(self);
    ftpCmd->__exit(ftpCmd);

    pthread_exit(NULL);
}

void session_init(void *_self, Socket *p_sock)
{
    SessionInfo *self = _self;
    char *home_dir = getenv("HOME");
    self->cmdSocket = calloc(1, sizeof(Socket));
    self->dataSocket = calloc(1, sizeof(Socket));
    memcpy(self->cmdSocket, p_sock, sizeof(Socket));
    memcpy(self->rootFile, home_dir, strlen(home_dir));
    memcpy(self->workFile, home_dir, strlen(home_dir));

    self->cmdSocket->fpIn = fdopen(p_sock->socketFd, "r");
    if (ferror(self->cmdSocket->fpIn))
    {
        printf("fdopen发生错误\n");
    }
    self->cmdSocket->fpOut = fdopen(dup(p_sock->socketFd), "w");
    if (ferror(self->cmdSocket->fpOut))
    {
        printf("fdopen发生错误\n");
    }
    setlinebuf(self->cmdSocket->fpIn);
    setlinebuf(self->cmdSocket->fpOut);

    log_info("session init");

    self->isRun = 1;
}

void session_exit(void *_self)
{
    SessionInfo *self = _self;

    fclose(self->cmdSocket->fpIn);
    fclose(self->cmdSocket->fpOut);
    free(self->cmdSocket);
    // free(self->dataSocket);
    free(_self);
}

void welccome(void *_self)
{
    SessionInfo *self = _self;
    send_by_cmd(self, "220 zjFTP 0.0.0.1 ready\r\n");
}

int send_by_cmd(void *_self, char *response) // 以cmd套接字发送数据
{
    SessionInfo *self = _self;
    int err;

    log_info("function: send_by_cmd: send by cmd start");
    err = fputs(response, self->cmdSocket->fpOut);
    if (err == EOF)
    {
        log_warn("fputs error");
        return -1;
    }
    log_info("function: send_by_cmd: send by cmd success. string: %s", response);

    return 0;
}
int recv_by_cmd(void *_self) // 以cmd套接字接收数据
{
    SessionInfo *self = _self;
    char *err;
    int ret;
    log_info("function: recv_by_cmd: recv by cmd start");
    bzero(self->cmdBuf, sizeof(self->cmdBuf));
    err = fgets(self->cmdBuf, sizeof(self->cmdBuf), self->cmdSocket->fpIn);
    if (err == NULL)
    {
        log_warn("fgets error or socket close");
        ret = -1;
        goto exit;
    }
    ret = strlen(self->cmdBuf);
    log_info("function: recv_by_cmd: recv by cmd success, string: %s", self->cmdBuf);

exit:
    return ret;
}

void set_session_method(void *_self)
{
    SessionInfo *self = _self;
    self->__init = session_init;
    self->__exit = session_exit;

}
