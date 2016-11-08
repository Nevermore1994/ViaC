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
