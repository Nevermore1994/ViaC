#include"viac.h"

#ifndef		ERROR_H_
#define 	ERROR_H_

/***************错误处理*****************************/
enum e_ErrorLevel
{
	LEVEL_Warning,
	LEVEL_Error
};

enum e_WorkStage
{
	STAGE_COMPILE,
	STAGE_LINK
};

void  Warning(const char* fmt, ...);
void  Error(const char* fmt, ...);
void  Expect(const char* msg);
void  Skip(const int c);
void  LinkError(const char* fmt, ...);

/*****************附加函数****************/

/********************end*******************/



#endif
