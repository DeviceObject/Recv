#include "stdafx.h"
#include "Utils.h"

LPWSTR UTF8_2_UTF16(LPSTR strSource)
{
	DWORD dwLen;
	LPWSTR strDest;

	if ((dwLen = MultiByteToWideChar(CP_UTF8,0,strSource,-1,NULL,0)) == 0)
	{
		return NULL;
	}

	if (!(strDest = (WCHAR *)LocalAlloc(LPTR,dwLen * sizeof(WCHAR))))
	{
		return NULL;
	}
	if (MultiByteToWideChar(CP_UTF8,0,strSource,-1,strDest,dwLen) == 0)
	{
		LocalFree(strDest);
		return NULL;
	}
	return strDest;
}
LPSTR UnicodeToANSI(LPWSTR str)
{
	DWORD dwLen;
	LPSTR strDest;
	// wide char to multi char
	dwLen = WideCharToMultiByte(CP_ACP,
		0,
		str,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	strDest = (LPSTR)LocalAlloc(LPTR,dwLen);
	::WideCharToMultiByte(CP_ACP,
		0,
		str,
		-1,
		strDest,
		dwLen,
		NULL,
		NULL );
	return strDest;
}
ULONG MyGetFileSize(WCHAR *wFilePath)
{
	HANDLE hFile;
	ULONG_PTR ulRetByteSize;
	hFile = CreateFile(wFilePath, \
		GENERIC_READ, \
		FILE_SHARE_READ | FILE_SHARE_WRITE, \
		NULL, \
		OPEN_EXISTING, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}
	ulRetByteSize = GetFileSize(hFile,NULL);
	CloseHandle(hFile);
	return ulRetByteSize;
}
ULONG SetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength)
{
	HANDLE hFile;
	ULONG_PTR ulRetByteSize;
	hFile = CreateFile(wFilePath, \
		GENERIC_READ | GENERIC_WRITE, \
		FILE_SHARE_READ | FILE_SHARE_WRITE, \
		NULL, \
		OPEN_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}
	SetFilePointer(hFile,ulOffset,NULL,FILE_BEGIN);
	if (WriteFile(hFile,pDatBuf,ulLength,&ulRetByteSize,NULL) == FALSE)
	{
		CloseHandle(hFile);
		return 0;
	}
	CloseHandle(hFile);
	return ulRetByteSize;
}
ULONG GetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength)
{
	HANDLE hFile;
	ULONG_PTR ulRetByteSize;
	hFile = CreateFile(wFilePath, \
		GENERIC_READ, \
		FILE_SHARE_READ | FILE_SHARE_WRITE, \
		NULL, \
		OPEN_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return 0;
	}
	SetFilePointer(hFile,ulOffset,NULL,FILE_BEGIN);
	if (ReadFile(hFile,pDatBuf,ulLength,&ulRetByteSize,NULL) == FALSE)
	{
		CloseHandle(hFile);
		return 0;
	}
	CloseHandle(hFile);
	return ulRetByteSize;
}