#include <bits/stdc++.h>
#include <getopt.h>
#include "MessageProcessor.h"
#include "Participant.h"
#include "Coordinator.h"


using namespace std;
#define COORDINATOR 0
#define PARTICIPANT 1

/*
 * 该程序分为参与者和协调者，在main.cpp中需要读取终端传入的命令并解析是哪一种模式
 * 程序的启动是这样的：./kvstore2pcsystem --config_path [my config path]
 *
 */

const option long_options[] = {
    {"config_path", required_argument, nullptr, 'c'},
    {0, 0, 0, 0} // 结束标志
};

int main(int argc, char* argv[])
{
    string path;
    int opt;
    while ((opt = getopt_long(argc, argv, "c:", long_options, nullptr)) != -1)
    {
        if (opt == 'c')
            path = optarg; // optarg是一个全局变量用于存储当前解析到的参数值
        else
            exit(1);
    }

    auto data_config = MessageProcessor::parseConfigFile(path);
    int mode = -1;
    string coordinatorIp;
    int coordinatorPort;

    vector<addr> participant_info;

/**
 * participant_info是用来记录参与者的信息的，其中addr是声明在Coordinator.h中的结构体：
 * particpant_info.ip、participant_info.port是int、participant.valid是该地址是否有效
*/

    for(const auto& info : data_config){
        //迭代配置文件
        if(info[0] == "mode"){
            if(info[1] == "coordinator") mode = COORDINATOR;
            else if(info[1] == "participant") mode = PARTICIPANT;
        }else if(info[0] == "coordinator_info"){
            coordinatorIp = info[1];
            coordinatorPort = stoi(info[2]);
        }else if(info[0] == "participant"){
            participant_info.push_back({info[1], stoi(info[2]), true});
        }
    }

    // 启动

    if(mode == COORDINATOR){
        Coordinator coordinator(coordinatorIp, coordinatorPort, participant_info);
        //实例化一个对象
        coordinator.startup(); // 启动对应的对象
    }else if(mode == PARTICIPANT){
        Participant participant(participant_info[0].ip, participant_info[0].port);
        participant.startup();
    }else {
        perror("没有匹配的选项，程序出现错误，请检查");
        exit(1);
    }
    return 0;
}