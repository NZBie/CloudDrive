#include "../BLL.h"

bool bllOperation::doLogin() {
	
	string email = fwriter.write(_params["email"]);
	string password = fwriter.write(_params["password"]);

	string sql_query = "select count(*) from users where email=" + email + "and password=" + password;
	MYSQL_RES *result = execute_query(sql_query);
	MYSQL_ROW row = mysql_fetch_row(result);

	if(atoi(row[0]) == 1) _rpsJson["msg"] = "ok";
	else _rpsJson["msg"] = "error";

	_rpsJson["token"] = generate_token(_params["email"].asString());

	return true;
}