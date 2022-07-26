class UploadCheckPoint {
public:
	UploadCheckPoint();
	~UploadCheckPoint();
	
private:
	int _id;				// 上传唯一标识id
	int _file_md5;			// MD5校验
	int _uploaded_state;	// 各分片的上传状态
	int _tot_size;			// 文件总大小
	int _part_size;			// 每片大小
};
