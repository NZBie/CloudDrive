#ifndef REACTOR_H
#define REACTOR_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <cstdio>
#include <string.h>
#include <assert.h>

#include "ThreadPool/ThreadPool.h"

const int MAX_EVENT_NUM = 100;

class reactor {
public:
	reactor();
	~reactor();	

	void event_listen();
	void event_loop();

	bool deal_client_connect();
	void deal_client_read(int client_fd);
	void deal_client_write(int client_fd);

private:
	
	int _listen_fd;
	int _epoll_fd;
	int _port;

	epoll_event _events[MAX_EVENT_NUM];
};

#endif