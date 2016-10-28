#include"scc.h"

int main(int argc, char ** argv)
{

	errno_t err = fopen_s(&fin, "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.c", "rb");
	if (err)
	{
		printf("不能打开文件!\n");
		return 0;
	}
	init();
	getch();
	test_lex();
	cleanup();
	fclose(fin);
	printf("%s 词法分析成功!", argv [1]);
	return 1;
}
