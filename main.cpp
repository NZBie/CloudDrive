
#include <jsoncpp/json/json.h>

#include "config.h"
#include "Reactor/Reactor.h"
#include "MySQL_CGI/SqlConnPool.h"
#include "Http/HttpConn.h"
#include "ThreadPool/ThreadPool.h"
#include "Log/Log.h"

using namespace config;

int main(int argc, char *argv[]) {

	// 初始化 线程池
	ThreadPool<HttpConn>* thread_pool = new ThreadPool<HttpConn> (THREAD_NUM, MAX_TASK_NUM);

	// 初始化 数据库连接池
	SqlConnPool* conn_pool = SqlConnPool::get_instance();
	conn_pool->init(URL, _MYSQL_PORT, MYSQL_NAME, MYSQL_PASSWD, MYSQL_DBNAME, MAX_CONN_NUM);

	// 初始化日志
    Log::get_instance()->init("./ServerLog", !LOG_OPEN, 2000, 800000, 0);

	// 事件反应堆
	Reactor reactor(SERVER_PORT, thread_pool, conn_pool);
	reactor.event_listen();
	reactor.event_loop();
}
