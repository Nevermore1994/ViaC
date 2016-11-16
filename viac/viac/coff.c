// ���Լ�����д��������������������Դ����

#include "viac.h"
DynArray sections;			// ������

Section *sec_text,			// �����
*sec_data,			// ���ݽ�
*sec_bss,			// δ��ʼ�����ݽ�
*sec_idata,			// ������
*sec_rdata,			// ֻ�����ݽ�
*sec_rel,			// �ض�λ��Ϣ��
*sec_symtab,		// ���ű��	
*sec_dynsymtab;		// ���ӿ���Ž�

int nsec_image;				// ӳ���ļ��ڸ���

							/***********************************************************
							* ����:		�����������·����ڴ�,�������ݳ�ʼ��Ϊ0
							* sec:			���·����ڴ�Ľ�
							* new_size:	�������³���
							**********************************************************/
void SectionRealloc(Section *sec, int new_size)
{
	int size;
	char *data;

	size = sec->data_allocated;
	while (size < new_size)
		size = size * 2;
	data = realloc(sec->data, size);
	if (!data)
		Error("�ڴ����ʧ��");
	memset(data + sec->data_allocated, 0, size - sec->data_allocated);/* �·�����ڴ����ݳ�ʼ��Ϊ0 */
	sec->data = data;
	sec->data_allocated = size;
}

/***********************************************************
* ����:		��������Ԥ������increment��С���ڴ�ռ�
* sec:			Ԥ���ڴ�ռ�Ľ�
* increment:	Ԥ���Ŀռ��С
* ����ֵ:		Ԥ���ڴ�ռ���׵�ַ
**********************************************************/
void *SectionPtrAdd(Section *sec, int increment)
{
	int offset, offset1;
	offset = sec->data_offset;
	offset1 = offset + increment;
	if (offset1 > sec->data_allocated)
		SectionRealloc(sec, offset1);
	sec->data_offset = offset1;
	return sec->data + offset;
}

/***********************************************************
* ����:			�½���
* name:			������
* Characteristics:	������
* ����ֵ:			�����ӽ�
**********************************************************/
Section * SectionNew(char *name, int Characteristics)
{
	Section *sec;
	int initsize = 8;
	sec = MallocInit(sizeof(Section));
	strcpy_s(sec->sh.Name, strlen(name) + 1, name);
	sec->sh.Characteristics = Characteristics;
	sec->index = sections.count + 1; //one-based index
	sec->data = MallocInit(sizeof(char)*initsize);
	sec->data_allocated = initsize;
	if (!(Characteristics & IMAGE_SCN_LNK_REMOVE))
		nsec_image++;
	DynArrayAdd(&sections, sec);
	return sec;
}

/***********************************************************
* ����:	����COFF����
* symtab:	����COFF���ű�Ľ�
* name:	��������
* ����ֵ:	����COFF���ű������
**********************************************************/
int CoffSymSearch(Section *symtab, char *name)
{
	CoffSym *cfsym;
	int cs, keyno;
	char *csname;
	Section *strtab;

	keyno = ElfHash(name);
	strtab = symtab->plink;
	cs = symtab->hashtab[keyno];
	while (cs)
	{
		cfsym = (CoffSym*)symtab->data + cs;
		csname = strtab->data + cfsym->Name;
		/*�˴������stricmp�ⲿ���õĺ���Դ�����пɲ����ִ�Сд�������ɵ�pe�����õĺ������ƴ�Сд������ȷ��
		����:printf��дΪPrintf�������н����Ȼ����ȷ��,�˴���ô���˿�����Щ���⣬����ElfHash��ϣ��������ĸ
		���ִ�Сд�����printf��PrintF�Ĺ�ϣֵ��ͬ�������������⣬����˴�Ҫ�ģ����뽫��ϣ������Ϊ�����ִ�Сд��
		���ڹ�ϣ�����Ŀ�ͷ��_strupr����һ��
		*/
		if (!strcmp(name, csname))
			return cs;
		cs = cfsym->Next;
	}
	return cs;
}


