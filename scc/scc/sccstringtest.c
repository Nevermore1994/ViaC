#include"scc.h"

int main()
{ 
	DynArray arr;
	DynArray* p = &arr;
	int num = 10;
	Dynarray_init(p,20);
	int a = 2;
	int b = 3;
	Dynarray_add(p, &a);
	Dynarray_add(p, &b);
	Dynarray_add(p, &num);
	printf("%d", Dynarray_find(p, 10));
	Dynarray_free(p);
	
	return 0;
}