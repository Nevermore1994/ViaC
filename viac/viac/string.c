/******************************************
*Author:Away
*Date:2016-10-23
*Function:动态字符串模块代码
*******************************************/

#include"viac.h"


void DynStringInit(DynString* pstr, const int initsize)
{
	if (pstr != NULL)
	{
		pstr->data = (char*)malloc(sizeof(char) * initsize);
		if (pstr->data == NULL)
		{
			Error("内存分配错误!");
		}
		pstr->count = 0;
		pstr->capacity = initsize;
	}
}


void DynStringFree(DynString* pstr)
{
	if (pstr != NULL)
	{
		if(pstr->data)
			free(pstr->data);
		pstr->count = 0;
		pstr->capacity = 0;
		pstr->data = NULL;
	}
}


void DynStringRealloc(DynString* pstr, const int newsize)
{
	if (pstr != NULL)
	{
		int capacity =	pstr->capacity;
		char* data;									//中间变量指针
		
		while (capacity < newsize)
		{
			capacity *= 2;
		}
		data = (char*) realloc(pstr->data, capacity);
		if (data == NULL)
		{
			Error("内存分配错误!");
		}
		pstr->data = data;
		pstr->capacity = capacity;
	}
}

void DynStringChcat(DynString* pstr, const int ch)
{
	if (pstr != NULL)
	{
		int count = pstr->count + 1;
		if (count > pstr->capacity)
		{
			DynStringRealloc(pstr, count);
		}
		((char*) pstr->data)[count - 1] = ch;
		pstr->count = count;
	}
}


void DynStringReset(DynString* pstr)
{
	if (pstr != NULL)
	{
		DynStringFree(pstr);
		DynStringInit(pstr, 8);
	}
}
