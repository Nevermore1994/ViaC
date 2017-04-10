/******************************************
* Author：Away
* Date: 2016-11-3
* Function: 符号分析模块单元代码
*******************************************/


#include"viac.h"


Symbol *StructSearch(const int v)
{
	if (v >= tktable.count)
		return NULL;
	else
		return ((TkWord*)tktable.data[v])->sym_struct;
}


Symbol *SymSearch(const int v)
{
	if (v >= tktable.count)
		return NULL;
	else
		return ((TkWord*)tktable.data[v])->sym_id;
}


Symbol* SymDirectPush(Stack *ss, const int v, const Type *type, const int c)
{
	if (type == NULL || ss == NULL)
		Error("symbol中指针未初始化");
	Symbol s, *p;
	s.v = v;
	s.type.t = type->t;
	s.type.ref = type->ref;
	s.c = c;
	s.next = NULL;
	p = (Symbol*)StackPush(ss, &s, sizeof(Symbol));
	return p;
}

Symbol* SymPush(const int v, const Type *type, const int r, const int c)
{
	if (type == NULL )
		Error("symbol中指针未初始化");
	Symbol *ps, **pps;
	TkWord *ts;
	Stack *ss;

	if (StackIsEmpty(&L_Sym) == 0)
	{
		ss = &L_Sym;
	}
	else
	{
		ss = &G_Sym;
	}
	ps = SymDirectPush(ss, v, type, c);
	ps->r = r;

	// 不记录结构体成员及匿名符号
	if ((v & ViaC_STRUCT) || v < ViaC_ANOM)
	{
		// 更新单词sym_struct或sym_id字段
		ts = (TkWord*)tktable.data[(v & ~ViaC_STRUCT)];
		if (v & ViaC_STRUCT)
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_id;
		ps->prev_tok = *pps;
		*pps = ps;
	}
	return ps;
}

 
Symbol* FuncSymPush(const int v, const Type *type)
{
	Symbol*  psym = NULL;
	Symbol** ppsym = NULL;
	psym = SymDirectPush(&G_Sym, v, type, 0);

	ppsym = &((TkWord*)tktable.data[v])->sym_id;
	// 同名符号，函数符号放在最后-> ->...s
	while (*ppsym != NULL)
		ppsym = &(*ppsym)->prev_tok;
	psym->prev_tok = NULL;
	*ppsym = psym;
	return psym;
}

Symbol* VarSymPut(const Type *type, const int r, const int v, const int addr)
{
	if (type == NULL)
		Error("symbol中有指针未初始化"); 
	Symbol *psym = NULL;
	if ((r & ViaC_VALMASK) == ViaC_LOCAL)			// 局部变量
	{

		psym = SymPush(v, type, r, addr);
	}
	else if (v && (r & ViaC_VALMASK) == ViaC_GLOBAL) // 全局变量
	{
		psym = SymSearch(v);
		if (psym)
			Error("%s重定义\n", ((TkWord*)tktable.data[v])->spelling);
		else
		{
			psym = SymPush(v, type, r | ViaC_SYM, 0);
		}
	}
	//else 字符串常量符号
	return psym;
}
Symbol* SecSymPut(const char* sec, const int c)
{
	if (sec == NULL)
		Error("symbol中有指针未初始化");
	TkWord* tp = NULL;
	Symbol* s = NULL; 
	Type type;
	type.t = T_INT;
	tp = TkwordInsert(sec);
	token = tp->tkcode;
	s = SymPush(token, &type, ViaC_GLOBAL, c);
	return s;
}

void SymPop(Stack* ptop, const Symbol* b) //b可以为NULL
{
	if (ptop == NULL)
		Error("指针未初始化");
	int v;
	Symbol*  psym = NULL;
	Symbol** ppsym = NULL;
	TkWord *ts;
	
	psym = ( Symbol* ) StackGetTop(ptop);
	while (psym != b)
	{
		v = psym->v; 
		if ((v & ViaC_STRUCT) || v < ViaC_ANOM)
		{
			ts = ( TkWord* ) tktable.data [v & ~ViaC_STRUCT];
			if (v & ViaC_STRUCT)
				ppsym = &ts->sym_struct;
			else
				ppsym = &ts->sym_id;
			*ppsym = psym->prev_tok;
		}
		StackPop(ptop);
		psym = ( Symbol* ) StackGetTop(ptop);
	}
}

void MkPointer(Type* ptype)
{
	Symbol* psym = NULL;
	psym = SymPush(ViaC_ANOM, ptype, 0, -1);
	ptype->t = T_PTR; 
	ptype->ref = psym;
}
 
int TypeSize(const Type* ptype, int* pint)
{
	if (ptype == NULL)
		Error("symbol有指针未初始化");
	Symbol* psym; 
	int bt;
	int PTR_SIZE = 4;

	bt = ptype->t & T_BTYPE; 
	switch (bt)
	{
		case T_STRUCT:
		{
			psym = ptype->ref;
			*pint = psym->r;
			return psym->c;
		}
		case T_PTR:
		{
			if (ptype->t & T_ARRAY)
			{
				psym = ptype->ref;
				return TypeSize(&psym->type, pint) * (psym->c);
			}
			else
			{
				*pint = PTR_SIZE;
				return PTR_SIZE;
			}
		}
		case T_INT:
		{
			*pint = 4;
			return 4;
		}
		case T_CHAR:
		{
			*pint = 1;
			return 1;
		}
		case T_SHORT:
		{
			*pint = 2;
			return 2;
		} 
		default:
		{
			*pint = 1; 
			return 1;
		}
	}
	return 0;
}