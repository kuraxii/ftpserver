/*********************************************
 * @FileName: ftp_cmd.h
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:27:11 2023
 *********************************************/
#include "ftp_server.h"
#include "session_thread.h"

#ifndef __FTP_CMD_H

typedef struct cmdMap
{
    const char **  Map;
    void (**cmdFunc)(void*);
} CmdMap;

enum CmdNo
{
    SYST = 0,
    USER,
    PASS,
    PORT,
    CWD,
    PWD,
    LIST,
    PASV,
    RETR,
    STOR,
    DELE,
    RMD,
    MKD,
    QUIT,
    SIZE,
    FEAT,
    COMMAND_COUNT
};

// cmd格式
typedef struct FtpCmd
{
    char cmd[8];   // cmd
    char arg[255]; // arg
    SessionInfo *sess;
    CmdMap cmdMap;                 // cmdmap
    void (*__cmd_structor)(void *); // cmd数据格式化
    void (*__init)(void *, SessionInfo *);
    void (*__exit)(void *);
} FtpCmd;

void set_ftp_cmd_method(void *self);

#define __ftp_cmd_H
#endif
