#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "MessageProcessor.h"

enum class NodeState
{
    Fllower,
    Candidate,
    Leader
};

class Node
{
public:
    Node(std::string ip, int port, int numNodes, int nodeid, std::vector<std::vector<std::string>> info)
        : ip(ip), port(port), numNodes(numNodes), myNodeId(nodeid)
    {
        currentLeader = 0;
        if (myNodeId == 0)
            currentState = NodeState::Leader;
        else
            currentState = NodeState::Fllower;
        Nodeip.resize(numNodes);
        Nodeport.resize(numNodes);
        for (auto t : info)
        {
            int nodeport = std::stoi(t[2]);
            std::string nodeip = t[1];
            Nodeip[nodeport % 8001] = nodeip;
            Nodeport[nodeport % 8001] = nodeport;
        }
    }
    void start(); // 启动这个节点
private:
    void HandleSockfd(int fd); // 处理fd端口接收到的信息
    void handleClientRequest(std::string &request, int socketfd);
    // ⬆️这是HandleSockfd的一个分支 它能够处理对客户端socketfd处理请求request
    void sendRequestToLeader(std::string &request);
    // ⬆️这是将信息发送给Leader
    void sendMessage(int fd, const std::string &data);
    // ⬆️这个是将data信息发送给fd套接字 比如+OK等
    void sendCommitinfo(int nodeid, std::vector<std::string> &parse_msg);
    // ⬆️这个是将需要commit的信息发送给对应的node以保持数据的一致性
    void Commit(std::vector<std::string> &parse_msg, int fd = -1);
    // ⬆️这个是将parse_msg中的信息提交 然后向fd也就是客户端返回信息 如果是-1则是其他节点 不向客户端返回信息
    void handleCommitRequest(std::string &data);
    // ⬆️这个是处理节点commit的信息 对应的是data

    std::string ip;    // node的ip
    int port;          // node的port
    int numNodes;      // 总共有几个node
    int nodeid;        // 当前nodeid 需要标识
    int myNodeId;      // 自己的id
    int currentLeader; // 当前的leader信息
    NodeState currentState;
    std::unordered_map<std::string, std::string> kvdb;
    std::vector<std::string> Nodeip;
    std::vector<int> Nodeport;
};