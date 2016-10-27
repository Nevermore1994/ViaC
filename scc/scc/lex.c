#include"scc.h"

typedef TkWord* pTkWord; 

DynArray tktable;
TkWord* tk_hashtable[MAXKEY];
DynString sourcestr;
DynString tkstr;
char ch;
int token;
int tkvalue;

TkWord* tkword_direct_insert(TkWord* pword)
{
	int keyno;
	Dynarray_add(&tktable, pword);
	keyno = elf_hash(pword->spelling);
	pword->next = tk_hashtable[keyno];
	tk_hashtable[keyno] = pword;
	return pword;
} 

TkWord* tkword_find(char* p, int key)
{
	pTkWord res = NULL, tp;
	for (tp = tk_hashtable[key]; tp ; tp = tp->next)
	{
		if ( ! strcmp(p ,tp->spelling))
		{
			token = tp->tkcode;
			res = tp;
		}
	}
	return res;
}

TkWord* tkword_insert(char* p)
{
	pTkWord tp;
	int keyno;
	char* s;
	char* end;
	int length;
	
	keyno = elf_hash(p);
	tp = tkword_find(p, keyno);

	if (tp == NULL)
	{
		length = strlen(p); 
		tp = (pTkWord)mallocz(sizeof(TkWord) + length + 1);
		tp->next = tk_hashtable[keyno];
		tk_hashtable[keyno] = tp;
		Dynarray_add(&tktable, tp);
		tp->tkcode = tktable.count - 1;
		s = (char*)tp + sizeof(TkWord);
		tp->spelling = (char*)s;
		for (end = p + length; p < end; )
		{
			*s++ = *p++;
		} 
		*s = (char) '\0';
	}
	return tp;
}

void* mallocz(int size)
{
	void* ptr = NULL;
	ptr = malloc(size);
	if (!ptr && size)
	{
		error("内存分配错误!");
	}
	memset(ptr, 0, size);
	return ptr;
}

char is_nogiht(char* c)
{
	return (c > 'A' && c < 'Z') || (c > 'a' && c < 'z') || c == '_';
}

void init_lex()
{
	pTkWord tp = NULL;
	static TkWord keywords[] = {
		{ TK_PLUS,		NULL,		 "+",				NULL,	NULL },
		{ TK_MINUS,		NULL,		 "-",				NULL,	NULL },
		{ TK_STAR,		NULL,		 "*",				NULL,	NULL },
		{ TK_DIVIDE,	NULL,		 "/",				NULL,	NULL },
		{ TK_MOD,		NULL,		 "%",				NULL,	NULL },
		{ TK_EQ,		NULL,		 "==",				NULL,	NULL },
		{ TK_NEQ,		NULL,		 "!=",				NULL,	NULL },
		{ TK_LT,		NULL,		 "<",				NULL,	NULL },
		{ TK_LEQ,		NULL,		 "<=",				NULL,	NULL },
		{ TK_GT,		NULL,		 ">",				NULL,	NULL },
		{ TK_GEQ,		NULL,		 ">=",				NULL,	NULL },
		{ TK_ASSIGN,	NULL,		 "=",				NULL,	NULL },
		{ TK_POINTSTO,	NULL,		 "->",				NULL,	NULL },
		{ TK_DOT,		NULL,		 ".",				NULL,	NULL },
		{ TK_AND,		NULL,		 "&",				NULL,	NULL },
		{ TK_OPENPA,	NULL,		 "(",				NULL,	NULL },
		{ TK_CLOSEPA,	NULL,		 ")",				NULL,	NULL },
		{ TK_OPENBR,	NULL,		 "[",				NULL,	NULL },
		{ TK_CLOSEBR,	NULL,		 "]",				NULL,	NULL },
		{ TK_BEGIN,		NULL,		 "{",				NULL,	NULL },
		{ TK_END,		NULL,		 "}",				NULL,	NULL },
		{ TK_SEMICOLON,	NULL,		 ";",				NULL,	NULL },
		{ TK_COMMA,		NULL,		 ",",				NULL,	NULL },
		{ TK_ELLIPSIS,	NULL,		 "...",				NULL,	NULL },
		{ TK_EOF,		NULL,		 "End_Of_File",		NULL,	NULL },
														
		{ TK_CINT,		NULL,	 	"整型常量",			NULL,	NULL },
		{ TK_CCHAR,		NULL,		"字符常量",			NULL,	NULL },
		{ TK_CSTR,		NULL,		"字符串常量",		NULL,	NULL },
														
		{ KW_CHAR,		NULL,		"char",				NULL,	NULL },
		{ KW_SHORT,		NULL,		"short",			NULL,	NULL },
		{ KW_INT,		NULL,		"int",				NULL,	NULL },
		{ KW_VOID,		NULL,		"void",				NULL,	NULL },
		{ KW_STRUCT,	NULL,		"struct",			NULL,	NULL },
														
		{ KW_IF,		NULL,		"if",				NULL,	NULL },
		{ KW_ELSE,		NULL,		"else",				NULL,	NULL },
		{ KW_FOR,		NULL,		"for",				NULL,	NULL },
		{ KW_CONTINUE,	NULL,		"continue",			NULL,	NULL },
		{ KW_BREAK,		NULL,		"break",			NULL,	NULL },
		{ KW_RETURN,	NULL,		"return",			NULL,	NULL },
		{ KW_SIZEOF,	NULL,		"sizeof",			NULL,	NULL },
		{ KW_ALIGN,		NULL,		"__align",			NULL,	NULL },
		{ KW_CDECL,		NULL,		"__cdecl",			NULL,	NULL },
		{ KW_STDCALL,	NULL,		"__stdcall",		NULL,	NULL },
		{0,				NULL,		NULL,				NULL,	NULL}
	};			
	Dynarray_init(&tktable, 8);
	for (tp = &keywords[0]; tp->spelling != NULL; tp++)
		tkword_direct_insert(tp);
}

/****************************************************
*函数功能：跳过c这个单词，如果当前单词不是c,提示错误
*c:需要跳过的单词
****************************************************/
void skip(int c)
{
	if (token != c)
	{
		error("缺少'%s'", get_tkstr(c));
	}
	get_token();
}

char* get_tkstr(int c)
{
	if (c > tktable.count)
		return NULL;
	else if (c >= TK_CINT && c <= TK_CSTR)
		return sourcestr.data;
	else
		return ((TkWord*)tktable.data[c])->spelling;
}
