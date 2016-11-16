// ���Լ�����д��������������������Դ����

#include "viac.h"
/* �����õ���ȫ�ֱ��� */
int rsym;						// ��¼returnָ��λ��
int ind = 0;					// ָ���ڴ����λ��
int loc;						// �ֲ�������ջ��λ��
int func_begin_ind;				// ������ʼָ��
int func_ret_sub;				// ���������ͷ�ջ�ռ��С
Symbol *sym_sec_rdata;			// ֻ���ڷ���
Operand opstack[OPSTACK_SIZE];  // ������ջ
Operand *optop;					// ������ջջ��

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
* ����:	����ָ��ǰ׺
* opcode:	ָ��ǰ׺����
**********************************************************/
void GenPrefix(char opcode)
{
	GenByte(opcode);
}

/***********************************************************
* ����:	���ɵ��ֽ�ָ��
* opcode:  ָ�����
**********************************************************/
void GenOpcode_1(char opcode)
{
	GenByte(opcode);
}

/***********************************************************
* ����:	����˫�ֽ�ָ��
* first:	ָ���һ���ֽ�
* second:	ָ��ڶ����ֽ�
**********************************************************/
void GenOpcode_2(char first, char second)
{
	GenByte(first);
	GenByte(second);
}

/***********************************************************
* ����:	����4�ֽڲ�����
* c:		4�ֽڲ�����
**********************************************************/
void GenDword(unsigned int c)
{
	GenByte(c);
	GenByte(c >> 8);
	GenByte(c >> 16);
	GenByte(c >> 24);
}

/***********************************************************
* ����:	���������tΪ���׵ĸ���������ת��ַ������Ե�ַ
* t:		����
* a:		ָ����תλ��
**********************************************************/
void BackPatch(int t, int a)
{
	int n, *ptr;
	while (t)
	{
		ptr = (int *)(sec_text->data + t);
		n = *ptr; // ��һ����Ҫ����λ��
		*ptr = a - t - 4;
		t = n;
	}
}

/***********************************************************
* ����:	��¼������ת��ַ��ָ����
* s:		ǰһ��תָ���ַ
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
* ����:	����ȫ�ַ��ŵ�ַ,������COFF�ض�λ��¼
* output constant with relocation if 'r & ViaC_SYM' is true
* r:		���Ŵ洢����
* sym:		����ָ��
* c:		���Ź���ֵ
**********************************************************/
void GenAddr32(int r, Symbol *sym, int c)
{
	if (r & ViaC_SYM)
		CoffRelocAdd(sec_text, sym, ind, IMAGE_REL_I386_DIR32);
	GenDword(c);
}

