#include "../BLL.h"

bool uploadFile(const Value& params, Value& rpsJson) {

	string token = params["token"].asString();
	int uid = parse_token(token);

	// 数据库插入操作
	string parID = params["fid"].asString();
	string fName = params["fName"].asString();
	int fSize = params["fSize"].asInt();

	string curTime = get_now_dateTime();
	string file_insert = "insert into file (parID, fName, size, extension, deleted, modifyTime, createTime) value(\'" + 
							parID + "\',\'" + fName + "\'," + to_string(fSize) + ",\'" + ".png" + "\'," + "false" + "," + curTime + "," + curTime + ")";
	if(execute_insert(file_insert) == false) {
		return false;
	}

	// 获取新文件的fid
	string fid_query = "select fid from file where parID=\'" + parID + "\' and fName=\'" + fName + "\'";
	MYSQL_RES* result = execute_query(fid_query);
	MYSQL_ROW row = mysql_fetch_row(result);
	char fid[8];
	strcpy(fid, row[0]);

	// 将文件写入服务器磁盘中
	long long fData_p = params["fData"].asLargestInt();
	char** fData = (char**)&fData_p;

	char path[64] = "./users-drive/";
	strcat(path, fid);
	int ret = mkdir(path, S_IRWXU);
	if(ret == -1) return false;

	strcat(strcat(path, "/"), fName.c_str());
	FILE* fp = fopen(path, "w");
	if(fp == nullptr) return false;

	int x = fwrite(*fData, sizeof(char), fSize, fp);
	fclose(fp);

	rpsJson["msg"] = "ok";

	return true;
}