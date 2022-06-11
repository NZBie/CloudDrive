#ifndef SQL_CONNECTION_POOL_H
#define SQL_CONNECTION_POOL_H

#include <mysql.h>

#include <list>
#include <string>

#include "../Locker/Locker.h"

using std::string;

class SqlConnPool {
private:
	SqlConnPool();
	~SqlConnPool();

public:
	static SqlConnPool* get_instance();		// 获取唯一实例
	void init(string url, short port, string user, string passwd, string database);
	void destroy();							// 销毁连接池

	MYSQL* get_connection();				// 获取连接
	bool release_connection(MYSQL *conn);	// 释放连接
	int get_free_conn_num();				// 获取空闲连接数

private:
	int _max_conn_num;	// 连接池容量
	int _conn_num;		// 当前已用连接数
	int _conn_free;		// 当前空闲连接数

	std::list<MYSQL*> _conn_list;	// sql连接池集合
	Locker _conn_locker;
	Sem _conn_semaphore;

	string _url;		// 数据库 所在主机地址
	short _port;		// 数据库 端口号
	string _user;		// 数据库 用户名
	string _passwd;		// 数据库 密码
	string _database;	// 使用的数据库名
};

#endif