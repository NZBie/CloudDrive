#include "BLL.h"

#include "user/emailVerify.cpp"
#include "user/doRegister.cpp"
#include "user/doLogin.cpp"

#include "user/getInfo.cpp"

#include "file/getFileList.cpp"
#include "file/getTaskList.cpp"
#include "file/newFolder.cpp"
#include "file/deleteFile.cpp"
#include "file/uploadFile.cpp"

#include "token.cpp"

// Json::Reader reader;
Json::FastWriter fwriter;
// Json::StyledWriter swriter;

std::map <string, bllOperation::bll_func> bllOperation::m_bll;

void bllOperation::map_bll_init() {

	m_bll.insert({"/user/emailVerify", &bllOperation::emailVerify});
	m_bll.insert({"/user/doRegister", &bllOperation::doRegister});
	m_bll.insert({"/user/doLogin", &bllOperation::doLogin});
	
	m_bll.insert({"/user/getInfo", &bllOperation::getInfo});

	m_bll.insert({"/file/getFileList", &bllOperation::getFileList});
	m_bll.insert({"/file/newFolder", &bllOperation::newFolder});
	m_bll.insert({"/file/deleteFile", &bllOperation::deleteFile});

	m_bll.insert({"/file/getTaskList", &bllOperation::getTaskList});
	
	m_bll.insert({"/file/newUploadTask", &bllOperation::newUploadTask});
	m_bll.insert({"/file/deleteUploadTask", &bllOperation::deleteUploadTask});
	m_bll.insert({"/file/queryUploadProgress", &bllOperation::queryUploadProgress});
	m_bll.insert({"/file/uploadPart", &bllOperation::uploadPart});
}

bool bllOperation::isExist(string name) {

	return (bllOperation::m_bll.find(name) != bllOperation::m_bll.end());
}

bool bllOperation::execute(string name) {

	if(!isExist(name)) return false;
	(this->*m_bll[name])();
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

int execute_insert_returnID(const string sql_insert) {

	// 从数据库连接池获取连接
	MYSQL* mysql = SqlConnPool::get_instance()->get_connection();

	// sql插入语句
	if(mysql_query(mysql, sql_insert.c_str())) {
		// 释放连接
		SqlConnPool::get_instance()->release_connection(mysql);
		LOG_ERROR("MySQL select error: %s", mysql_error(mysql));
		return -1;
	}

	// 获取自增id
	if(mysql_query(mysql, "SELECT LAST_INSERT_ID()")) {
		// 释放连接
		SqlConnPool::get_instance()->release_connection(mysql);
		LOG_ERROR("MySQL select error: %s", mysql_error(mysql));
		return -1;
	}
	MYSQL_RES *result = mysql_store_result(mysql);
	
	// 释放连接
	SqlConnPool::get_instance()->release_connection(mysql);

	MYSQL_ROW id_row = mysql_fetch_row(result);
	return atoi(id_row[0]);
}

bool execute_delete(const string sql_delete) {

	// 从数据库连接池获取连接
	MYSQL* mysql = SqlConnPool::get_instance()->get_connection();

	if(mysql_query(mysql, sql_delete.c_str())) {
		// 释放连接
		SqlConnPool::get_instance()->release_connection(mysql);
		LOG_ERROR("MySQL select error: %s", mysql_error(mysql));
		return false;
	}

	// 释放连接
	SqlConnPool::get_instance()->release_connection(mysql);

	return true;
}

bool execute_update(const string sql_update) {

	// 从数据库连接池获取连接
	MYSQL* mysql = SqlConnPool::get_instance()->get_connection();

	if(mysql_query(mysql, sql_update.c_str())) {
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
