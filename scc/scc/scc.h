/******************************************
*author:Away
*date:2016-10-25
*******************************************/

#ifndef SCC_H_
#define SCC_H_

#include<windows.h>
#include<stdio.h>

typedef int bool;
typedef unsigned int   DWORD;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
#define TRUE  1
#define FLASE 0

/********定义动态字符串********/
typedef struct DynString
{
	int count;		//字符串长度
	int capacity;	//字符串缓冲区
	char* data;		//字符指针
}DynString;
/******动态字符串函数***********/
void DynStringInit(DynString* pstr, const int initsize);		//初始化动态字符串
void DynStringFree(DynString* pstr);					//释放动态字符串
void DynStringReset(DynString* pstr);
void DynStringChcat(DynString* pstr, const int ch);
void DynStringRealloc(DynString* pstr, const int newsize);


/************定义动态数组*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************动态数组函数*****/
void DynArrayInit(DynArray* parr, const int size);
void DynArrayRealloc(DynArray* parr, const int newsize);
void DynArrayAdd(DynArray* parr, void* data);
void DynArrayFree(DynArray* parr);
int  DynArrayFind(DynArray* parr, const int data);
void DynArrayDelete(DynArray* parr, const int i);
/************单词编码**********/
enum e_TokenCode
{
	/* 运算符及分隔符 */
	TK_PLUS,							// + 加号
	TK_MINUS,							// - 减号
	TK_STAR,							// * 星号
	TK_DIVIDE,							// / 除号
	TK_MOD,								// % 求余运算符
	TK_EQ,								// == 等于号
	TK_NEQ,								// != 不等于号
	TK_LT,								// < 小于号
	TK_LEQ,								// <= 小于等于号
	TK_GT,								// > 大于号
	TK_GEQ,								// >= 大于等于号
	TK_ASSIGN,							// = 赋值运算符 
	TK_POINTSTO,						// -> 指向结构体成员运算符
	TK_DOT,								// . 结构体成员运算符
	TK_AND,								// & 地址与运算符
	TK_OPENPA,							// ( 左圆括号
	TK_CLOSEPA,							// ) 右圆括号
	TK_OPENBR,							// [ 左中括号
	TK_CLOSEBR,							// ] 右圆括号
	TK_BEGIN,							// { 左大括号
	TK_END,								// } 右大括号
	TK_SEMICOLON,						// ; 分号    
	TK_COMMA,							// , 逗号
	TK_ELLIPSIS,						// ... 省略号
	TK_EOF,								// 文件结束符

	/* 常量 */
	TK_CINT,							// 整型常量
	TK_CCHAR,							// 字符常量
	TK_CSTR,							// 字符串常量

	/* 关键字 */
	KW_CHAR,							// char关键字
	KW_SHORT,							// short关键字
	KW_INT,								// int关键字
	KW_VOID,							// void关键字  
	KW_STRUCT,							// struct关键字   
	KW_IF,								// if关键字
	KW_ELSE,							// else关键字
	KW_FOR,								// for关键字
	KW_CONTINUE,						// continue关键字
	KW_BREAK,							// break关键字   
	KW_RETURN,							// return关键字
	KW_SIZEOF,							// sizeof关键字

	KW_ALIGN,							// __align关键字	
	KW_CDECL,							// __cdecl关键字 standard c call
	KW_STDCALL,							// __stdcall关键字 pascal c call

	/* 标识符 */
	TK_IDENT
};
/*****************词法分析**************************/
/**********词法状态***********/
enum e_LexState
{
	LEX_NORMAL,
	LEX_SEP
};

/***********单词表***********/
typedef struct TkWord
{
	int tkcode;
	struct TkWord* next;
	char* spelling;
	struct Symbol* sym_struct;
	struct Symbol* sym_id;
}TkWord;


#define MAXKEY 2048    

#define CH_EOF (-1)						//文件尾部标识

