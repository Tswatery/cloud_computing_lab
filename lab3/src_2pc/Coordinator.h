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
#include <sys/select.h>

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
    void handleConnection(int fd);
    int getMessage(int fd, std::string& data);
    int sendMessage(int fd, const std::string& data);
    std::string handleGet(const std::vector<std::string>& message_info);
    std::string handleDel(const std::vector<std::string>& message_info);
    std::string handleSet(const std::vector<std::string>& message_info); // 处理请求的函数
    int connectToAllParticipants(const std::vector<int>& participantSocket); //参数是存储了所有参与者的socket描述符，返回值是与协调者保持连接的参与者的数量
    int getRamdonID();
    int waitForPrepared(std::vector<int> &participantSocket, int &ConnectionParticpantCount, std::string &preRequest);
    int connectParticipant(int id); // 连接
    bool checkPaticipants(const std::vector<int> &responce);
    int waitForTwoPhase(std::vector<int> participantSocket, int &aliveParticipant, const std::string& CommitOrAbortMessage, std::vector<std::string> &response_msg);
private:
    addr Myaddr; // 自己的地址
    std::vector<addr> ParticipantAddr; //参与者的地址以及是否可用
    int ParticipantCount;
    const int MAX_BUF_SIZE = 512;
    const int TIME_OUT_SECS = 1;
    const int TIME_OUT_US = 0;
    std::random_device randdevice; // 可以生成高质量的随机数 与操作系统硬件有关
};
#endif // __COORDINATOR_H__