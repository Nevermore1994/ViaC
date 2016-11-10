#include"scc.h"

int rsym;
int ind = 0;
int loc; 
int func_begin_ind; 
int func_ret_sub;
Symbol* sym_sec_rdata; 
Operand opstack[OPSTACK_SIZE];
Operand* optop;

void GenByte(char c)  //向代码节中写入一个字节
{
	int ind1; 
	ind1 = ind + 1;
	if (ind1 > sec_text->data_allocated)
		SectionRealloc(sec_text, ind1);
	sec_text->data[ind] = 0;
	ind = ind1;
}

void GenPrefix(char opcode)
{
	GenByte(opcode);
}

void GenOpcode_1(char opcode)
{
	GenByte(opcode);
}

void GenOpcode_2(char first, char end)
{
	GenByte(first);
	GenByte(end);
}

void GenDword(unsigned int c)
{
	GenByte(c);
	GenByte(c >> 8);
	GenByte(c >> 16);
	GenByte(c >> 24);
}

void BackPatch(int t, int a)   //回填函数,把t为链首的待定跳转地址填入相对地址
{
	int n;
	int* ptr;
	while (t)
	{
		ptr = (int*)(sec_text->data + t);
		n = *ptr;
		*ptr = a - t - 4;
		t = n;
	}
}

int Makelist(int add)  
{
	int ind1; 
	ind1 = ind + 4;
	if (ind > sec_text->data_allocated)
		SectionRealloc(sec_text, ind1);
	*(int*)(sec_text->data + ind) = add;
	add = ind; 
	ind = ind1; 
	return add;
}
  
void GenAddr32(const int r, const Symbol* sym, const int c)
{
	if (r & SC_SYM)
	{
		CoffelocAdd(sec_text, sym, ind, IMAGE_REL_I386_DIR32);
	}
	GenDword(c);
}

void GenModrm(int mod, int reg_opcode, const int r_m, const Symbol* sym, const int c)
{
	mod <<= 6;
	reg_opcode <<= 3;
	if (mod == 0xc0)
	{
		GenByte(mod | reg_opcode | (r_m & SC_VALMASK));
	}
	else if ((r_m | SC_VALMASK) == SC_GLOBAL)
	{
		GenByte(0x05 | reg_opcode);
		GenAddr32(r_m, sym, c);
	}
	else if((r_m & SC_VALMASK) == SC_LOCAL)
	{ 
		if (c == (char)c)
		{
			GenByte(0x45 | reg_opcode);
			GenByte(c);
		}
		else
		{
			GenByte(0x85 | reg_opcode);
			GenByte(c);
		}
	}
	else
	{
		GenByte(0x00 | reg_opcode | (r_m & SC_VALMASK));
	}
}

void Load(const int r, Operand* opd)
{
	int v, ft, fc, fr;
	
	fr = opd->reg;
	ft = opd->type.t;
	fc = opd->value;

	v = fr & SC_VALMASK;
	if (fr & SC_LVAL)
	{
		if ((ft & T_BTYPE) == T_CHAR)
		{
			GenOpcode_2(0x0f, 0xbe);
		}
		else if ((ft & T_BTYPE) == T_SHORT)
		{
			GenOpcode_2(0x0f, 0xbf);
		}
		else
		{
			GenOpcode_1(0x8b);
		}
		GenModrm(ADDR_OTHER, r, fr, opd->sym, fc);
	}
	else
	{
		if (v == SC_GLOBAL)
		{
			GenOpcode_1(0xb8 + r);
			GenAddr32(fr, opd->sym, fc);
		}
		else if (v == SC_LOCAL)
		{
			GenOpcode_1(0x8d);
			GenOpcode_2(ADDR_OTHER, r, SC_LOCAL, opd->sym, fc);
		}
		else if (v == SC_CMP)
		{
			GenOpcode_1(0xb8 + r);
			GenDword(0);
			
			GenOpcode_2(0x0f, fc+16);
			GenModrm(ADDR_REG, 0, r, NULL, 0);
		}
		else if (v != r)
		{
			GenOpcode_1(0x89);
			GenOpcode_2(ADDR_REG, v, r, NULL, 0); 
		}
	}
}


