#include"scc.h"

typedef TkWord* pTkWord;

DynArray tktable;
TkWord* tk_hashtable [MAXKEY];
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
	Dynarray_add(&tktable, tp);
	keyno = ElfHash(tp->spelling);
	tp->next = tk_hashtable [keyno];
	tk_hashtable [keyno] = tp;
	return tp;
}



TkWord* TkwordFind(char *p, const int keyno)
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
void Getch()
{
	ch = getc(fin);
}


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
		{ KW_INT,		NULL,		"const int",	NULL,	NULL },
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

/****************************************************
*函数功能：跳过c这个单词，如果当前单词不是c,提示错误
*c:需要跳过的单词
****************************************************/

char* GetTkstr(const const int c)
{
	if (c > tktable.count)
		return NULL;
	else if (c >= TK_CINT && c <= TK_CSTR)
		return sourcestr.data;
	else
		return (( TkWord* ) tktable.data [c])->spelling;
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
			Getch();
			if (ch == '*')
			{
				ParseComment();
			}
			else
			{
				ungetc(ch, fin);
				ch = '/';
				break;
			}
		}
		else
			break;
	}
}

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
				return;                              //注释完成		
			}
		}
		else
		{
			Error("缺少注释符");
			return;
		}
	} while (1);
}

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
		printf("%c", ch);
		Getch();
	}
}


int IsNoDigit(char* c)
{
	return (c >= 'A' && c <= 'Z') || 
		   (c >= 'a' && c <= 'z') ||
		    c == '_';
}

int IsDigit(char c)
{
	return c >= '0' && c <= '9';
}


TkWord* ParseIdentifier()
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
	return TkwordInsert(tkstr.data);
}


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
		} while (IsDigit(ch));
	}
	Dynstring_chcat(&tkstr, '\0');
	Dynstring_chcat(&sourcestr, '\0');
	tkvalue = atoi(tkstr.data);
}

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
			tp = ParseIdentifier();
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
			Getch();
			token = TK_PLUS;
			break;
		}
		case '-':
		{
			Getch();
			if (ch == '>')
			{
				token = TK_POINTSTO;
				Getch();
			}
			else
				token = TK_MINUS;
			break;
		}
		case '%':
		{
			Getch();
			token = TK_MOD;
			break;
		}
		case '/':
		{
			Getch();
			token = TK_DIVIDE;
			break;
		}
		case '=':
		{
			Getch();
			if (ch == '=')
			{
				token = TK_EQ;
				Getch();
			}
			else
			{
				token = TK_ASSIGN;
			}
			break;
		}
		case '!':
		{
			Getch();
			if (ch == '=')
			{
				token = TK_NEQ;
				Getch();
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
			Getch();
			if (ch == '<')
			{
				Error("非法操作符");
				//TODO:左移位操作符
			}
			else if (ch == '=')
			{
				token = TK_LEQ;
				Getch();
			}
			else
				token = TK_LT;
			break;
		}
		case '>':
		{
			Getch();
			if (ch == '>')
			{
				Error("非法操作符");
				//TODO:右移位操作符
			}
			else if (ch == '=')
			{
				token = TK_GEQ;
				Getch();
			}
			else
				token = TK_GT;
		
			break;
		}
		case '.':
		{
			Getch();
			if (ch == '.')
			{
				Getch();
				if (ch == '.')
				{
					token = TK_ELLIPSIS;
				}
				else
				{
					Error("拼写错误");
				}
				Getch();
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
			Getch();
			break;
		}
		case '|':
		{
			Getch();
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
			Getch();
			break;
		}
		case ')':
		{
			token = TK_CLOSEPA;
			Getch();
			break;
		}
		case '[':
		{
			token = TK_OPENBR;
			Getch();
			break;
		}
		case ']':
		{
			token = TK_CLOSEBR;
			Getch();
			break;
		}
		case '{':
		{
			token = TK_BEGIN;
			Getch();
			break;
		}
		case '}':
		{
			token = TK_END;
			Getch();
			break;
		}
		case ';':
		{
			token = TK_SEMICOLON;
			Getch();
			break;
		}
		case ',':
		{
			
			token = TK_COMMA;
			Getch();
			break;
		}
		case '*':
		{
			token = TK_STAR;
			Getch();
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
			tkvalue = *( char* ) tkstr.data;
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
			Getch();
			break;
		}
	}
}


void ColorToken(const const int lex_state)
{
	HANDLE had = GetStdHandle(STD_OUTPUT_HANDLE);
	char* p;
	switch (lex_state)
	{
		case LEX_NORMAL:
		{
			if (token >= TK_IDENT)   //标配符为蓝色
				SetConsoleTextAttribute(had, FOREGROUND_INTENSITY);
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
	printf("\n代码行数:%d行\n", line_num);
}

