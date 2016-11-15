/******************************************
*Author:Away
*Date:2016-10-25
*Function:ͨ�õ�ͷ�ļ�
*******************************************/

#ifndef ViaC_H_
#define ViaC_H_

#include<windows.h>
#include<stdio.h>

typedef int bool;
typedef unsigned int   DWORD;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
#define TRUE  1
#define FLASE 0

/********���嶯̬�ַ���********/
typedef struct DynString
{
	int count;		//�ַ�������
	int capacity;	//�ַ���������
	char* data;		//�ַ�ָ��
}DynString;
/******��̬�ַ�������***********/
void DynStringInit(DynString* pstr, const int initsize);		//��ʼ����̬�ַ���
void DynStringFree(DynString* pstr);					//�ͷŶ�̬�ַ���
void DynStringReset(DynString* pstr);
void DynStringChcat(DynString* pstr, const int ch);
void DynStringRealloc(DynString* pstr, const int newsize);


/************���嶯̬����*******/
typedef struct DynArray
{
	int count;
	int capacity;
	void** data;
}DynArray;
/************��̬���麯��*****/
void DynArrayInit(DynArray* parr, const int size);
void DynArrayRealloc(DynArray* parr, const int newsize);
void DynArrayAdd(DynArray* parr, void* data);
void DynArrayFree(DynArray* parr);
int  DynArrayFind(DynArray* parr, const int data);
void DynArrayDelete(DynArray* parr, const int i);
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
	TK_SPACE,							//
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
	KW_INCLUDE,
	KW_DO,
	KW_END,
	
	KW_REQUIRE,
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
	struct Symbol* sym_struct;
	struct Symbol* sym_id;
}TkWord;


#define MAXKEY 2048    

#define CH_EOF (-1)						//�ļ�β����ʶ

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
void ParseComment_2();
char* GetTkstr(const int v);
void TestLex(void);


/***************���ñ���****************************/
extern TkWord* tk_hashtable[MAXKEY];	//���ʹ�ϣ��
extern DynArray tktable;				//���ʶ�̬����
extern DynString tkstr;

extern char ch;
extern int tkvalue;
extern int token;


#define  OUT_FILE_SIZE 256      //����ļ��������С
extern FILE* fin;				//Դ�ļ�ָ��
extern char* filename;				//Դ�ļ���
extern int linenum;				//�к�
extern DynArray srcfiles;			//Դ�ļ�����
extern char outfile[OUT_FILE_SIZE];		//����ļ���
extern int outtype;          //����ļ�����
extern float ViaCVersion; //�������汾

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
void GetObjFname(const char* fname);

/*****************���Ӻ���****************/
void* MallocInit(const int size);
int ElfHash(char* key);				// �ַ���ϣ����
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
void* StackGetTop(Stack* stack);
bool StackIsEmpty(Stack* stack);
void StackDestroy(Stack* stack);
/******************end*************************/


/*******************SYM*************************/
Stack GSYM ; //ȫ�ַ���ջ
Stack LSYM ; //�ֲ�����ջ


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
void MkPointer(Type* ptype);
/********************end**********************/

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
	IMAGE_SECTION_HEADER  sh;//��ͷ
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

extern pSection sec_text, sec_data, sec_bss, sec_idata, sec_rdata, sec_rel, sec_symtab, sec_dynsymtab;

extern int nsec_image;

void SectionRealloc(pSection sec, const int newsize);
void*  SectionPtrAdd(const pSection sec, const int increment);
Section* SectionNew(const char* name, const int characteristics);
int CoffSymSearch(const pSection symtab, const char* name);
char* CoffStrAdd(const pSection strtab, const char* name);
int CoffSymAdd(const pSection symtab, const char* name, const int val, const int sec_index, const short type, const char StrorageClass);
void CoffSymAddUpdate(Symbol* ps, const int val, const int sec_index, const short type, const char StroageClass);
void FreeSection(void);
pSection NewCoffSymSection(const char* symtab_name, const int Characteristics, const char* strtab_name);
void CoffElocDirectAdd(const int offset, const int cfsym, const char section, const char type);
void CoffElocAdd(pSection sec, Symbol* sym, const int offset, const char type);
void InitCoff(void);
void Fpad(const FILE* fp, const int new_pos);
void WriteObj(const char* name);
/*********************end**************************/
/*********************operand.h*********************/
typedef struct Operand
{
	Type type;
	unsigned short reg;
	int value;
	struct Symbol* sym;
}Operand;

