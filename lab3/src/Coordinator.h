#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <sys/types.h>
#include "MessageProcessor.h"
#include <unistd.h>
#include <random>

#ifndef __COORDINATOR_H__
#define __COORDINATOR_H__

struct addr{
    std::string ip;
    int port;
    bool valid;
};

class Coordinator{
public:
    Coordinator(const std::string &IP, int port, const std::vector<addr> &info)
        : Myaddr({IP, port, true}), ParticipantAddr(info), ParticipantCount((int) info.size())
        {}
        // 构造函数
    void startup(); // 启动函数 绑定端口

private:
    void handleConnection(int fd, sockaddr_in clientaddr);
    int getMessage(int fd, std::string& data) const;
    int sendMessage(int fd, const std::string& data);
    std::string handleGet(int fd, std::vector<std::string>& message_info);
    std::string handleDel(int fd, std::vector<std::string>& message_info);
    std::string handleSet(int fd, std::vector<std::string>& message_info); // 处理请求的函数
private:
    addr Myaddr; // 自己的地址
    std::vector<addr> ParticipantAddr; //参与者的地址以及是否可用
    int ParticipantCount;
    const int MAX_BUF_SIZE = 512;
    const int TIME_OUT_SECS = 1;
    const int TIME_OUT_US = 0;
};
#endif // __COORDINATOR_H__