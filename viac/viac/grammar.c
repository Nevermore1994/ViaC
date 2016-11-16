// 《自己动手写编译器、链接器》配套源代码

#include "viac.h"
int syntax_state;
int syntax_level;

/***********************************************************
* 功能:	解析翻译单位
*
* <TranslationUnit>::={ExternalDeclaration}<TK_EOF>
**********************************************************/
void TranslationUnit()
{
	while (token != TK_EOF)
	{
		ExternalDeclaration(ViaC_GLOBAL);
	}
}

/***********************************************************
* 功能:	解析外部声明
* l:		存储类型，局部的还是全局的
*
* <ExternalDeclaration>::=<function_definition>|<declaration>
*
* <function_definition>::= <TypeSpecifier> <Declarator><Funcbody>
*
* <declaration>::= <TypeSpecifier><TK_SEMICOLON>
*		|<TypeSpecifier>< init_Declarator_list><TK_SEMICOLON>
*
* <init_Declarator_list>::=
*      <init_Declarator>{<TK_COMMA> <init_Declarator>}
*
* <init_Declarator>::=
*      <Declarator>|<Declarator> <TK_ASSIGN><Initializer>
*
* 改写后文法：
* <ExternalDeclaration>::=
*  <TypeSpecifier> (<TK_SEMICOLON>
*      |<Declarator><Funcbody>
*	    |<Declarator>[<TK_ASSIGN><Initializer>]
*	     {<TK_COMMA> <Declarator>[<TK_ASSIGN><Initializer>]}
*		 <TK_SEMICOLON>
**********************************************************/
void ExternalDeclaration(int l)
{
	Type btype, type;
	int v, has_init, r, addr;
	Symbol *sym;
	Section *sec = NULL;

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

		if (token == TK_BEGIN) //函数定义
		{
			if (l == ViaC_LOCAL)
				Error("不支持函数嵌套定义");

			if ((type.t & T_BTYPE) != T_FUNC)
				Expect("<函数定义>");

			sym = SymSearch(v);
			if (sym)	// 函数前面声明过，现在给出函数定义
			{
				if ((sym->type.t & T_BTYPE) != T_FUNC)
					Error("'%s'重定义", GetTkstr(v));
				sym->type = type;
			}
			else
			{
				sym = FuncSymPush(v, &type);
			}
			sym->r = ViaC_SYM | ViaC_GLOBAL;
			Funcbody(sym);
			break;
		}
		else
		{
			if ((type.t & T_BTYPE) == T_FUNC) // 函数声明
			{
				if (SymSearch(v) == NULL)
				{
					SymPush(v, &type, ViaC_GLOBAL | ViaC_SYM, 0);
				}
			}
			else //变量声明
			{
				r = 0;
				if (!(type.t & T_ARRAY))
					r |= ViaC_LVAL;

				r |= l;
				has_init = (token == TK_ASSIGN);

				if (has_init)
				{
					GetToken(); //不能放到后面，char str[]="abc"情况，需要AllocateStorage求字符串长度				    
				}

				sec = AllocateStorage(&type, r, has_init, v, &addr);
				sym = VarSymPut(&type, r, v, addr);
				if (l == ViaC_GLOBAL)
					CoffSymAddUpdate(sym, addr, sec->index, 0, IMAGE_SYM_CLASS_EXTERNAL);

				if (has_init)
				{
					Initializer(&type, addr, sec);
				}
			}
			if (token == TK_COMMA)
			{
				GetToken();
			}
			else
			{
				syntax_state = SNTX_LF_HT;
				Skip(TK_SEMICOLON);
				break;
			}
		}
	}
}


/***********************************************************
* 功能:	解析初值符
* type:	变量类型
* c:		变量相关值
* sec:		变量所在节
*
* < Initializer>::=<AssignmentExpression>
**********************************************************/
void Initializer(Type *type, int c, Section *sec)
{
	if (type->t & T_ARRAY && sec)
	{
		memcpy(sec->data + c, tkstr.data, tkstr.count);
		GetToken();
	}
	else
	{
		AssignmentExpression();
		InitVariable(type, sec, c, 0);
	}
}

