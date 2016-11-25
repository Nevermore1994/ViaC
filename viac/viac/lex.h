#include"viac.h"

#ifndef		LEX_H_
#define 	LEX_H_

/*****************词法分析**************************/
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
	TK_SPACE,							//
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
	KW_INCLUDE,
	KW_DO,
	KW_END,

	KW_REQUIRE,
	KW_ALIGN,							// __align关键字	
	KW_CDECL,							// __cdecl关键字 standard c call
	KW_STDCALL,							// __stdcall关键字 pascal c call

										/* 标识符 */
	TK_IDENT
};
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

TkWord* TkwordInsert(const char* p);
TkWord* TkwordDirectInsert(TkWord* pWord);
TkWord* TkwordFind(const char* p, const int key);


void GetCh(void);
void Preprocess(void);
void ParseIdentifier(void);
void ParseNum(void);
void ParseString(const char sep);
void InitLex(void);
void GetToken(void);
int IsDigit(const char c);
int IsNoDigit(const char c);
void SkipWhiteSpace(void);
void ParseComment(void);
void ParseComment_2();
char* GetTkstr(const int v);
void TestLex(void);
void ColorToken(const int lex_state);

/***************引用变量****************************/
extern TkWord* tk_hashtable[MAXKEY];	//单词哈希表
extern Array tktable;				//单词动态数组
extern String tkstr;

extern char ch;
extern int tkvalue;
extern int token;

 
#endif