#include"scc.h"

FILE* fin;
char* filename;
char* outname;
int line_num; 

/****************�ڴ��ʼ������***************/
void* mallocz(int size)
{
	void* ptr = NULL;
	ptr = malloc(size);
	if (!ptr && size)
	{
		error("�ڴ�������!");
	}
	memset(ptr, 0, size);
	return ptr;
}