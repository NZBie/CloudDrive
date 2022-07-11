#include "../BLL.h"

bool newFolder(const Value& params, Value& rpsJson) {
	
	string token = params["token"].asString();
	int uid = parse_token(token);

	string fid = params["fid"].asString();
	string fName = params["fName"].asString();

	time_t now = time(NULL);
	tm* tm_t = localtime(&now);
	string curTime = "\'" + to_string(tm_t->tm_year+1900) + "-" + to_string(tm_t->tm_mon+1) + "-" + to_string(tm_t->tm_mday) + " " + 
					to_string(tm_t->tm_hour) + ":" + to_string(tm_t->tm_min) + ":" + to_string(tm_t->tm_sec) + "\'";
	string folder_insert = "insert into folder (parID, fName, size, deleted, modifyTime, createTime) value(\'" + 
							fid + "\',\'" + fName + "\'," + to_string(0) + "," + "false" + "," + curTime + "," + curTime + ")";
	if(execute_insert(folder_insert) == false) return false;

	rpsJson["msg"] = "ok";

	return true;
}