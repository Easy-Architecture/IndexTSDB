//
// Created by maomao on 2021/2/9.
//

#ifndef INDEXTSDB_TOOLS_H
#define INDEXTSDB_TOOLS_H
#include "cstring"
#include "iostream"
#include <mutex>
#include <list>

using namespace std;

class User {
private:
    char *userid;
    char *username;
    char *password;
    User *next;
public:

    char *getUserid() const;

    void setUserid(char *userid);

    char *getUsername() const;

    void setUsername(char *username);

    char *getPassword() const;

    void setPassword(char *password);

    User *getNext() const;

    void setNext(User *next);

    User();

    User(char *userid, char *username, char *password);

    ~User();
};

class UserTable {
private:
    int hash(char *username, char *password);

public:
    User *users[100];
    mutex user_lock;

    list<char *> selectAllUsername();

    bool insertUser(User *user);

    bool deleteUser(char *username, char *password);

    bool containsUsername(char *newusername);

    User *selectUser(char *username, char *password);

    UserTable();

    ~UserTable();
};

#endif //INDEXTSDB_TOOLS_H