void Store(const int r, Operand* opd)
{
	int fr, bt;
	
	fr = opd->reg & SC_VALMASK;
	bt = opd->type.t & T_BTYPE;
	
	if (bt == T_SHORT)
		GenPrefix(0x66);
	if (bt == T_CHAR)
		GenOpcode_1(0x88);
	else
		GenOpcode_1(0x89);
	
	if (fr == SC_GLOBAL || fr == SC_LOCAL || (opd->reg & SC_LVAL))
	{
		GenModrm(ADDR_OTHER, r, opd->reg, opd->sym, opd->value);
	}
}

int Load_1(const int rc, Operand* opd)
{
	int reg;
	reg = opd->reg & SC_VALMASK;
	
	if (reg >= SC_GLOBAL || (opd->reg & SC_LVAL))
	{
		reg = AllocateReg(rc);
		Load(reg, opd);
	}
	opd->reg = reg;
	return reg;
}

void Load_2(const int lrc, const int rrc)
{
	Load_1(lrc, &optop[-1]);
	Load_1(rrc, optop);
}

void Store_1(void)
{
	int r, t;
	r = Load_1(REG_ANY, optop);
	
	if ((optop[-1].reg & SC_VALMASK) == SC_LLOACL)
	{
		Operand opd;
		t = AllocateReg(REG_ANY);
		OperandAssgin(&opd, T_INT, SC_LOCAL | SC_LVAL, optop[-1].value);
		Load(t, &opd);
		optop[-1].reg = t | SC_LVAL; 
	}
	Store(r, optop - 1);
	OperandSwap();
	OperandPop();
}

void GenAddsp(const int val)
{
	int opc = 0;
	if (val == (char)val)
	{
		GenOpcode_1(0x83);
		GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
		GenByte(val);
	}
	else
	{
		GenOpcode_1(81);
		GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
		GenDword(val);
	}
}

void GenCall()
{
	int r;
	int temp = (optop->reg) & (SC_VALMASK | SC_LVAL);
	if ( temp == SC_GLOBAL)
	{
		CoffelocAdd(sec_text, optop->sym, ind+1, IMAGE_REL_I386_REL32);
		GenOpcode_1(0xe8);
		GenDword(optop->value -4);
	}
	else
	{
		r = Load_1(REG_ANY, optop);
		GenOpcode_1(0xff);
		GenOpcode_1(0xd0 + r);
	}
}

void GenInvoke(const int nb_args)
{
	int size, r, args_size, func_call;
	
	args_size = 0;
	int i;
	for (i = 0; i < nb_args; ++i)
	{
		r = Load_1(REG_ANY, optop);
		size = 4;
		GenOpcode_1(0x50 + r);
		args_size += size;
		OpernandPop();
	}
	SpillRegs();
	func_call = optop->type.ref->r;
	GenCall();
	if (args_size && (func_call != KW_STDCALL))
	{
		GenAddsp(args_size);
	}
	OpernandPop();
}

void GenOpi_2(const int opc, const int op)
{
	int r, fr, c;
	int temp = optop->reg & (SC_VALMASK | SC_LVAL | SC_SYM);
	if (temp == SC_GLOBAL)
	{
		r = Load_1(REG_ANY, &optop[-1]);
		c = optop->value; 
		if (c == (char)c)
		{
			GenOpcode_1(0x83);
			GenModrm(ADDR_REG, opc, r, NULL, 0);
		}
		else
		{
			GenOpcode_1(0x81);
			GenModrm(ADDR_REG, opc, r, NULL, 0);
			GenByte(c);
		}
	}
	else
	{
		Load_2(REG_ANY, REG_ANY);
		r = optop[-1].reg; 
		fr = optop[0].reg;

		GenOpcode_1((opc << 3) | 0x01);
		GenModrm(ADDR_REG, fr, r, NULL, 0);
	}
	OpernandPop(); 
	if (op >= TK_EQ  && op <= TK_LEQ)
	{
		optop->reg = SC_CMP; 
		switch (op)
		{
			case TK_EQ:
			{
				optop->value = 0x84;
				break;
			}
			case TK_NEQ:
			{
				optop->value = 0x85;
				break;
			}
			case TK_LT:
			{
				optop->value = 0x8c;
				break;
			}
			case TK_LEQ:
			{
				optop->value = 0x8e;
				break;
			}
			case TK_GT:
			{
				optop->value = 0x8f;
				break;
			}
			case TK_GEQ:
			{
				optop->value = 0x8d;
				break;
			}
		}
	}
}

