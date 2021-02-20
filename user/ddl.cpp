//
// Created by maomao on 2021/2/9.
//

#include <string.h>
#include <string>
#include <mutex>
#include <list>
#include <time.h>
#include "Db_conn.h"
using namespace std;
//

mutex crud_lock;

Db_conn getDB_conn(){
    Db_conn db;
    db.initDB("127.0.0.1","mxd","@#MA847547125**","jb",3306);
    return db;
}

char* cache(char* databaseName){
    return databaseName;
}

char* addUser(char* username ,char* password1,char* password2){
    lock_guard<mutex> l(crud_lock);//自动锁
    int result=strcmp(password1,password2);
    if(result==0){
        string str="create user ";
        str.append(username).append("identified by").append(" '").append(password1).append("'");
        //todo port
        Db_conn db= getDB_conn();
        bool b = db.exeSQL(str);
        if(b){
            return (char* )"OK";
        }
    }
    return (char* ) "password error";

}
int deleteUser(char* username,char* password){
    lock_guard<mutex> l(crud_lock);//自动锁

    string str="drop user ";
    str.append(username);
    Db_conn db= getDB_conn();
    bool b = db.exeSQL(str);
    if(b){
        return 1;
    }
    return 0;
}
//返回数据库id -
char* createDatabase(char* databaseName){
    lock_guard<mutex> l(crud_lock);//自动锁
    string str="create database ";
    str.append(databaseName);
    //todo port
    Db_conn db= getDB_conn();
    bool b = db.exeSQL(str);
    if(b){
        return (char* )"create ok";
    }
    return (char* )"error";
}
//参数可以是数据库名字，也可以是库id ,待定
bool dropDataBase(char* databaseName){
    lock_guard<mutex> l(crud_lock);//自动锁
    string str="drop database ";
    str.append(databaseName);
    Db_conn db= getDB_conn();
    bool b = db.exeSQL(str);
    return b;
}
//
bool useDataBase(char* databaseName){
    lock_guard<mutex> l(crud_lock);//自动锁
    string str="use ";
    str.append(databaseName);
    Db_conn db= getDB_conn();
    bool b = db.exeSQL(str);
    if(b){
        cache(databaseName);
        //todo 落缓存接口给鑫磊
    }
    return b;
}


list<char* > showDatabases(){
    lock_guard<mutex> l(crud_lock);//自动锁
    string str="show databases";
    //todo
    Db_conn db= getDB_conn();
    list<char *> result = db.exeSQL2(str);
    return result;
}
//
list<char* > showTables(){
    lock_guard<mutex> l(crud_lock);//自动锁
    string str="show tables";
    //todo
    Db_conn db= getDB_conn();
    list<char *> result = db.exeSQL2(str);
    return result;
}
//加时间戳给姜楠 -
bool insert(char* tableName,char cols[]){
    lock_guard<mutex> l(crud_lock);//自动锁

    time_t now;
    char* unixTime = (char*)time(&now);

    string str="insert into ";
    str.append(tableName).append(" values (").append(cols).append(",").append(unixTime).append(")");
    Db_conn db= getDB_conn();
    bool result = db.exeSQL(str);
    return result;
}

bool dropTable(char* tableName){
    lock_guard<mutex> l(crud_lock);//自动锁
    string str="drop table ";
    str.append(tableName);
    Db_conn db= getDB_conn();
    bool result = db.exeSQL(str);
    return result;
}
