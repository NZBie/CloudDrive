#include "HttpConn.h"

int HttpConn::_user_count = 0;
int HttpConn::_epoll_fd = -1;

const HttpConn::HTTP_MESSAGE HttpConn::http_messages[10] = {
	{200, "OK", ""},
	{400, "Bad Request", "Your request has bad syntax or is inherently impossible to staisfy.\n"},
	{403, "Forbidden", "You do not have permission to get file form this server.\n"},
	{404, "Not Found", "The requested file was not found on this server.\n"},
	{500, "Internal Error", "There was an unusual problem serving the request file.\n"},
};

HttpConn::HttpConn() {

}

HttpConn::~HttpConn() {
	
}

// ?
void HttpConn::event_modify(int epoll_fd, int client_fd, int e) {

	epoll_event event;
	event.data.fd = client_fd;

	event.events = e | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
}

// 外部初始化
void HttpConn::init(int client_fd, sockaddr_in& address, char* root, string user, string passwd, string dbname) {

	_client_fd = client_fd;
	_address = address;
	_root = root;

	// 在内核事件表注册读事件
	epoll_event event;
	event.data.fd = _client_fd;
	event.events = EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _client_fd, &event);
	
	// 设置文件描述符为非阻塞
    int old_option = fcntl(_client_fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(_client_fd, F_SETFL, new_option);

	_user_count++;

	strcpy(sql_user, user.c_str());
	strcpy(sql_passwd, passwd.c_str());
	strcpy(sql_dbname, dbname.c_str());

	init();
}

// 内部初始化
void HttpConn::init() {
	
	_check_status = REQUEST_LINE;
	_linger = false;

	_read_len = 0;
	_write_len = 0;
	_file_address = nullptr;

	memset(_read_buf, 0, READ_BUFFER_SIZE);
	memset(_write_buf, 0, WRITE_BUFFER_SIZE);
	memset(_real_file, 0, FILENAME_LEN);
	
}

// 断开和客户端的连接
void HttpConn::close_connection() {

	if(_client_fd != -1) {

		LOG_INFO("close %d\n", _client_fd);

		// 从内核事件表删除_client_fd
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _client_fd, 0);
		close(_client_fd);

		_client_fd = -1;
		_user_count--;
	}
}

// http从读到写的过程
void HttpConn::complete_process() {

	read_request();

	// 解析报文
	HTTP_CODE read_ret = parse_message();

	// 无响应报文
	if(read_ret == NO_REQUEST) {
		event_modify(_epoll_fd, _client_fd, EPOLLIN);
		return;
	}

	// 将报文写入缓存中
	bool write_ret = add_message(read_ret);

	// 修改epoll事件类型触发响应
	if(write_ret == false) {
		close_connection();
		return;
	}
	
	// event_modify(_epoll_fd, _client_fd, EPOLLOUT);
	write_response();
}

