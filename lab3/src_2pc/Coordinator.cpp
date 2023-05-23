#include "Coordinator.h"

using namespace std;

void Coordinator::startup(){
    //绑定端口
    int socketfd = socket(AF_INET, SOCK_STREAM, 0); // 创建一个套接字描述符号
    if(socketfd == -1){
        perror("套接字创建失败");
        exit(1);
    }
    auto coordinatorAddress = MessageProcessor::getSocket(Myaddr.ip, Myaddr.port);
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
        handleConnection(connectfd);
        close(connectfd);
    }
    close(socketfd);
}

/** Coordinator::startup()引用
 *  处理来自fd套接字端口的客户端请求
*/

void Coordinator::handleConnection(int fd){
    string data;
    int flag = getMessage(fd, data); //内容读取到data中
    if(flag == 0 || flag == -1) {
        perror("客户端发生了错误");
        exit(1);
    }
    vector<string> client_message;
    if(!MessageProcessor::parseClinetMessage(data, client_message)){//解析字符串
        sendMessage(fd, MessageProcessor::getClinetERROR());
    }

    if(client_message[0] == "GET")
        sendMessage(fd, handleGet(client_message));
    else if(client_message[0] == "SET")
        sendMessage(fd, handleSet(client_message));
    else if(client_message[0] == "DEL")
        sendMessage(fd, handleDel(client_message));

}

// 从fd套接字端口读取字符串data

