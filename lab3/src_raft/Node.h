#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <functional>
#include <thread>
#include "MessageProcessor.h"
#define MAX_BUF_SIZE 1024

#ifndef __NODE_H__
#define __NODE_H__

// typedef 1024 MAX_BUF_SIZE;

enum class NodeState
{
    Fllower,
    Candidate,
    Leader
};


struct Operation{
    std::string type; // 操作的类型
    std::string data; // 操作的数据 SET key value  GET key  DEL key1 key2 ...
};
struct LogEntry{
    int index;
    int term;
    Operation LogContent;
};

// 在一个log的条目中有这条信息是哪一任期被加入term的、现在这个条目是第几个索引 index

class Node
{
public:
    Node(std::string ip, int port, int numNodes, int nodeid, std::vector<std::vector<std::string>> info):ip(ip), port(port), numNodes(numNodes), myNodeId(nodeid){
        currentTerm = 0;
        votesReceived = 0;
        votedFor = 0;
        LogEntries.clear();
        currentLeaderId = 0; // 默认第一个是Leader
        logalready = 0;
        if(myNodeId == 0) 
            currentState = NodeState::Leader;
        else 
            currentState = NodeState::Fllower;
        NodeIp.resize(numNodes);
        NodePort.resize(numNodes);
        for(auto t : info){
            int idx = std::stoi(t[2]) % 8001;
            NodeIp[idx] = t[1];
            NodePort[idx] = std::stoi(t[2]);
        }
    }

    void start();

private:

    void startElectionTimer();
    void startTimer(int timeout, std::function<void()> callback); // 超时函数
    int generateElectionTimeout(); // 生成随机的超时时间
    void resetElectionTimer();
    void becomeFollower();
    int getlastLogIndex(); int getlastLogTerm(); // 最近的一个log信息

    void becomeCandidate(); // 成为候选者应该做的事
    void sendRequestVote(int Nodeid); // 向某一个node发送请求投票的消息
    void handleRequestVoteRequest(int candidateId, int term, int lastLogIndex, int lastLogTerm);
    bool isCandidateLogUpToDate(int lastLogIndex, int lastLogTerm);
    void sendRequestVoteResponse(int candidateId, int currentTerm, bool VoteGranted);
    void handleRequestVoteResponse(int term, bool voteGranted);

    void becomeLeader();
    void sendHeartBeats();
    void sendHeartBeatToOne(int nodeid);
    void handlerHeartbeat(int leaderid, int term);
    int getLeaderNodeId(); // 返回leader的nodeid
    void sendClientRequestToNode(int nodeid, std::string& message);

    // 处理业务
    void handleClientRequest(std::string& request, int socketfd);
    void sendRequestToLeader(std::string& request);
    void HandleSockfd(int fd);
    int getMessage(int fd, std::string& data);

    //发送消息
    int sendMessage(int fd, const std::string& data);

    //日志复制过程
    LogEntry CreateLogData(std::vector<std::string> &message);
    void sendAppendEntries(int nodeid, LogEntry &Logdata, int& numReplySuccess);
    void HandleLog(LogEntry &Mylog);
    void sendLogResponse(int nodeid, std::string& type, std::string& data, bool success);
    void HandleLogResponse(int nodeid, std::string type, std::string& data, bool flag);
    void receiveAppendEntrySuccess(int nodeid);
    //提交
    void CommitLog(LogEntry Logdata, int fd = -1); // 提交信息
    void sendCommitToFollowers(); // 给所有的节点发送commit的信息
    void SendCommitMsg(int nodeid); // 给nodeid的节点发送commit的信息

// 变量
    std::string ip;
    int port;
    int numNodes;
    int myNodeId;
    NodeState currentState;
    int currentTerm;
    int votesReceived;
    int votedFor;
    std::vector<std::string> NodeIp;
    std::vector<int> NodePort;
    std::vector<LogEntry> LogEntries; // 日志都复制到这里面
    std::thread electionTimerThread;
    int currentLeaderId;
    int logalready;
    std::unordered_map<std::string, std::string> kvdb; // 提交
};

#endif // __NODE_H__