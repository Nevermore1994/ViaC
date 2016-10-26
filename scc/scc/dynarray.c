#include"scc.h"

/*****************************************************
*��������Dysting_init(DynString* pstr, int initsize)
*������pstrΪ��ʼ����ָ��, initsize��ʼ���Ŀռ��С
*���ܣ���ʼ��
*****************************************************/
void Dynarray_init(DynArray* parr, int size)
{
	if (parr != NULL)
	{
		parr->count = 0;
		parr->capacity = size;
		parr->data = (void**)malloc(sizeof(int) * size);
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
void Dynarray_free(DynArray* parr)
{
	if (parr != NULL)
	{
		if (parr->data)
			free(parr->data);
		parr->data = NULL;
		parr->count = 0;
		parr->capacity = 0;
	}
}
int Dynarray_find(DynArray* parr, int data)
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

