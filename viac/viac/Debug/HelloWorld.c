/***********************************************************
 * hello.cԴ�ļ�
 **********************************************************/
#require "viacio.h"
int main() //
{
	int* x = 10;
	printf("%d\n", x);
	return 0;
}

void _entry()
do
int ret;
ret = main();
exit(ret);
end
