#include"viac.h"

#ifndef		OPERAND_H_
#define 	OPERAND_H_


/*********************operand.h*********************/
typedef struct Operand
{
	Type type;
	unsigned short reg;
	int value;
	struct Symbol* sym;
}Operand;

void OperandPush(Type* type, const int r, const int value);
void OperandPop(void);
void OperandSwap();
void OperandAssign(Operand* opd, const int t, const int r, const int value);
void CancelLvalue(void);
void CheckLvalue(void);
void Indirection(void);
/*********************end**************************/

#endif