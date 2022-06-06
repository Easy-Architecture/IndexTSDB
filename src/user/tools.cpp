//
// Created by maomao on 2021/2/9.
//

#include "../../../IndexTSDB/src/include/tools.h"

int UserTable::hash(char *username, char *password) {
    int num = 0;
    for (int i = 0; username[i] != '\0'; ++i) {
        num += username[i];
    }
    for (int i = 0; password[i] != '\0'; ++i) {
        num += password[i];
    }
    return num % 100;
}

bool UserTable::insertUser(User *user) {
    bool isok;
    int home = this->hash(user->getUsername(), user->getPassword());
    user_lock.lock();
    User *temp = this->users[home];
    if (temp == nullptr) {
        this->users[home] = user;
        isok = true;
    } else {
        while (temp->getNext() != nullptr) {
            temp = temp->getNext();
        }
        temp->setNext(user);
        isok = true;
    }
    //cout<<users->users[home]->getUsername()<<"\t"<<users->users[home]->getPassword()<<endl;
    user_lock.unlock();
    return isok;
}

bool UserTable::deleteUser(char *username, char *password) {
    bool isok;
    int home = this->hash(username, password);
    user_lock.lock();
    User *temp = this->users[home];
    if (temp == nullptr) {
        isok = false;
    } else if (temp->getNext() == nullptr) {
        if (strcmp(temp->getUsername(), username) == 0 && strcmp(temp->getPassword(), password) == 0) {
            delete (temp);
            this->users[home] = nullptr;
            isok = true;
        }
    } else {
        while (temp->getNext() != nullptr) {
            User *p = temp->getNext();
            if (strcmp(temp->getUsername(), username) == 0 && strcmp(temp->getPassword(), password) == 0) {
                temp->setNext(p->getNext());
                delete (p);
                isok = true;
                break;
            }
            temp = temp->getNext();
        }
    }
    user_lock.unlock();
    return isok;
}

User *UserTable::selectUser(char *username, char *password) {
    User *user = nullptr;
    user_lock.lock();
    int home = hash(username, password);
    User *temp = this->users[home];
    if (temp == nullptr) {
        user = nullptr;
    } else if (temp->getNext() == nullptr) {
        //cout<<users->users[home]->getUsername()<<"\t"<<users->users[home]->getPassword()<<endl;
        //cout<<temp->getUsername()<<"\t"<<temp->getPassword()<<endl;
        if ((strcmp(temp->getUsername(), username) == 0) && (strcmp(temp->getPassword(), password) == 0))
            user = temp;
    } else {
        while (temp->getNext() != nullptr) {
            User *p = temp->getNext();
            if (strcmp(p->getUsername(), username) == 0 && strcmp(p->getPassword(), password) == 0) {
                user = p;
            }
            temp = p;
        }
    }
    user_lock.unlock();
    return user;
}

UserTable::UserTable() {}

UserTable::~UserTable() {
    for (int i = 0; i < 100; i++) {
        User *temp = users[i];
        User *next;
        if (temp != nullptr) {
            if (temp->getNext() != nullptr) {
                while (temp != nullptr) {
                    next = temp->getNext();
                    delete (temp);
                    temp = next;
                }
                users[i] = nullptr;
            } else {
                delete (temp);
                users[i] = nullptr;
            }
        }
    }
}

list<char *> UserTable::selectAllUsername() {
    list<char *> list;
    for (int i = 0; i < 100; i++) {
        User *temp = this->users[i];
        User *next;
        if (temp != nullptr) {
            if (temp->getNext() != nullptr) {
                while (temp != nullptr) {
                    next = temp->getNext();
                    list.push_back(temp->getUsername());
                    temp = next;
                }
            } else {
                list.push_back(temp->getUsername());
            }
        }
    }
    return list;
}
bool UserTable::containsUsername(char *newusername) {
    bool ishave = false;
    for (int i = 0; i < 100; i++) {

        User *temp = this->users[i];
        User *next;
        if (temp != nullptr) {
            if (temp->getNext() != nullptr) {
                while (temp != nullptr) {
                    next = temp->getNext();
                    if (!strcmp(temp->getUsername(), newusername)) {
                        ishave = true;
                        i = 100;
                        break;
                    }
                    temp = next;
                }
            } else {
                if (!strcmp(temp->getUsername(), newusername)) {
                    ishave = true;
                    break;
                }
            }
        }
    }
    return ishave;
}

char *User::getUserid() const {
    return userid;
}

void User::setUserid(char *userid) {
    User::userid = userid;
}

char *User::getUsername() const {
    return username;
}

void User::setUsername(char *username) {
    User::username = username;
}

char *User::getPassword() const {
    return password;
}

void User::setPassword(char *password) {
    User::password = password;
}

User *User::getNext() const {
    return next;
}

void User::setNext(User *next) {
    User::next = next;
}

User::User() {}

User::User(char *userid, char *username, char *password) {
    this->userid = userid;
    this->username = username;
    this->password = password;
}

User::~User() {
    this->userid = nullptr;
    this->username = nullptr;
    this->password = nullptr;
}

