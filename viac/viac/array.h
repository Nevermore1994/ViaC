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
void ArrayInit(Array* parr, const int size);
void ArrayRealloc(Array* parr, const int newsize);
void ArrayAdd(Array* parr, const void* data);
void ArrayFree(Array* parr);
int  ArrayFind(const Array* parr, const int data);
void ArrayDelete(Array* parr, const int i);

#endif