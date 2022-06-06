//
// Created by maomao on 2021/2/4.
//
#include "../gloabl.h"
#define KEYNUMBER 32
class Trie{
    bool is_string = false;
    Trie*next[KEYNUMBER] = {nullptr}; //孩子列表
    int typenum = 0;

public:
    Trie(){
    }

    ~Trie(){
        for (int i = 0; i < KEYNUMBER; i++) {
            if (next[i]== nullptr) continue;
            delete(next[i]);
            next[i] = nullptr;
        }
    }
    void insert(string word)
    {
        Trie *node = this;
        for (auto c :word) {
            if (node->next[c-'a'] == nullptr){
                node -> next[c- 'a']=new Trie();
            }
            node = node -> next[c-'a'];
        }
        node->is_string = true;
    }
    bool search(string word){
        Trie *node = this;
        for (auto c:word) {
            if (node->next[c-'a']== nullptr) return false;
            node = node->next[c-'a'];
        }
        return node->is_string;
    }
    bool startsWith(string prefix){
        Trie *node = this;
        for (auto c:prefix) {
            if (node->next[c-'a']== nullptr) return false;
            node = node->next[c-'a'];
        }
        return true;
    }
};
static void initializerkeyword(Trie *trie){

    string keywords[] ={"select","insert","delete","create","drop","as","group","by","order","show"};
    for(string c:keywords){
        trie->insert(c);
    }
}
int test(){
    bool flag;
    Trie *trie = new Trie();
    initializerkeyword(trie);
    flag = trie->search("select");
    cout<<flag<<endl;
    flag = trie->search("xxs");
    cout<<flag<<endl;
    flag = trie->search("s");
    cout<<flag<<endl;

    return 0;
}