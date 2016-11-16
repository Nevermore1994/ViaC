/******************************************
*author:Away
*date:2016-10-28
*function:词法分析模块单元代码
*******************************************/

#include"viac.h"

typedef TkWord* pTkWord;

DynArray tktable;
TkWord* tk_hashtable[MAXKEY];
DynString sourcestr;
DynString tkstr;
char ch;
int token;
int tkvalue;


TkWord* TkwordDirectInsert(TkWord* tp)
{
	int keyno;
	tp->sym_id = NULL;
	tp->sym_struct = NULL;

	DynArrayAdd(&tktable, tp);
	keyno = ElfHash(tp->spelling);

	tp->next = tk_hashtable[keyno];
	tk_hashtable[keyno] = tp;
	return tp;
}



TkWord* TkwordFind(const char *p, const int keyno)
{
	TkWord *tp = NULL, *tp1;
	for (tp1 = tk_hashtable[keyno]; tp1; tp1 = tp1->next)
	{
		if (!strcmp(p, tp1->spelling))
		{
			token = tp1->tkcode;
			tp = tp1;
		}
	}
	return tp;
}


TkWord* TkwordInsert(const char *p)
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
		tp = (TkWord*)MallocInit(sizeof(TkWord) + length + 1);
		tp->next = tk_hashtable[keyno];
		tk_hashtable[keyno] = tp;
		DynArrayAdd(&tktable, tp);
		tp->tkcode = tktable.count - 1;
		s = (char *)tp + sizeof(TkWord);
		tp->spelling = (char *)s;
		for (end = p + length; p < end; ++p)
		{
			*s = *p;
			++s;
		}
		*s = (char) '\0';

		tp->sym_id = NULL;
		tp->sym_struct = NULL;
	}
	return tp;
}

void  GetCh()
{
	ch = getc(fin);
}

void InitLex()
{
	TkWord* tp;
	static TkWord keywords[] = {
		{ TK_PLUS,		NULL,	  "+",	            NULL,	NULL },
		{ TK_MINUS,		NULL,	  "-",	            NULL,	NULL },
		{ TK_STAR,		NULL,	  "*",	            NULL,	NULL },
		{ TK_DIVIDE,	NULL,	  "/",	            NULL,	NULL },
		{ TK_MOD,		NULL,	  "%",	            NULL,	NULL },
		{ TK_EQ,		NULL,	  "==",             NULL,	NULL },
		{ TK_NEQ,		NULL,	  "!=",             NULL,	NULL },
		{ TK_LT,		NULL,	  "<",	            NULL,	NULL },
		{ TK_LEQ,		NULL,	  "<=",             NULL,	NULL },
		{ TK_GT,		NULL,	  ">",	            NULL,	NULL },
		{ TK_GEQ,		NULL,	  ">=",             NULL,	NULL },
		{ TK_ASSIGN,	NULL,	  "=",	            NULL,	NULL },
		{ TK_POINTSTO,	NULL,	  "->",             NULL,	NULL },
		{ TK_DOT,		NULL,	  ".",	            NULL,	NULL },
		{ TK_AND,		NULL,	  "&",	            NULL,	NULL },
		{ TK_OPENPA,	NULL,	  "(",	            NULL,	NULL },
		{ TK_CLOSEPA,	NULL,	  ")",	            NULL,	NULL },
		{ TK_OPENBR,	NULL,	  "[",	            NULL,	NULL },
		{ TK_CLOSEBR,	NULL,	  "]",	            NULL,	NULL },
		{ TK_BEGIN,		NULL,	  "{",	            NULL,	NULL },
		{ TK_END,		NULL,	  "}",	            NULL,	NULL },
		{ TK_SEMICOLON,	NULL,	  ";",	            NULL,	NULL },
		{ TK_COMMA,		NULL,	  ",",	            NULL,	NULL },
		{ TK_ELLIPSIS,	NULL,	  "...",            NULL,	NULL },
		{ TK_EOF,		NULL,	 "End_Of_File\n",	NULL,	NULL },
		{ TK_SPACE,		NULL,		"",			    NULL,	NULL },
		{ TK_CINT,		NULL,	 	"整型常量",	    NULL,	NULL },
		{ TK_CCHAR,		NULL,		"字符常量",	    NULL,	NULL },
		{ TK_CSTR,		NULL,		"字符串常量",	NULL,	NULL },

		{ KW_CHAR,		NULL,		"char",	        NULL,	NULL },
		{ KW_SHORT,		NULL,		"short",        NULL,	NULL },
		{ KW_INT,		NULL,		"int",	        NULL,	NULL },
		{ KW_VOID,		NULL,		"void",	        NULL,	NULL },
		{ KW_STRUCT,	NULL,		"struct",       NULL,	NULL },

		{ KW_IF,		NULL,		"if",           NULL,	NULL },
		{ KW_ELSE,		NULL,		"else",	        NULL,	NULL },
		{ KW_FOR,		NULL,		"for",	        NULL,	NULL },
		{ KW_CONTINUE,	NULL,		"continue",     NULL,	NULL },
		{ KW_BREAK,		NULL,		"break",        NULL,	NULL },
		{ KW_RETURN,	NULL,		"return",       NULL,	NULL },
		{ KW_SIZEOF,	NULL,		"sizeof",       NULL,	NULL },
		{ KW_INCLUDE,	NULL,		"include",      NULL,	NULL },
		{ KW_DO,		NULL,		"do",			NULL,	NULL },
		{ KW_END,		NULL,		"end",			NULL,	NULL },
		{ KW_ALIGN,		NULL,		"__align",      NULL,	NULL },
		{ KW_CDECL,		NULL,		"__cdecl",      NULL,	NULL },
		{ KW_STDCALL,	NULL,		"__stdcall",    NULL,	NULL },
		{ 0,			NULL,	    NULL,	        NULL,	NULL }
	};

	DynArrayInit(&tktable, 8);
	for (tp = &keywords[0]; tp->spelling != NULL; tp++)
		TkwordDirectInsert(tp);
}



