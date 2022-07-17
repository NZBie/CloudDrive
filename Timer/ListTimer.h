#ifndef LIST_TIMER_H
#define LIST_TIMER_H

#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>
#include <ctime>
#include <sys/epoll.h>

#include <cstring>

class Client_data {
public:
	sockaddr_in address;
	int sock_fd;
	UtilTimer* timer;
};

class UtilTimer {
public:
	UtilTimer():prev(nullptr), next(nullptr) {};

	time_t expire;
	void (* cb_func)(Client_data*);

	Client_data* user_data;
	UtilTimer* prev;
	UtilTimer* next;
};

// 定时器的容器，有序链表
class SortTimerList {
public:
	SortTimerList();
	~SortTimerList();

	void add_timer(UtilTimer* node);		// 添加定时器
	void delete_timer(UtilTimer* node);		// 删除定时器
	void adjust_timer(UtilTimer* node);		// 调整定时器位置
	void tick();
private:
	UtilTimer* head;
	UtilTimer* tail;
};

class Utils {
public:
	Utils();
	~Utils();

	void init(int time_slot);

	void add_signal(int sig, void(handler)(int), bool restart);
	void signal_handler(int sig);
	void timer_handler();
	void show_error(int conn_fd, const char *info);

public:
	static int _epoll_fd;
	static int* _pipe_fd;
	SortTimerList _timer_list;
	int _time_slot;
};


#endif