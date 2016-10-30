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

void parameter_type_list(int func_call) //解析形参类型表
{
	get_token(); 
	while (token == TK_CLOSEPA)
	{
		if (!type_specifier())
		{
			error("标识符无效");
		}
		declarator();
		if (token == TK_ELLIPSIS)
		{
			func_call = KW_CDECL;
			get_token();
			break;
		}
	}
	syntax_state = SNTX_DELAY;
	skip(TK_CLOSEPA);
	if (token == TK_BEGIN)
	{
		syntax_state = SNTX_LF_HT;
	} 
	else
	{
		syntax_state = SNTX_NUL;
	}
	syntax_indent();
}
void direct_declarator_postfix() // 直接声明符后缀
{
	int m;
	if (token == TK_OPENPA) //括号开始
	{
		parameter_type_list ();
	}
	else if (token == TK_OPENBR)  // 中括号
	{
		get_token();
		if (token == TK_OPENBR)
		{
			get_token(); 
			m = tkvalue;
		}
		skip(TK_CLOSEBR);
		direct_declarator_postfix();//递归调用
	}
}
void direct_declarator()
{
	if (token >= TK_IDENT)
	{
		get_token();
	}
	else
	{
		expect("表示符");
	}
	direct_declarator_postfix();
}
void declarator()
{
	int fc; 
	while (token == TK_STAR)
	{ 
		get_token();
	}
	function_calling_convention(&fc);
	struct_member_aligment();
	direct_declarator();
}
void translation_unit()
{
	
}
void Initializer() // 初值符
{
	assignment_expression();
}

void external_declaration(int level)
{
	if (!type_specifier())
	{
		expect("<类型区分符>");
	}

	if (token == TK_SEMICOLON)
	{
		get_token();
		return;
	}

	while (1)
	{
		declarator();
		if (token == TK_BEGIN)
		{
			if (level == SC_LOCAL)
				error("不允许嵌套定义");
			Funcbody();
			break;
		}
		else
		{
			if (token == TK_ASSIGN)
			{
				get_token();
				Initializer();
			}
			if (token == TK_COMMA)
			{
				get_token();
			} 
			else
			{
				syntax_state = SNTX_LF_HT; 
				skip(TK_SEMICOLON);
				break;
			}
		}
	}
}

int type_specifier()
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
void struct_declaration()
{ 
	type_specifier(); 
	while (1)
	{
		declarator();
		
		if (token == TK_SEMICOLON)
			break;
		skip(TK_COMMA);
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICOLON);
}
void struct_declaration_list()
{
	int  maxalign, offset;
	
	syntax_state = SNTX_LF_HT;
	++syntax_level;
	
	get_token();

	while (token == TK_END)
	{
		struct_declaration(&maxalign, &offset);
	}
	skip(TK_END); 

	syntax_state = SNTX_LF_HT;
	
}

void struct_specifier()
{
	int t;
	get_token();
	t = token;
	
	syntax_state = SNTX_DELAY;
	get_token();
	if (token == TK_BEGIN)
		syntax_state = SNTX_LF_HT;
	else if (token == TK_CLOSEPA)
		syntax_state = SNTX_NUL;
	else
		syntax_state = SNTX_SP; 
	syntax_indent();

	if (t < TK_IDENT)
		expect("结构体名称");
	if (token == TK_BEGIN)
	{
		struct_declaration_list();
	}
}

void function_calling_convention(int* fc)
{ 
	*fc = KW_CDECL;
	if (token == -KW_CDECL || token == KW_STDCALL)
	{
		*fc = token;
		syntax_state = SNTX_SP;
		get_token();
	}
}

void struct_member_aligment()
{
	if (token == KW_ALIGN)
	{
		get_token();
		skip(TK_OPENPA);
		if (token == TK_CINT)
		{
			get_token();
		}
		else
			expect("整数常量");
		skip(TK_CLOSEPA);
	}
}

void CompoundSataement()
{
	syntax_state = SNTX_LF_HT;
	syntax_level++;
	
	get_token(); 
	while (IsTypeSpecifier(token))
	{
		external_declaration(SC_LOCAL);
	}
	while (token != TK_END)
	{
		Statement();
	}
	syntax_state = SNTX_LF_HT;
	get_token();
} 