/****************************************************
*函数功能：跳过c这个单词，如果当前单词不是c,提示错误
*c:需要跳过的单词
****************************************************/

char* GetTkstr(const int c)
{
	if (c > tktable.count)
		return NULL;
	else if (c >= TK_CINT && c <= TK_CSTR)
		return sourcestr.data;
	else
		return ((TkWord*)tktable.data[c])->spelling;
}

void Preprocess()
{
	while (1)
	{
		if (ch == ' ' || ch == '\t' || ch == '\r')
		{
			SkipWhiteSpace();
		}
		else if (ch == '/')
		{
			 GetCh();
			if (ch == '*')
			{
				ParseComment();
			}
			else if (ch == '/')
			{
				ParseComment_2();
			}
			else
			{
				ungetc(ch, fin);
				ch = '/';
				break;
			}
		}
		else if (ch == '#')
		{
			ParseComment_2();
		}
		else
			break;
	}
}

void ParseComment_2()
{
	 GetCh();
	while (1)
	{
		if (ch == '\n')
		{
			++linenum;
			break;
		}
		else if (ch == CH_EOF)
		{
			return;
		}
		else
		{
			 GetCh();
		}

	}
	 GetCh();
}


void ParseComment()
{
	 GetCh();
	while (1)
	{
		while (1)
		{
			if (ch == '\n' || ch == '*' || ch == CH_EOF)
				break;
			else
				 GetCh();
		};
		if (ch == '\n')
		{
			linenum++;
			 GetCh();
		}
		else if (ch == '*')
		{
			 GetCh();
			if (ch == '/')
			{
				 GetCh();
				return;                              //注释完成		
			}
		}
		else
		{
			Error("缺少注释符");
			return;
		}
	}
}

void SkipWhiteSpace()
{
	while (ch == ' ' || ch == '\t' || ch == '\r')
	{
		if (ch == '\r')
		{
			 GetCh();
			if (ch != '\n')
				return;
			linenum++;
		}
		else if (ch == ' ')
			printf("%c", ch);
		else
			printf("%c", ch);  //决定是否打印源码中的换行
		 GetCh();
	}
}


