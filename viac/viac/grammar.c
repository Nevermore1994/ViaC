// ���Լ�����д��������������������Դ����

#include "viac.h"
int syntax_state;
int syntax_level;

/***********************************************************
* ����:	�������뵥λ
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
* ����:	�����ⲿ����
* l:		�洢���ͣ��ֲ��Ļ���ȫ�ֵ�
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
	Type btype, type;
	int v, has_init, r, addr;
	Symbol *sym;
	Section *sec = NULL;

	if (!TypeSpecifier(&btype))
	{
		Expect("<�������ַ�>");
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

		if (token == TK_BEGIN) //��������
		{
			if (l == ViaC_LOCAL)
				Error("��֧�ֺ���Ƕ�׶���");

			if ((type.t & T_BTYPE) != T_FUNC)
				Expect("<��������>");

			sym = SymSearch(v);
			if (sym)	// ����ǰ�������������ڸ�����������
			{
				if ((sym->type.t & T_BTYPE) != T_FUNC)
					Error("'%s'�ض���", GetTkstr(v));
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
			if ((type.t & T_BTYPE) == T_FUNC) // ��������
			{
				if (SymSearch(v) == NULL)
				{
					SymPush(v, &type, ViaC_GLOBAL | ViaC_SYM, 0);
				}
			}
			else //��������
			{
				r = 0;
				if (!(type.t & T_ARRAY))
					r |= ViaC_LVAL;

				r |= l;
				has_init = (token == TK_ASSIGN);

				if (has_init)
				{
					GetToken(); //���ܷŵ����棬char str[]="abc"�������ҪAllocateStorage���ַ�������				    
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
* ����:	������ֵ��
* type:	��������
* c:		�������ֵ
* sec:		�������ڽ�
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
* ����:		�������ַ�
* type(���):	��������
* ����ֵ:		�Ƿ��ֺϷ����������ַ�
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
* ����:		�����ṹ���ַ�
* type(���):	�ṹ����
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
	s = StructSearch(v);
	if (!s)
	{
		type1.t = KW_STRUCT;
		// -1��ʾ�ṹ��δ����
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
* ����:		�����ṹ��������
* type(���):	�ṹ����
*
* <StructDeclarationList>::=<StructDeclaration>{<StructDeclaration>}
**********************************************************/
void StructDeclarationList(Type *type)
{
	int maxalign, offset;
	Symbol *s, **ps;
	s = type->ref;
	syntax_state = SNTX_LF_HT;   // ��һ���ṹ���Ա��'{'��д��һ��
	syntax_level++;              // �ṹ���Ա������������������һ��
	GetToken();
	if (s->c != -1)
		Error("�ṹ���Ѷ���");
	maxalign = 1;
	ps = &s->next;
	offset = 0;
	while (token != TK_END)
	{
		StructDeclaration(&maxalign, &offset, &ps);
	}
	Skip(TK_END);
	syntax_state = SNTX_LF_HT;

	s->c = CalcAlign(offset, maxalign);//�ṹ���С
	s->r = maxalign; //�ṹ�����
}

