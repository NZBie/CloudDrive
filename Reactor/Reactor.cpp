#include "Reactor.h"

Reactor::Reactor(short port, ThreadPool<HttpConn>* thread_pool, SqlConnPool* conn_pool):
	_port(port), _thread_pool(thread_pool), _conn_pool(conn_pool) {
	_users = new HttpConn[config::MAX_FD_NUM];
	_users_timer = new Client_data[config::MAX_FD_NUM];
}; 

Reactor::~Reactor() {
	
	close(_epoll_fd);
	close(_listen_fd);
	close(_pipe_fd[0]);
	close(_pipe_fd[1]);
	delete _thread_pool;
	delete[] _users;
	delete[] _users_timer;
}

// 在内核事件表中注册事件
void Reactor::add_event(int epoll_fd, int sock_fd, int events) {

	epoll_event event;
	event.data.fd = sock_fd;
	event.events = events;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event);
}

// 设置文件描述符为非阻塞
void Reactor::set_nonblocking(int sock_fd) {

    int old_option = fcntl(sock_fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(sock_fd, F_SETFL, new_option);
}

// 开启监听socket
void Reactor::event_listen() {
	
	// 创建listen的socket
	_listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	assert(_listen_fd >= 0);

	// 设置关闭连接方式
	linger lin = {0, 0};
	setsockopt(_listen_fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));
	
	// 设置socket地址参数
	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(_port);

	// 防止重启服务器后bind失败
	int flag = 1;
    setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	// 绑定地址 & 开启监听
	int ret;
	ret = bind(_listen_fd, (sockaddr*)&address, sizeof(address));
	assert(ret >= 0);
	ret = listen(_listen_fd, 5);
	assert(ret >= 0);

	// 创建epoll
	_epoll_fd = epoll_create(5);
	assert(_epoll_fd != -1);

	// 在内核事件表注册读事件
	add_event(_epoll_fd, _listen_fd, EPOLLIN | EPOLLRDHUP | EPOLLET);

	// 设置文件描述符为非阻塞
    set_nonblocking(_listen_fd);

	// 设置HttpConn的_epoll_fd;
	HttpConn::_epoll_fd = _epoll_fd;

	init_utils_timer();
}

// 主事件循环体
void Reactor::event_loop() {

	bool server_close = false;
	bool timeout = false;

	while(server_close == false) {
		// printf("%s\n", "epoll start wait...");

		// epoll等待事件
		int event_num = epoll_wait(_epoll_fd, _events, config::MAX_EVENT_NUM, -1);
		if(event_num < 0 && errno != EINTR) {
			LOG_ERROR("epoll failure!");
			break;
		}

		// LOG_DEBUG("there are %d events.\n", event_num);
		for(int i = 0; i < event_num; ++i) {

			int sock_fd = _events[i].data.fd;
			// LOG_DEBUG(" -> %d %d\n", sock_fd, _events[i].events);

			// 接收到 新的客户端连接
			if(sock_fd == _listen_fd) {
				if(deal_client_connect() == false) continue;
			}

			// 接收到 信号消息
			else if((sock_fd == _pipe_fd[0]) && (_events[i].events & EPOLLIN)) {
				if(deal_with_signal(timeout, server_close) == false) continue;
			}

			// 接收到 关闭定时器的信号
			else if(_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				// 移除指定定时器
				UtilTimer* timer = _users_timer[sock_fd].timer;
				delete_timer(timer, sock_fd);
			}

			// 接收到 客户端发来的数据
			else if(_events[i].events & EPOLLIN) {
				deal_client_read(sock_fd);
			}
			
			// 接收到 服务器发来的写事件，写完发送回客户端
			else if(_events[i].events & EPOLLOUT) {
				deal_client_write(sock_fd);
			}
		}

		if(timeout) {
			_utils.timer_handler();
			LOG_INFO("timer tick.");
			timeout = false;
		}
	}
}

// 处理 新的客户端 连接请求
bool Reactor::deal_client_connect() {

	sockaddr_in client_address;
	socklen_t address_len = sizeof(client_address);

	// ET边缘触发，一次性循环处理
	while(true) {

		int client_fd = accept(_listen_fd, (sockaddr *)&client_address, &address_len);
		// 连接失败
		if(client_fd  < 0) {
			break;
		}
		// 服务器连接客户端达到上限
		if(HttpConn::_user_count >= config::MAX_FD_NUM) {
			LOG_INFO("Server busy");
			return false;
		}

		_users[client_fd].init(client_fd, client_address);
		new_timer(client_fd, client_address);
		LOG_INFO("Accept successful ip: %s", inet_ntoa(client_address.sin_addr));
	}

	return true;
}

