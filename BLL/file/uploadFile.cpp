#include "../BLL.h"
#include "../../FileTransfer/FileUploader.h"

// 新建上传任务
bool bllOperation::newUploadTask() {

	string token = _params["token"].asString();
	int uid = parse_token(token);

	string name = _params["name"].asString();
	string md5 = _params["md5"].asString();
	int tot_size = stoi(_params["tot_size"].asString());
	int part_num = stoi(_params["part_num"].asString());

	FileUploader fileUper(name, md5, tot_size, part_num);

	// 插入任务到task表
	string task_insert = "insert into task (id, uid, fid, name, tot_size) value(" + 
						to_string(fileUper.get_task_id()) + "," + to_string(uid) + "," + _params["fid"].asString() + ",\'" + fileUper.get_file_name() + "\'," + to_string(fileUper.get_tot_size()) + ")";
	execute_insert(task_insert);
	
	_rpsJson["task_id"] = fileUper.get_task_id();
	_rpsJson["msg"] = "ok";
	return true;
}

// 删除上传任务
bool bllOperation::deleteUploadTask() {

	string token = _params["token"].asString();
	int uid = parse_token(token);

	string id = _params["tid"].asString();
	FileUploader fileUper(stoi(id));

	// 上传任务未完成
	if(fileUper.get_task_id() != -1) {

		// 删除临时文件
		fileUper.remove_parts();
		fileUper.delete_upload();
	}

	// 从task表中删除任务
	string task_delete = "delete from task where id=" + id;
	execute_delete(task_delete);

	_rpsJson["msg"] = "ok";
	return true;
}

// 查询上传进度
bool bllOperation::queryUploadProgress() {

	int id = stoi(_params["id"].asString());
	FileUploader fileUper(id);

	int part_id = stoi(_params["part_id"].asString());

	// 获取上传信息
	int sent = fileUper.get_upload_state(part_id);

	_rpsJson["sent"] = sent;
	_rpsJson["msg"] = "ok";

	return true;
}

// 上传文件分片
bool bllOperation::uploadPart() {

	int id = stoi(_params["id"].asString());
	FileUploader fileUper(id);

	int part_id = stoi(_params["part_id"].asString());
	long long fData_p = _params["fData"].asLargestInt();
	char** fData = (char**)&fData_p;
	int part_len = stoi(_params["fSize"].asString());
	string md5 = _params["md5"].asString();

	// 上传文件分片
	fileUper.upload_part(part_id, *fData, part_len, md5);
	_rpsJson["sent"] = fileUper.get_upload_state(part_id);

	// 合并分片
	if(fileUper.check_complete()) {
		int fid = fileUper.insert_file(stoi(_params["folder_id"].asString()));
		fileUper.unite_parts(fid);
		fileUper.remove_parts();
		fileUper.delete_upload();
		_rpsJson["upload_state"] = "success";
	}
	else _rpsJson["upload_state"] = "uploading";

	_rpsJson["msg"] = "ok";
	return true;
}

// bool bllOperation::uploadFile() {

// 	string token = _params["token"].asString();
// 	int uid = parse_token(token);

// 	// 数据库插入操作
// 	string parID = _params["fid"].asString();
// 	string fName = _params["fName"].asString();
// 	int fSize = _params["fSize"].asInt();
// 	string extension;
// 	for(int i=fName.size()-1;i>=0;i--) {
// 		if(fName[i] == '.') {
// 			extension = fName.substr(i);
// 			break;
// 		}
// 	}

// 	string curTime = get_now_dateTime();
// 	string file_insert = "insert into file (parID, fName, size, extension, deleted, modifyTime, createTime) value(\'" + 
// 							parID + "\',\'" + fName + "\'," + to_string(fSize) + ",\'" + extension + "\'," + "false" + "," + curTime + "," + curTime + ")";
// 	if(execute_insert(file_insert) == false) {
// 		return false;
// 	}

// 	// 获取新文件的fid
// 	string fid_query = "select fid from file where parID=\'" + parID + "\' and fName=\'" + fName + "\'";
// 	MYSQL_RES* result = execute_query(fid_query);
// 	MYSQL_ROW row = mysql_fetch_row(result);
// 	char fid[8];
// 	strcpy(fid, row[0]);

// 	// 将文件写入服务器磁盘中
// 	long long fData_p = _params["fData"].asLargestInt();
// 	char** fData = (char**)&fData_p;

// 	char path[64] = "./users_drive/";
// 	strcat(path, fid);
// 	int ret = mkdir(path, S_IRWXU);
// 	if(ret == -1) return false;

// 	strcat(strcat(path, "/"), fName.c_str());
// 	FILE* fp = fopen(path, "w");
// 	if(fp == nullptr) return false;

// 	int x = fwrite(*fData, sizeof(char), fSize, fp);
// 	fclose(fp);

// 	_rpsJson["msg"] = "ok";

// 	return true;
// }