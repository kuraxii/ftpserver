/*********************************************
 * @FileName: gtest.c
 * @Author: Null-zzj
 * @Mail: zj.zhu.cn@gmail.com
 * @Created Time: Sat Dec  9 16:48:40 2023
 *********************************************/

#include <ftp_cmd.h>
#include <ftp_log.h>
#include <ftp_server.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <libgen.h>
#include <string.h>

using namespace testing;

// 假设我们已经有了一个mock的系统调用类
class MockSystemCalls {
public:
    MOCK_METHOD(int, getsockname, (int sockfd, struct sockaddr *addr, socklen_t *addrlen));
    MOCK_METHOD(char*, inet_ntoa, (struct in_addr in));
};

void getip(int sockfd, int *ip)
{
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    getsockname(sockfd, (struct sockaddr *)&addr, &addr_size);

    char *host = inet_ntoa(addr.sin_addr);

#ifdef DEBUG
    log_debug("function: getip: ip: %s", host);
#endif

    sscanf(host, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

// 测试getip函数
TEST(FTPCommandTest, GetIPTest) {
    // 创建一个sockfd和ip数组
    int sockfd = 0;
    int ip[4] = {0};

    // 创建一个mock的系统调用对象
    MockSystemCalls mock_syscalls;

    // 设置expectations
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    EXPECT_CALL(mock_syscalls, getsockname(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(addr), Return(0)));  // 假设getsockname成功
    EXPECT_CALL(mock_syscalls, inet_ntoa(_))
        .WillOnce(Return(ByMove("127.0.0.1")));  // 假设inet_ntoa返回了正确的IP地址

    // 调用getip函数
    getip(sockfd, ip);

    // 检查结果
    EXPECT_EQ(ip[0], 127);
    EXPECT_EQ(ip[1], 0);
    EXPECT_EQ(ip[2], 0);
    EXPECT_EQ(ip[3], 1);
}


int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}