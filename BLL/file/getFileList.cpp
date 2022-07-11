#include "../BLL.h"

bool getFileList(const Value& params, Value& rpsJson) {

	string token = params["token"].asString();
	int uid = parse_token(token);
	
	string fid = params["fid"].asString();

	if(fid.size() == 0) {
		string rootID = "select rootID from info where\'" + to_string(uid) + "\'";
		MYSQL_RES* result = execute_query(rootID);
		MYSQL_ROW row = mysql_fetch_row(result);
		fid = row[0];
	}

	// 当前文件夹信息
	string folder_query = "select * from folder where fid=\'" + fid + "\'";
	MYSQL_RES* result = execute_query(folder_query);
	MYSQL_ROW row = mysql_fetch_row(result);
	rpsJson["fid"] = row[0];
	rpsJson["fName"] = row[2];

	// 当前目录下的文件信息
	folder_query = "select * from folder where parID=\'" + fid + "\'";
	result = execute_query(folder_query);
	while(row = mysql_fetch_row(result)) {
		if(strcmp(row[4], "1") == 0) continue;
		Value file;
		file["fid"] = row[0];
		file["fName"] = row[2];
		file["size"] = row[3];
		file["modifyTime"] = row[5];
		file["createTime"] = row[6];
		file["isFolder"] = true;

		rpsJson["fileList"].append(file);
	}

	rpsJson["msg"] = "ok";

	return true;
}