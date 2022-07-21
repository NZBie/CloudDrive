#ifndef REACTOR_H
#define REACTOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>

#include <cstdio>
#include <string.h>
#include <assert.h>

#include "../MySQL_CGI/SqlConnPool.h"
#include "../Http/HttpConn.h"
#include "../ThreadPool/ThreadPool.h"
#include "../log/log.h"
#include "../Timer/ListTimer.h"
#include "../config.h"

using std::string;

class Reactor {
public:
	Reactor(short port, ThreadPool<HttpConn>* thread_pool, SqlConnPool* conn_pool);

	~Reactor();

	// epoll相关
	void event_listen();
	void event_loop();

private:

	// 处理客户端的请求
	bool deal_client_connect();
	void deal_client_read(int client_fd);
	void deal_client_write(int client_fd);

	// epoll添加、修改事件
	void add_event(int epoll_fd, int sock_fd, int events);
	void set_nonblocking(int sock_fd);

	// 定时器相关
	void init_utils_timer();
	void new_timer(int client_fd, sockaddr_in client_address);
	bool deal_with_signal(bool& timeout, bool& stop_server);
	void adjust_timer(UtilTimer* timer);
	void delete_timer(UtilTimer* timer, int client_fd);

private:

	// epoll事件表
	int _port;
	int _listen_fd;
	int _epoll_fd;
	epoll_event _events[config::MAX_EVENT_NUM];

	// 线程池
	ThreadPool<HttpConn>* _thread_pool;

	// 数据库连接池
	string sql_user;
	string sql_passwd;
	string sql_dbname;
	SqlConnPool* _conn_pool;

	// _client_fd到用户的索引
	HttpConn* _users;

	// 定时器
	int _pipe_fd[2];
	Client_data* _users_timer;
	Utils _utils;
};

void cb_func(Client_data* user_data);

#endif