int IsNoDigit(const char c)
{
	return (c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z') ||
		c == '_';
}

int IsDigit(const char c)
{
	return c >= '0' && c <= '9';
}


void ParseIdentifier()
{
	DynStringReset(&tkstr);
	DynStringChcat(&tkstr, ch);
	 GetCh();
	while (IsNoDigit(ch) || IsDigit(ch))
	{
		DynStringChcat(&tkstr, ch);
		 GetCh();
	}
	DynStringChcat(&tkstr, '\0');
}


void ParseNum()
{
	DynStringReset(&tkstr);
	DynStringReset(&sourcestr);
	do
	{
		DynStringChcat(&tkstr, ch);
		DynStringChcat(&sourcestr, ch);
		 GetCh();
	} while (IsDigit(ch));
	if (ch == '.')
	{
		do
		{
			DynStringChcat(&tkstr, ch);
			DynStringChcat(&sourcestr, ch);
		} while (IsDigit(ch));
	}
	DynStringChcat(&tkstr, '\0');
	DynStringChcat(&sourcestr, '\0');
	tkvalue = atoi(tkstr.data);
}

void ParseString(const char sep)
{
	char c;
	DynStringReset(&tkstr);
	DynStringReset(&sourcestr);
	DynStringChcat(&sourcestr, sep);
	 GetCh();
	while (1)
	{
		if (ch == sep)
			break;
		else if (ch == '\\')
		{
			DynStringChcat(&sourcestr, ch);
			 GetCh();
			switch (ch)
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
				case 'r':
					c = '\r';
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
					{
						Warning("非法转义字符:\' \\ %c \'", c);
					}
					else
					{
						Warning("非法转义字符:\' \\ 0x%x \'", c);
					}
					break;
			}
			DynStringChcat(&tkstr, c);
			DynStringChcat(&sourcestr, ch);
			 GetCh();
		}
		else
		{
			DynStringChcat(&tkstr, ch);
			DynStringChcat(&sourcestr, ch);
			 GetCh();
		}
	}
	DynStringChcat(&tkstr, '\0');
	DynStringChcat(&sourcestr, sep);
	DynStringChcat(&sourcestr, '\0');
	 GetCh();
}



