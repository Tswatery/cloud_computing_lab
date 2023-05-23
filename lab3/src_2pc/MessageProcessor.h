#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef __MESSAGEPROCESSOR_H__
#define __MESSAGEPROCESSOR_H__

class MessageProcessor{
public:
    static std::vector<std::vector<std::string>> parseConfigFile(const std::string &path);
    static bool parseClinetMessage(std::string &data, std::vector<std::string> &message);
    static bool parseClusterMessage(std::string &data, std::vector<std::string> &message);
    static std::string getClinetERROR();
    static std::string getClientOK();
    static std::string getClusterRequestToPrepare(ing id, const std::vector<std::string> &split_message);
    static std::string getClusterCommit(int id);
    static std::string getCluserAbort(int id);
    static std::string getClusterPrepare(int id, const std::string& method);
    static sockaddr_in getSocket(const std::string& ip, int &port);

};

#endif // __MESSAGEPROCESSOR_H__