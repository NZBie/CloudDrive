#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using std::string;

namespace config {

	// 服务器ip及端口
	const string URL = "localhost";
	const short SERVER_PORT = 9006;

	// 反应堆
	const int MAX_FD_NUM = 65536;
	const int MAX_EVENT_NUM = 8192;
	const int TIME_SLOT = 5;

	// Http
	const int FILENAME_LEN = 128;
	const int READ_BUFFER_SIZE = 1024;
	const int WRITE_BUFFER_SIZE = 1024;

	// 数据库连接池
	const short _MYSQL_PORT = 3306;
	const string MYSQL_NAME = "root";
	const string MYSQL_PASSWD = "123456";
	const string MYSQL_DBNAME = "CloudDrive";
	const int MAX_CONN_NUM = 8;

	// 线程池
	const int THREAD_NUM = 8;
	const int MAX_TASK_NUM = 16384;

	// 日志
	const bool LOG_OPEN = true;
	// bool log_mode = 1;
}

#endif
