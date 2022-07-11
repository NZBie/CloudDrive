#include "BLL.h"

string generate_token(const string email) {
	return email;
}

int parse_token(const string token) {

	string uid_query = "select uid from users where email=\'" + token + "\'";
	MYSQL_RES* uid_result = execute_query(uid_query);
	MYSQL_ROW uid_row = mysql_fetch_row(uid_result);
	return atoi(uid_row[0]);
}