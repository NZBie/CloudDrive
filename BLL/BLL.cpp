#include "BLL.h"

#include "user/emailVerify.cpp"
#include "user/doRegister.cpp"
#include "user/doLogin.cpp"

#include "user/getInfo.cpp"

#include "file/getFileList.cpp"
#include "file/newFolder.cpp"
#include "file/deleteFile.cpp"
#include "file/uploadFile.cpp"

#include "token.cpp"

// Json::Reader reader;
Json::FastWriter fwriter;
// Json::StyledWriter swriter;

std::map <string, bll> m_bll;

void map_bll_init() {

	m_bll.insert({"/user/emailVerify", &emailVerify});
	m_bll.insert({"/user/doRegister", &doRegister});
	m_bll.insert({"/user/doLogin", &doLogin});
	
	m_bll.insert({"/user/getInfo", &getInfo});

	m_bll.insert({"/file/getFileList", &getFileList});
	m_bll.insert({"/file/newFolder", &newFolder});
	m_bll.insert({"/file/deleteFile", &deleteFile});
	m_bll.insert({"/file/uploadFile", &uploadFile});
}

bool execute_insert(const string sql_insert) {

	// 从数据库连接池获取连接
	MYSQL* mysql = SqlConnPool::get_instance()->get_connection();

	if(mysql_query(mysql, sql_insert.c_str())) {
		// 释放连接
		SqlConnPool::get_instance()->release_connection(mysql);
		LOG_ERROR("MySQL select error: %s", mysql_error(mysql));
		return false;
	}

	// 释放连接
	SqlConnPool::get_instance()->release_connection(mysql);

	return true;
}

MYSQL_RES* execute_query(const string sql_query) {

	// 从数据库连接池获取连接
	MYSQL* mysql = SqlConnPool::get_instance()->get_connection();

	if(mysql_query(mysql, sql_query.c_str())) {
		// 释放连接
		SqlConnPool::get_instance()->release_connection(mysql);
		LOG_ERROR("MySQL select error: %s", mysql_error(mysql));
		return nullptr;
	}

	MYSQL_RES *result = mysql_store_result(mysql);
	// 释放连接
	SqlConnPool::get_instance()->release_connection(mysql);

	return result;
}

string get_now_dateTime() {

	time_t now = time(nullptr);
	tm* tm_t = localtime(&now);
	string curTime = "\'" + to_string(tm_t->tm_year+1900) + "-" + to_string(tm_t->tm_mon+1) + "-" + to_string(tm_t->tm_mday) + " " + 
					to_string(tm_t->tm_hour) + ":" + to_string(tm_t->tm_min) + ":" + to_string(tm_t->tm_sec) + "\'";
	return curTime;
}
