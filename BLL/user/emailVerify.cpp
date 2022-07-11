#include "../BLL.h"

bool emailVerify(const Value& params, Value& rpsJson) {
	
	string email = fwriter.write(params["email"]);

	string sql_query = "select count(*) from users where email=" + email;
	MYSQL_RES *result = execute_query(sql_query);
	MYSQL_ROW row = mysql_fetch_row(result);

	if(atoi(row[0]) == 1) rpsJson["msg"] = "registered";
	else rpsJson["msg"] = "unregister";

	return true;
}