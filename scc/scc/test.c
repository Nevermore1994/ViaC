#include"scc.h"

int main(int argc, char ** argv)
{
	filename = "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.c";
	errno_t err = fopen_s(&fin, filename, "rb");
	if (err)
	{
		printf("���ܴ��ļ�!\n");
		return 0;
	}
	Init();
	Getch();
	TestLex();
	TranslationUnit();
	Cleanup();
	fclose(fin);
	printf("%s �﷨�����ɹ�!", filename);
	return 1;
}
