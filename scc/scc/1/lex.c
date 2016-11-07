// 《自己动手写编译器、链接器》配套源代码

#include "scc.h"
TkWord* tk_hashtable [MAXKEY];	// 单词哈希表
Dynarray tktable;				// 单词表中放置标识符，包括变量名,函数名,结构定义名
DynString tkstr;
DynString sourcestr;
int tkvalue;
DynString tkstr;
char ch;
int token;
int tkvalue;
int line_num;


/***********************************************************
* 功能: 将运算符、关键字、常量直接放入单词表
* tp:	 单词指针
**********************************************************/
TkWord* TkwordDirectInsert(TkWord* tp)
{
	int keyno;
	tp->sym_id = NULL;
	tp->sym_struct = NULL;
	Dynarray_add(&tktable, tp);
	keyno = ElfHash(tp->spelling);
	tp->next = tk_hashtable [keyno];
	tk_hashtable [keyno] = tp;
	return tp;
}

/***********************************************************
*	功能:	在单词表中查找单词
*  p:		单词字符串指针
*  keyno:	单词的哈希值
**********************************************************/
TkWord* TkwordFind(char *p, int keyno)
{
	TkWord *tp = NULL, *tp1;
	for (tp1 = tk_hashtable [keyno]; tp1; tp1 = tp1->next)
	{
		if (!strcmp(p, tp1->spelling))
		{
			token = tp1->tkcode;
			tp = tp1;
		}
	}
	return tp;
}


/***********************************************************
* 功能:	标识符插入单词表，先查找，查找不到再插入单词表
* p:		单词字符串指针
**********************************************************/
TkWord* TkwordInsert(char *p)
{
	TkWord*  tp;
	int keyno;
	char   *s;
	char *end;
	int length;

	keyno = ElfHash(p);
	tp = TkwordFind(p, keyno);
	if (tp == NULL)
	{
		length = strlen(p);
		tp = ( TkWord* ) MallocInit(sizeof(TkWord) + length + 1);
		tp->next = tk_hashtable [keyno];
		tk_hashtable [keyno] = tp;
		Dynarray_add(&tktable, tp);
		tp->tkcode = tktable.count - 1;
		s = ( char * ) tp + sizeof(TkWord);
		tp->spelling = ( char * ) s;
		for (end = p + length; p < end;)
		{
			*s++ = *p++;
		}
		*s = (char) '\0';

		tp->sym_id = NULL;
		tp->sym_struct = NULL;
	}
	return tp;
}