/***********************************************************
* ����:	����COFF�������ַ���
* strtab:	����COFF�ַ�����Ľ�
* name:	���������ַ���
* ����ֵ:	����COFF�ַ���
**********************************************************/
char *CoffStrAdd(Section *strtab, char* name)
{
	int len;
	char *pstr;
	len = strlen(name);
	pstr = SectionPtrAdd(strtab, len + 1);
	memcpy(pstr, name, len);
	return pstr;
}

/***********************************************************
* ����:			����COFF����
* symtab:			����COFF���ű�Ľ�
* name:			��������
* val:				�������ص�ֵ
* sec_index:		����˷��ŵĽ�
* type:			Coff��������
* StorageClass:	Coff���Ŵ洢���
* ����ֵ:			����COFF���ű������
**********************************************************/
int CoffSymAdd(Section *symtab, char* name, int val, int sec_index,
	short type, char StorageClass)
{
	CoffSym *cfsym;
	int cs, keyno;
	char *csname;
	Section *strtab = symtab->plink;
	int *hashtab;
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
		cfsym->StorageClass = StorageClass;
		cfsym->Value = val;
		keyno = ElfHash(name);
		cfsym->Next = hashtab[keyno];

		cs = cfsym - (CoffSym*)symtab->data;
		hashtab[keyno] = cs;
	}
	return cs;
}

/***********************************************************
* ����:			���ӻ����COFF����,����ֻ�����ں�����������������
* s:				����ָ��
* val:				����ֵ
* sec_index:		����˷��ŵĽ�
* type:			Coff��������
* StorageClass:	Coff���Ŵ洢���
**********************************************************/
void CoffSymAddUpdate(Symbol *s, int val, int sec_index,
	short type, char StorageClass)
{
	char *name;
	CoffSym *cfsym;
	if (!s->c)
	{
		name = ((TkWord*)tktable.data[s->v])->spelling;
		s->c = CoffSymAdd(sec_symtab, name, val, sec_index, type, StorageClass);
	}
	else //��������������
	{
		cfsym = &((CoffSym *)sec_symtab->data)[s->c];
		cfsym->Value = val;
		cfsym->sSection = sec_index;
	}
}

/***********************************************************
* ����:	�ͷ����н�����
**********************************************************/
void FreeSection()
{
	int i;
	Section *sec;
	for (i = 0; i < sections.count; i++)
	{
		sec = (Section*)sections.data[i];
		if (sec->hashtab != NULL)
			free(sec->hashtab);
		free(sec->data);
	}
	DynArrayFree(&sections);
}

/***********************************************************
* ��Ҫ��һ�·��ű�Ĵ洢�ṹ���ڴ�洢�ṹ���ļ��洢�ṹ
* ����:			�½��洢COFF���ű�Ľ�
* symtab:			COFF���ű���
* Characteristics: ������
* strtab_name:		����ű���ص��ַ�����
* ����ֵ:			�洢COFF���ű�Ľ�
**********************************************************/
Section *NewCoffSymSection(char *symtab_name, int Characteristics, char *strtab_name)
{
	Section *sec;
	sec = SectionNew(symtab_name, Characteristics);
	sec->plink = SectionNew(strtab_name, Characteristics);
	sec->hashtab = MallocInit(sizeof(int)*MAXKEY);
	return sec;
}

/***********************************************************
* ����:	����COFF�ض�λ��Ϣ
* offset:	��Ҫ�����ض�λ�Ĵ��������������Ӧ�ڵ�ƫ��λ��
* cfsym:	���ű������
* section: �������ڽڣ��ص㽲һ����Coff����
* type:	�ض�λ����
**********************************************************/
void CoffRelocDirectAdd(int offset, int cfsym, char section, char type)
{
	CoffReloc *rel;
	rel = SectionPtrAdd(sec_rel, sizeof(CoffReloc));
	rel->offset = offset;
	rel->cfsym = cfsym;
	rel->section = section;
	rel->type = type;
}

