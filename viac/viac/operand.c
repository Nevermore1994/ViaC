/******************************************
*author:Away
*date:2016-11-6
*function:辅助模块单元代码
*******************************************/

#include"viac.h"

void OperandPush(Type* type, const int r, const int value)
{
	if (optop >= opstack + (OPSTACK_SIZE - 1))
	{
		Error("内存分配失败");
	} 
	optop++; 
	optop->type = *type;
	optop->reg = r;
	optop->value = value;
}

void OperandPop(void)
{
	--optop;
}

void OperandSwap()
{
	Operand tmp;
	
	tmp = optop[0]; 
	optop[0] = optop[-1];
	optop[-1] = tmp;
}

void OperandAssign(Operand* opd, const int t, const int r, const int value )
{
	opd->type.t = t;
	opd->reg = r;
	opd->value = value;
}

void CancelLvalue(void)
{
	CheckLvalue(); // 左值
	optop->reg &= ~ViaC_LVAL;
}

void CheckLvalue(void)
{
	if (!(optop->reg & ViaC_LVAL))
	{
		Expect("左值");
	}
}

void Indirection(void)
{
	if ((optop->type.t & T_BTYPE) != T_PTR)
	{
		if ((optop->type.t & T_BTYPE) == T_FUNC)
			return;
		Expect("指针");
	}
	if ((optop->reg & ViaC_LVAL))
	{
		Load_1(REG_ANY, optop);
	}
	optop->type = *PointedType(&optop->type);
 
	if ( (!(optop->type.t & T_ARRAY)  && ((optop->type.t)  &  T_BTYPE) ) != T_FUNC )
	{
		optop->reg |= ViaC_LVAL;
	}
}