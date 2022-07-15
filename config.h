#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using std::string;

namespace config {

	// 服务器ip及端口
	string url = "localhost";
	short server_port = 9006;

	// 数据库连接池
	short mysql_port = 3306;
	string mysql_name = "root";
	string mysql_passwd = "123456";
	string mysql_dbname = "CloudDrive";
	int max_conn_num = 8;

	// 线程池
	int thread_num = 8;
	int max_task_num = 1000;

	// 日志
	bool log_open = true;
	// bool log_mode = 1;
}

#endif
