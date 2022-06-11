#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <list>
#include <assert.h>
#include <exception>

#include "../Locker/Locker.h"

template<typename T>
class ThreadPool {
public:
	ThreadPool(int thread_num, int max_request);
	~ThreadPool();

	bool append(T *request, int state = 0);	// 放入请求

private:
	void* worker(void* arg);
	void run();						// 运行loop函数

private:
	int _thread_num;				// 线程池容量
	pthread_t* _threads;			// 线程池数组

	int _max_request_num;			// 请求队列允许的最大请求数
	std::list<T*> _request_queue;	// 请求队列
	Locker _request_locker;			// 保护请求队列的互斥锁
	Sem _request_semaphore;			// 是否剩余待处理的请求
};

#endif