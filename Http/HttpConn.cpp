#include "HttpConn.h"

int HttpConn::_user_count = 0;
int HttpConn::_epoll_fd = -1;

const HttpConn::HTTP_MESSAGE HttpConn::http_messages[10] = {
	{},
	{},
	{200, "OK", ""},
	{400, "Bad Request", "Your request has bad syntax or is inherently impossible to satisfy.\n"},
	{403, "Forbidden", "You do not have permission to get file form this server.\n"},
	{404, "Not Found", "The requested file was not found on this server.\n"},
	{500, "Internal Error", "There was an unusual problem serving the request file.\n"},
	{}
};

HttpConn::HttpConn() {
	map_bll_init();
	_read_buf = new char[config::READ_BUFFER_SIZE];
	_write_buf = new char[config::WRITE_BUFFER_SIZE];
};

HttpConn::~HttpConn() {
	delete[] _read_buf;
	delete[] _write_buf;
	close_connection();
};

// 修改client_fd在内核事件表的监听事件
void HttpConn::event_modify(int epoll_fd, int client_fd, int e) {

	epoll_event event;
	event.data.fd = client_fd;

	event.events = e | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
}

// 外部初始化
void HttpConn::init(int client_fd, sockaddr_in& address) {

	// root文件夹路径
	char server_path[128];
	getcwd(server_path, sizeof(server_path));
	char root[32] = "/cloud_drive/dist";
	_root = new char[strlen(server_path) + strlen(root) + 1];
	strcpy(_root, server_path);
	strcat(_root, root);

	_client_fd = client_fd;
	_address = address;

	// 在内核事件表注册读事件
	epoll_event event;
	event.data.fd = _client_fd;
	event.events = EPOLLIN | EPOLLRDHUP | EPOLLET | EPOLLONESHOT;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _client_fd, &event);
	
	// 设置文件描述符为非阻塞
    int old_option = fcntl(_client_fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(_client_fd, F_SETFL, new_option);

	init();
}

// 内部初始化
void HttpConn::init() {

	// IO
	_read_len = 0;
	_write_len = 0;

	// request
	_line_end = 0;
	_line_begin = 0;
	_check_status = REQUEST_LINE;
	_url = nullptr;
	_content_length = 0;

	// response
	_file_address = nullptr;
	memset(_real_file, 0, config::FILENAME_LEN);
	memset(_content_type, 0, sizeof(_content_type));

	// 
	_rqs_params = Value();
	_response_json = Value();

	improve = false;
	_linger = false;
}

// 断开和客户端的连接
void HttpConn::close_connection() {

	if(_client_fd != -1) {

		LOG_INFO("Close Connection: %s", _address);
		
		// 从内核事件表删除_client_fd
		epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _client_fd, 0);
		close(_client_fd);

		_client_fd = -1;
		_user_count--;
	}

	delete[] _read_buf;
	_read_buf = nullptr;
}

// 解析 & 处理 & 反馈 的过程
void HttpConn::complete_process() {

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
	
	event_modify(_epoll_fd, _client_fd, EPOLLOUT);
}

