#include"scc.h"

int main()
{ 
	DynString string;
	DynString* str = &string;
	Dynstring_init(str, 20);
	Dynstring_chcat(str, 'A');
	printf("%d\n", str->capacity);
	printf("%d\n", str->count);
	printf("%s\n", str->data);
	return 0;
}