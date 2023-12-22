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
    SessionInfo *sess = calloc(1, sizeof(SessionInfo));

    session_init(sess, sock);
    // // 功能测试 默认已登录
    sess->islogged = 1;
   
    ftp_cmd_init(ftpCmd, sess);

    welccome(sess);

    while (sess->isRun)
    {
        ret = recv_by_cmd(sess);
        if (ret == -1)
        {
            goto exit;
        }
        cmd_structor(ftpCmd);
        for (i = SYST; i < COMMAND_COUNT; i++)
        {
            if (!strcmp(ftpCmd->cmdMap.Map[i], ftpCmd->cmd))
            {
                // 未登录状态 除了接收到USER 否则返回  530 Login first with USER and PASS
                if (!sess->islogged && i != USER)
                {

                    send_by_cmd(sess, "530 Login first with USER and PASS\r\n");
                    log_info("Operation failed, Not logged in. IP: %s", inet_ntoa(sess->cmdSocket->addr.sin_addr));
                    break;
                }
                ftpCmd->cmdMap.cmdFunc[i](ftpCmd);
                break;
            }
        }
        if (i == COMMAND_COUNT)
        {
            send_by_cmd(sess, "502 Comman not recognized\r\n");
            log_info("Operation failed, Invalid command. Command: %s ,IP: %s", ftpCmd->cmd,
                     inet_ntoa(sess->cmdSocket->addr.sin_addr));
        }
    }

exit:
    log_info("session thread exit, ip: %s", inet_ntoa(sess->cmdSocket->addr.sin_addr));
    session_exit(sess);
    ftp_cmd_exit(ftpCmd);

    pthread_exit(NULL);
}

void session_init(SessionInfo *sess, Socket *p_sock)
{

    char *home_dir = getenv("HOME");
    sess->cmdSocket = calloc(1, sizeof(Socket));
    sess->dataSocket = calloc(1, sizeof(Socket));
    memcpy(sess->cmdSocket, p_sock, sizeof(Socket));
    memcpy(sess->rootDir, home_dir, strlen(home_dir));
    memcpy(sess->workDir, home_dir, strlen(home_dir));

    sess->cmdSocket->fpIn = fdopen(p_sock->socketFd, "r");
    if (ferror(sess->cmdSocket->fpIn))
    {
        printf("fdopen发生错误\n");
    }
    sess->cmdSocket->fpOut = fdopen(dup(p_sock->socketFd), "w");
    if (ferror(sess->cmdSocket->fpOut))
    {
        printf("fdopen发生错误\n");
    }
    setlinebuf(sess->cmdSocket->fpIn);
    setlinebuf(sess->cmdSocket->fpOut);

    log_info("session init");

    sess->isRun = 1;
    sess->isPasv = 0;
    
}

void session_exit(SessionInfo *sess)
{

    fclose(sess->cmdSocket->fpIn);
    fclose(sess->cmdSocket->fpOut);
    free(sess->cmdSocket);
    // free(self->dataSocket);
    free(sess);
}

void welccome(SessionInfo *sess)
{

    send_by_cmd(sess, "220 zjFTP 0.0.0.1 ready\r\n");
}

int send_by_cmd(SessionInfo *sess, char *response) // 以cmd套接字发送数据
{
    int err;
    log_info("function: send_by_cmd: send by cmd start");
    err = fputs(response, sess->cmdSocket->fpOut);
    if (err == EOF)
    {
        log_warn("fputs error");
        return -1;
    }
    log_info("function: send_by_cmd: send by cmd success. string: %s", response);

    return 0;
}

int send_by_cmd2(SessionInfo *sess) //
{

    int err;

    log_info("function: send_by_cmd: send by cmd start");
    err = fputs(sess->messsge, sess->cmdSocket->fpOut);
    if (err == EOF)
    {
        log_warn("fputs error");
        return -1;
    }
    log_info("function: send_by_cmd: send by cmd success. string: %s", sess->messsge);

    return 0;
}
int recv_by_cmd(SessionInfo *sess) // 以cmd套接字接收数据
{
    char *err;
    int ret;
    log_info("function: recv_by_cmd: recv by cmd start");
    bzero(sess->cmdBuf, sizeof(sess->cmdBuf));
    err = fgets(sess->cmdBuf, sizeof(sess->cmdBuf), sess->cmdSocket->fpIn);
    if (err == NULL)
    {
        log_warn("fgets error or socket close");
        ret = -1;
        goto exit;
    }
    ret = strlen(sess->cmdBuf);
    log_info("function: recv_by_cmd: recv by cmd success, string: %s", sess->cmdBuf);

exit:
    return ret;
}
