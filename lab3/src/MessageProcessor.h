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
private:
    static void cutStringBySpace(const std::string &str, std::vector<std::string> &split_str);
};

#endif // __MESSAGEPROCESSOR_H__