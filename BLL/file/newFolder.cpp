#include "../BLL.h"

bool bllOperation::newFolder() {
	
	string token = _params["token"].asString();
	int uid = parse_token(token);

	string fid = _params["fid"].asString();
	string fName = _params["fName"].asString();

	time_t now = time(NULL);
	tm* tm_t = localtime(&now);
	string curTime = "\'" + to_string(tm_t->tm_year+1900) + "-" + to_string(tm_t->tm_mon+1) + "-" + to_string(tm_t->tm_mday) + " " + 
					to_string(tm_t->tm_hour) + ":" + to_string(tm_t->tm_min) + ":" + to_string(tm_t->tm_sec) + "\'";
	string folder_insert = "insert into folder (parID, fName, size, deleted, modifyTime, createTime) value(\'" + 
							fid + "\',\'" + fName + "\'," + to_string(0) + "," + "false" + "," + curTime + "," + curTime + ")";
	if(execute_insert(folder_insert) == false) return false;

	_rpsJson["msg"] = "ok";

	return true;
}