#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "DLX.h"
#include <unordered_map>
#include <atomic>
#define DEBUG 0

using namespace std;

queue<string> queue_of_filename;
condition_variable filename_cv;
mutex filename_lock;
bool read_file_done = false;

queue<string> question_of_sudoku;
mutex input_question_lock;
condition_variable input_cv;
bool read_question_done = false;

queue<string> answer_of_sudoku;
mutex output_mutex;
condition_variable output_cv;
bool work_done = false;

atomic<int> work_id_now(0), out_id_now(0);
unordered_map<int, string> ans_map;
//Given an id, we can get the answer by using this map 

void pre_func(){
    string file_name;
	while(1){
	    if(DEBUG)
	        cout << "this is pre_func" << endl;
        cin >> file_name;
        if(cin.eof()) break;
	    {
		    unique_lock<mutex> lock(filename_lock);
		    queue_of_filename.push(file_name);
	    }
	    filename_cv.notify_one();
	    this_thread::yield();
	}
	// 读入完毕
	read_file_done = true;
	filename_cv.notify_one();
}

void input_func(){
	while(1){
	    if(DEBUG)
            cout << "this is input_func" << endl;
		string file, line;
		unique_lock<mutex> lock(filename_lock);
		filename_cv.wait(lock, []{return !queue_of_filename.empty() || read_file_done;});
		//对条件的检查，也就是说条件变量在拿到锁之后并不保证队列是有东西的
		if(queue_of_filename.empty() && read_file_done) break;
		//如果重新取文件名 发现文件名的队列是空的并且已经发出了读完了的信号那就break
		file = queue_of_filename.front();
		queue_of_filename.pop();
		ifstream fp(file);// read from file, will not affect out stream.
	    if (!fp.is_open()) {
	        cerr << "Failed to open file " << file << endl;
	        return;
	    }
		while(getline(fp, line)){
			{
				unique_lock<mutex> lock_in(input_question_lock);
				question_of_sudoku.push(line);
				input_cv.notify_one();
				if(DEBUG)
					cout << line << endl; // this can be on terminal
			}
		}
		fp.close();
	}
	read_question_done = true;
	input_cv.notify_one();
}

void work_func(){
	while(1){
	    if(DEBUG)
	        cout << "this is work_func" << endl;
		unique_lock<mutex> lock_in(input_question_lock);
		input_cv.wait(lock_in, []{return !question_of_sudoku.empty() || read_question_done;});
		if(question_of_sudoku.empty() && read_question_done) break;
		//如果问题队列空了并且已经读完了问题 就break
		string str = question_of_sudoku.front();
		question_of_sudoku.pop();
		DLX solver;
		solver.build(729, 324);
		for (int i = 1; i <= 9; ++i)
			for (int j = 1; j <= 9; ++j) {
				solver.ans[i][j] = str[(i - 1) * 9 + j - 1] - '0';
				for (int v = 1; v <= 9; ++v) {
					if (solver.ans[i][j] && solver.ans[i][j] != v) continue;
					Insert(i, j, v, solver);
				}
		}
		solver.dance(1);
		str.clear();
		for(int i = 1; i <= 9; ++ i)
			for(int j = 1; j <= 9; ++ j)
				str += char(solver.ans[i][j] + '0');
		if(DEBUG)
			cout << str << endl; // this can not.
		unique_lock<mutex> lock_out(output_mutex);
		answer_of_sudoku.push(str);
		ans_map[work_id_now] = str;
		work_id_now ++;
		output_cv.notify_one();
	}
	work_done = true;
	output_cv.notify_one();
}

void output_func(){
	while(1){
	    if(DEBUG)
            cout << "this is output_func" << endl;
		unique_lock<mutex> lock_out(output_mutex);
		output_cv.wait(lock_out, []{return !answer_of_sudoku.empty() || work_done;});
		if(answer_of_sudoku.empty() && work_done) break;
		//cout << answer_of_sudoku.front() << endl;
		if(out_id_now > work_id_now) this_thread::yield();
		cout << ans_map[out_id_now] << endl;
		out_id_now ++;
		answer_of_sudoku.pop();
	}
}


int main(){
	thread pre(pre_func);
	thread input(input_func);
	const int num_threads = thread::hardware_concurrency(); 
	vector<thread> workers;
	for(int i = 0; i < num_threads; ++ i){
	    workers.emplace_back(work_func);
	}
	thread output(output_func);

	pre.join();
	input.join();
	for(auto& t : workers){
	    t.join();
	}
	//work.join();
	output.join();
	return 0;
}
