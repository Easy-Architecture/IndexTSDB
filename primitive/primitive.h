//
// Created by maomao on 2021/2/9.
//

#ifndef INDEXTSDB_PRIMITIVE_H
#define INDEXTSDB_PRIMITIVE_H
#include <cstring>

class Map{ //表
public:
    List vals ;
};

class List{//一条数据
    Data* data;//当前字段数据
    Data* start;//数据起始字段
    Data* end;//数据结束字段
public:
    List* next;//下一条数据
    List* prev//上一条数据
    void appendData(Data* data){//追加数据
        Data* end = this->end
        data->prev = end;
        end->next = data;
        this->end = data;
    }

    Data* getData(){//获取当前字段
        return data
    }

    Data* getStart(){//获取起始字段
        return start
    }

    Data* getEnd(){//获取结束字段
        return end
    }
    void delList(){//删除当前数据
        prev->next = next;
        next->prev = prev;
        delete this
    }
};

class Data{//字段数据
public:
    string val;//值
    Data* next;//下一字段
    Data* prev;//上一字段

};

Map* filterByItem(Map* map, int item, string val){//item属性 val过滤值
    for(List* ptr = map->vals;ptr!= nullptr;ptr = ptr->next){//遍历数据集合
        Data* temp=ptr->getStart()//初始化指针位置
        for(int i =0;i<item;i++,temp=temp->next);//获取当前数据字段
        if(strcmp(temp->val, val)!=0){//判断是否符合过滤条件
            ptr->delList()//删除不满足条件数据
        }
    }
}
#endif //INDEXTSDB_PRIMITIVE_H
