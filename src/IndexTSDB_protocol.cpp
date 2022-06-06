//
// Created by maomao on 2021/2/2.
//
#include "../gloabl.h"
#include "IndexTSDB_protocol.h"
//数组中有数据的值大于127 需要使用无符号类型的char
unsigned char commandCollect[] = {ADD_USER,DELETE_USER,UPDATE_USERPOWER,CREATE_DATABASE,DELETE_DATABASE,CREATE_TABLE
,DELETE_TABLE,DELETE_TABLE,SHOW_TABLE,SHOW_DATABASE,CHECK_DATABASE,CLEAR_CACHE,BATCH_DATA,SELECT_SQL,INSERT_SQL,DELETE_SQL
,REQUEST_CONNECT,RESPONSE_CONNECT,SUCCESS_CONNECT,REQUEST_PING,RESPONSE_PANG,DATA_CLOSE,AGAIN_CONNECT,FAIL_CONNECT
,END_FLAG};
int checkCommand(u8 cmd){
    int cmdIndex=0;
    u8 tmp_cmd = cmd ^ R_CODE
    while (commandCollect[cmdIndex]!=END_FLAG)
    {
        if (commandCollect[cmdIndex] == tmp_cmd)
        {
            printf("find command\n");
            return 1;
        }
        cmdIndex++;
    }
    printf("Not find command\n");
    return 0;
}




u16 Message_count_CRC_2(u8 CrcLenth,u8 *CRC_Data)
{
    u8  Crc_H = 0xFF;
    u8  Crc_L = 0xFF;
    u8  Index;
    while (CrcLenth--)
    {
        Index = Crc_H ^ *CRC_Data++;
        Crc_H = Crc_L ^  CRC_Array_H[Index];
        Crc_L = CRC_Array_L[Index];
    }
    return((Crc_H << 8) | Crc_L);
}


static void ThisFmtU8(u8 d){
    transferProtocol.MsgPayLoad[SendProtocolLength++]=d;
}
static void ThisFmtU16(u16 d){
    memcpy(&transferProtocol.MsgPayLoad[SendProtocolLength],&d,2);
    SendProtocolLength +=2;
}
static void ThisFmtU32(u32 d)
{
    memcpy(&transferProtocol.MsgPayLoad[SendProtocolLength],&d,4);
    SendProtocolLength +=4;
}
static void ThisFmtStr(char* str)
{
    memcpy(&transferProtocol.MsgPayLoad[SendProtocolLength],str,strlen(str));
    SendProtocolLength += strlen(str);
}
//命令判断
static void ThisFmCmd(u8 cmd)
{
    u16 tmp_cmd =cmd ^ R_CODE;
    transferProtocol.command = tmp_cmd;
}

static void ThisFmtCrc16()
{
    transferProtocol.crc16 = Message_count_CRC_2(SendProtocolLength,transferProtocol.MsgPayLoad) ^ R_CODE;
}
static void ThisFmtEnd()
{
    //总数据长度
    transferProtocol.msgLength = (SendProtocolLength+(sizeof (IndexProtocol)- SEND_BUFFER)) ^ R_CODE;
}
/*static void FmtSVersion()
{
    protocolBody.pVersion = transferProtocol.MsgPayLoad[0];
    SendProtocolLength ++;
}
static void FmtCVersion()
{
    protocolBody.cVersion = transferProtocol.MsgPayLoad[0];
    SendProtocolLength ++;
}
static void FmtSVersion(char *databaseName)
{
    protocolBody.name =databaseName;
    SendProtocolLength ++;
}*/
static u8* ThisGetBuffer()
{
    return (u8*)&transferProtocol;
}
static void ThisFmtHex(u8* Dt,u16 Len)
{
    memcpy(&transferProtocol.MsgPayLoad[SendProtocolLength],Dt,Len);
    SendProtocolLength += Len;
}
//得到数据包的总长度
static u16 ThisGetLength()
{
    return SendProtocolLength + (sizeof (IndexProtocol)- SEND_BUFFER);
}

