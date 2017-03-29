/******************************************
*author:Away
*date:2016-11-8
*function:机器码的生成模块单元代码
*******************************************/
#include"viac.h"

int rsym;
int ind = 0;
int loc;
int func_begin_ind;
int func_ret_sub;
Symbol* sym_sec_rdata;
Operand opstack[OPSTACK_SIZE];
Operand* optop;

void GenByte(const char c)
{
	int ind_add = ind + 1;
	if (ind_add > sec_text->data_allocated)
		SectionRealloc(sec_text, ind_add);
	sec_text->data[ind] = c;
	ind = ind_add;
}

void GenPrefix(const char opcode) //生成指令前缀
{
	GenByte(opcode);
}

void GenOpcode_1(const char opcode)//生成单指令
{
	GenByte(opcode);
}

void GenOpcode_2(const char begin, const char end)//生成双字节指令
{
	GenByte(begin);
	GenByte(end);
}

void GenDword(unsigned int c)		//生成4字节的指令
{
	GenByte(c);
	GenByte(c >> 8);
	GenByte(c >> 16);
	GenByte(c >> 24);
}

void BackPatch(int top, const int a) //回填函数,把t为链首的待定跳转地址填入相对地址 a为指令跳转地址
{
	int n;
	int* ptr;
	while (top)
	{
		ptr = (int*)(sec_text->data + top);
		n = *ptr;
		*ptr = a - top - 4;
		top = n;
	}
}

int MakeList(int add)  //记录待定跳转地址的指令链 add前一跳转地址
{
	int ind_add;
	ind_add = ind + 4;
	if (ind > sec_text->data_allocated)
		SectionRealloc(sec_text, ind_add);
	*(int*)(sec_text->data + ind) = add;
	add = ind;
	ind = ind_add;
	return add;
}

void GenAddr32(const int r, Symbol* psym, const int c)//生成全局符号地址，增加coff的重定位
{
	/*psym可以为空*/
	if (r & ViaC_SYM)
	{
		CoffRelocAdd(sec_text, psym, ind, IMAGE_REL_I386_DIR32);
	}
	GenDword(c);
}

void GenModrm(int mod, int reg_opcode, const int r_m, const Symbol* psym, const int c) //生成指令寻址方式字节ModR/M
{
	/********psym可以为空****/
	mod <<= 6;
	reg_opcode <<= 3;
	if (mod == 0xc0) //mod =11 寄存器寻址 89 E5	MOV EBP, EBP
	{
		GenByte(mod | reg_opcode | (r_m & ViaC_VALMASK));
	}
	else if ((r_m & ViaC_VALMASK) == ViaC_GLOBAL)
	{
		GenByte(0x05 | reg_opcode); //直接寻址
		GenAddr32(r_m, psym, c);
	}
	else if ((r_m & ViaC_VALMASK) == ViaC_LOCAL)
	{
		if (c == (char)c)
		{
			GenByte(0x45 | reg_opcode);
			GenByte(c);
		}
		else
		{
			GenByte(0x85 | reg_opcode);
			GenDword(c);
		}
	}
	else
	{
		GenByte(0x00 | reg_opcode | (r_m & ViaC_VALMASK));
	}
}

void Load(const int r, Operand* opd)	//操作数opd加载到寄存器r中
{
	if (opd == NULL)
	{
		Error("指针未初始化");
	}
	int v, ft, fc, fr;

	fr = opd->reg;
	ft = opd->type.t;
	fc = opd->value;

	v = fr & ViaC_VALMASK;
	if (fr & ViaC_LVAL)
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
		if (v == ViaC_GLOBAL)
		{
			GenOpcode_1(0xb8 + r);
			GenAddr32(fr, opd->sym, fc);
		}
		else if (v == ViaC_LOCAL)
		{
			GenOpcode_1(0x8d);
			GenModrm(ADDR_OTHER, r, ViaC_LOCAL, opd->sym, fc);
		}
		else if (v == ViaC_CMP)
		{
			GenOpcode_1(0xb8 + r);
			GenDword(0);

			GenOpcode_2(0x0f, fc + 16);
			GenModrm(ADDR_REG, 0, r, NULL, 0);
		}
		else if (v != r)
		{
			GenOpcode_1(0x89);
			GenModrm(ADDR_REG, v, r, NULL, 0);
		}
	}
}


