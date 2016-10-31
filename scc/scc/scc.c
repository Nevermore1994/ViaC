#include"scc.h"

FILE* fin=NULL;
char* filename;
char* outname;
int line_num;


/****************�ڴ��ʼ������***************/
void* mallocz(const int size)
{
	void* ptr = NULL;
	ptr = malloc(size);
	if (!ptr && size)
	{
		Error("�ڴ�������!");
	}
	memset(ptr, 0, size);
	return ptr;
}

void init()
{
	line_num = 1;
	init_lex();
}

void  cleanup()
{
	int i;
	for (i = TK_IDENT; i < tktable.count; ++i)
	{
		free(tktable.data [i]);
	}
	free(tktable.data);
}

void* get_file_text(char* fname)
{
	char* p;
	p = strchr(fname, '.');
	return p + 1;
}
