#include"viac.h"

#ifndef		STACK_H_
#define 	STACK_H_

/******************Stack*********************/
typedef struct Stack
{
	void** base;
	void** top;
	int size;
}Stack;

void StackInit(Stack* stack, const int size);
void* StackPush(Stack* stack, const void* data, const int size);
void StackPop(Stack* stack);
void* StackGetTop(const Stack* stack);
BOOL StackIsEmpty(const Stack* stack);
void StackDestroy(Stack* stack);
/******************end*************************/

#endif