void Store(const int r, Operand* opd)  //将寄存器的中‘r’的值存入Opd
{
	if (opd == NULL)
	{
		Error("指针未初始化");
	}
	int fr, bt;

	fr = opd->reg & ViaC_VALMASK;
	bt = opd->type.t & T_BTYPE;

	if (bt == T_SHORT)
		GenPrefix(0x66);
	if (bt == T_CHAR)
		GenOpcode_1(0x88);
	else
		GenOpcode_1(0x89);

	if (fr == ViaC_GLOBAL || fr == ViaC_LOCAL || (opd->reg & ViaC_LVAL))
	{
		GenModrm(ADDR_OTHER, r, opd->reg, opd->sym, opd->value);
	}
}

int Load_1(const int rc, Operand* opd) //将栈顶的操作数加载到‘rc’寄存器中
{
	if (opd == NULL)
	{
		Error("指针未初始化");
	}
	int reg;
	reg = opd->reg & ViaC_VALMASK;

	if (reg >= ViaC_GLOBAL || (opd->reg & ViaC_LVAL))
	{
		reg = AllocateReg(rc);
		Load(reg, opd);
	}
	opd->reg = reg;
	return reg;
}

void Load_2(const int lrc, const int rrc) //分别将栈顶的操作数加载带lrc，次栈顶加载到rrc
{	
	Load_1(rrc, optop);
	Load_1(lrc, &optop[-1]);
}

void Store_1(void) //将栈顶的操作数存入次栈顶的操作中
{
	int r, t;
	r = Load_1(REG_ANY, optop);

	if ((optop[-1].reg & ViaC_VALMASK) == ViaC_LLOCAL)
	{
		Operand opd;
		t = AllocateReg(REG_ANY);
		OperandAssign(&opd, T_INT, ViaC_LOCAL | ViaC_LVAL, optop[-1].value);
		Load(t, &opd);
		optop[-1].reg = t | ViaC_LVAL;
	}
	Store(r, optop - 1);
	OperandSwap();
	OperandPop();
}

void GenAddsp(const int val) //调用完函数释放栈 val为栈的空间
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

void GenCall(void)//生成函数调用指令
{
	int r;
	int temp = (optop->reg) & (ViaC_VALMASK | ViaC_LVAL);
	if (temp == ViaC_GLOBAL)
	{
		CoffRelocAdd(sec_text, optop->sym, ind + 1, IMAGE_REL_I386_REL32);
		GenOpcode_1(0xe8);
		GenDword(optop->value - 4);
	}
	else
	{
		r = Load_1(REG_ANY, optop);
		GenOpcode_1(0xff);
		GenOpcode_1(0xd0 + r);
	}
}

void GenInvoke(const int nb_args) //生成函数调用代码，参数入栈，然后调用。nb_args为参数个数
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
		OperandPop();
	}
	SpillRegs();
	func_call = optop->type.ref->r;
	GenCall();
	if (args_size && (func_call != KW_STDCALL))
		GenAddsp(args_size);
	OperandPop();
}

