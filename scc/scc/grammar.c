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


void ParameterTypeList(int func_call) //解析形参类型表
{
	GetToken(); 
	while (token == TK_CLOSEPA)
	{
		if (!TypeSpecifier())
		{
			Error("标识符无效");
		}
		Declarator();
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
}


void DirectDeclaratorPostfix() // 直接声明符后缀
{
	int m;
	if (token == TK_OPENPA) //括号开始
	{
		//ParameterTypeList ();
	}
	else if (token == TK_OPENBR)  // 中括号
	{
		GetToken();
		if (token == TK_OPENBR)
		{
			GetToken(); 
			m = tkvalue;
		}
		Skip(TK_CLOSEBR);
		DirectDeclaratorPostfix();//递归调用
	}
}


void DirectDeclarator()
{
	if (token >= TK_IDENT)
	{
		GetToken();
	}
	else
	{
		Expect("表示符");
	}
	DirectDeclaratorPostfix();
}


void Declarator()
{
	int fc; 
	while (token == TK_STAR)
	{ 
		GetToken();
	}
	FunctionCallingConvention(&fc);
	StructMemberAligment();
	DirectDeclarator();
}

void TranslationUnit()
{
	while (token != TK_EOF)
	{
		ExternalDeclaration(SC_GLOBAL);
	}
}

void Initializer() // 初值符
{
	AssignmentExpression();
}

void ExternalDeclaration(int level)
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

	while (1)
	{
		Declarator();
		if (token == TK_BEGIN)
		{
			if (level == SC_LOCAL)
				Error("不允许嵌套定义");
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

int TypeSpecifier()
{
	int type_found = 0;
	switch (token)
	{
		case KW_CHAR:
		{
			type_found = 1;
			syntax_state = SNTX_SP;
			break;
		}
		case KW_INT:
		{
			type_found = 1; 
			syntax_state = SNTX_SP;
			break;
		}
		case KW_VOID:
		{
			type_found = 1; 
			syntax_state = SNTX_SP; 
			break;
		}
		case KW_STRUCT:
		{
			type_found = 1; 
			syntax_state = SNTX_SP;
			break;
		}
		case KW_SHORT:
		{
			type_found = 1; 
			syntax_state = SNTX_SP;
		}
		default:
			break;
	}
	return type_found;
}
void StructDeclaration()
{ 
	TypeSpecifier(); 
	while (1)
	{
		Declarator();
		
		if (token == TK_SEMICOLON)
			break;
		Skip(TK_COMMA);
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

void StructDeclarationList()
{
	int  maxalign, offset;
	
	syntax_state = SNTX_LF_HT;
	++syntax_level;
	
	GetToken();

	while (token == TK_END)
	{
		StructDeclaration(&maxalign, &offset);
	}
	Skip(TK_END); 
	syntax_state = SNTX_LF_HT;	
}

void StructSpecifier()
{
	int t;
	GetToken();
	t = token;
	
	syntax_state = SNTX_DELAY;
	GetToken();
	if (token == TK_BEGIN)
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP; 
	SyntaxIndent();

	if (t < TK_IDENT)
		Expect("结构体名称");
	if (token == TK_BEGIN)
	{
		StructDeclarationList();
	}
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

void StructMemberAligment()
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

void CompoundStatement()
{
	syntax_state = SNTX_LF_HT;
	syntax_level++;
	
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


void Funcbody()
{
	CompoundStatement();
	
}


void Statement()
{
	switch (token)
	{
		case TK_BEGIN:
		{
			CompoundStatement();
			break;
		}
		case KW_IF:
		{
			IfStatement();
			break;
		}
		case KW_RETURN:
		{
			ReturnStatement();
			break;
		}
		case KW_BREAK:
		{
			BreakStatement();
			break;
		}
		case KW_CONTINUE:
		{
			ContinueStatement();
			break;
		}
		case KW_FOR:
		{
			ForStatement();
			break;
		}
		default:
		{
			AssignmentExpression();
			break;
		}
	}
}


int IsTypeSpecifier(int id)
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

void ExpressionStatement()
{
	if (token == TK_SEMICOLON)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

void IfStatement()
{
	syntax_state = SNTX_SP;
	GetToken();
	Skip(TK_OPENPA);
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


void ForStatement()
{
	GetToken();
	Skip(TK_OPENPA);
	if (token != TK_SEMICOLON)
	{
		Expression();
	} 
	Skip(TK_SEMICOLON);
	if (token == TK_SEMICOLON)
	{
		Expression();
	}
	Skip(TK_SEMICOLON);
	if (token == TK_CLOSEPA)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_CLOSEPA);
	Statement();
}

void ContinueStatement()
{
	GetToken();
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

void BreakStatement()
{
	GetToken();
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}


void ReturnStatement()
{
	syntax_state = SNTX_DELAY;
	GetToken();
	if (token == TK_SEMICOLON)
	{
		syntax_state = SNTX_NUL;
	}
	else
	{
		syntax_state = SNTX_SP;
	}
	
	SyntaxIndent();
	
	if (token != TK_SEMICOLON)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
}

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

void AssignmentExpression()
{
	EquaityExpression();
	if (token == TK_ASSIGN)
	{
		GetToken();
		AssignmentExpression();
	}
}
void EquaityExpression()
{
	RealtionalExpression();
	while (token == TK_EQ || token == TK_NEQ)
	{
		GetToken(); 
		RealtionalExpression();
	}
}
void RealtionalExpression()
{ 
	AdditiveExpression();
	while(token == TK_LT || token == TK_LEQ ||
		  token == TK_GT || token == TK_GEQ)
	{
		GetToken(); 
		AdditiveExpression();
	}
}

void AdditiveExpression()
{
	MultiplicativeExpression();
	while (token == TK_PLUS || token == TK_MINUS)
	{
		GetToken(); 
		MultiplicativeExpression();
	}
}

void MultiplicativeExpression()
{
	UnaryExpression(); 
	while (token == TK_STAR || token == TK_DIVIDE || token == TK_MOD)
	{
		GetToken();
		UnaryExpression();
	}
}
void UnaryExpression()
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


void SizeofExpression()
{
	GetToken();
	Skip(TK_OPENPA);
	TypeSpecifier();
	Skip(TK_CLOSEPA);
}
 
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
	int id;
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
			GetToken();
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
			{
				Expect("常量或者是标识符");
			}
			break;
		}
	}
}

void ArgumentExpressionList()
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

void SyntaxIndent()
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
			{
				if (token == TK_END)
					syntax_level--;
				printf("\n");
				PrintTab(syntax_level);
			}
			ColorToken(LEX_NORMAL);
			break;
		}
		case SNTX_DELAY:
			break;
	}
	syntax_state = SNTX_NUL;
}

void PrintTab(int num)
{
	int  count = 0;
	for (; count < num; ++count)
	{
		printf("\t");
	}
}