#include"viac.h"

#ifndef		STRING_H_
#define 	STRING_H_

/********¶¨Òå¶¯Ì¬×Ö·û´®********/
typedef struct String
{
	int count;		//×Ö·û´®³¤¶È
	int capacity;	//×Ö·û´®»º³åÇø
	char* data;		//×Ö·ûÖ¸Õë
}String;
/******¶¯Ì¬×Ö·û´®º¯Êý***********/
void StringInit(String* pstr, const int initsize);		//³õÊ¼»¯¶¯Ì¬×Ö·û´®
void StringFree(String* pstr);					//ÊÍ·Å¶¯Ì¬×Ö·û´®
void StringReset(String* pstr);
void StringChcat(String* pstr, const int ch);
void StringRealloc(String* pstr, const int newsize);


#endif