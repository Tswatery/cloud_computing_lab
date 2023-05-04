#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    char str* = "hello world\r\n\r\n HTTP \r\n\r\n C++ \r\n\r\n";
    char *sub = strtok(str, "\r\n\r\n");
    while(sub != NULL){
        printf("%s\n", sub);
        sub = strtok(NULL, "\r\n\r\n");
    }
}