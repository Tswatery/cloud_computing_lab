#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libhttp.h"
#include "libget.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <regex.h>
#define N 2048

void post_method(struct http_request *request, int server_socket)
{
    // valid -- 2023-04-27
    regex_t regex;
    regmatch_t matches[3];
    if(!strcmp(request->path, "/api/echo")){
        // 先使用正则表达式判断content是否合理
        int reti = regcomp(&regex, "id=[0-9]+&name=[A-Za-z0-9]+", REG_EXTENDED);
        if(reti) {
            perror("re compile error");
            exit(1);
        }
        reti = regexec(&regex, request->content, 3, matches, 0);
        if(reti){
            //匹配不成功
            strcpy(request->path, "./data/error.txt");
            echo_back(request, server_socket, 404);
        }else {
            // 匹配成功
            http_start_response(server_socket, 200);
            http_send_header(server_socket, "Content-Type", request->content_type);
            char *content_len = malloc(10);
            snprintf(content_len, 10, "%lu", strlen(request->content));
            http_send_header(server_socket, "Content-Length", content_len);
            free(content_len);
            http_end_headers(server_socket);
            write(server_socket, request->content, strlen(request->content));
        }
    }else if(!strcmp(request->path, "/api/upload")){
        http_start_response(server_socket, 200);
        http_send_header(server_socket, "Content-Type", http_get_mime_type(".json"));
        char *content_len = malloc(10);
        snprintf(content_len, 10, "%lu", strlen(request->content));
        http_send_header(server_socket, "Content-Length", content_len);
        free(content_len);
        http_end_headers(server_socket);
        write(server_socket, request->content, strlen(request->content));  
    }else {
            //404
            strcpy(request->path, "./static/404.html");
            echo_back(request, server_socket, 404);
        }
    
}