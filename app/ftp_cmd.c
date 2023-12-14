/*********************************************
 * @FileName: ftp_cmd.c
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:24:53 2023
 *********************************************/

#include <errno.h>
#include <ftp_cmd.h>
#include <ftp_server.h>
#include <libgen.h>
#include <log.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

const char *map[] = {"SYST", "USER", "PASS", "CWD", "PWD",  "LIST", "PASV", "PORT", "RETR",
                     "STOR", "DELE", "RMD",  "MKD", "QUIT", "SIZE", "FEAT", "TYPE"};

const char *administrator[2] = {"zzj", "123456"}; // 默认账号

// 通过文件描述符获取服务器ip
void getip(int sockfd, int *ip)
{
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    getsockname(sockfd, (struct sockaddr *)&addr, &addr_size);

    char *host = inet_ntoa(addr.sin_addr);
    log_trace("function: getip: ip: %s", host);
    sscanf(host, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

void get_port(int *port)
{
    srand(time(NULL));
    port[0] = 128 + (rand() % 64);
    port[1] = rand() % 0xff;
}

void struct_path(char *path, const char *workDir, const char *arg)
{
    if (arg[0] == '/')
        strcpy(path, arg);
    else
        sprintf(path, "%s/%s", workDir, arg);
}

// 返回系统信息
void syst_run(FtpCmd *ftpCmd)
{

    SessionInfo *sess = ftpCmd->sess;
    char *response = "215 UNIX Type: L8\r\n";
    log_info("function: syst_run: SYST executing");
    send_by_cmd(sess, response);
    log_info("function: syst_run: SYST finished");
}

// 用户
void user_run(FtpCmd *ftpCmd)
{

    SessionInfo *sess = ftpCmd->sess;
    char *response = "331 Send password\r\n";
    log_info("function: user_run: USER executing");
    send_by_cmd(sess, response);
    log_info("function: user_run: USER finished");
}

// 密码
void pass_run(FtpCmd *ftpCmd)
{

    SessionInfo *sess = ftpCmd->sess;
    char *response = "230 Access granted\r\n";
    log_info("function: pass_run: PASS executing");
    send_by_cmd(sess, response);
    log_info("function: pass_run: PASS finished");
}

// 切换目录
void cwd_run(FtpCmd *ftpCmd)
{
    int err;

    SessionInfo *sess = ftpCmd->sess;
    char path[256];
    struct stat statbuf;
    char *errStr = NULL;

    log_info("function: cwd_run: CWD executing");
    // 访问到根目录以外的目录
    if ((ftpCmd->arg[0] == '/' && strcmp(sess->rootDir, ftpCmd->arg) > 0) ||
        (!strcmp(ftpCmd->arg, "..") && !strcmp(sess->rootDir, sess->workDir)))
    {
        errStr = "550 Invalid name or chroot violation\r\n";
        send_by_cmd(sess, errStr);
        goto err;
    }

    struct_path(path, sess->workDir, ftpCmd->arg);

    err = stat(path, &statbuf);
    // 没有这个文件
    if (err < 0 || !S_ISDIR(statbuf.st_mode))
    {
        errStr = "550 Invalid name\r\n";
        send_by_cmd(sess, errStr);
        goto err;
    }
    if (ftpCmd->arg[0] == '/')
    {
        strcpy(sess->workDir, ftpCmd->arg);
    }
    else
    {
        if (!strcmp(ftpCmd->arg, ".."))
            dirname(sess->workDir);
        else if (strcmp(ftpCmd->arg, "."))
        {
            strcat(sess->workDir, "/");
            strcat(sess->workDir, ftpCmd->arg);
        }
    }

    send_by_cmd(sess, "250 CWD successful\r\n");
    log_info("function: cwd_run: CWD complete, workFile%s", sess->workDir);
    return;

err:
    log_warn("function: cwd_run: %s", errStr);
}

// 当前目录
void pwd_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char response[512];

    log_info("function: pwd_run: PWD executing");
    snprintf(response, sizeof(response), "257 \"%s\"\r\n", sess->workDir);

    send_by_cmd(sess, response);

    log_info("function: pwd_run: PWD complete, %s", ftpCmd->arg);
}

// 目录列表
void list_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char cmd[512];
    char buf[256];
    char path[256];
    char *err;
    int ret;
    struct stat statbuf;
    FILE *fp = NULL;
    int len;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";

        log_warn("function: pasv_run: USER don't logged");
        goto notlogged;
    }

    if (sess->isPasv)
    { // 被动模式

        if (*ftpCmd->arg == '\0')
        {
            strcpy(path, sess->workDir);
        }
        else
        {
            struct_path(path, sess->workDir, ftpCmd->arg);
        }

        ret = stat(path, &statbuf);
        if (ret == -1 || !S_ISDIR(statbuf.st_mode))
        {
            response = "550 Failed to open file.\r\n";
            log_error("function: retr_run: open file: %s error", path);
            goto open_err;
        }

        log_trace("path: %s", path);

        sprintf(cmd, "ls -l %s", path);

        if (access(path, R_OK) || (fp = popen(cmd, "r")) == NULL)
        {
            response = "550 Failed to open file.\r\n";
            log_error("function: retr_run: open file: %s error", path);

            goto open_err;
        }

        sess->dataSocket->socketFd =
            accept(sess->dataSocket->listenFd, (struct sockaddr *)&sess->dataSocket->addr, &sess->dataSocket->len);
        if (sess->dataSocket->socketFd < 0)
        {
            log_error("function: retr_run: accept error, %s", strerror(sess->dataSocket->socketFd));
            response = "550 Failed to accept.\r\n";
            goto accept_err;
        }

        close(sess->dataSocket->listenFd);
        sess->dataSocket->listenFd = -1;
        sess->isPasv = 0;
        log_trace("function: retr_run: set ispasv: 0");

        sess->dataSocket->fpOut = fdopen(sess->dataSocket->socketFd, "w");

        response = "150 Send file.\n";
        send_by_cmd(sess, response);

        while (1)
        {
            err = fgets(buf, sizeof(buf), fp);
            if (err == NULL)
            {
                if (ferror(fp))
                {
                    log_error("function: retr_run: read file: %s error", path);
                    response = "550 Failed to read file.\r\n";
                    goto read_err;
                }
                log_info("function: retr_run: read end of file: %s", path);
                response = "226 Transmission finished.\r\n";
                goto end_of_file;
            }

            len = strlen(buf);
            if (buf[len - 1] == '\n')
            {
                buf[len - 1] = '\r';
                buf[len] = '\n';
                buf[len + 1] = '\0';
            }
            fputs(buf, sess->dataSocket->fpOut);
        }
    }
    else if (sess->isPort)
    {
        // 主动模式
    }