void GenOpi_1(int op)
{
	int r, fr, opc;
	 
	switch (op)
	{
		case TK_PLUS:
		{
			opc = 0;
			GenOpi_2(opc, op);
			break;
		}
		case TK_MINUS:
		{
			opc = 5;
			GenOpi_2(opc, op);
			break;
		}
		case TK_STAR:
		{
			Load_2(REG_ANY, REG_ANY);
			r = optop[-1].reg;
			fr = optop[0].reg;
			OperandPop();

			GenOpcode_2(0x0f, 0xaf);
			GenModrm(ADDR_REG, r, fr, NULL, 0);
			break;
		}
		case TK_DIVIDE:
		case TK_MOD:
		{
			opc = 7; 
			Load_2(REG_EAX, REG_ECX);
			r = optop[-1].reg;
			fr = optop[0].reg; 
			OpernandPop(); 
			SpillReg(REG_EDX);
			
			GenOpcode_1(0x99); 
			GenOpcode_1(0xf7); 
			GenModrm(ADDR_REG, opc, fr, NULL, 0);
			
			if (op == TK_AND)
				r = REG_EDX;
			else
				r = REG_EAX;
			break;
		}
		default:
		{
			opc = 7;
			GenOpi_2(opc, op);
			break;
		}
	}
}

Type* PointedType(Type* t)
{
	return &t->ref->type;
}

int PointedSize(Type* t)
{
	int align; 
	Type* ptype = PointedType(t);
	return TypeSize(ptype, &align);
}
 
void GenOp(const int op)
{
	int u, bt1, bt2;
	Type type;
	
	bt1 = optop[-1].type.t & T_BTYPE;
	bt2 = optop[0].type.t  & T_BTYPE;
	
	if (bt1 == T_PTR || bt2 == T_PTR)
	{
		if (op >= TK_EQ && op <= TK_GEQ)
		{
			GenOpi_1(op);
			optop->type.t = T_INT;
		}
		else if(bt1 == T_PTR && bt2 == T_PTR)
		{
			if (op != TK_MINUS)
			{
				Error("指针运算仅支持关系和减法运算");
			}
			u = PointedSize(&optop[-1].type);
			GenOpi_1(op); 
			optop->type.t = T_INT;
			OperandPush(&int_type, SC_GLOBAL, u);
			GenOp(TK_DIVIDE);
		}
		else
		{
			if (op != TK_MINUS && op != TK_PLUS)
				Error("指针不能与非指针进行非关系运算");
			if (bt2 == T_PTR)
				OperandSwap();
			type = optop[-1].type;
			int restosize = PointedSize(&optop[-1].type);
			OperandPush(&int_type, SC_GLOBAL, restosize);
			GenOp(TK_STAR);
			
			GenOpi_1(op);
			optop->type = type;
		}
	}
	else
	{
		GenOpi_1(op);
		if (op >= TK_EQ && op <= TK_GEQ)
		{
			optop->type.t = T_INT;  
		}
	}
}

