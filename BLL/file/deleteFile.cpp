#include "../BLL.h"

bool deleteFile(const Value& params, Value& rpsJson) {
	
	string token = params["token"].asString();
	int uid = parse_token(token);

	string fid = params["fid"].asString();
	string table = fid[0] == '1' ? "folder" : "file";
	string file_update = "update " + table + " set deleted=true where fid=" + fid; 
	execute_insert(file_update);

	rpsJson["msg"] = "ok";
	return true;
}