#include"viac.h"

#ifndef		GRAMMAR_H_
#define 	GRAMMAR_H_

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
	ViaC_GLOBAL = 0x00f0,
	ViaC_LOCAL = 0x00f1,
	ViaC_LLOCAL = 0x00f2,
	ViaC_CMP = 0x00f3,
	ViaC_VALMASK = 0x00ff, //存储地址掩码
	ViaC_LVAL = 0x0100,//左值
	ViaC_SYM = 0x0200,//符号

	ViaC_ANOM = 0x10000000,  //匿名符号
	ViaC_STRUCT = 0x20000000,  //结构体
	ViaC_MEMBER = 0x40000000,  //结构体的成员变量
	ViaC_PARAMS = 0x80000000,  //函数参数
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
void Initializer(Type* ptype, const int c, Section* psec); // 初值符
int TypeSpecifier(Type* type);
void StructSpecifier(Type* type);
void StructDeclarationList(Type* type);
void StructDeclaration(int* maxalign, int* offset, Symbol*** ps);
void Declarator(Type* type, int* v, int* force_align);
void FunctionCallingConvention(int* fc);
void StructMemberAlignment(int* force_align);
void DirectDeclarator(Type* type, int* v, const int func_call);
void DirectDeclaratorPostfix(Type* type, const int func_call);
void ParameterTypeList(Type* type, int func_call);
void Funcbody(Symbol* sym);
int IsTypeSpecifier(const int id);
void Statement(int* bsym, int* csym);
void CompoundStatement(int* bsym, int* csym);
void IfStatement(int* bsym, int* csym);
void ForStatement(int* bsym, int* csym);
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


#endif