// 处理 来自客户端 的读事件
void Reactor::deal_client_read(int client_fd) {

	UtilTimer* timer = _users_timer[client_fd].timer;
	if(timer) adjust_timer(timer);

	// 检测到读事件，放入请求队列
	_thread_pool->append(_users + client_fd, 0);

	// 接收报文失败时，关闭连接
	while(true) {
		if(_users[client_fd].improve) {
			if(_users[client_fd].timer_flag) {
				delete_timer(timer, client_fd);
				_users[client_fd].timer_flag = false;
			}
			_users[client_fd].improve = false;
			break;
		}
	}
}

// 处理 来自服务器 的写事件
void Reactor::deal_client_write(int client_fd) {

	UtilTimer* timer = _users_timer[client_fd].timer;
	if(timer) adjust_timer(timer);

	// 检测到写事件，放入请求队列
	_thread_pool->append(_users + client_fd, 1);

	// 响应报文失败 或 keep-alive为空时，关闭连接
	while(true) {
		if(_users[client_fd].improve) {
			if(_users[client_fd].timer_flag) {
				delete_timer(timer, client_fd);
				_users[client_fd].timer_flag = false;
			}
			_users[client_fd].improve = false;
			break;
		}
	}
}

// 处理 来自内核的信号
bool Reactor::deal_with_signal(bool& timeout, bool& stop_server) {

	// 通过管道接收信号
	char signals[1024];
	int ret = recv(_pipe_fd[0], signals, sizeof(signals), 0);
	if(ret <= 0) return false;

	for(int i=0;i<ret;i++) {
		switch(signals[i]) {
			case SIGALRM: {
				timeout = true;
				break;
			}
			case SIGTERM: {
				stop_server = true;
				break;
			}
			default: {
				break;
			}
		}
	}
	return true;
}

// 初始化定时器
void Reactor::init_utils_timer() {

	_utils.init(config::TIME_SLOT);

	// 创建通信管道
	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, _pipe_fd);
	assert(ret != -1);

	// 注册事件
	add_event(_epoll_fd, _pipe_fd[0], EPOLLIN | EPOLLRDHUP);
	set_nonblocking(_pipe_fd[0]);
	set_nonblocking(_pipe_fd[1]);

	// 添加信号响应函数 
    _utils.add_signal(SIGPIPE, SIG_IGN, true);
    _utils.add_signal(SIGALRM, _utils.signal_handler, false);
    _utils.add_signal(SIGTERM, _utils.signal_handler, false);

	alarm(config::TIME_SLOT);

	Utils::_pipe_fd = _pipe_fd;
	Utils::_epoll_fd = _epoll_fd;
}

// 新建定时器并插入链表
void Reactor::new_timer(int client_fd, sockaddr_in client_address) {

	// 新建定时器
	UtilTimer* timer = new UtilTimer;
	timer->user_data = _users_timer + client_fd;
	timer->cb_func = cb_func;
	timer->expire = time(nullptr) + 3 * config::TIME_SLOT;

	// 客户端信息
	// Client_data& client_data = _users_timer[client_fd];
	_users_timer[client_fd].address = client_address;
	_users_timer[client_fd].sock_fd = client_fd;
	_users_timer[client_fd].timer = timer;

	// 将新定时器加入有序链表
	_utils._timer_list.add_timer(timer);
}

// 删除定时器
void Reactor::delete_timer(UtilTimer* timer, int client_fd) {
	
	if(timer) {
		timer->cb_func(&_users_timer[client_fd]);
		_utils._timer_list.delete_timer(timer);
	}
	LOG_INFO("close fd %d.", client_fd);
}

// 活跃连接，重新调整定时器
void Reactor::adjust_timer(UtilTimer* timer) {

    timer->expire = time(nullptr) + 3 * config::TIME_SLOT;
    _utils._timer_list.adjust_timer(timer);
    // LOG_DEBUG("adjust timer once");
}

// 定时处理函数
void cb_func(Client_data* user_data) {

	epoll_ctl(Utils::_epoll_fd, EPOLL_CTL_DEL, user_data->sock_fd, 0);
	assert(user_data);
	close(user_data->sock_fd);
	HttpConn::_user_count--;
}
