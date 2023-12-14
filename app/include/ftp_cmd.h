/*********************************************
 * @FileName: ftp_cmd.h
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Fri Dec  8 19:27:11 2023
 *********************************************/
#include "ftp_server.h"
#include "session_thread.h"

#ifndef __FTP_CMD_H
#define __FTP_CMD_H

struct FtpCmd;

typedef struct FtpCmd FtpCmd;

typedef struct cmdMap
{
    const char **  Map;
    void (**cmdFunc)(FtpCmd*);
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
    TYPE,
    COMMAND_COUNT
};

// cmd格式
typedef struct FtpCmd
{
    char cmd[8];   // cmd
    char arg[255]; // arg
    SessionInfo *sess;
    CmdMap cmdMap;                 // cmdmap
   
} FtpCmd;


void cmd_structor(FtpCmd *);
void ftp_cmd_init(FtpCmd *, struct SessionInfo *);
void ftp_cmd_exit(FtpCmd *);


#endif
