#include"scc.h"

DynArray sections; 
pSection sec_text; 
pSection sec_data;
pSection sec_bss; 
pSection sec_idata; 
pSection sec_rdata; 
pSection sec_rel; 
pSection sec_symtab;
pSection sec_dynsymtab;

int nsec_image;

void SectionRealloc(pSection sec, const int newsize)
{
	if (sec == NULL)
	{
		Error("coff中的指针未初始化");
	}
	
	int size;
	char* data;
	size = sec->data_allocated; 
	while (size < newsize)
		size = size * 2;
	data = (char*)realloc(sec->data, size);
	if (!data)
	{
		Error("内存分配失败");
	}
	memset(data+sec->data_allocated, 0 ,size - sec->data_allocated);
	sec->data = data; 
	sec->data_allocated = size;
}

void*  SectionPtrAdd(pSection sec, int increment)
{
	int offset, offset1;
	offset = sec->data_offset;
	offset1 = offset1 + increment;
	if (offset1 > sec->data_allocated)
	{
		SectionRealloc(sec, offset1);
	}
	sec->data_offset = offset1;
	return sec->data + offset;
}

Section* SectionNew(const char* name, int characteristics)
{
	Section* sec; 
	int initsize = 8;
	sec = (Section*)MallocInit(sizeof(Section));
	size_t length = strlen(name); 
	strcpy_s((char*)sec->sh.Name, IMAGE_SIZEOF_SHORT_NAME,name);
	sec->sh.Characteristics = characteristics; 
	sec->index = sections.count + 1; 
	sec->data = ( char* ) MallocInit(sizeof(char) * initsize);
	sec->data_allocated = initsize; 
	if (!(characteristics & IMAGE_SCN_LNK_REMOVE))
	{
		nsec_image++;
	}
	DynArrayAdd(&sections, sec);
	return sec;
}

int CoffsymSearch(pSection symtab, char* name)
{
	CoffSym* cfsym;
	int cs, keyno; 
	char* csname; 
	pSection strtab;
	
	keyno = ElfHash(name);
	strtab = symtab->plink;
	cs = symtab->hashtab [keyno]; 
	while (cs)
	{
		cfsym = ( CoffSym* ) symtab->data + cs;
		csname = strtab->data + cfsym->Name;
		if (!strcmp(name, csname))
			return cs;
		cs = cfsym->Next;
	}
	return cs;
}

char* CoffstrAdd(pSection strtab, char* name)
{
	int len; 
	char* pstr; 
	len = strlen(name);
	pstr = SectionPtrAdd(strtab, len + 1);
	memcpy_s(pstr, len, name, len);
	return pstr;
}

int CoffsymAdd(pSection symtab, char* name, int val, int sec_index, short type, char StrorageClass)
{
	CoffSym*  cfsym;
	int cs, keyno;
	char* csname;
	pSection strtab = symtab->plink;
	int* hashtab; 
	hashtab = symtab->hashtab;
	cs = CoffsymSearch(symtab, name);
	if (cs == 0)
	{
		cfsym = SectionPtrAdd(symtab, sizeof(CoffSym));
		csname = CoffstrAdd(strtab, name);
		cfsym->Name = csname - strtab->data;
		cfsym->Value = val;
		cfsym->sSection = sec_index;
		cfsym->Type = type;
		cfsym->StorageClass = StrorageClass;
		cfsym->Value = val;
		keyno = ElfHash(name);
		cfsym->Next = hashtab [keyno];

		cs = cfsym - ( CoffSym* ) symtab->data;
		hashtab [keyno] = cs;
	}
	return cs;
}