/***********************************************************
* ����:				�����ṹ����
* maxalign(����,���):	��Ա����������
* offset(����,���):	ƫ����
* ps(���):			�ṹ�������
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
* ����:				����������
* type:				��������
* v(���):				���ʱ��
* force_align(���):	ǿ�ƶ�������
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
* ����:	������������Լ��
* fc(���):����Լ��
*
* <FunctionCallingConvention>::=<KW_CDECL>|<KW_STDCALL>
* ���ں��������ϣ��������������Ϻ��Ե�
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
* ����:				�����ṹ��Ա����
* force_align(���):	ǿ�ƶ�������
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
			Expect("��������");
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
* ����:			����ֱ��������
* type(����,���):	��������
* v(���):			���ʱ��
* func_call:		��������Լ��
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
		Expect("��ʶ��");
	}
	DirectDeclaratorPostfix(type, func_call);
}

/***********************************************************
* ����:			ֱ����������׺
* type(����,���):	��������
* func_call:		��������Լ��
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
* ����:			�����β����ͱ�
* type(����,���):	��������
* func_call:		��������Լ��
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
			Error("��Ч���ͱ�ʶ��");
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
	if (token == TK_BEGIN)			// ��������
		syntax_state = SNTX_LF_HT;
	else							// ��������
		syntax_state = SNTX_NUL;
	SyntaxIndent();

	// �˴��������������ʹ洢��Ȼ��ָ����������type��Ϊ�������ͣ����õ������Ϣ����ref��
	s = SymPush(ViaC_ANOM, type, func_call, 0);
	s->next = first;
	type->t = T_FUNC;
	type->ref = s;
}

/***********************************************************
* ����:	����������
* sym:		��������
*
* <Funcbody>::=<CompoundStatement>
**********************************************************/
void Funcbody(Symbol *sym)
{
	ind = sec_text->data_offset;
	CoffSymAddUpdate(sym, ind, sec_text->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
	/* ��һ���������ھֲ����ű��� */
	SymDirectPush(&LSYM, ViaC_ANOM, &int_type, 0);
	GenProlog(&sym->type);
	rsym = 0;
	CompoundStatement(NULL, NULL);
	BackPatch(rsym, ind);
	GenEpilog();
	sec_text->data_offset = ind;
	SymPop(&LSYM, NULL); /* ��վֲ�����ջ*/
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
* ����:	�����������
* bsym:	break��תλ��
* csym:	continue��תλ��
*
* <CompoundStatement>::=<TK_BEGIN>{<declaration>}{<Statement>}<TK_END>
**********************************************************/
void CompoundStatement(int *bsym, int *csym)
{
	Symbol *s;
	s = (Symbol*)StackGetTop(&LSYM);
	syntax_state = SNTX_LF_HT;
	syntax_level++;						// ������䣬��������һ��

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
* ����:	����if���
* bsym:	break��תλ��
* csym:	continue��תλ��
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
		BackPatch(b, ind); /* ����else��ת */
	}
	else
		BackPatch(a, ind);
}

/***********************************************************
* ����:	����for���
* bsym:	break��תλ��
* csym:	continue��תλ��
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
	Statement(&a, &b);//ֻ�д˴��õ�break,��continue,һ��ѭ���п����ж��break,����continue,����Ҫ�����Ա�����
	GenJmpBackWord(c);
	BackPatch(a, ind);
	BackPatch(b, c);
}

/***********************************************************
* ����:	����continue���
* csym:	continue��תλ��
*
* <ContinueStatement>::=<KW_CONTINUE><TK_SEMICOLON>
**********************************************************/
void ContinueStatement(int *csym)
{
	if (!csym)
		Error("�˴�������continue");
	*csym = GenJmpForWard(*csym);
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
void BreakStatement(int *bsym)
{
	if (!bsym)
		Error("�˴�������break");
	*bsym = GenJmpForWard(*bsym);
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
		Load_1(REG_IRET, optop);
		OperandPop();
	}
	syntax_state = SNTX_LF_HT;
	Skip(TK_SEMICOLON);
	rsym = GenJmpForWard(rsym);
}

/***********************************************************
* ����:	�������ʽ���
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
		OperandPop();
		GetToken();
	}
}


/***********************************************************
* ����:	������ֵ���ʽ
*RelationalExpression
* <AssignmentExpression>::= <EqualityExpression>
*		|<UnaryExpression><TK_ASSIGN> <EqualityExpression>
**********************************************************/
//��������ݹ飬������ȡ������
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
		GenOp(t);
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
* ����:	�����Ӽ�����ʽ
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
		GenOp(t);
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
* ����:	����sizeof���ʽ
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
		Error("sizeof�������ͳߴ�ʧ��");
	OperandPush(&int_type, ViaC_GLOBAL, size);
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
				Expect("�ṹ�����");
			s = optop->type.ref;
			token |= ViaC_MEMBER;
			while ((s = s->next) != NULL)
			{
				if (s->v == token)
					break;
			}
			if (!s)
				Error("û�д˳�Ա����: %s", GetTkstr(token & ~ViaC_MEMBER));
			/* ��Ա������ַ = �ṹ����ָ�� + ��Ա����ƫ�� */
			optop->type = char_pointer_type; /* ��Ա������ƫ����ָ����ڽṹ���׵�ַ���ֽ�ƫ�ƣ���˴˴��任����Ϊ�ֽڱ���ָ�� */
			OperandPush(&int_type, ViaC_GLOBAL, s->c);
			GenOp(TK_PLUS);  //ִ�к�optop->value�����˳�Ա��ַ
							  /* �任����Ϊ��Ա������������ */
			optop->type = s->type;
			/* ����������ܳ䵱��ֵ */
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
* ����:	�������ȱ��ʽ
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
				Expect("��ʶ������");
			s = SymSearch(t);
			if (!s)
			{
				if (token != TK_OPENPA)
					Error("'%s'δ����\n", GetTkstr(t));

				s = FuncSymPush(t, &default_func_type);//��������������ֱ������
				s->r = ViaC_GLOBAL | ViaC_SYM;
			}
			r = s->r;
			OperandPush(&s->type, r, s->c);
			/* �������ã������������¼���ŵ�ַ */
			if (optop->reg & ViaC_SYM)
			{
				optop->sym = s;
				optop->value = 0;  //���ں������ã���ȫ�ֱ������� printf("g_cc=%c\n",g_cc);
			}
			break;
	}
}

/***********************************************************
* ����:	����ʵ�α��ʽ��
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
		Error("ʵ�θ������ں����βθ���"); //��һ���βΣ�ʵ��
	Skip(TK_CLOSEPA);
	GenInvoke(nb_args);

	OperandPush(&ret.type, ret.reg, ret.value);
}

/***********************************************************
* ����:	����n��tab��
* n:		��������
**********************************************************/
void PrintTab(int n)
{
	int i = 0;
	for (; i < n; i++)
		printf("\t");
}

/***********************************************************
* ����:	�﷨����
* ͨ�������GetToken�����ã�
* ������������ȡ���������ж������ʽ������ȡ�õ���ǰ����syntax_state = SNTX_DELAY���ݲ������
* ��ȡ���µ��ʺ󣬸��ݵ�����������syntax_state�����µ��øú�������syntax_state�������ʵ����
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