/***********************************************************
* 功能:	判断c是否为字母(a-z,A-Z)或下划线(-)
* c:		字符值
**********************************************************/
int IsNoDigit(char c)
{
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

/***********************************************************
* 功能:	判断c是否为数字
* c:		字符值
**********************************************************/
int IsDigit(char c)
{
	return c >= '0' && c <= '9';
}

/***********************************************************
* 功能:	从SC源文件中读取一个字符
**********************************************************/
void Getch()
{
	ch = getc(fin); //文件尾返回EOF，其它返回实际字节值		
}

/***********************************************************
* 功能:	解析标识符
**********************************************************/
void ParseIdentifier()
{
	Dynstring_reset(&tkstr);
	Dynstring_chcat(&tkstr, ch);
	Getch();
	while (IsNoDigit(ch) || IsDigit(ch))
	{
		Dynstring_chcat(&tkstr, ch);
		Getch();
	}
	Dynstring_chcat(&tkstr, '\0');
}

/***********************************************************
* 功能:	解析整型常量
**********************************************************/
void ParseNum()
{
	Dynstring_reset(&tkstr);
	Dynstring_reset(&sourcestr);
	do
	{
		Dynstring_chcat(&tkstr, ch);
		Dynstring_chcat(&sourcestr, ch);
		Getch();
	} while (IsDigit(ch));
	if (ch == '.')
	{
		do
		{
			Dynstring_chcat(&tkstr, ch);
			Dynstring_chcat(&sourcestr, ch);
			Getch();
		} while (IsDigit(ch));
	}
	Dynstring_chcat(&tkstr, '\0');
	Dynstring_chcat(&sourcestr, '\0');
	tkvalue = atoi(tkstr.data);
}

/***********************************************************
* 功能:	解析字符常量、字符串常量
* sep:		字符常量界符标识为单引号(')
字符串常量界符标识为双引号(")
**********************************************************/
void ParseString(char sep)
{
	char c;
	Dynstring_reset(&tkstr);
	Dynstring_reset(&sourcestr);
	Dynstring_chcat(&sourcestr, sep);
	Getch();
	for (;;)
	{
		if (ch == sep)
			break;
		else if (ch == '\\')
		{
			Dynstring_chcat(&sourcestr, ch);
			Getch();
			switch (ch) // 解析转义字符
			{
				case '0':
					c = '\0';
					break;
				case 'a':
					c = '\a';
					break;
				case 'b':
					c = '\b';
					break;
				case 't':
					c = '\t';
					break;
				case 'n':
					c = '\n';
					break;
				case 'v':
					c = '\v';
					break;
				case 'f':
					c = '\f';
					break;
				case 'r':
					c = '\r';
					break;
				case '\"':
					c = '\"';
					break;
				case '\'':
					c = '\'';
					break;
				case '\\':
					c = '\\';
					break;
				default:
					c = ch;
					if (c >= '!' && c <= '~')
						Warning("非法转义字符: \'\\%c\'", c); // 33-126 0x21-0x7E可显示字符部分
					else
						Warning("非法转义字符: \'\\0x%x\'", c);
					break;
			}
			Dynstring_chcat(&tkstr, c);
			Dynstring_chcat(&sourcestr, ch);
			Getch();
		}
		else
		{
			Dynstring_chcat(&tkstr, ch);
			Dynstring_chcat(&sourcestr, ch);
			Getch();
		}

	}
	Dynstring_chcat(&tkstr, '\0');
	Dynstring_chcat(&sourcestr, sep);
	Dynstring_chcat(&sourcestr, '\0');
	Getch();
}

/***********************************************************
* 功能:	词法着色
**********************************************************/
void ColorToken(int lex_state)
{
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	char *p;
	switch (lex_state)
	{
		case LEX_NORMAL:
		{
			if (token >= TK_IDENT)
				SetConsoleTextAttribute(h, FOREGROUND_INTENSITY);
			else if (token >= KW_CHAR)
				SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if (token >= TK_CINT)
				SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN);
			else
				SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
			p = GetTkstr(token);
			printf("%s", p);
			break;
		}
		case LEX_SEP:
			printf("%c", ch);
			break;
	}
}