read_err:
end_of_file:
    fclose(fp);
    fclose(sess->dataSocket->fpOut);

accept_err:
open_err:
notlogged:
    if (sess->isPasv)
    {
        close(sess->dataSocket->listenFd);
        sess->dataSocket->listenFd = -1;
        sess->isPasv = 0;
        log_trace("function: retr_run: set ispasv: 0");
    }
    send_by_cmd(sess, response);
}

// 被动模式
void pasv_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    int ip[4];
    int port[2];
    char *response = NULL;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";
        strcpy(sess->messsge, response);
        log_warn("function: pasv_run: USER don't logged");
        return;
    }

    log_info("function: pasv_run: PASV set start");

    getip(sess->cmdSocket->socketFd, ip);
    get_port(port);

    // 诺监听描述存在则关闭
    if (sess->isPasv)
        close(sess->dataSocket->listenFd);

    sess->dataSocket->listenFd = sockinit(NULL, port[0] * 256 + port[1]);
    sess->isPasv = 1;
    log_trace("function: retr_run: set ispasv: 1");

    sprintf(sess->messsge, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", ip[0], ip[1], ip[2], ip[3], port[0],
            port[1]);

    send_by_cmd(sess, sess->messsge);
    log_info("function: pasv_run: PASV set complete");
}

// @TODO   主动模式
void port_run(FtpCmd *ftpCmd)
{
    // SessionInfo *sess = ftpCmd->sess;
    // int ip[4];
    // int port[2];
    // char *response = NULL;

    // if (!sess->islogged)
    // {
    //     response = "530 Please login with USER and PASS.\r\n";
    //     strcpy(sess->messsge, response);
    //     log_warn("function: pasv_run: USER don't logged");
    //     return;
    // }

    // log_info("function: pasv_run: PASV set start");

    // getip(sess->cmdSocket->socketFd, ip);
    // get_port(port);

    // // 诺监听描述存在则关闭
    // if (sess->dataSocket->listenFd != -1)
    //     close(sess->dataSocket->listenFd);

    // sess->dataSocket->listenFd = sockinit(NULL, port[0] * 256 + port[1]);
    // sess->isPasv = 1;
    // sprintf(sess->messsge, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", ip[0], ip[1], ip[2], ip[3], port[0],
    //         port[1]);

    // send_by_cmd(sess, sess->messsge);
    // log_info("function: pasv_run: PASV set complete");
}

// 从服务器下载文件
void retr_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char path[255];
    char buf[4096];
    char *err;
    int ret;
    struct stat statbuf;
    FILE *fp = NULL;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";

        log_warn("function: retr_run: USER don't logged");
        goto notlogged;
    }

    if (sess->isPasv)
    { // 被动模式
        struct_path(path, sess->workDir, ftpCmd->arg);

        ret = stat(path, &statbuf);
        if (ret == -1 || S_ISDIR(statbuf.st_mode))
        {
            response = "550 Failed to open file.\r\n";
            log_error("function: retr_run: open file: %s error", path);
            goto open_err;
        }

        log_trace("path: %s", path);

        if (access(path, R_OK) || (fp = fopen(path, "r")) == NULL)
        {
            response = "550 Failed to open file.\r\n";
            log_error("function: retr_run: open file: %s error", path);

            goto open_err;
        }

        sess->dataSocket->socketFd =
            accept(sess->dataSocket->listenFd, (struct sockaddr *)&sess->dataSocket->addr, &sess->dataSocket->len);
        if (sess->dataSocket->socketFd < 0)
        {
            log_error("function: retr_run: accept error, %s", strerror(sess->dataSocket->socketFd));
            response = "550 Failed to accept.\r\n";
            goto accept_err;
        }

        close(sess->dataSocket->listenFd);
        sess->dataSocket->listenFd = -1;
        sess->isPasv = 0;
        log_trace("function: retr_run: set ispasv: 0");

        sess->dataSocket->fpOut = fdopen(sess->dataSocket->socketFd, "w");

        response = "150 Send file.\n";
        send_by_cmd(sess, response);

        while (1)
        {
            err = fgets(buf, sizeof(buf), fp);
            if (err == NULL)
            {
                if (ferror(fp))
                {
                    log_error("function: retr_run: read file: %s error", path);
                    response = "550 Failed to read file.\r\n";
                    goto read_err;
                }
                log_info("function: retr_run: read end of file: %s", path);
                response = "226 Transmission finished.\r\n";
                goto end_of_file;
            }

            fputs(buf, sess->dataSocket->fpOut);
        }
    }
    else if (sess->isPort)
    {
        // 主动模式
    }

read_err:
end_of_file:
    fclose(fp);
    fclose(sess->dataSocket->fpOut);

accept_err:
open_err:
notlogged:
    if (sess->isPasv)
    {
        close(sess->dataSocket->listenFd);
        sess->dataSocket->listenFd = -1;
        sess->isPasv = 0;

        log_trace("function: retr_run: set ispasv: 0");
    }
    send_by_cmd(sess, response);
}

