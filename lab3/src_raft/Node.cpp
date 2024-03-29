#include "Node.h"

using namespace std;

void Node::start(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        cout << "Fail to create socket" << endl;
        return ;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == -1) {
        printf("%s:%d fail to bind socket\n", ip.c_str(), port);
        close(sockfd);
        return;
    }
    if (listen(sockfd, numNodes) < 0)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        close(sockfd);
        return;
    }
    printf("%d Node start on %s:%d we have %d nodes in total and my nodeid is %d\n", myNodeId,ip.c_str(), port, numNodes, myNodeId);
    while(1){
        struct sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        // 连接
        int clientSockfd = accept(sockfd, reinterpret_cast<sockaddr *>(&clientAddr), &clientAddrLen);
        if (clientSockfd < 0)
        {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }
        // printf("%d在Node::start中clientSockfd是%d\n",myNodeId, clientSockfd); //执行
        HandleSockfd(clientSockfd); // 这里接受的应该是客户端的信息
        close(clientSockfd);
    }
    close(sockfd);
}


void Node::HandleSockfd(int sockfd){
    // 处理来自客户端的请求 后续高级版本会要求处理投票信息、心跳信息等等
    string data;
    int flag = MessageProcessor::getMessage(sockfd, data);
    if(flag == 0 || flag == -1){
        printf("\033[31m 在Node::HandleSockfd中的MessageProcessor::getMessage发生了错误\033[0m\n");
    }
    //将data拿出来了 解析
    printf("\033[32m 响应了数据是%s\n\033[0m\n", data.c_str());
    stringstream sscin(data);
    if(data.find('$') != string::npos) handleClientRequest(data, sockfd);
    else if(data.find("COMMIT") != string::npos){
        //节点的commit信息
        handleCommitRequest(data);
    }else if(data.find("HeartBeat") != string::npos){
        //Follower接受到了心跳信息
        handleHeartBeat();
    }else if(data.find("RequestVote") != string::npos){
        string s; sscin >> s; sscin >> s;
        handleRequestVote(stoi(s));
    }else if(data.find("AgreeVote") != string::npos){
        handleRecvVote();
    }
}


// ⬇️ 处理来自客户端的请求 比如GET返回什么 SET更新数据库 DEL则删除某一个条目
void Node::handleClientRequest(string& request, int fd){
    if(currentState != NodeState::Leader){// 如果不是leader的话 需要将请求传递给leader
        printf("%d is not leader\n", myNodeId);
        // sendRequestToLeader(request);
        sendLeaderInfo(fd);
    }else {
        printf("%d is leader\n", myNodeId);
        vector<string> parse_msg; // 由于客户端来的request是很奇怪的 因此需要解析
        if(!MessageProcessor::parseClinetMessage(request, parse_msg))
            sendMessage(fd, MessageProcessor::getClinetERROR());
        if(parse_msg[0] == "GET"){
            //如果是GET的话 需要记录到日志中 但是它不需要与其他两个node进行沟通
            string key = parse_msg[1]; // 拿到键
            if(kvdb.count(key) <= 0) {
                //不存在这个键 返回nil
                sendMessage(fd,"*1\r\n$3\r\nnil\r\n");
            }else {
                string value = kvdb[key]; // 拿到了Cloud Computing
                stringstream sscin(value);// 构建一个字符串流
                int space = 0;
                for(char c : value)
                    if(c == ' ') space ++;
                string message = '*' + to_string(space) + "\r\n";
                string s;
                while(sscin >> s){
                    int len = s.size();
                    message += ('$' + to_string(len) + "\r\n" + s + "\r\n");
                }
                sendMessage(fd, message);
            }
        }else {
            //如果是SET和DEL的话 首先要和其他两个节点进行通信 以此达到一致性
            for(int nodeid = 0; nodeid < numNodes; ++ nodeid){
                if(nodeid == myNodeId) continue;
                sendCommitinfo(nodeid, parse_msg); // 给出自己外的所有node发送commit的信息 以达到数据的一致性
            }
            //然后再自己提交 并向客户端返回信息
            Commit(parse_msg, fd);
        }
    }
}

void Node::sendLeaderInfo(int fd){
    auto ip = Nodeip[currentLeader];
    auto port = Nodeport[currentLeader];
    string message = ip + ':' + to_string(port);
    send(fd, message.c_str(), message.size(), 0);
    close(fd);
}