int AllocateReg(const int rc)
{
	int reg;
	Operand* p;
	int used;

	for (reg = 0; reg <= REG_ANY; reg++)
	{
		if (rc & REG_ANY || reg == rc)
		{
			used = 0;
			for (p = opstack; p <= optop; ++p)
			{
				if ((p->reg & SC_VALMASK) == reg)
					used = 1;
			}
			if (used == 0)			//寄存器空闲
				return reg;
		}
	}
	
for (p = opstack; p <= optop; ++p)
{
	reg = p->reg & SC_VALMASK;
	if (reg < SC_GLOBAL && (rc & REG_ANY || reg == rc))
	{
		SpillReg(reg);
		return -1;
	}
}

return -1;  //此处代码不可达，但是适应平台需要return一个数
}

void SpillReg(int reg)
{
	int size, align;
	Operand* popd;
	Operand opd;
	Type* ptype;

	for (popd = opstack; popd <= optop; ++popd)
	{
		if ((popd->reg & SC_VALMASK) == reg)
		{
			reg = popd->reg & SC_VALMASK;
			ptype = &popd->type;
			if (popd->reg & SC_LVAL)
				ptype = &int_type;
			size = TypeSize(ptype, &align);
			loc = CalcAlign(loc - size, align);
			OperandAssgin(&opd, ptype->t, SC_LOCAL | SC_LVAL, loc);
			Store(reg, &opd);
			if (popd->reg & SC_LVAL)
			{
				popd->reg = (popd->reg &  ~(SC_VALMASK)) | SC_LLOACL;

			}
			else
			{
				popd->reg = SC_LOCAL | SC_LVAL;
			}
			popd->value = loc;
			break;
		}
	}
}

void SpillRegs()
{
	int reg;
	Operand* popd;

	for (popd = opstack; popd <= optop; ++popd)
	{
		reg = popd->reg & SC_VALMASK;
		if (reg < SC_GLOBAL)
		{
			SpillReg(reg);
		}
	}
}


int GenJmpForWard(const int t)
{
	GenOpcode_1(0xe9);
	return makelist(t);
}

void GenJmpBcakWord(const int a)
{
	int reg;
	reg = a - ind - 2;
	if (reg == (char)reg)
	{
		GenOpcode_1(0xeb);
		GenByte(reg);
	}
	else
	{
		GenOpcode_1(0xe9);
		GenDword(a - ind - 4);
	}
}

int GenJcc(int t)
{
	int v;
	int inv = 1;

	v = optop->reg & SC_VALMASK;
	if (v == SC_CMP)
	{
		GenOpcode_2(0x0f, optop->value ^ inv);
		t = Makelist(t);
	}
	else
	{
		int SC_RES = SC_VALMASK | SC_LVAL | SC_SYM;
		if ( optop->reg & SC_RES == SC_GLOBAL)
		{
			t = GenJmpForWard(t);
		}
		else
		{
			v = Load_1(REG_ANY, optop);
			
			GenOpcode_1(0x85);
			GenModrm(ADDR_REG, v, v, NULL, 0);
			GenOpcode_2(0x0f, 0x85 ^inv);
			t = Makelist(t);
		}
	}
	OperandPop();
	return t;
}

void GenProlog(Type* func_type)
{
	if (func_type == NULL)
	{
		Error("指针未初始化");
	}
	int addr, align, size, func_call;
	int param_addr;
	Symbol* sym = NULL; 
	Type* ptype = NULL;
	
	sym = func_type->ref;
	func_call = sym->r;
	addr = 8;
	loc = 0;
	 
	func_begin_ind = ind; //函数体的开始
	ind += FUNC_PROLOG_SIZE;
	if (sym->type.t == T_STRUCT)
		Error("仅支持返回结构体指针");
	while ((sym == sym->next) != NULL)
	{
		ptype = &sym->type;
		size = TypeSize(ptype, &align);
		size = CalcAlign(size, 4);
		
		if ((ptype->t & T_BTYPE) == T_STRUCT)
			size = 4;

		param_addr = addr; 
		addr += size;
		
		SymPush(sym->v & ~SC_PARAMS , ptype, SC_LOCAL | SC_LVAL, param_addr);
	}
	func_ret_sub = 0;
	if (func_call == KW_STDCALL)
		func_ret_sub = addr - 8;
}
 
