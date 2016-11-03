/******************************************
* Author：Away
* Date: 2016-11-3
*******************************************/


#include"scc.h" 

void StackInit(Stack* stack, int size)
{
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	stack->base = ( void** ) malloc(sizeof(void**) * size); 
	if (stack->base == NULL)
	{
		Error("内存分配失败");
	}
	else
	{
		stack->top = stack->base;
		stack->size = size; 
	}
}

void* StackPush(Stack* stack, void* data, int size)
{
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	int newsize;
	if (stack->top >= stack->size + stack->base)
	{
		newsize = stack->size * 2;
		stack->base = ( void** ) realloc(stack->base, newsize);
		if (stack->base == NULL)
		{
			Error("重新分配失败");
			return NULL;
		}
		stack->top = stack->size + stack->base;
		stack->size = newsize;
	}
	*stack->top = ( void** ) malloc(size);
	if (*stack->top == NULL)
	{
		Error("内存分配失败");
	}
	errno_t  err=memcpy_s(*stack->top, size, data, size);
	if (err)
	{
		Error("编译过程中出现内存复制错误");
	}
	stack->top++;

	return *(stack->top - 1);
}

void StackPop(Stack* stack)
{
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	if (stack->top > stack->base)
	{
		free(*stack->top);
		stack->top--;
	}
}

void* StackgGetTop(Stack* stack)
{
	void** topdata = NULL;
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	if (stack->top > stack->base)
	{ 
		topdata = stack->top-1; 
		return *topdata;
	}
	return NULL;
}

bool StackIsEmpty(Stack* stack)
{
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	if (stack->top  == stack->base)
		return TRUE;
	return FALSE;
}

void StackDestroy(Stack* stack)
{
	if (stack == NULL)
	{
		Error("指针未初始化");
	}
	void** element;
	for (element = stack->base; element < stack->top; element++)
	{
		free(*element);
	}  
	if (stack->base)
	{
		free(stack->base);
	} 
	stack->base = NULL;
	stack->top  = NULL;
	stack->size = 0;
}

