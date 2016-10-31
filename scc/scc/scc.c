#include"scc.h"

FILE* fin=NULL;
char* filename;
char* outname;
int line_num;


/****************内存初始化函数***************/
void* MallocInit(const int size)
{
	void* ptr = NULL;
	ptr = malloc(size);
	if (!ptr && size)
	{
		Error("内存分配错误!");
	}
	memset(ptr, 0, size);
	return ptr;
}

void Init()
{
	line_num = 1;
	InitLex();
}

void Cleanup()
{
	int i;
	for (i = TK_IDENT; i < tktable.count; ++i)
	{
		free(tktable.data [i]);
	}
	free(tktable.data);
}

void* GetFileText(char* fname)
{
	char* p;
	p = strchr(fname, '.');
	return p + 1;
}
