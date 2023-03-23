#include <bits/stdc++.h>
#include <semaphore.h>
#include <pthread.h>
#define DEBUG 1

using namespace std;

const int N = 1e5 + 10;

pthread_mutex_t jobQueueMutex=PTHREAD_MUTEX_INITIALIZER;
// 这个锁还是要有的，因为我们对队列的插入和取任务是同时的

sem_t fullSlots; 
// 信号量 用来记录任务队列中的任务是否为空

struct Job
{
	int id; 
	vector<vector<int> > ans;
}Job_queue[N];

typedef struct {
	char filename[];
} ThreadParas;

 
int Job_We_Have_Now = 0;

void* Input_a_Sudoku_into_a_queue(void *args){
	// 读线程
	ThreadParas* para = (ThreadParas*) args;
	ifstream fp(para->filename);
	string line;
	while(getline(fp, line)){
		pthread_mutex_lock(&jobQueueMutex);
//		获取锁
		Job_queue[Job_We_Have_Now].ans.resize(10);
		for(auto& t : Job_queue[Job_We_Have_Now].ans)
			t.resize(10);
		for(int i = 1; i <= 9; ++ i){
			for(int j = 1; j <= 9; ++ j){
				int idx = (i - 1) * 9 + j - 1;
				Job_queue[Job_We_Have_Now].ans[i][j] = line[idx] - '0';
			}
		}
		Job_queue[Job_We_Have_Now].id = Job_We_Have_Now;
		Job_We_Have_Now ++;
		pthread_mutex_unlock(&jobQueueMutex);// 释放锁
		sem_post(&fullSlots); // 队列中的任务+1
	}
}

void print(vector<vector<int> > a){
	for(int i = 1; i < a.size(); ++ i){
		for(int j = 1; j < a.size(); ++ j){
			cout << a[i][j];
		}
	}
	cout << endl;
}

int main(){
	char name[1010];
	scanf("%s", name);
	// printf("%s", name);
	pthread_t Read;
	pthread_create(&Read, NULL, Input_a_Sudoku_into_a_queue, name);
	pthread_join(Read, NULL);
	if(DEBUG){
		// printf("共有%d个任务\n", Job_We_Have_Now);
		for(int i = 0; i < Job_We_Have_Now; ++ i){
			print(Job_queue[i].ans);
		}
	}
}
