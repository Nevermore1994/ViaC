/******************************************
*author:Away
*date:2016-10-25
*******************************************/

#ifndef SCC_H_
#define SCC_H_

#include<windows.h>
#include<stdio.h> 

/********���嶯̬�ַ���********/
typedef struct DynString
{
	int count;		//�ַ�������
	int capacity;	//�ַ���������
	char* data;		//�ַ�ָ��


}DynString;
/******��̬�ַ�������***********/
void Dynstring_init(DynString* pstr, const int initsize);		//��ʼ����̬�ַ���
void Dynstring_free(DynString* pstr);					//�ͷŶ�̬�ַ���
void Dynstring_reset(DynString* pstr);
void Dynstring_chcat(DynString* pstr, const int ch);
void Dynstring_realloc(DynString* pstr, const int newsize);


/************���嶯̬����*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************��̬���麯��*****/
void Dynarray_init(DynArray* parr, const int size);
void Dynarray_realloc(DynArray* parr, const int newsize);
void Dynarray_add(DynArray* parr, void* data);
void Dynarray_free(DynArray* parr);
int  Dynarray_find(DynArray* parr, const int data);
void Dynarray_delete(DynArray* parr, const int i);
/************���ʱ���**********/
enum e_TokenCode
{
	/* ��������ָ��� */
	TK_PLUS,							// + �Ӻ�
	TK_MINUS,							// - ����
	TK_STAR,							// * �Ǻ�
	TK_DIVIDE,							// / ����
	TK_MOD,								// % ���������
	TK_EQ,								// == ���ں�
	TK_NEQ,								// != �����ں�
	TK_LT,								// < С�ں�
	TK_LEQ,								// <= С�ڵ��ں�
	TK_GT,								// > ���ں�
	TK_GEQ,								// >= ���ڵ��ں�
	TK_ASSIGN,							// = ��ֵ����� 
	TK_POINTSTO,						// -> ָ��ṹ���Ա�����
	TK_DOT,								// . �ṹ���Ա�����
	TK_AND,								// & ��ַ�������
	TK_OPENPA,							// ( ��Բ����
	TK_CLOSEPA,							// ) ��Բ����
	TK_OPENBR,							// [ ��������
	TK_CLOSEBR,							// ] ��Բ����
	TK_BEGIN,							// { �������
	TK_END,								// } �Ҵ�����
	TK_SEMICOLON,						// ; �ֺ�    
	TK_COMMA,							// , ����
	TK_ELLIPSIS,						// ... ʡ�Ժ�
	TK_EOF,								// �ļ�������

	/* ���� */
	TK_CINT,							// ���ͳ���
	TK_CCHAR,							// �ַ�����
	TK_CSTR,							// �ַ�������

	/* �ؼ��� */
	KW_CHAR,							// char�ؼ���
	KW_SHORT,							// short�ؼ���
	KW_INT,								// int�ؼ���
	KW_VOID,							// void�ؼ���  
	KW_STRUCT,							// struct�ؼ���   
	KW_IF,								// if�ؼ���
	KW_ELSE,							// else�ؼ���
	KW_FOR,								// for�ؼ���
	KW_CONTINUE,						// continue�ؼ���
	KW_BREAK,							// break�ؼ���   
	KW_RETURN,							// return�ؼ���
	KW_SIZEOF,							// sizeof�ؼ���

	KW_ALIGN,							// __align�ؼ���	
	KW_CDECL,							// __cdecl�ؼ��� standard c call
	KW_STDCALL,							// __stdcall�ؼ��� pascal c call

	/* ��ʶ�� */
	TK_IDENT
};
/*****************�ʷ�����**************************/
/**********�ʷ�״̬***********/
enum e_LexState
{
	LEX_NORMAL,
	LEX_SEP
};

/***********���ʱ�***********/
typedef struct TkWord
{
	int tkcode;
	struct TkWord* next;
	char* spelling;
	struct Symol* sym_struct;
	struct Symol* sym_id;
}TkWord;


#define MAXKEY 2048    

#define CH_EOF (-1)						//�ļ�β����ʶ

TkWord* TkwordInsert(char* p);
TkWord* TkwordDirectInsert(TkWord* pWord);
TkWord* TkwordFind(char* p, const int key);


void Getch();
void Preprocess();
TkWord* parse_identifiler();
void ParseNum();
void ParseString();
void InitLex();
void GetToken();
int IsDigit(char c);
int IsNoDigit(char* c);
void SkipWhiteSpace();
void ParseComment();
char* GetTkstr(const int v);
void TestLex();


/***************���ñ���****************************/
extern TkWord* tk_hashtable[MAXKEY];	//���ʹ�ϣ��
extern DynArray tktable;				//���ʶ�̬����
extern DynString tkstr;
extern FILE* fin;
extern char ch;
extern char* filename;
extern int token;
extern int line_num;
extern int tkvalue;

/***************������*****************************/
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

/***************�ʷ�������********************/
void ColorToken(const int lex_state);
void Init(); 
void Cleanup();
void* GetFileText();

/*****************���Ӻ���****************/
void* MallocInit(const int size);
int ElfHash(char* key);				// �ַ���ϣ����

/********************end*******************/
/*****************�﷨����*********************/
enum e_SynTaxState
{
	SNTX_NUL,
	SNTX_SP, 
	SNTX_LF_HT, 
	SNTX_DELAY
};

enum  e_StorageClass
{
	SC_GLOBAL  = 0x00f0, 
	SC_LOCAL   = 0x00f1, 
	SC_LLOACL  = 0x00f2, 
	SC_CMP     = 0x00f3,
	SC_VALMASK = 0x00ff,
	SC_LAVL    = 0x1000,
	SC_SYM     = 0x2000,
	 
	SC_ANOM    = 0x10000000,
	SC_STRUCT  = 0x20000000,
	SC_MEMBER  = 0x40000000,
	SC_PARAMS  = 0x80000000,
};

enum  e_TypeCode
{
	T_INT    = 0,
	T_CHAR   = 1, 
	T_SHORT  = 2,
	T_VOID   = 3, 
	T_PTR    = 4, 
	T_FUNC   = 5, 
	T_STRUCT = 6, 

	T_BTYPE  = 0x000f, 
	T_ARRAY  = 0x0010,
};

#define ALIGH_SET 0x100

extern int syntax_state;
extern int syntax_level;

void ParameterTypeList(int func_call); //�����β����ͱ�
void DirectDeclaratorPostfix();
void DirectDeclarator();
void Declarator();
void TranslationUnit();
void Initializer();
void ExternalDeclaration(int level);
int TypeSpecifier();
void StructDeclaration();
void StructDeclarationList();
void StructSpecifier();
void FunctionCallingConvention(int* fc);
void StructMemberAligment();
void CompoundStatement();
void Funcbody();
void Statement();
int IsTypeSpecifier(int id);
void ExpressionStatement();
void IfStatement();
void ForStatement();
void ContinueStatement();
void BreakStatement();
void ReturnStatement();
void Expression();
void AssignmentExpression();
void EquaityExpression();
void RealtionalExpression();
void AdditiveExpression();
void MultiplicativeExpression();
void UnaryExpression();
void SizeofExpression();
void PostfixExpression();
void PrimaryExpression();
void ArgumentExpressionList();
void SyntaxIndent();
void PrintTab(int num);
/*******************end*************************/
#endif // !SCC_H_




