#include"scc.h"

DynString tkstr;

 
void HandleException(int stage, int level, char* fmt, va_list ap)
{
	char buf[1024];
	vsprintf_s(buf, 1024,fmt, ap);
	if (stage == STAGE_COMPILE)
	{
		if (level == LEVEL_WARNING)
			printf("%s(第%d行):编译警告：%s\n:", filename, line_num, buf);
		else
		{
			printf("%s(第%d行:编译错误:%s\n", filename, line_num, buf);
			exit(-1);
		}
	}
	else
	{
		printf("链接错误:%s\n", buf);
		exit(-1);
	}
}

void Warning(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_COMPILE, LEVEL_WARNING, fmt, ap);
	va_end(ap);
}

void Error(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_COMPILE, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
 
void Skip(int c)
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
void Expect(char* msg)
{
	Error("缺少%s", msg);
}

void LinkError(char* fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_LINK, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
