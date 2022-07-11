#include "../BLL.h"

bool doLogin(const Value& params, Value& rpsJson) {
	
	string email = fwriter.write(params["email"]);
	string password = fwriter.write(params["password"]);

	string sql_query = "select count(*) from users where email=" + email + "and password=" + password;
	MYSQL_RES *result = execute_query(sql_query);
	MYSQL_ROW row = mysql_fetch_row(result);

	if(atoi(row[0]) == 1) rpsJson["msg"] = "ok";
	else rpsJson["msg"] = "error";

	rpsJson["token"] = generate_token(params["email"].asString());

	return true;
}