#include"scc.h"

DynString tkstr;

 
void handle_exception(int stage, int level, char* fmt, va_list ap)
{
	char buf[1024];
	vsprintf_s(buf, 1024,fmt, ap);
	if (stage == STAGE_COMPILE)
	{
		if (level == LEVEL_WARNING)
			printf("%s(��%d��):���뾯�棺%s\n:", filename, line_num, buf);
		else
		{
			printf("%s(��%d��:�������:%s\n", filename, line_num, buf);
			exit(-1);
		}
	}
	else
	{
		printf("���Ӵ���:%s\n", buf);
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
		error("ȱ��'%s'", get_tkstr(c));
	}
	get_token();
}
/*******************************************
*��������:��ʾȱ�ٵĳɷ�
*msg:��Ҫʲô�﷨�ɷ�
*******************************************/
void expect(char* msg)
{
	error("ȱ��%s", msg);
}

void link_error(char* fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	handle_exception(STAGE_LINK, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
