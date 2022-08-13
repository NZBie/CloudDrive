#include "../BLL.h"
#include "../../FileTransfer/FileUploader.h"

bool bllOperation::getTaskList() {

	string token = _params["token"].asString();
	int uid = parse_token(token);

	string upload_query = "select id, name, tot_size from task where uid=" + to_string(uid);
	MYSQL_RES* res = execute_query(upload_query);
	MYSQL_ROW row;
	Value tasks = Json::arrayValue;
	while(row = mysql_fetch_row(res)) {
		Value task;
		int task_id = atoi(row[0]);
		task["id"] = task_id;
		task["name"] = row[1];
		task["tot_size"] = atoi(row[2]);

		FileUploader fileUper(task_id);
		// 上传任务已完成
		if(fileUper.get_task_id() == -1) {
			task["sent"] = task["tot_size"];
		}
		// 上传中
		else {
			int part_num = fileUper.get_part_num();
			int sent = 0;
			for(int i=0;i<part_num;i++) {
				sent += fileUper.get_upload_state(i);
			}
			task["sent"] = sent;		
		}
		tasks.append(task);
	}
	_rpsJson["tasks"] = tasks;
	_rpsJson["msg"] = "ok";
	return true;
}

