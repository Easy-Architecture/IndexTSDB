//
// Created by maomao on 2021/2/9.
//

#ifndef INDEXTSDB_OPERATIONUSER_H
#define INDEXTSDB_OPERATIONUSER_H
#include "iostream"
#include "cstring"
#include <list>

using namespace std;

#include "tools.h"
#include "sqlite3.h"

extern UserTable *users;
//extern UserTable users;

extern sqlite3 *db;

char *addUser(char *username, char *password1, char *password2);

char *login(char *username, char *password);

list<char *> showUsers();

void closeUser();

int deleteUser(char *username, char *password);

bool initUser();
#endif //INDEXTSDB_OPERATIONUSER_H
