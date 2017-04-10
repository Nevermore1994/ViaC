#include"viac.h"

#ifndef		ARRAY_H_
#define 	ARRAY_H_

/************定义动态数组*******/
typedef struct Array
{
	int count;
	int capacity;
	void** data;
}Array;
/************动态数组函数*****/
void ArrayInit(Array* parr, const int size);				//动态数组初始化
void ArrayRealloc(Array* parr, const int newsize);			//动态数组内存重新分配
void ArrayAdd(Array* parr, const void* data);				//动态数组添加
void ArrayFree(Array* parr);								//动态数组的释放
int  ArrayFind(const Array* parr, const int data);			//动态数组查找
void ArrayDelete(Array* parr, const int i);					//动态数组删除

#endif