/***********************************************************
* ����:		����ָ��Ѱַ��ʽ�ֽ�ModR/M,
* mod:			ModR/M [7:6]
* reg_opcode:	ModR/M [5:3]ָ�������3λ������ Դ������(�з���׼ȷ)
* r_m:			ModR/M [2:0] Ŀ�������(�з���׼ȷ��
* sym:		����ָ��
* c:		���Ź���ֵ
**********************************************************/
void GenModrm(int mod, int reg_opcode, int r_m, Symbol *sym, int c)
{
	mod <<= 6;
	reg_opcode <<= 3;
	if (mod == 0xc0) // mod=11 �Ĵ���Ѱַ 89 E5(11 reg_opcode=100ESP r=101EBP) MOV EBP,ESP
	{
		GenByte(mod | reg_opcode | (r_m & ViaC_VALMASK));
	}
	else if ((r_m & ViaC_VALMASK) == ViaC_GLOBAL)
	{
		// mod=00 r=101 disp32  8b 05 50 30 40 00  MOV EAX,DWORD PTR DS:[403050]
		// 8B /r MOV r32,r/m32 Move r/m32 to r32
		// mod=00 r=101 disp32  89 05 14 30 40 00  MOV DWORD PTR DS:[403014],EAX
		// 89 /r MOV r/m32,r32 Move r32 to r/m32
		GenByte(0x05 | reg_opcode); //ֱ��Ѱַ
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
* ����:	��������opd���ص��Ĵ���r��
* r:		���Ŵ洢����
* opd:		������ָ��
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
		else if (v == ViaC_CMP) // ������c=a>b���
		{
			/*����������ɵ���������
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
* ����:	���Ĵ���'r'�е�ֵ���������'opd'
* r:		���Ŵ洢����
* opd:		������ָ��
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
* ����:	��ջ�����������ص�'rc'��Ĵ�����
* rc:		�Ĵ�������
* opd:		������ָ��
**********************************************************/
int Load_1(int rc, Operand *opd)
{
	int r;
	r = opd->reg & ViaC_VALMASK;
	// ��Ҫ���ص��Ĵ����������
	// 1.ջ��������Ŀǰ��δ����Ĵ���
	// 2.ջ���������ѷ���Ĵ�������Ϊ��ֵ *p
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
* ����:	��ջ�����������ص�'rc1'��Ĵ���������ջ�����������ص�'rc2'��Ĵ���
* rc1:		ջ�����������ص��ļĴ�������
* rc2:		��ջ�����������ص��ļĴ�������
**********************************************************/
void Load_2(int rc1, int rc2)
{
	Load_1(rc2, optop);
	Load_1(rc1, &optop[-1]);
}

/***********************************************************
* ����:	��ջ�������������ջ����������
**********************************************************/
void Store_1()
{
	int r, t;
	r = Load_1(REG_ANY, optop);
	// ��ֵ����������ջ�У�������ص��Ĵ�����
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
* ����:	�����꺯����ָ�/�ͷ�ջ,����__cdecl
* val:		��Ҫ�ͷ�ջ�ռ��С(���ֽڼ�)
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
* ����:	���ɺ�������ָ��
**********************************************************/
void GenCall()
{
	int r;
	if ((optop->reg & (ViaC_VALMASK | ViaC_LVAL)) == ViaC_GLOBAL)
	{
		// ��¼�ض�λ��Ϣ
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
* ����:	���ɺ������ô���,�Ƚ�������ջ��Ȼ��call
* nb_args: ��������
**********************************************************/
void GenInvoke(int nb_args)
{
	int size, r, args_size, i, func_call;

	args_size = 0;
	// ����������ջ
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
	func_call = optop->type.ref->r;  // �õ�����Լ����ʽ
	GenCall();
	if (args_size && func_call != KW_STDCALL)
		gen_addsp(args_size);
	OperandPop();
}

/***********************************************************
* ����:	����������Ԫ����
* opc:		ModR/M [5:3]
* op:		���������
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
* ����:	������������
* op:		���������
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
			// EDX:EAX������ r/m32����
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
* ����:	����t��ָ�����������
* t:		ָ������
**********************************************************/
Type *PointedType(Type *t)
{
	return &t->ref->type;
}

/***********************************************************
* ����:	����t��ָ����������ͳߴ�
* t:		ָ������
**********************************************************/
int PointedSize(Type *t)
{
	int align;
	return TypeSize(PointedType(t), &align);
}

/***********************************************************
* ����:	���ɶ�Ԫ����,��ָ�����������һЩ���⴦��
* op:		���������
**********************************************************/
void GenOp(int op)
{
	int u, bt1, bt2;
	Type type1;

	bt1 = optop[-1].type.t & T_BTYPE;
	bt2 = optop[0].type.t & T_BTYPE;

	if (bt1 == T_PTR || bt2 == T_PTR)
	{
		if (op >= TK_EQ && op <= TK_GEQ) // ��ϵ����
		{
			GenOpi_1(op);
			optop->type.t = T_INT;
		}
		else if (bt1 == T_PTR && bt2 == T_PTR) //������������Ϊָ��
		{
			if (op != TK_MINUS)
				Error("����ָ��ֻ�ܽ��й�ϵ���������");
			u = PointedSize(&optop[-1].type);
			GenOpi_1(op);
			optop->type.t = T_INT;
			OperandPush(&int_type, ViaC_GLOBAL, u);
			GenOp(TK_DIVIDE);
		}
		else //����������һ����ָ�룬��һ������ָ�룬���ҷǹ�ϵ����
		{
			if (op != TK_MINUS && op != TK_PLUS)
				Error("ָ��ֻ�ܽ��й�ϵ��Ӽ�����");
			// ָ����Ϊ��һ������
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
			// ��ϵ������ΪT_INT����
			optop->type.t = T_INT;
		}
	}
}


/***********************************************************
* ����:	�Ĵ������䣬�������Ĵ�����ռ��,�Ƚ������������ջ��
* rc:		�Ĵ�������
**********************************************************/
int AllocateReg(int rc)
{
	int r;
	Operand *p;
	int used;

	/* ���ҿ��еļĴ��� */
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

	// ���û�п��еļĴ������Ӳ�����ջ�׿�ʼ���ҵ���һ��ռ�õļĴ����ٳ���ջ��
	for (p = opstack; p <= optop; p++)
	{
		r = p->reg & ViaC_VALMASK;
		if (r < ViaC_GLOBAL && (rc & REG_ANY || r == rc))
		{
			SpillReg(r);
			return r;
		}
	}
	/* �˴���Զ�����ܵ��� */
	return -1;
}

/***********************************************************
* ����:	���Ĵ���'r'������ڴ�ջ��,���ұ���ͷ�'r'�Ĵ����Ĳ�����Ϊ�ֲ�����
* r:		�Ĵ�������
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
				p->reg = (p->reg & ~(ViaC_VALMASK)) | ViaC_LLOCAL; //��ʶ����������ջ��
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
* ����:	��ռ�õļĴ���ȫ�������ջ��
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
* ����:	������ߵ�ַ��תָ���ת��ַ����
* t:		ǰһ��תָ���ַ
**********************************************************/
int GenJmpForWard(int t)
{
	// JMP--Jump		
	// E9 cd	JMP rel32	Jump near,relative,displacement relative to next instruction
	GenOpcode_1(0xe9);
	return MakeList(t);;
}

/***********************************************************
* ����:	������͵�ַ��תָ���ת��ַ��ȷ��
* a:		��ת����Ŀ���ַ
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
* ����:	����������תָ��
* t:		ǰһ��תָ���ַ
* ����ֵ:  ����תָ���ַ
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
* ����:		���ɺ�����ͷ����
* func_type:	��������
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

	func_begin_ind = ind;  //��¼�˺����忪ʼ���Ա㺯�������ʱ��亯��ͷ����Ϊ��ʱ����ȷ�����ٵ�ջ��С
	ind += FUNC_PROLOG_SIZE;
	if (sym->type.t == T_STRUCT)
		Error("��֧�ַ��ؽṹ�壬���Է��ؽṹ��ָ��");
	//  ��������
	while ((sym = sym->next) != NULL)
	{
		type = &sym->type;
		size = TypeSize(type, &align);
		size = CalcAlign(size, 4); //�˾�ǳ���Ҫ����Ϊ����ѹջ����4�ֽڶ���ģ����Դ˴�ÿ����������4�ֽڶ���
									//  �ṹ����Ϊָ�봫��
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
	//  __stdcall����Լ�������������������ջ
	if (func_call == KW_STDCALL)
		func_ret_sub = addr - 8;
}

/***********************************************************
* ����:	���ɺ�����β����
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
* ����:	������ʼ��
* type:	��������
* sec:		�������ڽ�
* c:		�������ֵ
* v:		�������ű��
**********************************************************/
void InitVariable(Type *type, Section *sec, int c, int v)
{
	int bt;
	void *ptr;

	if (sec)
	{
		if ((optop->reg & (ViaC_VALMASK | ViaC_LVAL)) != ViaC_GLOBAL)
			Error("ȫ�ֱ��������ó�����ʼ��");

		bt = type->t & T_BTYPE;
		ptr = sec->data + c;

		switch (bt)
		{
			case T_CHAR:
				*(char *)ptr = optop->value; // ������char g_cc = 'a';	printf("g_cc=%c\n",g_cc);
				break;
			case T_SHORT:
				*(short *)ptr = optop->value; // ������short g_ss = 1234;	printf("g_ss=%d\n",g_ss);
				break;
			default:
				if (optop->reg & ViaC_SYM)
				{
					CoffRelocAdd(sec, optop->sym, c, IMAGE_REL_I386_DIR32);	// ������char *g_pstr = "g_pstr_Hello";
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
* ����:		����洢�ռ�
* type:		��������
* r:			�����洢����
* has_init:	�Ƿ���Ҫ���г�ʼ��
* v:			�������ű��
* addr(���):  �����洢��ַ
* ����ֵ:		�����洢��
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
			Error("���ͳߴ�δ֪");
	}

	// �ֲ�������ջ�з���洢�ռ�
	if ((r & ViaC_VALMASK) == ViaC_LOCAL)
	{
		loc = CalcAlign(loc - size, align);
		*addr = loc;
	}
	else
	{

		if (has_init == 1)		// ��ʼ����ȫ�ֱ�����.data�ڷ���洢�ռ�
			sec = sec_data;
		else if (has_init == 2)	// �ַ���������.regdata�ڷ���洢�ռ�
			sec = sec_rdata;
		else					// δ��ʼ����ȫ�ֱ�����.bss�ڷ���洢�ռ�
			sec = sec_bss;

		sec->data_offset = CalcAlign(sec->data_offset, align);
		*addr = sec->data_offset;
		sec->data_offset += size;

		// Ϊ��Ҫ��ʼ���������ڽ��з���洢�ռ�
		if (sec->sh.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA &&
			sec->data_offset > sec->data_allocated)
			SectionRealloc(sec, sec->data_offset);

		if (v == 0) //�����ַ���
		{
			OperandPush(type, ViaC_GLOBAL | ViaC_SYM, *addr);
			optop->sym = sym_sec_rdata;
		}
	}
	return sec;
}



