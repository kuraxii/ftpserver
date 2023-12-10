# 指定目标
TARGET = server
TARGET_TEST = test

# 源文件路径
APP_DIR = ../app
#第三方库路径
THIRD_PARTY = ../third-party

# 头文件路径
INCLUDE_APP =  $(APP_DIR)/include  
INCLUDE_THIRD_PARTY = $(THIRD_PARTY)/include



# app源文件
SERVER_SRCS = $(APP_DIR)/ftp_cmd.c \
				$(APP_DIR)/ftp_server.c \
				$(APP_DIR)/session_thread.c \
				$(APP_DIR)/ftp_log.c \
				$(THIRD_PARTY)/log.c \


# 测试单元源文件
TEST_SRCS = ./gtest.cpp

# obj文件
C_OBJS = $(SERVER_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)

# 编译器
CC = gcc
CPP = g++

# 链接库
LIBS =  -lpthread -lsqlite3
TEST_LIBS = -lgtest -DLOG_USE_COLOR

# 头文件参数
INCLUDE = -I $(INCLUDE_APP) -I $(INCLUDE_THIRD_PARTY)

# gcc编译参数 
CFLAGS = -Wall -DLOG_USE_COLOR

# 规则编译.o 
%.o: %.c 
	$(CC) $(INCLUDE) $(CFLAGS) -c $< -o $@
%.o: %.cpp 
	$(CPP) $(INCLUDE) $(CFLAGS) -c $< -o $@

# server
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CC) -o $(TARGET) $(C_OBJS) $(CPP_OBJS) $(LIBS) $(LDFLAGS) $(INCLUDE) $(INCLUDE) -g
	

# test
$(TARGET_TEST): $(TEST_SRCS)
	$(CPP) -o $(TARGET_TEST) $(TEST_SRCS) $(TEST_LIBS) $(CFLAGS)

# 清理
clean:
	rm -f $(TARGET) $(TARGET_TEST) $(C_OBJS) $(CPP_OBJS)
