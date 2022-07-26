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

class bllOperation {
public:
	bllOperation(const Value& params, Value& rpsJson):
	_params(params), _rpsJson(rpsJson) {
		if(m_bll.empty()) map_bll_init();
	};
	~bllOperation() {};
	static bool isExist(string name);	// 查询是否存在字符串对应的操作
	bool execute(string name);			// 由字符串转到对应的操作函数，并执行
	
private:
	// map_bll及其初始化
	static void map_bll_init();
	typedef bool (bllOperation::*bll_func)();
	static std::map <string, bll_func> m_bll;

	// 所有操作声明
	bool emailVerify();		// 验证邮箱
	bool doRegister();		// 注册
	bool doLogin();			// 登录

	bool getInfo();			// 获取用户信息

	bool getFileList();		// 获取文件列表
	bool newFolder(); 		// 新建文件夹
	bool deleteFile();		// 删除文件

	bool uploadFile();		// 上传文件
	bool newUploadTask();	// 新建上传任务
	bool uploadPart();		// 上传文件片段

	string generate_token(const string email);				// 生成token
	int parse_token(const string token);					// 解析token

	// 基本数据库操作
	bool execute_insert(const string sql_insert);			// 插入单条sql语句
	int execute_insert_returnID(const string sql_insert);	// 插入单条sql语句并返回自增ID
	MYSQL_RES* execute_query(const string sql_query);		// 单次查询

	string get_now_dateTime();

private:
	const Value& _params;
	Value& _rpsJson;
};

#endif