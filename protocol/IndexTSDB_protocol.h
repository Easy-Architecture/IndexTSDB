//
// Created by maomao on 2021/2/2.
//

#ifndef INDEXTSDB_INDEXTSDB_PROTOCOL_H
#define INDEXTSDB_INDEXTSDB_PROTOCOL_H
//DDL
#define ADD_USER 0x00
#define DELETE_USER 0x01
#define UPDATE_USERPOWER  0x02 //修改用户权限
#define CREATE_DATABASE 0x03
#define DELETE_DATABASE 0x04
#define CREATE_TABLE 0x05
#define DELETE_TABLE 0x06
#define SHOW_TABLE 0x07
#define SHOW_DATABASE 0x08
#define CHECK_DATABASE 0x09
#define CLEAR_CACHE 0x0a
//DML
#define BATCH_DATA 0x40  //批处理数据
#define SELECT_SQL 0x41
#define INSERT_SQL 0x42
#define DELETE_SQL 0x43
//server/client
#define REQUEST_CONNECT 0x80 //请求连接
#define RESPONSE_CONNECT 0x81 //请求响应
#define SUCCESS_CONNECT 0x82 //连接成功
#define REQUEST_PING 0x84 //ping请求
#define RESPONSE_PANG 0x85 //pang响应
#define DATA_CLOSE  0x87//数据传送完毕关闭连接
#define AGAIN_CONNECT 0x88 //重新连接 发送数据
#define FAIL_CONNECT 0x89 //连接失败
//result
#define DDL_SUCCESSFUL 0xc0 //DDL成功响应
#define DML_SUCCESSFUL 0xc1 //DML成功响应
#define DDL_FAIL 0xc2 //DDL响应失败
#define DML_FAIL 0xc3 //DML 响应失败
#define DDL_MESSAGE 0xc4 //DDL成功响应并返回信息
#define DML_MESSAGE 0xc5 //成功响应并返回信息

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#endif //INDEXTSDB_INDEXTSDB_PROTOCOL_H

struct package_head{
    u8 command; //命令
    u8 msgLength; //消息长度
    char *addLength; //增加的地址
    u16 crc16; //完整性校验
};
struct package_body_DDL{
    u8 cVersion; //缓存版本
    char name;
}ddl;
struct package_body_DML{
    u8 cVersion; //缓存版本
    char name;
    char* buf[1024];//接收数据缓冲区
}dml;
struct package_body_SERVER{
    u8 pVersion; //协议版本
    u8 code; //字符编码
    int heartTime; //心跳时间
}ser;
struct package_body_CLIENT{
    u8 pVersion; //协议版本
    u8 code;//字符编码
    char *userName;
    char *password;
}cli;
struct package_body_PINGPANG{
    u8 pVersion; //协议版本
    char result; //响应结果
    char message[1024]; //携带信息
}ping_pang;
struct package_body_RESULT{
    u8 result; //结果命令
    char name;
    char buf[1024];
}result;

typedef struct
{
    void (*FmtU8)(u8 d);
    void (*FmtU16)(u16 d);
    void (*FmtU32)(u32 d);
    void (*FmtStr)(char* str);
    void (*FmtHex)(u8* Dt,u16 len);
    void (*FmtCmd)(u8 cmd);
    void (*FmtCrc16)(void);
    u8* (*GetBuffer)(void);
    u16 (*GetLength)(void);
}ProtocolFormat_t;

bool insertSql(char* sql);
bool deleteSql(char *sql);
char* selectSql(char*sql);