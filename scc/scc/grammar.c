/******************************************
* Author：Away
* Date: 2016-10-30
* SCC语法分析:
* 递归自顶向下分析法，由于是上下文无关，有些
* 语法的限制需要语义控制程序去限制
*******************************************/

#include"scc.h"

int syntax_state;
int syntax_level;

void TranslationUnit(void)
{
	while (token != TK_EOF)
	{
		ExternalDeclaration(SC_GLOBAL);
	}
}

void ExternalDeclaration(const int level)
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol *sym;
	if (!TypeSpecifier(&btype))
	{
		Expect("<类型区分符>");
	}

	if (btype.t == T_STRUCT && token == TK_SEMICOLON)
	{
		GetToken();
		return;
	}

	while (1)
	{
		type = btype;
		Declarator(&type, &v, NULL);

		if (token == TK_BEGIN)
		{
			if (level == SC_LOCAL)
				Error("不允许嵌套定义");
			if ((type.t & T_BTYPE) != T_FUNC)
				Expect("<函数定义>");

			sym = SymSearch(v);
			if (sym)
			{
				if ((sym->type.t & T_BTYPE) != T_FUNC)
					Error("'%s'重定义", GetTkstr(v));
				sym->type = type;
			}
			else
				sym = FuncSymPush(v, &type);
			sym->r = SC_SYM | SC_GLOBAL;
			Funcbody(sym);
			break;
		}
		else
		{
			if ((type.t & T_BTYPE) == T_FUNC)
			{
				if (SymSearch(v) == NULL)
				{
					sym = SymPush(v, &type, SC_GLOBAL | SC_SYM, 0);
				}
			}
			else
			{
				r = 0;
				if (!(type.t & T_ARRAY))
					r |= SC_LVAL;
				r |= level;
				has_init = (token == TK_ASSIGN);

				if (has_init)
				{
					GetToken();
					Initializer(&type);
				}
				sym = VarSymPut(&type, r, v, addr);
			}
			if (token == TK_COMMA)
				GetToken();
			else
			{
				syntax_state = SNTX_LF_HT;
				Skip(TK_SEMICOLON);
				break;
			}
		}
	}
}

void Initializer(Type* type) // 初值符
{
	if (type->t & T_ARRAY)
	{
		GetToken();
	}
	else
		AssignmentExpression();
}

int TypeSpecifier(Type* type)
{
	int  t, type_found = 0;
	Type typel;
	t = 0;
	switch (token)
	{
		case KW_CHAR:
		{
			t = T_CHAR;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		}
		case KW_INT:
		{
			t = T_INT;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		}
		case KW_SHORT:
		{
			t = T_SHORT;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
		}
		case KW_VOID:
		{
			t = T_VOID;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		}
		case KW_STRUCT:
		{
			StructSpecifier(&typel);
			type->ref = typel.ref;
			t = T_STRUCT;

			syntax_state = SNTX_SP;
			type_found = 1;
			break;
		}
		default:
			break;
	}
	type->t = t;
	return type_found;
}

