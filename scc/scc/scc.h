/******************************************
*author:Away
*date:2016-10-25
*	说		明
*函数名采用单词大写，函数名仅是首字母大写
*******************************************/

#ifndef SCC_H_
#define SCC_H_

#include<windows.h>
#include<stdio.h> 

/********定义动态字符串********/
typedef struct DynString
{
	int count;		//字符串长度
	int capacity;	//字符串缓冲区
	char* data;		//字符指针


}DynString;
/******动态字符串函数***********/
void Dynstring_init(DynString* pstr, int initsize);		//初始化动态字符串
void Dynstring_free(DynString* pstr);					//释放动态字符串
void Dynstring_reset(DynString* pstr);
void Dynstring_chcat(DynString* pstr, int ch);
void Dynstring_realloc(DynString* pstr, int newsize);


/************定义动态数组*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************动态数组函数*****/
void Dyarray_init(DynArray* parr, int size);
void Dynarray_realloc(DynArray* parr, int newsize);
void Dynarray_add(DynArray* parr, void* data);
void Dynarray_free(DynArray* parr);
void Dynarray_find(DynArray* parr, void* data);


#endif // !SCC_H_

