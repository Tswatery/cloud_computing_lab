#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>


using namespace std;

vector<string> input_strings;
mutex input_mutex;
condition_variable input_cv;

vector<string> output_strings;
mutex output_mutex;
condition_variable output_cv;

bool input_done = false;
mutex input_done_mutex;
condition_variable input_done_cv;

bool work_done = false;
mutex work_done_mutex;
condition_variable work_done_cv;

atomic<int> complete_job(0);
int sum_of_job = 0;

bool available(int guess, int cell, vector<int>& board)
{
  for (int i = 0; i < NEIGHBOR; ++i) {
    int neighbor = neighbors[cell][i];
    if (board[neighbor] == guess) {
      return false;
    }
  }
  return true;
}

bool solve_sudoku_basic(int which_space, vector<int>& board)
{
  if (which_space >= nspaces) {
    return true;
  }

  // find_min_arity(which_space);
  int cell = spaces[which_space];

  for (int guess = 1; guess <= NUM; ++guess) {
    if (available(guess, cell, board)) {
      // hold
      assert(board[cell] == 0);
      board[cell] = guess;

      // try
      if (solve_sudoku_basic(which_space+1, board)) {
        return true;
      }

      // unhold
      assert(board[cell] == guess);
      board[cell] = 0;
    }
  }
  return false;
}


void input_thread(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        {
            unique_lock<mutex> lock(input_mutex);
            input_strings.push_back(line);
            sum_of_job ++;
        }
        input_cv.notify_one();
    }
    //读入完毕
    input_done = true;
	input_cv.notify_all(); // 读入结束后要全部唤醒
}

void work_thread()
{
    while (true) {
        unique_lock<mutex> input_lock(input_mutex);
        input_cv.wait(input_lock, []{ return !input_strings.empty() || input_done; }); // 表示对条件的检查

        if (input_strings.empty() && input_done && complete_job == sum_of_job) {
            break;
        }

        string str = input_strings.front();
        input_strings.erase(input_strings.begin());
        input_lock.unlock();
        // 计算str即可。还没有计算
        complete_job ++;

        {
            lock_guard<mutex> output_lock(output_mutex);
            output_strings.push_back(str);
        }
        output_cv.notify_one();
    }
    if(complete_job == sum_of_job){
    	work_done = true;
    	output_cv.notify_all();
    }
}

void output_thread()
{
    while (true) {
        unique_lock<mutex> output_lock(output_mutex);
        output_cv.wait(output_lock, []{ return !output_strings.empty() || work_done; });

        if (output_strings.empty() && work_done) {
            break;
        }

        string str = output_strings.front();
        output_strings.erase(output_strings.begin());
        output_lock.unlock();
        cout << str << endl;
    }

    output_cv.notify_one();
}

int main()
{
    thread input(input_thread, "./test1000");
    const int num_threads = thread::hardware_concurrency() - 2; // use all available cores except main and output threads
    vector<thread> workers;
    for (int i = 0; i < 1; i++) {
        workers.emplace_back(work_thread);
    }
    thread output(output_thread);
    input.join();
    for (auto& worker : workers) {
        worker.join();
    }
    output.join();
    return 0;
}
