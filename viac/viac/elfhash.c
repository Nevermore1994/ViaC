/******************************************
*author:Away
*date:2016-11-8
*function:字符哈希函数
*******************************************/
#include"viac.h"


int ElfHash(const char* key)
{
	if (key == NULL)
		Error("字符哈希函数中指针未初始化");
	int h = 0;
	int g;
	while (*key)
	{
		h = (h << 4) + *key++;
		g = h & 0xf0000000;
		if (g)
		{
			h ^= g >> 24;
		}
		h &= ~g;
	}
	return h % MAXKEY;
}