all: server

server: main.cpp Reactor.cpp ThreadPool/ThreadPool.h Http/HttpConn.cpp Mysql_CGI/SqlConnPool.cpp log/log.cpp log/block_queue.h Locker/Locker.h config.h
	g++ -g -o server $^ -lpthread $$(mysql_config --cflags --libs)

clean:
	rm ./server