void OperandPush(Type* type, const int r, const int value);
void OperandPop(void);
void OperandSwap();
void OperandAssign(Operand* opd, const int t, const int r, const int value);
void CancelLvalue(void);
void CheckLvalue(void);
void Indirection(void);
/*********************end**************************/

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
	ViaC_GLOBAL = 0x00f0,
	ViaC_LOCAL = 0x00f1,
	ViaC_LLOACL = 0x00f2,
	ViaC_CMP = 0x00f3,
	ViaC_VALMASK = 0x00ff, //�洢��ַ����
	ViaC_LVAL = 0x0100,//��ֵ
	ViaC_SYM = 0x0200,//����

	ViaC_ANOM = 0x10000000,  //��������
	ViaC_STRUCT = 0x20000000,  //�ṹ��
	ViaC_MEMBER = 0x40000000,  //�ṹ��ĳ�Ա����
	ViaC_PARAMS = 0x80000000,  //��������
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
void Initializer(const Type* ptype, const int c, Section* psec); // ��ֵ��
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
void ContinueStatement(int* csym);
void BreakStatement(int* bsym);
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


/**********************gencode.h**************************/
enum e_Register
{
	REG_EAX = 0,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESP,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_ANY
};

#define REG_IRET REG_EAX

enum e_Address
{
	ADDR_OTHER,
	ADDR_REG =3,
};

#define OPSTACK_SIZE 256
#define FUNC_PROLOG_SIZE 9

extern int rsym;
extern int ind;
extern int loc;
extern int func_begin_ind;
extern int func_ret_sub;
extern Symbol* sym_sec_rdata;
extern Operand opstack[OPSTACK_SIZE];
extern Operand* optop;

void GenByte(const char c);  
void GenPrefix(const char opcode); 
void GenOpcode_1(const char opcode);
void GenOpcode_2(const char first, const char end);
void GenDword(unsigned int c);		
void BackPatch(int t, const int a); 
int MakeList(int add);  
void GenAddr32(const int r, const Symbol* sym, const int c);
void GenModrm(int mod, int reg_opcode, const int r_m, const Symbol* sym, const int c);
void Load(const int r, Operand* opd);	
void Store(const int r, Operand* opd); 
int Load_1(const int rc, Operand* opd);
void Load_2(const int lrc, const int rrc);
void Store_1(void);
void GenAddsp(const int val);
void GenCall(void);
void GenInvoke(const int nb_args);
void GenOpi_2(const int opc, const int op);
void GenOpi_1(int op);
Type* PointedType(Type* t);
int PointedSize(Type* t);
void GenOp(const int op);
int AllocateReg(const int rc);
void SpillReg(int reg);
void SpillRegs();
int GenJmpForWard(const int t);
void GenJmpBackWord(const int a);
int GenJcc(int t);
void GenProlog(Type* func_type);
void GenEpilog();
void InitVariable(const Type* ptype, const Section* psec, const int c, const int v);
Section* AllocateStorage(Type* ptype, const int reg, const int has_init, const int v, int* addr);

/***********************end*****************************/
/*************************outpe.h******/
typedef struct ImportSym
{
	int iat_index;
	int thk_offset;
	IMAGE_IMPORT_BY_NAME imp_sym;
}ImportSym;

/* ����ģ���ڴ�洢�ṹ */
typedef struct ImportInfo
{
	int dll_index;
	DynArray imp_syms;
	IMAGE_IMPORT_DESCRIPTOR imphdr;
}ImportInfo;

/* PE��Ϣ�洢�ṹ */
typedef struct PEInfo
{
	Section *thunk;
	const char *filename;
	DWORD entry_addr;
	DWORD imp_offs;
	DWORD imp_size;
	DWORD iat_offs;
	DWORD iat_size;
	Section **secs;
	int   sec_size;
	DynArray imps;
}PEInfo;

char* GetDllName(const char* libfile);
#endif // viac.h_




