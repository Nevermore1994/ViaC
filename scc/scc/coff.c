/******************************************
*Author:Away
*Date:2016-11-6
*Function:windows平台连接模块单元代码
*******************************************/
#include"viac.h"

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

void*  SectionPtrAdd(const pSection sec, const int increment)
{
	int offset, offset1;
	offset = sec->data_offset;
	offset1 = offset + increment;
	if (offset1 > sec->data_allocated)
	{
		SectionRealloc(sec, offset1);
	}
	sec->data_offset = offset1;
	return sec->data + offset;
}

Section* SectionNew(const char* name, const int characteristics)
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

int CoffSymSearch(const pSection symtab, const char* name)
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

char* CoffStrAdd(const pSection strtab, const char* name)
{
	int len; 
	char* pstr; 
	len = strlen(name);
	pstr = SectionPtrAdd(strtab, len + 1);
	memcpy_s(pstr, len, name, len);
	return pstr;
}

int CoffSymAdd(const pSection symtab, const char* name, const int val, const int sec_index, const short type, const char StrorageClass)
{
	CoffSym*  cfsym;
	int cs, keyno;
	char* csname;
	pSection strtab = symtab->plink;
	int* hashtab; 
	hashtab = symtab->hashtab;
	cs = CoffSymSearch(symtab, name);
	if (cs == 0)
	{
		cfsym = SectionPtrAdd(symtab, sizeof(CoffSym));
		csname = CoffStrAdd(strtab, name);
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

void CoffSymAddUpdate(Symbol* ps, const int val, const int sec_index, const short type, const char StroageClass)
{
	char* name; 
	CoffSym* cfsym;
	if (!ps->c)
	{
		name = (( TkWord* ) tktable.data [ps->v])->spelling;
		ps->c = CoffSymAdd(sec_symtab, name, val, sec_index, type, StroageClass);
	}
	else
	{
		cfsym = &(( CoffSym* ) sec_symtab->data) [ps->c]; 
		cfsym->Value = val;
		cfsym->sSection = sec_index;
	}
}

void FreeSection(void)
{
	int i;
	Section*  sec; 
	for (i = 0; i < sections.count; ++i)
	{
		sec = ( Section* ) sections.data [i]; 
		if (sec->hashtab != NULL)
		{
			free(sec->hashtab);
		}
		free(sec->data);
	}
	DynArrayFree(&sections);
}

pSection NewCoffSymSection(const char* symtab_name, const int Characteristics, const char* strtab_name)
{
	pSection sec;
	sec = SectionNew(symtab_name, Characteristics); 
	sec->plink = SectionNew(strtab_name, Characteristics);
	sec->hashtab = MallocInit(sizeof(int) * MAXKEY);
	return sec;
}

void CoffElocDirectAdd(const int offset, const int cfsym, const char section, const char type)
{
	CoffReloc* rel;
	rel = SectionPtrAdd(sec_rel, sizeof(CoffReloc));
	rel->offset = offset; 
	rel->cfsym = cfsym;
	rel->section = section; 
	rel->type = type;
}
 
void CoffElocAdd(pSection sec, Symbol* sym, const int offset, const char type)
{
	int  cfsym; 
	char* name;
	if (!sym->c)
	{
		CoffSymAddUpdate(sym, 0, IMAGE_SYM_UNDEFINED, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
	}
	name = (( TkWord* ) tktable.data [sym->v])->spelling;
	cfsym = CoffSymSearch(sec_symtab, name);
	CoffElocDirectAdd(offset, cfsym, sec->index, type);
}

void InitCoff(void)
{
	DynArrayInit(&sections, 8);
	nsec_image = 0; 

	sec_text = SectionNew(".text", IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE);
	
	sec_data = SectionNew(".data", IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA); 
	
	sec_rdata = SectionNew(".rdata", IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA );
	
	sec_idata = SectionNew(".idata", IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA);
	
	sec_bss = SectionNew(".bss", IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA);

	sec_rel = SectionNew(".rel", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ);
	sec_symtab = NewCoffSymSection(".symtab", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ, ".strtab");
	sec_dynsymtab = NewCoffSymSection(".dynsym", IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ, ".dynstr");

	CoffSymAdd(sec_symtab, "", 0, 0, 0, IMAGE_SYM_CLASS_NULL);
	CoffSymAdd(sec_symtab, ".data", 0, sec_data->index, 0, IMAGE_SYM_CLASS_STATIC); 
	CoffSymAdd(sec_symtab, ".bss", 0, sec_bss->index, 0,IMAGE_SYM_CLASS_STATIC );
	CoffSymAdd(sec_symtab, ".rdata", 0, sec_rdata->index, 0, IMAGE_SYM_CLASS_STATIC);
	CoffSymAdd(sec_dynsymtab, "", 0, 0, 0, IMAGE_SYM_CLASS_STATIC);
}

void Fpad(const FILE* fp, const int new_pos)
{
	if (fp == NULL)
	{
		Error("指针未初始化");
	}
	
	int curpos = ftell(fp);
	while (++curpos <= new_pos)
		fputc(0, fp);
}

void WriteObj(const char* name)
{
	if (name == NULL)
	{
		Error("文件名不能为空");
	}
	
	int file_offset;
	FILE* fout;
	errno_t err = fopen_s(&fout, name, "wb");
	if (err)
	{
		Error("文件打开失败");
	}

	int i, sh_size, nsec_obj = 0;
	IMAGE_FILE_HEADER* fh;

	nsec_obj = sections.count - 2;
	sh_size = sizeof(IMAGE_SECTION_HEADER);
	file_offset = sizeof(IMAGE_FILE_HEADER) + nsec_obj * sh_size;
	Fpad(fout, file_offset);
	fh = MallocInit(sizeof(IMAGE_FILE_HEADER));
	for (i = 0; i < nsec_obj; ++i)
	{
		pSection sec = ( pSection ) sections.data [i];
		if (sec->data == NULL)
			continue;
		fwrite(sec->data, 1, sec->data_offset, fout);
		sec->sh.PointerToRawData = file_offset;
		sec->sh.SizeOfRawData = sec->data_offset;
		file_offset += sec->data_offset;
	}
	fseek(fout, SEEK_SET, 0); 
	fh->Machine = IMAGE_FILE_MACHINE_I386; 
	fh->NumberOfSections = nsec_obj;
	fh->PointerToSymbolTable = sec_symtab->sh.PointerToRawData;
	fh->NumberOfSymbols =sec_symtab->sh.SizeOfRawData/sizeof(CoffSym);
	fwrite(fh, 1, sizeof(IMAGE_FILE_HEADER), fout); 
	for (i = 0; i < nsec_obj; ++i)
	{
		pSection sec = ( pSection ) sections.data [i];
		fwrite(sec->sh.Name, 1, sh_size, fout);
	}

	free(fh);
	fclose(fout);
}