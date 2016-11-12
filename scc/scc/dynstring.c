/******************************************
*Author:Away
*Date:2016-10-23
*Function:动态字符串模块代码
*******************************************/

#include"scc.h"

/*****************************************************
*函数名：Dysting_init(DynString* pstr, int initsize)
*参数：pstr为初始化的指针, initsize初始化的空间大小
*功能：初始化动态字符串
*****************************************************/
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

/****************************************************
*函数名：DynStringFree(DynString* pstr)
*参数：pstr释放指针
*功能：将动态字符串空间释放
****************************************************/
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

/*************************************************
*函数名：
*参数：
*功能：
**************************************************/
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
/*************************************************
*函数名：DynStringChcat(DynString* pstr, int ch)
*参数：pstr为字符指针，ch为添加的字符
*功能：为动态字符串添加字符
**************************************************/
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

/*************************************************
*函数名：DynStringReset(DynString* pstr)
*参数：pstr为字符指针
*功能：重置字符指针
**************************************************/
void DynStringReset(DynString* pstr)
{
	if (pstr != NULL)
	{
		DynStringFree(pstr);
		DynStringInit(pstr, 8);
	}
}
