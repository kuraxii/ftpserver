// /*********************************************
//  * @FileName: gtest.c
//  * @Author: Null-zzj
//  * @Mail: zj.zhu.cn@gmail.com
//  * @Created Time: Sat Dec  9 16:48:40 2023
//  *********************************************/

// #include <ftp_cmd.h>
// #include <ftp_log.h>
// #include <ftp_server.h>
// #include <gmock/gmock.h>
// #include <gtest/gtest.h>
// #include <libgen.h>
// #include <string.h>

// int foobar(void)
// {
//     return 1;
// }

// void cwd_run(void *_self)
// {
//     int err;
//     FtpCmd *self = (FtpCmd *)_self;
//     struct stat statbuf;
//     const char *errStr;

//     log_info("CWD executing");
//     // 访问到根目录以外的目录
//     if ((self->cmd[0] == '/' && strcmp(self->sess->rootFile, self->cmd)) ||
//         (!strcmp(self->arg, "..") && strcmp(self->sess->rootFile, self->sess->workFile)))
//     {
//         errStr = "550 Invalid name or chroot violation\r\n";
//         self->sess->__send_by_cmd((void *)errStr);
//         goto err;
//     }

//     err = stat(self->arg, &statbuf);
//     // 没有这个文件
//     if (err == -1)
//     {
//         errStr = "550 Invalid name or chroot violation\r\n";
//         self->sess->__send_by_cmd((void *)errStr);
//         goto err;
//     }
//     if (self->cmd[0] == '/')
//     {
//         strncpy(self->sess->workFile, self->arg, 255);
//     }
//     else
//     {
//         if (!strcmp(self->arg, ".."))
//             dirname(self->sess->workFile);
//         else
//             strncat(self->sess->workFile, self->arg, 255);
//     }

//     self->sess->__send_by_cmd((void *)"250 CWD successful");

//     log_info("CWD complete, %s", self->arg);

// err:
//     log_warn(errStr);
// }

// TEST(cwd_run, test)
// {
//     ASSERT_EQ(2, foobar());
// }

// int main(int argc, char *argv[])
// {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }