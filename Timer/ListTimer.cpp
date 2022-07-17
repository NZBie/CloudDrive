#include "ListTimer.h"

SortTimerList::~SortTimerList() {

	// 循环删除结点
	UtilTimer* tmp = nullptr;
	while(head != nullptr) {
		tmp = tmp->next;
		delete head;
		head = tmp;
	}
}

// 添加定时器到容器中
void SortTimerList::add_timer(UtilTimer* node) {

	if(node == nullptr) return;

	// 链表为空
	if(head == nullptr) {
		head = tail = node;
		return;
	}

	// 找到合适的插入位置
	UtilTimer* tmp = head;
	while(tmp != nullptr && node->expire > tmp->expire) tmp = tmp->next;
	
	// 插入头部
	if(tmp == head) {
		node->next = tmp;
		tmp->prev = node;
		head = node;
	}

	// 插入尾部
	else if(tmp == nullptr) {
		tmp->next = node;
		node->prev = tmp;
		tail = node;
	}

	// 插入中间
	else {
		UtilTimer* prev = tmp->prev;
		prev->next = node;
		node->prev = prev;
		tmp->prev = node;
		node->next = tmp;
	}
}

// 删除定时器
void SortTimerList::delete_timer(UtilTimer* node) {

	if(node == nullptr) return;

	// 删除唯一结点
	if(node == head && node == tail) {
		head = nullptr;
		tail = nullptr;
	}

	// 删除头结点
	if(node == head) {
		head = head->next;
		head->prev = nullptr;
	}

	// 删除尾结点
	else if(node == tail) {
		tail = tail->prev;
		tail->next = nullptr;
	}

	// 删除中间结点
	else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}
	delete node;
}

// 调整定时器位置
void SortTimerList::adjust_timer(UtilTimer* node) {

}

// 
void SortTimerList::tick() {

	// 链表为空
	if(head == nullptr) return;

	// 取出并执行所有到点的定时器
	time_t now = time(nullptr);
	while(head != nullptr && now >= head->expire) {
		head->cb_func(head->user_data);
		delete_timer(head);
	}
}

// 初始化定时器工具类
void Utils::init(int time_slot) {
	_time_slot = time_slot;
}

//设置信号函数
void Utils::add_signal(int sig, void (handler)(int), bool restart) {

    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if(restart) sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);

    assert(sigaction(sig, &sa, NULL) != -1);
}

//信号处理函数
void Utils::signal_handler(int sig) {

    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(_pipe_fd[1], (char *)(&msg), 1, 0);
    errno = save_errno;
}

// 执行定时任务，并重新定时
void Utils::timer_handler() {
    _timer_list.tick();
    alarm(_time_slot);
}

// 出错
void Utils::show_error(int conn_fd, const char *info) {
    send(conn_fd, info, strlen(info), 0);
    close(conn_fd);
}

int Utils::_epoll_fd = 0;
int* Utils::_pipe_fd = nullptr;

void cb_func(Client_data* user_data) {

	epoll_ctl(Utils::_epoll_fd, EPOLL_CTL_DEL, user_data->sock_fd, 0);
	assert(user_data);
	close(user_data->sock_fd);
	// Http
}