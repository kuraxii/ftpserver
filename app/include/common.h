#ifndef __COMMON_H
#define __COMMON_H

/*
    报文格式：
    客户端： 命令 + 字符串 \r\n

    服务端： 状态码 + 字符串 \r\n


*/

#define FILEEOF 0
// 服务端响应

#define COMMAND_ERR 502           // 没有识别到命令 502 Command not recognized\r\n
#define SERVER_ALREADLY 220       // 连接成功   220 SwiFTP4.5.1.0 ready\r\n
#define USERNAME_OK 331           // 用户名正确,需要密码 331 Send password\r\n
#define USER_LOIGNING_SUCCESS 230 // 用户登陆成功，具有数据访问权限  230 Access granted\r\n
#define PATH 257                  // 返回路径   257 "\"\r\n 表示在根目录
#define DATA_CONN 150             // 150 opening BINARY mode data conne ction for file list.
#define DATA_EOF 226 //  226 关闭数据连接。请求的文件操作已成功 226 Data transmiss ion oK\r\n

// 客户端请求指令

// SYST     用于获取远程服务器的系统信息。
// USER     用户身份验证命令，用于指定登录用户名。
// PASS     用户身份验证命令，用于指定登录密码。
// FEAT     获取服务器的特性列表。
// CWD     用于更改远程服务器上的当前工作目录。
// PWD     用于获取当前工作目录。
// LIST     列出指定目录中的文件列表。
// RETR     从服务器下载文件。
// NLST     列出指定目录中的文件名列表。
// NOOP     空操作，用于保持连接。
// STOR     上传文件到服务器。
// DELE     删除服务器上的文件。
// RMD     删除服务器上的目录。
// MKD     创建服务器上的目录。

// PORT     主动模式，指定数据连接的端口和地址。
// PASV     进入被动模式，等待服务器端的数据连接。

// QUIT     断开与服务器的连接。

// SIZE     获取文件大小。
// CDUP     返回上一级目录。
// APPE     在服务器上追加数据到文件末尾。
// XCUP     同 CDUP，是 CDUP 的同义词。
// XPWD    同 PWD，是 PWD 的同义词。
// XMKD    同 MKD，是 MKD 的同义词。
// XRMD    同 RMD，是 RMD 的同义词。
// MDTM     获取文件的修改时间。
// MFMT     修改文件的修改时间。
// REST     恢复中断的文件传输。
// SITE     发送站点特定的命令。
// MLST     列出指定文件的详细信息。
// MLSD     列出指定目录的详细信息。
// HASH     计算文件的哈希值。
// RANG     定义数据传输的范围。

#endif