// @TODO reponse格式 上传文件到服务器
void stor_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char path[255];
    char buf[4096];
    char *err;
    int ret;
    struct stat statbuf;
    FILE *fp = NULL;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";

        log_warn("function: stor_run: USER don't logged");
        goto notlogged;
    }

    if (sess->isPasv)
    { // 被动模式
        struct_path(path, sess->workDir, ftpCmd->arg);

        ret = stat(path, &statbuf);
        if (ret == -1 || S_ISDIR(statbuf.st_mode))
        {
            response = "550 Failed to open file.\r\n";
            log_error("function: stor_run: open file: %s error", path);
            goto open_err;
        }

        log_trace("path: %s", path);

        if (access(path, R_OK) || (fp = fopen(path, "w")) == NULL)
        {
            response = "550 Failed to open file.\r\n";
            log_error("function: stor_run: open file: %s error", path);

            goto open_err;
        }

        sess->dataSocket->socketFd =
            accept(sess->dataSocket->listenFd, (struct sockaddr *)&sess->dataSocket->addr, &sess->dataSocket->len);
        if (sess->dataSocket->socketFd < 0)
        {
            log_error("function: stor_run: accept error, %s", strerror(sess->dataSocket->socketFd));
            response = "550 Failed to accept.\r\n";
            goto accept_err;
        }

        close(sess->dataSocket->listenFd);
        sess->dataSocket->listenFd = -1;
        sess->isPasv = 0;
        log_trace("function: retr_run: set ispasv: 0");

        sess->dataSocket->fpOut = fdopen(sess->dataSocket->socketFd, "w");

        response = "150 Send file.\n";
        send_by_cmd(sess, response);

        while (1)
        {
            err = fgets(buf, sizeof(buf), sess->dataSocket->fpOut);
            if (err == NULL)
            {
                if (ferror(sess->dataSocket->fpOut))
                {
                    log_error("function: stor_run: read file: %s error", path);
                    response = "550 Failed to read file.\r\n";
                    goto read_err;
                }
                log_info("function: stor_run: read end of file: %s", path);
                response = "226 Transmission finished.\r\n";
                goto end_of_file;
            }

            fputs(buf, fp);
        }
    }
    else if (sess->isPort)
    {
        // 主动模式
    }

read_err:
end_of_file:
    fclose(fp);
    fclose(sess->dataSocket->fpOut);

accept_err:
open_err:
notlogged:
    if (sess->isPasv)
    {
        close(sess->dataSocket->listenFd);
        sess->dataSocket->listenFd = -1;
        sess->isPasv = 0;
        log_trace("function: retr_run: set ispasv: 0");
    }
    send_by_cmd(sess, response);
}
// 删除文件
void dele_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char path[256];
    int ret;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";
        log_warn("function: dele_run: USER don't logged");
        goto err;
    }

    struct_path(path, sess->workDir, ftpCmd->arg);

    if ((ret = unlink(path)) < 0)
    {
        response = "550 Failed to delete file.\r\n";
        log_warn("function: dele_run: FIle dele err, %s", strerror(ret));
        goto err;
    }

    response = "250 Requested file action okay, completed.\r\n";

err:
    send_by_cmd(sess, response);
}

// 删除文件夹
void rmd_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char path[256];
    int ret;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";
        log_warn("function: rmd_run: USER don't logged");
        goto err;
    }

    struct_path(path, sess->workDir, ftpCmd->arg);

    if ((ret = rmdir(path)) < 0)
    {
        response = "550 Failed to delete file.\r\n";
        log_warn("function: rmd_run: dir rm err, %s", strerror(ret));
        goto err;
    }

    response = "250 Requested file action okay, completed.\r\n";

err:
    send_by_cmd(sess, response);
}

// @TODO  reponse 创建文件夹
void mkd_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char path[256];
    int ret;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";
        log_warn("function: mkd_run: USER don't logged");
        goto err;
    }

    struct_path(path, sess->workDir, ftpCmd->arg);

    if ((ret = rmdir(path)) < 0)
    {
        response = "550 Failed to delete file.\r\n";
        log_warn("function: mkd_run: USER don't logged");
        goto err;
    }

    response = "550 Cannot delete directory.\r\n";

err:
    send_by_cmd(sess, response);
}

