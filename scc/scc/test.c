#include"scc.h"

int main(const int argc, char ** argv)
{
	filename = "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.c";
	errno_t err = fopen_s(&fin, filename, "rb");
	if (err)
	{
		printf("不能打开文件!\n");
		return 0;
	}
	Init();
	Getch();
	TestLex();
	TranslationUnit();
	Cleanup();
	fclose(fin);
	printf("%s 语法分析成功!", filename);
	return 1;
}
