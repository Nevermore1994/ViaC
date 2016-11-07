 《自己动手写编译器、链接器》配套源代码

#include "scc.h"
int syntax_state;  //语法状态
int syntax_level;  //缩进级别

				   /***********************************************************
				   * 功能:	解析翻译单位
				   *
				   *  <TranslationUnit>::={ExternalDeclaration}<TK_EOF>
				   **********************************************************/
void TranslationUnit()
{
	while (token != TK_EOF)
	{
		ExternalDeclaration(SC_GLOBAL);
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
*		|<TypeSpecifier>< init_declarator_list><TK_SEMICOLON>
*
* <init_declarator_list>::=
*      <init_declarator>{<TK_COMMA> <init_declarator>}
*
* <init_declarator>::=
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
	if (!TypeSpecifier())
	{
		Expect("<类型区分符>");
	}

	if (token == TK_SEMICOLON)
	{
		GetToken();
		return;
	}
	while (1)// 逐个分析声明或函数定义
	{
		Declarator();
		if (token == TK_BEGIN)
		{
			if (l == SC_LOCAL)
				Error("不支持函数嵌套定义");
			Funcbody();
			break;
		}
		else
		{

			if (token == TK_ASSIGN)
			{
				GetToken();
				Initializer();
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
*
* < Initializer>::=<AssignmentExpression>
**********************************************************/
void Initializer()
{
		GetToken();
	AssignmentExpression();
}

/***********************************************************
* 功能:		类型区分符
* type(输出):	数据类型
* 返回值:		是否发现合法的类型区分符
*
*	<TypeSpecifier>::= <KW_INT>
*		| <KW_CHAR>
*		| <KW_SHORT>
*		| <KW_VOID >
*		| <StructSpecifier>
**********************************************************/
int TypeSpecifier()
{
	int type_found = 0;
	switch (token)
	{
		case KW_CHAR:
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		case KW_SHORT:
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		case KW_VOID:
			type_found = 1;
			syntax_state = SNTX_SP;
			GetToken();
			break;
		case KW_INT:
			syntax_state = SNTX_SP;
			type_found = 1;
			GetToken();
			break;
		case KW_STRUCT:
			syntax_state = SNTX_SP;
			StructSpecifier();
			type_found = 1;
			break;
		default:
			break;
	}
	return type_found;
}

/***********************************************************
* 功能:		解析结构区分符
*
* <StructSpecifier>::=
*	<KW_STRUCT><IDENTIFIER><TK_BEGIN><StructDeclarationList><TK_END>
*		| <KW_STRUCT>  <IDENTIFIER>
**********************************************************/
void StructSpecifier()
{
	int v;

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

	if (token == TK_BEGIN)
	{
		StructDeclarationList();
	}
}

/***********************************************************
* 功能:		解析结构声明符表
*
* <StructDeclarationList>::=<StructDeclaration>{<StructDeclaration>}
**********************************************************/
void StructDeclarationList()
{
	int maxalign, offset;

	syntax_state = SNTX_LF_HT;	// 第一个结构体成员与'{'不写在一行
	syntax_level++;				// 结构体成员变量声明，缩进增加一级

	GetToken();
	while (token != TK_END)
	{
		StructDeclaration(&maxalign, &offset);
	}
	Skip(TK_END);

	syntax_state = SNTX_LF_HT;
}

/***********************************************************
* 功能:				解析结构声明
*
* <StructDeclaration>::=
*		<TypeSpecifier><struct_declarator_list><TK_SEMICOLON>
*
* <struct_declarator_list>::=<Declarator>{<TK_COMMA><Declarator>}
**********************************************************/
void StructDeclaration()
{
	TypeSpecifier();
	while (1)
	{
		Declarator();

		if (token == TK_SEMICOLON || token == TK_EOF)
			break;
		Skip(TK_COMMA);
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}


/***********************************************************
* 功能:				解析声明符
*
* <Declarator>::={<pointer>}{<FunctionCallingConvention>}
*	{<StructMemberAlignment>}<direct_declfunction_calling_conventionarator>
*
* <pointer>::=<TK_STAR>
**********************************************************/
void Declarator()
{
	while (token == TK_STAR)
	{
		GetToken();
	}
	FunctionCallingConvention();
	StructMemberAlignment();
	DirectDeclarator();
}

/***********************************************************
* 功能:	解析函数调用约定
*
* <FunctionCallingConvention>::=<KW_CDECL>|<KW_STDCALL>
* 用于函数声明上，用在数据声明上忽略掉
**********************************************************/
void FunctionCallingConvention()
{
	if (token == KW_CDECL || token == KW_STDCALL)
	{
		syntax_state = SNTX_SP;
		GetToken();
	}
}

/***********************************************************
* 功能:				解析结构成员对齐
*
* <StructMemberAlignment>::=<KW_ALIGN><TK_OPENPA><TK_CINT><TK_CLOSEPA>
**********************************************************/
void StructMemberAlignment()
{
	if (token == KW_ALIGN)
	{
		GetToken();
		Skip(TK_OPENPA);
		if (token == TK_CINT)
		{
			GetToken();
		}
		else
			Expect("整数常量");
		Skip(TK_CLOSEPA);
	}
}

/***********************************************************
* 功能:			解析直接声明符
*
* <DirectDeclarator>::=  <IDENTIFIER><DirectDeclaratorPostfix>
**********************************************************/
void DirectDeclarator()
{
	if (token >= TK_IDENT)
	{
		GetToken();
	}
	else
	{
		Expect("标识符");
	}
	DirectDeclaratorPostfix();
}

/***********************************************************
* 功能:			直接声明符后缀
*
*<direct_declarator_ postfix>::= {<TK_OPENBR><TK_CINT><TK_CLOSEBR>
* 		|<TK_OPENBR><TK_CLOSEBR>
*		|<TK_OPENPA><ParameterTypeList><TK_CLOSEPA>
*		|<TK_OPENPA><TK_CLOSEPA>}
**********************************************************/
void DirectDeclaratorPostfix()
{
	int n;

	if (token == TK_OPENPA)
	{
		ParameterTypeList();
	}
	else if (token == TK_OPENBR)
	{
		GetToken();
		if (token == TK_CINT)
		{
			GetToken();
			n = tkvalue;
		}
		Skip(TK_CLOSEBR);
		DirectDeclaratorPostfix();
	}
}

/***********************************************************
* 功能:			解析形参类型表
* func_call:		函数调用约定
*
* <ParameterTypeList>::=<parameter_list>
*        |<parameter_list><TK_COMMA><TK_ELLIPSIS>
*
*  <parameter_list>::=<parameter_declaration>
*		  {<TK_COMMA ><parameter_declaration>}
**********************************************************/
void ParameterTypeList()
{

	GetToken();
	while (token != TK_CLOSEPA)
	{
		if (!TypeSpecifier())
		{
			Error("无效类型标识符");
		}
		Declarator();
		if (token == TK_CLOSEPA)
			break;
		Skip(TK_COMMA);
		if (token == TK_ELLIPSIS)
		{
			
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
}

/***********************************************************
* 功能:	解析函数体
*
* <Funcbody>::=<CompoundStatement>
**********************************************************/
void Funcbody()
{
	/* 放一匿名符号在局部符号表中 */
	CompoundStatement();
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
*		| <IfStatement>
*		| <ReturnStatement>
*		| <BreakStatement>
*		| <ContinueStatement>
*		| <ForStatement>
*		| <ExpressionStatement>
**********************************************************/
void Statement()
{
	switch (token)
	{
		case TK_BEGIN:
			CompoundStatement();
			break;
		case KW_IF:
			IfStatement();
			break;
		case KW_RETURN:
			ReturnStatement();
			break;
		case KW_BREAK:
			BreakStatement();
			break;
		case KW_CONTINUE:
			ContinueStatement();
			break;
		case KW_FOR:
			ForStatement();
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
void CompoundStatement()
{
	syntax_state = SNTX_LF_HT;
	syntax_level++;						// 复合语句，缩进增加一级

	GetToken();
	while (IsTypeSpecifier(token))
	{
		ExternalDeclaration(SC_LOCAL);
	}
	while (token != TK_END)
	{
		Statement();
	}

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
void IfStatement()
{
	syntax_state = SNTX_SP;
	GetToken();
	Skip(TK_OPENPA);
	Expression();
	syntax_state = SNTX_LF_HT;
	Skip(TK_CLOSEPA);
	Statement();
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT;
		GetToken();
		Statement();
	}
}

/***********************************************************
* 功能:	解析for语句
* bsym:	break跳转位置
* csym:	continue跳转位置
*
* <ForStatement>::=<KW_FOR><TK_OPENPA><ExpressionStatement>
*	<ExpressionStatement><Expression><TK_CLOSEPA><Statement>
**********************************************************/
void ForStatement()
{
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
	Statement();//只有此处用到break,及continue,一个循环中可能有多个break,或多个continue,故需要拉链以备反填
}

/***********************************************************
* 功能:	解析continue语句
* csym:	continue跳转位置
*
* <ContinueStatement>::=<KW_CONTINUE><TK_SEMICOLON>
**********************************************************/
void ContinueStatement()
{
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
void BreakStatement()
{
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
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

/***********************************************************
* 功能:	解析表达式语句
*
* <ExpressionStatement>::= <TK_SEMICOLON>|<Expression> <TK_SEMICOLON>
**********************************************************/
void ExpressionStatement()
{
	if (token != TK_SEMICOLON)
	{
		Expression();
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
		GetToken();
	}
}


/***********************************************************
* 功能:	解析赋值表达式
*
* <AssignmentExpression>::= <EqualityExpression>
*		|<UnaryExpression><TK_ASSIGN> <EqualityExpression>
**********************************************************/
void AssignmentExpression()
{
	EqualityExpression();
	if (token == TK_ASSIGN)
	{
		GetToken();
		AssignmentExpression();
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
	AdditiveExpression();
	while ((token == TK_LT || token == TK_LEQ) ||
		token == TK_GT || token == TK_GEQ)
	{
		GetToken();
		AdditiveExpression();
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
	MultiplicativeExpression();
	while (token == TK_PLUS || token == TK_MINUS)
	{
		GetToken();
		MultiplicativeExpression();
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
			break;
		case TK_STAR:
			GetToken();
			UnaryExpression();
			break;
		case TK_PLUS:
			GetToken();
			UnaryExpression();
			break;
		case TK_MINUS:
			GetToken();
			UnaryExpression();
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
	GetToken();
	Skip(TK_OPENPA);
	TypeSpecifier();
	Skip(TK_CLOSEPA);
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


void PrimaryExpression()
{
	int t;
	switch (token)
	{
		case TK_CINT:
		case TK_CCHAR:
			GetToken();
			break;
		case TK_CSTR:
			GetToken();
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
			break;
	}
}

void ArgumentExpressionList()
{
	GetToken();
	if (token != TK_CLOSEPA)
	{
		for (;;)
		{
			AssignmentExpression();
			if (token == TK_CLOSEPA)
				break;
			Skip(TK_COMMA);
		}
	}
	Skip(TK_CLOSEPA);
	 return value
}


void PrintTab(int n)
{
	int i = 0;
	for (; i < n; i++)
		printf("  ");
}


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


