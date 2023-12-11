/*********************************************
 * @FileName: ftp_cmd.c
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:24:53 2023
 *********************************************/

#include <ftp_cmd.h>
#include <ftp_server.h>
#include <libgen.h>
#include <log.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

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
    sscanf(host, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

void get_port(int *port)
{
    srand(time(NULL));
    port[0] = 128 + (rand() % 64);
    port[1] = rand() % 0xff;
}

// 返回系统信息
void syst_run(FtpCmd *self)
{

    SessionInfo *sess = self->sess;
    char *response = "215 UNIX Type: L8\r\n";
    log_info("function: syst_run: SYST executing");
    send_by_cmd(sess, response);
    log_info("function: syst_run: SYST finished");
}

// 用户
void user_run(FtpCmd *self)
{

    SessionInfo *sess = self->sess;
    char *response = "331 Send password\r\n";
    log_info("function: user_run: USER executing");
    send_by_cmd(sess, response);
    log_info("function: user_run: USER finished");
}

// 密码
void pass_run(FtpCmd *self)
{

    SessionInfo *sess = self->sess;
    char *response = "230 Access granted\r\n";
    log_info("function: pass_run: PASS executing");
    send_by_cmd(sess, response);
    log_info("function: pass_run: PASS finished");
}

// 切换目录
void cwd_run(FtpCmd *self)
{
    int err;

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
        send_by_cmd(sess, errStr);
        goto err;
    }
    sprintf(path, "%s/%s", sess->workFile, self->arg);
    err = stat(path, &statbuf);
    // 没有这个文件
    if (err == -1 || !S_ISDIR(statbuf.st_mode))
    {
        errStr = "550 Invalid name\r\n";
        send_by_cmd(sess, errStr);
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

    send_by_cmd(sess, "250 CWD successful\r\n");
    log_info("function: cwd_run: CWD complete, %s", self->arg);
    return;

err:
    log_warn("function: cwd_run: %s", errStr);
}

// 当前目录
void pwd_run(FtpCmd *self)
{

    SessionInfo *sess = self->sess;
    char response[512];

    log_info("function: pwd_run: PWD executing");
    snprintf(response, sizeof(response), "257 \"%s\"\r\n", sess->workFile);

    send_by_cmd(sess, response);

    log_info("function: pwd_run: PWD complete, %s", self->arg);
}

// 目录列表
void list_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}

// 被动模式
void pasv_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
    int ip[4];
    int port[2];
    char *response = NULL;

    if (sess->islogged)
    {
        log_info("function: pasv_run: PASV set start");

        getip(sess->cmdSocket->socketFd, ip);
        get_port(port);

        sess->dataSocket->socketFd = sockinit(NULL, port[0] * 256 + port[1]);
        sprintf(sess->messsge, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", ip[0], ip[1], ip[2], ip[3], port[0],
                port[1]);
    }
    else
    {
        response = "530 Please login with USER and PASS.\n";
        strcpy(sess->messsge, response);
        log_warn("function: pasv_run: USER don't logged");
    }
    send_by_cmd(sess, sess->messsge);
    log_info("function: pasv_run: PASV set complete");
}
// 主动模式
void port_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}
// /** RETR command */
// void ftp_retr(Command *cmd, State *state)
// {

//   if(fork()==0){
//     int connection;
//     int fd;
//     struct stat stat_buf;
//     off_t offset = 0;
//     int sent_total = 0;
//     if(state->logged_in){

//       /* Passive mode */
//       if(state->mode == SERVER){
//         if(access(cmd->arg,R_OK)==0 && (fd = open(cmd->arg,O_RDONLY))){
//           fstat(fd,&stat_buf);

//           state->message = "150 Opening BINARY mode data connection.\n";

//           write_state(state);

//           connection = accept_connection(state->sock_pasv);
//           close(state->sock_pasv);
//           if(sent_total = sendfile(connection, fd, &offset, stat_buf.st_size)){

//             if(sent_total != stat_buf.st_size){
//               perror("ftp_retr:sendfile");
//               exit(EXIT_SUCCESS);
//             }

//             state->message = "226 File send OK.\n";
//           }else{
//             state->message = "550 Failed to read file.\n";
//           }
//         }else{
//           state->message = "550 Failed to get file\n";
//         }
//       }else{
//         state->message = "550 Please use PASV instead of PORT.\n";
//       }
//     }else{
//       state->message = "530 Please login with USER and PASS.\n";
//     }

//     close(fd);
//     close(connection);
//     write_state(state);
//     exit(EXIT_SUCCESS);
//   }
//   state->mode = NORMAL;
//   close(state->sock_pasv);
// }

// 从服务器下载文件
void retr_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}

// 上传文件到服务器
void stor_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}
// 删除文件
void dele_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}

// 删除文件夹
void rmd_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}

// 创建文件夹
void mkd_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}

// 退出
void quit_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
    self->sess->isRun = 0;
    send_by_cmd(sess, "211 Goodbye\r\n");
}

// 获取文件大小
void size_run(FtpCmd *self)
{
    SessionInfo *sess = self->sess;
}

// 获取支持信息
void feat_run(FtpCmd *self)
{

    SessionInfo *sess = self->sess;
    log_info("function: feat_run: Giving FEAT");
    send_by_cmd(sess, "211-Features supported\r\n");
    send_by_cmd(sess, " UTF-8\r\n");
    send_by_cmd(sess, "211 End\r\n");
    log_info("function: feat_run: Gave FEAT");
}

void typre_run(FtpCmd *self)
{

    SessionInfo *sess = self->sess;

    char *response = NULL;
    if (sess->islogged)
    {
        if (self->arg[0] == 'I')
        {
            response = "200 Switching to Binary mode.\n";
        }
        else if (self->arg[0] == 'A')
        {

            /* Type A must be always accepted according to RFC */
            response = "200 Switching to ASCII mode.\n";
        }
        else
        {
            response = "504 Command not implemented for that parameter.\n";
        }
    }
    else
    {
        response = "530 Please login with USER and PASS.\n";
    }
    send_by_cmd(sess, response);
}

void cmd_structor(FtpCmd *self)
{

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

void ftp_cmd_init(FtpCmd *self, struct SessionInfo *sess)
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
    self->cmdMap.cmdFunc = func;

    self->sess = sess;
    self->cmdMap.Map = map;
}

void ftp_cmd_exit(FtpCmd *self)
{

    free(self->cmdMap.cmdFunc);
    free(self);
}

void set_ftp_cmd_method(FtpCmd *self)
{

    self->__init = ftp_cmd_init;
    self->__exit = ftp_cmd_exit;
}
