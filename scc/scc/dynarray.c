/******************************************
*Author:Away
*Date:2016-10-26
*Function:动态数组模块单元代码
*******************************************/

#include"scc.h"

/*****************************************************
*函数名：DystingInit(DynString* pstr, const int initsize)
*参数：pstr为初始化的指针, initsize初始化的空间大小
*功能：初始化
*****************************************************/
void DynArrayInit(DynArray* parr, const int size)
{
	if (parr != NULL)
	{
		parr->count = 0;
		parr->capacity = size;
		parr->data = (void**)malloc(sizeof(void*) * size);
		if (parr->data == NULL)
		{
			Error("内存分配错误!");
		}
	}
}

void DynArrayRealloc(DynArray* parr, const int newsize)
{
	if (parr != NULL)
	{
		int capacity = parr->capacity;
		void* data;

		while (capacity < newsize)
		{
			capacity *= 2;
		}
		data = realloc(parr->data, capacity);
		if (data == NULL)
		{
			Error("内存分配错误!");
		}
		parr->data = data;
		parr->capacity = capacity;
	}
}
void DynArrayAdd(DynArray* parr, void* data)
{
	if (parr != NULL)
	{
		const int count = parr->count + 1;
		if (count*sizeof(void*) > parr->capacity)
		{
			DynArrayRealloc(parr, count * sizeof(void*) );
		}
		parr->data[count - 1] = data;
		parr->count = count;
	}
}
void DynArrayFree(DynArray* parr)
{
	if (parr != NULL)
	{
		void** p;
		for (p = parr->data; parr->count; ++p, --parr->count)
		{
			if (*p)
				free(*p);
		}
		free(parr->data);
		parr->data = NULL;
		parr->capacity = 0;
	}
}
int DynArrayFind(DynArray* parr, const int data)
{
	if (parr != NULL)
	{
		const int** p = (const int**)parr->data;
		for (size_t i = 0; i < parr->count; ++i,++p)
		{
			if (  **p == data)
			{
				return i;
			}
		}
	}
	return -1;
}

void DynArrayDelete(DynArray* parr, const int i)
{
	if (parr->data [i])
	{
		free(parr->data [i]);
	}
	memcpy(parr->data + i, parr->data + i + 1, sizeof(void*) * (parr->count - i - 1)); //i为下标
}