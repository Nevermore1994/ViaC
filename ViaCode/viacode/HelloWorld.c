/***********************************************************
 * hello.cÔ´ÎÄ¼ş
 **********************************************************/
int max(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}
int main()
{
	int z = 10;             
	int* x = -19;
	int* y = 20;
	int* a = 9 < 10;
	if(2)
		printf("Hello ViaC World! %d\n", x> 0);
	else
	{
		printf("Hello ViaC World! %d\n", 5);
	}
	getchar();
	return 0;
}

void _entry()
{
	int ret;
	ret = main();
	exit(ret);
}