void StructSpecifier(Type* type)
{
	int t;
	Symbol* ps;
	Type typel;

	GetToken();
	t = token;

	syntax_state = SNTX_DELAY;
	GetToken();

	if (t < TK_IDENT)
		Expect("结构体名称");

	ps = StructSearch(t);
	if (!ps)
	{
		typel.t = KW_STRUCT;

		ps = SymPush(t | SC_STRUCT, &typel, 0, -1);
		ps->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = ps;

	if (token == TK_BEGIN)
	{
		StructDeclarationList(type);
	}

	if (token == TK_BEGIN)
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP;
	SyntaxIndent();
}

void StructDeclarationList(Type* type)
{
	int  maxalign, offset;
	Symbol *ps, **pps;
	ps = type->ref;

	syntax_state = SNTX_LF_HT; //第一个结构体的成员和"{"不在同一行
	++syntax_level;            //缩进进一行

	GetToken();
	if (ps->c != -1)
		Error("结构体已经定义");
	maxalign = 1;
	pps = &ps->next;
	offset = 0;
	while (token != TK_END)
	{
		StructDeclaration(&maxalign, &offset, &ps);
	}
	Skip(TK_END);
	syntax_state = SNTX_LF_HT;

	ps->c = CalcAlign(offset, maxalign);
	ps->r = maxalign;
	
}

void StructDeclaration(int* maxalign, int* offset, Symbol*** ps)
{
	int v, size, align;
	Symbol* pps;
	Type typel, btype;
	int force_align;
	TypeSpecifier(&btype);

	while (1)
	{
		v = 0;
		typel = btype;
		Declarator(&typel, &v, &force_align);
		size = TypeSize(&typel, &align);
		if (force_align  & ALIGN_SET)
		{
			align = force_align & ~ALIGN_SET;
		}
		*offset = CalcAlign(*offset, align);

		if (align > *maxalign)
		{
			*maxalign = align;
		}
		pps = SymPush(v | SC_MEMBER, &typel, 0, *offset);
		*offset += size;
		**ps == pps;
		*ps = &pps->next;

		if (token == TK_SEMICOLON || token == TK_EOF)
			break;
		Skip(TK_COMMA);
	}

	syntax_state = SNTX_LF_HT;

	Skip(TK_SEMICOLON);
}

void Declarator(Type* type, const int* v, const int* force_align)
{
	int fc;
	while (token == TK_STAR)
	{
		GetToken();
	}
	FunctionCallingConvention(&fc);
	if (force_align)
		StructMemberAlignment(force_align);
	DirectDeclarator(type, v, fc);
}

void FunctionCallingConvention(int* fc)
{
	*fc = KW_CDECL;
	if (token == -KW_CDECL || token == KW_STDCALL)
	{
		*fc = token;
		syntax_state = SNTX_SP;
		GetToken();
	}
}

void StructMemberAlignment(int* force_align)
{
	int align = 1;
	if (token == KW_ALIGN)
	{
		GetToken();
		Skip(TK_OPENPA);
		if (token == TK_CINT)
		{
			GetToken();
			align = tkvalue;
		}
		else
			Expect("整数常量");
		Skip(TK_CLOSEPA);

		if (align != 1 && align != 2 && align != 4)
			align = 1;
		align |= ALIGN_SET;
		*force_align = align;
	}
	else
		*force_align = 1;
}

void DirectDeclarator(Type* type, int* v,const int func_call)
{
	if (token >= TK_IDENT)
	{
		*v = token;
		GetToken();
	}
	else
	{
		Expect("标识符");
	}
	DirectDeclaratorPostfix(type, func_call);
}

void DirectDeclaratorPostfix(Type* type, const int func_call) // 直接声明符后缀
{
	int m;
	Symbol* ps;
	if (token == TK_OPENPA) //括号开始
	{
		ParameterTypeList(type, func_call);
	}
	else if (token == TK_OPENBR)  // 中括号
	{
		GetToken();
		m = -1;
		if (token == TK_CINT)
		{
			GetToken();
			m = tkvalue;
		}
		Skip(TK_CLOSEBR);
		DirectDeclaratorPostfix(type, func_call);//递归调用
		ps = SymPush(SC_ANOM, type, 0, m);
		type->t = T_ARRAY | T_PTR;
		type->ref = ps;
	}
}

void ParameterTypeList(Type* type, int func_call) //解析形参类型表
{
	int n;
	Symbol **plast, *s, *first;
	Type pt;

	GetToken();
	first = NULL;
	plast = &first;
	while (token != TK_CLOSEPA)
	{
		if (!TypeSpecifier(&pt))
		{
			Error("标识符无效");
		}
		Declarator(&pt, &n, NULL);
		s = SymPush(n | SC_PARAMS, &pt, 0, 0);
		*plast = s;
		plast = &s->next;

		if (token == TK_CLOSEPA)
			break;
		Skip(TK_COMMA);
		if (token == TK_ELLIPSIS)
		{
			func_call = KW_CDECL;
			GetToken();
			break;
		}
	}

	syntax_state = SNTX_DELAY;
	Skip(TK_CLOSEPA);
	if (token == TK_BEGIN)
	{
		syntax_state = SNTX_LF_HT;
	}
	else
	{
		syntax_state = SNTX_NUL;
	}
	SyntaxIndent();

	s = SymPush(SC_ANOM, type, func_call, 0);
	s->next = first;
	type->t = T_FUNC;
	type->ref = s;
}


void Funcbody(Symbol* sym)
{
	
	SymDirectPush(&LSYM, SC_ANOM, &int_type, 0);
	CompoundStatement(NULL, NULL);
	SymPop(&LSYM, NULL);
}

int IsTypeSpecifier(const int id)
{
	switch (id)
	{
		case KW_CHAR:
		case KW_SHORT:
		case KW_INT:
		case KW_VOID:
		case KW_STRUCT:
			return 1;
		default:
			break;
	}
	return 0;
}

void Statement(int* bsym, int* csym)
{
	switch (token)
	{
		case TK_BEGIN:
		{
			CompoundStatement(bsym, csym);
			break;
		}
		case KW_IF:
		{
			IfStatement(bsym, csym);
			break;
		}
		case KW_RETURN:
		{
			ReturnStatement();
			break;
		}
		case KW_BREAK:
		{
			BreakStatement(bsym);
			break;
		}
		case KW_CONTINUE:
		{
			ContinueStatement(csym);
			break;
		}
		case KW_FOR:
		{
			ForStatement(bsym, csym);
			break;
		}
		default:
		{
			ExpressionStatement();
			break;
		}
	}
}

void CompoundStatement(int* bsym, int* csym)
{
	Symbol* ps;
	ps = ( Symbol* ) StackgGetTop(&LSYM);

	syntax_state = SNTX_LF_HT;
	++syntax_level;

	GetToken();
	while (IsTypeSpecifier(token))
	{
		ExternalDeclaration(SC_LOCAL);
	}
	while (token != TK_END)
	{
		Statement(bsym, csym);
	}
	SymPop(&LSYM, ps);
	syntax_state = SNTX_LF_HT;
	GetToken();
}

void IfStatement(const int* bsym, const int* csym)
{
	int a, b;
	syntax_state = SNTX_SP;
	GetToken();
	Skip(TK_OPENPA);
	Expression();
	syntax_state = SNTX_LF_HT;
	Skip(TK_CLOSEPA);
	//a = GenJcc(0);
	Statement(bsym, csym);
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		GetToken();
		Statement(bsym, csym);
	}

}

