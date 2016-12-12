#include "viac.h"

char* entry_symbol = "_entry";
Array arr_dll;
Array arr_lib;
char* lib_path;
short subsystem;
#define MAX_PATH  260

IMAGE_DOS_HEADER dos_header = {
	/* IMAGE_DOS_HEADER doshdr */
	0x5A4D, /*WORD e_magic;         DOS可执行文件标记,为'MZ'  */
	0x0090, /*WORD e_cblp;          Bytes on last page of file */
	0x0003, /*WORD e_cp;            Pages in file */
	0x0000, /*WORD e_crlc;          Relocations */

	0x0004, /*WORD e_cparhdr;       Size of header in paragraphs */
	0x0000, /*WORD e_minalloc;      Minimum extra paragraphs needed */
	0xFFFF, /*WORD e_maxalloc;      Maximum extra paragraphs needed */
	0x0000, /*WORD e_ss;            DOS代码的初始化堆栈段 */

	0x00B8, /*WORD e_sp;            DOS代码的初始化堆栈指针 */
	0x0000, /*WORD e_csum;          Checksum */
	0x0000, /*WORD e_ip;            DOS代码的入口IP */
	0x0000, /*WORD e_cs;            DOS代码的入口CS */
	0x0040, /*WORD e_lfarlc;        File address of relocation table */
	0x0000, /*WORD e_ovno;          Overlay number */
	{ 0,0,0,0 }, /*WORD e_res[4];     Reserved words */
	0x0000, /*WORD e_oemid;         OEM identifier (for e_oeminfo) */
	0x0000, /*WORD e_oeminfo;       OEM information; e_oemid specific */
	{ 0,0,0,0,0,0,0,0,0,0 }, /*WORD e_res2[10];      Reserved words */
	0x00000080  /*DWORD   e_lfanew;        指向PE文件头 */
};

