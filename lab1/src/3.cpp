#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "DLX.h"
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


void pre_func(){
	string file_name;
	 while(cin >> file_name){
	 	{
			unique_lock<mutex> lock(filename_lock);
			queue_of_filename.push(file_name);
			cout << "读取了" << file_name << endl;
		}
		filename_cv.notify_one();
	}
	// 读入完毕
	read_file_done = true;
	filename_cv.notify_one();
}

void input_func(){
	while(1){
		string file, line;
		unique_lock<mutex> lock(filename_lock);
		filename_cv.wait(lock, []{return !queue_of_filename.empty() || read_file_done;});
		//对条件的检查，也就是说条件变量在拿到锁之后并不保证队列是有东西的
		if(queue_of_filename.empty() && read_file_done) break;
		//如果重新取文件名 发现文件名的队列是空的并且已经发出了读完了的信号那就break
		file = queue_of_filename.front();
		queue_of_filename.pop();
		ifstream fp(file);
	    if (!fp.is_open()) {
	        cerr << "Failed to open file " << file << endl;
	        return;
	    }
	    while(!question_of_sudoku.empty()) question_of_sudoku.pop();
	    //以防万一 将数独队列中的字符串全部弹出
		while(getline(fp, line)){
			{
				unique_lock<mutex> lock_in(input_question_lock);
				question_of_sudoku.push(line);
				if(DEBUG)
					cout << line << endl;
			}
		}
	}
	read_question_done = true;
	input_cv.notify_one();
}

void work_func(){
	while(1){
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
		string s;
		for(int i = 1; i <= 9; ++ i)
			for(int j = 1; j <= 9; ++ j)
				s += char(solver.ans[i][j] + '0');
		cout << s << endl;
		unique_lock<mutex> lock_out(output_mutex);
		answer_of_sudoku.push(s);
		output_cv.notify_all();
	}
	work_done = true;
	output_cv.notify_all();
}

void output_func(){
	while(1){
		unique_lock<mutex> lock_out(output_mutex);
		output_cv.wait(lock_out, []{return !answer_of_sudoku.empty() || work_done;});
		if(answer_of_sudoku.empty() && work_done) break;
		cout << answer_of_sudoku.front() << endl;
		answer_of_sudoku.pop();
	}
}


int main(){
	thread pre(pre_func);
	thread input(input_func);
	thread work(work_func);
	// thread output(output_func);

	pre.join();
	input.join();
	work.join();
	// output.join();
}