TkWord* TkwordInsert(char* p);
TkWord* TkwordDirectInsert(TkWord* pWord);
TkWord* TkwordFind(char* p, const int key);


void Getch(void);
void Preprocess(void);
void ParseIdentifier(void);
void ParseNum(void);
void ParseString(void);
void InitLex(void);
void GetToken(void);
int IsDigit(char c);
int IsNoDigit(char* c);
void SkipWhiteSpace(void);
void ParseComment(void);
char* GetTkstr(const int v);
void TestLex(void);


/***************引用变量****************************/
extern TkWord* tk_hashtable[MAXKEY];	//单词哈希表
extern DynArray tktable;				//单词动态数组
extern DynString tkstr;
extern FILE* fin;
extern char ch;
extern char* filename;
extern int token;
extern int line_num;
extern int tkvalue;

/***************错误处理*****************************/
enum e_ErrorLevel
{
	LEVEL_WARNING,
	LEVEL_ERROR
};

enum e_WorkStage
{
	STAGE_COMPILE,
	STAGE_LINK
};

void  Warning(char* fmt, ...);
void  Error(char* fmt,...);
void  Expect(char* msg);
void  Skip(const int c);
void  LinkError(char* fmt, ...);

/***************词法处理函数********************/
void ColorToken(const int lex_state);
void Init(); 
void Cleanup();
void* GetFileText();

/*****************附加函数****************/
void* MallocInit(const int size);
int ElfHash(char* key);				// 字符哈希函数
int CalcAlign(int n, int align);

/********************end*******************/

/******************Stack*********************/
typedef struct Stack
{
	void** base;
	void** top;
	int size;
}Stack;

void StackInit(Stack* stack, int size);
void* StackPush(Stack* stack, void* data, int size);
void StackPop(Stack* stack);
void* StackgGetTop(Stack* stack);
bool StackIsEmpty(Stack* stack);
void StackDestroy(Stack* stack);
/******************end*************************/


/*******************SYM*************************/
Stack GSYM; //全局符号栈
Stack LSYM; //局部符号栈


typedef struct Type
{
	int t;
	struct Symbol *ref;
}Type;

Type char_pointer_type;
Type int_type;
Type default_func_type;

typedef struct Symbol
{
	int v;
	int r; 
	int c; 
	Type type; 
	struct Symbol  *next; 
	struct Symbol  *prev_tok;
}Symbol;

Symbol* StructSearch(int v);
Symbol* SymSearch(int v);
Symbol* SymDirectPush(Stack* stack, int v, Type* type, int c);
Symbol* SymPush(int v, Type* type, int r, int c);
Symbol* FuncSymPush(int v, Type *type);
Symbol* VarSymPut(Type* type, int r, int v, int addr);
Symbol* SecSymPut(char* sec, int c);
void SymPop(Stack* stack, Symbol *b);
int TypeSize(Type *t, int *a);
/********************end**********************/
/*****************语法分析*********************/
enum e_SynTaxState
{
	SNTX_NUL,
	SNTX_SP,
	SNTX_LF_HT,
	SNTX_DELAY
};

enum  e_StorageClass
{
	SC_GLOBAL = 0x00f0,
	SC_LOCAL = 0x00f1,
	SC_LLOACL = 0x00f2,
	SC_CMP = 0x00f3,
	SC_VALMASK = 0x00ff,
	SC_LVAL = 0x0100,//左值
	SC_SYM = 0x0200,//符号

	SC_ANOM = 0x10000000,  //匿名符号
	SC_STRUCT = 0x20000000,  //结构体
	SC_MEMBER = 0x40000000,  //结构体的成员变量
	SC_PARAMS = 0x80000000,  //函数参数
};

enum  e_TypeCode
{
	T_INT = 0,
	T_CHAR = 1,
	T_SHORT = 2,
	T_VOID = 3,
	T_PTR = 4,
	T_FUNC = 5,
	T_STRUCT = 6,

