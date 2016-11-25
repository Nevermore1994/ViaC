#include"viac.h"

#ifndef		COMPILE_H_
#define 	COMPILE_H_

typedef int BOOL;

#define TRUE  1
#define FLASE 0

enum e_OutType
{
	OUTPUT_OBJ,		// 目标文件
	OUTPUT_EXE,		// EXE可执行文件
	OUTPUT_MEMORY	// 内存中直接运行，不输出
};

void Compile(char* fname);
char* GetFileText(char* fname);
int ProcessCommand(int argc, char** argv);
void Cleanup(void);
void Init(void);
void* MallocInit(const int size);
int ElfHash(const char* key);				// 字符哈希函数
int CalcAlign(const int n, const int align);

#define  OUT_FILE_SIZE 256      //输出文件名数组大小

extern FILE* fin;				//源文件指针
extern char* filename;				//源文件名
extern int linenum;				//行号
extern Array srcfiles;			//源文件数组
extern char* outfile;		//输出文件名
extern int outtype;          //输出文件类型
extern float ViaCVersion; //编译器版本



#endif