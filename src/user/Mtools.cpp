//
// Created by maomao on 2021/2/9.
//

#include "../../../IndexTSDB/src/include/MTools.h"


map<string, string> powers;

map<string, string>::iterator pit;

sqlite3 *pdb;

static int callback(void *data, int args_num, char **argv, char **argc) {
    string key;
    string value;
    for (int i = 0; i < args_num; i++) {
        if (!strcmp(argc[i], "usertableid")) {
            if (argv[i] != nullptr) {
                key = argv[i];
            }
        }
        if (!strcmp(argc[i], "powermask")) {
            if (argv[i] != nullptr) {
                value = argv[i];
            }
        }
        if ((i + 1) % 2 == 0) {
            powers[key] = value;
        }
    }
    return 0;
}

bool initMms() {
    int rc = sqlite3_open("/data/testsql.db", &pdb);//连接sqlite数据库
    if (rc != SQLITE_OK) {//连接失败则返回初始化失败
        cout << "open sqlite3 fail." << endl;
        return false;
    }
    char *zErrMsg = nullptr;//接受exec函数返回的信息
    const char *data = "Users:";
    const char *sql = "select usertableid,powermask from powers";
    int ok = sqlite3_exec(pdb, sql, callback, (void *) data, &zErrMsg);//将sqlite中的用户表缓存到本地
    if (ok != 0) {//若执行未成功则返回初始化失败
        cout << "failed select:" << zErrMsg << endl;
        sqlite3_close(pdb);
        return false;
    }
    return true;
}

bool closeMms() {
    powers.clear();
    return true;
}

string setPower(int len) {
    string power;
    //TODO:关键点,暂时没有思路
    return power;
}

bool givePower(string userid, string tableid) {
    if (userid.empty() || userid.compare("")
        || tableid.empty() || tableid.compare("")) {
        return false;
    }
    string key = userid + tableid;
    if (powers.count(key)) {
        if (updatePower(userid, tableid))
            return true;
        else
            return false;
    }
    int tablelen = 0;//获取当前表的行数，或者时间戳，
    string value = setPower(tablelen);
    powers[key]=value;
    return true;
}

bool updatePower(string userid, string tableid) {
    if (userid.empty() || userid.compare("")
        || tableid.empty() || tableid.compare("")) {
        return false;
    }
    string key = userid + tableid;
    int tablelen = 0;//获取当前表的行数，或者时间戳，
    string value = setPower(tablelen);
    powers[key]=value;
    return true;
}
