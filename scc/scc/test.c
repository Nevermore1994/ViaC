/******************************************
*Author:Away
*Date:2016-11-8
*Function:����ģ�鵥Ԫ����
*******************************************/
#include"viac.h"

int main(const int argc, char ** argv)
{
	
	Init();
	filename = "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.viac";
	errno_t err = fopen_s(&fin, filename, "rb");
	if (err)
	{
		printf("���ܴ��ļ�!\n");
		return 0;
	}
	
	Getch();
	GetToken();
	TranslationUnit();
	fclose(fin);

	//GetObjFname(filename);
	//WriteObj(outfile);
	printf("%s\n�﷨�����ɹ�!\n",filename);
	Cleanup();
	return 1;
}
