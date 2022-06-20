#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using std::string;

namespace config {

	// 服务器ip及端口
	string url = "127.0.0.1";
	short port = 9006;

	// 数据库连接池
	string mysql_name = "root";
	string mysql_passwd = "123456";
	string mysql_dbname = "mywebserver";

	// 线程池
	int thread_num = 5;
	int max_task_num = 10;
}

#endif
