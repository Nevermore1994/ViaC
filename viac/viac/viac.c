/******************************************
*author:Away
*date:2016-10-27
*function:通用模块单元代码
*******************************************/

#include"viac.h"

FILE* fin = NULL;				//源文件指针
char* filename = NULL;				//源文件名
int linenum;				//行号
Array srcfiles;			//源文件数组
char* outfile = NULL;		//输出文件名
int outtype;          //输出文件类型
float ViaCVersion = 1.00; //编译器版本

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
	ArrayInit(&srcfiles, 1);
	ArrayInit(&arr_lib, 4);
	ArrayInit(&arr_dll, 4);

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

void Cleanup(void)
{
	SymPop(&GSYM, NULL);
	StackDestroy(&LSYM);
	StackDestroy(&GSYM);
	FreeSection();
	int i;
	for (i = TK_IDENT; i < tktable.count; ++i)
	{
		free(tktable.data[i]);
	}
	free(tktable.data);
	ArrayFree(&arr_dll);
	if (srcfiles.data)
	{
		free(srcfiles.data);
	}
	if (arr_lib.data)
	{
		free(arr_lib.data);
	}
}

int ProcessCommand(int argc, char** argv)
{
	int i;
	for (i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			char* p = &argv[i][1];  //'-'的后一位为命令行标识符
			int c = *p;

			switch (c)
			{
				case 'o':
				{
					outfile = argv[++i];
					break;
				}
				case 'c':
				{
					ArrayAdd(&srcfiles, argv[++i]);
					outtype = OUTPUT_OBJ;
					return 1;
				}
				case 'l':
				{
					ArrayAdd(&arr_lib, &argv[i][2]);
					break;
				}
				case 'G':
				{
					subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
					break;
				}
				case 'v':
				{
					printf("ViaC编译器版本：%.2f", ViaCVersion);
					return 0;
				}
				case 'h':
				{
					printf("usage: scc [-c infile] [-o outfile] [-llib] [infile1 infile2...] \n");
					return 0;
				}
				default:
				{
					printf("unsupported command line option");
					return 0;
				}
			}
		}
		else
		{

			ArrayAdd(&srcfiles, argv[i]);
		}
	}

	return 1;
}

char* GetFileText(char* fname)
{
	char* p = NULL;
	p = strrchr(fname, '.');
	return p + 1;
}

void Compile(char* fname)
{
	errno_t err = fopen_s(&fin, fname, "rb");
	if (err)
	{
		printf("不能打开ViaC文件");
	}
	GetCh();
	linenum = 1;
	GetToken();
	TranslationUnit();
	fclose(fin);
	printf("\n\n%s 代码行数: %d行\n\n", fname, linenum);
}

int CalcAlign(const int n, const int align)
{
	int num = n + align - 1;
	int res = ~(align - 1);
	return (num & res);
}