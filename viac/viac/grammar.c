/******************************************
* Author：Away
* Date: 2016-10-30
* Function:SCC语法分析
* Note:递归自顶向下分析法，由于是上下文无关，
有些语法的限制需要语义控制程序去限制
*******************************************/

#include"viac.h"

int syntax_state;
int syntax_level;

void TranslationUnit(void)
{
	while (token != TK_EOF)
	{
		ExternalDeclaration(ViaC_GLOBAL);
	}
}

void ExternalDeclaration(const int level)
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol*  sym = NULL;
	Section* psec = NULL;

	if (!TypeSpecifier(&btype))
	{
		Expect("<类型区分符>");
	}

	if (btype.t == T_STRUCT && (token == TK_SEMICOLON || token == TK_SPACE))
	{
		GetToken();
		return;
	}
	while (1)
	{
		type = btype;
		Declarator(&type, &v, NULL);

		if (token == TK_BEGIN || token == KW_DO)
		{
			if (level == ViaC_LOCAL)
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
			sym->r = ViaC_SYM | ViaC_GLOBAL;
			Funcbody(sym);
			break;
		}
		else
		{
			if ((type.t & T_BTYPE) == T_FUNC)
			{
				if (SymSearch(v) == NULL)
				{
					sym = SymPush(v, &type, ViaC_GLOBAL | ViaC_SYM, 0);
				}
			}
			else
			{
				r = 0;
				if (!(type.t & T_ARRAY))
				{
					r |= ViaC_LVAL;
				}

				r |= level;
				has_init = (token == TK_ASSIGN);

				if (has_init)
				{
					GetToken();
				}
				psec = AllocateStorage(&type, r, has_init, v, &addr);
				sym = VarSymPut(&type, r, v, addr);

				if (level == ViaC_GLOBAL)
					CoffSymAddUpdate(sym, addr, psec->index, 0, IMAGE_SYM_CLASS_EXTERNAL);
				if (has_init)
				{
					Initializer(&type, addr, psec);
				}
			}
			if (token == TK_COMMA)
				GetToken();
			else
			{
				syntax_state = SNTX_LF_HT;
				if (token == TK_SEMICOLON)
					Skip(TK_SEMICOLON);
				else if (token == TK_SPACE)
					Skip(TK_SPACE);
				break;
			}
		}
	}
}

void Initializer(Type* ptype, const int c, Section* psec) // 初值符
{
	if (ptype->t & T_ARRAY && psec)
	{
		memcpy_s(psec->data + c, tkstr.count, tkstr.data, tkstr.count);
		GetToken();
	}
	else
	{
		AssignmentExpression();
		InitVariable(ptype, psec, c, 0);
	}

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
			syntax_state = SNTX_SP;

			StructSpecifier(&typel);
			type->ref = typel.ref;
			t = T_STRUCT;
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

	if (token == TK_BEGIN || token == KW_DO)
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP;
	SyntaxIndent();

	if (t < TK_IDENT)
		Expect("结构体名称");

	ps = StructSearch(t);
	if (!ps)
	{
		typel.t = KW_STRUCT;

		ps = SymPush(t | ViaC_STRUCT, &typel, 0, -1);
		ps->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = ps;

	if (token == TK_BEGIN || token == KW_DO)
	{
		StructDeclarationList(type);
	}
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
		Error("结构体已定义");
	maxalign = 1;
	pps = &ps->next;
	offset = 0;
	while (token != TK_END && token != KW_END)
	{
		StructDeclaration(&maxalign, &offset, &pps);
	}
	if (token == TK_END)
		Skip(TK_END);
	else if (token == KW_END)
		Skip(KW_END);
	syntax_state = SNTX_LF_HT;

	ps->c = CalcAlign(offset, maxalign);
	ps->r = maxalign;

}

void StructDeclaration(int* maxalign, int* offset, Symbol*** ps)
{
	int v, size, align;
	Symbol* psym;
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
		psym = SymPush(v | ViaC_MEMBER, &typel, 0, *offset);
		*offset += size;
		**ps = psym;
		*ps = &psym->next;

		if (token == TK_SEMICOLON || token == TK_EOF || token == TK_SPACE)
			break;
		Skip(TK_COMMA);
	}

	syntax_state = SNTX_LF_HT;
	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);

}

void Declarator(Type* type, int* v, int* force_align)
{
	int fc;
	while (token == TK_STAR)
	{
		MkPointer(type);
		GetToken();
	}
	FunctionCallingConvention(&fc);
	if (force_align)
		StructMemberAlignment(force_align);
	DirectDeclarator(type, v, fc);
}


void FunctionCallingConvention(int *fc)
{
	*fc = KW_CDECL;
	if (token == KW_CDECL || token == KW_STDCALL)
	{
		*fc = token;
		syntax_state = SNTX_SP;
		GetToken();
	}
}

