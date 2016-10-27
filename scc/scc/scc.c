#include"scc.h"

FILE* fin;
char* filename;
char* outname;
int line_num; 

/****************内存初始化函数***************/
void* mallocz(int size)
{
	void* ptr = NULL;
	ptr = malloc(size);
	if (!ptr && size)
	{
		error("内存分配错误!");
	}
	memset(ptr, 0, size);
	return ptr;
}