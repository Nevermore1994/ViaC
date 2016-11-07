 ���Լ�����д��������������������Դ����

#include "scc.h"
int syntax_state;  //�﷨״̬
int syntax_level;  //��������

				   /***********************************************************
				   * ����:	�������뵥λ
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
* ����:	�����ⲿ����
* l:		�洢���ͣ��ֲ��Ļ���ȫ�ֵ�
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
* ��д���ķ���
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
		Expect("<�������ַ�>");
	}

	if (token == TK_SEMICOLON)
	{
		GetToken();
		return;
	}
	while (1)// �������������������
	{
		Declarator();
		if (token == TK_BEGIN)
		{
			if (l == SC_LOCAL)
				Error("��֧�ֺ���Ƕ�׶���");
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
* ����:	������ֵ��
*
* < Initializer>::=<AssignmentExpression>
**********************************************************/
void Initializer()
{
		GetToken();
	AssignmentExpression();
}

/***********************************************************
* ����:		�������ַ�
* type(���):	��������
* ����ֵ:		�Ƿ��ֺϷ����������ַ�
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
* ����:		�����ṹ���ַ�
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

	syntax_state = SNTX_DELAY;      // ��ȡ���ʲ���ʱ������ӳٵ�ȡ�����ʺ���ݵ��������ж������ʽ
	GetToken();

	if (token == TK_BEGIN)			// �����ڽṹ�嶨��
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)	// ������ sizeof(struct struct_name)
		syntax_state = SNTX_NUL;
	else							// �����ڽṹ��������
		syntax_state = SNTX_SP;
	SyntaxIndent();

	if (v < TK_IDENT)				// �ؼ��ֲ�����Ϊ�ṹ����
		Expect("�ṹ����");

	if (token == TK_BEGIN)
	{
		StructDeclarationList();
	}
}

/***********************************************************
* ����:		�����ṹ��������
*
* <StructDeclarationList>::=<StructDeclaration>{<StructDeclaration>}
**********************************************************/
void StructDeclarationList()
{
	int maxalign, offset;

	syntax_state = SNTX_LF_HT;	// ��һ���ṹ���Ա��'{'��д��һ��
	syntax_level++;				// �ṹ���Ա������������������һ��

	GetToken();
	while (token != TK_END)
	{
		StructDeclaration(&maxalign, &offset);
	}
	Skip(TK_END);

	syntax_state = SNTX_LF_HT;
}

/***********************************************************
* ����:				�����ṹ����
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
* ����:				����������
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
* ����:	������������Լ��
*
* <FunctionCallingConvention>::=<KW_CDECL>|<KW_STDCALL>
* ���ں��������ϣ��������������Ϻ��Ե�
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
* ����:				�����ṹ��Ա����
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
			Expect("��������");
		Skip(TK_CLOSEPA);
	}
}

/***********************************************************
* ����:			����ֱ��������
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
		Expect("��ʶ��");
	}
	DirectDeclaratorPostfix();
}

/***********************************************************
* ����:			ֱ����������׺
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
* ����:			�����β����ͱ�
* func_call:		��������Լ��
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
			Error("��Ч���ͱ�ʶ��");
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
	if (token == TK_BEGIN)			// ��������
		syntax_state = SNTX_LF_HT;
	else							// ��������
		syntax_state = SNTX_NUL;
	SyntaxIndent();
}

/***********************************************************
* ����:	����������
*
* <Funcbody>::=<CompoundStatement>
**********************************************************/
void Funcbody()
{
	/* ��һ���������ھֲ����ű��� */
	CompoundStatement();
}

/***********************************************************
* ����:	�ж��Ƿ�Ϊ�������ַ�
* v:		���ʱ��
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
* ����:	�������
* bsym:	break��תλ��
* csym:	continue��תλ��
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
* ����:	�����������
* bsym:	break��תλ��
* csym:	continue��תλ��
*
* <CompoundStatement>::=<TK_BEGIN>{<declaration>}{<Statement>}<TK_END>
**********************************************************/
void CompoundStatement()
{
	syntax_state = SNTX_LF_HT;
	syntax_level++;						// ������䣬��������һ��

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
* ����:	����if���
* bsym:	break��תλ��
* csym:	continue��תλ��
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
* ����:	����for���
* bsym:	break��תλ��
* csym:	continue��תλ��
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
	Statement();//ֻ�д˴��õ�break,��continue,һ��ѭ���п����ж��break,����continue,����Ҫ�����Ա�����
}

/***********************************************************
* ����:	����continue���
* csym:	continue��תλ��
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
* ����:	����break���
* bsym:	break��תλ��
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
* ����:	����return���
*
* <ReturnStatement>::=<KW_RETURN><TK_SEMICOLON>
*			|<KW_RETURN><Expression><TK_SEMICOLON>
**********************************************************/
void ReturnStatement()
{
	syntax_state = SNTX_DELAY;
	GetToken();
	if (token == TK_SEMICOLON)	// ������ return;
		syntax_state = SNTX_NUL;
	else						// ������ return <Expression>;
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
* ����:	�������ʽ���
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
* ����:	�������ʽ
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
* ����:	������ֵ���ʽ
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
* ����:	�����������ʽ
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
* ����:	������ϵ���ʽ
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
* ����:	�����Ӽ�����ʽ
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
* ����:	�����˳�����ʽ
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
* ����:	����һԪ���ʽ
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
* ����:	����sizeof���ʽ
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
* ����:	������׺���ʽ
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
				Expect("��ʶ������");
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
			if (token == TK_END)		// ����'}',��������һ��
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


