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
void Dynstring_init(DynString* pstr, int initsize);		//��ʼ����̬�ַ���
void Dynstring_free(DynString* pstr);					//�ͷŶ�̬�ַ���
void Dynstring_reset(DynString* pstr);
void Dynstring_chcat(DynString* pstr, int ch);
void Dynstring_realloc(DynString* pstr, int newsize);


/************���嶯̬����*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************��̬���麯��*****/
void Dynarray_init(DynArray* parr, int size);
void Dynarray_realloc(DynArray* parr, int newsize);
void Dynarray_add(DynArray* parr, void* data);
void Dynarray_free(DynArray* parr);
int  Dynarray_find(DynArray* parr, int data);

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

TkWord* tkword_insert(char* p);
TkWord* tkword_direct_insert(TkWord* pWord);
TkWord* tkword_find(char* p, int key);



char  is_nogiht(char* c);
void  init_lex();
void  get_token();
char* get_tkstr(int v);
/********************************************/
extern TkWord* tk_hashtable[MAXKEY];	//���ʹ�ϣ��
extern DynArray tktable;				//���ʶ�̬����
extern DynString tkstr;
extern FILE* fin;
extern char ch;
extern char* filename;
extern int token;
extern int line_num;



/***************������*****************************/

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
/*****************���Ӻ���****************/
void* mallocz(int size);
int elf_hash(char* key);				// �ַ���ϣ����
#endif // !SCC_H_

