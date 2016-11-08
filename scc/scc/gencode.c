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

int makelist(int add)  
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

