#include "Node.h"

using namespace std;

void Node::start(){
    currentLeaderId = -1;
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
        printf("%d在Node::start中clientSockfd是%d\n",myNodeId, clientSockfd); //执行
        HandleSockfd(clientSockfd);//怎么处理信息 
        close(clientSockfd);
    }
    close(sockfd);
}

void Node::startElectionTimer(){
    // 启动选举超时定时器
    int electionTimeout = generateElectionTimeout();
    // 在 electionTimeout 毫秒后触发选举超时
    startTimer(electionTimeout, [this]() {
        // 选举超时，触发新一轮选举
        becomeCandidate();
    });
}

void Node::startTimer(int timeout, function<void()> callback){
    if (electionTimerThread.joinable()) {
        electionTimerThread.join();
    }

    // 启动新的选举超时定时器线程
    electionTimerThread = std::thread([this, timeout, callback]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        callback();
    });
}

void Node::resetElectionTimer() { // 取消之前的超时定时器
    // 取消之前的选举超时定时器
    if (electionTimerThread.joinable()) {
        electionTimerThread.join();
    }

    // 重新启动选举超时定时器
    int electionTimeout = generateElectionTimeout();
    electionTimerThread = std::thread([this, electionTimeout]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(electionTimeout));
        becomeCandidate();
    });
}

int Node::generateElectionTimeout(){
    return 150 + rand() % 150; // 150 - 300 毫秒的超时信息
}

void Node::becomeFollower(){
    currentState = NodeState::Fllower;
    votedFor = -1;
    //startElectionTimer();
}

void Node::becomeCandidate(){
    currentState = NodeState::Candidate;
    currentTerm ++;
    votedFor = myNodeId;
    votesReceived = 1;
    for(int NodeId = 0; NodeId < numNodes; NodeId ++){
        if(NodeId == myNodeId) continue;
        sendRequestVote(NodeId);
    }
    //startElectionTimer();
}

void Node::sendRequestVote(int Nodeid){
    auto IP = NodeIp[Nodeid];
    auto port = NodePort[Nodeid];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in NodeStruct;
    NodeStruct.sin_family = AF_INET;
    NodeStruct.sin_addr.s_addr = inet_addr(IP.c_str());
    NodeStruct.sin_port = htons(port);
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&NodeStruct), sizeof(NodeStruct)) == -1) {
        std::cerr << "Failed to connect to the server." << std::endl;
        close(sockfd);
        return;
    }
    stringstream ss;
    ss << "REQUEST_VOTE " << Nodeid << " "<< currentTerm << " " << getlastLogIndex() << " " << getlastLogTerm();
    auto message = ss.str();
    if(send(sockfd, message.c_str(), message.size(), 0) == -1){
        cout << "Fail to send back\n";
        return ;
    }
    close(sockfd);
}

int Node::getlastLogIndex(){
    if(LogEntries.empty()) return -1;
    return LogEntries.back().index;
}

int Node::getlastLogTerm(){
    if(LogEntries.empty()) return -1;
    return LogEntries.back().term;
}

void Node::handleRequestVoteRequest(int candidateId, int term, int lastLogIndex, int lastLogTerm){
    if(term < currentTerm){
        sendRequestVoteResponse(candidateId, currentTerm, false);
        return ;
        //如果请求节点的任期较小 就拒绝投票
    }
    if(term > currentTerm){
        currentTerm = term; // 更新自己的任期
        currentState = NodeState::Fllower;
        votedFor = -1; // 重置投票对象
    }

    bool VoteGranted = false; //是否允许投票
    if((votedFor == -1 || votedFor == candidateId) && isCandidateLogUpToDate(lastLogIndex, lastLogTerm)){
        VoteGranted = true;
        votedFor = candidateId;
        //resetElectionTimer();
    }
    sendRequestVoteResponse(candidateId, currentTerm, VoteGranted); // 回复投票结果
}

bool Node::isCandidateLogUpToDate(int lastLogIndex, int lastLogTerm){
    int index = getlastLogIndex(), term = getlastLogTerm();
    if(index == -1 || term == -1) return true;
    else if(index < lastLogIndex) return true;
    else if(index == lastLogIndex && term < lastLogTerm) return true;
    return false;
}

