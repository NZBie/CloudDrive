#include "reactor.h"

/**
 * @brief 开启监听socket
 * 
 */
void reactor::event_listen() {
	
	// 创建listen的socket
	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	assert(_listen_fd >= 0);
	
	// 设置socket地址参数
	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(_port);

	// unkown
	int flag = 1;
    setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	// 绑定地址 & 开启监听
	int ret;
	ret = bind(_listen_fd, (sockaddr*)&address, sizeof(address));
	assert(ret >= 0);
	ret = listen(_listen_fd, SOMAXCONN);
	assert(ret >= 0);

	// 创建epoll
	_epoll_fd = epoll_create(5);
	assert(_epoll_fd != -1);

	
}

/**
 * @brief 主事件循环体
 * 
 */
void reactor::event_loop() {

	bool server_close = false;

	while(!server_close) {

		// epoll等待事件
		int event_num = epoll_wait(_epoll_fd, _events, MAX_EVENT_NUM, -1);
		if(event_num < 0 && true) {
			printf("epoll failure!\n");
			break;
		}

		for(int i=0;i<event_num;i++) {

			int sock_fd = _events[i].data.fd;

			// 接收到 新的客户端连接
			if(sock_fd == _listen_fd) {
				if(deal_client_connect() == false) continue;
			}

			// 接收到 关闭服务器信号
			else if(_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				
			}

			// 接收到 信号
			else if(1) {

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
	}
}

/**
 * @brief 处理新的客户端连接请求
 * 
 * @return true 
 * @return false 
 */
bool reactor::deal_client_connect() {

	sockaddr_in client_address;
	socklen_t client_address_len = sizeof(client_address);
	
	int conn_fd = accept(_listen_fd, (sockaddr *)&client_address, &client_address_len);
	// 连接失败
	if(conn_fd  < 0) {
		printf("accept error");
		return false;
	}
	// 服务器连接客户端达到上限
	if(1) {
		return false;
	}
	return true;
}

/**
 * @brief 
 * 
 * @param client_fd 
 */
void reactor::deal_client_read(int client_fd) {

}

/**
 * @brief 
 * 
 * @param client_fd 
 */
void reactor::deal_client_write(int client_fd) {

	// if(user)
}
