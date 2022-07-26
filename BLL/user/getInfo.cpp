#include "../BLL.h"

bool bllOperation::getInfo() {
	
	string token = _params["token"].asString();
	int uid = parse_token(token);

	// 数据库查询语句
	string sql_query = " select * from info where uid=" + to_string(uid);

	// 获取查询结果
	MYSQL_RES *result = execute_query(sql_query);
	MYSQL_ROW row = mysql_fetch_row(result);

	Value info;
	info["email"] = string(row[1]);
	info["behalfEmail"] = string(row[2]);
	info["authority"] = string(row[3]);
	info["nickname"] = string(row[4]);
	info["birth"] = string(row[5] ? row[5] : "");
	info["nation"] = string(row[6] ? row[6] : "");
	info["phone"] = string(row[7] ? row[7] : "");

	_rpsJson["info"] = info;
	_rpsJson["msg"] = "ok";

	return true;
}