// 《自己动手写编译器、链接器》配套源代码

#include "viac.h"
/* 本章用到的全局变量 */
int rsym;						// 记录return指令位置
int ind = 0;					// 指令在代码节位置
int loc;						// 局部变量在栈中位置
int func_begin_ind;				// 函数开始指令
int func_ret_sub;				// 函数返回释放栈空间大小
Symbol *sym_sec_rdata;			// 只读节符号
Operand opstack[OPSTACK_SIZE];  // 操作数栈
Operand *optop;					// 操作数栈栈顶

void GenByte(char c)
{
	int ind1;
	ind1 = ind + 1;
	if (ind1 > sec_text->data_allocated)
		SectionRealloc(sec_text, ind1);
	sec_text->data[ind] = c;
	ind = ind1;
}

/***********************************************************
instruction prefix
The Instruction prefixes are divided into four groups, each with a set of allowable prefix codes:
1.Lock and repeat prefixes
2.Segment override
3.Operand-size override
4.Address-size override
**********************************************************/
/***********************************************************
* 功能:	生成指令前缀
* opcode:	指令前缀编码
**********************************************************/
void GenPrefix(char opcode)
{
	GenByte(opcode);
}

/***********************************************************
* 功能:	生成单字节指令
* opcode:  指令编码
**********************************************************/
void GenOpcode_1(char opcode)
{
	GenByte(opcode);
}

/***********************************************************
* 功能:	生成双字节指令
* first:	指令第一个字节
* second:	指令第二个字节
**********************************************************/
void GenOpcode_2(char first, char second)
{
	GenByte(first);
	GenByte(second);
}

/***********************************************************
* 功能:	生成4字节操作数
* c:		4字节操作数
**********************************************************/
void GenDword(unsigned int c)
{
	GenByte(c);
	GenByte(c >> 8);
	GenByte(c >> 16);
	GenByte(c >> 24);
}

/***********************************************************
* 功能:	回填函数，把t为链首的各个待定跳转地址填入相对地址
* t:		链首
* a:		指令跳转位置
**********************************************************/
void BackPatch(int t, int a)
{
	int n, *ptr;
	while (t)
	{
		ptr = (int *)(sec_text->data + t);
		n = *ptr; // 下一个需要回填位置
		*ptr = a - t - 4;
		t = n;
	}
}

/***********************************************************
* 功能:	记录待定跳转地址的指令链
* s:		前一跳转指令地址
**********************************************************/
int MakeList(int s)
{
	int ind1;
	ind1 = ind + 4;
	if (ind1 > sec_text->data_allocated)
		SectionRealloc(sec_text, ind1);
	*(int *)(sec_text->data + ind) = s;
	s = ind;
	ind = ind1;
	return s;
}

/***********************************************************
* 功能:	生成全局符号地址,并增加COFF重定位记录
* output constant with relocation if 'r & ViaC_SYM' is true
* r:		符号存储类型
* sym:		符号指针
* c:		符号关联值
**********************************************************/
void GenAddr32(int r, Symbol *sym, int c)
{
	if (r & ViaC_SYM)
		CoffRelocAdd(sec_text, sym, ind, IMAGE_REL_I386_DIR32);
	GenDword(c);
}