/***********************************************************
* 功能:		类型区分符
* type(输出):	数据类型
* 返回值:		是否发现合法的类型区分符
*
* <TypeSpecifier>::= <KW_INT>
*		| <KW_CHAR>
*		| <KW_SHORT>
*		| <KW_VOID >
*		| <StructSpecifier>
**********************************************************/
int TypeSpecifier(Type *type)
{
	int t, type_found;
	Type type1;
	t = 0;
	type_found = 0;
	switch (token)
	{
		case KW_CHAR:
			t = T_CHAR;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		case KW_SHORT:
			t = T_SHORT;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		case KW_VOID:
			t = T_VOID;
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		case KW_INT:
			t = T_INT;
			syntax_state = SNTX_SP;
			type_found = 1;
			GetToken();
			break;
		case KW_STRUCT:
			syntax_state = SNTX_SP;
			StructSpecifier(&type1);
			type->ref = type1.ref;
			t = T_STRUCT;
			type_found = 1;
			break;
		default:
			break;
	}
	type->t = t;
	return type_found;
}

/***********************************************************
* 功能:		解析结构区分符
* type(输出):	结构类型
*
* <StructSpecifier>::=
*	<KW_STRUCT><IDENTIFIER><TK_BEGIN><StructDeclarationList><TK_END>
*		| <KW_STRUCT><IDENTIFIER>
**********************************************************/
void StructSpecifier(Type *type)
{
	int v;
	Symbol *s;
	Type type1;

	GetToken();
	v = token;

	syntax_state = SNTX_DELAY;      // 新取单词不即时输出，延迟到取出单词后根据单词类型判断输出格式
	GetToken();

	if (token == TK_BEGIN)			// 适用于结构体定义
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)	// 适用于 sizeof(struct struct_name)
		syntax_state = SNTX_NUL;
	else							// 适用于结构变量声明
		syntax_state = SNTX_SP;
	SyntaxIndent();

	if (v < TK_IDENT)				// 关键字不能作为结构名称
		Expect("结构体名");
	s = StructSearch(v);
	if (!s)
	{
		type1.t = KW_STRUCT;
		// -1表示结构体未定义
		s = SymPush(v | ViaC_STRUCT, &type1, 0, -1);
		s->r = 0;
	}

	type->t = T_STRUCT;
	type->ref = s;

	if (token == TK_BEGIN)
	{
		StructDeclarationList(type);
	}
}

/***********************************************************
* 功能:		解析结构声明符表
* type(输出):	结构类型
*
* <StructDeclarationList>::=<StructDeclaration>{<StructDeclaration>}
**********************************************************/
void StructDeclarationList(Type *type)
{
	int maxalign, offset;
	Symbol *s, **ps;
	s = type->ref;
	syntax_state = SNTX_LF_HT;   // 第一个结构体成员与'{'不写在一行
	syntax_level++;              // 结构体成员变量声明，缩进增加一级
	GetToken();
	if (s->c != -1)
		Error("结构体已定义");
	maxalign = 1;
	ps = &s->next;
	offset = 0;
	while (token != TK_END)
	{
		StructDeclaration(&maxalign, &offset, &ps);
	}
	Skip(TK_END);
	syntax_state = SNTX_LF_HT;

	s->c = CalcAlign(offset, maxalign);//结构体大小
	s->r = maxalign; //结构体对齐
}