/***********************************************************
* 功能:	词法分析初始化
**********************************************************/
void InitLex()
{
	TkWord* tp;
	static TkWord keywords[] = {
		{ TK_PLUS,		NULL,	  "+",	NULL,	NULL },
		{ TK_MINUS,		NULL,	  "-",	NULL,	NULL },
		{ TK_STAR,		NULL,	  "*",	NULL,	NULL },
		{ TK_DIVIDE,		NULL,	  "/",	NULL,	NULL },
		{ TK_MOD,		NULL,	  "%",	NULL,	NULL },
		{ TK_EQ,			NULL,	  "==",	NULL,	NULL },
		{ TK_NEQ,		NULL,	  "!=",	NULL,	NULL },
		{ TK_LT,			NULL,	  "<",	NULL,	NULL },
		{ TK_LEQ,		NULL,	  "<=",	NULL,	NULL },
		{ TK_GT,			NULL,	  ">",	NULL,	NULL },
		{ TK_GEQ,		NULL,	  ">=",	NULL,	NULL },
		{ TK_ASSIGN,		NULL,	  "=",	NULL,	NULL },
		{ TK_POINTSTO,	NULL,	  "->",	NULL,	NULL },
		{ TK_DOT,		NULL,	  ".",	NULL,	NULL },
		{ TK_AND,		NULL,	  "&",	NULL,	NULL },
		{ TK_OPENPA,		NULL,	  "(",	NULL,	NULL },
		{ TK_CLOSEPA,	NULL,	  ")",	NULL,	NULL },
		{ TK_OPENBR,		NULL,	  "[",	NULL,	NULL },
		{ TK_CLOSEBR,	NULL,	  "]",	NULL,	NULL },
		{ TK_BEGIN,		NULL,	  "{",	NULL,	NULL },
		{ TK_END,		NULL,	  "}",	NULL,	NULL },
		{ TK_SEMICOLON,	NULL,	  ";",	NULL,	NULL },
		{ TK_COMMA,		NULL,	  ",",	NULL,	NULL },
		{ TK_ELLIPSIS,	NULL,	"...",	NULL,	NULL },
		{ TK_EOF,		NULL,	 "End_Of_File",	NULL,	NULL },

		{ TK_CINT,		NULL,	 	"整型常量",	NULL,	NULL },
		{ TK_CCHAR,		NULL,		"字符常量",	NULL,	NULL },
		{ TK_CSTR,		NULL,		"字符串常量",	NULL,	NULL },

		{ KW_CHAR,		NULL,		"char",	NULL,	NULL },
		{ KW_SHORT,		NULL,		"short",	NULL,	NULL },
		{ KW_INT,		NULL,		"int",	NULL,	NULL },
		{ KW_VOID,		NULL,		"void",	NULL,	NULL },
		{ KW_STRUCT,		NULL,		"struct",	NULL,	NULL },

		{ KW_IF,			NULL,		"if"	,	NULL,	NULL },
		{ KW_ELSE,		NULL,		"else",	NULL,	NULL },
		{ KW_FOR,		NULL,		"for",	NULL,	NULL },
		{ KW_CONTINUE,	NULL,		"continue",	NULL,	NULL },
		{ KW_BREAK,		NULL,		"break",	NULL,	NULL },
		{ KW_RETURN,		NULL,		"return",	NULL,	NULL },
		{ KW_SIZEOF,		NULL,		"sizeof",	NULL,	NULL },
		{ KW_ALIGN,		NULL,		"__align",	NULL,	NULL },
		{ KW_CDECL,		NULL,		"__cdecl",	NULL,	NULL },
		{ KW_STDCALL,	NULL,		"__stdcall",	NULL,	NULL },
		{ 0,				NULL,	NULL,	NULL,		NULL }
	};

	Dynarray_init(&tktable, 8);
	for (tp = &keywords [0]; tp->spelling != NULL; tp++)
		TkwordDirectInsert(tp);

}

/***********************************************************
*  功能:	忽略空格,TAB及回车
**********************************************************/
void SkipWhiteSpace()
{
	while (ch == ' ' || ch == '\t' || ch == '\r')
	{
		if (ch == '\r')
		{
			Getch();
			if (ch != '\n')
				return;
			line_num++;
		}
		else
			printf("%c", ch); //这句话，决定是否打印空格，如果不输出空格，源码中空格将被去掉，所有源码挤在一起
		Getch();
	}

}

/***********************************************************
*  功能:	预处理，忽略分隔符及注释
**********************************************************/
void Preprocess()
{
	while (1)
	{
		if (ch == ' ' || ch == '\t' || ch == '\r')
			SkipWhiteSpace();
		else if (ch == '/')
		{
			// 向前多读一个字节看是否是注释开始符，猜错了把多读的字符再放回去
			Getch();
			if (ch == '*')
			{
				ParseComment();
			}
			else
			{
				ungetc(ch, fin); //把一个字符退回到输入流中
				ch = '/';
				break;
			}
		}
		else
			break;
	}
}

/***********************************************************
*  功能:	解析注释
**********************************************************/
void ParseComment()
{
	Getch();
	do
	{
		do
		{
			if (ch == '\n' || ch == '*' || ch == CH_EOF)
				break;
			else
				Getch();
		} while (1);
		if (ch == '\n')
		{
			line_num++;
			Getch();
		}
		else if (ch == '*')
		{
			Getch();
			if (ch == '/')
			{
				Getch();
				return;
			}
		}
		else
		{
			Error("一直到文件尾未看到配对的注释结束符");
			return;
		}
	} while (1);
}

