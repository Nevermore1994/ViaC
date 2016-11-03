/******************************************
* Author：Away
* Date: 2016-11-3
*******************************************/


#include"scc.h"


Symbol* StructSearch(int v)
{
	if (v >= tktable.count)
		return NULL;
	else
		return (( TkWord* ) tktable.data [v])->sym_struct;
}

Symbol* SymSearch(int v)
{
	if (v >= tktable.count)
		return NULL;
	else
		return (( TkWord* ) tktable.data [v])->sym_id;
}

Symbol* SymDirectPush(Stack* stack, int v, Type* type, int c)
{
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	Symbol s, *p;
	s.v = v;
	s.type.t = type->t;
	s.type.ref = type->ref;
	s.c = c; 
	s.next = NULL; 
	p = ( Symbol* ) StackPush(stack, &s, sizeof(Symbol)); 
	return p;
}


Symbol* SymPush(int v, Type* type, int r, int c)
{
	Symbol *ps, **pps; 
	TkWord* ts;
	Stack** ss;
	if (StackIsEmpty(&LSYM) == FALSE)
	{
		ss = &LSYM;
	} 
	else
	{
		ss = &GSYM;
	} 
	ps = SymDirectPush(ss, v, type, c); 
	ps->r = r;
	
	if ((v & SC_STRUCT) || v < SC_ANOM)
	{
		ts = ( TkWord* ) tktable.data [(v & ~SC_STRUCT)]; 
		if (v & SC_STRUCT)
			pps = &ts->sym_struct;
		else
			pps = &ts->sym_id;
		ps->prev_tok = *pps;
		*pps = ps;
	}
	return ps;
}
 
Symbol* FuncSymPush(int v, Type *type)
{
	Symbol *ps, **pps;
	ps = SymDirectPush(&GSYM, v, type, 0);
	
	pps = & (( TkWord* ) tktable.data [v])->sym_id;
	
	//同名符号函数符号放在最后
	while (*pps != NULL)
		pps = &(*pps)->prev_tok;

	ps->prev_tok = NULL;
	*pps = ps;
	return ps;
} 

Symbol *VarSymPut(Type* type, int r, int v, int addr)
{
	Symbol *sym = NULL; 
	if ( (r&SC_VALMASK) == SC_LOCAL)
	{
		sym = SymPush(v, type, r,addr);
	}
	else if (v && (r & SC_VALMASK) == SC_GLOBAL)
	{
		sym = SymSearch(v);
		if (sym)
			Error("%s重定义\n", (( TkWord* ) tktable.data [v])->spelling);
		else
			sym = SymPush(v, type, r | SC_SYM, 0);
	}
	return sym;
}

Symbol* SecSymPut(char* sec, int c)
{
	TkWord* tp;
	Symbol* s; 
	Type type;
	type.t = T_INT;
	tp = TkwordInsert(sec);
	token = tp->tkcode;
	s = SymPush(token, &type, SC_GLOBAL, c);
	return s;
}

void SymPop(Stack* stack, Symbol *b)
{
	if (stack == NULL || b == NULL)
	{
		Error("指针未初始化");
	}
	
	int v;
	Symbol *ps, **pps;
	TkWord *ts;
	
	ps = ( Symbol* ) StackgGetTop(stack);
	while (ps != b)
	{
		v = ps->v; 
		if ((v & SC_STRUCT) || v < SC_ANOM)
		{
			ts = ( TkWord* ) tktable.data [v & ~SC_STRUCT];
			if (v & SC_STRUCT)
				pps = &ts->sym_struct;
			else
				pps = &ts->sym_id;
			*pps = ps->prev_tok;
		}
		StackPop(stack);
		ps = ( Symbol* ) StackgGetTop(stack);
	}
}
 
int TypeSize(Type *t, int *a)
{
	Symbol* ps; 
	int bt;
	int PTR_SIZE = 4;

	bt = t->t | T_BTYPE; 
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