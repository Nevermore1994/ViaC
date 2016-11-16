/******************************************
*author:Away
*date:2016-11-8
*function:�ַ���ϣ����
*******************************************/
#include"viac.h"


int ElfHash(const char* key)
{
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