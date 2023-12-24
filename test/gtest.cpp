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


void _getip(const char* s_ip, int *ip)
{
    sscanf(s_ip, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}




// 测试getip函数
TEST(GetIpTest, ReturnsCorrectIp) {
    const char *ip1 = "197.99.140.249";
    const char *ip2 = "50.52.5.243";
    const char *ip3 = "165.53.214.161";
    const char *ip4 = "83.250.46.47";

    int ip[4];

    // Test for ip1
    _getip(ip1, ip);
    EXPECT_EQ(ip[0], 197);
    EXPECT_EQ(ip[1], 99);
    EXPECT_EQ(ip[2], 140);
    EXPECT_EQ(ip[3], 249);

    // Test for ip2
    _getip(ip2, ip);
    EXPECT_EQ(ip[0], 50);
    EXPECT_EQ(ip[1], 52);
    EXPECT_EQ(ip[2], 5);
    EXPECT_EQ(ip[3], 243);

    // Test for ip3
    _getip(ip3, ip);
    EXPECT_EQ(ip[0], 165);
    EXPECT_EQ(ip[1], 53);
    EXPECT_EQ(ip[2], 214);
    EXPECT_EQ(ip[3], 161);

    // Test for ip4
    _getip(ip4, ip);
    EXPECT_EQ(ip[0], 83);
    EXPECT_EQ(ip[1], 250);
    EXPECT_EQ(ip[2], 46);
    EXPECT_EQ(ip[3], 47);
}


void struct_path(char *path, const char *workDir, const char *arg)
{
    if (arg[0] == '/')
        strcpy(path, arg);
    else
        sprintf(path, "%s/%s", workDir, arg);
}

class StructPathTest : public ::testing::Test {
protected:
    static const int pathSize = 1024;
    char path[pathSize];

    void SetUp() override {
        // Initial setup can be done here
    }

    void TearDown() override {
        // Clean up can be done here
    }
};

// Test when 'arg' is an absolute path
TEST_F(StructPathTest, HandlesAbsolutePath) {
    const char *workDir = "/home/user";
    const char *arg = "/absolute/path";

    struct_path(path, workDir, arg);
    EXPECT_STREQ(path, "/absolute/path");
}

// Test when 'arg' is a relative path
TEST_F(StructPathTest, HandlesRelativePath) {
    const char *workDir = "/home/user";
    const char *arg = "relative/path";

    struct_path(path, workDir, arg);
    EXPECT_STREQ(path, "/home/user/relative/path");
}





int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}