/***********************************************************
* 功能:		生成指令寻址方式字节ModR/M,
* mod:			ModR/M [7:6]
* reg_opcode:	ModR/M [5:3]指令的另外3位操作码 源操作数(叫法不准确)
* r_m:			ModR/M [2:0] 目标操作数(叫法不准确）
* sym:		符号指针
* c:		符号关联值
**********************************************************/
void GenModrm(int mod, int reg_opcode, int r_m, Symbol *sym, int c)
{
	mod <<= 6;
	reg_opcode <<= 3;
	if (mod == 0xc0) // mod=11 寄存器寻址 89 E5(11 reg_opcode=100ESP r=101EBP) MOV EBP,ESP
	{
		GenByte(mod | reg_opcode | (r_m & ViaC_VALMASK));
	}
	else if ((r_m & ViaC_VALMASK) == ViaC_GLOBAL)
	{
		// mod=00 r=101 disp32  8b 05 50 30 40 00  MOV EAX,DWORD PTR DS:[403050]
		// 8B /r MOV r32,r/m32 Move r/m32 to r32
		// mod=00 r=101 disp32  89 05 14 30 40 00  MOV DWORD PTR DS:[403014],EAX
		// 89 /r MOV r/m32,r32 Move r32 to r/m32
		GenByte(0x05 | reg_opcode); //直接寻址
		GenAddr32(r_m, sym, c);
	}
	else if ((r_m & ViaC_VALMASK) == ViaC_LOCAL)
	{
		if (c == (char)c)
		{
			// mod=01 r=101 disp8[EBP] 89 45 fc MOV DWORD PTR SS:[EBP-4],EAX
			GenByte(0x45 | reg_opcode);
			GenByte(c);
		}
		else
		{
			// mod=10 r=101 disp32[EBP]
			GenByte(0x85 | reg_opcode);
			GenDword(c);
		}
	}
	else
	{
		// mod=00 89 01(00 reg_opcode=000 EAX r=001ECX) MOV DWORD PTR DS:[ECX],EAX
		GenByte(0x00 | reg_opcode | (r_m & ViaC_VALMASK));
	}
}


/***********************************************************
* 功能:	将操作数opd加载到寄存器r中
* r:		符号存储类型
* opd:		操作数指针
**********************************************************/
void Load(int r, Operand *opd)
{
	int v, ft, fc, fr;

	fr = opd->reg;
	ft = opd->type.t;
	fc = opd->value;

	v = fr & ViaC_VALMASK;
	if (fr & ViaC_LVAL)
	{
		if ((ft & T_BTYPE) == T_CHAR)
		{
			// movsx -- move with sign-extention	
			// 0F BE /r	movsx r32,r/m8	move byte to doubleword,sign-extention
			GenOpcode_2(0x0f, 0xbe);
		}
		else if ((ft & T_BTYPE) == T_SHORT)
		{
			// movsx -- move with sign-extention	
			// 0F BF /r	movsx r32,r/m16	move word to doubleword,sign-extention
			GenOpcode_2(0x0f, 0xbf);
		}
		else
		{
			// 8B /r	mov r32,r/m32	mov r/m32 to r32
			GenOpcode_1(0x8b);
		}
		GenModrm(ADDR_OTHER, r, fr, opd->sym, fc);
	}
	else
	{
		if (v == ViaC_GLOBAL)
		{
			// B8+ rd	mov r32,imm32		mov imm32 to r32
			GenOpcode_1(0xb8 + r);
			GenAddr32(fr, opd->sym, fc);
		}
		else if (v == ViaC_LOCAL)
		{
			// LEA--Load Effective Address
			// 8D /r	LEA r32,m	Store effective address for m in register r32
			GenOpcode_1(0x8d);
			GenModrm(ADDR_OTHER, r, ViaC_LOCAL, opd->sym, fc);
		}
		else if (v == ViaC_CMP) // 适用于c=a>b情况
		{
			/*适用情况生成的样例代码
			00401384   39C8             CMP EAX,ECX
			00401386   B8 00000000      MOV EAX,0
			0040138B   0F9FC0           SETG AL
			0040138E   8945 FC          MOV DWORD PTR SS:[EBP-4],EAX*/

			/*B8+ rd	mov r32,imm32		mov imm32 to r32*/
			GenOpcode_1(0xb8 + r); /* mov r, 0*/
			GenDword(0);

			// SETcc--Set Byte on Condition
			// OF 9F			SETG r/m8	Set byte if greater(ZF=0 and SF=OF)
			// 0F 8F cw/cd		JG rel16/32	jump near if greater(ZF=0 and SF=OF)
			GenOpcode_2(0x0f, fc + 16);
			GenModrm(ADDR_REG, 0, r, NULL, 0);
		}
		else if (v != r)
		{
			// 89 /r	MOV r/m32,r32	Move r32 to r/m32
			GenOpcode_1(0x89);
			GenModrm(ADDR_REG, v, r, NULL, 0);
		}
	}
}

