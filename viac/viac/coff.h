#include"viac.h"

#ifndef		COFF_H_
#define 	COFF_H_


/********************coff***********************/
#pragma pack(push,1)
typedef struct Section
{
	int data_offset;
	int data_allocated;
	char* data;
	char index;
	struct Section * plink;
	int* hashtab;
	IMAGE_SECTION_HEADER  sh;//½ÚÍ·
}Section;

typedef struct CoffSym
{
	DWORD Name;
	DWORD Next;

	DWORD Value;
	short sSection;
	WORD Type;
	BYTE StorageClass;
	BYTE NumberOfAuxSymbols;
}CoffSym;

#define CST_FUNC 0x20
#define CST_NOFUNC 0

typedef struct CoffReloc
{
	DWORD offset;
	DWORD cfsym;
	BYTE  section;
	BYTE  type;
}CoffReloc;

#pragma pack(pop)
typedef Section* pSection;
extern Array sections;

extern pSection sec_text, sec_data, sec_bss, sec_idata, sec_rdata, sec_rel, sec_symtab, sec_dynsymtab;

extern int nsec_image;

void SectionRealloc(pSection sec, const int newsize);
void*  SectionPtrAdd(pSection sec, const int increment);
Section* SectionNew(const char* name, const int characteristics);
int CoffSymSearch(const pSection symtab, const char* name);
char* CoffStrAdd(const pSection strtab, const char* name);
int CoffSymAdd(pSection symtab, const char* name, const int val, const int sec_index, const short type, const char StrorageClass);
void CoffSymAddUpdate(Symbol* ps, const int val, const int sec_index, const short type, const char StroageClass);
void FreeSection(void);
Section* NewCoffSymSection(char* symtab_name, const int Characteristics, char* strtab_name);
void CoffRelocDirectAdd(const int offset, const int cfsym, const char section, const char type);
void CoffRelocAdd(pSection sec, Symbol* sym, const int offset, const char type);
void InitCoff(void);
void Fpad(FILE* fp, const int new_pos);
void WriteObj(const char* name);
/*********************end**************************/

#endif