void StructMemberAlignment(int *force_align)
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

void DirectDeclarator(Type* type, int* v, const int func_call)
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

void DirectDeclaratorPostfix(Type* type, const int func_call)
{
	if (type == NULL)
	{
		Error("指针未初始化");
	}
	int n;
	Symbol *s;

	if (token == TK_OPENPA)
	{
		ParameterTypeList(type, func_call);
	}
	else if (token == TK_OPENBR)
	{
		GetToken();
		n = -1;
		if (token == TK_CINT)
		{
			GetToken();
			n = tkvalue;
		}
		Skip(TK_CLOSEBR);
		DirectDeclaratorPostfix(type, func_call);
		s = SymPush(ViaC_ANOM, type, 0, n);
		type->t = T_ARRAY | T_PTR;
		type->ref = s;
	}
}

void ParameterTypeList(Type *type, int func_call)
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
			Error("无效类型标识符");
		}
		Declarator(&pt, &n, NULL);
		s = SymPush(n | ViaC_PARAMS, &pt, 0, 0);
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
	if (token == TK_BEGIN || token == KW_DO)			// 函数定义
		syntax_state = SNTX_LF_HT;
	else							// 函数声明
		syntax_state = SNTX_NUL;
	SyntaxIndent();

	// 此处将函数返回类型存储，然后指向参数，最后将type设为函数类型，引用的相关信息放在ref中
	s = SymPush(ViaC_ANOM, type, func_call, 0);
	s->next = first;
	type->t = T_FUNC;
	type->ref = s;
}


void Funcbody(Symbol *sym)
{
	ind = sec_text->data_offset;
	CoffSymAddUpdate(sym, ind, sec_text->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
	/* 放一匿名符号在局部符号表中 */
	SymDirectPush(&LSYM, ViaC_ANOM, &int_type, 0);
	GenProlog(&sym->type);
	rsym = 0;
	CompoundStatement(NULL, NULL);
	BackPatch(rsym, ind);
	GenEpilog();
	sec_text->data_offset = ind;
	SymPop(&LSYM, NULL); /* 清空局部符号栈*/
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
		case KW_DO:
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
	ps = (Symbol*)StackGetTop(&LSYM);

	syntax_state = SNTX_LF_HT;
	++syntax_level;

	GetToken();
	while (IsTypeSpecifier(token))
	{
		ExternalDeclaration(ViaC_LOCAL);
	}
	while (token != TK_END && token != KW_END)
	{
		Statement(bsym, csym);
	}
	SymPop(&LSYM, ps);
	syntax_state = SNTX_LF_HT;
	GetToken();
}

void IfStatement(int* bsym, int* csym)
{
	int a, b;
	syntax_state = SNTX_SP;
	GetToken();
	Skip(TK_OPENPA);
	Expression();
	syntax_state = SNTX_LF_HT;
	Skip(TK_CLOSEPA);
	a = GenJcc(0);
	Statement(bsym, csym);
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		GetToken();

		b = GenJmpForWard(0);
		BackPatch(a, ind);
		Statement(bsym, csym);
		BackPatch(b, ind);
	}
	else
		BackPatch(a, ind);
}

void ForStatement(int* bsym, int* csym)
{
	int a, b, c, d, e;

	GetToken();
	Skip(TK_OPENPA);
	if (token != TK_SEMICOLON && token != TK_SPACE)
	{
		Expression();
		OperandPop();
	}
	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);
	d = ind;
	c = ind;
	a = 0;
	b = 0;
	if (token != TK_SEMICOLON && token != TK_SPACE)
	{
		Expression();
		a = GenJcc(0);
	}

	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);
	else
		Expect("缺少***");
	if (token != TK_CLOSEPA)
	{
		e = GenJmpForWard(0);
		c = ind;
		Expression();
		OperandPop();
		GenJmpBackWord(d);
		BackPatch(e, ind);
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_CLOSEPA);
	Statement(&a, &b);  //拉链反填
	GenJmpBackWord(c);
	BackPatch(a, ind);
	BackPatch(b, c);
}

void ContinueStatement(int* csym)
{
	if (!csym)
		Error("未找到与continue相匹配的循环");
	*csym = GenJmpForWard(*csym);

	GetToken();
	syntax_state = SNTX_LF_HT;
	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);
}

void BreakStatement(int* bsym)
{
	if (!bsym)
		Error("未找到与break相匹配的循环");
	*bsym = GenJmpForWard(*bsym);
	GetToken();
	syntax_state = SNTX_LF_HT;
	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);
}