// ⬇️ 处理commit信息
void Node::handleCommitRequest(string& data){
    /*
    sscin << "COMMIT " << parse_msg[0] << " ";
    for(int i = 2; i < (int)parse_msg.size(); ++ i)
        sscin << parse_msg[i] << " ";
    ⬆️ data的信息
    */
   stringstream sscin(data);
   string type;  sscin >> type;// COMMIT
   sscin >> type; // SET ? DEL?
   if(type == "SET"){ // key value
        string value, key, s;
        sscin >> key;
        while(sscin >> s){
            value += (s + ' ');
        }
        kvdb[key] = value;
   }else {
        //DEL
        string key;
        while(sscin >> key){
            if(kvdb.count(key) <= 0) continue;
            kvdb.erase(key);
        }
   }
}

// ⬇️ 处理来自Leader的心跳信息 Follower需要重新计时
void Node::handleHeartBeat(){
    printf("\033[32m 节点%d接受到了心跳信息\033[0m\n", myNodeId);
    restartElection();
}

// ⬇️ 参与者开始计时
void Node::startElection(){ // 使用线程进行计时 如果超时了就启动另一个线程去handleTimeout
    //参与者开始计时
    FollowerTimerThread = thread([this](){
        unique_lock<mutex> lock(electionMutex);
        while(currentState == NodeState::Follower){
            printf("\033[32m %d跟随者在倒计时 \033[0m \n", myNodeId);
            if(currentState != NodeState::Follower) break;
            auto res = (cv_status)cv_election.wait_for(lock, chrono::milliseconds(rand() % 600 + 800), [this](){return electionTimeout;});
            if(res == cv_status::no_timeout){ // 超时了 妈的 这个地方卡好久 no_timeout才走下去
                printf("\033[31m 超时了\n \033[0m");
                {
                    unique_lock<mutex> lock(TimeoutLock);
                    is_timeout = true;
                }
                Timeout_cv.notify_one();
                break;
            } 
            electionTimeout = false;
        }
    });
}

// ⬇️ 开启线程HandleTimeout监听是否超时
void Node::handleElectionTimeout(){
    HandleTimeout = thread([this](){
        {
            unique_lock<mutex> lock(TimeoutLock);
            Timeout_cv.wait(lock, [this](){return is_timeout;});
        }
        if(is_timeout) 
            printf("\033[31m 超时重选 \033[0m \n");
        becomeCandidate();
    });
}

// ⬇️ 如果是handleHeartBeat的话 调用这个函数 然后就会给startElection发送信号让它重新计时
void Node::restartElection(){
    //接受了Leader的信号开始重新计时
    printf("\033[32m 节点%d开始重新计时\033[0m \n", myNodeId);
    {
        unique_lock<mutex> lock(electionMutex);
        electionTimeout = true;
    }
}

// ⬇️ commit信息
void Node::Commit(vector<string>& parse_msg, int fd){
    if(parse_msg[0] == "SET"){
        //如果是SET
        string key = parse_msg[1]; //拿到键
        string value;
        for(int i = 2; i < (int)parse_msg.size(); ++ i)
            value += (parse_msg[i] + ' ');
        kvdb[key] = value;
        if(fd != -1)
            sendMessage(fd, MessageProcessor::getClientOK());
    }else if(parse_msg[0] == "DEL"){
        //DEL key1 key2 ...
        int successDelCnt = 0;//成功删除的键的个数
        for(int i = 1; i < (int)parse_msg.size(); ++ i){
            auto key = parse_msg[i]; //键
            if(kvdb.count(key) <= 0) continue;
            else {
                kvdb.erase(key); successDelCnt ++;
            }
        }
        if(fd != -1)
            sendMessage(fd, ":" + to_string(successDelCnt) + "\r\n");
    }else {
        if(fd != -1)
            sendMessage(fd, MessageProcessor::getClinetERROR());
    }
}


// ⬇️ 向fd发送信息
void Node::sendMessage(int fd, const string& data){
    send(fd, data.c_str(), data.size(), 0);
}

