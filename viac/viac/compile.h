#include"viac.h"

#ifndef		COMPILE_H_
#define 	COMPILE_H_

typedef int BOOL;

#define TRUE  1
#define FLASE 0

enum e_OutType
{
	OUTPUT_OBJ,		// Ŀ���ļ�
	OUTPUT_EXE,		// EXE��ִ���ļ�
	OUTPUT_MEMORY	// �ڴ���ֱ�����У������
};

void Compile(char* fname);
char* GetFileText(char* fname);
int ProcessCommand(int argc, char** argv);
void Cleanup(void);
void Init(void);
void* MallocInit(const int size);
int ElfHash(const char* key);				// �ַ���ϣ����
int CalcAlign(const int n, const int align);

#define  OUT_FILE_SIZE 256      //����ļ��������С

extern FILE* fin;				//Դ�ļ�ָ��
extern char* filename;				//Դ�ļ���
extern int linenum;				//�к�
extern Array srcfiles;			//Դ�ļ�����
extern char* outfile;		//����ļ���
extern int outtype;          //����ļ�����
extern float ViaCVersion; //�������汾



#endif