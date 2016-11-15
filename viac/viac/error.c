/******************************************
*Author:Away
*Date:2016-11-8
*Function:异常模块单元代码
*******************************************/
#include"viac.h"

DynString tkstr;
#define BUF_SIZE 1024
#define ERR_SIZE 2048
void* HandleException(const int stage, const int level, char* fmt, const va_list ap)
{
	char buf[BUF_SIZE];
	char errstr[ERR_SIZE];
	vsprintf_s(buf, BUF_SIZE,fmt, ap);
	if (stage == STAGE_COMPILE)
	{
		if (level == LEVEL_WARNING)
		{
			printf("%s(第%d行):编译警告：%s\n:", filename, linenum, buf);
			sprintf_s(errstr, ERR_SIZE, "%s(第%d行):编译警告：%s\n:", filename, linenum, buf);
		}
		else
		{
			printf("%s(第%d行:编译错误:%s\n", filename, linenum, buf);
			sprintf_s(errstr, ERR_SIZE, "%s(第%d行:编译错误:%s\n", filename, linenum, buf);
			Cleanup();
			exit(-1);
		}
	}
	else
	{
		printf("链接错误:%s\n", buf);
		sprintf_s(errstr, ERR_SIZE, "链接错误:%s\n", buf);
		Cleanup();
		exit(-1);
	}
}

void Warning(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_COMPILE, LEVEL_WARNING, fmt, ap);
	va_end(ap);
}

void Error(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_COMPILE, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
 
void Skip( const int c)
{
	if (token != c)
	{
		Error("缺少'%s'", GetTkstr(c));
	}
	GetToken();
}
/*******************************************
*函数功能:提示缺少的成分
*msg:需要什么语法成分
*******************************************/
void Expect(const char* msg)
{
	Error("缺少%s", msg);
}

void LinkError(const char* fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_LINK, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
