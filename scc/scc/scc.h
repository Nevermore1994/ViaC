/******************************************
*author:Away
*date:2016-10-25
*******************************************/

#ifndef SCC_H_
#define SCC_H_

#include<windows.h>
#include<stdio.h> 

/********定义动态字符串********/
typedef struct DynString
{
	int count;		//字符串长度
	int capacity;	//字符串缓冲区
	char* data;		//字符指针


}DynString;
/******动态字符串函数***********/
void Dynstring_init(DynString* pstr, int initsize);		//初始化动态字符串
void Dynstring_free(DynString* pstr);					//释放动态字符串
void Dynstring_reset(DynString* pstr);
void Dynstring_chcat(DynString* pstr, int ch);
void Dynstring_realloc(DynString* pstr, int newsize);


/************定义动态数组*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************动态数组函数*****/
void Dynarray_init(DynArray* parr, int size);
void Dynarray_realloc(DynArray* parr, int newsize);
void Dynarray_add(DynArray* parr, void* data);
void Dynarray_free(DynArray* parr);
int  Dynarray_find(DynArray* parr, int data);

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
	struct Symol* sym_struct;
	struct Symol* sym_id;
}TkWord;


#define MAXKEY 2048    

#define CH_EOF (-1)						//文件尾部标识

TkWord* tkword_insert(char* p);
TkWord* tkword_direct_insert(TkWord* pWord);
TkWord* tkword_find(char* p, int key);



char  is_nogiht(char* c);
void  init_lex();
void  get_token();
char* get_tkstr(int v);
/********************************************/
extern TkWord* tk_hashtable[MAXKEY];	//单词哈希表
extern DynArray tktable;				//单词动态数组
extern DynString tkstr;
extern FILE* fin;
extern char ch;
extern char* filename;
extern int token;
extern int line_num;



/***************错误处理*****************************/

enum e_ErrorLevel
{
	LEVEL_WARNING,
	LEVEL_ERROR
};

enum e_WorkStage
{
	STAGE_COMPILE,
	SRAGE_LINK
};

void  warning(char* fmt, ...);
void  error(char* fmt,...);
void  expect(char* msg);
void  skip(int c);
/*****************附加函数****************/
void* mallocz(int size);
int elf_hash(char* key);				// 字符哈希函数
#endif // !SCC_H_

