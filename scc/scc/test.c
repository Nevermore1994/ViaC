/******************************************
*Author:Away
*Date:2016-11-8
*Function:测试模块单元代码
*******************************************/
#include"viac.h"

int main(const int argc, char ** argv)
{
	
	Init();
	filename = "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\HelloWorld.c";
	errno_t err = fopen_s(&fin, filename, "rb");
	if (err)
	{
		printf("不能打开文件!\n");
		return 0;
	}
	
	Getch();
	GetToken();
	TranslationUnit();
	fclose(fin);

	GetObjFname(filename);
	WriteObj(outfile);
	char file[2048];
	sprintf_s(file, 2048, "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.viac%c",'\0' );
	printf("%s\n语法分析成功!\n",file);
	Cleanup();
	return 1;
}
