#include "../BLL.h"

bool bllOperation::doRegister() {

	string email = fwriter.write(_params["email"]);
	string behalfEmail = fwriter.write(_params["behalfEmail"]);
	string password = fwriter.write(_params["password"]);

	string user_insert = "insert into users (email, password) values(" + 
							email + "," + password + ")";
	int uid = execute_insert_returnID(user_insert);

	time_t now = time(NULL);
	tm* tm_t = localtime(&now);
	string curTime = "\'" + to_string(tm_t->tm_year+1900) + "-" + to_string(tm_t->tm_mon+1) + "-" + to_string(tm_t->tm_mday) + " " + 
					to_string(tm_t->tm_hour) + ":" + to_string(tm_t->tm_min) + ":" + to_string(tm_t->tm_sec) + "\'";
	string folder_insert = "insert into folder (parID, fName, size, deleted, modifyTime, createTime) value(" + 
							to_string(100000) + "," + email + "," + to_string(0) + "," + "false" + "," + curTime + "," + curTime + ")";
	int fid = execute_insert_returnID(folder_insert);

	string info_insert = "insert into info (uid, email, behalfEmail, authority, nickname, rootID) value(" + 
							to_string(uid) + "," + email + "," + behalfEmail + "," + (email == behalfEmail ? "\'enterprise\'" : "\'staff\'") + "," + email + "," + to_string(fid) + ")";
	execute_insert(info_insert);

	_rpsJson["msg"] = "ok";

	return true;
}