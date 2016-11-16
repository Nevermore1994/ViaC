// ���Լ�����д��������������������Դ����

#include "viac.h"

FILE *fin = NULL;				// Դ�ļ�ָ��
char *filename;					// Դ�ļ�����
DynArray src_files;				// Դ�ļ�����
char *outfile;					// ����ļ���
int outtype;				// ����ļ�����
float scc_version = 1.00;		// SCC�������汾��
int linenum;
								/***********************************************************
								* ����:	������ڴ沢�����ݳ�ʼ��Ϊ'0'
								* size:	�����ڴ��С
								**********************************************************/
void *MallocInit(int size)
{
	void *ptr;
	ptr = malloc(size);
	if (!ptr && size)
		Error("�ڴ����ʧ��");
	memset(ptr, 0, size);
	return ptr;
}

/***********************************************************
* ����:	�����ϣ��ַ
* key:		��ϣ�ؼ���
* MAXKEY:	��ϣ����
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
* ����:	�����ֽڶ���λ��
* n:		δ����ǰֵ
* align:   ��������
**********************************************************/
int CalcAlign(int n, int align)
{
	return ((n + align - 1) & (~(align - 1)));
}


/***********************************************************
* ����:	��ʼ��
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
* ����:	ɨβ������
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
		free(tktable.data[i]);	//tktable�����TK_IDENT�����ͷţ��������DynArrayFree�������⣬��ΪTK_IDENT���µ�tokenû�з��ڶ��У����Ƿ��ھ�̬�洢��

	}
	free(tktable.data);
	DynArrayFree(&arr_dll);
	free(src_files.data);
	free(arr_lib.data);
}

/***********************************************************
* ����:	����������ѡ��
* argc:	�����в�������
* argv:	�����в�������
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
* ����:	�õ��ļ���չ��
* fname:	�ļ�����
**********************************************************/
char *GetFileText(char *fname)
{
	char *p;
	p = strrchr(fname, '.');
	return p + 1;
}

/***********************************************************
* ����:	����SCԴ�ļ�
* fname:	SCԴ�ļ���
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
	printf("\n\n%s ��������: %d��\n\n", fname, linenum);
}

/***********************************************************
* ����:	main������
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