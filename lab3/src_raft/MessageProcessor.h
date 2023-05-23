#include <bits/stdc++.h>
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
    static bool parseClinetMessage(std::string &data, std::vector<std::string> &message);
    static std::string getClinetERROR();
    static std::string getClientOK();
    static std::vector<std::vector<std::string>> parseConfigFile(const std::string &path);
};

#endif // __MESSAGEPROCESSOR_H__