/***********************************************************
* ����:	�����ض�λ��Ŀ
* section: �������ڽ�
* sym:		����ָ��
* offset:	��Ҫ�����ض�λ�Ĵ��������������Ӧ�ڵ�ƫ��λ��
* type:	�ض�λ����
**********************************************************/
void CoffRelocAdd(Section *sec, Symbol *sym, int offset, char type)
{
	int cfsym;
	char *name;
	if (!sym->c)
		CoffSymAddUpdate(sym, 0, IMAGE_SYM_UNDEFINED, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
	name = ((TkWord*)tktable.data[sym->v])->spelling;
	cfsym = CoffSymSearch(sec_symtab, name);
	CoffRelocDirectAdd(offset, cfsym, sec->index, type);
}

/***********************************************************
* ����:	COFF��ʼ��
**********************************************************/
void InitCoff()
{
	DynArrayInit(&sections, 8);
	nsec_image = 0;

	sec_text = SectionNew(".text",
		IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE);
	sec_data = SectionNew(".data",
		IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE |
		IMAGE_SCN_CNT_INITIALIZED_DATA);
	sec_rdata = SectionNew(".rdata",
		IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA);
	sec_idata = SectionNew(".idata",
		IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE |
		IMAGE_SCN_CNT_INITIALIZED_DATA);
	sec_bss = SectionNew(".bss",
		IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE |
		IMAGE_SCN_CNT_UNINITIALIZED_DATA);
	sec_rel = SectionNew(".rel",
		IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ);


	sec_symtab = NewCoffSymSection(".symtab",
		IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ, ".strtab");

	sec_dynsymtab = NewCoffSymSection(".dynsym",
		IMAGE_SCN_LNK_REMOVE | IMAGE_SCN_MEM_READ, ".dynstr");

	CoffSymAdd(sec_symtab, "", 0, 0, 0, IMAGE_SYM_CLASS_NULL);
	CoffSymAdd(sec_symtab, ".data", 0, sec_data->index, 0, IMAGE_SYM_CLASS_STATIC);
	CoffSymAdd(sec_symtab, ".bss", 0, sec_bss->index, 0, IMAGE_SYM_CLASS_STATIC);
	CoffSymAdd(sec_symtab, ".rdata", 0, sec_rdata->index, 0, IMAGE_SYM_CLASS_STATIC);
	CoffSymAdd(sec_dynsymtab, "", 0, 0, 0, IMAGE_SYM_CLASS_NULL);
}

/***********************************************************
* ����:	�ӵ�ǰ��дλ�õ�new_posλ����0��ļ�����
* fp:		�ļ�ָ��
* new_pos: ��յ�λ��
**********************************************************/
void Fpad(FILE *fp, int new_pos)
{
	int curpos = ftell(fp);
	while (++curpos <= new_pos)
		fputc(0, fp);
}

/***********************************************************
* ����:	���Ŀ���ļ�
* name:	Ŀ���ļ���
**********************************************************/
void WriteObj(char *name)
{
	int file_offset;
	FILE* fout;
	errno_t err = fopen_s(&fout, name, "wb");

	int i, sh_size, nsec_obj = 0;
	IMAGE_FILE_HEADER *fh;

	nsec_obj = sections.count - 2;
	sh_size = sizeof(IMAGE_SECTION_HEADER);
	file_offset = sizeof(IMAGE_FILE_HEADER) + nsec_obj*sh_size;
	Fpad(fout, file_offset);
	fh = MallocInit(sizeof(IMAGE_FILE_HEADER));
	for (i = 0; i < nsec_obj; i++)
	{
		Section *sec = (Section*)sections.data[i];
		if (sec->data == NULL) continue;
		fwrite(sec->data, 1, sec->data_offset, fout);
		sec->sh.PointerToRawData = file_offset;
		sec->sh.SizeOfRawData = sec->data_offset;
		file_offset += sec->data_offset;
	}
	fseek(fout, SEEK_SET, 0);
	fh->Machine = IMAGE_FILE_MACHINE_I386;
	fh->NumberOfSections = nsec_obj;
	fh->PointerToSymbolTable = sec_symtab->sh.PointerToRawData;
	fh->NumberOfSymbols = sec_symtab->sh.SizeOfRawData / sizeof(CoffSym);
	fwrite(fh, 1, sizeof(IMAGE_FILE_HEADER), fout);
	for (i = 0; i < nsec_obj; i++)
	{
		Section *sec = (Section*)sections.data[i];
		fwrite(sec->sh.Name, 1, sh_size, fout);
	}

	free(fh);
	fclose(fout);
}