#ifndef FILE_UPLOADER
#define FILE_UPLOADER
#include <string>

using std::string;

class FileUploader {
public:
	FileUploader(int id, string file_md5, int uploaded_state, int tot_size, int part_size):
	_id(id), _file_md5(file_md5), _uploaded_state(uploaded_state), _tot_size(tot_size), _part_size(part_size) {};
	~FileUploader();

	
	
private:
	int _id;				// 上传唯一标识id
	string _file_md5;		// MD5校验
	int _uploaded_state;	// 各分片的上传状态
	int _tot_size;			// 文件总大小
	int _part_size;			// 每片大小
};

#endif