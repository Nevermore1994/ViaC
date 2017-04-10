#include"viac.h"

#ifndef		STRING_H_
#define 	STRING_H_

/********���嶯̬�ַ���********/
typedef struct String
{
	int count;		//�ַ�������
	int capacity;	//�ַ���������
	char* data;		//�ַ�ָ��
}String;
/******��̬�ַ�������***********/
void StringInit(String* pstr, const int initsize);		//��ʼ����̬�ַ���
void StringFree(String* pstr);							//�ͷŶ�̬�ַ���
void StringReset(String* pstr);							//��̬�ַ���������
void StringChcat(String* pstr, const int ch);			//��̬�ַ�������
void StringRealloc(String* pstr, const int newsize);	//��̬�ַ�������ռ�


#endif