void ForStatement(const int* bsym, const int* csym)
{
	int a, b, c, d, e;
	
	GetToken();
	Skip(TK_OPENPA);
	if (token != TK_SEMICOLON)
	{
		Expression();
	}
	Skip(TK_SEMICOLON);

	if (token != TK_SEMICOLON)
	{
		Expression();
	}
	Skip(TK_SEMICOLON);

	if (token != TK_CLOSEPA)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_CLOSEPA);
	Statement(&a, &b);  //只有此处用到break,continue可能有多个break或者是continue，需要拉链反填
}

void ContinueStatement(const int* csym)
{
	if (!csym)
		Error("未找到与continue相匹配的循环");
	GetToken();
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

void BreakStatement(const int* bsym)
{
	if (!bsym)
		Error("未找到与break相匹配的循环");
	GetToken();
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}


void ReturnStatement(void)
{
	syntax_state = SNTX_DELAY;
	GetToken();
	if (token == TK_SEMICOLON)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP;

	SyntaxIndent();

	if (token != TK_SEMICOLON)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

void ExpressionStatement(void)
{
	if (token != TK_SEMICOLON)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

void Expression(void)
{
	while (1)
	{
		AssignmentExpression();
		if (token != TK_COMMA)
			break;
		GetToken();
	}
}

/*左循环提取公因子*/
void AssignmentExpression(void)
{
	EqualityExpression();
	if (token == TK_ASSIGN)
	{
		GetToken();
		AssignmentExpression();
	}
}

void EqualityExpression(void)
{
	int t;
	RelationalExpression();
	while (token == TK_EQ || token == TK_NEQ)
	{
		t = token;
		GetToken();
		RelationalExpression();
	}
}

void RelationalExpression(void)
{
	int t;
	AdditiveExpression();
	while (token == TK_LT || token == TK_LEQ ||
		token == TK_GT || token == TK_GEQ)
	{
		t = token;
		GetToken();
		AdditiveExpression();
	}
}

void AdditiveExpression(void)
{
	int t;
	MultiplicativeExpression();
	while (token == TK_PLUS || token == TK_MINUS)
	{
		t = token;
		GetToken();
		MultiplicativeExpression();
	}
}

void MultiplicativeExpression(void)
{
	int t;
	UnaryExpression();
	while (token == TK_STAR || token == TK_DIVIDE || token == TK_MOD)
	{
		t = token;
		GetToken();
		UnaryExpression();
	}
}

void UnaryExpression(void)
{
	switch (token)
	{
		case TK_AND:
		{
			GetToken();
			UnaryExpression();
			break;
		}
		case TK_STAR:
		{
			GetToken();
			UnaryExpression();
			break;
		}
		case TK_PLUS:
		{
			GetToken();
			UnaryExpression();
		}
		case TK_MINUS:
		{
			GetToken();
			UnaryExpression();
			break;
		}
		case KW_SIZEOF:
		{
			SizeofExpression();
			break;
		}
		default:
		{
			PostfixExpression();
			break;
		}
	}
}

void SizeofExpression(void)
{
	int align, size; 
	Type type;

	GetToken();
	Skip(TK_OPENPA);
	TypeSpecifier(&type);
	
	Skip(TK_CLOSEPA);
	
	size = TypeSize(&type, &align);
	if (size < 0)
		Error("sizeof计算类型空间失败");
}

void PostfixExpression(void)
{
	Symbol* ps;
	PrimaryExpression();
	while (1)
	{
		if (token == TK_DOT || token == TK_POINTSTO)
		{
			GetToken();
			token |= SC_MEMBER;
			GetToken();
		}
		else if (token == TK_OPENBR)
		{
			GetToken();
			Expression();
			Skip(TK_CLOSEBR);
		}
		else if (token == TK_OPENPA)
		{
			ArgumentExpressionList();
		}
		else
			break;
	}
}

void PrimaryExpression(void)
{
	int id, r, addr;
	Type type;
	Symbol* ps;
	
	switch (token)
	{
		case TK_CINT:
		case TK_CCHAR:
		{
			GetToken();
			break;
		}
		case TK_CSTR:
		{
			id = T_CHAR; 
			type.t = id;
			MkPointer(&type);
			type.t |= T_ARRAY;
			VarSymPut(&type, SC_GLOBAL, 0, addr);
			Initializer(&type);
		    //TODO	GetToken();
			break;
		}
		case TK_OPENPA:
		{
			GetToken();
			Expression();
			Skip(TK_CLOSEPA);
			break;
		}
		default:
		{
			id = token;
			GetToken();
			if (id < TK_IDENT)
				Expect("常量或者是标识符");
			ps = SymSearch(id);
			if (!ps)
			{
				if (token == TK_OPENPA)
					Error("'%s'未声明\n", GetTkstr(id));
				ps = FuncSymPush(id, &default_func_type);
				ps->r = SC_GLOBAL | SC_SYM;
			}
			r = ps->r;
			break;
		}
	}
}

void ArgumentExpressionList(void)
{
	GetToken();
	if (token != TK_CLOSEPA)
	{
		while (1)
		{
			AssignmentExpression();
			if (token == TK_CLOSEPA)
				break;
			Skip(TK_COMMA);
		}
	}
	Skip(TK_CLOSEPA);
}

void PrintTab(const int num)
{
	int count;
	for (count = 0; count < num; ++count)
	{
		printf("\t");
	}
}

void SyntaxIndent(void)
{
	switch (syntax_state)
	{
		case  SNTX_NUL:
		{
			ColorToken(LEX_NORMAL);
			break;
		}
		case  SNTX_SP:
		{
			printf(" ");
			ColorToken(LEX_NORMAL);
			break;
		}
		case SNTX_LF_HT:
		{
			if (token == TK_END)
				--syntax_level;
			printf("\n");
			PrintTab(syntax_level);
			ColorToken(LEX_NORMAL);
			break;
		}
		case SNTX_DELAY:
			break;
	}
	syntax_state = SNTX_NUL;
}
