#include "config.h"
#include "Reactor.h"

using namespace config;

int main(int argc, char *argv[]) {
	
	Reactor reactor;
	reactor.init(
		thread_num, max_task_num, 
		port, 
		mysql_name, mysql_passwd, mysql_dbname, 
		max_conn_num
	);
	reactor.event_listen();
	reactor.event_loop();
}
