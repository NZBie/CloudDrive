#include "Reactor.h"

Reactor::~Reactor() {
	
	close(_epoll_fd);
	close(_listen_fd);
	// delete _conn_pool;
	delete _thread_pool;
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
}

// 初始化定时器
void Reactor::init_timer_pipe() {

	int ret = socketpair(PF_UNIX, SOCK_STREAM, 0, _pipe_fd);
	assert(ret != -1);
	
}

// 主事件循环体
void Reactor::event_loop() {

	bool server_close = false;
	// bool timeout = false;

	while(server_close == false) {

		// printf("%s\n", "epoll start wait...");

		// epoll等待事件
		int event_num = epoll_wait(_epoll_fd, _events, MAX_EVENT_NUM, -1);
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
				deal_client_connect();
			}

			// 接收到 关闭服务器的信号
			else if(_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				
			}

			// 接收到 定时器信号
			// else if(1) {

			// }

			// 接收到 客户端发来的数据
			else if(_events[i].events & EPOLLIN) {
				deal_client_read(sock_fd);
			}
			
			// 接收到 服务器发来的写事件，写完发送回客户端
			else if(_events[i].events & EPOLLOUT) {
				deal_client_write(sock_fd);
			}
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
		if(HttpConn::_user_count >= MAX_FD_NUM) {
			LOG_INFO("Server busy");
			return false;
		}

		_users[client_fd].init(client_fd, client_address);
		LOG_INFO("Accept successful ip: %s", inet_ntoa(client_address.sin_addr));
	}

	return true;
}

// 处理 来自客户端 的读事件
void Reactor::deal_client_read(int client_fd) {

	// 检测到读事件，放入请求队列
	_thread_pool->append(_users + client_fd, 0);
	// printf("count: %d\n", HttpConn::count);
}

// 处理 来自服务器 的写事件
void Reactor::deal_client_write(int client_fd) {

	// 检测到写事件，放入请求队列
	_thread_pool->append(_users + client_fd, 1);
}
