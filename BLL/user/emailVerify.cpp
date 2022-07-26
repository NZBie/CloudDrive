#include "../BLL.h"

bool bllOperation::emailVerify() {
	
	string email = fwriter.write(_params["email"]);

	string sql_query = "select count(*) from users where email=" + email;
	MYSQL_RES *result = execute_query(sql_query);
	MYSQL_ROW row = mysql_fetch_row(result);

	if(atoi(row[0]) == 1) _rpsJson["msg"] = "registered";
	else _rpsJson["msg"] = "unregister";

	return true;
}