void Node::sendRequestVoteResponse(int candidateId, int currentTerm, bool VoteGranted){
    // 投票的回应
    auto ip = NodeIp[candidateId];
    auto port = NodePort[candidateId];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "sock create fail\n";
        return;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddress.sin_port = htons(port);

    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        cout << "Failed to connect to server" << endl;
        close(sockfd);
        return;
    }
    auto message = "VOTE_RESPONSE " + to_string(candidateId) + " " + to_string(currentTerm) + " "  + (VoteGranted ? "true" : "false");

    ssize_t sentBytes = send(sockfd, message.c_str(), message.size(), 0);
    if(sentBytes == -1){
        cout << "sent fail \n";
        return ;
    }
    close(sockfd);
}

void Node::handleRequestVoteResponse(int term, bool voteGranted){
    if(term > currentTerm){
        //收到了term比自己的大 更新任期
        currentTerm = term;
        currentState = NodeState::Fllower;
        votedFor = -1; // 重置已投票给其他节点的记录
        resetElectionTimer();
    }
    if(currentState != NodeState::Candidate || term != currentTerm){
        return ;
    }
    if(voteGranted){
        votesReceived ++;
        if(votesReceived > numNodes / 2) 
            becomeLeader();
    }
}

void Node::becomeLeader(){
    currentState = NodeState::Leader;
    currentLeaderId = myNodeId;
    //sendHeartBeats();
}

void Node::sendHeartBeats(){
    while(currentState == NodeState::Leader){
        for(int Nodeid = 0; Nodeid < numNodes; ++ Nodeid){
            if(Nodeid == myNodeId) continue;
            sendHeartBeatToOne(Nodeid);
        }
        this_thread::sleep_for(chrono::microseconds(50));
    }
}

void Node::sendHeartBeatToOne(int nodeid){
    auto ip = NodeIp[nodeid];
    auto port = NodePort[nodeid];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "sock create fail\n";
        return;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddress.sin_port = htons(port);

    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        cout << "Failed to connect to server" << endl;
        close(sockfd);
        return;
    }

    string beatMessage = "HEART_BEAT_MESSAGE " + to_string(currentLeaderId) + " " + to_string(currentTerm);

    send(sockfd, &beatMessage, sizeof(beatMessage), 0);
    close(sockfd);
}

void Node::handlerHeartbeat(int leaderid, int term){
    if(term < currentTerm) 
        return ;
    currentLeaderId = leaderid;
    if(currentState == NodeState::Candidate){
        becomeFollower();
    }else if(currentState == NodeState::Fllower && term > currentTerm){
        becomeFollower();
    }
    resetElectionTimer();
}

int Node::getLeaderNodeId(){
    return currentLeaderId;
}

LogEntry Node::CreateLogData(vector<string>& split_message){
    LogEntry log;
    log.index = getlastLogIndex();
    log.term = getlastLogTerm();
    log.LogContent.type = split_message[0];
    for(int i = 1; i < (int)split_message.size(); ++ i){
        log.LogContent.data += (split_message[i] + ' ');
    }
    return log;
}

