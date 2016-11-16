// 《自己动手写编译器、链接器》配套源代码

#include "viac.h"

FILE *fin = NULL;				// 源文件指针
char *filename;					// 源文件名称
DynArray src_files;				// 源文件数组
char *outfile;					// 输出文件名
int outtype;				// 输出文件类型
float scc_version = 1.00;		// SCC编译器版本号
int linenum;
								/***********************************************************
								* 功能:	分配堆内存并将数据初始化为'0'
								* size:	分配内存大小
								**********************************************************/
void *MallocInit(int size)
{
	void *ptr;
	ptr = malloc(size);
	if (!ptr && size)
		Error("内存分配失败");
	memset(ptr, 0, size);
	return ptr;
}

/***********************************************************
* 功能:	计算哈希地址
* key:		哈希关键字
* MAXKEY:	哈希表长度
**********************************************************/
int ElfHash(char *key)
{
	int h = 0, g;
	while (*key)
	{
		h = (h << 4) + *key++;
		g = h & 0xf0000000;
		if (g)
			h ^= g >> 24;
		h &= ~g;
	}
	return h % MAXKEY;
}

/***********************************************************
* 功能:	计算字节对齐位置
* n:		未对齐前值
* align:   对齐粒度
**********************************************************/
int CalcAlign(int n, int align)
{
	return ((n + align - 1) & (~(align - 1)));
}


/***********************************************************
* 功能:	初始化
**********************************************************/
void init()
{
	DynArrayInit(&src_files, 1);
	DynArrayInit(&arr_lib, 4);
	DynArrayInit(&arr_dll, 4);
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
	default_func_type.ref = SymPush(ViaC_ANOM, &int_type, KW_CDECL, 0);

	optop = opstack - 1;

	InitCoff();

	lib_path = GetLibPath();
}

/***********************************************************
* 功能:	扫尾清理工作
**********************************************************/
void Cleanup()
{
	int i;
	SymPop(&GSYM, NULL);
	StackDestroy(&LSYM);
	StackDestroy(&GSYM);
	FreeSection();

	for (i = TK_IDENT; i < tktable.count; i++)
	{
		free(tktable.data[i]);	//tktable必须从TK_IDENT形如释放，如果调用DynArrayFree会有问题，因为TK_IDENT以下的token没有放在堆中，而是放在静态存储区

	}
	free(tktable.data);
	DynArrayFree(&arr_dll);
	free(src_files.data);
	free(arr_lib.data);
}

/***********************************************************
* 功能:	处理命令行选项
* argc:	命令行参数个数
* argv:	命令行参数数组
**********************************************************/
int ProcessCommand(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			char *p = &argv[i][1];
			int c = *p;
			switch (c)
			{
				case 'o':
					outfile = argv[++i];
					break;
				case 'c':
					DynArrayAdd(&src_files, argv[++i]);
					outtype = OUTPUT_OBJ;
					return 1;
				case 'l':
					DynArrayAdd(&arr_lib, &argv[i][2]);
					break;
				case 'G':
					subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
					break;
				case 'v':
					printf("SCC Version %.2f", scc_version);
					return 0;
				case 'h':
					printf("usage: scc [-c infile] [-o outfile] [-llib] [infile1 infile2...] \n");
					return 0;
				default:
					printf("unsupported command line option");
					return 0;
			}
		}
		else
		{
			DynArrayAdd(&src_files, argv[i]);
		}

	}
	return 1;

}

/***********************************************************
* 功能:	得到文件扩展名
* fname:	文件名称
**********************************************************/
char *GetFileText(char *fname)
{
	char *p;
	p = strrchr(fname, '.');
	return p + 1;
}

/***********************************************************
* 功能:	编译SC源文件
* fname:	SC源文件名
**********************************************************/
void Compile(char *fname)
{
	fopen_s( &fin ,fname, "rb");
	if (!fin)
		printf("cannot open SC source file");
	 GetCh();
	linenum = 1;
	GetToken();
	TranslationUnit();
	fclose(fin);
	printf("\n\n%s 代码行数: %d行\n\n", fname, linenum);
}

/***********************************************************
* 功能:	main主函数
**********************************************************/
void main(int argc, char ** argv)
{
	int i, opind;
	char *ext;
	init();
	outtype = OUTPUT_EXE;
	subsystem = IMAGE_SUBSYSTEM_WINDOWS_CUI;
	char* str[5] = { "viac","-lmsvcrt","-o","HelloWorld.exe","HelloWorld.c" };
	opind = ProcessCommand(5, str);
	if (opind == 0)
		return;
	for (i = 0; i < src_files.count; i++)
	{
		filename = src_files.data[i];
		ext = GetFileText(filename);
		if (!strcmp(ext, "c"))
			Compile(filename);
		if (!strcmp(ext, "obj"))
		{
			printf("ss");
			LoadObjFile(filename);
		}

	}
	if (outtype == OUTPUT_OBJ)
		WriteObj(outfile);
	else
		PeOutputFile(outfile);

	Cleanup();
}