int Coordinator::getMessage(int fd, string &data) {
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

// 向fd套接字端口发送字符串data

int Coordinator::sendMessage(int fd, const string& data){
    int bytesend = 0;
    if(data.empty())
        return 0;
    if(send(fd, data.c_str(), data.size(), 0) < 0) // 发送数据
        return -1;
    return 1; // 成功
}

string Coordinator::handleSet(const vector<string> &message_info){
    vector<int> participantSocket(ParticipantCount, -1); // 存储所有参与者的socket -1表示没有连接
    int ConnectParticipantCount = connectToAllParticipants(participantSocket);
    if(!ConnectParticipantCount)
        return MessageProcessor::getClinetERROR();
    // 如果没有一个保持连接的话 说明全挂了 就返回error
    
    //第一阶段 请求准备
    
    int id = getRamdonID(); // 为本次事务申请一次随机的ID
    string prepareRequest = MessageProcessor::getClusterRequestToPrepare(id, message_info);
    for(const int socketfd : participantSocket)
        if(socketfd > 0)
            sendMessage(socketfd, prepareRequest); //向参与者发送准备信息
    // 等待参与者投票
    int participantResponse = waitForPrepared(participantSocket, ConnectParticipantCount, prepareRequest);
    
    // 第二阶段开始 需要判断是什么情况 如果全都不连接直接返回error 
    if(participantResponse == -1){ // 第一阶段中全都不连接
        return MessageProcessor::getClinetERROR();
    }else {
        string commitOrAbortMessage;
        if(participantResponse == 0)
            commitOrAbortMessage = MessageProcessor::getCluserAbort(id);
        else if(participantResponse == 1)
            commitOrAbortMessage = MessageProcessor::getClusterCommit(id);
        //拿到信息了 继续向参与者发送
        for(const int& fd : participantSocket)
            if(fd > 0) sendMessage(fd, commitOrAbortMessage);
        //等待参与者返回
        vector<string> PaticipantResponcePhaseTwo; //参与者第二阶段返回的内容
        int PhaseTwoResponce = waitForTwoPhase(participantSocket, ConnectParticipantCount, commitOrAbortMessage);
    }
}

string Coordinator::handleGet(const vector<string> &message_info){
    
}

string Coordinator::handleDel(const vector<string> &message_info){
    
}

/** Coordinator::connectToAllParticipants
 * 此函数会对ParticipantAddr中存储的参与者的ip、port简历socket连接
 * 如果此socket与协调者socket连不通的话，就说明这个参与者宕机了 可以将ParticipantAddr中的valid变为false
 * 如果连接得通的话 就向participantSocket中加入对应socket
 * 返回成功连接的socket数
*/

int Coordinator::connectToAllParticipants(const std::vector<int>& participantSocket){
    int connectSuccesscnt = 0;
    for(int i = 0; i < ParticipantCount; ++ i){
        if(ParticipantAddr[i].valid){//失败是一次性的
            int sockfd = sock(AF_INET, SOCK_STREAM, 0);
            auto participantsock = MessageProcessor::getSocket(ParticipantAddr[i].ip, ParticipantAddr[i].port);
            auto connectsock = MessageProcessor::getSocket(Myaddr.ip, -1);
            if(bind(sockfd, (sockaddr*)&connectsock, sizeof(connectsock)) < 0){
                perror("在connectToAllParticipants中connectsock绑定失败");
                exit(1);
            }
            int connectcnt = 0;
            for(connectcnt = 0; connectcnt < 3; ++ connectcnt){
                // 最多尝试3次连接
                if(connect(sockfd, (sockaddr*)&participantsock, sizeof(participantsock)) < 0)
                    continue; // 连接失败则重试
                else {
                    participantSocket[i] = sockfd;
                    connectSuccesscnt ++;
                    break; // 连接成功就退出
                }
            }
            if(connectcnt == 3) 
                ParticipantAddr[i] = false;
        }
    }
    return connectSuccesscnt;
}

int Coordinator::getRamdonID(){ // 每次调用都会返回一个随机数
    mt19937 gen(randdevice());
    uniform_int_distribution<int> dis(0, 9999);
    return dis(gen);//返回0 - 9999的随机数
}

/**
 * waitForPerpared 
 * output: -1 全都不连接
 * 0:有一个返回了no
 * 1:全都prepare
*/

int Coordinator::waitForPrepared(vector<int> &participantSocket, int &AliveParticpantCount, string &preRequest){
    int maxfd = 0;
    fd_set allFdSet;// 位图 文件描述符在底层中就是一个位图
    FD_ZERO(&allFdSet); // 全置为0
    for(int fd : participantSocket){
        if(fd > 0){
            FD_SET(fd, &allFdSet);
            maxfd = max(fd, maxfd);
        }
    }

    int preparecnt = 0;
    int timeout = 0; // 超时
    vector<int> Response(ParticipantCount, -1);
    // -1:not used 0:response NO 1: Prepare
    while(AliveParticpantCount > 0 && (preparecnt < AliveParticpantCount)){
        auto curFdSet = allFdSet;
        timeval tv{TIME_OUT_SECS, TIME_OUT_US};
        int state = select(maxfd + 1, &curFdSet, nullptr, nullptr, &tv); 
        // state表示的是准备好进行IO操作的文件操作符数目
        if(state < 0){
            return -1;
        }else if(state == 0){
            // 超时
            if(timeout == 3){
                // 超时3次 全部去掉
                for(int i = 0; i < ParticipantCount; ++ i){
                    if(participantSocket[i] > 0 && (FD_ISSET(participantSocket[i], &allFdSet))){
                        close(participantSocket[i]);
                        FD_CLR(participantSocket[i], &allFdSet);
                        participantSocket[i] = -1;
                        ParticipantAddr[i].valid = false;
                        AliveParticpantCount --;
                    }
                }
                timeout = 0;
            }else {
                //重新发送
                for(int fd : participantSocket){
                    if(fd > 0 && FD_ISSET(fd, &allFdSet))
                        sendMessage(fd, preRequest);
                }
                timeout ++;
            }
        }else {
            //正常的情况
            timeout = 0;
            for(int i = 0; i < ParticipantCount && state > 0; ++ i){
                int fd = participantSocket[i];
                if(fd > 0 && FD_ISSET(fd, &allFdSet)){
                    state -- ;
                    string participantBackMsg;
                    if(getMessage(fd, participantBackMsg) <= 0){
                        //未接收到消息
                        close(fd);
                        FD_CLR(fd, &allFdSet);
                        int reconnectfd = connectParticipant(i);
                        if(reconnectfd < 0){
                            //重连失败 将
                            participantSocket[i] = -1;
                            ParticipantAddr[i].valid = false;
                            AliveParticpantCount --;
                        }else {
                            participantSocket[i] = reconnectfd;
                            sendMessage(reconnectfd, participantBackMsg);
                            FD_SET(reconnectfd, &allFdSet);
                        }
                    }
                }else {
                    //接收到了消息就要解析
                    vector<string> split_parttici_msg;
                    if(MessageProcessor::parseClusterMessage(preRequest, split_parttici_msg)){
                        if(split_parttici_msg[i] == "PREPARED"){
                            Response[i] = 1;
                        }else Response[i] = 0;
                    }else Response[i] = 0;
                    FD_CLR(participantSocket[i], &allFdSet);
                    preparecnt ++;
                }
            }
        }
    }

    if(!preparecnt) return -1; // 全都不连接

    if(checkPaticipants(Response)) return 1;//全部prepare
    else return 0; // 至少有一个NO
}

int Coordinator::connectParticipant(int id){
    if(!ParticipantAddr[i].valid)
        return -1;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in participant = MessageProcessor::getSocket(ParticipantAddr[i].ip, ParticipantAddr[i].port);
    sockaddr_in communicate = MessageProcessor::getSocket(Myaddr.ip, -1);

    if(bind(sockfd, (sockaddr*)&communicate, sizeof(communicate)) < 0)
        perror("在函数Coordinator::connectParticipant中绑定失败");
    int connectcnt = 0;
    for(connectcnt; connectcnt < 3; ++ connectcnt){
        if(connect(sockfd, (sockaddr*)participant, sizeof(participant)) < 0){
            continue;
        }else return sockfd;
    }
    //超过三次说明失败
    ParticipantAddr[i].valid = false;
    return -1;
}

bool Coordinator::checkPaticipants(const vector<int> &responce){
    if(responce.empty()) return false; //没有存活的参与者
    for(auto& t : responce){
        if(!t) return false; // 没有准备好
    }
    return true; //全部准备好了
}

int Coordinator::waitForTwoPhase(vector<int> participantSocket, int &aliveParticipant, const string& CommitOrAbortMessage, std::vector<std::string> &response_msg){
    int maxfd = 0;
    fd_set allFdSet;// 位图 文件描述符在底层中就是一个位图
    FD_ZERO(&allFdSet); // 全置为0
    for(int fd : participantSocket){
        if(fd > 0){
            FD_SET(fd, &allFdSet);
            maxfd = max(fd, maxfd);
        }
    }

    int finishcnt = 0;
    int timeout = 0; // 超时
    while(aliveParticipant > 0 && (finishcnt < aliveParticipant)){
        timeval tv{TIME_OUT_SECS, TIME_OUT_US};
        int state = select(maxfd + 1, &allFdSet, NULL, NULL, &tv);
        if(state < 0) return -1;
        else if(state == 0){
            //全部超时
            if(timeout == 3){
                for(int i = 0; i < ParticipantCount; ++ i){
                    int fd = participantSocket[i];
                    if(fd > 0 && FD_ISSET(fd, &allFdSet)){
                        close(fd);
                        FD_CLR(fd, &allFdSet);
                        participantSocket[i] = -1;
                        ParticipantAddr[i] = -1;
                        ParticipantAddr[i].valid = false;
                        aliveParticipant -- ;
                    }
                }
                timeout = 0;
            }else {
                for(int sockfd: participantSocket){
                    if(sockfd > 0 && FD_ISSET(sockfd, &allFdSet))
                        sendMessage(sockfd, CommitOrAbortMessage);
                }
                timeout ++;
            }
        }else {
            timeout = 0;
            for(int i = 0; i < ParticipantCount && state > 0; ++ i){
                int fd = participantSocket[i];
                if(fd > 0 && FD_ISSET(fd, &allFdSet)){
                    state --;
                    string participantBack;
                    if(getMessage(fd, participantBack) <= 0){
                        // 连接被断开或者错误
                        close(fd);
                        FD_CLR(fd, &allFdSet);
                        int reconnectfd = connectParticipant(i);
                        if(reconnectfd < 0){
                            participantSocket[i] = -1;
                            participantSocket[i].valid = false;
                            aliveParticipant --;
                        }else {
                            participantSocket[i] = reconnectfd;
                            sendMessage(reconnectfd, participantBack);
                            FD_SET(reconnectfd, &allFdSet);
                        }
                    }else {
                        //正常接受
                    }
                }
            }
        }
    }
}