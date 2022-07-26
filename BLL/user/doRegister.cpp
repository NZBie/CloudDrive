#include "../BLL.h"

bool bllOperation::doRegister() {

	// 数据库查询语句
	string email = fwriter.write(_params["email"]);
	string behalfEmail = fwriter.write(_params["behalfEmail"]);
	string password = fwriter.write(_params["password"]);

	string user_insert = "insert into users (email, password) values(" + 
							email + "," + password + ")";
	execute_insert(user_insert);

	time_t now = time(NULL);
	tm* tm_t = localtime(&now);
	string curTime = "\'" + to_string(tm_t->tm_year+1900) + "-" + to_string(tm_t->tm_mon+1) + "-" + to_string(tm_t->tm_mday) + " " + 
					to_string(tm_t->tm_hour) + ":" + to_string(tm_t->tm_min) + ":" + to_string(tm_t->tm_sec) + "\'";
	string folder_insert = "insert into folder (parID, fName, size, deleted, modifyTime, createTime) value(" + 
							to_string(100000) + "," + email + "," + to_string(0) + "," + "false" + "," + curTime + "," + curTime + ")";
	execute_insert(folder_insert);

	string uid_query = "select uid from users where email=" + email;
	MYSQL_RES *uid_result = execute_query(uid_query);
	MYSQL_ROW uid_row = mysql_fetch_row(uid_result);
	string uid = string(uid_row[0]);
	string fid_query = "select fid from folder where fName=" + email;
	MYSQL_RES *fid_result = execute_query(fid_query);
	MYSQL_ROW fid_row = mysql_fetch_row(fid_result);
	string fid = fid_row[0];
	string info_insert = "insert into info (uid, email, behalfEmail, authority, nickname, rootID) value(" + 
							uid + "," + email + "," + behalfEmail + "," + (email == behalfEmail ? "\'enterprise\'" : "\'staff\'") + "," + email + "," + fid + ")";
	execute_insert(info_insert);

	_rpsJson["msg"] = "ok";

	return true;
}