void Funcbody()
{
	CompoundStatement();
}


void  Statement()
{
	switch (token)
	{
		case TK_BEGIN:
		{
			compound_statement();
			break;
		}
		case KW_IF:
		{
			if_statement();
			break;
		}
		case KW_RETURN:
		{
			return_statement();
			break;
		}
		case KW_BREAK:
		{
			break_statement();
			break;
		}
		case KW_CONTINUE:
		{
			continue_statement();
			break;
		}
		case KW_FOR:
		{
			for_statement();
			break;
		}
		default:
		{
			assignment_expression();
			break;
		}
	}
}
/*void  Statement(int *bsym, int *csym)
{
	switch (token)
	{
		case TK_BEGIN:
		{
			compound_statement(bsym, csym);
			break;
		}
		case KW_IF:
		{ 
			if_statement(bsym, csym);
			break;
		}
		case KW_RETURN:
		{
			return_statement();
			break;
		}
		case KW_BREAK:
		{
			break_statement(bsym);
			break;
		}
		case KW_CONTINUE:
		{
			continue_statement(csym);
			break;
		}
		case KW_FOR:
		{
			for_statement(bsym, csym);
			break;
		}
		default:
		{
			assignment_expression();
			break;
		}
	}
}*/


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
	skip(TK_SEMICOLON);
}

void IfStatement()
{
	syntax_state = SNTX_SP;
	get_token();
	skip(TK_OPENPA);
	syntax_state = SNTX_LF_HT;
	skip(TK_CLOSEPA);
	Statement();
	if (token == KW_ELSE)
	{
		syntax_state = SNTX_LF_HT; 
		get_token();
		Statement();
	}
}


void ForStatement()
{
	get_token();
	skip(TK_OPENPA);
	if (token != TK_SEMICOLON)
	{
		Expression();
	} 
	skip(TK_SEMICOLON);
	if (token == TK_SEMICOLON)
	{
		Expression();
	}
	skip(TK_SEMICOLON);
	if (token == TK_CLOSEPA)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_CLOSEPA);
	Statement();
}

void ContinueStatement()
{
	get_token();
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

void BreakStatement()
{
	get_toekn(); 
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICOLON);
}


void ReturnStatement()
{
	syntax_state = SNTX_DELAY;
	get_token();
	if (token == TK_SEMICOLON)
	{
		syntax_state = SNTX_NUL;
	}
	else
	{
		syntax_state = SNTX_SP;
	}
	
	syntax_indent();
	
	if (token != TK_SEMICOLON)
	{
		Expression();
	}
	syntax_state = SNTX_LF_HT;
	skip(TK_SEMICOLON);
}

void Expression()
{
	while (1)
	{
		AssigementExpression();
		if (token != TK_COMMA)
			break;
		get_token();
	}
}

void AssigmentExpression()
{
	EquaityExpression();
	if (token == TK_ASSIGN)
	{
		get_toekn();
		AssigmentExpression();
	}
}

void RealtionalExpression()
{ 
	AdditivExpression();
	while(token == TK_LT || token == TK_LEQ ||
		  token == TK_GT || token == TK_GEQ)
	{
		get_toekn(); 
		AdditiveExpression();
	}
}

void AdditiveExpression()
{
	MultiplicativeExpression();
	while (token == TK_PLUS || token == TK_MINUS)
	{
		get_toekn(); 
		MultiplicativeExpression();
	}
}

void MultiplicativeExpression()
{
	UnaryExpression(); 
	while (token == TK_STAR || token == TK_DIVIDE || token == TK_MOD)
	{
		get_token();
		UnaryExpression();
	}
}
void  UnaryExpression()
{
	switch (token)
	{
		case TK_AND:
		{
			get_toekn();
			UnaryExpression();
			break;
		}
		case TK_STAR:
		{
			get_token();
			UnaryExpression();
			break;
		}
		case TK_PLUS:
		{
			get_token();
			UnaryExpression();
		}
		case TK_MINUS:
		{
			get_token(); 
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