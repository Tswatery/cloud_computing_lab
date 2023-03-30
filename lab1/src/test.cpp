#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#define cv condition_variable

using namespace std;

mutex lock_;
queue<string> q;
cv cv1;

void pre_func(){
	while(1){
		string s;
		cin >> s;
		{
			unique_lock<mutex> lock1(lock_);
			q.push(s);
		}
		cv1.notify_one();
	}	
}

void out_func(){
	while(1){
		{
			unique_lock<mutex> lock1(lock_);
			cv1.wait(lock1, []{return !q.empty();});
		}
		cout << q.front() << endl;
		q.pop();
	}
}

int main(){
	thread pre(pre_func);
	thread out(out_func);

	pre.join();
	out.join();
}
