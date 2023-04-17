#include <iostream>
#include <getopt.h>

int main(int argc, char** argv) {
    int option;
    std::string ip = "127.0.0.1", port = "8888";
    int threads = 8;
    /*
    注意，没有任何参数传入的话，那么会输出默认参数
    */

    // 定义长选项
    struct option long_options[] = {
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"threads", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };

    while ((option = getopt_long(argc, argv, "i:p:t:", long_options, NULL)) != -1) {
        switch (option) {
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 't':
                threads = atoi(optarg);
                break;
            default:
                std::cout << "Usage: " << argv[0] << " --ip <ip> --port <port> --threads <threads>" << std::endl;
                return 1;
        }
    }

    std::cout << "ip: " << ip << ", port: " << port << ", threads: " << threads << std::endl;

    return 0;
}
