#ifndef REACTOR_H
#define REACTOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstdio>
#include <string.h>
#include <assert.h>

#include "../MySQL_CGI/SqlConnPool.h"
#include "../Http/HttpConn.h"
#include "../ThreadPool/ThreadPool.h"
#include "../log/log.h"

using std::string;

const int MAX_FD_NUM = 2048;
const int MAX_EVENT_NUM = 1024;

class Reactor {
public:
	Reactor(short port, ThreadPool<HttpConn>* thread_pool, SqlConnPool* conn_pool):
		_port(port), _thread_pool(thread_pool), _conn_pool(conn_pool) {}; 

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
	void init_timer_pipe();

private:

	// epoll事件表
	int _port;
	int _listen_fd;
	int _epoll_fd;
	epoll_event _events[MAX_EVENT_NUM];

	// 定时器
	int _pipe_fd[2];

	// 线程池
	ThreadPool<HttpConn>* _thread_pool;

	// 数据库连接池
	string sql_user;
	string sql_passwd;
	string sql_dbname;
	SqlConnPool* _conn_pool;

	// _client_fd到用户的索引
	HttpConn _users[MAX_FD_NUM];
};

#endif