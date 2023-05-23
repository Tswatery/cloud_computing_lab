#include <bits/stdc++.h>
#include <getopt.h>
#include "MessageProcessor.h"
#include "Node.h"

using namespace std;

/**
 * 如果是和基础版本一样的话，那么应该是./kvstoreraftsystem --config_path [my_config_file]
 * 
*/

const option long_options[] = {
    {"config_path", required_argument, nullptr, 'c'},
    {0, 0, 0, 0} // 结束标志
};

    /**
    follower_info 127.0.0.1:8001
    !
    ! The address and port the other follower processes are listening on.
    follower_info 127.0.0.1:8002
    follower_info 127.0.0.1:8003
    */

int main(int argc, char* argv[]){
    string path;
    int opt;
    while ((opt = getopt_long(argc, argv, "c:", long_options, nullptr)) != -1)
    {
        if (opt == 'c')
            path = optarg; // optarg是一个全局变量用于存储当前解析到的参数值
        else
            exit(1);
    }
    auto config_info = MessageProcessor::parseConfigFile(path);
    // 配置信息
    string ip = config_info[0][1];
    int port = stoi(config_info[0][2]);
    Node node(ip, port, config_info.size(), port % 8001, config_info);
    node.start();
}