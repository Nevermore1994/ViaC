#include"scc.h"

typedef TkWord* pTkWord;

DynArray tktable;
TkWord* tk_hashtable [MAXKEY];
DynString sourcestr;
DynString tkstr;
char ch;
int token;
int tkvalue;




TkWord* tkword_direct_insert(TkWord* tp)
{
	int keyno;
	tp->sym_id = NULL;
	tp->sym_struct = NULL;
	Dynarray_add(&tktable, tp);
	keyno = elf_hash(tp->spelling);
	tp->next = tk_hashtable [keyno];
	tk_hashtable [keyno] = tp;
	return tp;
}



TkWord* tkword_find(char *p, int keyno)
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


TkWord* tkword_insert(char *p)
{
	TkWord*  tp;
	int keyno;
	char   *s;
	char *end;
	int length;

	keyno = elf_hash(p);
	tp = tkword_find(p, keyno);
	if (tp == NULL)
	{
		length = strlen(p);
		tp = ( TkWord* ) mallocz(sizeof(TkWord) + length + 1);
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
void getch()
{
	ch = getc(fin);
}


void init_lex()
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

		{ TK_CINT,		NULL,	 	"���ͳ���",	NULL,	NULL },
		{ TK_CCHAR,		NULL,		"�ַ�����",	NULL,	NULL },
		{ TK_CSTR,		NULL,		"�ַ�������",	NULL,	NULL },

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
		tkword_direct_insert(tp);

}

/****************************************************
*�������ܣ�����c������ʣ������ǰ���ʲ���c,��ʾ����
*c:��Ҫ�����ĵ���
****************************************************/

char* get_tkstr(int c)
{
	if (c > tktable.count)
		return NULL;
	else if (c >= TK_CINT && c <= TK_CSTR)
		return sourcestr.data;
	else
		return (( TkWord* ) tktable.data [c])->spelling;
}

void preprocess()
{
	while (1)
	{
		if (ch == ' ' || ch == '\t' || ch == '\r')
		{
			skip_white_space();
		}
		else if (ch == '/')
		{
			getch();
			if (ch == '*')
			{
				parse_comment();
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

void parse_comment()
{
	getch();
	do
	{
		do
		{
			if (ch == '\n' || ch == '*' || ch == CH_EOF)
				break;
			else
				getch();
		} while (1);
		if (ch == '\n')
		{
			line_num++;
			getch();
		}
		else if (ch == '*')
		{
			getch();
			if (ch == '/')
			{
				getch();
				return;                              //ע�����		
			}
		}
		else
		{
			error("ȱ��ע�ͷ�");
			return;
		}
	} while (1);
}

void skip_white_space()
{
	while (ch == ' ' || ch == '\t' || ch == '\r')
	{
		if (ch == '\r')
		{
			getch();
			if (ch != '\n')
				return;
			line_num++;
		}
		printf("%c", ch);
		getch();
	}
}


int is_nogiht(char* c)
{
	return (c >= 'A' && c <= 'Z') || 
		   (c >= 'a' && c <= 'z') ||
		    c == '_';
}

int is_digit(char c)
{
	return c >= '0' && c <= '9';
}


TkWord* parse_identifier()
{
	Dynstring_reset(&tkstr);
	Dynstring_chcat(&tkstr, ch);
	getch();
	while (is_nogiht(ch) || is_digit(ch))
	{
		Dynstring_chcat(&tkstr, ch);
		getch();
	}
	Dynstring_chcat(&tkstr, '\0');
	return tkword_insert(tkstr.data);
}


void parse_num()
{
	Dynstring_reset(&tkstr);
	Dynstring_reset(&sourcestr);
	do
	{
		Dynstring_chcat(&tkstr, ch);
		Dynstring_chcat(&sourcestr, ch);
		getch();
	} while (is_digit(ch));
	if (ch == '.')
	{
		do
		{
			Dynstring_chcat(&tkstr, ch);
			Dynstring_chcat(&sourcestr, ch);
		} while (is_digit(ch));
	}
	Dynstring_chcat(&tkstr, '\0');
	Dynstring_chcat(&sourcestr, '\0');
	tkvalue = atoi(tkstr.data);
}

void parse_string(char sep)
{
	char c;
	Dynstring_reset(&tkstr);
	Dynstring_reset(&sourcestr);
	Dynstring_chcat(&sourcestr, sep);
	getch();
	for (;;)
	{
		if (ch == sep)
			break;
		else if (ch == '\\')
		{
			Dynstring_chcat(&sourcestr, ch);
			getch();
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
						warning("�Ƿ�ת���ַ�:\' \\ %c \'", c);
					}
					else
					{
						warning("�Ƿ�ת���ַ�:\' \\ 0x%x \'", c);
					}
					break;
			}
			Dynstring_chcat(&tkstr, c);
			Dynstring_chcat(&sourcestr, ch);
			getch();
		}
		else
		{
			Dynstring_chcat(&tkstr, ch);
			Dynstring_chcat(&sourcestr, ch);
			getch();
		}
	}
	Dynstring_chcat(&tkstr, '\0');
	Dynstring_chcat(&sourcestr, sep);
	Dynstring_chcat(&sourcestr, '\0');
	getch();
}



void get_token()
{
	preprocess();
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
			tp = parse_identifier();
			token = tp->tkcode;
			break;
		}
		case '0':	case '1':	case '2':	case'3':
		case '4':   case '5':	case '6':	case '7':
		case '8':	case '9':
		{
			parse_num();
			token = TK_CINT;
			break;
		}
		case '+':
		{
			getch();
			token = TK_PLUS;
			break;
		}
		case '-':
		{
			getch();
			if (ch == '>')
			{
				token = TK_POINTSTO;
				getch();
			}
			else
				token = TK_MINUS;
			break;
		}
		case '%':
		{
			getch();
			token = TK_MOD;
			break;
		}
		case '/':
		{
			getch();
			token = TK_DIVIDE;
			break;
		}
		case '=':
		{
			getch();
			if (ch == '=')
			{
				token = TK_EQ;
				getch();
			}
			else
			{
				token = TK_ASSIGN;
			}
			break;
		}
		case '!':
		{
			getch();
			if (ch == '=')
			{
				token = TK_NEQ;
				getch();
			}
			else
			{
				error("����ʶ��,�Ƿ�������");
				//TODO:�߼�ȡ��
			}
			break;
		}
		case '<':
		{
			getch();
			if (ch == '<')
			{
				error("�Ƿ�������");
				//TODO:����λ������
			}
			else if (ch == '=')
			{
				token = TK_LEQ;
				getch();
			}
			else
				token = TK_LT;
			break;
		}
		case '>':
		{
			getch();
			if (ch == '>')
			{
				error("�Ƿ�������");
				//TODO:����λ������
			}
			else if (ch == '=')
			{
				token = TK_GEQ;
				getch();
			}
			else
				token = TK_GT;
		
			break;
		}
		case '.':
		{
			getch();
			if (ch == '.')
			{
				getch();
				if (ch == '.')
				{
					token = TK_ELLIPSIS;
				}
				else
				{
					error("ƴд����");
				}
				getch();
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
				error("����ʶ��Ĳ�����");
				//TODO:���߼�������
			}
			else
			{
				token = TK_AND;
			}
			getch();
			break;
		}
		case '|':
		{
			getch();
			if (ch == '|')
			{
				error("����ʶ��Ĳ�����");
				//TODO:���߼�������
			}
			else
			{
				error("����ʶ��Ĳ�����");
				//TODO:����߼�������
			}
			break;
		}
		case '(':
		{
			token = TK_OPENPA;
			getch();
			break;
		}
		case ')':
		{
			token = TK_CLOSEPA;
			getch();
			break;
		}
		case '[':
		{
			token = TK_OPENBR;
			getch();
			break;
		}
		case ']':
		{
			token = TK_CLOSEBR;
			getch();
			break;
		}
		case '{':
		{
			token = TK_BEGIN;
			getch();
			break;
		}
		case '}':
		{
			token = TK_END;
			getch();
			break;
		}
		case ';':
		{
			token = TK_SEMICOLON;
			getch();
			break;
		}
		case ',':
		{
			
			token = TK_COMMA;
			getch();
			break;
		}
		case '*':
		{
			token = TK_STAR;
			getch();
			break;
		}
		case '\"':
		{
			parse_string(ch);
			token = TK_CSTR;
			break;
		}
		case '\'':
		{
			parse_string(ch);
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
			error("δ��ʶ��Ĳ�����%c", ch);
			getch();
			break;
		}
	}
}


void color_token(int lex_state)
{
	HANDLE had = GetStdHandle(STD_OUTPUT_HANDLE);
	char* p;
	switch (lex_state)
	{
		case LEX_NORMAL:
		{
			if (token >= TK_IDENT)   //�����Ϊ��ɫ
				SetConsoleTextAttribute(had, FOREGROUND_INTENSITY);
			else if (token >= KW_CHAR)
				SetConsoleTextAttribute(had, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			else if (token >= TK_CINT)
				SetConsoleTextAttribute(had, FOREGROUND_RED | FOREGROUND_GREEN);
			else
				SetConsoleTextAttribute(had, FOREGROUND_RED | FOREGROUND_INTENSITY);
			p = get_tkstr(token);
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


void test_lex()
{
	do
	{
		get_token();
		color_token(LEX_NORMAL);
	} while (token != TK_EOF);
	printf("\n��������:%d��\n", line_num);
}

