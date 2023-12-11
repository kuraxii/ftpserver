/*********************************************
 * @FileName: ftp_cmd.c
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:24:53 2023
 *********************************************/

#include "log.h"
#include <ftp_cmd.h>
#include <libgen.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

const char *map[] = {
    "SYST", "USER", "PASS", "CWD", "PWD", "LIST", "PASV", "PORT",
    "RETR", "STOR", "DELE", "RMD", "MKD", "QUIT", "SIZE", "FEAT",
};

const char *administrator[2] = {"zzj", "123456"}; // 默认账号

// 返回系统信息
void syst_run(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    char *response = "215 UNIX Type: L8\r\n";
    log_info("function: syst_run: SYST executing");
    sess->__send_by_cmd(sess, response);
    log_info("function: syst_run: SYST finished");
}

// 用户
void user_run(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    char *response = "331 Send password\r\n";
    log_info("function: user_run: USER executing");
    sess->__send_by_cmd(sess, response);
    log_info("function: user_run: USER finished");
}

// 密码
void pass_run(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    char *response = "230 Access granted\r\n";
    log_info("function: pass_run: PASS executing");
    sess->__send_by_cmd(sess, response);
    log_info("function: pass_run: PASS finished");
}

// 切换目录
void cwd_run(void *_self)
{
    int err;
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    char path[256];
    struct stat statbuf;
    char *errStr = NULL;

    log_info("function: cwd_run: CWD executing");
    // 访问到根目录以外的目录
    if ((self->cmd[0] == '/' && strcmp(sess->rootFile, self->cmd) < 0) ||
        (!strcmp(self->arg, "..") && !strcmp(sess->rootFile, sess->workFile)))
    {
        errStr = "550 Invalid name or chroot violation\r\n";
        sess->__send_by_cmd(sess, errStr);
        goto err;
    }
    sprintf(path, "%s/%s", sess->workFile, self->arg);
    err = stat(path, &statbuf);
    // 没有这个文件
    if (err == -1 || !S_ISDIR(statbuf.st_mode))
    {
        errStr = "550 Invalid name\r\n";
        sess->__send_by_cmd(sess, errStr);
        goto err;
    }
    if (self->cmd[0] == '/')
    {
        strcpy(sess->workFile, self->arg);
    }
    else
    {
        if (!strcmp(self->arg, ".."))
            dirname(sess->workFile);
        else if (strcmp(self->arg, "."))
        {
            strcat(sess->workFile, "/");
            strcat(sess->workFile, self->arg);
        }
    }

    sess->__send_by_cmd(sess, "250 CWD successful\r\n");
    log_info("function: cwd_run: CWD complete, %s", self->arg);
    return;

err:
    log_warn("function: cwd_run: %s", errStr);
}

// 当前目录
void pwd_run(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    char response[512];

    log_info("function: pwd_run: PWD executing");
    snprintf(response, sizeof(response), "257 \"%s\"\r\n", sess->workFile);

    sess->__send_by_cmd(sess, response);

    log_info("function: pwd_run: PWD complete, %s", self->arg);
}

// 目录列表
void list_run(void *_self)
{
}

// 被动模式
void pasv_run(void *_self)
{
}
// 主动模式
void port_run(void *_self)
{
}

// 从服务器下载文件
void retr_run(void *_self)
{
}

// 上传文件到服务器
void stor_run(void *_self)
{
}
// 删除文件
void dele_run(void *_self)
{
}

// 删除文件夹
void rmd_run(void *_self)
{
}

// 创建文件夹
void mkd_run(void *_self)
{
}

// 退出
void quit_run(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    self->sess->isRun = 0;
    sess->__send_by_cmd(sess, "211 Goodbye\r\n");
}

// 获取文件大小
void size_run(void *_self)
{
}

// 获取支持信息
void feat_run(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    log_info("function: feat_run: Giving FEAT");
    sess->__send_by_cmd(sess, "211-Features supported\r\n");
    sess->__send_by_cmd(sess, " UTF-8\r\n");
    sess->__send_by_cmd(sess, "211 End\r\n");
    log_info("function: feat_run: Gave FEAT");
}

void cmd_structor(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    SessionInfo *sess = self->sess;
    char *ptrptr = NULL;
    char *token = NULL;
    char *rest = NULL;
    bzero(self->cmd, sizeof(self->cmd));
    bzero(self->arg, sizeof(self->arg));

    sess->cmdBuf[strlen(sess->cmdBuf) - 2] = '\0';

    rest = sess->cmdBuf;
    token = strtok_r(rest, " ", &ptrptr);
    strcpy(self->cmd, token);
    if (ptrptr != NULL)
    {
        strcpy(self->arg, ptrptr);
    }

    log_info("function: cmd_structor: cmd:%s, arg:%s.", self->cmd, self->arg);
}

void ftp_cmd_init(void *_self, struct SessionInfo *sess)
{
    FtpCmd *self = _self;

    // 将功能模块装入函数列表
    void (**func)(void *) = calloc(COMMAND_COUNT, sizeof(void (*)(void *)));
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
    self->cmdMap.cmdFunc = func;

    self->sess = sess;
    self->cmdMap.Map = map;
}

void ftp_cmd_exit(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    free(self->cmdMap.cmdFunc);
    free(self);
}

void set_ftp_cmd_method(void *_self)
{
    FtpCmd *self = (FtpCmd *)_self;
    self->__init = ftp_cmd_init;
    self->__exit = ftp_cmd_exit;
    self->__cmd_structor = cmd_structor;
}
