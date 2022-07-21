all: server

server: main.cpp Reactor/Reactor.cpp ThreadPool/ThreadPool.h Http/HttpConn.cpp MySQL_CGI/SqlConnPool.cpp Timer/ListTimer.cpp BLL/BLL.cpp log/log.cpp Locker/Locker.h config.h Http/FormData/FormDataParser.cpp
	g++ -g -o server $^ -lpthread $$(mysql_config --cflags --libs) -L./usr/lib/x86_64-linux-gnu -ljsoncpp

clean:
	rm ./server

test: test.cpp
	g++ -g -o test $^ 
