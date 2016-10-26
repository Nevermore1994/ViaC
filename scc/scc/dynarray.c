#include"scc.h"

/*****************************************************
*函数名：Dysting_init(DynString* pstr, int initsize)
*参数：pstr为初始化的指针, initsize初始化的空间大小
*功能：初始化
*****************************************************/
void Dyarray_init(DynArray* parr, int size)
{
	if (parr != NULL)
	{
		parr->count = 0;
		parr->capacity = size;
		parr->data = (void**)malloc(sizeof(char) * size);
		if (parr->data == NULL)
		{
			//TODO
		}
	}
}
void Dynarray_realloc(DynArray* parr, int newsize)
{
	if (parr != NULL)
	{
		int capacity = parr->capacity;
		void** data;

		while (capacity < newsize)
		{
			capacity *= 2;
		}
		data = realloc(parr, capacity);
		if (data == NULL)
		{
			 //TODO:
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
		if (count > parr->capacity)
		{
			Dynarray_realloc(parr, count);
		}
		((void**)parr->data)[count - 1] = data;
		parr->count = count;
	}
}
void Dynarray_free(DynArray* parr);
void Dynarray_find(DynArray* parr, void* data);

