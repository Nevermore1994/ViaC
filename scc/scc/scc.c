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
	
	StackInit(&LSYM, 8);
	StackInit(&GSYM, 8);
	//SymSecRdata = SecSymPut(".rdata", 0);


	int_type.t = T_INT;
	char_pointer_type.t = T_CHAR; 
	//MkPointer(&char_pointer_type);
	default_func_type.t = T_FUNC;
	default_func_type.ref = SymPush(SC_ANOM, &int_type, KW_CDECL, 0);
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

int CalcAlign(int n, int align)
{
	return ((n + align + 1)  &  ( ~(align - 1) ));
}