// ⬇️ 给所有的节点发送提交的信息
void Node::sendCommitinfo(int nodeid, vector<string>& parse_msg){
    // 为了向每个nodeid发送parse_msg的内容 需要将每个nodeid的IP、port存储下来
    auto ip = Nodeip[nodeid];
    int port = Nodeport[nodeid];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    auto serverAddress = MessageProcessor::getSockAddr(ip, port);
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        printf("\033[31m 在sendCommitinfo中Failed to connect to server\033[0m\n");
        close(sockfd);
        return;
    }
    // 构造信息 COMMIT + Type(SET ? DEL) + key
    stringstream sscin;
    sscin << "COMMIT " << parse_msg[0] << " ";
    for(int i = 2; i < (int)parse_msg.size(); ++ i)
        sscin << parse_msg[i] << " ";
    string message = sscin.str(); 
    //printf("在Node::sendCommitinfo中向%d发送了%s\n", nodeid, message.c_str());
    printf("\033[32m 给%d节点发送了心跳信息\033[0m\n", nodeid);
    send(sockfd, message.c_str(), message.size(), 0);
    close(sockfd);
}

// ⬇️ 给leader发送消息 
void Node::sendRequestToLeader(string& request){
    auto ip = Nodeip[currentLeader];
    auto port = Nodeport[currentLeader];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    auto serverAddress = MessageProcessor::getSockAddr(ip, port);
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        perror("\033[31m 在sendRequestToLeader中Failed to connect to server\033[0m\n");
        close(sockfd);
        return;
    }
    send(sockfd, request.c_str(), request.size(), 0);
    close(sockfd);
}

// ⬇️ Leader 向所有的跟随者节点发送心跳信息
void Node::SendHeartBeat(){
    for(int nodeid = 0; nodeid < numNodes; ++ nodeid){
        if(nodeid == myNodeId) continue; // 自己不能向自己发送
        auto ip = Nodeip[nodeid];
        auto port = Nodeport[nodeid];
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        auto serverAddress = MessageProcessor::getSockAddr(ip, port);
        if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
            printf("\033[31m %d节点挂掉了\033[0m\n", nodeid);
            close(sockfd);
            continue; // 某个节点挂掉了之后可以向其他节点发送信息
        }
        string message = "HeartBeat!";
        send(sockfd, message.c_str(), message.size(), 0);
        close(sockfd);
    }
}

// ⬇️ 开启heartbeatThread线程
void Node::startHeartBeat(){
    heartbeatThread = thread([this]() {
        while (currentLeader == myNodeId) {
            SendHeartBeat();
            this_thread::sleep_for(chrono::milliseconds(20)); 
        }
    });
}

void Node::becomeLeader(){
    currentState = NodeState::Leader;
    currentLeader = myNodeId;
    printf("Now the leaderid is %d\n", myNodeId);
    startHeartBeat(); 
}

void Node::becomeFollower(){
    printf("\033[032m Now the leader is %d\033[0m\n", currentLeader);
    currentState = NodeState::Follower;
    startElection();
}


/*⬇️ 未完成*/

/*⬇️ 成为候选者后的投票*/
void Node::becomeCandidate(){
    // 成为候选者后需要请求投票
    currentState = NodeState::Candidate;
    candidateVotenum = 1;
    sendRequestVote();// 发送投票
}

/*⬇️ 发送投票请求*/
void Node::sendRequestVote(){
    for(int nodeid = 0; nodeid < numNodes; ++ nodeid){
        if(nodeid == myNodeId) continue; //不能向自己发送消息
        auto ip = Nodeip[nodeid];
        auto port = Nodeport[nodeid];
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        auto serverAddress = MessageProcessor::getSockAddr(ip, port);
        if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
            printf("\033[31m 发送投票的过程中发现%d节点挂掉了\033[0m\n", nodeid);
            close(sockfd);
            continue;
        }
        string message = "RequestVote! " + to_string(myNodeId);
        send(sockfd, message.c_str(), message.size(), 0);
        close(sockfd);
    }
}

/*⬇️ follower响应投票请求*/
// 接收到投票后也要重置自己的倒计时
void Node::handleRequestVote(int nodeid ){
    restartElection(); // 如果有一个节点接受到了请求投票的信息 那么它将继续是Follower
    auto ip = Nodeip[nodeid];
    auto port = Nodeport[nodeid];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    auto serverAddress = MessageProcessor::getSockAddr(ip, port);
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        printf("\033[31m 响应投票的过程中发现%d节点挂掉了\033[0m\n", nodeid);
        close(sockfd);
        return;
    }
    string message = "AgreeVote!";
    send(sockfd, message.c_str(), message.size(), 0);
    close(sockfd);
}

/*⬇️ candidate接收到投票*/
void Node::handleRecvVote(){
    candidateVotenum ++;
    if(candidateVotenum >= numNodes / 2) {
        becomeLeader();
    }
    // becomeCandidate();
}