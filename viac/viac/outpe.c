#include "viac.h"

char* entry_symbol = "_entry";
DynArray arr_dll;
DynArray arr_lib;
char* lib_path;
short subsystem;

IMAGE_DOS_HEADER dos_header = {
	/* IMAGE_DOS_HEADER doshdr */
	0x5A4D, /*WORD e_magic;         DOS��ִ���ļ����,Ϊ'MZ'  */
	0x0090, /*WORD e_cblp;          Bytes on last page of file */
	0x0003, /*WORD e_cp;            Pages in file */
	0x0000, /*WORD e_crlc;          Relocations */

	0x0004, /*WORD e_cparhdr;       Size of header in paragraphs */
	0x0000, /*WORD e_minalloc;      Minimum extra paragraphs needed */
	0xFFFF, /*WORD e_maxalloc;      Maximum extra paragraphs needed */
	0x0000, /*WORD e_ss;            DOS����ĳ�ʼ����ջ�� */

	0x00B8, /*WORD e_sp;            DOS����ĳ�ʼ����ջָ�� */
	0x0000, /*WORD e_csum;          Checksum */
	0x0000, /*WORD e_ip;            DOS��������IP */
	0x0000, /*WORD e_cs;            DOS��������CS */
	0x0040, /*WORD e_lfarlc;        File address of relocation table */
	0x0000, /*WORD e_ovno;          Overlay number */
	{ 0,0,0,0 }, /*WORD e_res[4];     Reserved words */
	0x0000, /*WORD e_oemid;         OEM identifier (for e_oeminfo) */
	0x0000, /*WORD e_oeminfo;       OEM information; e_oemid specific */
	{ 0,0,0,0,0,0,0,0,0,0 }, /*WORD e_res2[10];      Reserved words */
	0x00000080  /*DWORD   e_lfanew;        ָ��PE�ļ�ͷ */
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
	0x00004550, /* DWORD Signature = IMAGE_NT_SIGNATURE PE�ļ���ʶ*/
	{
		/* IMAGE_FILE_HEADER FileHeader */
		0x014C,		/*WORD    Machine; ����ƽ̨*/
		0x0003,		/*WORD    NumberOfSections; �ļ��Ľ���Ŀ */
	0x00000000, /*DWORD   TimeDateStamp; �ļ��Ĵ������ں�ʱ��*/
	0x00000000, /*DWORD   PointerToSymbolTable; ָ����ű�(���ڵ���)*/
	0x00000000, /*DWORD   NumberOfSymbols; ���ű��еķ������������ڵ��ԣ�*/
	0x00E0,		/*WORD    SizeOfOptionalHeader; IMAGE_OPTIONAL_HEADER32�ṹ�ĳ���*/
	0x030F		/*WORD    Characteristics; �ļ�����*/
	},
	{
			/* IMAGE_OPTIONAL_HEADER OptionalHeader */
			/* ��׼��. */
			0x010B, /*WORD    Magic; */
			0x06, /*BYTE    MajorLinkerVersion; ���������汾��*/
			0x00, /*BYTE    MinorLinkerVersion; �������ΰ汾��*/
			0x00000000, /*DWORD   SizeOfCode; ���к�����ε��ܴ�С*/
			0x00000000, /*DWORD   SizeOfInitializedData; �����ѳ�ʼ�����ݶε��ܴ�С*/
			0x00000000, /*DWORD   SizeOfUninitializedData; ���к���δ��ʼ�����ݶεĴ�С*/
			0x00000000, /*DWORD   AddressOfEntryPoint; ����ִ����ڵ���������ַ*/
			0x00000000, /*DWORD   BaseOfCode; ����ε���ʼRVA*/
			0x00000000, /*DWORD   BaseOfData; ����ε���ʼRVA*/

			/* NT������ */
			0x00400000, /*DWORD   ImageBase; ����Ľ���װ�ص�ַ ��һ��Windows�ڴ�ṹ���������0x80000000����ʲô���*/
			0x00001000, /*DWORD   SectionAlignment; �ڴ��жεĶ�������*/
			0x00000200, /*DWORD   FileAlignment; �ļ��жεĶ�������*/
			0x0004, /*WORD    MajorOperatingSystemVersion; ����ϵͳ�����汾��*/
			0x0000, /*WORD    MinorOperatingSystemVersion; ����ϵͳ�Ĵΰ汾��*/
			0x0000, /*WORD    MajorImageVersion; ��������汾��*/
			0x0000, /*WORD    MinorImageVersion; ����Ĵΰ汾��*/
			0x0004, /*WORD    MajorSubsystemVersion; ��ϵͳ�����汾��*/
			0x0000, /*WORD    MinorSubsystemVersion; ��ϵͳ�Ĵΰ汾��*/
			0x00000000, /*DWORD   Win32VersionValue; ��������Ϊ0*/
			0x00000000, /*DWORD   SizeOfImage; �ڴ�������PEӳ��ߴ�*/
			0x00000200, /*DWORD   SizeOfHeaders; ����ͷ+�ڱ�Ĵ�С*/
			0x00000000, /*DWORD   CheckSum; У���*/
			0x0003, /*WORD    Subsystem; �ļ�����ϵͳ*/
			0x0000, /*WORD    DllCharacteristics; */
			0x00100000, /*DWORD   SizeOfStackReserve; ��ʼ��ʱ��ջ��С*/
			0x00001000, /*DWORD   SizeOfStackCommit; ��ʼ��ʱʵ���ύ�Ķ�ջ��С*/
			0x00100000, /*DWORD   SizeOfHeapReserve; ��ʼ��ʱ�����ĶѴ�С*/
			0x00001000, /*DWORD   SizeOfHeapCommit; ��ʼ��ʱʵ���ύ�ĶѴ�С*/
			0x00000000, /*DWORD   LoaderFlags; ��������Ϊ0*/
			0x00000010, /*DWORD   NumberOfRvaAndSizes; ���������Ŀ¼�ṹ������*/

						/* IMAGE_DATA_DIRECTORY DataDirectory[16]; ����Ŀ¼*/
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