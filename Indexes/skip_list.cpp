//
// Created by maomao on 2021/2/9.
//


#include "skip_list.h"

node *height[MAX_LEVEL];

/**
 * 获取一个随机数
 */
int get_current_level() {
    int k = 0;
    while (rand() % 2) ++k;
    return k;
}

/**
 * 初始化调表
 */
skip_list *skip_list_init(int tableId) {
    srand((unsigned) time(NULL));   //每次运行都会产生一个随机数序列
    skip_list *sl;
    sl = new skip_list();
    tableMap.insert(pair<int, skip_list *>(tableId,sl));    //把skip list放进map中
    sl->tID = tableId;
    sl->max_level = 0;
    sl->header = new data_node *[MAX_LEVEL];
    for (int i = 0; i < MAX_LEVEL; ++i) {   //定义跳表中每层的头节点
        data_node *t = new data_node();
        t->key = -10;
        t->right = NULL;
        sl->header[i] = t;
    }
    for (int i = MAX_LEVEL - 1; i; --i) {   //将每个头结点连接起来
        sl->header[i]->down = sl->header[i - 1];
    }
    return sl;
}

/**
 * 查找
 * @param sl 所查找的跳表
 * @param x 查找的值
 * @return  data_note *
 */
data_node *find_x_from_skip_list(skip_list *sl, int x) {
    data_node *h = sl->header[sl->max_level];     //最高层的头结点
    while (h) {
        if (h->key == x && h->down == NULL) {
            return h;
        } else {
            if (x >= h->key && (h->right == NULL || h->right->key > x)) {
                h = h->down;
#ifdef DEBUG
                cout<<"d-";
#endif
            } else {
                if (x < h->key) {
#ifdef DEBUG
                    cout<<"return imediately"<<endl;
#endif
                    return NULL;
                } else {
                    h = h->right;
#ifdef DEBUG
                    cout<<"r-";
#endif
                }
            }
        }
    }
#ifdef DEBUG
    cout<<endl;
#endif
    return h;
}

/**
 * 把数据插入到节点中
 * @param head 插入节点
 * @param x
 * @return
 */
data_node *insert_x_into_list(node *head, int x, char * data) {

    node *prev = NULL;         //上一个节点
    while (head && x > head->key) {   //判断所要插入到节点在当前层中的那个位置
        prev = head;
        head = head->right;
    }

    if (prev != NULL && prev->right != NULL && prev->right->key == x) {   //判断当前层中是否有这个节点的值
        prev->right->list.push_back(data);
        return prev->right;
    }

    node *n = new node;
    n->key = x;
    //n->data = data;
    n->list.push_back(data);
    n->right = NULL;
    n->down = NULL;
    if (prev == NULL) {
        if (head) {
            n->right = head->right;
        } else {
            head = n;
        }
    } else {
        prev->right = n;
        n->right = head;
    }
    return n;
}

/** 1、
 *  插入跳表得值
 * @param sl
 * @param x
 * @return
 */
int insert_x_into_skip_list(skip_list *sl, int x, char *data) {
    int current_level = get_current_level();    //获得一个随机层
    if (current_level > sl->max_level) {    //获得的随机层大于当前跳表中的最大 就把最大层数跟新
        sl->max_level = current_level;
    }
#ifdef DEBUG
    cout<<"current_level "<<current_level<<" insert "<<x<<endl;
#endif
    for (int i = current_level; i >= 0; --i) {      //把所要插入key和value插入随机层
        height[i] = insert_x_into_list(sl->header[i], x, data);
    }
    for (int i = current_level; i; --i) {   //将当前插入所有的节点连接起来
        height[i]->down = height[i - 1];
#ifdef DEBUG
        cout<<"connect down "<<height[i]->key<<" "<<endl;
#endif
    }
    ++sl->size;
    return 0;
}

//int insert_data_into_skip_list()


/**
 * 删除跳表
 * @param head
 * @param x
 * @return
 */
int remove_data_from_list(node *head, int x) {
    node *prev = NULL;
    node *cur = head;
    while (cur) {
        if (cur->key == x) {
            if (prev) {
                prev->right = cur->right;
                delete cur;
                cur = prev->right;
            } else {
                prev = cur;
                delete cur;
                cur = prev->right;
                prev = NULL;
            }
        } else {
            prev = cur;
            cur = cur->right;
        }
    }
    return 0;
}

/**
 * 删除跳表
 * @param sl
 * @param x
 * @return
 */
int remove_x_from_skip_list(skip_list *sl, int key) {
    for (int i = 0; i <= sl->max_level; ++i) {
        remove_data_from_list(sl->header[i], key);
    }
    return 0;
}

/**
 * 输出跳表
 * @param sl
 * @return
 */
int print_list(skip_list *sl) {
    for (int i = sl->max_level; i >= 0; i--) {
        data_node *current = sl->header[i];
        while (current) {
            cout << current->key << "  ";
            current = current->right;
        }
        cout << endl;
    }
    return 0;
}

/**
 * 返回list中所有的元素
 * @param list
 * @param name
 */
void put_CharList(CHARLIST list) {
    CHARLIST::iterator plist;
    for (plist = list.begin(); plist != list.end(); plist++) {
        cout << *plist << " ";
    }
    cout << endl;
}

/**
 * 从map中查找调表
 * @param dataID
 * @return
 */
skip_list *all_data(int dataID) {
    map<int, skip_list *>::iterator iter;
    iter= tableMap.find(dataID);
    if(iter!=tableMap.end())
        return iter->second;
    return NULL;
}