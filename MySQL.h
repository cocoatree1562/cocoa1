#include "/usr/include/mysql/mysql.h"
#include <iostream>

using namespace std;

//����� MYSQL�� ������ �ص���
MYSQL* connectedDatabase;

//Result! �����ͺ��̽��� "����"�� �Ѵٰ� �ؿ�
//"����"�� �����ΰ� Ư�� �����͸� �ܾ���� ����
MYSQL_RES* queryResult;
//�����͸� �����Դµ�..? ��û���� ū �����Ͱ� ���� �ſ���
//�̰Ÿ� �� �������� �����;� �մϴ�
MYSQL_ROW queryRow;

bool MYSQLInitialize()
{
	if (connectedDatabase = mysql_init((MYSQL*)nullptr))
	{
		cout << "MySQL �ʱ�ȭ�� �����Ͽ����ϴ�." << endl;
		return false;
	}
	cout << "���������� MySQL �ʱ�ȭ�� �����Ͽ����ϴ�." << endl;

	//						MYSQL ������ ��ġ    MYSQL�� �ּ�  ID     ��й�ȣ
	if (!mysql_real_connect(connectedDatabase, "localhost", "root", "cocoa1"))
	{
		cout << "MySQL ���ῡ �����Ͽ����ϴ�." << endl;
		return false;
	}
	cout << "���������� MySQL���ῡ �����Ͽ����ϴ�." << endl;
	return true;
}

void MySQLClose()
{
	mysql_close(connectedDatabase);
}