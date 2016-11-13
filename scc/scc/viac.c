/******************************************
*author:Away
*date:2016-10-27
*function:ͨ��ģ�鵥Ԫ����
*******************************************/

#include"viac.h"

FILE* fin = NULL;				//Դ�ļ�ָ��
char* filename;				//Դ�ļ���
int linenum;				//�к�
DynArray srcfiles;			//Դ�ļ�����
char outfile[OUT_FILE_SIZE];		//����ļ���
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
	DynArrayInit(&srcfiles, 1);
	linenum = 1;
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