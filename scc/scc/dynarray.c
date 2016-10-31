#include"scc.h"

/*****************************************************
*函数名：DystingInit(DynString* pstr, int initsize)
*参数：pstr为初始化的指针, initsize初始化的空间大小
*功能：初始化
*****************************************************/
void Dynarray_init(DynArray* parr, const int size)
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

void Dynarray_realloc(DynArray* parr, const int newsize)
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
void Dynarray_add(DynArray* parr, void* data)
{
	if (parr != NULL)
	{
		int count = parr->count + 1;
		if (count*sizeof(void*) > parr->capacity)
		{
			Dynarray_realloc(parr, count * sizeof(void*) );
		}
		parr->data[count - 1] = data;
		parr->count = count;
	}
}
void Dynarray_free(DynArray* parr)
{
	if (parr != NULL)
	{
		void** p;
		for (p = parr->data; parr->data; ++p, --parr->count)
		{
			if (*p)
				free(*p);
		}
		free(parr->data);
		parr->data = NULL;
		parr->count = 0;
		parr->capacity = 0;
	}
}
int Dynarray_find(DynArray* parr, const int data)
{
	if (parr != NULL)
	{
		int** p = (int**)parr->data;
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

void Dynarray_delete(DynArray* parr, const int i)
{
	if (parr->data [i])
	{
		free(parr->data [i]);
	}
	memcpy(parr->data + i, parr->data + i + 1, sizeof(void*) * (parr->count - i - 1)); //i为下标
}