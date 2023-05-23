#include <bits/stdc++.h>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_BUF_SIZE 1024

#ifndef __MESSAGEPROCESSOR_H__
#define __MESSAGEPROCESSOR_H__


class MessageProcessor{
public:
    static bool parseClinetMessage(std::string &data, std::vector<std::string> &message);
    static std::string getClinetERROR();
    static std::string getClientOK();
    static std::vector<std::vector<std::string>> parseConfigFile(const std::string &path);
    static int getMessage(int fd, std::string& data);//拿fd套接字里面的信息并转换为data
    static sockaddr_in getSockAddr(const std::string &ip,int port);
};

#endif // __MESSAGEPROCESSOR_H__