/***********************************************************
*  功能:	取单词
**********************************************************/
void GetToken()
{
	Preprocess();
	switch (ch)
	{
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
		case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
		case 'o': case 'p': case 'q': case 'r': case 's': case 't':
		case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
		case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
		case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
		case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
		{
			TkWord* tp;
			ParseIdentifier();
			tp = TkwordInsert(tkstr.data);
			token = tp->tkcode;
			break;
		}
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
		case '8': case '9':
			ParseNum();
			token = TK_CINT;
			break;
		case '+':
			Getch();
			token = TK_PLUS;
			break;
		case '-':
			Getch();
			if (ch == '>')
			{
				token = TK_POINTSTO;
				Getch();
			}
			else
				token = TK_MINUS;
			break;
		case '/':
			token = TK_DIVIDE;
			Getch();
			break;
		case '%':
			token = TK_MOD;
			Getch();
			break;
		case '=':
			Getch();
			if (ch == '=')
			{
				token = TK_EQ;
				Getch();
			}
			else
				token = TK_ASSIGN;
			break;
		case '!':
			Getch();
			if (ch == '=')
			{
				token = TK_NEQ;
				Getch();
			}
			else
				Error("暂不支持'!'(非操作符)");
			break;
		case '<':
			Getch();
			if (ch == '=')
			{
				token = TK_LEQ;
				Getch();
			}
			else
				token = TK_LT;
			break;
		case '>':
			Getch();
			if (ch == '=')
			{
				token = TK_GEQ;
				Getch();
			}
			else
				token = TK_GT;
			break;
		case '.':
			Getch();
			if (ch == '.')
			{
				Getch();
				if (ch != '.')
					Error("省略号拼写错误");
				else
					token = TK_ELLIPSIS;
				Getch();
			}
			else
			{
				token = TK_DOT;
			}
			break;
		case '&':
			token = TK_AND;
			Getch();
			break;
		case ';':
			token = TK_SEMICOLON;
			Getch();
			break;
		case ']':
			token = TK_CLOSEBR;
			Getch();
			break;
		case '}':
			token = TK_END;
			Getch();
			break;
		case ')':
			token = TK_CLOSEPA;
			Getch();
			break;
		case '[':
			token = TK_OPENBR;
			Getch();
			break;
		case '{':
			token = TK_BEGIN;
			Getch();
			break;
		case ',':
			token = TK_COMMA;
			Getch();
			break;
		case '(':
			token = TK_OPENPA;
			Getch();
			break;
		case '*':
			token = TK_STAR;
			Getch();
			break;
		case '\'':
			ParseString(ch);
			token = TK_CCHAR;
			tkvalue = *( char * ) tkstr.data;
			break;
		case '\"':
		{
			ParseString(ch);
			token = TK_CSTR;
			break;
		}
		case EOF:
			token = TK_EOF;
			break;
		default:
			Error("不认识的字符:\\x%02x", ch); //上面字符以外的字符，只允许出现在源码字符串，不允许出现的源码的其它位置
			Getch();
			break;
	}
	SyntaxIndent();
}


/***********************************************************
* 功能:	取得单词v所代表的源码字符串，错误提示用
* v:		单词编号
**********************************************************/
char *GetTkstr(int v)
{
	if (v > tktable.count)
		return NULL;
	else if (v >= TK_CINT && v <= TK_CSTR)
		return sourcestr.data;
	else
		return (( TkWord* ) tktable.data [v])->spelling;
}

/***********************************************************
*  功能:	词法分析测试
**********************************************************/
void TestLex()
{
	do
	{
		GetToken();
		ColorToken(LEX_NORMAL);
	} while (token != TK_EOF);
	printf("\n\n代码行数: %d行\n\n", line_num);
}