//反序列化测试
//get object
ProtocolFormat_t * ProtocolFormatNew()
{
    //使用静态代表 每次获取的实例的对象都是一样的
    static ProtocolFormat_t SendProtocol;
    SendProtocol.FmtU8 = ThisFmtU8;
    SendProtocol.FmtU16 = ThisFmtU16;
    SendProtocol.FmtU32 = ThisFmtU32;
    SendProtocol.FmtHex = ThisFmtHex;
    SendProtocol.FmtHex = ThisFmtHex;
    SendProtocol.FmtStr = ThisFmtStr;
    SendProtocol.FmtEnd = ThisFmtEnd;
    SendProtocol.FmtCmd = ThisFmCmd;
    SendProtocol.FmtCrc16 = ThisFmtCrc16;
    SendProtocol.GetBuffer = ThisGetBuffer;
    SendProtocol.GetLength = ThisGetLength;
    return &SendProtocol;
}
//接收端对协议分析还原
IndexProtocol * Protocol_Analysis(void *d)
{
    IndexProtocol *p=(IndexProtocol*)d;
    p->command = p->command ^ R_CODE;
    p->msgLength = p->msgLength ^ R_CODE;
    ///TODO 判断是否需要增加长度字段
    //p->addLength = p->addLength ^ R_CODE;
    p->crc16 = p->crc16 ^ R_CODE;
    return p;
}
//测试
static u8 DisplayBuffer[128];
//序列化
void Serialize()
{
    u8 i = 0;
    protocolFormat->FmtCmd(0x10);
    protocolFormat->FmtU8(0x01);
    protocolFormat->FmtCrc16();
    protocolFormat->FmtEnd();
    memcpy(DisplayBuffer,protocolFormat->GetBuffer(),protocolFormat->GetLength());
    for(i=0;i<protocolFormat->GetLength();i++)
        printf("%02x ",DisplayBuffer[i]);
    printf("\n");
}


//反序列化

void DecodeTestHandler(void* p,u16 length)
{
    u8 *Data = (u8*)p;
    u16 indexof = 0;
    u8 i =0;
    if(length > 6)
    {
        while( (length-indexof) >= 7)//解包
        {
            IndexProtocol *p = Protocol_Analysis(&Data[indexof]);
            printf("cmd=0x%02x,crc16=0x%02x,len=0x%02x\n",p->command,p->crc16,p->msgLength);
            printf("Data: ");
            for(i=0;i<p->msgLength-6;i++)
            {
                printf("%02X ",p->MsgPayLoad[i]);
            }
            indexof += p->msgLength;//下一包数据

        }
    }
}

//反序列化测试
static u8 DecodeTestBuffer[7] = {0xb5,0xb4,0xf6,0x90,0x71,0x7e,0x01};



void TestProtocol()
{
    protocolFormat= ProtocolFormatNew();
    Serialize();
    DecodeTestHandler(DecodeTestBuffer,7);
}


int DDLCommand(u8 command){
    switch (command) {
        case ADD_USER:
            break;
        case DELETE_USER:
            break;
        case UPDATE_USERPOWER:
            break;
        case CREATE_DATABASE:
            break;
        case DELETE_DATABASE:
            break;
        case CREATE_TABLE:
            break;
        case DELETE_TABLE:
            break;
        case SHOW_TABLE:
            break;
        case SHOW_DATABASE:
            break;
        case CHECK_DATABASE:
            break;
        case CLEAR_CACHE:
            break;
        default:
            return 0;
            break;
    }
}
int DMLCommand(u8 command){
    switch (command) {
        case BATCH_DATA: //批处理数据指令
            break;
        case SELECT_SQL:
            selectSql("select * from tableName where name = 'xx' ");
            break;
        case INSERT_SQL:
            insertSql("insert into tableName values(v,v,v,v,v)");
            break;
        case DELETE_SQL:
            deleteSql("delete from tableName where x=x ");
            break;
        default:
            return 0;
            break;
    }
}
int CliCommand(u8 command){
    switch (command) {
        case REQUEST_CONNECT:
            break;
        case RESPONSE_CONNECT:
            break;
        case SUCCESS_CONNECT:
            break;
        case REQUEST_PING:
            break;
        case RESPONSE_PANG:
            break;
        case DATA_CLOSE:
            break;
        case AGAIN_CONNECT:
            break;
        default:
            return 0;
            break;
    }
}

//判断命令类型
int handleCommand( IndexProtocol *p) {

    u8 cmdType = p->command ^ C_Code; //通过异或判断是哪种类型的命令
    ///TODO命令判断
    switch (cmdType) {
        case IsDDL:
            if(!DDLCommand(cmdType)){
                return 0;
            }
            return 1;
            break;
        case IsDML:
            if (!DMLCommand(cmdType)){
                return 0;
            }
            return 2;
            break;
        case IsCli:
            if (!CliCommand(cmdType)){
                return 0;
            }
            return 3;
            break;
        default:
            return 0;
            break;
    }

}
int ProtocolProcessing(char sendBuffer[])
{
    u8 *Data = (u8*)sendBuffer;
    printf("协议开始处理\n");
    u16 protocolIndex = 0;
    //进行异或解密
    IndexProtocol *p = Protocol_Analysis(&sendBuffer[protocolIndex]);
    protocolIndex++;
    handleCommand(p);

}
bool insertSql(char* sql){

    return false;
}

bool deleteSql(char*sql){

    return false;
}

char* selectSql(char*sql){

    return " ";
}