void Node::handleClientRequest(string& request, int fd){
    printf("%d 进入了 Node::handleClientRequest ", myNodeId);
    // 只有Leader才能执行这个函数 来了一个请求后 需要将数据库操作加入日志 然后再广播日志 最后才能返回
    if(currentState != NodeState::Leader){
        printf("%d不是领导者\n", myNodeId);
        sendRequestToLeader(request);
    }else {
        printf("%d是leader\n", myNodeId);
        vector<string> split_message;
        if(!MessageProcessor::parseClinetMessage(request, split_message)){
            sendMessage(fd, MessageProcessor::getClinetERROR());
        }
        printf("解析的结果是\n");
        for(auto t : split_message)
            cout << t << ' ';
        puts("");
        auto Logdata = CreateLogData(split_message);
        LogEntries.push_back(Logdata); // 存在日志中
        if(split_message[0] == "GET"){
            //其实GET信息是不需要提交的 因为它并不会改变本地数据库的情况
            string key = split_message[1]; // 拿到键
            if(kvdb.count(key) <= 0) sendMessage(fd, "*1\r\n$3\r\nnil\r\n");
            else {
                //存在
                string value = kvdb[key]; // 拿到了Cloud Computing
                stringstream sscin(value);
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
            return ;
        }
        // 并发地向所有的跟随者发送追加信息
        int numReplySuccess = 0; 
        for(int nodeid = 0; nodeid < numNodes; ++ nodeid){
            if(nodeid == myNodeId) continue;
            sendAppendEntries(nodeid, Logdata, numReplySuccess);  // 应该是将Logadta给nodeid 因为在基础版本中不会有机器挂掉
        }
        //线程全部执行完毕
        if(numReplySuccess >= numNodes / 2 + 1){
            CommitLog(Logdata, fd); // 提交事务 并向fd发回消息
            sendCommitToFollowers(); // 让所有的跟随者提交
            //sendMessage(fd, HandleLogEntry(Logdata)); // 提交完毕 给所有的跟随者发送commit的消息 只有SET和DEL
        }
    }
}

void Node::CommitLog(LogEntry Logdata, int fd){
    //提交事务
    stringstream sscin(Logdata.LogContent.data);
    if(Logdata.LogContent.type == "DEL"){
        // DEL key1 key2 ...
        string s; int successcnt = 0;
        while(sscin >> s){
            //删除
            if(kvdb.count(s)){
                kvdb.erase(s);
                successcnt ++;
            } 
        }
        if(fd != -1)
            sendMessage(fd, ':' + to_string(successcnt) + "\r\n");
    }else if(Logdata.LogContent.type == "SET"){
        // SET CS06142 "Cloud Computing"
        string key, value;
        sscin >> key;
        string s;
        while(sscin >> s) {
            value += (s + ' ');
        }
        kvdb[key] = value; // 提交成功
        if(fd != -1)
            sendMessage(fd, "+OK\r\n");
    }
}

void Node::sendCommitToFollowers(){ // 让所有的follower提交
    // 使用线程来并行化
    for(int nodeid = 0; nodeid < numNodes; ++ nodeid){
        if(nodeid == myNodeId) continue;
        SendCommitMsg(nodeid);
    }
}

void Node::SendCommitMsg(int nodeid){
    auto ip = NodeIp[nodeid];
    auto port = NodePort[nodeid];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "create socket fail\n";
        return ;
    }
    sockaddr_in Mysock;
    Mysock.sin_family = AF_INET;
    Mysock.sin_addr.s_addr = inet_addr(ip.c_str());
    Mysock.sin_port = htons(port);
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&Mysock), sizeof(Mysock)) == -1) {
        cout << "Failed to connect to server" << endl;
        close(sockfd);
        return;
    }
    string message = "Commit!";
    send(sockfd, message.c_str(), message.size(), 0);
    close(sockfd);
}

void Node::sendRequestToLeader(string& request){
    //while(currentLeaderId == -1) this_thread::sleep_for(chrono::microseconds(100));
    auto ip = NodeIp[currentLeaderId];
    auto port = NodePort[currentLeaderId];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "create socket fail\n";
        return ;
    }
    sockaddr_in Mysock;
    Mysock.sin_family = AF_INET;
    Mysock.sin_addr.s_addr = inet_addr(ip.c_str());
    Mysock.sin_port = htons(port);
    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&Mysock), sizeof(Mysock)) == -1) {
        cout << "Failed to connect to server" << endl;
        close(sockfd);
        return;
    }
    send(sockfd, request.c_str(), request.size(), 0);
    close(sockfd);
}

void Node::HandleSockfd(int fd){
    printf("%d进入了Node::HandleSockf, fd 是%d\n",myNodeId, fd);
    string data;
    int flag = getMessage(fd, data);
    if(flag == 0 || flag == -1) {
        cout << "getmessage fail\n";
        return ;
    }
    printf("%d在HandleSockfd中接受到了%s flag 是 %d\n",myNodeId, data.c_str(), flag);
    //data有几种类型，如果是客户端来的请求 里面一定含有$
    //如果是请求投票 那么就是以REQUEST_VOTE开头的
    //如果是回应投票 那么就是以VOTE_RESPONSE开头
    //如果是心跳信息 那么就是以HEART_BEAT_MESSAGE开头的
    stringstream ss(data);
    if(data.find("$") != string::npos) handleClientRequest(data, fd);
    else if(data.find("REQUEST_VOTE") != string::npos){
        int candidateid, term, lastLogIndex, lastLogTerm;
        string type;
        ss >> type;
        ss >> candidateid;
        ss >> term;
        ss >> lastLogIndex;
        ss >> lastLogTerm;
        handleRequestVoteRequest(candidateid, term, lastLogIndex, lastLogTerm);
    }else if(data.find("VOTE_RESPONSE") != string::npos){
        int nodeid, term;
        string type, votegrant;
        ss >> type;
        ss >> nodeid;
        ss >> term;
        ss >> votegrant;
        handleRequestVoteResponse(term, (votegrant == "true" ? true : false));
    }else if(data.find("HEART_BEAT_MESSAGE") != string::npos){
        string type; int leaderid, term;
        ss >> type; 
        ss >> leaderid;
        ss >> term;
        handlerHeartbeat(leaderid, term);
    }else if(data.find("Log_APPEND") != string::npos){
        string messagetype, type, messagedata; int index, term;
        ss >> messagetype;
        ss >> index; ss >> term;
        ss >> type;
        ss >> messagedata;
        LogEntry Mylog;
        Mylog.index = index; Mylog.term = term; Mylog.LogContent.data = data; Mylog.LogContent.type = type;
        HandleLog(Mylog);
    }else if(data.find("Commit") != string::npos){
        CommitLog(LogEntries.back());
    }
}

