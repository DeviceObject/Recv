// PassXsEver.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rsa.h"
#include "base64.h"

LIST_ENTRY g_RecvFileList;
CRITICAL_SECTION g_CriticalSection;
WCHAR g_wCurPath[MAX_PATH];
#define SERVER_PORT 4040
#define BACKLOG 10

void SetBlackGreen()
{
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);  
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return;
}
ULONG MyGetFileSize(WCHAR *wFilePath)
{
	HANDLE hFile;
	PCHAR pDat;
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
int _tmain(int argc, _TCHAR* argv[])
{
	SOCKADDR_IN LocalAddr;
	SOCKADDR_IN ServerAddr;
	SOCKET ListenSocket;
	SOCKET tmpSocket,ClientSocket,ServerSocket;
	WSADATA WSAData;
	DWORD dwTid = 0;
	ULONG_PTR ulOffset,ulRsaLength,ulDatLength;
	PRECV_FILE_DAT pRecvFileDat;
	PRECV_FILE_INFO pRecvFileInfo,pRecvWrite;
	PLIST_ENTRY pCurRecvWrite,pCurRandomBlock;
	PRECV_RANDOM_BLOCK pRecvRandomBlock;
	PRANDOM_BLOCK pRandomBlock,pWriteRandomBlock;
	PCHAR pDencryptRsa,pDencryptBase64,pSrcData;
	char szRecvDat[1024];
	InitializeListHead(&g_RecvFileList);
	InitializeCriticalSection(&g_CriticalSection);
	RtlZeroMemory(g_wCurPath,sizeof(WCHAR) * MAX_PATH);

	GetCurrentDirectory(MAX_PATH,g_wCurPath);
	StringCchCatW(g_wCurPath,MAX_PATH,L"\\Cache\\");
	if (!PathFileExists(g_wCurPath))
	{
		_wmkdir(g_wCurPath);
	}
	SetBlackGreen();
	printf("=====================================================================\n");
	printf("Start...\n");
	printf("Initialize socket.....\n");
	if(WSAStartup(MAKEWORD(2,2), &WSAData))
	{
		printf("initializationing error!\n");
		WSACleanup();
		exit(0);
	}
	printf("Initialize socket done...\n");

	ListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(SERVER_PORT);
	LocalAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(ListenSocket,(sockaddr* )&LocalAddr,sizeof(sockaddr));

	listen(ListenSocket,BACKLOG);
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DelRecvList,NULL,0,NULL);
	while (TRUE)
	{
		int nSize = sizeof(struct sockaddr_in);
		tmpSocket = accept(ListenSocket,(sockaddr *)&ServerAddr,&nSize);
		if (tmpSocket == -1)
		{
			printf("accept failed....\n");
		}
		while (TRUE)
		{
			do 
			{
				pRecvFileDat = (PRECV_FILE_DAT)LocalAlloc(LPTR,sizeof(RECV_FILE_DAT));
			} while (NULL == pRecvFileDat);
			if (recv(tmpSocket,(char*)pRecvFileDat,sizeof(RECV_FILE_DAT),0) == -1)
			{
				continue;
			}
			if (pRecvFileDat->CmdLine.ulCommand == COMMAND_REVICE_FILE)
			{
				printf("Start recv files\n");
				do 
				{
					pRecvFileInfo = (PRECV_FILE_INFO)LocalAlloc(LPTR,sizeof(RECV_FILE_INFO));
				} while (NULL == pRecvFileInfo);
				pRecvFileInfo->bIsWrited = FALSE;
				pRecvFileInfo->bIsCrypt = pRecvFileDat->bIsCrypt;
				pRecvFileInfo->ulFileSize = pRecvFileDat->ulFileSize;
				pRecvFileInfo->ulRandomCount = pRecvFileDat->ulRandomCount;
				memcpy(pRecvFileInfo->FileHash,pRecvFileDat->FileHash,32);
				StringCchCopy(pRecvFileInfo->wFileName,MAX_PATH,pRecvFileDat->wFileName);
				GetCurrentDirectory(MAX_PATH,pRecvFileInfo->wFileFullPath);
				StringCchCat(pRecvFileInfo->wFileFullPath,MAX_PATH,L"\\RecvFile\\");
				StringCchCat(pRecvFileInfo->wFileFullPath,MAX_PATH,pRecvFileDat->wFileName);
				InitializeListHead(&pRecvFileInfo->List);
				InitializeListHead(&pRecvFileInfo->RandomBlockList);
				//LocalFree(pRecvFileDat);
				printf("File name : %ws\r\n",pRecvFileInfo->wFileName);
				printf("File size : %d  File random module : %d\n",pRecvFileInfo->ulFileSize,pRecvFileInfo->ulRandomCount);
				printf("File hash : %s\n",pRecvFileInfo->FileHash);
				for (int i = 0;i < pRecvFileInfo->ulRandomCount;i++)
				{
					do 
					{
						pRecvRandomBlock = (PRECV_RANDOM_BLOCK)LocalAlloc(LPTR,sizeof(RECV_RANDOM_BLOCK));
					} while (NULL == pRecvRandomBlock);
					if (recv(tmpSocket,(char *)pRecvRandomBlock,sizeof(RECV_RANDOM_BLOCK),0) == -1)
					{
						continue;
					}
					do 
					{
						pRandomBlock = (PRANDOM_BLOCK)LocalAlloc(LPTR,sizeof(RANDOM_BLOCK));
					} while (NULL == pRandomBlock);
					pRandomBlock->ulCryptDatLength = pRecvRandomBlock->ulCryptDatLength;
					pRandomBlock->ulOffset = pRecvRandomBlock->ulOffset;
					pRandomBlock->ulIndex = pRecvRandomBlock->ulIndex;
					printf("Random block offset : %d   Random block size : %d\r\n",pRandomBlock->ulOffset,pRandomBlock->ulCryptDatLength);
					do 
					{
						pRandomBlock->pCryptDat = (PCHAR)LocalAlloc(LPTR,pRandomBlock->ulCryptDatLength);
					} while (NULL == pRandomBlock->pCryptDat);
					if (recv(tmpSocket,szRecvDat,pRandomBlock->ulCryptDatLength,0) == -1)
					{
						continue;
					}
					memcpy(pRandomBlock->pCryptDat,szRecvDat,pRandomBlock->ulCryptDatLength);
					InsertTailList(&pRecvFileInfo->RandomBlockList,&pRandomBlock->ListRandomBlock);
					LocalFree(pRecvRandomBlock);
				}
				EnterCriticalSection(&g_CriticalSection);
				InsertTailList(&g_RecvFileList,&pRecvFileInfo->List);
				LeaveCriticalSection(&g_CriticalSection);
			}
			else if (pRecvFileDat->CmdLine.ulCommand == COMMAND_OVER)
			{
			}
			else if (pRecvFileDat->CmdLine.ulCommand == COMMAND_COMPLETE)
			{
				closesocket(tmpSocket);
				pCurRecvWrite = g_RecvFileList.Flink;
				if (!IsListEmpty(pCurRecvWrite))
				{
					while (pCurRecvWrite != &g_RecvFileList)
					{
						pRecvFileInfo = (PRECV_FILE_INFO)pCurRecvWrite;
						if (!IsListEmpty(&pRecvFileInfo->RandomBlockList))
						{
							for (int i = 0;i < pRecvFileInfo->ulRandomCount;i++)
							{
								pCurRandomBlock = pRecvFileInfo->RandomBlockList.Flink;
								while (pCurRandomBlock != &pRecvFileInfo->RandomBlockList && !IsListEmpty(pCurRandomBlock))
								{
									pWriteRandomBlock = CONTAINING_RECORD(pCurRandomBlock,RANDOM_BLOCK,ListRandomBlock);
									if (pWriteRandomBlock)
									{
										if (pWriteRandomBlock->ulIndex == i)
										{
											SetFileDat(pRecvFileInfo->wFileFullPath, \
												pWriteRandomBlock->pCryptDat, \
												pWriteRandomBlock->ulOffset, \
												pWriteRandomBlock->ulCryptDatLength);
											//LocalFree(pWriteRandomBlock->pCryptDat);
											//EnterCriticalSection(&g_CriticalSection);
											//RemoveEntryList(&pWriteRandomBlock->ListRandomBlock);
											//LeaveCriticalSection(&g_CriticalSection);
											//LocalFree(pWriteRandomBlock);
										}
									}
									pCurRandomBlock = pCurRandomBlock->Flink;
								}
							}
						}
						EnterCriticalSection(&g_CriticalSection);
						pRecvFileInfo->ulFileSize = MyGetFileSize(pRecvFileInfo->wFileFullPath);
						LeaveCriticalSection(&g_CriticalSection);
						
						do 
						{
							pDencryptRsa = (PCHAR)LocalAlloc(LPTR,pRecvFileInfo->ulFileSize);
						} while (NULL == pDencryptRsa);
						if (GetFileDat(pRecvFileInfo->wFileFullPath,pDencryptRsa,0,pRecvFileInfo->ulFileSize))
						{
							ulOffset = 0;
							DeleteFile(pRecvFileInfo->wFileFullPath);
							while (TRUE)
							{
								if (ulOffset > pRecvFileInfo->ulFileSize)
								{
									break;
								}
								ulRsaLength = MyRSADecrypt((char *)((ULONG_PTR)pDencryptRsa + ulOffset),&pDencryptBase64);
								if (ulRsaLength)
								{
									SetFileDat(pRecvFileInfo->wFileFullPath,pDencryptBase64,ulOffset,ulRsaLength);
									ulOffset += ulRsaLength;
								}
							}
							LocalFree(pDencryptRsa);
							EnterCriticalSection(&g_CriticalSection);
							pRecvFileInfo->ulFileSize = MyGetFileSize(pRecvFileInfo->wFileFullPath);
							LeaveCriticalSection(&g_CriticalSection);
							do 
							{
								pDencryptBase64 = (PCHAR)LocalAlloc(LPTR,pRecvFileInfo->ulFileSize);
							} while (NULL == pDencryptBase64);
							if (GetFileDat(pRecvFileInfo->wFileFullPath,pDencryptBase64,0,pRecvFileInfo->ulFileSize))
							{
								DeleteFile(pRecvFileInfo->wFileFullPath);
								pSrcData = (PCHAR)base64_decode(pDencryptBase64,pRecvFileInfo->ulFileSize,(int *)&ulDatLength);
								if (pSrcData)
								{
									SetFileDat(pRecvFileInfo->wFileFullPath,pSrcData,0,ulDatLength);
								}
							}
						}
						EnterCriticalSection(&g_CriticalSection);
						pRecvFileInfo->bIsWrited = TRUE;
						LeaveCriticalSection(&g_CriticalSection);
						pCurRecvWrite = pCurRecvWrite->Flink;
					}
				}
				break;
			}
			else
			{

			}
			LocalFree(pRecvFileDat);
		}
	}
	WSACleanup();
	return 0;
}
DWORD DelRecvList(LPVOID lpParameter)
{
	PRECV_FILE_INFO pRecvFileInfo;
	PLIST_ENTRY pCurListEntry,pCurRandomBlock,pRemovedList;
	PRANDOM_BLOCK pRandomBlock;

	while (TRUE)
	{
		if (!IsListEmpty(&g_RecvFileList))
		{
			pCurListEntry = g_RecvFileList.Flink;
			while (pCurListEntry != &g_RecvFileList)
			{
				pRecvFileInfo = CONTAINING_RECORD(pCurListEntry,RECV_FILE_INFO,List);
				if (pRecvFileInfo && pRecvFileInfo->bIsWrited == TRUE)
				{
					if (!IsListEmpty(&pRecvFileInfo->RandomBlockList))
					{
						//pRandomBlock = (PRANDOM_BLOCK)pRecvFileInfo->RandomBlockList.Flink;
						pCurRandomBlock = pRecvFileInfo->RandomBlockList.Flink;

						while (pCurRandomBlock != &pRecvFileInfo->RandomBlockList)
						{
							pRandomBlock = CONTAINING_RECORD(pCurRandomBlock,RANDOM_BLOCK,ListRandomBlock);
							if (pRandomBlock)
							{
								if (pRandomBlock->pCryptDat)
								{
									LocalFree(pRandomBlock->pCryptDat);
									pRandomBlock->pCryptDat = NULL;
								}
								EnterCriticalSection(&g_CriticalSection);
								pRemovedList = RemoveHeadList(&pRandomBlock->ListRandomBlock);
								LeaveCriticalSection(&g_CriticalSection);
								if (pRemovedList)
								{
									//LocalFree((char*)pRemovedList);
									pRemovedList = NULL;
								}
							}
							pCurRandomBlock = pCurRandomBlock->Flink;
						}
					}
					EnterCriticalSection(&g_CriticalSection);
					RemoveEntryList(&pRecvFileInfo->List);
					LeaveCriticalSection(&g_CriticalSection);
				}
				pCurListEntry = pCurListEntry->Flink;
			}
		}
	}
	
	return 0;
}
