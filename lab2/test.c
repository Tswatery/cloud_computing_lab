#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    char request[] = "POST /api/echo HTTP/1.1\r\n"
                     "Host: localhost:8888\r\n"
                     "User-Agent: curl/7.84.0\r\n"
                     "Accept: */*\r\n"
                     "Content-Type: application/x-www-form-urlencoded\r\n"
                     "Content-Length: 13\r\n"
                     "\r\n"
                     "id=1&name=Foo";

    char* method = malloc(10);
    char* path = malloc(10);
    sscanf(request, "%s %s", method, path);
    printf("%s %s\n", method, path);
    char* content_length = strstr(request, "Content-Length: ");
    int content_len = 0;
    sscanf(content_length, "Content-Length: %d", &content_len);
    printf("%d\n", content_len);

    char *body_strat = strstr(request, "\r\n\r\n") + 4;
    char body[1048];
    int cnt = 0;
    for(int i = 0; i < content_len; ++ i)
        body[i] = body_strat[i];
    body[content_len] = '\0';
    printf("%s", body);
}