BYTE dos_stub[0x40] = {
	/* BYTE dosstub[0x40] */
	/* 14 code bytes + "This program cannot be run in DOS mode.\r\r\n$" + 6 * 0x00 */
	0x0e,0x1f,0xba,0x0e,0x00,0xb4,0x09,0xcd,0x21,0xb8,0x01,0x4c,0xcd,0x21,0x54,0x68,
	0x69,0x73,0x20,0x70,0x72,0x6f,0x67,0x72,0x61,0x6d,0x20,0x63,0x61,0x6e,0x6e,0x6f,
	0x74,0x20,0x62,0x65,0x20,0x72,0x75,0x6e,0x20,0x69,0x6e,0x20,0x44,0x4f,0x53,0x20,
	0x6d,0x6f,0x64,0x65,0x2e,0x0d,0x0d,0x0a,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

IMAGE_NT_HEADERS32 nt_header = {
	0x00004550, /* DWORD Signature = IMAGE_NT_SIGNATURE PE文件标识*/
	{
		/* IMAGE_FILE_HEADER FileHeader */
		0x014C,		/*WORD    Machine; 运行平台*/
		0x0003,		/*WORD    NumberOfSections; 文件的节数目 */
	0x00000000, /*DWORD   TimeDateStamp; 文件的创建日期和时间*/
	0x00000000, /*DWORD   PointerToSymbolTable; 指向符号表(用于调试)*/
	0x00000000, /*DWORD   NumberOfSymbols; 符号表中的符号数量（用于调试）*/
	0x00E0,		/*WORD    SizeOfOptionalHeader; IMAGE_OPTIONAL_HEADER32结构的长度*/
	0x030F		/*WORD    Characteristics; 文件属性*/
	},
	{
		/* IMAGE_OPTIONAL_HEADER OptionalHeader */
		/* 标准域. */
		0x010B, /*WORD    Magic; */
		0x06, /*BYTE    MajorLinkerVersion; 链接器主版本号*/
	0x00, /*BYTE    MinorLinkerVersion; 链接器次版本号*/
	0x00000000, /*DWORD   SizeOfCode; 所有含代码段的总大小*/
	0x00000000, /*DWORD   SizeOfInitializedData; 所有已初始化数据段的总大小*/
	0x00000000, /*DWORD   SizeOfUninitializedData; 所有含有未初始化数据段的大小*/
	0x00000000, /*DWORD   AddressOfEntryPoint; 程序执行入口的相对虚拟地址*/
	0x00000000, /*DWORD   BaseOfCode; 代码段的起始RVA*/
	0x00000000, /*DWORD   BaseOfData; 代码段的起始RVA*/

				/* NT附加域 */
	0x00400000, /*DWORD   ImageBase; 程序的建议装载地址 讲一下Windows内存结构，如果大于0x80000000会有什么后果*/
	0x00001000, /*DWORD   SectionAlignment; 内存中段的对齐粒度*/
	0x00000200, /*DWORD   FileAlignment; 文件中段的对齐粒度*/
	0x0004, /*WORD    MajorOperatingSystemVersion; 操作系统的主版本号*/
	0x0000, /*WORD    MinorOperatingSystemVersion; 操作系统的次版本号*/
	0x0000, /*WORD    MajorImageVersion; 程序的主版本号*/
	0x0000, /*WORD    MinorImageVersion; 程序的次版本号*/
	0x0004, /*WORD    MajorSubsystemVersion; 子系统的主版本号*/
	0x0000, /*WORD    MinorSubsystemVersion; 子系统的次版本号*/
	0x00000000, /*DWORD   Win32VersionValue; 保留，设为0*/
	0x00000000, /*DWORD   SizeOfImage; 内存中整个PE映像尺寸*/
	0x00000200, /*DWORD   SizeOfHeaders; 所有头+节表的大小*/
	0x00000000, /*DWORD   CheckSum; 校验和*/
	0x0003, /*WORD    Subsystem; 文件的子系统*/
	0x0000, /*WORD    DllCharacteristics; */
	0x00100000, /*DWORD   SizeOfStackReserve; 初始化时堆栈大小*/
	0x00001000, /*DWORD   SizeOfStackCommit; 初始化时实际提交的堆栈大小*/
	0x00100000, /*DWORD   SizeOfHeapReserve; 初始化时保留的堆大小*/
	0x00001000, /*DWORD   SizeOfHeapCommit; 初始化时实际提交的堆大小*/
	0x00000000, /*DWORD   LoaderFlags; 保留，设为0*/
	0x00000010, /*DWORD   NumberOfRvaAndSizes; 下面的数据目录结构的数量*/

				/* IMAGE_DATA_DIRECTORY DataDirectory[16]; 数据目录*/
	{
		{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
		{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },{ 0,0 }
	}
	}
};

DWORD PeFileAlign(const DWORD addr)
{
	DWORD FileAlignment = nt_header.OptionalHeader.FileAlignment;
	return CalcAlign(addr, FileAlignment);
}


DWORD PeVirtualAlign(const DWORD addr)
{
	DWORD SectionAlignment = nt_header.OptionalHeader.SectionAlignment;
	return CalcAlign(addr, SectionAlignment);
}

void PeSetDatadir(const int dir, const DWORD addr, const DWORD size)
{
	nt_header.OptionalHeader.DataDirectory[dir].VirtualAddress = addr;
	nt_header.OptionalHeader.DataDirectory[dir].Size = size;

}

int LoadObjFile(const char* fname)
{
	if (fname == NULL)
		Error("文件名不能为空");

	int sh_size;
	sh_size = sizeof(IMAGE_SECTION_HEADER);
	FILE* fobj = NULL;
	errno_t err = fopen_s(&fobj, fname, "rb");
	if (err)
	{
		Error("不能打开文件");
	}
	IMAGE_FILE_HEADER fh;
	fread(&fh, 1, sizeof(IMAGE_FILE_HEADER), fobj);
	int nsec_obj;
	nsec_obj = fh.NumberOfSections;
	Section** ppsec = (Section**)sections.data;

	int i;
	for (i = 0; i < nsec_obj; ++i)
	{
		fread(ppsec[i]->sh.Name, 1, sh_size, fobj);
	}
	int cur_text_offset = sec_text->data_offset;
	int cur_rel_offset = sec_rel->data_offset;
	CoffSym* cfsyms = NULL;
	CoffSym* cfsym = NULL;
	int nsym = 0;
	char* strs = NULL;
	void* ptr = NULL;
	for (i = 0; i < nsec_obj; ++i)
	{
		if (!strcmp(ppsec[i]->sh.Name, ".symtab"))
		{
			cfsyms = MallocInit(ppsec[i]->sh.SizeOfRawData);
			fseek(fobj, SEEK_SET, ppsec[i]->sh.PointerToRawData);
			fread(cfsyms, 1, ppsec[i]->sh.SizeOfRawData, fobj);
			nsym = (ppsec[i]->sh.SizeOfRawData) / sizeof(CoffSym);
			continue;
		}
		if (!strcmp(ppsec[i]->sh.Name, ".strtab"))
		{
			strs = MallocInit(ppsec[i]->sh.SizeOfRawData);
			fseek(fobj, SEEK_SET, ppsec[i]->sh.PointerToRawData);
			fread(strs, 1, ppsec[i]->sh.SizeOfRawData, fobj);
			continue;
		}
		if (!strcmp(ppsec[i]->sh.Name, ".dynsym") || !strcmp(ppsec[i]->sh.Name, ".dynstr"))
			continue;
		fseek(fobj, SEEK_SET, ppsec[i]->sh.PointerToRawData);
		ptr = SectionPtrAdd(ppsec[i], ppsec[i]->sh.SizeOfRawData);
		fread(ptr, 1, ppsec[i]->sh.SizeOfRawData, fobj);
	}
	int* replace_sym = MallocInit(sizeof(int) * nsym);
	char* csname = NULL;
	int cfsym_index = 0;
	for (i = 1; i < nsym; ++i)
	{
		cfsym = &cfsyms[i];
		csname = strs + cfsym->Name;
		cfsym_index = CoffSymAdd(sec_symtab, csname, cfsym->Value, cfsym->sSection, cfsym->Type, cfsym->StorageClass);
		replace_sym[i] = cfsym_index;
	}
	CoffReloc* rel = (CoffReloc*)(sec_rel->data + cur_rel_offset);
	CoffReloc* rel_end = (CoffReloc*)(sec_rel->data + sec_rel->data_offset);
	for (; rel < rel_end; ++rel)
	{
		cfsym_index = rel->cfsym;
		rel->cfsym = replace_sym[cfsym_index];
		rel->offset += cur_text_offset;
	}
	free(cfsyms);
	free(strs);
	free(replace_sym);
	fclose(fobj);
	return 1;
}

char* GetLine(char* line, const int size, const FILE* fp)
{
	char* p;
	char* p1;
	if (NULL == fgets(line, size, fp))
		return NULL;
	p = line;
	while (*p && isspace(*p))
		++p;
	p1 = strchr(p, '\0');
	while (p1 > p && p1[-1] <= ' ')
		--p1;
	*p1 = '\0';
	return p;
}

char* GetDllName(const char* libfile)
{

	if (libfile == NULL)
		Error("链接器内部指针未初始化");
	int len;
	const char* libname;
	len = strlen(libfile);
	libname = libfile;
	const char* p;
	for (p = libfile + len; len > 0; --p)
	{
		if (*p == '\\')
		{
			libname = p + 1;
			break;
		}
		--len;
	}

	int namelen = strlen(libname);
	char* dllname;
	dllname = MallocInit(sizeof(char) * namelen);
	memcpy_s(dllname, namelen - 4, libname, namelen - 4);
	memcpy_s(dllname + namelen - 4, 4, "dll", 3);
	return dllname;
}

int PeLoadLibFile(char* fname)
{
	if (fname == NULL)
		Error("名字不能为空");
	char libfile[MAX_PATH];
	sprintf_s(libfile, MAX_PATH, "%s%s.slib", lib_path, fname);
	FILE* fp;
	errno_t err = fopen_s(&fp, libfile, "rb");
	if (err)
		printf("不能打开静态库文件");
	char* dllname;
	char* p;
	char line[400];
	int ret = -1;
	if (fp)
	{
		dllname = GetDllName(libfile);
		ArrayAdd(&arr_dll, dllname);
		while (1)
		{
			p = GetLine(line, sizeof(line), fp);
			if (NULL == p)
				break;
			else if ((0 == *p) || (';' == *p))
				continue;
			CoffSymAdd(sec_dynsymtab, p, arr_dll.count, sec_text->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
		}
		ret = 0;
		if (fp)
			fclose(fp);
	}
	else
	{
		LinkError("\" %s \" 文件打开失败 ", libfile);
	}
	return ret;
}

void GetEntryAddr(PEInfo* pe)
{

	unsigned long addr = 0;
	int cs = CoffSymSearch(sec_symtab, entry_symbol);
	CoffSym*  cfsym_entry = (CoffSym*)sec_symtab->data + cs;
	addr = cfsym_entry->Value;
	pe->entry_addr = addr;
}

void* GetLibPath(void)
{
	char path[MAX_PATH];
	char* p;
	GetModuleFileNameA(NULL, path, sizeof(path));
	p = strrchr(path, '\\');
	*p = '\0';
	strcat_s(path, MAX_PATH, "\\lib\\");
	return _strdup(path);
}

void AddRuntimeLibs(void)
{
	int i;
	int pe_type = 0;
	for (i = 0; i < arr_lib.count; ++i)
	{
		PeLoadLibFile(arr_lib.data[i]);
	}
}

int PeFindImport(char* symbol)
{
	int sym_index;
	sym_index = CoffSymSearch(sec_dynsymtab, symbol);
	return sym_index;
}

ImportSym* PeAddImport(PEInfo* pe, const int sym_index, const  char* name)
{
	if (name == NULL)
		Error("名字不能为空");
	CoffSym* psym = (CoffSym*)sec_dynsymtab->data + sym_index;
	int dll_index = psym->Value;
	if (0 == dll_index)
		return NULL;
	int i = ArrayFind(&pe->imps, dll_index);
	ImportInfo* p;
	ImportSym* s = NULL;
	if (-1 != i)
	{
		p = pe->imps.data[i];
	}
	else
	{
		p = MallocInit(sizeof(*p));
		ArrayInit(&p->imp_syms, 8);
		p->dll_index = dll_index;
		ArrayAdd(&pe->imps, p);
	}

	i = ArrayFind(&p->imp_syms, sym_index);
	if (-1 != i)
	{
		return (ImportSym*)p->imp_syms.data[i];
	}
	else
	{
		s = MallocInit(sizeof(ImportSym) + strlen(name));
		ArrayAdd(&p->imp_syms, s);
		strcpy_s((char*)&s->imp_sym.Name, strlen(name) + 1, name);
		return s;
	}
	return NULL;
}

int ResolveCoffsym(PEInfo* pe)
{
	if (pe == NULL)
		Error("指针未初始化");
	int sym_end = (sec_symtab->data_offset) / sizeof(CoffSym);
	int sym_index;
	CoffSym* psym = NULL;
	int found = 1;
	int ret = 0;
	for (sym_index = 1; sym_index < sym_end; sym_index++)
	{
		psym = (CoffSym*)sec_symtab->data + sym_index;

		if (psym->sSection == IMAGE_SYM_UNDEFINED)
		{
			char* name = sec_symtab->plink->data + psym->Name;

			unsigned type = psym->Type;
			int imp_sym = PeFindImport(name);
			ImportSym* is;
			if (0 == imp_sym)
				found = 0;
			is = PeAddImport(pe, imp_sym, name);
			if (!is)
				found = 0;

			if (found && type == CST_FUNC)
			{
				int offset = is->thk_offset;
				char buffer[100];
				offset = sec_text->data_offset;
				*(WORD*)SectionPtrAdd(sec_text, 6) = 0x25FF;

				sprintf_s(buffer, 100, "IAT.%s", name);
				is->iat_index = CoffSymAdd(sec_symtab, buffer, 0, sec_idata->index, CST_FUNC, IMAGE_SYM_CLASS_EXTERNAL);
				CoffRelocDirectAdd(offset + 2, is->iat_index, sec_text->index, IMAGE_REL_I386_DIR32);
				is->thk_offset = offset;

				psym = (CoffSym*)sec_symtab->data + sym_index;
				psym->Value = offset;
				psym->sSection = sec_text->index;
			}
			else
			{
				LinkError("'%s'未定义", name);
				ret = 1;
			}
		}
	}
	return ret;
}

int PutImportStr(Section* sec, const char* sym)
{
	int len = strlen(sym) + 1;
	int offset = sec->data_offset;
	char* ptr = SectionPtrAdd(sec, len);
	memcpy_s(ptr, len, sym, len);
	return offset;
}

void PeBuildImports(PEInfo* pe)
{

	DWORD rva_base = pe->thunk->sh.VirtualAddress - nt_header.OptionalHeader.ImageBase;
	int ndlls = pe->imps.count;

	int i, sym_cnt;
	for (sym_cnt = i = 0; i < ndlls; ++i)
		sym_cnt += ((ImportInfo*)pe->imps.data[i])->imp_syms.count;
	if (0 == sym_cnt)
		return;
	int dll_ptr = pe->thunk->data_offset;
	pe->imp_offs = dll_ptr;
	pe->imp_size = (ndlls + 1) * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	pe->iat_offs = dll_ptr + pe->imp_size;
	pe->iat_size = (sym_cnt + ndlls) * sizeof(DWORD);

	SectionPtrAdd(pe->thunk, pe->imp_size + (2 * pe->iat_size));

	int thk_ptr = pe->iat_offs;
	int ent_ptr = pe->iat_offs + pe->iat_size;

	for (i = 0; i < pe->imps.count; ++i)
	{
		ImportInfo* p = pe->imps.data[i];
		char* name = arr_dll.data[p->dll_index - 1];

		int v = PutImportStr(pe->thunk, name);

		p->imphdr.OriginalFirstThunk = ent_ptr + rva_base;
		p->imphdr.FirstThunk = thk_ptr + rva_base;
		p->imphdr.Name = v + rva_base;
		memcpy_s(pe->thunk->data + dll_ptr, sizeof(IMAGE_IMPORT_DESCRIPTOR), &p->imphdr, sizeof(IMAGE_IMPORT_DESCRIPTOR));

		int k;
		int n = p->imp_syms.count;
		for (k = 0; k <= n; ++k)
		{
			if (k < n)
			{
				ImportSym* is = (ImportSym*)p->imp_syms.data[k];
				DWORD iat_index = is->iat_index;
				CoffSym* org_sym = (CoffSym*)sec_symtab->data + iat_index;

				org_sym->Value = thk_ptr;
				org_sym->sSection = pe->thunk->index;
				v = pe->thunk->data_offset + rva_base;

				SectionPtrAdd(pe->thunk, sizeof(is->imp_sym.Hint));
				PutImportStr(pe->thunk, is->imp_sym.Name);
			}
			else
			{
				v = 0;
			}
			*(DWORD*)(pe->thunk->data + thk_ptr) = v;
			*(DWORD*)(pe->thunk->data + ent_ptr) = v;
			thk_ptr += sizeof(DWORD);
			ent_ptr += sizeof(DWORD);
		}
		dll_ptr += sizeof(IMAGE_IMPORT_DESCRIPTOR);
		ArrayFree(&p->imp_syms);
	}
	ArrayFree(&pe->imps);
}

int PeAssginAddress(PEInfo* pe)
{
	if (pe == NULL)
		Error("链接器中有指针未初始化");

	pe->thunk = sec_idata;
	pe->secs = (Section**)MallocInit(nsec_image * sizeof(Section*));

	DWORD addr = nt_header.OptionalHeader.ImageBase + 1;
	int i;
	Section* sec;
	Section** pps;
	for (i = 0; i < nsec_image; ++i)
	{
		sec = (Section*)sections.data[i];
		pps = &pe->secs[pe->sec_size];
		*pps = sec;

		addr = PeVirtualAlign(addr);
		sec->sh.VirtualAddress = addr;

		if (sec == pe->thunk)
			PeBuildImports(pe);
		if (sec->data_offset)
		{
			addr += sec->data_offset;
			++pe->sec_size;
		}
	}
	return 0;
}

void RelocateSyms(void)
{
	CoffSym* sym_end = (CoffSym*)(sec_symtab->data + sec_symtab->data_offset);
	CoffSym* sym;
	Section* sec;
	for (sym = (CoffSym*)(sec_symtab->data) + 1; sym < sym_end; ++sym)
	{
		sec = (Section*)sections.data[sym->sSection - 1];
		sym->Value += sec->sh.VirtualAddress;
	}
}

void CoffRelocsFixup(void)
{
	Section* sr = sec_rel;
	CoffReloc* rel_end = (CoffReloc*)(sr->data + sr->data_offset);
	CoffReloc* qrel = (CoffReloc*)sr->data;
	CoffReloc* rel = NULL;
	Section* sec = NULL;
	CoffSym* sym = NULL;
	char* name = NULL;
	char* ptr = NULL;
	int sym_index, type;
	unsigned long val, addr;
	for (rel = qrel; rel < rel_end; ++rel)
	{
		sec = (Section*)sections.data[rel->section - 1];

		sym_index = rel->cfsym;
		sym = &((CoffSym*)sec_symtab->data)[sym_index];
		name = sec_symtab->plink->data + sym->Name;
		val = sym->Value;
		addr = sec->sh.VirtualAddress + rel->offset;
		type = rel->type;

		ptr = sec->data + rel->offset;
		switch (type)
		{
			case IMAGE_REL_I386_DIR32:
			{
				*(int*)ptr += val;
				break;
			}
			case IMAGE_REL_I386_REL32:
			{
				*(int*)ptr += (val - addr);
				break;
			}
		}

	}
}

int PeWrite(PEInfo* pe)
{
	FILE* fp;
	errno_t err = fopen_s(&fp, pe->filename, "wb");
	if (err)
		Error("不能打开文件");
	if (fp == NULL)
	{
		LinkError("'%s'生成失败", pe->filename);
		return 1;
	}

	int size = sizeof(dos_header) + sizeof(dos_stub) + sizeof(nt_header) + (pe->sec_size * sizeof(IMAGE_SECTION_HEADER));
	int sizeofheaders = PeFileAlign(size);

	DWORD file_offset = sizeofheaders;
	Fpad(fp, file_offset);

	int i;
	for (i = 0; i < pe->sec_size; ++i)
	{
		Section* sec = pe->secs[i];
		char* sh_name = sec->sh.Name;
		unsigned long addr = sec->sh.VirtualAddress - nt_header.OptionalHeader.ImageBase;
		unsigned long size = sec->data_offset;
		IMAGE_SECTION_HEADER* psh = &sec->sh;

		if (!strcmp(sec->sh.Name, ".text"))
		{
			nt_header.OptionalHeader.BaseOfCode = addr;
			nt_header.OptionalHeader.AddressOfEntryPoint = addr + pe->entry_addr;
		}
		else if (!strcmp(sec->sh.Name, ".data"))
			nt_header.OptionalHeader.BaseOfData = addr;
		else if (!strcmp(sec->sh.Name, ".idata"))
		{
			if (pe->imp_size)
			{
				PeSetDatadir(IMAGE_DIRECTORY_ENTRY_IMPORT, pe->imp_offs + addr, pe->imp_size);
				PeSetDatadir(IMAGE_DIRECTORY_ENTRY_IAT, pe->iat_offs + addr, pe->iat_size);
			}
		}

		strcpy_s((char*)psh->Name, IMAGE_SIZEOF_SHORT_NAME, sh_name);
		psh->VirtualAddress = addr;
		psh->Misc.VirtualSize = size;
		nt_header.OptionalHeader.SizeOfImage = PeVirtualAlign(size + addr);

		DWORD r;
		if (sec->data_offset)
		{
			r = file_offset;
			psh->PointerToRawData = r;
			if (!strcmp(sec->sh.Name, ".bss"))
			{
				sec->sh.SizeOfRawData = 0;
				continue;
			}
			fwrite(sec->data, 1, sec->data_offset, fp);
			file_offset = PeFileAlign(file_offset + sec->data_offset);
			psh->SizeOfRawData = file_offset - r;
			Fpad(fp, file_offset);
		}
	}

	nt_header.FileHeader.NumberOfSections = pe->sec_size;
	nt_header.OptionalHeader.SizeOfHeaders = sizeofheaders;

	nt_header.OptionalHeader.Subsystem = subsystem;
	fseek(fp, SEEK_SET, 0);
	fwrite(&dos_header, 1, sizeof(dos_header), fp);
	fwrite(&dos_stub, 1, sizeof(dos_stub), fp);
	fwrite(&nt_header, 1, sizeof(nt_header), fp);

	for (i = 0; i < pe->sec_size; ++i)
		fwrite(&pe->secs[i]->sh, 1, sizeof(IMAGE_SECTION_HEADER), fp);
	fclose(fp);
	return 0;
}

int PeOutputFile(const char* filename)
{
	if (filename == NULL)
	{
		Error("名字不能为空");
	}
	PEInfo pe;

	memset(&pe, 0, sizeof(pe));
	ArrayInit(&pe.imps, 8);

	pe.filename = filename;
	AddRuntimeLibs();
	GetEntryAddr(&pe);

	int ret = ResolveCoffsym(&pe);
	if (0 == ret)
	{

		PeAssginAddress(&pe);

		RelocateSyms();

		CoffRelocsFixup();

		ret = PeWrite(&pe);
		free(pe.secs);
	}

	return ret;
}