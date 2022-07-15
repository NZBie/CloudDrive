
#include <jsoncpp/json/json.h>

#include "config.h"
#include "Reactor/Reactor.h"
#include "MySQL_CGI/SqlConnPool.h"
#include "Http/HttpConn.h"
#include "ThreadPool/ThreadPool.h"
#include "log/log.h"

using namespace config;

int main(int argc, char *argv[]) {

	// 初始化 线程池
	ThreadPool<HttpConn>* thread_pool = new ThreadPool<HttpConn> (thread_num, max_task_num);

	// 初始化 数据库连接池
	SqlConnPool* conn_pool = SqlConnPool::get_instance();
	conn_pool->init("localhost", config::mysql_port, mysql_name, mysql_passwd, mysql_dbname, max_conn_num);

	// 初始化日志
    if(log_open == true) {
        // if (1 == m_log_write)
            Log::get_instance()->init("./ServerLog", false, 2000, 800000, 800);
        // else
        //     Log::get_instance()->init("./ServerLog", m_close_log, 2000, 800000, 0);
    }

	// 事件反应堆
	Reactor reactor(server_port, thread_pool, conn_pool);
	reactor.event_listen();
	reactor.event_loop();
}
