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

Symbol *SymPush(const int v, const Type *type, const int r, const int c)
{
	if (type == NULL )
		Error("symbol中指针未初始化");
	Symbol *ps, **pps;
	TkWord *ts;
	Stack *ss;

	if (StackIsEmpty(&LSYM) == 0)
	{
		ss = &LSYM;
	}
	else
	{
		ss = &GSYM;
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
	Symbol *s, **ps;
	s = SymDirectPush(&GSYM, v, type, 0);

	ps = &((TkWord*)tktable.data[v])->sym_id;
	// 同名符号，函数符号放在最后-> ->...s
	while (*ps != NULL)
		ps = &(*ps)->prev_tok;
	s->prev_tok = NULL;
	*ps = s;
	return s;
}

Symbol *VarSymPut(const Type *type, const int r, const int v, const int addr)
{
	Symbol *sym = NULL;
	if ((r & ViaC_VALMASK) == ViaC_LOCAL)			// 局部变量
	{

		sym = SymPush(v, type, r, addr);
	}
	else if (v && (r & ViaC_VALMASK) == ViaC_GLOBAL) // 全局变量
	{
		sym = SymSearch(v);
		if (sym)
			Error("%s重定义\n", ((TkWord*)tktable.data[v])->spelling);
		else
		{
			sym = SymPush(v, type, r | ViaC_SYM, 0);
		}
	}
	//else 字符串常量符号
	return sym;
}
Symbol* SecSymPut(const char* sec, const int c)
{
	TkWord* tp;
	Symbol* s; 
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
	Symbol *ps, **pps;
	TkWord *ts;
	
	ps = ( Symbol* ) StackGetTop(ptop);
	while (ps != b)
	{
		v = ps->v; 
		if ((v & ViaC_STRUCT) || v < ViaC_ANOM)
		{
			ts = ( TkWord* ) tktable.data [v & ~ViaC_STRUCT];
			if (v & ViaC_STRUCT)
				pps = &ts->sym_struct;
			else
				pps = &ts->sym_id;
			*pps = ps->prev_tok;
		}
		StackPop(ptop);
		ps = ( Symbol* ) StackGetTop(ptop);
	}
}

void MkPointer(Type* ptype)
{
	Symbol* psym;
	psym = SymPush(ViaC_ANOM, ptype, 0, -1);
	ptype->t = T_PTR; 
	ptype->ref = psym;
}
 
int TypeSize(const Type* t, int* a)
{
	Symbol* ps; 
	int bt;
	int PTR_SIZE = 4;

	bt = t->t & T_BTYPE; 
	switch (bt)
	{
		case T_STRUCT:
		{
			ps = t->ref;
			*a = ps->r;
			return ps->c;
		}
		case T_PTR:
		{
			if (t->t & T_ARRAY)
			{
				ps = t->ref;
				return TypeSize(&ps->type, a) * (ps->c);
			}
			else
			{
				*a = PTR_SIZE;
				return PTR_SIZE;
			}
		}
		case T_INT:
		{
			*a = 4;
			return 4;
		}
		case T_CHAR:
		{
			*a = 1;
			return 1;
		}
		case T_SHORT:
		{
			*a = 2;
			return 2;
		} 
		default:
		{
			*a = 1; 
			return 1;
		}
	}
	return 0;
}