#include "/usr/include/mysql/mysql.h"
#include <iostream>

using namespace std;

//연결된 MYSQL을 저장을 해두죠
MYSQL* connectedDatabase;

//Result! 데이터베이스에 "쿼리"를 한다고 해요
//"쿼리"는 무엇인가 특정 데이터를 긁어오는 행위
MYSQL_RES* queryResult;
//데이터를 가져왔는데..? 엄청나게 큰 데이터가 있을 거에요
//이거르 줄 형식으로 가져와야 합니다
MYSQL_ROW queryRow;

bool MYSQLInitialize()
{
	if (connectedDatabase = mysql_init((MYSQL*)nullptr))
	{
		cout << "MySQL 초기화에 실패하였습니다." << endl;
		return false;
	}
	cout << "성공적으로 MySQL 초기화에 성공하였습니다." << endl;

	//						MYSQL 저장할 위치    MYSQL의 주소  ID     비밀번호
	if (!mysql_real_connect(connectedDatabase, "localhost", "root", "cocoa1"))
	{
		cout << "MySQL 연결에 실패하였습니다." << endl;
		return false;
	}
	cout << "성공적으로 MySQL연결에 성공하였습니다." << endl;
	return true;
}

void MySQLClose()
{
	mysql_close(connectedDatabase);
}