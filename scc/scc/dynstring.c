#include"scc.h"

/*****************************************************
*函数名：Dysting_init(DynString* pstr, int initsize)
*参数：pstr为初始化的指针, initsize初始化的空间大小
*功能：初始化动态字符串
*****************************************************/
void Dynstring_init(DynString* pstr, int initsize)
{
	if (pstr != NULL)
	{
		pstr->count = 0;
		pstr->capacity = initsize;
		pstr->data = (char*)malloc(sizeof(char) * initsize);
		if (pstr->data == NULL)
		{
			//TODO:抛出错误
		}
	}
}

/****************************************************
*函数名：Dynstring_free(DynString* pstr)
*参数：pstr释放指针
*功能：将动态字符串空间释放
****************************************************/
void Dynstring_free(DynString* pstr)
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
void Dynstring_realloc(DynString* pstr, int newsize)
{
	if (pstr != NULL)
	{
		int capacity =	pstr->capacity;
		char* data;									//中间变量指针
		
		while (capacity < newsize)
		{
			capacity *= 2;
		}
		data = (char*) realloc(pstr, capacity);
		if (data == NULL)
		{
			//TODO:抛出错误
		}
		pstr->data = data;
		pstr->capacity = capacity;
	}
}
/*************************************************
*函数名：Dynstring_chcat(DynString* pstr, int ch)
*参数：pstr为字符指针，ch为添加的字符
*功能：为动态字符串添加字符
**************************************************/
void Dynstring_chcat(DynString* pstr, int ch)
{
	if (pstr != NULL)
	{
		int count = pstr->count + 1;
		if (count > pstr->capacity)
		{
			Dynstring_realloc(pstr, count);
		}
		((char*) pstr->data)[count - 1] = ch;
		pstr->count = count;
	}
}

/*************************************************
*函数名：Dynstring_reset(DynString* pstr)
*参数：pstr为字符指针
*功能：重置字符指针
**************************************************/
void Dynstring_reset(DynString* pstr)
{
	if (pstr != NULL)
	{
		Dynstring_free(pstr);
		Dynstring_init(pstr, 8);
	}
}