/***********************************************************
* 功能:	将寄存器'r'中的值存入操作数'opd'
* r:		符号存储类型
* opd:		操作数指针
**********************************************************/
void Store(int r, Operand *opd)
{
	int fr, bt;

	fr = opd->reg & ViaC_VALMASK;
	bt = opd->type.t & T_BTYPE;
	if (bt == T_SHORT)
		GenPrefix(0x66); //Operand-size override, 66H
	if (bt == T_CHAR)
		// 88 /r	MOV r/m,r8	Move r8 to r/m8
		GenOpcode_1(0x88);
	else
		// 89 /r	MOV r/m32,r32	Move r32 to r/m32
		GenOpcode_1(0x89);

	if (fr == ViaC_GLOBAL ||
		fr == ViaC_LOCAL ||
		(opd->reg & ViaC_LVAL))
	{
		GenModrm(ADDR_OTHER, r, opd->reg, opd->sym, opd->value);
	}
}

/***********************************************************
* 功能:	将栈顶操作数加载到'rc'类寄存器中
* rc:		寄存器类型
* opd:		操作数指针
**********************************************************/
int Load_1(int rc, Operand *opd)
{
	int r;
	r = opd->reg & ViaC_VALMASK;
	// 需要加载到寄存器中情况：
	// 1.栈顶操作数目前尝未分配寄存器
	// 2.栈顶操作数已分配寄存器，但为左值 *p
	if (r >= ViaC_GLOBAL ||
		(opd->reg & ViaC_LVAL)
		)
	{
		r = AllocateReg(rc);
		Load(r, opd);
	}
	opd->reg = r;
	return r;
}

/***********************************************************
* 功能:	将栈顶操作数加载到'rc1'类寄存器，将次栈顶操作数加载到'rc2'类寄存器
* rc1:		栈顶操作数加载到的寄存器类型
* rc2:		次栈顶操作数加载到的寄存器类型
**********************************************************/
void Load_2(int rc1, int rc2)
{
	Load_1(rc2, optop);
	Load_1(rc1, &optop[-1]);
}

