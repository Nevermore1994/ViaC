#include"scc.h"

int main(int argc, char ** argv)
{

	errno_t err = fopen_s(&fin, "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.c", "rb");
	if (err)
	{
		printf("���ܴ��ļ�!\n");
		return 0;
	}
	init();
	getch();
	test_lex();
	cleanup();
	fclose(fin);
	printf("%s �ʷ������ɹ�!", argv [1]);
	return 1;
}
