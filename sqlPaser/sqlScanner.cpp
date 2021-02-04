//
// Created by maomao on 2021/1/18.
//
#include "../gloabl.h"
#define BUF_LEN 1024
#define _END_ "END"
/*
 * tpyenum表示数据类型
 * -1 ERROR
 * 1 select
 * 2 insert
 * 3 delete
 * 4 from
 * 5 into
 * 6 where
 * 7 group
 * 8 by
 * 9 between
 * 10 and
 * 11 or
 * 12 primary
 * 13 like
 * 14 values
 * 100 letter
 * 150 >=
 * 160 <=
 * 180 !=
 * 200 number
 * 38 &
 * 40 (
 * 41 )
 * 42 *
 * 44 ,
 * 45 -
 * 46 .
 * 58 :
 * 59 ;
 * 60 <
 * 61 =
 * 62 >
 * 127 |
 * 1000 \0
 * */
typedef struct{
    int typenum;
    char *word;
}sqlWord;
char *keywords[] = {"select","insert","delete","from","into","where","group","by","between","and","or","primary","like","values",_END_};
char input[BUF_LEN];
char token[255] = "";
int p_input;//指针
int p_token;
char ch;
sqlWord  *scanner();
void sqlsacnner(char*sqlsource)
{
    int over =1;
    sqlWord *oneword=new sqlWord;
    strcpy(input,sqlsource);
    //printf("%s",input);
    int range=sizeof(sqlsource);
    printf("range:%d\n",range);
    while (range)
    {
        p_input = 0;
       printf("your words:\n%s\n",input);
        while (over<1000&&over !=-1)
        {
            oneword =scanner();
            if (oneword -> typenum <1000)
            {
                cout<<"["<<oneword->typenum<<"\t"<<oneword->word<<"]"<<endl;
            }
            over = oneword->typenum;

        }
        range=0;
    }


}
//从缓冲区读取一个字符道ch
char m_getch()
{
    ch=input[p_input];
    p_input++;
    return ch;
}
//去掉空白符号
void removeSpace()
{
    while(ch == ' '||ch ==32)
    {
        ch=input[p_input];
        p_input++;
    }
}
//回退一个字符
void retract()
{
    p_input--;
}
//拼接单词
void conTokens(){
    token[p_token] = ch;
    p_token++;
    token[p_token] = '\0';
}
//判断是否为字母
int letter()
{
    if (ch >='a' && ch <='z'||ch >='A'&&ch<='Z')
    {
        return 1;
    }
    else
    {
        return 0;
    }

}
//判断是否为数字
int digit()
{
    if (ch>='0'&&ch<='9')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int reserver()
{
    int i = 0;
    //相同为0 表示读到末尾
    while (strcmp(keywords[i],_END_))
    {
        if (!strcmp(keywords[i],token))
            return i+1;
        i++;
    }
    return 100; //不是关键字 普通字段

}

sqlWord *scanner()
{
    sqlWord *myword=new sqlWord ;
    myword->typenum = 0;
    myword->word = " ";
    p_token = 0; //单词缓冲区
    m_getch();
    removeSpace();
    if (letter()) //判断读取到首字母是字母
    {
        while (letter()||digit())
        {
            conTokens();
            m_getch();
        }
        retract();//回退一个字符
        myword->typenum = reserver();
        myword->word = token;
        return myword;
    }else if (digit()) //判断是不是数字
    {
        while (digit()) //所有数字连接起来
        {
            conTokens();
            m_getch();
        }
        retract();
        myword->typenum = 200;
        myword->word = token;
        return myword;
    }
    else
        switch(ch){
            case '<':
                m_getch();
                if (ch == '=')
                {
                    myword->typenum=150;
                    myword->word="<=";
                    return myword;
                }
                retract();//读取到的下个字符不是=,则要回退
                myword->typenum = 60;
                myword->word = "<";
                return myword;
                break;
            case '>':
                m_getch();
                if (ch == '=')
                {
                    myword->typenum=150;
                    myword->word=">=";
                    return myword;
                }
                retract();//读取到的下个字符不是=,则要回退
                myword->typenum = 62;
                myword->word = ">";
                return myword;
                break;
            case '=':
                myword->typenum = 61;
                myword->word = "=";
                return myword;
                break;

            case '!':
                m_getch();
                if (ch == '=')
                {
                    myword->typenum = 180;
                    myword->word = "!=";
                    return myword;
                    break;
                }
                retract();
                myword->typenum = -1;
                myword->word = "ERROR";
                return myword;
                break;

            case '&':
                myword->typenum = 38;
                myword->word = "&";
                return myword;
                break;
            case '(':
                myword->typenum = 40;
                myword->word = "(";
                return myword;
                break;
            case ')':
                myword->typenum = 41;
                myword->word = ")";
                return myword;
                break;
            case  '*':
                myword->typenum = 42;
                myword->word = "*";
                return myword;
                break;
            case ';':
                myword->typenum = 59;
                myword->word = ";";
                return myword;
                break;
            case ',':
                myword->typenum = 44;
                myword->word = ",";
                return myword;
                break;
            case '.':
                myword->typenum = 46;
                myword->word = ".";
                return myword;
                break;
            case '|':
                myword->typenum = 127;
                myword->word = "|";
                return myword;
                break;
            case '-':
                myword->typenum = 45;
                myword->word = "-";
                return myword;
                break;
            case ':':
                myword->typenum = 58;
                myword->word = ":";
                return myword;
                break;
            case '\0':
                myword->typenum = 1000;
                myword->word = "OVER";
                return myword;
                break;
            default:
                myword->typenum = -1;
                myword->word = "ERROR";
                return myword;
                break;
        }
}
