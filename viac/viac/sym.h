#include"viac.h"

#ifndef		SYM_H_
#define 	SYM_H_

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

Symbol* StructSearch(const int v);
Symbol* SymSearch(const int v);
Symbol* SymDirectPush(Stack* stack, const int v, const Type* type, const int c);
Symbol* SymPush(const int v, const Type* type, const int r, const int c);
Symbol* FuncSymPush(const int v, const Type *type);
Symbol *VarSymPut(const Type *type, const int r, const int v, const int addr);
Symbol* SecSymPut(const char* sec, const int c);
void SymPop(Stack* ptop, const Symbol* b); //b可以为NULL
int TypeSize(const Type* t, int* a);
void MkPointer(Type* ptype);
/********************end**********************/

#endif