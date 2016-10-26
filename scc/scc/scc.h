/******************************************
*author:Away
*date:2016-10-25
*	˵		��
*���������õ��ʴ�д����������������ĸ��д
*******************************************/

#ifndef SCC_H_
#define SCC_H_

#include<windows.h>
#include<stdio.h> 

/********���嶯̬�ַ���********/
typedef struct DynString
{
	int count;		//�ַ�������
	int capacity;	//�ַ���������
	char* data;		//�ַ�ָ��


}DynString;
/******��̬�ַ�������***********/
void Dynstring_init(DynString* pstr, int initsize);		//��ʼ����̬�ַ���
void Dynstring_free(DynString* pstr);					//�ͷŶ�̬�ַ���
void Dynstring_reset(DynString* pstr);
void Dynstring_chcat(DynString* pstr, int ch);
void Dynstring_realloc(DynString* pstr, int newsize);


/************���嶯̬����*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************��̬���麯��*****/
void Dynarray_init(DynArray* parr, int size);
void Dynarray_realloc(DynArray* parr, int newsize);
void Dynarray_add(DynArray* parr, void* data);
void Dynarray_free(DynArray* parr);
int Dynarray_find(DynArray* parr, int data);


/***********���ʱ�***********/
typedef struct TkWord
{
	int tkcode;
	struct TkWord* next;
	char* spelling;
	struct Symol* sym_struct;
	struct Symol* sym_id;
}TkWord;


#define MAXKEY 2048 
s
int elf_hash(char* key);

#endif // !SCC_H_

