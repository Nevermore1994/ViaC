/******************************************
*author:Away
*date:2016-10-27
*function:ͨ��ģ�鵥Ԫ����
*******************************************/

#include"viac.h"

FILE* fin = NULL;				//Դ�ļ�ָ��
char* filename = NULL;				//Դ�ļ���
int linenum;				//�к�
Array srcfiles;			//Դ�ļ�����
char* outfile = NULL;		//����ļ���
int outtype;          //����ļ�����
float ViaCVersion = 1.00; //�������汾

						  /****************�ڴ��ʼ������***************/
void* MallocInit(const int size)
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
			char* p = &argv[i][1];  //'-'�ĺ�һλΪ�����б�ʶ��
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
					printf("ViaC�������汾��%.2f", ViaCVersion);
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
		printf("���ܴ�ViaC�ļ�");
	}
	GetCh();
	linenum = 1;
	GetToken();
	TranslationUnit();
	fclose(fin);
	printf("\n\n%s ��������: %d��\n\n", fname, linenum);
}

int CalcAlign(const int n, const int align)
{
	int num = n + align - 1;
	int res = ~(align - 1);
	return (num & res);
}