	T_BTYPE = 0x000f,
	T_ARRAY = 0x0010,
};

#define ALIGN_SET 0x100

extern int syntax_state;
extern int syntax_level;

void TranslationUnit(void);
void ExternalDeclaration(const int level);
void Initializer(Type* type); // 初值符
int TypeSpecifier(Type* type);
void StructSpecifier(Type* type);
void StructDeclarationList(Type* type);
void StructDeclaration(int* maxalign, int* offset, Symbol*** ps);
void Declarator(Type* type, const int* v, const int* force_align);
void FunctionCallingConvention(int* fc);
void StructMemberAlignment(int* force_align);
void DirectDeclarator(Type* type, int* v, const int func_call);
void DirectDeclaratorPostfix(Type* type, const int func_call);
void ParameterTypeList(Type* type, int func_call);
void Funcbody(Symbol* sym);
int IsTypeSpecifier(const int id);
void Statement(int* bsym, int* csym);
void CompoundStatement(int* bsym, int* csym);
void IfStatement(const int* bsym, const int* csym);
void ForStatement(const int* bsym, const int* csym);
void ContinueStatement(const int* csym);
void BreakStatement(const int* bsym);
void ReturnStatement(void);
void ExpressionStatement(void);
void Expression(void);
void AssignmentExpression(void);
void EqualityExpression(void);
void RelationalExpression(void);
void AdditiveExpression(void);
void MultiplicativeExpression(void);
void UnaryExpression(void);
void SizeofExpression(void);
void PostfixExpression(void);
void PrimaryExpression(void);
void ArgumentExpressionList(void);
void PrintTab(const int num);
void SyntaxIndent(void);
/*******************end*************************/

/********************coff***********************/

#pragma pack(push,1)
typedef struct Section
{
	int data_offset; 
	int data_allocated;
	char* data;
	char index; 
	struct Section * plink;
	int* hashtab;
	IMAGE_SECTION_HEADER  sh;//节头
}Section;

typedef struct CoffSym
{
	DWORD Name; 
	DWORD Next; 
	
	DWORD Value;
	short sSection; 
	WORD Type; 
	BYTE StorageClass; 
	BYTE NumberOfAuxSymbols;
}CoffSym;

#define CST_FUNC 0x20
#define CST_NOFUNC 0

typedef struct CoffReloc
{
	DWORD offset; 
	DWORD cfsym; 
	BYTE  section; 
	BYTE  type;
}CoffReloc;

#pragma pack(pop)
typedef Section* pSection;
extern DynArray sections;

extern pSection sec_text,sec_data, sec_bss, sec_idata, sec_rdata, sec_rel, sec_symtab, sec_dynsymtab;

extern int nsec_image;

void SectionRealloc(pSection sec, const int newsize);
void*  SectionPtrAdd(const pSection sec, const int increment);
Section* SectionNew(const char* name, const int characteristics);
int CoffsymSearch(const pSection symtab, const char* name);
char* CoffstrAdd(const pSection strtab, const char* name);
int CoffsymAdd(const pSection symtab, const char* name, const int val, const int sec_index, const short type, const char StrorageClass);
void CoffsymAddUpdate(Symbol* ps, const int val, const int sec_index, const short type, const char StroageClass);
void FreeSection(void);
pSection NewCoffsymSection(const char* symtab_name, const int Characteristics, const char* strtab_name);
void CoffelocDirectAdd(const int offset, const int cfsym, const char section, const char type);
void CoffelocAdd(pSection sec, Symbol* sym, const int offset, const char type);
void InitCoff(void);
void Fpad(const FILE* fp, const int new_pos);
void WriteObj(const char* name);
/*********************end**************************/
typedef struct Operand
{
	Type type; 
	unsigned short reg;
	int value;
	struct Symbol* sym;
}Operand;


#endif // SCC_H_




