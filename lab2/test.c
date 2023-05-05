#include <stdio.h>
#include <string.h>

#define MAX_REQUESTS 100

int main() {
    char buffer[1024] = "Http\r\n\r\n, C++\r\n\r\n";
    printf("%d", strstr(buffer, "\r\n\r\n") - buffer);
}
