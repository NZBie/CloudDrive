#include "config.h"
#include "Reactor.h"

using namespace config;

int main() {
	
	Reactor reactor;
	reactor.init(thread_num, max_task_num, url, port, mysql_name, mysql_passwd, mysql_dbname);

	reactor.event_listen();
	reactor.event_loop();
}
