#include "FileUploader.h"

// 新建上传任务 调用的构造
FileUploader::FileUploader(string name, string md5, int tot_size, int part_num):
_file_md5(md5), _name(name), _tot_size(tot_size), _part_num(part_num) {

	string state = "";
	for(int i=0;i<part_num;i++) {
		state += "00000000";
	}
	
	// 将新建的上传任务，存储到mysql
	string upload_insert = "insert into upload (name, md5, tot_size, part_num, upload_state) value(\'" + 
							name + "\',\'" + md5 + "\'," + to_string(tot_size) + "," + to_string(part_num) + ",\'" + state + "\')";
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
	memset(_upload_state, 0, sizeof(_upload_state));
	
	// string 转成 十六进制形式的 unsigned
	for(int i=0;i<_part_num;i++) {
		for(int j=0;j<8;j++) { 
			char bitVal_char = row[5][i*8+j];
			int bitVal = bitVal_char - (bitVal_char < 'a' ? '0' : 'a' - 10);
			_upload_state[i] = (_upload_state[i] << 4) + bitVal;
		}
	}
};

// 上传单个分片
bool FileUploader::upload_part(int part_id, char* file_part, unsigned part_len, string md5) {

	// 将文件写入服务器磁盘中
	char path[64] = "./users_drive/uploading/";
	string part_name = _name + ".part_" + to_string(part_id);
	strcat(strcat(strcat(path, to_string(_id).c_str()), "/"), part_name.c_str());
	FILE* fp = fopen(path, "a");
	if(fp == nullptr) return false;

	// 写
	int written_len = fwrite(file_part, sizeof(char), part_len, fp);
	fclose(fp);

	// 更新上传进度
	unsigned new_state = (part_len == written_len ? 0xffffffff : _upload_state[part_id] + written_len);
	update_state(part_id, new_state);
	update_info();

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

// 更新分片上传进度
inline void FileUploader::update_state(int part_id, unsigned progress) {
	if(part_id >= _part_num) return;
	_upload_state[part_id] = progress;
}

// 更新upload表中的信息
bool FileUploader::update_info() {

	// 将upload_state转成string，便于存入mysql
	string state_str;
	for(int i=0;i<_part_num;i++) {
		// unsigned 以十六进制形式 转 string
		string num_str;
		for(int j=0;j<8;j++) {
			unsigned num = _upload_state[i];
			int bitVal = num & 0xf;
			num >>= 4;
			char bitVal_char = bitVal + (bitVal < 10 ? '0' : 'a' - 10);
			num_str = bitVal_char + num_str;
		}
		state_str += num_str;
	}
	
	string sql_update = "update upload set upload_state=\'" + state_str + "\' where id=\'" + to_string(_id) + "\'";
	return execute_update(sql_update);
}				

// 校验MD5码
inline bool FileUploader::check_MD5(string md5) {
	return md5 == _file_md5;
}

// 检验任务是否全部完成
bool FileUploader::check_complete() {

	for(int i=0;i<_part_num;i++) {
		if(_upload_state[i] != 0xffffffff) return false;
	}
	return true;
}