// 将请求 从socket 读取至 缓存
bool HttpConn::read_request() {

	if(_read_len >= READ_BUFFER_SIZE) {
		return false;
	}

	// ET边缘触发，循环读取数据
	int tmp = 0;
	while(true) {
		tmp = recv(_client_fd, _read_buf + _read_len, READ_BUFFER_SIZE - _read_len, 0);
		if(tmp <= 0) {
			if(tmp == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
			else return false;
		}

		_read_len += tmp;
	}
	return true;
}

// 读取并解析 缓存中 客户端发送的请求
HttpConn::HTTP_CODE HttpConn::parse_message() {
	
	printf("接收报文：\n%s\n\n", _read_buf);

	LINE_STATE line_state = LINE_OK;
	HTTP_CODE ret = NO_REQUEST;
	char* text = nullptr;

	// 逐行解析
	while(true) {

		// ?
		if(!(_check_status == REQUEST_CONTENT && line_state == LINE_OK)) {
			if((line_state = get_line()) != LINE_OK) break;
		}

		// 从_read_buf中读取一行
		// line_state = get_line();
		if(line_state != LINE_OK) break;
		text = _read_buf + _line_begin;
		_line_begin = _line_end;

		// 状态机判断与转移
		switch(_check_status) {
		
			case REQUEST_LINE: {
				ret = parse_request_line(text);
				if(ret == BAD_REQUEST) {
					return BAD_REQUEST;}
				break;
			}

			case REQUEST_HEADER: {
				ret = parse_request_headers(text);
				if(ret == BAD_REQUEST) return BAD_REQUEST;
				if(ret == GET_REQUEST) return do_request();
				break;
			}

			case REQUEST_CONTENT: {
				ret = parse_request_content(text);
				if(ret == GET_REQUEST) return do_request();
				line_state = LINE_OPEN;
				break;
			}
			
			default: {
				return INTERNAL_ERROR;
			}
		}
	}
	return NO_REQUEST;
}

// 从缓冲区中拿出一行
HttpConn::LINE_STATE HttpConn::get_line() {
	
	char tmp;
	for(; _line_end < _read_len; ++_line_end) {
		tmp = _read_buf[_line_end];

		// 检测 换行符 作为行间隔
		if(tmp == '\n') {
			if(_line_end > 1 && _read_buf[_line_end - 1] == '\r') {
				_read_buf[_line_end - 1] = '\0';
				_read_buf[_line_end++] = '\0';
				return LINE_OK;
			}
			else return LINE_BAD;
		}
	}
	return LINE_OPEN;
}

// 解析请求行 & 请求头 & 请求数据
HttpConn::HTTP_CODE HttpConn::parse_request_line(char* text) {

	char* tmp = nullptr;

	// 解析请求方法
	tmp = strpbrk(text, " \t");
	if(tmp == nullptr) return BAD_REQUEST;
	*tmp++ = '\0';
	char* method = text;

	if(strcmp(method, "GET") == 0) _method = GET;
	else if(strcmp(method, "POST") == 0) _method = POST;
	else return BAD_REQUEST;

	// 解析version
	tmp += strspn(tmp, " \t");
	_version = strpbrk(tmp, " \t");
	if(_version == nullptr) return BAD_REQUEST;
	*_version++ = '\0';
	_version += strspn(_version, " \t");

	if(strcmp(_version, "HTTP/1.1") != 0) return BAD_REQUEST;

	// 解析url
	if(strncmp(tmp, "http://", 7) == 0) {
		tmp += 7;
		tmp = strchr(tmp, '/');
	}
	if(strncmp(tmp, "https://", 8) == 0) {
		tmp += 8;
		tmp = strchr(tmp, '/');
	}

	if(!tmp || tmp[0] != '/') return BAD_REQUEST;
	if(strlen(tmp) == 1) strcat(tmp, "index.html");
	_url = tmp;

	// 状态变化
	_check_status = REQUEST_HEADER;
	return NO_REQUEST;
}
HttpConn::HTTP_CODE HttpConn::parse_request_headers(char* text) {

    if (text[0] == '\0')
    {
        if (_content_length != 0)
        {
            _check_status = REQUEST_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0)
    {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0)
        {
            _linger = true;
        }
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0)
    {
        text += 15;
        text += strspn(text, " \t");
        _content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        _host = text;
    }
    else
    {
        LOG_INFO("oop!unknow header: %s", text);
    }
    return NO_REQUEST;
}
HttpConn::HTTP_CODE HttpConn::parse_request_content(char* text) {

    if (_read_len >= _line_end + _content_length)
    {
        text[_content_length] = '\0';
        //POST请求中最后为输入的用户名和密码
        _string = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

// 将报文写入缓存中
bool HttpConn::add_message(HTTP_CODE ret) {

	// 添加 响应行
	add_response_line(http_messages[ret].code, http_messages[ret].title);

	// 文件请求
	if(ret == FILE_REQUEST && _file_stat.st_size != 0) {
		add_response_headers(_file_stat.st_size);
		return true;
	}

	// 非文件请求 & 请求失败
	else {

		// 添加预设的 响应头 和 响应数据
		add_response_headers(strlen(http_messages[ret].form));
		add_response_content(http_messages[ret].form);
	}
	return true;
}

// 将单行报文写入缓存中
bool HttpConn::add_single_line(const char* format, ...) {

	if(_write_len >= WRITE_BUFFER_SIZE) return false;

	// 利用可变参数，将format解析后，放入_write_buf缓存中
	va_list arg_list;
	va_start(arg_list, format);
	int len = vsnprintf(_write_buf + _write_len, WRITE_BUFFER_SIZE - _write_len - 1, format, arg_list);
	va_end(arg_list);

	// 判断缓存是否超出
	if(len < WRITE_BUFFER_SIZE - _write_len - 1) {
		_write_len += len;
		return true;
	}
	else {
		return false;
	}
}

// 添加响应行 & 响应头 & 响应数据
inline bool HttpConn::add_response_line(int status, const char* title) {
	return add_single_line("%s %d %s\r\n", "HTTP/1.1", status, title);
}
inline bool HttpConn::add_response_headers(int content_len) {
	bool res = true;
	res &= add_single_line("Content-Length:%d\r\n", content_len);
	res &= add_single_line("Content-Type:%s\r\n", "text/html");
	res &= add_single_line("Connection:%s\r\n", (_linger ? "keep-alive" : "close"));
	res &= add_single_line("\r\n");
	return res;
}
inline bool HttpConn::add_response_content(const char* content) {
	return add_single_line("%s", content);
}

// 对解析后的报文进行处理
HttpConn::HTTP_CODE HttpConn::do_request() {

	strcpy(_real_file, _root);
	int len = strlen(_root);

	// 处理登陆注册
	if(_method == POST) {

	}

	// 直接访问_url对应的html文件
	else strncpy(_real_file + len, _url, FILENAME_LEN - len - 1);

	// ?
	if(stat(_real_file, &_file_stat) < 0) return NO_RESOURCE;
    if(!(_file_stat.st_mode & S_IROTH)) return FORBIDDEN_REQUEST;
    if(S_ISDIR(_file_stat.st_mode)) return BAD_REQUEST;

	// 硬盘到内存的地址映射
    int file_fd = open(_real_file, O_RDONLY);
    _file_address = (char *)mmap(0, _file_stat.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    close(file_fd);
    return FILE_REQUEST;
}

// 将响应 从缓存 写回到 socket
bool HttpConn::write_response() {

	if(_write_len == 0) {
		event_modify(_epoll_fd, _client_fd, EPOLLIN);
		init();
		return true;
	}

	// 发送报文头和报文主体
	bool ret = true;
	ret &= write_single_buffer(_write_buf, _write_len);
	if(_file_address) {
		ret &= write_single_buffer(_file_address, _file_stat.st_size);
		munmap(_file_address, _file_stat.st_size);
		_file_address = nullptr;
	}
	if(ret == true) init();
	return ret;
}
bool HttpConn::write_single_buffer(char* buf, int len) {

	// ET边缘触发，循环发送数据
	int bytes_sent = 0;	// 已经发送回socket的字符数量
	int tmp = 0;		// 本次发送的字符数
	while(true) {
		tmp = send(_client_fd, buf + bytes_sent, len - bytes_sent, 0);

		// 发送异常
		if(tmp < 0) {

			// 缓存池繁忙，重试
			if(errno == EAGAIN) {
				// event_modify(_epoll_fd, _client_fd, EPOLLOUT);
				usleep(1000);
				continue;
			}

			// 出错
			else return false;
		}

		bytes_sent += tmp;
		if(bytes_sent >= len) {
			return true;
		}
	}
}
