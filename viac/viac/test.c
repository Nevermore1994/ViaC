/******************************************
*Author:Away
*Date:2016-11-8
*Function:����ģ�鵥Ԫ����
*******************************************/
#include"viac.h"

int main(const int argc, char ** argv)
{
	
	/*Init();
	filename = "C:\\Users\\Away\\Documents\\ViaC\\viac\\viac\\Debug\\HelloWorld.c";
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

	GetObjFname(filename);
	WriteObj(outfile);
	char file[2048];
	sprintf_s(file, 2048, "C:\\Users\\Away\\Documents\\scc\\scc\\Debug\\1.viac%c",'\0' );
	printf("%s\n�﷨�����ɹ�!\n",file);
	Cleanup();*/
	char* name = GetDllName("hello.slib");
	printf("%s", name); 
	free(name);
	return 1;
}
