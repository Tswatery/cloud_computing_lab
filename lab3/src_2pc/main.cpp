#include <vector>
#include <getopt.h>
#include "Coordinator.h"
#include "Participant.h"
#include "MessageProcessor.h"
using namespace std;

const option long_options[]={
        {"config_path",required_argument,nullptr,'c'},
        {nullptr,0,nullptr,0}
};

int main(int argc, char* argv[]) {
    string path;

    int opt;
    while((opt= getopt_long(argc,argv,"c:",long_options, nullptr))!=-1)
    {
        if(opt=='c')
            path=optarg;
        else
            exit(EXIT_FAILURE);
    }

    /*----\u89e3\u6790\u914d\u7f6e\u6587\u4ef6-----*/
    vector<vector<string>> configFileData=MessageProcessor::parseConfigFile(path);

    int mode=-1;
    string coordinatorIp;
    int coordinatorPort;
    vector<addr> partiAddr;

    for(auto &split_str:configFileData)
    {
        if(split_str[0]=="mode")
        {
            if(split_str[1]=="coordinator")
                mode=0;
            else if(split_str[1]=="participant")
                mode=1;
        }
        else if(split_str[0]=="coordinator_info")
        {
            coordinatorIp=split_str[1];
            coordinatorPort= stoi(split_str[2]);
        }
        else if(split_str[0]=="participant_info")
            partiAddr.push_back({split_str[1], stoi(split_str[2]),true});
    }

    if(mode==0)
    {
        Coordinator coor1(coordinatorIp,coordinatorPort,partiAddr);
        coor1.startup();
    }
    else if(mode==1)
    {
        Participant parti1(partiAddr[0].ip,partiAddr[0].port);
        parti1.startup();
    }

    return 0;
}