void GetToken()
{
	Preprocess();
	switch (ch)
	{
		case 'a':	case 'b':	case 'c':	case 'd':
		case 'e':	case 'f':	case 'g':	case 'h':
		case 'i':	case 'j':	case 'k':	case 'l':
		case 'm':	case 'n':	case 'o':	case 'p':
		case 'q':	case 'r':	case 's':	case 't':
		case 'u':	case 'v':	case 'w':	case 'x':
		case 'y':	case 'z':
		case 'A':	case 'B':	case 'C':	case 'D':
		case 'E':	case 'F':	case 'G':	case 'H':
		case 'I':	case 'J':	case 'K':	case 'L':
		case 'M':	case 'N':	case 'O':	case 'P':
		case 'Q':	case 'R':	case 'S':	case 'T':
		case 'U':	case 'V':	case 'W':	case 'X':
		case 'Y':	case 'Z':
		case '_':
		{
			TkWord* tp;
			ParseIdentifier();
			tp = TkwordInsert(tkstr.data);
			token = tp->tkcode;
			break;
		}
		case '0':	case '1':	case '2':	case'3':
		case '4':   case '5':	case '6':	case '7':
		case '8':	case '9':
		{
			ParseNum();
			token = TK_CINT;
			break;
		}
		case '+':
		{
			 GetCh();
			token = TK_PLUS;
			break;
		}
		case '-':
		{
			 GetCh();
			if (ch == '>')
			{
				token = TK_POINTSTO;
				 GetCh();
			}
			else
				token = TK_MINUS;
			break;
		}
		case '%':
		{
			 GetCh();
			token = TK_MOD;
			break;
		}
		case '/':
		{
			 GetCh();
			token = TK_DIVIDE;
			break;
		}
		case '=':
		{
			 GetCh();
			if (ch == '=')
			{
				token = TK_EQ;
				 GetCh();
			}
			else
			{
				token = TK_ASSIGN;
			}
			break;
		}
		case '!':
		{
			 GetCh();
			if (ch == '=')
			{
				token = TK_NEQ;
				 GetCh();
			}
			else
			{
				Error("不能识别,非法操作符");
				//TODO:逻辑取反
			}
			break;
		}
		case '<':
		{
			 GetCh();
			if (ch == '<')
			{
				Error("非法操作符");
				//TODO:左移位操作符
			}
			else if (ch == '=')
			{
				token = TK_LEQ;
				 GetCh();
			}
			else
				token = TK_LT;
			break;
		}
		case '>':
		{
			 GetCh();
			if (ch == '>')
			{
				Error("非法操作符");
				//TODO:右移位操作符
			}
			else if (ch == '=')
			{
				token = TK_GEQ;
				 GetCh();
			}
			else
				token = TK_GT;

			break;
		}
		case '.':
		{
			 GetCh();
			if (ch == '.')
			{
				 GetCh();
				if (ch == '.')
				{
					token = TK_ELLIPSIS;
				}
				else
				{
					Error("拼写错误");
				}
				 GetCh();
			}
			else
			{
				token = TK_DOT;
			}
			break;
		}
		case '&':
		{
			if (ch == '&')
			{
				Error("不能识别的操作符");
				//TODO:与逻辑操作符
			}
			else
			{
				token = TK_AND;
			}
			 GetCh();
			break;
		}
		case '|':
		{
			 GetCh();
			if (ch == '|')
			{
				Error("不能识别的操作符");
				//TODO:或逻辑操作符
			}
			else
			{
				Error("不能识别的操作符");
				//TODO:或的逻辑操作符
			}
			break;
		}
		case '(':
		{
			token = TK_OPENPA;
			 GetCh();
			break;
		}
		case ')':
		{
			token = TK_CLOSEPA;
			 GetCh();
			break;
		}
		case '[':
		{
			token = TK_OPENBR;
			 GetCh();
			break;
		}
		case ']':
		{
			token = TK_CLOSEBR;
			 GetCh();
			break;
		}
		case '{':
		{
			token = TK_BEGIN;
			 GetCh();
			break;
		}
		case '}':
		{
			token = TK_END;
			 GetCh();
			break;
		}
		case ';':
		{
			token = TK_SEMICOLON;
			 GetCh();
			break;
		}
		case '\n':
		{
			token = TK_SPACE;
			 GetCh();
			break;
		}
		case ',':
		{

			token = TK_COMMA;
			 GetCh();
			break;
		}
		case '*':
		{
			token = TK_STAR;
			 GetCh();
			break;
		}
		case '\"':
		{
			ParseString(ch);
			token = TK_CSTR;
			break;
		}
		case '\'':
		{
			ParseString(ch);
			token = TK_CCHAR;
			tkvalue = *(char*)tkstr.data;
			break;
		}
		case EOF:
		{
			token = TK_EOF;
			break;
		}
		default:
		{
			Error("未能识别的操作符%c", ch);
			 GetCh();
			break;
		}
	}
	SyntaxIndent();
}


void ColorToken(const int lex_state)
{
	HANDLE had = GetStdHandle(STD_OUTPUT_HANDLE);
	char* p;
	switch (lex_state)
	{
		case LEX_NORMAL:
		{
			if (token >= TK_IDENT)
				SetConsoleTextAttribute(had, FOREGROUND_INTENSITY | FOREGROUND_BLUE | FOREGROUND_GREEN);
			else if (token >= KW_CHAR)
				SetConsoleTextAttribute(had, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if (token >= TK_CINT)
				SetConsoleTextAttribute(had, FOREGROUND_RED | FOREGROUND_GREEN);
			else
				SetConsoleTextAttribute(had, FOREGROUND_RED | FOREGROUND_INTENSITY);
			p = GetTkstr(token);
			printf("%s", p);
			break;
		}
		case LEX_SEP:
		{
			printf("%c", ch);
			break;
		}
	}
}


void TestLex()
{
	do
	{
		GetToken();
		ColorToken(LEX_NORMAL);
	} while (token != TK_EOF);
	printf("\n代码行数:%d行\n", linenum);
}

