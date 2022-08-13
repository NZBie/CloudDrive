#ifndef FILE_UPLOADER
#define FILE_UPLOADER

#include <string>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../BLL/BLL.h"

using std::string;

class FileUploader {
public:
	FileUploader(string name, string md5, int tot_size, int part_num);
	FileUploader(int id);
	~FileUploader() {};

	// 获取基本信息
	int get_task_id() { return _id; };
	int get_tot_size() { return _tot_size; }
	string get_file_name() { return _name; };
	int get_part_num() { return _part_num; };
	int get_upload_state(int part_id);
	char* get_part_path(int part_id);

	bool upload_part(int part_id, char* file_part, int part_len, string md5);	// 上传单个分片
	bool unite_parts(int fid);			// 合并临时文件
	bool remove_parts();				// 删除临时文件

	int insert_file(int folder_id);		// 添加文件信息到file表
	bool delete_upload();				// 从upload表删除上传信息

	inline bool check_MD5(string md5);	// 校验MD5码
	bool check_complete();				// 检验任务是否全部完成
	
private:
	int _id;					// 上传唯一标识id
	string _name;				// 文件名
	string _file_md5;			// MD5校验
	int _tot_size;				// 文件总大小
	int _part_num;				// 分片数
	// unsigned _upload_state[16];	// 各分片的上传状态，每8位表示一个十六进制数，即对应分片的上传进度。128 = 8*16，即最多分片数为16
};

#endif