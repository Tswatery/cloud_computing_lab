#include <bits/stdc++.h>
#include <getopt.h>
#include "MessageProcessor.h"

using namespace std;

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
            exit(EXIT_FAILURE);
    }

    auto data = MessageProcessor::parseConfigFile(path);
    for (auto i : data)
    {
        for (auto j : i)
            cout << j << ' ';
        puts("");
    }
}