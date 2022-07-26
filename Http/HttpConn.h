#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <cstdio>
#include <cstdarg>

#include <sys/mman.h>
#include <cstring>
#include <string>
#include <set>

#include <jsoncpp/json/json.h>

#include "../log/log.h"
#include "../MySQL_CGI/SqlConnPool.h"
#include "../BLL/BLL.h"
#include "FormData/FormDataParser.h"
#include "FormData/FormItem.h"
#include "../config.h"

using std::string;
using Json::Value;

class HttpConn {
public:
	enum METHOD
	{
		GET = 0,
		POST,
		HEAD,
		PUT,
		DELETE,
		TRACE,
		OPTIONS,
		CONNECT,
		PATH
	};
	enum CHECK_STATUS
	{
		REQUEST_LINE = 0,	// 解析 请求行
		REQUEST_HEADER,		// 解析 请求头
		REQUEST_CONTENT		// 解析 消息体，仅POST请求
	};
	enum HTTP_CODE
	{
		NO_REQUEST = 0,		// 无需响应的请求
		GET_REQUEST,		// 解析完成

		OK_REQUEST,			// 请求成功		200
		BAD_REQUEST,		// 请求失败		400
		FORBIDDEN_REQUEST,	// 拒绝访问		403
		NO_RESOURCE,		// 未找到资源	404
		INTERNAL_ERROR,		// 服务器出错	500
		CLOSED_CONNECTION
	};
	enum LINE_STATE
	{
		LINE_OK = 0,	// 当前行 读取并解析成功
		LINE_BAD,		// 当前行 语法错误
		LINE_OPEN		// 当前行 未读取完整
	};
	struct HTTP_MESSAGE {
		int code;
		char title[16];
		char form[128];
	};
	static const HTTP_MESSAGE http_messages[10];

public:
	HttpConn();
	~HttpConn();

	void init(int client_fd, sockaddr_in& address);
	void complete_process();	// 解析 & 处理 & 反馈 的过程

	// socket ~ buffer
	bool read_request();		// 将请求 从socket 读取至 缓存
	bool write_response();		// 将响应 从缓存 写回到 socket

private:
	void init();
	void close_connection();
	void event_modify(int epoll_fd, int client_fd, int event);

	bool write_single_buffer(char* buf, int len);

	// 获取 & 解析 报文
	HTTP_CODE parse_message();
	HTTP_CODE parse_request_line(char* text);
	HTTP_CODE parse_request_headers(char* text);
	HTTP_CODE parse_request_content(char* text);
	LINE_STATE get_line();

	// 写报文
	bool add_message(HTTP_CODE ret);
    inline bool add_response_line(int status, const char* title);
    inline bool add_response_headers(int content_len);
	inline bool add_response_content(const char* content);
	bool add_single_line(const char* format, ...);

	// 处理报文
	HTTP_CODE do_request();
    
public:
	static std::set<int> _users;
	static int _user_count;
	static int _epoll_fd;
	sockaddr_in _address;

	int _state;
	bool improve;
	bool timer_flag;
	bool _linger;

private:
	int _client_fd;
	char* _root;			// 服务器资源root目录

	// 读
	char* _read_buf;					// 读缓存
	int _read_len;						// 读缓存 中内容的大小
	int _line_begin;					// 当前解析行 的起始位置
	int _line_end;						// 当前解析行 的末位置
	CHECK_STATUS _check_status;

	// 写
	char* _write_buf;					// 写缓存
	int _write_len;						// 写缓存 中内容的大小
	char* _file_address;				// 文件存储的地址
	struct stat _file_stat;				// 文件类

	// 数据库
	char sql_user[16];
	char sql_passwd[16];
	char sql_dbname[16];

	// 请求报文的信息
	METHOD _method;			// 请求方法
	char* _url;				// 请求行url
	Value _rqs_params;		// 请求报文携带的参数，包含url和content
	char* _version;			// http协议版本
	char* _host;			// 主机ip
	int _content_length;	// 请求体大小
	char _boundary[50];		// FormData数据边界

	// 响应报文的信息
	Value _response_json;			// 响应的json参数
	char _real_file[config::FILENAME_LEN];	// 文件资源路径
	bool _response_type; 			// 响应类型，file or data
	char _content_type[16];			// 响应体数据类型

	bllOperation _bll;
};

#endif