void GenEpilog()
{
	int v, saved_ind, opc;
	 
	GenOpcode_1(0x8b);
	GenModrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0);
	
	GenOpcode_1(0x58 + REG_EBP);
	
	if (func_ret_sub == 0)
	{
		GenOpcode_1(0xc3);
	}
	else
	{
		GenOpcode_1(0xc2);
		GenByte(func_ret_sub);
		GenByte(func_ret_sub >> 8);
	}
	
	v = CalcAlign(-loc, 4);
	saved_ind = ind; 
	ind = func_begin_ind;

	GenOpcode_1(0x50 + REG_EBP);
	
	GenOpcode_1(0x89);
	GenModrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0);
	
	GenOpcode_1(0x81);
	opc = 5;
	GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
	GenDword(v);
	ind = saved_ind;
}
 
void InitVariable(const Type* ptype, const Section* psec, const int c, const int v)
{
	int bt;
	void* ptr;
	
	if (psec)
	{ 
		int SC_RES = SC_VALMASK | SC_LVAL;
		if ((optop->reg & SC_RES) != SC_GLOBAL)
			Error("全局变量必须常量初始化");
		bt = ptype->t & T_BTYPE;
		ptr = psec->data + c;
		
		switch (bt)
		{
			case T_CHAR:
			{
				*(char*)ptr = optop->value;
				break;
			}
			case T_STRUCT:
			{
				*(short*)ptr = optop->value;
				break;
			}
			default:
			{
				if (optop->reg & SC_SYM)
				{
					CoffelocAdd(psec, optop->sym, c, IMAGE_REL_I386_REL32);
				}
				*(int*)ptr = optop->value;
				break;
			}
		}
	}
	else
	{
		if (ptype->t & T_ARRAY)
		{
			OperandPush(ptype, SC_LOCAL | SC_LVAL, c);
			OperandSwap();
			SpillReg(REG_ECX);

			GenOpcode_1(0xB8 + REG_ECX);
			GenDword(optop->type.ref->c);
			GenOpcode_1(0xB8 + REG_ESI);
			GenAddr32(optop->reg, optop->sym, optop->value);
			OpernandPop();

			GenOpcode_1(0x80);
			GenModrm(ADDR_OTHER, REG_EDI, SC_LOCAL, optop->sym, optop->value);

			GenPrefix(0xf3);

			GenOpcode_1(0xA4);
			optop -= 2;
		}
		else
		{
			OperanPush(ptype, SC_LOCAL | SC_LVAL, c);
			OperandSwap();
			Store_1();
			OperandPop();
		}
	}
}

Section* AllocateStorage(Type* ptype, const int reg, const int has_init, const int v, int* addr)
{
	int size, align; 
	Section* psec = NULL;
	size = TypeSize(ptype, &align);
	
	if (size < 0)
	{
		if ((ptype->t & T_ARRAY) && (ptype->ref->type.t == T_CHAR))
		{
			ptype->ref->c = strlen((char*)tkstr.data) + 1;
			size = TypeSize(ptype, &align);
		}
		else
		{
			Error("类型尺寸未知");
		}
	}
	
	if ((reg & SC_VALMASK) == SC_LOCAL)
	{
		loc = CalcAlign(loc - size, align);
		*addr = loc;
	}
	else
	{
		if (has_init == 1)
			psec = sec_data;
		else if (has_init == 2)
			psec = sec_rdata;
		else
			psec = sec_bss;

		psec->data_offset = CalcAlign(psec->data_offset, align);
		*addr = psec->data_offset;
		psec->data_offset += size;
		 
		if (psec->sh.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA &&
			psec->data_offset > psec->data_allocated)
			SectionRealloc(psec, psec->data_offset);
		if (v == 0)
		{
			OperandPush(ptype, SC_GLOBAL & SC_SYM, *addr);
			optop->sym = sym_sec_rdata;
		}
	}
	return psec;
 }