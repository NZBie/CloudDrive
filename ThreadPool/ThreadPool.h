#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <list>
#include <assert.h>
#include <exception>
#include <cstdio>

#include "../Locker/Locker.h"

template<typename T>
class ThreadPool {
public:
	ThreadPool(int thread_num, int max_task_num);
	~ThreadPool();

	bool append(T* task, int state);	// 放入任务

private:
	static void* worker(void* arg);
	void run();					// 循环取出任务进行处理

private:
	int _thread_num;			// 线程池容量
	pthread_t* _threads;		// 线程池数组

	int _max_task_num;			// 等待队列的最大任务数
	std::list<T*> _task_queue;	// 等待队列
	Locker _queue_locker;		// 保护等待队列的互斥锁
	Sem _queue_semaphore;		// 是否剩余待处理的任务
};

template<typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_task_num):
_thread_num(thread_num), _threads(nullptr), _max_task_num(max_task_num) {

	// 创建线程
	assert(_thread_num > 0 && _max_task_num > 0);
	_threads = new pthread_t[_thread_num];
	assert(_threads);

	// 启动线程 & 分离线程
	// 线程入口函数: ThreadPoll::run()
	for(int i=0;i<thread_num;++i) {
		if(pthread_create(_threads+i, NULL, worker, this) || pthread_detach(_threads[i])) {
			delete[] _threads;
			throw std::exception();
		}
	}
}

template<typename T>
ThreadPool<T>::~ThreadPool() {
	delete[] _threads;
}

// 将任务加入队列
template<typename T>
bool ThreadPool<T>::append(T* task, int state) {

	// 加锁
	_queue_locker.lock();

	// 任务数达到上限
	if(_task_queue.size() >= _max_task_num) {
		_queue_locker.unlock();
		return false;
	}

	// 加入等待队列
	task->_state = state;
	_task_queue.push_back(task);

	// 解锁 & 信号量++
	_queue_locker.unlock();
	_queue_semaphore.post();

	return true;
}

// 全局函数 转成 类成员函数
template<typename T>
void* ThreadPool<T>::worker(void* arg) {

	ThreadPool<T>* pool = (ThreadPool<T>*)arg;
	pool->run();
	return nullptr;
}

// 循环从_task_queue等待队列中取出任务处理
template<typename T>
void ThreadPool<T>::run() {

	while(true) {

		// 等待 其他线程 操作_task_queue 完成
		_queue_semaphore.wait();
		_queue_locker.lock();
		if(_task_queue.empty()) {
			_queue_locker.unlock();
			continue;
		}

		// 从等待队列中取出任务
		T* task = _task_queue.front();
		_task_queue.pop_front();
		_queue_locker.unlock();
		if(task == nullptr) continue;
		
		// 处理请求报文
		if(task->_state == 0) {
			int ret = task->read_request();
			if(ret == false) {
				task->timer_flag = true;
			}
			task->improve = true;
			if(ret) task->complete_process();
		}
		else {
			if(task->write_response() == false) {
				task->timer_flag = true;
			}
			task->improve = true;
		}
	}
}

#endif