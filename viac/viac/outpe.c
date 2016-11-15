#include "viac.h"

char* entry_symbol = "_entry";
DynArray arr_dll;
DynArray arr_lib;
char* lib_path;
short subsystem;

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

DWORD PeFileAlign(DWORD addr)
{
	DWORD FileAlignment = nt_header.OptionalHeader.FileAlignment;
	return ClacAlign();
}