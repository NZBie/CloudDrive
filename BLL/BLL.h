#ifndef BLL_H
#define BLL_H

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>
#include <ctime>
#include <map>

#include "../log/log.h"
#include "../MySQL_CGI/SqlConnPool.h"
#include "../config.h"

using Json::Value;

// extern Json::Reader reader;
extern Json::FastWriter fwriter;
// extern Json::StyledWriter swriter;

typedef bool (*bll)(const Value&, Value&);
extern std::map <string, bll> m_bll;

// map_bll初始化
void map_bll_init();

// 所有操作声明
bool emailVerify(const Value& params, Value& rpsJson);	// 验证邮箱
bool doRegister(const Value& params, Value& rpsJson);	// 注册
bool doLogin(const Value& params, Value& rpsJson);		// 登录

bool getInfo(const Value& params, Value& rpsJson);		// 获取用户信息

bool getFileList(const Value& params, Value& rpsJson);	// 获取文件列表
bool newFolder(const Value& params, Value& rpsJson); 	// 新建文件夹
bool deleteFile(const Value& params, Value& rpsJson);	// 删除文件
bool uploadFile(const Value& params, Value& rpsJson);	// 上传文件

string generate_token(const string email);				// 生成token
int parse_token(const string token);					// 解析token

// 基本数据库操作
bool execute_insert(const string sql_insert);
MYSQL_RES* execute_query(const string sql_query);

string get_now_dateTime();

#endif