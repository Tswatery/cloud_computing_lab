#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include "helper.h"

using namespace std;

pthread_mutex_t jobQueueMutex=PTHREAD_MUTEX_INITIALIZER;
sem_t SubfullSlots, fullSlots; // 信号量 用来记录子任务队列中的人物是否为空
pthread_mutex_t SubjobQueueMutex = PTHREAD_MUTEX_INITIALIZER; // 子任务队列的锁
string SubjobQueue[100010]; 
Job Job_queue[100010];

/*---------------------------------------------------------------------------------------------------*/
// 打印任务线程所需要的变量
int numsOfjobHavedone = 0;

void* print_ans_of_soduku(void* arg) {
    while(1) {
        sem_wait(&SubfullSlots);
        pthread_mutex_lock(&SubjobQueueMutex); // 把队列上锁
        int res = numsOfjobHavedone ++;
        if(numsOfjobHavedone >= 2) break;
        cout << pthread_self() <<' '<< res << ' ' << SubjobQueue[res] << '\n';
        pthread_mutex_unlock(&SubjobQueueMutex);
    }
}

/*---------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------*/
// 读线程所需要的函数与变量
int Job_We_Have_Now = 0;

void* Input_a_Sudoku_into_a_queue(void *args){
    // 读线程
    string file_name;
    while(cin >> file_name){
        ifstream fp(file_name);
        string line;
        while(getline(fp, line)){
            cout << line << endl;
            pthread_mutex_lock(&jobQueueMutex);
    //      获取锁
            Job_queue[Job_We_Have_Now].s = line;
            Job_queue[Job_We_Have_Now].id = Job_We_Have_Now;
            SubjobQueue[Job_We_Have_Now] = line; 
            Job_We_Have_Now ++;
            sem_post(&fullSlots); // 队列中的任务+1
            pthread_mutex_unlock(&jobQueueMutex);// 释放锁
        }
    }
}
/*---------------------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------------------*/

int nextjobToBeDone = 0;

int recvJob(){
    int current_job = 0;
    sem_wait(&fullSlots); // 避免队列中没有数
    pthread_mutex_lock(&jobQueueMutex);
    current_job = nextjobToBeDone ++;
    if(nextjobToBeDone >= 2) return -1; 
    pthread_mutex_unlock(&jobQueueMutex);
    return current_job;
}

void InputaSubJob(int id, string s){
    pthread_mutex_lock(&SubjobQueueMutex);
    SubjobQueue[id] = s;
    pthread_mutex_unlock(&SubjobQueueMutex);
    sem_post(&SubfullSlots);
}

void* Sudoku(void* arg){
    while(1){
        DLX solver;
        int current_job = recvJob(); 
        if(current_job == -1) break;
        auto job_do_now = Job_queue[current_job]; // 获取任务
        for(int i = 1; i <= 9; ++ i){
            for(int j = 1; j <= 9; ++ j){
                int idx = (i - 1) * 9 + j - 1;
                solver.ans[i][j] = job_do_now.s[idx] - '0';
                for(int v = 1; v <= 9; ++ v){
                    if(solver.ans[i][j] && solver.ans[i][j] != v)
                        continue;
                    Insert(i, j, v, solver);
                }
            }
        }
        solver.dance(1);
        string res;
        for(int i = 1; i <= 9; ++ i){
            for(int j = 1; j <= 9; ++ j){
                char c = char(solver.ans[i][j] + '0');  // 强行转换成char
                res += c;
            }
        }
        InputaSubJob(job_do_now.id, res); // 第job_do_now.id的答案是res
    }
}
/*---------------------------------------------------------------------------------------------------*/

int main()
{
	string file_name;
    int NumOfWorker = 8;
    sem_init(&fullSlots, 0, 0); 
    sem_init(&SubfullSlots, 0, 0); // 每一个文件刚开始都需要初始化这个队列。
    pthread_t Read, Output; // 读线程的标识量
    pthread_create(&Read, NULL, Input_a_Sudoku_into_a_queue, NULL); // 读线程是没有问题的
    // pthread_t Worker[8];
    // for(int i = 0; i < 8; ++ i)
    //     pthread_create(&Worker[i], NULL, Sudoku, NULL);
    pthread_create(&Output, NULL, print_ans_of_soduku, NULL);
    pthread_join(Read, NULL);
    pthread_join(Output, NULL);
    // for(int i = 0; i < 8; ++ i)
    //     pthread_join(Worker[i], NULL);
    return 0;
}