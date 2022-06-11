#include "ThreadPool.h"

template<typename T>
ThreadPool<T>::ThreadPool(int thread_num, int max_request):
_thread_num(thread_num), _max_requests(max_request) {

	// 创建线程
	assert(_thread_num > 0 && _max_request_num > 0);
	_threads = new pthread_t[_thread_num];
	assert(threads);

	// 启动线程 & 分离线程
	// 线程入口函数: ThreadPoll::run()
	for(int i=0;i<thread_num;++i) {
		if(pthread_create(_threads+i, NULL, run) || pthread_detach(_threads[i])) {
			delete[] _threads;
			throw std::exception();
		}
	}
}

template<typename T>
ThreadPool<T>::~ThreadPool() {
	delete[] _threads;
}

template<typename T>
bool ThreadPool<T>::append(T *request, int state = 0) {

	// 加锁
	_request_locker.lock();

	// 请求数达到上限
	if(_request_queue.size() >= _max_request_num) {
		_request_locker.unlock();
		return false;
	}

	// 加入请求队列
	request->_state = state;
	_request_queue.push_back(request);

	// 解锁 & 信号量++
	_request_locker.unlock();
	_request_semaphore.post();

	return true;
}

// template<typename T>
// void* ThreadPool<T>::worker(void* arg) {

// 	ThreadPool* pool = (1);
// }

template<typename T>
void ThreadPool<T>::run() {

	while(true) {

		// 等待 其他线程 操作_request_queue 完成
		_request_semaphore.wait();
		_request_locker.lock();
		if(_request_queue.empty()) {
			_request_locker.unlock();
			continue;
		}

		// 从请求队列中取出请求
		T* request = _request_queue.front();
		_request_queue.pop_front();
		_request_locker.unlock();

		if(request == nullptr) continue;

		// 处理该条请求request
		if(request->read_once()) {
			request->process();
		}
		else {

		}
	}
}