// 将请求 从socket 读取至 缓存
bool HttpConn::read_request() {

	if(_read_len >= config::READ_BUFFER_SIZE) {
		return false;
	}

	// ET边缘触发，循环读取数据
	int tmp = 0;
	while(true) {
		tmp = recv(_client_fd, _read_buf + _read_len, config::READ_BUFFER_SIZE - _read_len, 0);
		if(tmp <= 0) {
			if(tmp == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
			else return false;
		}
		_read_len += tmp;
	}
	// LOG_INFO("Request message received: \n%s", _read_buf);
	return true;
}

// 读取并解析 缓存中 客户端发送的请求
HttpConn::HTTP_CODE HttpConn::parse_message() {

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

	LOG_INFO("Request line received: %s", _read_buf);

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
	*(_version++) = '\0';
	_version += strspn(_version, " \t");

	if(strcmp(_version, "HTTP/1.1") != 0 && strcmp(_version, "HTTP/1.0") != 0) return BAD_REQUEST;

	// 解析url
	if(strncmp(tmp, "http://", 7) == 0) {
		tmp += 7;
		tmp = strchr(tmp, '/');
	}
	if(strncmp(tmp, "https://", 8) == 0) {
		tmp += 8;
		tmp = strchr(tmp, '/');
	}

	if(tmp == nullptr || tmp[0] != '/') return BAD_REQUEST;
	if(strlen(tmp) == 1) strcat(tmp, "index.html");
	_url = tmp;

	// 解析url中的参数
	tmp = strpbrk(tmp, "?");
	if(tmp != nullptr) {

		*(tmp++) = '\0';
		char* param_key;
		char* param_val;
		while(tmp != nullptr) {
			param_key = tmp;
			tmp = strpbrk(tmp, "=");
			if(tmp == nullptr) break;
			*(tmp++) = '\0';
			param_val = tmp;
			tmp = strpbrk(tmp, "&");
			if(tmp != nullptr) *(tmp++) = '\0';
			
			_rqs_params[param_key] = param_val;
		}
	}

	// 状态变化
	_check_status = REQUEST_HEADER;
	return NO_REQUEST;
}
HttpConn::HTTP_CODE HttpConn::parse_request_headers(char* text) {

    if (text[0] == '\0')
    {
        if (_content_length > 0)
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
	else if(strncmp(text, "Content-Type:", 13) == 0) {
        text += 12;
        text += strspn(text, " :\t");
        // _content_type = text;
		text = strpbrk(text, " ;\t");
		*(text++) = '\0';
		text += strspn(text, " ;\t");
		if(strncmp(text, "boundary=", 9) == 0) {
			text += 9;
			text += strspn(text, " \t");
			strcpy(_boundary, "--");
			strcpy(_boundary+2, text);
		}
	}
    else if (strncasecmp(text, "Host:", 5) == 0)
    {
        text += 5;
        text += strspn(text, " \t");
        _host = text;
    }
    else
    {
        // LOG_INFO("oop!unknow header: %s", text);
    }
    return NO_REQUEST;
}
HttpConn::HTTP_CODE HttpConn::parse_request_content(char* text) {

printf("(%d,%d,%d)\n", _read_len, _line_end, _content_length);

    if (_read_len >= _line_end + _content_length) {

		// 解析请求体
        text[_content_length] = '\0';
		FormDataParser fmdParser(text, _content_length, _boundary, 0);
		std::vector<FormItem>* data = fmdParser.parse();
		_rqs_params["fName"] = (*data)[0].getFileName();
		_rqs_params["fSize"] = (*data)[0].getSize();
		const void* xxx = ((*data)[0].getContent());
		const void* xxxx = &xxx;
		const long long* xx = (long long*)xxxx;
		_rqs_params["fData"] = (int64_t)*xx;

        return GET_REQUEST;
    }
    return NO_REQUEST;
}

// 对解析后的报文进行处理
HttpConn::HTTP_CODE HttpConn::do_request() {

	strcpy(_real_file, _root);
	int len = strlen(_root);

	printf("%s\n", _url);
	string url(_url);

	// 数据请求
	if(m_bll.find(url) != m_bll.end()) {

		if(m_bll[url](_rqs_params, _response_json) == false) {
			return BAD_REQUEST;
		}

		strcpy(_content_type, "application/json");
		_response_type = false;
	}

	// 直接访问_url对应的文件
	else {
		strncpy(_real_file + len, _url, config::FILENAME_LEN - len - 1);

		// 从_file_stat中获取文件信息
		if(stat(_real_file, &_file_stat) < 0) return NO_RESOURCE;
		if(!(_file_stat.st_mode & S_IROTH)) return FORBIDDEN_REQUEST;
		if(S_ISDIR(_file_stat.st_mode)) return BAD_REQUEST;

		// 硬盘到内存的地址映射
		int file_fd = open(_real_file, O_RDONLY);
		_file_address = (char *)mmap(0, _file_stat.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
		close(file_fd);

		_response_type = true;
	}

    return OK_REQUEST;
}

// 将报文写入缓存中
bool HttpConn::add_message(HTTP_CODE ret) {

	// 添加 响应行
	add_response_line(http_messages[ret].code, http_messages[ret].title);

	// 请求
	if(ret == OK_REQUEST) {

		if(_response_type && _file_stat.st_size > 0) {
			add_response_headers(_file_stat.st_size);
		}
		
		// 响应数据
		else {
			const string content = fwriter.write(_response_json);
			add_response_headers(content.size());
			add_response_content((content).c_str());
		}
	}

	// 请求失败
	else {

		// 添加预设的 响应头 和 响应数据
		add_response_headers(strlen(http_messages[ret].form));
		add_response_content(http_messages[ret].form);
	}
	return true;
}

// 添加响应行 & 响应头 & 响应数据
inline bool HttpConn::add_response_line(int status, const char* title) {
	return add_single_line("%s %d %s\r\n", "HTTP/1.1", status, title);
}
inline bool HttpConn::add_response_headers(int content_len) {
	bool res = true;
	res &= add_single_line("Content-Length:%d\r\n", content_len);
	res &= add_single_line("Content-Type:%s\r\n", _content_type);
	res &= add_single_line("Connection:%s\r\n", (_linger ? "keep-alive" : "close"));
	res &= add_single_line("Access-Control-Allow-Origin:*\r\n");
	res &= add_single_line("\r\n");
	return res;
}
inline bool HttpConn::add_response_content(const char* content) {
	return add_single_line("%s", content);
}

// 将单行报文写入缓存中
bool HttpConn::add_single_line(const char* format, ...) {

	if(_write_len >= config::WRITE_BUFFER_SIZE) return false;

	// 利用可变参数，将format解析后，放入_write_buf缓存中
	va_list arg_list;
	va_start(arg_list, format);
	int len = vsnprintf(_write_buf + _write_len, config::WRITE_BUFFER_SIZE - _write_len - 1, format, arg_list);
	va_end(arg_list);

	// 判断缓存是否超出
	if(len < config::WRITE_BUFFER_SIZE - _write_len - 1) {
		_write_len += len;
		return true;
	}
	else {
		return false;
	}
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

	// 响应文件资源
	if(_response_type && _file_address && _file_stat.st_size > 0) {
		if(ret) ret &= write_single_buffer(_file_address, _file_stat.st_size);
		munmap(_file_address, _file_stat.st_size);
		_file_address = nullptr;
	}

	event_modify(_epoll_fd, _client_fd, EPOLLIN);

	if(!ret || !_linger) return false;
	init();
	return true;
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
				event_modify(_epoll_fd, _client_fd, EPOLLOUT);
				usleep(1000);
				continue;
			}
			// 出错
			else return false;
		}

		// 发送成功
		bytes_sent += tmp;
		if(bytes_sent >= len) {
			// printf("发送报文：\n%s\n\n", buf);
			return true;
		}
	}
}
