#include "FileUploader.h"

// 新建上传任务 调用的构造
FileUploader::FileUploader(string name, string md5, int tot_size, int part_num):
_file_md5(md5), _name(name), _tot_size(tot_size), _part_num(part_num) {

	// 将新建的上传任务，存储到mysql
	string upload_insert = "insert into upload (name, md5, tot_size, part_num) value(\'" + 
							name + "\',\'" + md5 + "\'," + to_string(tot_size) + "," + to_string(part_num) + ")";
	_id = execute_insert_returnID(upload_insert);

	// 创建临时文件夹
	char path[64] = "./users_drive/uploading/";
	strcat(path, to_string(_id).c_str());
	int ret = mkdir(path, S_IRWXU);
	// assert(ret);
};

// 读取上传任务 调用的构造
FileUploader::FileUploader(int id):	_id(id) {
	
	// 由id从mysql中查询上传任务的记录
	string file_query = "select * from upload where id=" + to_string(id);
	MYSQL_RES* result = execute_query(file_query);
	MYSQL_ROW row = mysql_fetch_row(result);

	_name = row[1];
	_file_md5 = row[2];
	_tot_size = atoi(row[3]);
	_part_num = atoi(row[4]);
};

// 获取上传进度
int FileUploader::get_upload_state(int part_id) {

	// 临时文件路径
	char part_path[64] = "./users_drive/uploading/";
	strcat(strcat(part_path, to_string(_id).c_str()), "/");
	string part_name = _name + ".part_" + to_string(part_id);
	strcat(part_path, part_name.c_str());

	// 从临时文件中获取已写大小
	struct stat info;
	if(stat(part_path, &info) < 0) return 0;
	if(!(info.st_mode & S_IROTH)) return 0;
	if(S_ISDIR(info.st_mode)) return 0;

	return info.st_size;
}

// 上传单个分片
bool FileUploader::upload_part(int part_id, char* file_part, int part_len, string md5) {

	// 将文件写入服务器磁盘中
	char path[64] = "./users_drive/uploading/";
	string part_name = _name + ".part_" + to_string(part_id);
	strcat(strcat(strcat(path, to_string(_id).c_str()), "/"), part_name.c_str());
	FILE* fp = fopen(path, "a");
	if(fp == nullptr) return false;

	// 写
	int written_len = fwrite(file_part, sizeof(char), part_len, fp);
	fclose(fp);

	return true;
}

// 合并临时文件
bool FileUploader::unite_parts(int fid) {

	// 新建文件夹
	char file_path[64] = "./users_drive/";
	strcat(file_path, to_string(fid).c_str());
	int ret = mkdir(file_path, S_IRWXU);
	if(ret == -1) return false;

	// 最终文件和临时文件的路径
	strcat(strcat(file_path, "/"), _name.c_str());
	FILE* fp = fopen(file_path, "a");
	if(fp == nullptr) return false;

	char part_path[64] = "./users_drive/uploading/";
	strcat(strcat(part_path, to_string(_id).c_str()), "/");
	int path_len = strlen(part_path);

	for(int i=0;i<_part_num;i++) {

		// 临时文件路径
		part_path[path_len] = '\0';
		string part_name = _name + ".part_" + to_string(i);
		strcat(part_path, part_name.c_str());
		
		// 从_real_file文件中获取文件信息
		struct stat part_stat;
		if(stat(part_path, &part_stat) < 0) return false;
		if(!(part_stat.st_mode & S_IROTH)) return false;
		if(S_ISDIR(part_stat.st_mode)) return false;

		// 硬盘到内存的地址映射
		int part_fd = open(part_path, O_RDONLY);
		char* part_address = (char *)mmap(0, part_stat.st_size, PROT_READ, MAP_PRIVATE, part_fd, 0);
		close(part_fd);

		// 写入到最终文件
		int written_len = fwrite(part_address, sizeof(char), part_stat.st_size, fp);
	}
	fclose(fp);
	return true;
}

// 删除临时文件
bool FileUploader::remove_parts() {

	// 临时文件的路径
	char part_path[64] = "./users_drive/uploading/";
	strcat(strcat(part_path, to_string(_id).c_str()), "/");
	int path_len = strlen(part_path);

	for(int i=0;i<_part_num;i++) {

		// 临时文件路径
		part_path[path_len] = '\0';
		string part_name = _name + ".part_" + to_string(i);
		strcat(part_path, part_name.c_str());
		
		int ret = remove(part_path);
		if(ret == -1) return false;
	}

	// 删除文件夹
	char file_path[64] = "./users_drive/uploading/";
	strcat(file_path, to_string(_id).c_str());
	int ret = rmdir(file_path);
	if(ret == -1) return false;
	return true;
}

// 添加文件信息到file表
int FileUploader::insert_file(int folder_id) {

	// 数据库插入操作
	string extension;
	for(int i=_name.size()-1;i>=0;i--) {
		if(_name[i] == '.') {
			extension = _name.substr(i);
			break;
		}
	}

	string curTime = get_now_dateTime();
	string file_insert = "insert into file (parID, fName, size, extension, deleted, modifyTime, createTime) value(\'" + 
							to_string(folder_id) + "\',\'" + _name + "\'," + to_string(_tot_size) + ",\'" + extension + "\'," + "false" + "," + curTime + "," + curTime + ")";
	int fid = execute_insert_returnID(file_insert);
	return fid;
}

// 从upload表删除上传信息
bool FileUploader::delete_upload() {
	string upload_dlt = "delete from upload where id=" + to_string(_id);
	return execute_delete(upload_dlt);
}

// 校验MD5码
inline bool FileUploader::check_MD5(string md5) {
	return md5 == _file_md5;
}

// 检验任务是否全部完成
bool FileUploader::check_complete() {

	for(int i=0;i<_part_num;i++) {
		// 从临时文件中获取已写大小
		int part_size = get_upload_state(i);
		int part_tot = (_tot_size - 1) / _part_num + 1;
		if(i == _part_num - 1) part_tot = _tot_size - (_part_num - 1) * part_tot;
		if(part_size < part_tot) return false; 
	}
	return true;
}

