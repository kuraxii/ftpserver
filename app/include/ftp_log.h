/*********************************************
* @FileName: ftp_log.h
* @Author: Null-zzj
* @Mail: zj.zhu.cn@gmail.com
* @Created Time: Sat Dec  9 14:37:10 2023
*********************************************/
#include <log.h>

#ifndef __FTP_LOG_H
#define __FTP_LOG_H

// 日志级别
enum level
{
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};
void log_init();


#endif
