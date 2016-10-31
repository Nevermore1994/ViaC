#include"scc.h"

DynString tkstr;

 
void HandleException(int stage, int level, char* fmt, va_list ap)
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
		Error("ȱ��'%s'", GetTkstr(c));
	}
	GetToken();
}
/*******************************************
*��������:��ʾȱ�ٵĳɷ�
*msg:��Ҫʲô�﷨�ɷ�
*******************************************/
void Expect(char* msg)
{
	Error("ȱ��%s", msg);
}

void LinkError(char* fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_LINK, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
