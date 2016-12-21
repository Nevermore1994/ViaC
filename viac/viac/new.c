#include<stdlib.h>
int new(void** p, int size)
{
	*p = malloc(size);
	if (*p == 0)
	{
		return 0;
	}
	return 1;
}

int cleanup(void** p)
{
	if (*p != 0)
	{
		free(*p);
		*p = 0;
	}
	return 1;
}