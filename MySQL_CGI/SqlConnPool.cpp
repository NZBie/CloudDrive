#include "SqlConnPool.h"

SqlConnPool::SqlConnPool():
_conn_num(0), _conn_free(0) {

}

SqlConnPool::~SqlConnPool() {
	destroy();
}

SqlConnPool* SqlConnPool::get_instance() {
	static SqlConnPool connPool;
	return &connPool;
}

void SqlConnPool::init(string url, short port, string user, string passwd, string dbname, int max_conn_num) {

	_url = url;
	_port = port;
	_user = user;
	_passwd = passwd;
	_dbname = dbname;
	// printf("%s, %d, %s, %s, %s\n", url.c_str(), port, user.c_str(), passwd.c_str(), dbname.c_str());
	
	for(int i=0;i<max_conn_num;++i) {

		MYSQL* conn = nullptr;

		// 数据库初始化
		conn = mysql_init(conn);
		if(conn == nullptr) {
			printf("MySQL init error");
			exit(1);
		}

		// 连接数据库
		conn = mysql_real_connect(conn, _url.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, NULL, 0);
		if(conn == nullptr) {
			printf("MySQL connect error");
			exit(1);
		}

		// 加入连接池
		_conn_list.push_back(conn);
		_conn_free++;
	}

	// 统计连接池的容量 & 信号量
	_conn_semaphore = Sem(_conn_free);
	_max_conn_num = _conn_free;
}

void SqlConnPool::destroy() {

	_conn_locker.lock();

	// 存在未释放的连接
	if(_conn_num > 0) return;

	// 关闭连接池中所有连接
	while(_conn_list.size() > 0) {
		MYSQL* conn = _conn_list.front();
		_conn_list.pop_front();
		mysql_close(conn);
	}

	// 解锁 & 可用连接置零
	_conn_locker.unlock();
	_conn_free = 0;
	_max_conn_num = 0;
}

MYSQL* SqlConnPool::get_connection() {

	// 等待 其他线程 操作_conn_list 完成
	_conn_semaphore.wait();
	_conn_locker.lock();
	if(_conn_list.empty()) {
		_conn_locker.unlock();
		return nullptr;
	}

	// 从连接池中取出连接信息
	MYSQL* conn = _conn_list.front();
	_conn_list.pop_front();

	// 更新连接池信息
	_conn_free--;
	_conn_num++;

	// 解锁
	_conn_locker.unlock();
	return conn;
}

bool SqlConnPool::release_connection(MYSQL* conn) {

	if(conn == nullptr) return false;

	_conn_locker.lock();

	// 放回连接池
	_conn_list.push_back(conn);
	_conn_free++;
	_conn_num--;

	_conn_locker.unlock();
	_conn_semaphore.post();
	return true;
}

int SqlConnPool::get_free_conn_num() {
	return _conn_free;
}

