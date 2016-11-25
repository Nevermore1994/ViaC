#include"viac.h"

#ifndef		GENCODE_H_
#define 	GENCODE_H_

/**********************gencode.h**************************/
enum e_Register
{
	REG_EAX = 0,
	REG_ECX,
	REG_EDX,
	REG_EBX,
	REG_ESP,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_ANY
};

#define REG_IRET REG_EAX

enum e_Address
{
	ADDR_OTHER,
	ADDR_REG = 3,
};

#define OPSTACK_SIZE 256
#define FUNC_PROLOG_SIZE 9

extern int rsym;
extern int ind;
extern int loc;
extern int func_begin_ind;
extern int func_ret_sub;
extern Symbol* sym_sec_rdata;
extern Operand opstack[OPSTACK_SIZE];
extern Operand* optop;

void GenByte(const char c);
void GenPrefix(const char opcode);
void GenOpcode_1(const char opcode);
void GenOpcode_2(const char first, const char end);
void GenDwordd(unsigned int c);
void BackPatch(int t, const int a);
int MakeList(int add);
void GenAddr32(const int r, Symbol* sym, const int c);
void GenModrm(int mod, int reg_opcode, const int r_m, const Symbol* sym, const int c);
void Load(const int r, Operand* opd);
void Store(const int r, Operand* opd);
int Load_1(const int rc, Operand* opd);
void Load_2(const int lrc, const int rrc);
void Store_1(void);
void GenAddsp(const int val);
void GenCall(void);
void GenInvoke(const int nb_args);
void GenOpi_1_2(const int opc, const int op);
void GenOpi_1_1(int op);
Type* PointedType(Type* t);
int PointedSize(Type* t);
void GenOp(const int op);
int AllocateReg(const int rc);
void SpillReg(int reg);
void SpillRegs();
int GenJmpForWard(const int t);
void GenJmpBackWord(const int a);
int GenJcc(int t);
void GenProlog(Type* func_type);
void GenEpilog();
void InitVariable(Type* ptype, Section* psec, const int c, const int v);
Section* AllocateStorage(Type* ptype, const int reg, const int has_init, const int v, int* addr);

/***********************end*****************************/

#endif