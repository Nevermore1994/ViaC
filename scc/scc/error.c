#include"scc.h"

DynString tkstr;
FILE* fin;
int line_num;

 
void handle_exception(int stage, int level, char* fmt, va_list ap)
{
	char buf[1024];
	vsprintf(buf, fmt, ap);
	if (stage == STAGE_COMPILE)
	{
		if (level == LEVEL_WARNING)
			printf("%s(第%d行):编译警告：%s\n:", filename, line_num);
		else
		{
			printf("%s(第%d行:编译错误:%s\n", filename, line_num);
			exit(-1);
		}
	}
	else
	{
		printf("链接错误");
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
 
void expect(char* msg)
{
	error("缺少%s", msg);
}

/****************************************************
*函数功能：跳过c这个单词，如果当前单词不是c,提示错误
*c:需要跳过的单词
****************************************************/
void skip(int c)
{
	if (token != c)
	{
		error("缺少'%s'", get_tkstr(c));
	}
	get_token();
}
