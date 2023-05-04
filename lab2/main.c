#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "libhttp.h"
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include "libget.h"
#include "libpost.h"
#include <pthread.h>
#define N 2048

void *server(void *args)
{
    int *temp_fd = (int *)args;
    int server_socket = *temp_fd;
    int nread;
    char *readbuffer = malloc(N + 1);
    wait_for_data(server_socket);
    while ((nread = read(server_socket, readbuffer, N)) > 0)
    { // 要无限循环等待//先读取
        // printf("读取到了%s\n", readbuffer);
        char *sub_http_request = malloc(100);
        strcpy(sub_http_request, readbuffer);
        while(sub_http_request != NULL){
            struct http_request *request = malloc(sizeof(struct http_request));
            request = http_request_parse(sub_http_request); 
            // 可以用线程优化不然会很慢
            if(!strcmp(request->method, "GET"))
                get_method(request, server_socket);
            else
                post_method(request, server_socket);
            free(request);
            sub_http_request = strtok(NULL, "\r\n\r\n");
        }
    }
    free(temp_fd);
    free(readbuffer);
    return NULL;
}

int main(int argc, char **argv)
{
    int option;
    char *ip = "127.0.0.1";
    int port = 8888;
    int threads = 8;
    /*
    注意，没有任何参数传入的话，那么会输出默认参数
    */

    // 定义长选项
    struct option long_options[] = {
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"threads", required_argument, 0, 't'},
        {0, 0, 0, 0}};

    while ((option = getopt_long(argc, argv, "i:p:t:", long_options, NULL)) != -1)
    {
        switch (option)
        {
        case 'i':
            ip = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 't':
            threads = atoi(optarg);
            break;
        default:
            // cout << "Usage: " << argv[0] << " --ip <ip> --port <port> --threads <threads>" << endl;
            return 1;
        }
    }

    // cout << "ip: " << ip << ", port: " << port << ", threads: " << threads << endl;

    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = inet_addr(ip);
    my_addr.sin_port = htons(port);

    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // 创建一个套接字
    {
        perror("socket");
        return 1;
    }

    int on = 1;
    if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        perror("setsockopt failed");
        return 1;
    }
    // 套接字选项

    if (bind(server_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    // 将服务器的套接字与指定的地址和端口绑定
    socklen_t sin_size = sizeof(struct sockaddr_in);
    struct sockaddr_in remote_addr; // 客户端的socket结构体
    if (listen(server_fd, 1024) < 0)
    {
        perror("listen");
        return 1;
    }
    // 监听套接字
    int client = 0;
    while (1)
    {
        int *clinet_fd = malloc(sizeof(int));
        if ((*clinet_fd = accept(server_fd, (struct sockaddr *)&remote_addr, &sin_size)) < 0)
        {
            perror("Error: accept");
            return 1;
        }
        // 等待客户端连接  accept会阻塞当前进程直到有客户端请求
        // accept会返回一个套接字socketfd该套接字与客户端相对应
        // 注意remote_addr是引用 它对应的是客户端的结构体指针
        client++;
        printf("[%d connections accept] from clinet %s:%d\n", client, inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
        // pthread_create(server, clinet_fd); // 处理对应的套接字
        pthread_t th;
        if (pthread_create(&th, NULL, server, clinet_fd) != 0)
        {
            perror("pthread_create failed");
            exit(1);
        }
    }
    return 0;
}