// 退出
void quit_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    ftpCmd->sess->isRun = 0;
    send_by_cmd(sess, "211 Goodbye\r\n");
}

// @TODO  reponse  获取文件大小
void size_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    char *response = NULL;
    char path[256];
    int ret;
    size_t size;
    struct stat statbuf;

    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";

        log_warn("function: size_run: USER don't logged");
        goto notlogged;
    }

    struct_path(path, sess->workDir, ftpCmd->arg);

    if ((ret = stat(path, &statbuf)) == -1)
    {
        response = "550 Failed to open file.\r\n";
        log_error("function: size_run: open file: %s error", path);
        goto open_err;
    }

    size = statbuf.st_size;
    response = "550 Failed to open file.\r\n";
    sprintf(sess->messsge, "232 %lu\r\n", size);
    log_info("function: size_run: get size success");

    send_by_cmd2(sess);
    return;
open_err:
notlogged:
    send_by_cmd(sess, response);
}

// 获取支持信息
void feat_run(FtpCmd *ftpCmd)
{
    SessionInfo *sess = ftpCmd->sess;
    log_info("function: feat_run: Giving FEAT");
    send_by_cmd(sess, "211-Features supported\r\n");
    send_by_cmd(sess, " UTF-8\r\n");
    send_by_cmd(sess, " SIZE\r\n");
    send_by_cmd(sess, "211 End\r\n");
    log_info("function: feat_run: Gave FEAT");
}

void type_run(FtpCmd *ftpCmd)
{

    SessionInfo *sess = ftpCmd->sess;

    char *response = NULL;
    if (!sess->islogged)
    {
        response = "530 Please login with USER and PASS.\r\n";
        goto err;
    }

    if (ftpCmd->arg[0] == 'I')
    {
        response = "200 Switching to Binary mode.\r\n";
    }
    else if (ftpCmd->arg[0] == 'A')
    {

        /* Type A must be always accepted according to RFC */
        response = "200 Switching to ASCII mode.\r\n";
    }
    else
    {
        response = "504 Command not implemented for that parameter.\r\n";
    }
err:
    send_by_cmd(sess, response);
}

void cmd_structor(FtpCmd *ftpCmd)
{

    SessionInfo *sess = ftpCmd->sess;
    char *ptrptr = NULL;
    char *token = NULL;
    char *rest = NULL;
    bzero(ftpCmd->cmd, sizeof(ftpCmd->cmd));
    bzero(ftpCmd->arg, sizeof(ftpCmd->arg));

    sess->cmdBuf[strlen(sess->cmdBuf) - 2] = '\0';

    rest = sess->cmdBuf;
    token = strtok_r(rest, " ", &ptrptr);
    strcpy(ftpCmd->cmd, token);
    if (ptrptr != NULL)
    {
        strcpy(ftpCmd->arg, ptrptr);
    }

    log_info("function: cmd_structor: cmd:%s, arg:%s.", ftpCmd->cmd, ftpCmd->arg);
}

void ftp_cmd_init(FtpCmd *ftpCmd, struct SessionInfo *sess)
{

    // 将功能模块装入函数列表
    void (**func)(FtpCmd *) = calloc(COMMAND_COUNT, sizeof(void (*)(void *)));
    func[0] = syst_run;
    func[1] = user_run;
    func[2] = pass_run;
    func[3] = cwd_run;
    func[4] = pwd_run;
    func[5] = list_run;
    func[6] = pasv_run;
    func[7] = port_run;
    func[8] = retr_run;
    func[9] = stor_run;
    func[10] = dele_run;
    func[11] = rmd_run;
    func[12] = mkd_run;
    func[13] = quit_run;
    func[14] = size_run;
    func[15] = feat_run;
    func[16] = type_run;
    ftpCmd->cmdMap.cmdFunc = func;

    ftpCmd->sess = sess;
    ftpCmd->cmdMap.Map = map;
}

void ftp_cmd_exit(FtpCmd *ftpCmd)
{

    free(ftpCmd->cmdMap.cmdFunc);
    free(ftpCmd);
}
