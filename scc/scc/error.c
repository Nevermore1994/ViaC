#include"scc.h"

DynString tkstr;

 
void handle_exception(int stage, int level, char* fmt, va_list ap)
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

void warning(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_COMPILE, LEVEL_WARNING, fmt, ap);
	va_end(ap);
}

void error(char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_COMPILE, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
 
void skip(int c)
{
	if (token != c)
	{
		error("缺少'%s'", get_tkstr(c));
	}
	get_token();
}
/*******************************************
*函数功能:提示缺少的成分
*msg:需要什么语法成分
*******************************************/
void expect(char* msg)
{
	error("缺少%s", msg);
}

void link_error(char* fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_LINK, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
