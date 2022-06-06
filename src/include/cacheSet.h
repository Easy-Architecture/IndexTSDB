//
// Created by maomao on 2021/2/16.
//

#ifndef INDEXTSDB_CACHESET_H
#define INDEXTSDB_CACHESET_H
#include "gloabl.h"

typedef struct{
    char *tableId;
    char *key;
    char *value;
}SqlMessage;



typedef struct{
    char *tableId;
    int ttlTime;
}TTL;


#endif //INDEXTSDB_CACHESET_H
