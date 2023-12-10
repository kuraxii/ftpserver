/*********************************************
* @FileName: ftp_log.c
* @Author: Null-zzj
* @Mail: zj.zhu.cn@gmail.com
* @Created Time: Sat Dec  9 14:32:51 2023
*********************************************/

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include <log.h>
#include <ftp_log.h>
#include <pthread.h>

static pthread_mutex_t MUTEX_LOG = PTHREAD_MUTEX_INITIALIZER;



void log_lock(bool lock, void *udata)
{
    pthread_mutex_t *LOCK = (pthread_mutex_t *)(udata);
    if (lock)
        pthread_mutex_lock(LOCK);
    else
        pthread_mutex_unlock(LOCK);
}

void log_init()
{


    FILE *fp = fopen("logs.txt", "a");
    log_add_fp(fp, DEBUG); // 设置的级别为写入文件的最低级别
    // log_set_level(INFO);   // 设置输出级别 >= level
    log_set_quiet(true);
    log_set_lock(log_lock, &MUTEX_LOG);
    log_info("log init");

}



