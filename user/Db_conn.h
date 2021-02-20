//
// Created by maomao on 2021/2/9.
//

#ifndef INDEXTSDB_DB_CONN_H
#define INDEXTSDB_DB_CONN_H
#include <list>
#include <iostream>
#include "string"
//#include "include/mysql.h"
using namespace std;

class Db_conn
{
public:
    Db_conn();
    ~Db_conn();
    bool initDB(string host,string user,string pwd,string db_name,int port); //连接mysql
    bool exeSQL(string sql);   //执行sql语句
    list<char*> exeSQL2(string sql);
private:
   // MYSQL *mysql;          //连接mysql句柄指针
   // MYSQL_RES *result;    //指向查询结果的指针
   // MYSQL_ROW row;       //按行返回的查询信息
};

#endif //INDEXTSDB_DB_CONN_H
