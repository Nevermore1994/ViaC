#include"viac.h"

#ifndef		ARRAY_H_
#define 	ARRAY_H_

/************���嶯̬����*******/
typedef struct Array
{
	int count;
	int capacity;
	void** data;
}Array;
/************��̬���麯��*****/
void ArrayInit(Array* parr, const int size);				//��̬�����ʼ��
void ArrayRealloc(Array* parr, const int newsize);			//��̬�����ڴ����·���
void ArrayAdd(Array* parr, const void* data);				//��̬�������
void ArrayFree(Array* parr);								//��̬������ͷ�
int  ArrayFind(const Array* parr, const int data);			//��̬�������
void ArrayDelete(Array* parr, const int i);					//��̬����ɾ��

#endif