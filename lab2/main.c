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
#define N 1024

int get_file_size(FILE *fp)
{
    fseek(fp, 0, SEEK_END); // move file pointer to the end
    int res = ftell(fp);
    fseek(fp, 0, SEEK_SET); // move file pointer to the start
    return res;
}

void serve_file(int fd, char *path, int status)
{
    /* TODO: PART 2 */
    /* PART 2 BEGIN */
    http_start_response(fd, status);
    http_send_header(fd, "Content-Type", http_get_mime_type(path));
    // 在这里读取path的内容
    FILE *fp = fopen(path, "rb");
    char *str = malloc(30); // 分配20个字节
    snprintf(str, 30, "%d", get_file_size(fp));
    http_send_header(fd, "Content-Length", str); // TODO: change this line too
    char *buf = malloc(1024);
    while (!feof(fp))
    { // 不遇到末尾
        memset(buf, 0, sizeof(buf));
        size_t bytes_read = fread(buf, 1, sizeof(buf), fp); // 每次读1个字节，最多读sizeof(buf)个
        write(fd, buf, bytes_read);
    }
    http_end_headers(fd);
    free(str);
    free(buf);
    /* PART 2 END */
}

void server(int fd)
{
    int clientfd = fd;
    struct http_request *request = http_request_parse(clientfd);
    // 监听套接字clientfd 这个函数会与accept一样阻塞clientfd直到请求是合理的
    // int n1 = write(STDERR_FILENO, request->method, strlen(request->method));
    // printf("method: %s, path: %s", request->method, request->path); 这是打印不出来的
    char directory[40] = "./static";
    strcat(directory, request->path);
    strcpy(request->path, directory);
    // int n2 = write(STDERR_FILENO, request->path, strlen(request->path));
    // 使用stat系统调用去获取文件的元数据信息  -> 操作系统的知识
    struct stat file_stat;
    int status = 200;

    if (stat(request->path, &file_stat) < 0) // 不存在这个文件
    {
        status = 404;
        request->path = "./static/404.html";
    }
    else if (strstr(request->path, "404"))
        status = 404;
    else if (strstr(request->path, "501"))
        status = 501;
    else if (strstr(request->path, "502"))
        status = 502;
    else if (strstr(request->path, "403"))
        status = 403;
    else if(strcmp(request->method,"GET") && strcmp(request->method, "POST")){
        request->path = "./static/501.html"; status = 501;
    }
    // int n2 = write(STDERR_FILENO, request->path, strlen(request->path));
    serve_file(fd, request->path, status);
    close(fd);
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