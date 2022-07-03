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

#include "Mysql_CGI/SqlConnPool.h"
#include "Http/HttpConn.h"
#include "ThreadPool/ThreadPool.h"
#include "log/log.h"

using std::string;

const int MAX_FD_NUM = 2048;
const int MAX_EVENT_NUM = 1024;

class Reactor {
public:
	// 获取唯一实例
	// static SqlConnPool* get_instance();
	Reactor();
	~Reactor();

	// 初始化
	void init(
		int thread_num, int max_task_num,
		short port, string user, string passwd, string dbname, int max_conn_num
	);

	// epoll相关
	void event_listen();
	void event_loop();

private:

	// 处理客户端的请求
	bool deal_client_connect();
	void deal_client_read(int client_fd);
	void deal_client_write(int client_fd);

	// 日志 & 线程池 & 数据库连接池 的初始化
	void init_log();
	void init_thread_pool(int thread_num, int max_task_num);
	void init_sql_conn_pool(short port, string user, string passwd, string dbname, int max_conn_num);

private:
	int _listen_fd;
	int _epoll_fd;
	int _port;
	char* _root_path;
	string sql_user;
	string sql_passwd;
	string sql_dbname;

	// 事件表
	epoll_event _events[MAX_EVENT_NUM];

	// 线程池 & 数据库连接池
	ThreadPool<HttpConn>* _thread_pool;
	SqlConnPool* _conn_pool;

	// _client_fd到用户的索引
	HttpConn _users[MAX_FD_NUM];
};

#endif