int Node::getMessage(int fd, string& data){
    printf("进入了getMessage中\n");
    char buffer[MAX_BUF_SIZE];
    ssize_t byterecv = 0;
    do{
        byterecv = recv(fd, &buffer, MAX_BUF_SIZE, 0);
        printf("byterecv 是 %d\n", (int)byterecv);
        printf("在getMessage中接受到的数据是%s\n", buffer);
        if(byterecv < 0){
            perror("读取数据出现了问题");
            return -1;
        }else if(byterecv == 0){ //关闭了连接
            cout << "关闭了连接\n";
            return 0;
        }else {
            cout << "执行了 byterecv是" << byterecv << endl;
            for(int i = 0; i < byterecv; ++ i)
                data.push_back(buffer[i]);
        }
        printf("在Node::getMessage中data是%s 它的长度是%ld\n", data.c_str(), data.size());
    }while(byterecv == MAX_BUF_SIZE);
    return 1;
}

int Node::sendMessage(int fd, const string& data){
    if(data.empty()) return 0;
    if(send(fd, data.c_str(), data.empty(), 0) < 0) return -1;
    return 1;
}

void Node::sendAppendEntries(int nodeid, LogEntry &Logdata, int& numReplySuccess){
    auto ip = NodeIp[nodeid];
    auto port = NodePort[nodeid];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddress.sin_port = htons(port);

    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        cout << "Failed to connect to server" << endl;
        close(sockfd);
        return;
    }
    stringstream ss;
    ss << "Log_APPEND " <<  Logdata.index <<  Logdata.term <<  Logdata.LogContent.type << Logdata.LogContent.data;
    auto message = ss.str();
    send(sockfd, message.c_str(), message.size(), 0);
    //发送完毕
    //处理请求  监听sockfd这个套接字 看是否有回复
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    timeval timeout{};
    timeout.tv_sec = 5;
    int numReady = select(sockfd + 1, &readfds, nullptr, nullptr, &timeout);
    if (numReady == -1) {
        cout << "Error in select" << endl;
    } else if (numReady == 0) {
        cout << "Timeout: No response received" << endl;
    } else {
        if (FD_ISSET(sockfd, &readfds)) {
            char buffer[1024];
            int bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
            if (bytesRead == -1) {
                cout << "Error in recv" << endl;
            } else if (bytesRead == 0) {
                cout << "Connection closed by remote server" << endl;
            } else {
                // 处理接收到的回复信息
                string response(buffer, bytesRead);
                if(response.find("true") != string::npos) numReplySuccess ++;
            }
        }
    }
    close(sockfd);
}


void Node::HandleLog(LogEntry &Mylog){
    if(Mylog.term < currentTerm)
        sendLogResponse(myNodeId, Mylog.LogContent.type, Mylog.LogContent.data, false);
    else {
        LogEntries.push_back(Mylog);
        sendLogResponse(myNodeId, Mylog.LogContent.type, Mylog.LogContent.data, true);
    }
}

void Node::sendLogResponse(int nodeid, string& type, string& data, bool success){
    auto ip = NodeIp[currentLeaderId];
    auto port = NodePort[currentLeaderId];
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cout << "sock create fail\n";
        return;
    }
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip.c_str());
    serverAddress.sin_port = htons(port);

    if (connect(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1) {
        cout << "Failed to connect to server" << endl;
        close(sockfd);
        return;
    }

    stringstream ss;
    ss << "Log_Response " << nodeid << " " << type << " " << data << " " << (success ? " true" : " false");

    auto message = ss.str();
    send(sockfd, message.c_str(), message.size(), 0);
    close(sockfd);
}