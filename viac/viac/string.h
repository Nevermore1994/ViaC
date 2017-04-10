#include"viac.h"

#ifndef		STRING_H_
#define 	STRING_H_

/********定义动态字符串********/
typedef struct String
{
	int count;		//字符串长度
	int capacity;	//字符串缓冲区
	char* data;		//字符指针
}String;
/******动态字符串函数***********/
void StringInit(String* pstr, const int initsize);		//初始化动态字符串
void StringFree(String* pstr);							//释放动态字符串
void StringReset(String* pstr);							//动态字符串的重置
void StringChcat(String* pstr, const int ch);			//动态字符串连接
void StringRealloc(String* pstr, const int newsize);	//动态字符串分配空间


#endif