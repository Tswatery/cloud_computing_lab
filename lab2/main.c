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
#define N 1024

void server(int fd)
{
    int clientfd = fd;
    char reqbuf[N];
    int n;
    while (1)
    {
        // struct http_request *request = http_request_parse(clientfd);
        // printf("1");
        // // 监听套接字clientfd 这个函数会与accept一样阻塞clientfd直到请求是合理的
        // if (request == NULL || request->path[0] != '/')
        // {
        //     http_start_response(clientfd, 400);
        //     http_send_header(clientfd, "Content-Type", "text/html");
        //     http_end_headers(clientfd);
        //     close(clientfd);
        //     return;
        // }
        // printf("method: %s, path: %s", request->method, request->path);
        memset(reqbuf, 0, N);
        n = read(fd, reqbuf, N - 1); /* Recv */
        n = write(STDOUT_FILENO, reqbuf, strlen(reqbuf));
        n = write(fd, reqbuf, strlen(reqbuf)); /* echo*/
    }
    close(clientfd);
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
    if (listen(server_fd, 5) < 0)
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
        server(*clinet_fd); // 处理对应的套接字
    }
    return 0;
}