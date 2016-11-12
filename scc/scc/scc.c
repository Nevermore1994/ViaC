/******************************************
*author:Away
*date:2016-10-27
*function:通用模块单元代码
*******************************************/

#include"scc.h"

FILE* fin = NULL;				//源文件指针
char* filename;				//源文件名
int line_num;				//行号
DynArray src_files;			//源文件数组
char outfile[OUT_FILE_SIZE];		//输出文件名
int output_type;          //输出文件类型
float scc_version = 1.00; //编译器版本

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

void Init(void)
{
	DynArrayInit(&src_files, 1);
	line_num = 1;
	InitLex(); 
	
	syntax_state = SNTX_NUL;
	syntax_level = 0;

	StackInit(&LSYM, 8);
	StackInit(&GSYM, 8);
	sym_sec_rdata = SecSymPut(".rdata", 0);


	int_type.t = T_INT;
	char_pointer_type.t = T_CHAR; 
	MkPointer(&char_pointer_type);
	default_func_type.t = T_FUNC;
	default_func_type.ref = SymPush(SC_ANOM, &int_type, KW_CDECL, 0);
	
	optop = opstack - 1;
	InitCoff();
}

void Cleanup(void)
{
	SymPop(&GSYM, NULL);
	StackDestroy(&LSYM);
	StackDestroy(&GSYM);
	FreeSection();
	int i;
	for (i = TK_IDENT; i < tktable.count; ++i)
	{
		free(tktable.data [i]);
	}
	free(tktable.data);
}

void GetObjFname(const char* fname)
{
	char* p;
	int i;
	p = strrchr(fname, '.');
	i = p - fname + 1;
	strcpy_s(outfile, OUT_FILE_SIZE, fname);
	strcpy_s(outfile + i, OUT_FILE_SIZE, "obj");
}

int CalcAlign(const int n, const int align)
{
	return ((n + align + 1)  &  ( ~(align - 1) ));
}