/***********************************************************
* 功能:	将栈顶操作数存入次栈顶操作数中
**********************************************************/
void Store_1()
{
	int r, t;
	r = Load_1(REG_ANY, optop);
	// 左值如果被溢出到栈中，必须加载到寄存器中
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

/***********************************************************
* 功能:	调用完函数后恢复/释放栈,对于__cdecl
* val:		需要释放栈空间大小(以字节计)
**********************************************************/
void gen_addsp(int val)
{
	int opc = 0;
	if (val == (char)val)
	{
		// ADD--Add	83 /0 ib	ADD r/m32,imm8	Add sign-extended imm8 from r/m32
		GenOpcode_1(0x83);	// ADD esp,val
		GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
		GenByte(val);
	}
	else
	{
		// ADD--Add	81 /0 id	ADD r/m32,imm32	Add sign-extended imm32 to r/m32
		GenOpcode_1(81);	// add esp, val
		GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
		GenDword(val);
	}
}

/***********************************************************
* 功能:	生成函数调用指令
**********************************************************/
void GenCall()
{
	int r;
	if ((optop->reg & (ViaC_VALMASK | ViaC_LVAL)) == ViaC_GLOBAL)
	{
		// 记录重定位信息
		CoffRelocAdd(sec_text, optop->sym, ind + 1, IMAGE_REL_I386_REL32);

		//	CALL--Call Procedure E8 cd   
		//	CALL rel32    call near,relative,displacement relative to next instrution
		GenOpcode_1(0xe8); /* call im */
		GenDword(optop->value - 4);
	}
	else
	{
		// FF /2 CALL r/m32 Call near, absolute indirect, address given in r/m32
		r = Load_1(REG_ANY, optop);
		GenOpcode_1(0xff);	// call/jmp *r
		GenOpcode_1(0xd0 + r); //d0 = 11 010 000
	}
}

/***********************************************************
* 功能:	生成函数调用代码,先将参数入栈，然后call
* nb_args: 参数个数
**********************************************************/
void GenInvoke(int nb_args)
{
	int size, r, args_size, i, func_call;

	args_size = 0;
	// 参数依次入栈
	for (i = 0; i < nb_args; i++)
	{
		r = Load_1(REG_ANY, optop);
		size = 4;
		// PUSH--Push Word or Doubleword Onto the Stack
		// 50+rd	PUSH r32	Push r32
		GenOpcode_1(0x50 + r);	// push r
		args_size += size;
		OperandPop();
	}
	SpillRegs();
	func_call = optop->type.ref->r;  // 得到调用约定方式
	GenCall();
	if (args_size && func_call != KW_STDCALL)
		gen_addsp(args_size);
	OperandPop();
}

/***********************************************************
* 功能:	生成整数二元运算
* opc:		ModR/M [5:3]
* op:		运算符类型
**********************************************************/
void GenOpi_1_2(int opc, int op)
{
	int r, fr, c;
	if ((optop->reg & (ViaC_VALMASK | ViaC_LVAL | ViaC_SYM)) == ViaC_GLOBAL)
	{
		r = Load_1(REG_ANY, &optop[-1]);
		c = optop->value;
		if (c == (char)c)
		{
			// ADC--Add with Carry			83 /2 ib	ADC r/m32,imm8	Add with CF sign-extended imm8 to r/m32
			// ADD--Add						83 /0 ib	ADD r/m32,imm8	Add sign-extended imm8 from r/m32
			// SUB--Subtract				83 /5 ib	SUB r/m32,imm8	Subtract sign-extended imm8 to r/m32
			// CMP--Compare Two Operands	83 /7 ib	CMP r/m32,imm8	Compare imm8 with r/m32
			GenOpcode_1(0x83);
			GenModrm(ADDR_REG, opc, r, NULL, 0);
			GenByte(c);
		}
		else
		{
			// ADD--Add					81 /0 id	ADD r/m32,imm32	Add sign-extended imm32 to r/m32
			// SUB--Subtract				81 /5 id	SUB r/m32,imm32	Subtract sign-extended imm32 from r/m32
			// CMP--Compare Two Operands	81 /7 id	CMP r/m32,imm32	Compare imm32 with r/m32
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

		// ADD--Add						01 /r	ADD r/m32,r32	Add r32 to r/m32
		// SUB--Subtract				29 /r	SUB r/m32,r32	Subtract r32 from r/m32
		// CMP--Compare Two Operands	39 /r	CMP r/m32,r32	Compare r32 with r/m32
		GenOpcode_1((opc << 3) | 0x01);
		GenModrm(ADDR_REG, fr, r, NULL, 0);
	}
	OperandPop();
	if (op >= TK_EQ && op <= TK_GEQ)
	{
		optop->reg = ViaC_CMP;
		switch (op)
		{
			case TK_EQ:
				optop->value = 0x84;
				break;
			case TK_NEQ:
				optop->value = 0x85;
				break;
			case TK_LT:
				optop->value = 0x8c;
				break;
			case TK_LEQ:
				optop->value = 0x8e;
				break;
			case TK_GT:
				optop->value = 0x8f;
				break;
			case TK_GEQ:
				optop->value = 0x8d;
				break;
		}
	}
}


/***********************************************************
* 功能:	生成整数运算
* op:		运算符类型
**********************************************************/
void GenOpi_1(int op)
{
	int r, fr, opc;

	switch (op)
	{
		case TK_PLUS:
			opc = 0;
			GenOpi_1_2(opc, op);
			break;
		case TK_MINUS:
			opc = 5;
			GenOpi_1_2(opc, op);
			break;
		case TK_STAR:
			Load_2(REG_ANY, REG_ANY);
			r = optop[-1].reg;
			fr = optop[0].reg;
			OperandPop();

			// IMUL--Signed Multiply
			// 0F AF /r	IMULr32,r/m32	doubleword register <--doubleword register * r/m doubleword		
			GenOpcode_2(0x0f, 0xaf); /*imul  r, fr */
			GenModrm(ADDR_REG, r, fr, NULL, 0);
			break;
		case TK_DIVIDE:
		case TK_MOD:
			opc = 7;
			Load_2(REG_EAX, REG_ECX);
			r = optop[-1].reg;
			fr = optop[0].reg;
			OperandPop();
			SpillReg(REG_EDX);

			// CWD/CDQ--Convert Word to Doubleword/Convert Doubleword to Qradword
			// 99	CWQ	EDX:EAX<--sign_entended EAX
			GenOpcode_1(0x99);

			// IDIV--Signed Divide
			// F7 /7	IDIV r/m32	Signed divide EDX:EAX(where EDX must contain signed extension of EAX)
			// by r/m doubleword.(Result:EAX=Quotient, EDX=Remainder)
			// EDX:EAX被除数 r/m32除数
			GenOpcode_1(0xf7); /* idiv eax, fr*/
			GenModrm(ADDR_REG, opc, fr, NULL, 0);

			if (op == TK_MOD)
				r = REG_EDX;
			else
				r = REG_EAX;
			optop->reg = r;
			break;
		default:
			opc = 7;
			GenOpi_1_2(opc, op);
			break;
	}
}


/***********************************************************
* 功能:	返回t所指向的数据类型
* t:		指针类型
**********************************************************/
Type *PointedType(Type *t)
{
	return &t->ref->type;
}

/***********************************************************
* 功能:	返回t所指向的数据类型尺寸
* t:		指针类型
**********************************************************/
int PointedSize(Type *t)
{
	int align;
	return TypeSize(PointedType(t), &align);
}

/***********************************************************
* 功能:	生成二元运算,对指针操作数进行一些特殊处理
* op:		运算符类型
**********************************************************/
void GenOp(int op)
{
	int u, bt1, bt2;
	Type type1;

	bt1 = optop[-1].type.t & T_BTYPE;
	bt2 = optop[0].type.t & T_BTYPE;

	if (bt1 == T_PTR || bt2 == T_PTR)
	{
		if (op >= TK_EQ && op <= TK_GEQ) // 关系运算
		{
			GenOpi_1(op);
			optop->type.t = T_INT;
		}
		else if (bt1 == T_PTR && bt2 == T_PTR) //两个操作数都为指针
		{
			if (op != TK_MINUS)
				Error("两个指针只能进行关系或减法运算");
			u = PointedSize(&optop[-1].type);
			GenOpi_1(op);
			optop->type.t = T_INT;
			OperandPush(&int_type, ViaC_GLOBAL, u);
			GenOp(TK_DIVIDE);
		}
		else //两个操作数一个是指针，另一个不是指针，并且非关系运算
		{
			if (op != TK_MINUS && op != TK_PLUS)
				Error("指针只能进行关系或加减运算");
			// 指针作为第一操作数
			if (bt2 == T_PTR)
			{
				OperandSwap();
			}
			type1 = optop[-1].type;
			OperandPush(&int_type, ViaC_GLOBAL, PointedSize(&optop[-1].type));
			GenOp(TK_STAR);

			GenOpi_1(op);
			optop->type = type1;
		}
	}
	else
	{
		GenOpi_1(op);
		if (op >= TK_EQ && op <= TK_GEQ)
		{
			// 关系运算结果为T_INT类型
			optop->type.t = T_INT;
		}
	}
}


/***********************************************************
* 功能:	寄存器分配，如果所需寄存器被占用,先将其内容溢出到栈中
* rc:		寄存器类型
**********************************************************/
int AllocateReg(int rc)
{
	int r;
	Operand *p;
	int used;

	/* 查找空闲的寄存器 */
	for (r = 0; r <= REG_EBX; r++)
	{
		if (rc & REG_ANY || r == rc)
		{
			used = 0;
			for (p = opstack; p <= optop; p++)
			{
				if ((p->reg & ViaC_VALMASK) == r)
					used = 1;
			}
			if (used == 0) return r;
		}
	}

	// 如果没有空闲的寄存器，从操作数栈底开始查找到第一个占用的寄存器举出到栈中
	for (p = opstack; p <= optop; p++)
	{
		r = p->reg & ViaC_VALMASK;
		if (r < ViaC_GLOBAL && (rc & REG_ANY || r == rc))
		{
			SpillReg(r);
			return r;
		}
	}
	/* 此处永远不可能到达 */
	return -1;
}

/***********************************************************
* 功能:	将寄存器'r'溢出到内存栈中,并且标记释放'r'寄存器的操作数为局部变量
* r:		寄存器编码
**********************************************************/
void SpillReg(int r)
{
	int size, align;
	Operand *p, opd;
	Type *type;

	for (p = opstack; p <= optop; p++)
	{
		if ((p->reg & ViaC_VALMASK) == r)
		{
			r = p->reg & ViaC_VALMASK;
			type = &p->type;
			if (p->reg & ViaC_LVAL)
				type = &int_type;
			size = TypeSize(type, &align);
			loc = CalcAlign(loc - size, align);
			OperandAssign(&opd, type->t, ViaC_LOCAL | ViaC_LVAL, loc);
			Store(r, &opd);
			if (p->reg & ViaC_LVAL)
			{
				p->reg = (p->reg & ~(ViaC_VALMASK)) | ViaC_LLOCAL; //标识操作数放在栈中
			}
			else //sum = add(a = add(a,b),b = add(c,d));
			{
				p->reg = ViaC_LOCAL | ViaC_LVAL;
			}
			p->value = loc;
			break;
		}
	}
}

/***********************************************************
* 功能:	将占用的寄存器全部溢出到栈中
**********************************************************/
void SpillRegs()
{
	int r;
	Operand *p;
	for (p = opstack; p <= optop; p++)
	{
		r = p->reg & ViaC_VALMASK;
		if (r < ViaC_GLOBAL)
		{
			SpillReg(r);
		}
	}
}

/***********************************************************
* 功能:	生成向高地址跳转指令，跳转地址待定
* t:		前一跳转指令地址
**********************************************************/
int GenJmpForWard(int t)
{
	// JMP--Jump		
	// E9 cd	JMP rel32	Jump near,relative,displacement relative to next instruction
	GenOpcode_1(0xe9);
	return MakeList(t);;
}

/***********************************************************
* 功能:	生成向低地址跳转指令，跳转地址已确定
* a:		跳转到的目标地址
**********************************************************/
void GenJmpBackWord(int a)
{
	int r;
	r = a - ind - 2;
	if (r == (char)r)
	{
		// EB cb	JMP rel8	Jump short,relative,displacement relative to next instruction
		GenOpcode_1(0xeb);
		GenByte(r);
	}
	else
	{
		// E9 cd	JMP rel32	Jump short,relative,displacement relative to next instruction
		GenOpcode_1(0xe9);
		GenDword(a - ind - 4);
	}
}

/***********************************************************
* 功能:	生成条件跳转指令
* t:		前一跳转指令地址
* 返回值:  新跳转指令地址
**********************************************************/
int GenJcc(int t)
{
	int v;
	int inv = 1;

	v = optop->reg & ViaC_VALMASK;
	if (v == ViaC_CMP)
	{
		// Jcc--Jump if Condition Is Met
		// .....
		// 0F 8F cw/cd		JG rel16/32	jump near if greater(ZF=0 and SF=OF)
		// .....
		GenOpcode_2(0x0f, optop->value ^ inv);
		t = MakeList(t);
	}
	else
	{
		if ((optop->reg & (ViaC_VALMASK | ViaC_LVAL | ViaC_SYM)) == ViaC_GLOBAL)
		{

			t = GenJmpForWard(t);
		}
		else
		{
			v = Load_1(REG_ANY, optop);

			// TEST--Logical Compare
			// 85 /r	TEST r/m32,r32	AND r32 with r/m32,set SF,ZF,PF according to result		
			GenOpcode_1(0x85);
			GenModrm(ADDR_REG, v, v, NULL, 0);

			//  Jcc--Jump if Condition Is Met
			// .....
			// 0F 8F cw/cd		JG rel16/32	jump near if greater(ZF=0 and SF=OF)
			// .....
			GenOpcode_2(0x0f, 0x85 ^ inv);
			t = MakeList(t);
		}
	}
	OperandPop();
	return t;
}

/***********************************************************
* 功能:		生成函数开头代码
* func_type:	函数类型
**********************************************************/
void GenProlog(Type *func_type)
{
	int addr, align, size, func_call;
	int param_addr;
	Symbol *sym;
	Type *type;

	sym = func_type->ref;
	func_call = sym->r;
	addr = 8;
	loc = 0;

	func_begin_ind = ind;  //记录了函数体开始，以便函数体结束时填充函数头，因为那时才能确定开辟的栈大小
	ind += FUNC_PROLOG_SIZE;
	if (sym->type.t == T_STRUCT)
		Error("不支持返回结构体，可以返回结构体指针");
	//  参数定义
	while ((sym = sym->next) != NULL)
	{
		type = &sym->type;
		size = TypeSize(type, &align);
		size = CalcAlign(size, 4); //此句非常重要，因为参数压栈是以4字节对齐的，所以此处每个参数必须4字节对齐
									//  结构体作为指针传递
		if ((type->t & T_BTYPE) == T_STRUCT)
		{
			size = 4;
		}

		param_addr = addr;
		addr += size;

		SymPush(sym->v & ~ViaC_PARAMS, type,
			ViaC_LOCAL | ViaC_LVAL, param_addr);
	}
	func_ret_sub = 0;
	//  __stdcall调用约定，函数本身负责清理堆栈
	if (func_call == KW_STDCALL)
		func_ret_sub = addr - 8;
}

/***********************************************************
* 功能:	生成函数结尾代码
**********************************************************/
void GenEpilog()
{
	int v, saved_ind, opc;

	// 8B /r	mov r32,r/m32	mov r/m32 to r32
	GenOpcode_1(0x8b);/* mov esp, ebp*/
	GenModrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0);

	// POP--Pop a Value from the Stack
	// 58+	rd		POP r32		POP top of stack into r32; increment stack pointer
	GenOpcode_1(0x58 + REG_EBP);  /*pop ebp*/

	if (func_ret_sub == 0)
	{
		// RET--Return from Procedure
		// C3	RET	near return to calling procedure
		GenOpcode_1(0xc3); // ret
	}
	else
	{
		// RET--Return from Procedure
		// C2 iw	RET imm16	near return to calling procedure and pop imm16 bytes form stack
		GenOpcode_1(0xc2);	// ret n
		GenByte(func_ret_sub);
		GenByte(func_ret_sub >> 8);
	}

	v = CalcAlign(-loc, 4);
	saved_ind = ind;
	ind = func_begin_ind; // - FUNC_PROLOG_SIZE;

						  // PUSH--Push Word or Doubleword Onto the Stack
						  // 50+rd	PUSH r32	Push r32
	GenOpcode_1(0x50 + REG_EBP);	// push ebp 

									// 89 /r	MOV r/m32,r32	Move r32 to r/m32
	GenOpcode_1(0x89); //  mov ebp, esp
	GenModrm(ADDR_REG, REG_ESP, REG_EBP, NULL, 0);

	//SUB--Subtract		81 /5 id	SUB r/m32,imm32	Subtract sign-extended imm32 from r/m32
	GenOpcode_1(0x81);	// sub esp, stacksize
	opc = 5;
	GenModrm(ADDR_REG, opc, REG_ESP, NULL, 0);
	GenDword(v);
	ind = saved_ind;
}

/***********************************************************
* 功能:	变量初始化
* type:	变量类型
* sec:		变量所在节
* c:		变量相关值
* v:		变量符号编号
**********************************************************/
void InitVariable(Type *type, Section *sec, int c, int v)
{
	int bt;
	void *ptr;

	if (sec)
	{
		if ((optop->reg & (ViaC_VALMASK | ViaC_LVAL)) != ViaC_GLOBAL)
			Error("全局变量必须用常量初始化");

		bt = type->t & T_BTYPE;
		ptr = sec->data + c;

		switch (bt)
		{
			case T_CHAR:
				*(char *)ptr = optop->value; // 适用于char g_cc = 'a';	printf("g_cc=%c\n",g_cc);
				break;
			case T_SHORT:
				*(short *)ptr = optop->value; // 适用于short g_ss = 1234;	printf("g_ss=%d\n",g_ss);
				break;
			default:
				if (optop->reg & ViaC_SYM)
				{
					CoffRelocAdd(sec, optop->sym, c, IMAGE_REL_I386_DIR32);	// 适用于char *g_pstr = "g_pstr_Hello";
				}
				*(int *)ptr = optop->value;
				break;
		}
		OperandPop();
	}
	else
	{
		if (type->t&T_ARRAY)
		{
			OperandPush(type, ViaC_LOCAL | ViaC_LVAL, c);
			OperandSwap();
			SpillReg(REG_ECX);

			// B8+ rd	mov r32,imm32		mov imm32 to r32
			GenOpcode_1(0xB8 + REG_ECX);//move ecx,n
			GenDword(optop->type.ref->c);
			GenOpcode_1(0xB8 + REG_ESI);
			GenAddr32(optop->reg, optop->sym, optop->value); // move esi <- 
			OperandSwap();

			// LEA--Load Effective Address
			// 8D /r	LEA r32,m	Store effective address for m in register r32
			GenOpcode_1(0x8D);
			GenModrm(ADDR_OTHER, REG_EDI, ViaC_LOCAL, optop->sym, optop->value); // lea edi, [ebp-n]

																				// Instruction prefix	F3H--REPE/REPZ prefix(used only with string instructions)
			GenPrefix(0xf3);//rep movs byte

							 // MOVS/MOVSB/MOVSW/MOVSD--Move Data from String to String
							 // A4	MOVSB	Move byte at address DS:(E)SI to address ES:(E)SI
			GenOpcode_1(0xA4);
			optop -= 2;
		}
		else
		{
			OperandPush(type, ViaC_LOCAL | ViaC_LVAL, c);
			OperandSwap();
			Store_1();
			OperandPop();
		}
	}
}


/***********************************************************
* 功能:		分配存储空间
* type:		变量类型
* r:			变量存储类型
* has_init:	是否需要进行初始化
* v:			变量符号编号
* addr(输出):  变量存储地址
* 返回值:		变量存储节
**********************************************************/
Section *AllocateStorage(Type *type, int r, int has_init, int v, int *addr)
{
	int size, align;
	Section *sec = NULL;
	size = TypeSize(type, &align);

	if (size < 0)
	{
		if (type->t&T_ARRAY && type->ref->type.t == T_CHAR)
		{
			type->ref->c = strlen((char*)tkstr.data) + 1;
			size = TypeSize(type, &align);
		}
		else
			Error("类型尺寸未知");
	}

	// 局部变量在栈中分配存储空间
	if ((r & ViaC_VALMASK) == ViaC_LOCAL)
	{
		loc = CalcAlign(loc - size, align);
		*addr = loc;
	}
	else
	{

		if (has_init == 1)		// 初始化的全局变量在.data节分配存储空间
			sec = sec_data;
		else if (has_init == 2)	// 字符串常量在.regdata节分配存储空间
			sec = sec_rdata;
		else					// 未初始化的全局变量在.bss节分配存储空间
			sec = sec_bss;

		sec->data_offset = CalcAlign(sec->data_offset, align);
		*addr = sec->data_offset;
		sec->data_offset += size;

		// 为需要初始化的数据在节中分配存储空间
		if (sec->sh.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA &&
			sec->data_offset > sec->data_allocated)
			SectionRealloc(sec, sec->data_offset);

		if (v == 0) //常量字符串
		{
			OperandPush(type, ViaC_GLOBAL | ViaC_SYM, *addr);
			optop->sym = sym_sec_rdata;
		}
	}
	return sec;
}



