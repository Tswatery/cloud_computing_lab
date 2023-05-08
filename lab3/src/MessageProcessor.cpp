#include "MessageProcessor.h"

using namespace std;

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
        // !是注释
        vector<string> spilt_data;
        cutStringBySpace(str, spilt_data); // 使用stringsteam来划分字符串
        /**
         * split_data[0]: mode coordinator | participant
         * split_data[1]: 
        */
        if(spilt_data[0] == "coordinator_info" || spilt_data[0] == "participant_info"){
            auto pos = spilt_data[1].find(":");
            spilt_data.push_back(spilt_data[1].push_back(spilt_data.substr(pos + 1)));
            spilt_data[1] = spilt_data[1].substr(0, pos);
        }
        res.push_back(spilt_data);
    }
    return res;
}

void cutStringBySpace(string& str, vector<string>& split_data){
    split_data.clear();
    stringstream sscin(str);
    string word;
    while(sscin >> word)
        split_data.push_back(word);
}