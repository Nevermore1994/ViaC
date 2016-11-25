#include"viac.h"

#ifndef		OUTPE_H_
#define 	OUTPE_H_


/*************************outpe.h******/
typedef struct ImportSym
{
	int iat_index;
	int thk_offset;
	IMAGE_IMPORT_BY_NAME imp_sym;
}ImportSym;

/* 导入模块内存存储结构 */
typedef struct ImportInfo
{
	int dll_index;
	Array imp_syms;
	IMAGE_IMPORT_DESCRIPTOR imphdr;
}ImportInfo;

/* PE信息存储结构 */
typedef struct PEInfo
{
	Section *thunk;
	const char *filename;
	DWORD entry_addr;
	DWORD imp_offs;
	DWORD imp_size;
	DWORD iat_offs;
	DWORD iat_size;
	Section **secs;
	int   sec_size;
	Array imps;
}PEInfo;

extern char* entry_symbol;
extern Array arr_dll;
extern Array arr_lib;
extern char* lib_path;
extern short subsystem;

DWORD PeFileAlign(const DWORD addr);
DWORD PeVirtualAlign(const DWORD addr);
void PeSetDatadir(const int dir, const DWORD addr, const DWORD size);
int LoadObjFile(const char* fname);
char* GetLine(char* line, const int size, const FILE* fp);
int PeLoadLibFile(char* fname);
void GetEntryAddr(PEInfo* pe);
void* GetLibPath();
void AddRuntimeLibs();
int PeFindImport(char* symbol);
ImportSym* PeAddImport(PEInfo* pe, const int sym_index, const  char* name);
int ResolveCoffsym(PEInfo* pe);
int PutImportStr(Section* sec, const char* sym);
void PeBuildImports(PEInfo* pe);
int PeAssginAddress(PEInfo* pe);
void RelocateSyms();
void CoffRelocsFixup();
int PeWrite(PEInfo* pe);
int PeOutputFile(const char* filename);
char* GetDllName(const char* libfile);


#endif