#include "Coordinator.h"

using namespace std;

void Coordinator::startup(){
    //绑定端口
    int socketfd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个套接字描述符号
    if(socketfd == -1){
        perror("套接字创建失败");
        exit(1);
    }
    sockaddr_in coordinatorAddress;
    coordinatorAddress.sin_addr.s_addr = inet_addr(Myaddr.ip.c_str()); 
    coordinatorAddress.sin_port = htons(Myaddr.port);
    // 设置协调者的socket结构体
    int optval = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    //设置监听端口重用 不知道干什么的
    if(bind(socketfd, (sockaddr*)&coordinatorAddress, sizeof(coordinatorAddress)) < 0){
        perror("协调者绑定地址失败");
        exit(1);
    }
    //绑定

    listen(socketfd, 10); //监听

    printf("Coordinator in %s : %d starts listening ... \n", Myaddr.ip.c_str(), Myaddr.port);
    while(1){
        //等待连接
        sockaddr_in client_addr;
        socklen_t client_addrlength = sizeof(client_addr);
        int connectfd = accept(socketfd, (sockaddr*)&client_addr, &client_addrlength);
        handleConnection(connectfd, client_addr);
        close(connectfd);
    }
    close(socketfd);
}

void Coordinator::handleConnection(int fd, sockaddr_in client_addr){
    string data;
    int flag = getMessage(fd, data); // 获取客户端发来的请求 不知道是干啥的
    if(flag == 0 || flag == -1) {
        perror("客户端发生了错误");
        exit(1);
    }
    vector<string> client_message;
    if(!MessageProcessor::parseClinetMessage(data, client_message)){
        sendMessage(fd, MessageProcessor::getClinetERROR());
    }

    if(client_message[0] == "GET")
        sendMessage(fd, handleGet(client_message));
    else if(client_message[0] == "SET")
        sendMessage(fd, handleSet(client_message));
    else if(client_message[0] == "DEL")
        sendMessage(fd, handleDel(client_message));

}

int Coordinator::getMessage(int fd, string &data) const{
    // 从fd中读取客户端的数据
    vector<char> buffer(MAX_BUF_SIZE);
    ssize_t byterecv = 0;
    do{
        byterecv = recv(fd, &buffer, MAX_BUF_SIZE, 0);
        if(byterecv < 0){
            perror("读取数据出现了问题");
            return -1;
        }else if(byterecv == 0){ //关闭了连接
            return 0;
        }else {
            data.append(buffer.cbegin(), buffer.cbegin() + byterecv); // 常量迭代器
        }
    }while(byterecv == MAX_BUF_SIZE);

    return 1;
}

int Coordinator::sendMessage(int fd, const string& data){
    int bytesend = 0;
    if(data.empty())
        return 0;
    if(send(fd, data.c_str(), data.size(), 0) < 0) // 发送数据
        return -1;
    return 1; // 成功
}

