/******************************************
*Author:Away
*Date:2016-11-8
*Function:�쳣ģ�鵥Ԫ����
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
			printf("%s(��%d��):���뾯�棺%s\n:", filename, linenum, buf);
			sprintf_s(errstr, ERR_SIZE, "%s(��%d��):���뾯�棺%s\n:", filename, linenum, buf);
		}
		else
		{
			printf("%s(��%d��:�������:%s\n", filename, linenum, buf);
			sprintf_s(errstr, ERR_SIZE, "%s(��%d��:�������:%s\n", filename, linenum, buf);
			Cleanup();
			exit(-1);
		}
	}
	else
	{
		printf("���Ӵ���:%s\n", buf);
		sprintf_s(errstr, ERR_SIZE, "���Ӵ���:%s\n", buf);
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
		Error("ȱ��'%s'", GetTkstr(c));
	}
	GetToken();
}
/*******************************************
*��������:��ʾȱ�ٵĳɷ�
*msg:��Ҫʲô�﷨�ɷ�
*******************************************/
void Expect(const char* msg)
{
	Error("ȱ��%s", msg);
}

void LinkError(const char* fmt,...)
{
	va_list ap;
	va_start(ap, fmt);
	HandleException(STAGE_LINK, LEVEL_ERROR, fmt, ap);
	va_end(ap);
}
