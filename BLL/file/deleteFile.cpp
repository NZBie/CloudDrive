#include "../BLL.h"

bool bllOperation::deleteFile() {
	
	string token = _params["token"].asString();
	int uid = parse_token(token);

	string fid = _params["fid"].asString();
	string table = fid[0] == '1' ? "folder" : "file";
	string file_update = "update " + table + " set deleted=true where fid=" + fid; 
	execute_insert(file_update);

	_rpsJson["msg"] = "ok";
	return true;
}