void GenOpi_2(const int opc, const int op) //生成整数二元运算
{
	int r, fr, c;
	int temp = optop->reg & (ViaC_VALMASK | ViaC_LVAL | ViaC_SYM);
	if (temp == ViaC_GLOBAL)
	{
		r = Load_1(REG_ANY, &optop[-1]);
		c = optop->value;
		if (c == (char)c)
		{
			GenOpcode_1(0x83);
			GenModrm(ADDR_REG, opc, r, NULL, 0);
			GenByte(c);
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
	OperandPop();
	if (op >= TK_EQ  && op <= TK_GEQ)
	{
		optop->reg = ViaC_CMP;
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

void GenOpi_1(int op) //生成整数运算
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
			Load_2(REG_EAX, REG_ECX);//EAX被除数 ECX为除数
			r = optop[-1].reg;
			fr = optop[0].reg;
			OperandPop();
			SpillReg(REG_EDX);

			GenOpcode_1(0x99);
			GenOpcode_1(0xf7);
			GenModrm(ADDR_REG, opc, fr, NULL, 0);

			if (op == TK_MOD)
				r = REG_EDX;
			else
				r = REG_EAX;
			optop->reg = r;
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

Type* PointedType(Type* type) //返回T指针所指向的数据类型
{
	if (type != NULL)
		return &type->ref->type;
	else
		return NULL;
}

int PointedSize(Type* type) //返回指针的尺寸
{
	if (type == NULL)
		Error("gencode中有指针未初始化");
	int align;
	Type* ptype = PointedType(type);
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
		else if (bt1 == T_PTR && bt2 == T_PTR)
		{
			if (op != TK_MINUS)
			{
				Error("指针运算仅支持关系和减法运算");
			}
			u = PointedSize(&optop[-1].type);
			GenOpi_1(op);
			optop->type.t = T_INT;
			OperandPush(&int_type, ViaC_GLOBAL, u);
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
			OperandPush(&int_type, ViaC_GLOBAL, restosize);
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
			//关系运算符结果为T_INT类型
			optop->type.t = T_INT;
		}
	}
}

int AllocateReg(const int rc) //寄存器分配
{
	int reg;
	Operand* p = NULL;
	int used;

	for (reg = 0; reg <= REG_EBX; reg++)
	{
		if (rc & REG_ANY || reg == rc)
		{
			used = 0;
			for (p = opstack; p <= optop; ++p)
			{
				if ((p->reg & ViaC_VALMASK) == reg)
					used = 1;
			}
			if (used == 0)			//寄存器空闲
				return reg;
		}
	}

	for (p = opstack; p <= optop; ++p)
	{
		reg = p->reg & ViaC_VALMASK;
		if (reg < ViaC_GLOBAL && (rc & REG_ANY || reg == rc))
		{
			SpillReg(reg);
			return reg;
		}
	}

	return -1;  //此处代码不可达，但是适应平台需要return一个数
}

void SpillReg(int reg) //将‘r’溢出到内存栈中，并且标记释放‘r’寄存器的操作数
{
	int size, align;
	Operand* popd;
	Operand opd;
	Type* ptype;

	for (popd = opstack; popd <= optop; ++popd)
	{
		if ((popd->reg & ViaC_VALMASK) == reg)
		{
			reg = popd->reg & ViaC_VALMASK;
			ptype = &popd->type;
			if (popd->reg & ViaC_LVAL)
				ptype = &int_type;

			size = TypeSize(ptype, &align);
			loc = CalcAlign(loc - size, align);
			OperandAssign(&opd, ptype->t, ViaC_LOCAL | ViaC_LVAL, loc);
			Store(reg, &opd);

			if (popd->reg & ViaC_LVAL)
			{
				popd->reg = (popd->reg &  ~(ViaC_VALMASK)) | ViaC_LLOCAL;
			}
			else
			{
				popd->reg = ViaC_LOCAL | ViaC_LVAL;
			}
			popd->value = loc;
			break;
		}
	}
}

void SpillRegs() //将占用的寄存器全部溢出到栈中
{
	int reg;
	Operand* popd;

	for (popd = opstack; popd <= optop; ++popd)
	{
		reg = popd->reg & ViaC_VALMASK;
		if (reg < ViaC_GLOBAL)
		{
			SpillReg(reg);
		}
	}
}

int GenJmpForWard(const int t)  //生成向高地址跳转的指令
{
	GenOpcode_1(0xe9);
	return MakeList(t);
}

void GenJmpBackWord(const int a) //生成向低地址跳转的指令
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

int GenJcc(int t) //生成条件跳转指令
{
	int v;
	int inv = 1;

	v = optop->reg & ViaC_VALMASK;
	if (v == ViaC_CMP)
	{
		GenOpcode_2(0x0f, optop->value ^ inv);
		t = MakeList(t);
	}
	else
	{
		int ViaC_RES = ViaC_VALMASK | ViaC_LVAL | ViaC_SYM;
		if ((optop->reg & ViaC_RES) == ViaC_GLOBAL)
		{
			t = GenJmpForWard(t);
		}
		else
		{
			v = Load_1(REG_ANY, optop);

			GenOpcode_1(0x85);
			GenModrm(ADDR_REG, v, v, NULL, 0);
			GenOpcode_2(0x0f, 0x85 ^ inv);
			t = MakeList(t);
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
	while ((sym = sym->next) != NULL)
	{
		ptype = &sym->type;
		size = TypeSize(ptype, &align);
		size = CalcAlign(size, 4);

		if ((ptype->t & T_BTYPE) == T_STRUCT)
			size = 4;

		param_addr = addr;
		addr += size;

		SymPush(sym->v & ~ViaC_PARAMS, ptype, ViaC_LOCAL | ViaC_LVAL, param_addr);
	}
	func_ret_sub = 0;
	if (func_call == KW_STDCALL)
		func_ret_sub = addr - 8;
}

void GenEpilog() //生成结尾代码
{
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

	int calc = CalcAlign(-loc, 4);
	int saved_ind = ind;
	ind = func_begin_ind;

	GenOpcode_1(0x50 + REG_EBP);

	GenOpcode_1(0x89);
	GenModrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0);

	GenOpcode_1(0x81);
	int opc = 5;
	GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
	GenDword(calc);
	ind = saved_ind;
}

void InitVariable(Type* ptype, Section* psec, const int c, const int v) //变量初始化
{
	int bt;
	void* ptr = NULL;

	if (psec)
	{
		int ViaC_RES = ViaC_VALMASK | ViaC_LVAL;
		if ((optop->reg & ViaC_RES) != ViaC_GLOBAL)
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
				if (optop->reg & ViaC_SYM)
				{
					CoffRelocAdd(psec, optop->sym, c, IMAGE_REL_I386_DIR32);
				}
				*(int*)ptr = optop->value;
				break;
			}
		}
		OperandPop();
	}
	else
	{
		if (ptype->t & T_ARRAY)
		{
			OperandPush(ptype, ViaC_LOCAL | ViaC_LVAL, c);
			OperandSwap();
			SpillReg(REG_ECX);

			GenOpcode_1(0xB8 + REG_ECX);
			GenDword(optop->type.ref->c);
			GenOpcode_1(0xB8 + REG_ESI);
			GenAddr32(optop->reg, optop->sym, optop->value);
			OperandSwap();

			GenOpcode_1(0x80);
			GenModrm(ADDR_OTHER, REG_EDI, ViaC_LOCAL, optop->sym, optop->value);

			GenPrefix(0xf3);

			GenOpcode_1(0xA4);
			optop -= 2;
		}
		else
		{
			OperandPush(ptype, ViaC_LOCAL | ViaC_LVAL, c);
			OperandSwap();
			Store_1();
			OperandPop();
		}
	}
}
//分配存储空间
Section* AllocateStorage(Type* ptype, const int reg, const int has_init, const int v, int* addr)
{
	if (ptype == NULL || addr == NULL)
	{
		Error("指针未初始化");
	}
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

	if ((reg & ViaC_VALMASK) == ViaC_LOCAL)
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
			(psec->data_offset > psec->data_allocated))
			SectionRealloc(psec, psec->data_offset);
		if (v == 0)
		{
			OperandPush(ptype, ViaC_GLOBAL | ViaC_SYM, *addr);
			optop->sym = sym_sec_rdata;
		}
	}
	return psec;
}

