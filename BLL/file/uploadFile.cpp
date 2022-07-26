#include "../BLL.h"

bool bllOperation::newUploadTask() {


}

bool bllOperation::uploadPart() {
	
}

bool bllOperation::uploadFile() {

	string token = _params["token"].asString();
	int uid = parse_token(token);

	// 数据库插入操作
	string parID = _params["fid"].asString();
	string fName = _params["fName"].asString();
	int fSize = _params["fSize"].asInt();
	string extension;
	for(int i=fName.size()-1;i>=0;i--) {
		if(fName[i] == '.') {
			extension = fName.substr(i);
			break;
		}
	}

	string curTime = get_now_dateTime();
	string file_insert = "insert into file (parID, fName, size, extension, deleted, modifyTime, createTime) value(\'" + 
							parID + "\',\'" + fName + "\'," + to_string(fSize) + ",\'" + extension + "\'," + "false" + "," + curTime + "," + curTime + ")";
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
	long long fData_p = _params["fData"].asLargestInt();
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

	_rpsJson["msg"] = "ok";

	return true;
}