void ReturnStatement(void)
{
	syntax_state = SNTX_DELAY;
	GetToken();
	if (token == TK_SEMICOLON || token == TK_SPACE)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP;

	SyntaxIndent();

	if (token != TK_SEMICOLON && token != TK_SPACE)
	{
		Expression();
		Load_1(REG_IRET, optop);
		OperandPop();
	}
	syntax_state = SNTX_LF_HT;
	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);
	rsym = GenJmpForWard(rsym);
}

void ExpressionStatement(void)
{
	if (token != TK_SEMICOLON && token != TK_SPACE)
	{
		Expression();
		OperandPop();
	}
	syntax_state = SNTX_LF_HT;
	if (token == TK_SEMICOLON)
		Skip(TK_SEMICOLON);
	else if (token == TK_SPACE)
		Skip(TK_SPACE);
}

void Expression(void)
{
	while (1)
	{
		AssignmentExpression();
		if (token != TK_COMMA)
			break;
		OperandPop();
		GetToken();
	}
}

/*左循环提取公因子*/
void AssignmentExpression(void)
{
	EqualityExpression();
	if (token == TK_ASSIGN)
	{
		CheckLvalue();
		GetToken();
		AssignmentExpression();
		Store_1();
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
		GenOp(t);
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
		GenOp(t);
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
		GenOp(t);
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
		GenOp(t);
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
			if ((optop->type.t & T_BTYPE) != T_FUNC  && !(optop->type.t & T_ARRAY))
			{
				CancelLvalue();
			}
			MkPointer(&optop->type);
			break;
		}
		case TK_STAR:
		{
			GetToken();
			UnaryExpression();
			Indirection();
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
			OperandPush(&int_type, ViaC_GLOBAL, 0);
			UnaryExpression();
			GenOp(TK_MINUS);
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
	OperandPush(&int_type, ViaC_GLOBAL, size);
}

void PostfixExpression(void)
{
	Symbol* ps;
	PrimaryExpression();
	while (1)
	{
		if (token == TK_DOT || token == TK_POINTSTO)
		{
			if (token == TK_POINTSTO)
				Indirection();
			CancelLvalue();
			GetToken();
			if ((optop->type.t & T_BTYPE) != T_STRUCT)
				Expect("结构体变量");
			ps = optop->type.ref;
			token |= ViaC_MEMBER;
			while ((ps = ps->next) != NULL)
			{
				if (ps->v == token)
					break;
			}
			if (!ps)
				Error("没有此成员变量：%s", GetTkstr(token  & ~ViaC_MEMBER));
			optop->type = char_pointer_type;
			OperandPush(&int_type, ViaC_GLOBAL, ps->c);
			GenOp(TK_PLUS);
			optop->type = ps->type;
			if (!(optop->type.t & T_ARRAY))
			{
				optop->reg |= ViaC_LVAL;
			}
			GetToken();
		}
		else if (token == TK_OPENBR)
		{
			GetToken();
			Expression();
			GenOp(TK_PLUS);
			Indirection();
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
	Symbol* ps = NULL;
	pSection psec = NULL;

	switch (token)
	{
		case TK_CINT:
		case TK_CCHAR:
		{
			OperandPush(&int_type, ViaC_GLOBAL, tkvalue);
			GetToken();
			break;
		}
		case TK_CSTR:
		{
			id = T_CHAR;
			type.t = id;
			MkPointer(&type);
			type.t |= T_ARRAY;
			psec = AllocateStorage(&type, ViaC_GLOBAL, 2, 0, &addr);
			VarSymPut(&type, ViaC_GLOBAL, 0, addr);
			Initializer(&type, addr, psec);
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
				if (token != TK_OPENPA)
					Error("'%s'未声明\n", GetTkstr(id));
				ps = FuncSymPush(id, &default_func_type);
				ps->r = ViaC_GLOBAL | ViaC_SYM;
			}
			r = ps->r;
			OperandPush(&ps->type, r, ps->c);
			if (optop->reg & ViaC_SYM)
			{
				optop->sym = ps;
				optop->value = 0;
			}
			break;
		}
	}
}

void ArgumentExpressionList(void)
{
	Operand opd;
	Symbol* s = NULL;
	Symbol* sa = NULL;
	int nb_args = 0;
	s = optop->type.ref;

	GetToken();

	sa = s->next;
	nb_args = 0;
	opd.value = 0;
	opd.reg = REG_IRET;
	opd.type = s->type;

	if (token != TK_CLOSEPA)
	{
		while (1)
		{
			AssignmentExpression();
			nb_args++;
			if (sa)
				sa = sa->next;
			if (token == TK_CLOSEPA)
				break;
			Skip(TK_COMMA);
		}
	}
	if (sa)
		Error("实参个数少于形参个数");
	Skip(TK_CLOSEPA);
	GenInvoke(nb_args);

	OperandPush(&opd.type, opd.reg, opd.value);
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
			if (token == TK_END || token == KW_END)
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
