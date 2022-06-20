#ifndef REACTOR_H
#define REACTOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cstdio>
#include <string.h>
#include <assert.h>

#include "Mysql_CGI/SqlConnPool.h"
#include "Http/HttpConn.h"
#include "ThreadPool/ThreadPool.h"

using std::string;

const int MAX_EVENT_NUM = 100;
const int MAX_FD_NUM = 65536;

class Reactor {
public:
	Reactor();
	~Reactor();

	void init(
		int thread_num, int max_task_num,
		string url, short port, string user, string passwd, string dbname
	);

	// epoll相关
	void event_listen();
	void event_loop();

	// 处理客户端的请求
	bool deal_client_connect();
	void deal_client_read(int client_fd);
	void deal_client_write(int client_fd);

	// 线程池 & 数据库连接池 的初始化	
	void init_thread_pool(int thread_num, int max_task_num);
	void init_sql_conn_pool(string url, short port, string user, string passwd, string dbname);

	// 线程池 & 数据库连接池
	ThreadPool<HttpConn>* _thread_pool;
	SqlConnPool* _conn_pool;

	// client_fd到用户的索引
	HttpConn _users[MAX_FD_NUM];

private:
	int _listen_fd;
	int _epoll_fd;
	int _port;

	char* _root_path;

	epoll_event _events[MAX_EVENT_NUM];
};

#endif