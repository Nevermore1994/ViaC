/******************************************
* Author��Away
* Date: 2016-11-3
* Function������ջģ�鵥Ԫ����
*******************************************/

#include"viac.h" 

void StackInit(Stack* stack, const int size)
{
	if (stack == NULL)
	{
		Error("ָ��δ��ʼ��");
	}
	stack->base = ( void** ) malloc(sizeof(void**) * size); 
	if (stack->base == NULL)
	{
		Error("�ڴ����ʧ��");
	}
	else
	{
		stack->top = stack->base;
		stack->size = size; 
	}
}

void* StackPush(Stack* stack, const void* data, const int size)
{
	if (stack == NULL)
	{
		Error("ָ��δ��ʼ��");
	}
	int newsize;
	if (stack->top >= stack->size + stack->base)
	{
		newsize = stack->size * 2;
		stack->base = ( void** ) realloc(stack->base, sizeof(void**) * newsize);
		if (stack->base == NULL)
		{
			Error("���·���ʧ��");
			return NULL;
		}
		stack->top = stack->size + stack->base;
		stack->size = newsize;
	}
	*stack->top = ( void** ) malloc(size);

	if (*stack->top == NULL)
	{
		Error("�ڴ����ʧ��");
	}
	errno_t  err = memcpy_s(*stack->top, size, data, size);
	if (err)
	{
		Error("��������г����ڴ渴�ƴ���");
	}
	stack->top++;

	return *(stack->top - 1);
}

void StackPop(Stack* stack)
{
	if (stack == NULL)
	{
		Error("ָ��δ��ʼ��");
	}
	if (stack->top > stack->base)
	{
		free(* (--stack->top));
	}
}

void* StackGetTop(const Stack* stack)
{
	void** topdata = NULL;
	if (stack == NULL)
	{
		Error("ָ��δ��ʼ��");
	}
	if (stack->top > stack->base)
	{ 
		topdata = stack->top - 1; 
		return *topdata;
	}
	return NULL;
}

bool StackIsEmpty(const Stack* stack)
{
	if (stack == NULL)
	{
		Error("ָ��δ��ʼ��");
	}
	if (stack->top  == stack->base)
		return TRUE;
	return FALSE;
}

void StackDestroy(Stack* stack)
{
	if (stack == NULL)
	{
		Error("ָ��δ��ʼ��");
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
