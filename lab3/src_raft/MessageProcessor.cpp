#include "MessageProcessor.h"

using namespace std;

bool MessageProcessor::parseClinetMessage(string &data, vector<string> &res){
    if(data.empty()) return false;
    /** 起先客户端会有一个* 加 数字比如 *1\r\n 1表示后续字符串的数量
     * 客户端发来的数据是这样的：$7\r\nCS06142\r\n
     * 可以使用stringstream来解决\r\n的读取问题 可以直接忽略
     * 读到的结果就是$7和CS06142
     * 其实可以不用管*和$的行，直接忽略即可
    */
    res.clear();
    stringstream sscin(data); 
    string tmp;
    while(sscin >> tmp){
        if(tmp[0] != '*' && tmp[0] != '$'){
            res.push_back(tmp);
        }
    }

    int linenum = res.size(); // 有多少个请求
    if(linenum < 2 || (res[0] != "SET" && res[0] != "GET" && res[0] != "DEL")){
        cout << "parseClinetMessage fail\n";
        return false;
    }
    /**可以发现在SET操作的时候，value是分开的，因此选择合并它们
     * *4\r\n$3\r\nSET\r\n$7\r\nCS06142\r\n$5\r\nCloud\r\n$9\r\nComputing\r\n
     * 解析结果是 SET CS06142 Cloud Computing
    */
    if(res[0] == "SET" && linenum > 3){
        for(int i = 3; i < linenum; ++ i)
            data[2] += (' ' + data[i]);
        for(int i = 3; i < linenum; ++ i)
            data.pop_back(); // 删除合并的
    }
    return true;
}

string MessageProcessor::getClinetERROR(){
    return "-ERROR\r\n";
}

string MessageProcessor::getClientOK(){
    return "+OK\r\n";
}

vector<vector<string>> MessageProcessor::parseConfigFile(const string &path){
    vector<vector<string>> res; // 结果

    ifstream fin(path);
    if(!fin){
        perror("打不开文件");
        exit(1);
    }
    string str;
    while(getline(fin, str)){
        if(str.empty() || str[0] == '!') continue;
        // !是注释或者空行则跳过
        vector<string> split_data;
        split_data.clear();
        stringstream sscin(str);
        string word;
        while(sscin >> word)
            split_data.push_back(word);
        // 使用stringstream来解析文件
        auto pos = split_data[1].find(":");
        split_data.push_back(split_data[1].substr(pos + 1));
        split_data[1] = split_data[1].substr(0, pos);
        res.push_back(split_data);
    }
    return res;
}