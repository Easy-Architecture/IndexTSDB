//
// Created by maomao on 2021/2/9.
//

#include "operationUser.h"

UserTable *users = nullptr;

sqlite3 *db;

string insertsql(string username, string password) {
    string sql = "insert into users(username,password) values('" + username + "','" + password + "');";
    return sql;
}

string selectsql(string username, string password) {
    string sql = "select * from users where username='" + username + "' and password='" + password + "';";
    return sql;
}

string deletesql(string username, string password) {
    string sql = "delete from users where username='" + username + "' and password='" + password + "';";
    return sql;
}

static int callback(void *data, int args_num, char **argv, char **argc) {
    User *user = new User;
    for (int i = 0; i < args_num; i++) {
        if (!strcmp(argc[i], "userid")) {
            if (argv[i] != nullptr) {
                cout << argv[i] << endl;
                user->setUserid(argv[i]);
            }
        }
        if (!strcmp(argc[i], "username")) {
            if (argv[i] != nullptr) {
                cout << argv[i] << endl;
                user->setUsername(argv[i]);
            }
        }
        if (!strcmp(argc[i], "password")) {
            if (argv[i] != nullptr) {
                cout << argv[i] << endl;
                user->setPassword(argv[i]);
            }
        }
        if ((i + 1) % 3 == 0) {
            users->insertUser(user);
            if (args_num != i + 1)
                user = new User;
        }
    }
    return 0;
}

bool initUser() {
    bool isinit = false;
    users = new UserTable();
    int rc = sqlite3_open("/data/testsql.db", &db);//连接sqlite数据库
    if (rc != SQLITE_OK) {//连接失败则返回初始化失败
        cout << "open sqlite3 fail." << endl;
        return false;
    }
    char *zErrMsg = nullptr;//接受exec函数返回的信息
    const char *data = "Users:";
    const char *sql = "select * from users";
    int ok = sqlite3_exec(db, sql, callback, (void *) data, &zErrMsg);//将sqlite中的用户表缓存到本地
    if (ok != 0) {//若执行未成功则返回初始化失败
        cout << "failed select:" << zErrMsg << endl;
        sqlite3_close(db);
        delete (users);
        return false;
    }
    isinit = true;
    return isinit;
}


char *addUser(char *username, char *password1, char *password2) {
    if (username == nullptr || password1 == nullptr || password2 == nullptr ||
        !strcmp(username, "") || !strcmp(password1, "") || !strcmp(password2, "")) {//判断字符串是否为空
        return nullptr;
    }
    if (!!strcmp(password1, password2)) {//判断两次密码相不相同
        return nullptr;
    }
    if (users->containsUsername(username)) {//判断是否重名
        return nullptr;
    }

    char *error_msg = nullptr;//sqlite报错提示

    string sql = insertsql(username, password1);//拼sql语句
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_msg);//执行sql语句
    if (rc != SQLITE_OK) {//若执行未成功则输出错误并且返回null，TODO:后期可以改为log
        cout << "Insert failed! Error msg: " << error_msg << endl;
        sqlite3_free(error_msg);
        return nullptr;
    }

    char *userid = nullptr;
    sql = selectsql(username, password1);//拼sql语句
    rc = sqlite3_exec(db, sql.c_str(), callback, (void *) users, &error_msg);//若添加成功则查询添加用户的id，并且返回
    if (rc != SQLITE_OK) {
        cout << "Select failed! Error msg: " << error_msg << endl;
        sqlite3_free(error_msg);
        return nullptr;
    }
    User *user = users->selectUser(username, password1);
    if (user != nullptr) {
        userid = user->getUserid();//获得加入缓存中的userid，并且返回
    }
    return userid;
}

char *login(char *username, char *password) {
    if (username == nullptr || password == nullptr ||
        !strcmp(username, "") || !strcmp(password, "")) {//判断字符串是否为空
        return nullptr;
    }
    User *user = users->selectUser(username, password);//查询，查询到则返回id
    if (user == nullptr) {
        return nullptr;
    }
    return user->getUserid();
}

list<char *> showUsers() {
    return users->selectAllUsername();//查询缓存中所有用户并返回用户名
}

int deleteUser(char *username, char *password) {
    if (username == nullptr || password == nullptr ||
        !strcmp(username, "") || !strcmp(password, "")) {//判断字符串是否为空
        return false;
    }

    char *error_msg = nullptr;//sqlite报错提示
    string sql = deletesql(username, password);
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &error_msg);//执行sql语句
    if (rc != SQLITE_OK) {//若执行未成功则输出错误并且返回null，TODO:后期可以改为log
        cout << "Delete failed! Error msg: " << error_msg << endl;
        sqlite3_free(error_msg);
        return false;
    }

    return users->deleteUser(username, password);
}

void closeUser() {
    delete (users);//清楚用户表本地缓存
    sqlite3_close(db);//关闭数据库
}

