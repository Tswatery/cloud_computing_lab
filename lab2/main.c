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
#define N 8192

void *server(void *args)
{
    int *temp_fd = (int *)args;
    int server_socket = *temp_fd;
    int fd = server_socket;
    int nread;
    char *readbuffer = malloc(N + 1);
    char *request_arr[10]; // 已知大小且比较小，可以分配到栈空间中
    int request_cnt = 0;
    struct http_request *request = malloc(sizeof(struct http_request));
    nread = read(server_socket, readbuffer, N);
    if (strstr(readbuffer, "POST"))
    {
        request = http_request_parse(readbuffer);
        post_method(request, server_socket);
    }
    else
    {
        char *start_str = readbuffer;
        char *end_str;
        while ((end_str = strstr(start_str, "\r\n\r\n")))
        {
            size_t len = end_str - start_str;
            request_arr[request_cnt] = malloc(len + 1);
            memcpy(request_arr[request_cnt], start_str, len);
            request_arr[request_cnt][len] = '\0';
            request_cnt++;
            start_str = end_str + 4;
        }
        // 处理完后需要清空缓冲区
        memset(readbuffer, 0, N + 1);
        for (int i = 0; i < request_cnt; ++i)
        {
            request = http_request_parse(request_arr[i]);
            get_method(request, server_socket);
        }
        // 处理完一个请求之后需要清空 request_arr 数组，以便下一个请求的处理
        for (int i = 0; i < request_cnt; ++i)
        {
            free(request_arr[i]);
            request_arr[i] = NULL;
        }
        request_cnt = 0;
    }
    close(fd);
    free(request);
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