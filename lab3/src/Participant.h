#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef __PARTICIPANT_H__
#define __PARTICIPANT_H__

class Participant{
public:

private:
    std::string ip;
    int port;
    int phase; // 二阶段提交中必备的标记状态的标志
    std::string CoordinatorMsg; // 最近的一条协调者的信息
};


#endif // __PARTICIPANT_H__