#include"scc.h"

/*****************************************************
*��������Dysting_init(DynString* pstr, int initsize)
*������pstrΪ��ʼ����ָ��, initsize��ʼ���Ŀռ��С
*���ܣ���ʼ����̬�ַ���
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
			error("�ڴ�������!");
		}
	}
}

/****************************************************
*��������Dynstring_free(DynString* pstr)
*������pstr�ͷ�ָ��
*���ܣ�����̬�ַ����ռ��ͷ�
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
*��������
*������
*���ܣ�
**************************************************/
void Dynstring_realloc(DynString* pstr, int newsize)
{
	if (pstr != NULL)
	{
		int capacity =	pstr->capacity;
		char* data;									//�м����ָ��
		
		while (capacity < newsize)
		{
			capacity *= 2;
		}
		data = (char*) realloc(pstr, capacity);
		if (data == NULL)
		{
			error("�ڴ�������!");
		}
		pstr->data = data;
		pstr->capacity = capacity;
	}
}
/*************************************************
*��������Dynstring_chcat(DynString* pstr, int ch)
*������pstrΪ�ַ�ָ�룬chΪ��ӵ��ַ�
*���ܣ�Ϊ��̬�ַ�������ַ�
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
*��������Dynstring_reset(DynString* pstr)
*������pstrΪ�ַ�ָ��
*���ܣ������ַ�ָ��
**************************************************/
void Dynstring_reset(DynString* pstr)
{
	if (pstr != NULL)
	{
		Dynstring_free(pstr);
		Dynstring_init(pstr, 8);
	}
}