/***********************************************************
* 功能:				解析结构声明
* maxalign(输入,输出):	成员最大对齐粒度
* offset(输入,输出):	偏移量
* ps(输出):			结构定义符号
*
* <StructDeclaration>::=
*		<TypeSpecifier><struct_Declarator_list><TK_SEMICOLON>
*
* <struct_Declarator_list>::=<Declarator>{<TK_COMMA><Declarator>}
**********************************************************/
void StructDeclaration(int *maxalign, int *offset, Symbol ***ps)
{
	int v, size, align;
	Symbol *ss;
	Type type1, btype;
	int force_align;
	TypeSpecifier(&btype);
	while (1)
	{
		v = 0;
		type1 = btype;
		Declarator(&type1, &v, &force_align);
		size = TypeSize(&type1, &align);

		if (force_align & ALIGN_SET)
			align = force_align & ~ALIGN_SET;

		*offset = CalcAlign(*offset, align);

		if (align > *maxalign)
			*maxalign = align;
		ss = SymPush(v | ViaC_MEMBER, &type1, 0, *offset);
		*offset += size;
		**ps = ss;
		*ps = &ss->next;

		if (token == TK_SEMICOLON || token == TK_EOF)
			break;
		Skip(TK_COMMA);
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}


/***********************************************************
* 功能:				解析声明符
* type:				数据类型
* v(输出):				单词编号
* force_align(输出):	强制对齐粒度
*
* <Declarator>::={<pointer>}{<FunctionCallingConvention>}
*	{<StructMemberAlignment>}<DirectDeclarator>
*
* <pointer>::=<TK_STAR>
**********************************************************/
void Declarator(Type *type, int *v, int *force_align)
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

/***********************************************************
* 功能:	解析函数调用约定
* fc(输出):调用约定
*
* <FunctionCallingConvention>::=<KW_CDECL>|<KW_STDCALL>
* 用于函数声明上，用在数据声明上忽略掉
**********************************************************/
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

/***********************************************************
* 功能:				解析结构成员对齐
* force_align(输出):	强制对齐粒度
*
* <StructMemberAlignment>::=<KW_ALIGN><TK_OPENPA><TK_CINT><TK_CLOSEPA>
**********************************************************/
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

/***********************************************************
* 功能:			解析直接声明符
* type(输入,输出):	数据类型
* v(输出):			单词编号
* func_call:		函数调用约定
*
* <DirectDeclarator>::=<IDENTIFIER><DirectDeclaratorPostfix>
**********************************************************/
void DirectDeclarator(Type *type, int *v, int func_call)
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

/***********************************************************
* 功能:			直接声明符后缀
* type(输入,输出):	数据类型
* func_call:		函数调用约定
*
* <DirectDeclarator_ postfix>::= {<TK_OPENBR><TK_CINT><TK_CLOSEBR>
* 		|<TK_OPENBR><TK_CLOSEBR>
*		|<TK_OPENPA><ParameterTypeList><TK_CLOSEPA>
*		|<TK_OPENPA><TK_CLOSEPA>}
**********************************************************/
void DirectDeclaratorPostfix(Type *type, int func_call)
{
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

/***********************************************************
* 功能:			解析形参类型表
* type(输入,输出):	数据类型
* func_call:		函数调用约定
*
* <ParameterTypeList>::=<parameter_list>
*        |<parameter_list><TK_COMMA><TK_ELLIPSIS>
*
*  <parameter_list>::=
*		<parameter_declaration>{<TK_COMMA ><parameter_declaration>}
**********************************************************/
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
	if (token == TK_BEGIN)			// 函数定义
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

/***********************************************************
* 功能:	解析函数体
* sym:		函数符号
*
* <Funcbody>::=<CompoundStatement>
**********************************************************/
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

/***********************************************************
* 功能:	判断是否为类型区分符
* v:		单词编号
**********************************************************/
int IsTypeSpecifier(int v)
{
	switch (v)
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

/***********************************************************
* 功能:	解析语句
* bsym:	break跳转位置
* csym:	continue跳转位置
*
* <Statement >::=<CompoundStatement>
*		|<IfStatement>
*		| <ReturnStatement>
*		| <BreakStatement>
*		| <ContinueStatement>
*		| <ForStatement>
*		| <ExpressionStatement>
**********************************************************/
void Statement(int *bsym, int *csym)
{
	switch (token)
	{
		case TK_BEGIN:
			CompoundStatement(bsym, csym);
			break;
		case KW_IF:
			IfStatement(bsym, csym);
			break;
		case KW_RETURN:
			ReturnStatement();
			break;
		case KW_BREAK:
			BreakStatement(bsym);
			break;
		case KW_CONTINUE:
			ContinueStatement(csym);
			break;
		case KW_FOR:
			ForStatement(bsym, csym);
			break;
		default:
			ExpressionStatement();
			break;
	}
}

/***********************************************************
* 功能:	解析复合语句
* bsym:	break跳转位置
* csym:	continue跳转位置
*
* <CompoundStatement>::=<TK_BEGIN>{<declaration>}{<Statement>}<TK_END>
**********************************************************/
void CompoundStatement(int *bsym, int *csym)
{
	Symbol *s;
	s = (Symbol*)StackGetTop(&LSYM);
	syntax_state = SNTX_LF_HT;
	syntax_level++;						// 复合语句，缩进增加一级

	GetToken();
	while (IsTypeSpecifier(token))
	{
		ExternalDeclaration(ViaC_LOCAL);
	}
	while (token != TK_END)
	{
		Statement(bsym, csym);
	}
	SymPop(&LSYM, s);
	syntax_state = SNTX_LF_HT;
	GetToken();
}

/***********************************************************
* 功能:	解析if语句
* bsym:	break跳转位置
* csym:	continue跳转位置
*
* <IfStatement>::=<KW_IF><TK_OPENPA><Expression>
*	<TK_CLOSEPA><Statement>[<KW_ELSE><Statement>]
**********************************************************/
void IfStatement(int *bsym, int *csym)
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
		BackPatch(b, ind); /* 反填else跳转 */
	}
	else
		BackPatch(a, ind);
}

/***********************************************************
* 功能:	解析for语句
* bsym:	break跳转位置
* csym:	continue跳转位置
*
* <ForStatement>::=<KW_FOR><TK_OPENPA><ExpressionStatement>
*	<ExpressionStatement><Expression><TK_CLOSEPA><Statement>
**********************************************************/
void ForStatement(int *bsym, int *csym)
{
	int a, b, c, d, e;
	GetToken();
	Skip(TK_OPENPA);
	if (token != TK_SEMICOLON)
	{
		Expression();
		OperandPop();
	}
	Skip(TK_SEMICOLON);
	d = ind;
	c = ind;
	a = 0;
	b = 0;
	if (token != TK_SEMICOLON)
	{
		Expression();
		a = GenJcc(0);
	}
	Skip(TK_SEMICOLON);
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
	Statement(&a, &b);//只有此处用到break,及continue,一个循环中可能有多个break,或多个continue,故需要拉链以备反填
	GenJmpBackWord(c);
	BackPatch(a, ind);
	BackPatch(b, c);
}

/***********************************************************
* 功能:	解析continue语句
* csym:	continue跳转位置
*
* <ContinueStatement>::=<KW_CONTINUE><TK_SEMICOLON>
**********************************************************/
void ContinueStatement(int *csym)
{
	if (!csym)
		Error("此处不能用continue");
	*csym = GenJmpForWard(*csym);
	GetToken();
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

/***********************************************************
* 功能:	解析break语句
* bsym:	break跳转位置
*
* <BreakStatement>::=<KW_BREAK><TK_SEMICOLON>
**********************************************************/
void BreakStatement(int *bsym)
{
	if (!bsym)
		Error("此处不能用break");
	*bsym = GenJmpForWard(*bsym);
	GetToken();
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}
/***********************************************************
* 功能:	解析return语句
*
* <ReturnStatement>::=<KW_RETURN><TK_SEMICOLON>
*			|<KW_RETURN><Expression><TK_SEMICOLON>
**********************************************************/
void ReturnStatement()
{
	syntax_state = SNTX_DELAY;
	GetToken();
	if (token == TK_SEMICOLON)	// 适用于 return;
		syntax_state = SNTX_NUL;
	else						// 适用于 return <Expression>;
		syntax_state = SNTX_SP;
	SyntaxIndent();

	if (token != TK_SEMICOLON)
	{
		Expression();
		Load_1(REG_IRET, optop);
		OperandPop();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
	rsym = GenJmpForWard(rsym);
}

/***********************************************************
* 功能:	解析表达式语句
*
* <ExpressionStatement>::= <TK_SEMICOLON>|<Expression><TK_SEMICOLON>
**********************************************************/
void ExpressionStatement()
{
	if (token != TK_SEMICOLON)
	{
		Expression();
		OperandPop();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

/***********************************************************
* 功能:	解析表达式
*
* <Expression>::=<AssignmentExpression>{<TK_COMMA><AssignmentExpression>}
**********************************************************/
void Expression()
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


/***********************************************************
* 功能:	解析赋值表达式
*RelationalExpression
* <AssignmentExpression>::= <EqualityExpression>
*		|<UnaryExpression><TK_ASSIGN> <EqualityExpression>
**********************************************************/
//这里有左递归，可以提取公因子
void AssignmentExpression()
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

/***********************************************************
* 功能:	解析相等类表达式
*
* < EqualityExpression >::=<RelationalExpression>
*		{<TK_EQ> <RelationalExpression>
*		|<TK_NEQ><RelationalExpression>}
**********************************************************/
void EqualityExpression()
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

/***********************************************************
* 功能:	解析关系表达式
*
* <RelationalExpression>::=<AdditiveExpression>{
*		<TK_LT><AdditiveExpression>
*		|<TK_GT><AdditiveExpression>
*		|<TK_LEQ><AdditiveExpression>
*		|<TK_GEQ><AdditiveExpression>}
**********************************************************/
void RelationalExpression()
{
	int t;
	AdditiveExpression();
	while ((token == TK_LT || token == TK_LEQ) ||
		token == TK_GT || token == TK_GEQ)
	{
		t = token;
		GetToken();
		AdditiveExpression();
		GenOp(t);
	}
}

/***********************************************************
* 功能:	解析加减类表达式
*
* <AdditiveExpression>::=< MultiplicativeExpression>
*		{<TK_PLUS> <MultiplicativeExpression>
*		<TK_MINUS>< MultiplicativeExpression>}
**********************************************************/
void AdditiveExpression()
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

/***********************************************************
* 功能:	解析乘除类表达式
*
* <MultiplicativeExpression>::=<UnaryExpression>
*		{<TK_STAR>  < UnaryExpression >
*		|<TK_DIVIDE>< UnaryExpression >
*		|<TK_MOD>  < UnaryExpression >}
**********************************************************/
void MultiplicativeExpression()
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

/***********************************************************
* 功能:	解析一元表达式
*
* <UnaryExpression>::= <PostfixExpression>
*			|<TK_AND><UnaryExpression>
*			|<TK_STAR><UnaryExpression>
*			|<TK_PLUS><UnaryExpression>
*			|<TK_MINUS><UnaryExpression>
*			|<KW_SIZEOF><TK_OPENPA><TypeSpecifier><TK_ CLOSEPA>
**********************************************************/
void UnaryExpression()
{
	switch (token)
	{
		case TK_AND:
			GetToken();
			UnaryExpression();
			if ((optop->type.t & T_BTYPE) != T_FUNC &&
				!(optop->type.t & T_ARRAY))
				CancelLvalue();
			MkPointer(&optop->type);
			break;
		case TK_STAR:
			GetToken();
			UnaryExpression();
			Indirection();
			break;
		case TK_PLUS:
			GetToken();
			UnaryExpression();
			break;
		case TK_MINUS:
			GetToken();
			OperandPush(&int_type, ViaC_GLOBAL, 0);
			UnaryExpression();
			GenOp(TK_MINUS);
			break;
		case KW_SIZEOF:
			SizeofExpression();
			break;
		default:
			PostfixExpression();
			break;
	}

}

/***********************************************************
* 功能:	解析sizeof表达式
*
* <SizeofExpression>::=
*		<KW_SIZEOF><TK_OPENPA><TypeSpecifier><TK_ CLOSEPA>
**********************************************************/
void SizeofExpression()
{
	int align, size;
	Type type;

	GetToken();
	Skip(TK_OPENPA);
	TypeSpecifier(&type);
	Skip(TK_CLOSEPA);

	size = TypeSize(&type, &align);
	if (size < 0)
		Error("sizeof计算类型尺寸失败");
	OperandPush(&int_type, ViaC_GLOBAL, size);
}

/***********************************************************
* 功能:	解析后缀表达式
*
* <PostfixExpression>::=  <PrimaryExpression>
*		{<TK_OPENBR><Expression> <TK_CLOSEBR>
*		|<TK_OPENPA><TK_CLOSEPA>
*		|<TK_OPENPA><ArgumentExpressionList><TK_CLOSEPA>
*		|<TK_DOT><IDENTIFIER>
*		|<TK_POINTSTO><IDENTIFIER>}
**********************************************************/
void PostfixExpression()
{
	Symbol *s;
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
			s = optop->type.ref;
			token |= ViaC_MEMBER;
			while ((s = s->next) != NULL)
			{
				if (s->v == token)
					break;
			}
			if (!s)
				Error("没有此成员变量: %s", GetTkstr(token & ~ViaC_MEMBER));
			/* 成员变量地址 = 结构变量指针 + 成员变量偏移 */
			optop->type = char_pointer_type; /* 成员变量的偏移是指相对于结构体首地址的字节偏移，因此此处变换类型为字节变量指针 */
			OperandPush(&int_type, ViaC_GLOBAL, s->c);
			GenOp(TK_PLUS);  //执行后optop->value记忆了成员地址
							  /* 变换类型为成员变量数据类型 */
			optop->type = s->type;
			/* 数组变量不能充当左值 */
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

/***********************************************************
* 功能:	解析初等表达式
*
* <PrimaryExpression>::=<IDENTIFIER>
*		|<TK_CINT>
*		|<TK_CSTR>
*		|<TK_CCHAR>
*		|<TK_OPENPA><Expression><TK_CLOSEPA>
**********************************************************/
void PrimaryExpression()
{
	int t, r, addr;
	Type type;
	Symbol *s;
	Section *sec = NULL;
	switch (token)
	{
		case TK_CINT:
		case TK_CCHAR:
			OperandPush(&int_type, ViaC_GLOBAL, tkvalue);
			GetToken();
			break;
		case TK_CSTR:
			t = T_CHAR;
			type.t = t;
			MkPointer(&type);
			type.t |= T_ARRAY;
			sec = AllocateStorage(&type, ViaC_GLOBAL, 2, 0, &addr);
			VarSymPut(&type, ViaC_GLOBAL, 0, addr);
			Initializer(&type, addr, sec);
			break;
		case TK_OPENPA:
			GetToken();
			Expression();
			Skip(TK_CLOSEPA);
			break;
		default:
			t = token;
			GetToken();
			if (t < TK_IDENT)
				Expect("标识符或常量");
			s = SymSearch(t);
			if (!s)
			{
				if (token != TK_OPENPA)
					Error("'%s'未声明\n", GetTkstr(t));

				s = FuncSymPush(t, &default_func_type);//允许函数不声明，直接引用
				s->r = ViaC_GLOBAL | ViaC_SYM;
			}
			r = s->r;
			OperandPush(&s->type, r, s->c);
			/* 符号引用，操作数必须记录符号地址 */
			if (optop->reg & ViaC_SYM)
			{
				optop->sym = s;
				optop->value = 0;  //用于函数调用，及全局变量引用 printf("g_cc=%c\n",g_cc);
			}
			break;
	}
}

/***********************************************************
* 功能:	解析实参表达式表
*
* <ArgumentExpressionList >::=<AssignmentExpression>
*		{<TK_COMMA> <AssignmentExpression>}
**********************************************************/
void ArgumentExpressionList()
{
	Operand ret;
	Symbol *s, *sa;
	int nb_args;
	s = optop->type.ref;
	GetToken();
	sa = s->next;
	nb_args = 0;
	ret.type = s->type;
	ret.reg = REG_IRET;
	ret.value = 0;
	if (token != TK_CLOSEPA)
	{
		for (;;)
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
		Error("实参个数少于函数形参个数"); //讲一下形参，实参
	Skip(TK_CLOSEPA);
	GenInvoke(nb_args);

	OperandPush(&ret.type, ret.reg, ret.value);
}

/***********************************************************
* 功能:	缩进n个tab键
* n:		缩进个数
**********************************************************/
void PrintTab(int n)
{
	int i = 0;
	for (; i < n; i++)
		printf("\t");
}

/***********************************************************
* 功能:	语法缩进
* 通常情况在GetToken最后调用，
* 如果必须根据新取单词类型判断输出格式，则在取该单词前设置syntax_state = SNTX_DELAY，暂不输出，
* 待取出新单词后，根据单词类型设置syntax_state，重新调用该函数根据syntax_state，进行适当输出
**********************************************************/
void SyntaxIndent()
{
	switch (syntax_state)
	{
		case SNTX_NUL:
			ColorToken(LEX_NORMAL);
			break;
		case SNTX_SP:
			printf(" ");
			ColorToken(LEX_NORMAL);
			break;
		case SNTX_LF_HT:
		{
			if (token == TK_END)		// 遇到'}',缩进减少一级
				syntax_level--;
			printf("\n");
			PrintTab(syntax_level);
		}
		ColorToken(LEX_NORMAL);
		break;
		case SNTX_DELAY:
			break;
	}
	syntax_state = SNTX_NUL;
}