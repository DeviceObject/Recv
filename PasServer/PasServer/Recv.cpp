#include "stdafx.h"
#include "Utils.h"
#include "base64.h"
#include "rsa.h"
#include "Recv.h"

LIST_ENTRY g_RecvFileList;
LIST_ENTRY g_RecvSocket;
CRITICAL_SECTION g_CriticalSection;
CRITICAL_SECTION g_CriticalSection_Socket;
BOOLEAN g_bInitSocket;
ULONG g_ulRecvThreadCount;
WCHAR g_wCurPath[MAX_PATH];
WCHAR g_wRecvPath[MAX_PATH];
WCHAR g_wMoveFilePath[MAX_PATH];

DWORD RecvWorkThread(LPVOID lpParameter)
{
	PRECV_SOCKET pRecvSock;

	if (lpParameter == NULL)
	{
		return -1;
	}
	pRecvSock = (PRECV_SOCKET)lpParameter;
	if (Commander(pRecvSock))
	{
		if (pRecvSock)
		{
			LocalFree(pRecvSock);
		}
	}
	return 0;
}
ULONG_PTR DecodeFile(PRECV_FILE_INFO pRecvFileInfo)
{
	ULONG_PTR ulOffset,ulRsaLength,ulDatLength;
	PCHAR pDencryptRsa,pDencryptBase64,pSrcData;

	if (!pRecvFileInfo)
	{
		return 0;
	}
	ulDatLength = 0;
	pRecvFileInfo->ulFileSize = MyGetFileSize(pRecvFileInfo->wFileFullPath);
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
		pRecvFileInfo->ulFileSize = MyGetFileSize(pRecvFileInfo->wFileFullPath);
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
	return ulDatLength;
}
ULONG_PTR FixRecvFiles(PRECV_FILE_INFO pRecvFileInfo)
{
	ULONG_PTR uli;
	PLIST_ENTRY pCurRandomBlock;
	PRANDOM_BLOCK pWriteRandomBlock;
	if (pRecvFileInfo && !IsListEmpty(&pRecvFileInfo->RandomBlockList))
	{
		for (uli = 0;uli < pRecvFileInfo->ulRandomCount;uli++)
		{
			pCurRandomBlock = pRecvFileInfo->RandomBlockList.Flink;
			while (pCurRandomBlock != &pRecvFileInfo->RandomBlockList && !IsListEmpty(pCurRandomBlock))
			{
				pWriteRandomBlock = CONTAINING_RECORD(pCurRandomBlock,RANDOM_BLOCK,ListRandomBlock);
				if (pWriteRandomBlock)
				{
					if (pWriteRandomBlock->ulIndex == uli)
					{
						SetFileDat(pRecvFileInfo->wFileFullPath, \
							pWriteRandomBlock->pCryptDat, \
							pWriteRandomBlock->ulOffset, \
							pWriteRandomBlock->ulCryptDatLength);
					}
				}
				pCurRandomBlock = pCurRandomBlock->Flink;
			}
		}
	}
	return uli;
}
ULONG_PTR RecvFileInfo(PRECV_FILE_DAT pRecvFileDat,PRECV_SOCKET pRecvSocket,PRECV_FILE_INFO pRecvFileInfo)
{
	PRANDOM_BLOCK pRandomBlock;
	PRECV_RANDOM_BLOCK pRecvRandomBlock;
	ULONG_PTR ulRet;

	ulRet = 0;
	if (pRecvFileDat == NULL)
	{
		return ulRet;
	}

	pRecvFileInfo->bIsWrited = FALSE;
	pRecvFileInfo->bIsCrypt = pRecvFileDat->bIsCrypt;
	pRecvFileInfo->ulFileSize = pRecvFileDat->ulFileSize;
	pRecvFileInfo->ulRandomCount = pRecvFileDat->ulRandomCount;
	memcpy(pRecvFileInfo->FileHash,pRecvFileDat->FileHash,32);
	StringCchCopy(pRecvFileInfo->wFileName,MAX_PATH,pRecvFileDat->wFileName);
	GetCurrentDirectory(MAX_PATH,pRecvFileInfo->wFileFullPath);
	StringCchCat(pRecvFileInfo->wFileFullPath,MAX_PATH,L"\\RecvFiles\\");
	StringCchCat(pRecvFileInfo->wFileFullPath,MAX_PATH,pRecvFileDat->wFileName);
	InitializeListHead(&pRecvFileInfo->List);
	InitializeListHead(&pRecvFileInfo->RandomBlockList);
	printf("File name : %ws\r\n",pRecvFileInfo->wFileName);
	printf("File size : %d  File random module : %d\n",pRecvFileInfo->ulFileSize,pRecvFileInfo->ulRandomCount);
	printf("File hash : %s\n",pRecvFileInfo->FileHash);
	for (int i = 0;i < pRecvFileInfo->ulRandomCount;i++)
	{
		do 
		{
			pRecvRandomBlock = (PRECV_RANDOM_BLOCK)LocalAlloc(LPTR,sizeof(RECV_RANDOM_BLOCK));
		} while (NULL == pRecvRandomBlock);
		if (recv(pRecvSocket->socket,(char *)pRecvRandomBlock,sizeof(RECV_RANDOM_BLOCK),0) == -1)
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
		if (recv(pRecvSocket->socket,pRandomBlock->pCryptDat,pRandomBlock->ulCryptDatLength,0) == -1)
		{
			continue;
		}
		InsertTailList(&pRecvFileInfo->RandomBlockList,&pRandomBlock->ListRandomBlock);
		ulRet++;
		LocalFree(pRecvRandomBlock);
	}
	//EnterCriticalSection(&g_CriticalSection);
	//InsertTailList(&g_RecvFileList,&pRecvFileInfo->List);
	//LeaveCriticalSection(&g_CriticalSection);

	EnterCriticalSection(&g_CriticalSection_Socket);
	pRecvSocket->bIsWork = FALSE;
	pRecvSocket->bIsOk = TRUE;
	LeaveCriticalSection(&g_CriticalSection_Socket);
	return ulRet;
}
ULONG_PTR Commander(PRECV_SOCKET pRecvSocket)
{
	ULONG_PTR ulRet;
	PRECV_FILE_DAT pRecvFileDat;
	PRECV_FILE_INFO pRecvFileInfo;
	char szRecvDat[1024];

	RtlZeroMemory(szRecvDat,sizeof(char) * 1024);
	ulRet = 0;
	if (recv(pRecvSocket->socket,(char*)szRecvDat,sizeof(RECV_FILE_DAT),0) == -1)
	{
		return ulRet;
	}
	pRecvFileDat = (PRECV_FILE_DAT)szRecvDat;
	switch(pRecvFileDat->CmdLine.ulCommand)
	{
	case COMMAND_RUN_EXE:
		break;
	case COMMAND_RUN_DLL:
		break;
	case COMMAND_REVICE_FILE:
		do 
		{
			pRecvFileInfo = (PRECV_FILE_INFO)LocalAlloc(LPTR,sizeof(RECV_FILE_INFO));
		} while (NULL == pRecvFileInfo);
		ulRet = RecvFileInfo(pRecvFileDat,pRecvSocket,pRecvFileInfo);
		if (ulRet)
		{
			ulRet = FixRecvFiles(pRecvFileInfo);
			if (ulRet)
			{
				ulRet = DecodeFile(pRecvFileInfo);
			}
		}
		if (pRecvFileInfo)
		{
			LocalFree(pRecvFileInfo);
		}
		break;
	case COMMAND_SEND_FILE:
		break;
	case COMMAND_COMPLETE:
		break;
	case COMMAND_OVER:
		break;
	default:
		break;

	}
	return ulRet;
}
DWORD RecvData(LPVOID lpParameter)
{
	ULONG_PTR ulOffset,ulRsaLength,ulDatLength;
	PRECV_FILE_DAT pRecvFileDat;
	PRECV_FILE_INFO pRecvFileInfo;
	PLIST_ENTRY pCurRecvWrite,pCurRandomBlock;
	PRECV_RANDOM_BLOCK pRecvRandomBlock;
	PRANDOM_BLOCK pRandomBlock,pWriteRandomBlock;
	PCHAR pDencryptRsa,pDencryptBase64,pSrcData;
	PRECV_SOCKET pRecvSocket,pWorkSocket;
	PLIST_ENTRY pCurSocket,pReleaseSocket;
	char szRecvDat[1024];

	if (lpParameter)
	{
		pWorkSocket = (PRECV_SOCKET)lpParameter;
	}
	printf("Work thread num is : %08x\r\n",g_ulRecvThreadCount);
	if (g_ulRecvThreadCount >= RECV_THREAD_MAXIMUM_NUM)
	{
		OutputDebugString(L"Work thread num is max\r\n");
		return RECV_THREAD_MAXIMUM_NUM;
	}
	if (IsListEmpty(&g_RecvSocket))
	{
		OutputDebugString(L"g_RecvSocket is empty\r\n");
		return 0;
	}
	pCurSocket = g_RecvSocket.Flink;
	while (TRUE)
	{
		if (pCurSocket != &g_RecvSocket)
		{
			pRecvSocket = CONTAINING_RECORD(pCurSocket,RECV_SOCKET,ListSocket);
			if (pRecvSocket->bIsWork == TRUE || pRecvSocket->bIsOk == TRUE)
			{
				continue;
			}
			else
			{
				EnterCriticalSection(&g_CriticalSection_Socket);
				pRecvSocket->bIsWork = TRUE;
				LeaveCriticalSection(&g_CriticalSection_Socket);
			}
		}
		OutputDebugString(L"Start work thread\r\n");
		printf("Start work thread\r\n");
		printf("Index : %08x  ip : %s\r\n",pRecvSocket->socket,inet_ntoa(pRecvSocket->ServerAddr.sin_addr));
		do 
		{
			pRecvFileDat = (PRECV_FILE_DAT)LocalAlloc(LPTR,sizeof(RECV_FILE_DAT));
		} while (NULL == pRecvFileDat);
		if (recv(pRecvSocket->socket,(char*)pRecvFileDat,sizeof(RECV_FILE_DAT),0) == -1)
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
			StringCchCat(pRecvFileInfo->wFileFullPath,MAX_PATH,L"\\RecvFiles\\");
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
				if (recv(pRecvSocket->socket,(char *)pRecvRandomBlock,sizeof(RECV_RANDOM_BLOCK),0) == -1)
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
				if (recv(pRecvSocket->socket,szRecvDat,pRandomBlock->ulCryptDatLength,0) == -1)
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
			closesocket(pRecvSocket->socket);
			EnterCriticalSection(&g_CriticalSection_Socket);
			pRecvSocket->bIsWork = FALSE;
			pRecvSocket->bIsOk = TRUE;
			pReleaseSocket = RemoveTailList(&pRecvSocket->ListSocket);
			LeaveCriticalSection(&g_CriticalSection_Socket);
			if (pRecvSocket)
			{
				LocalFree(pReleaseSocket);
				pReleaseSocket = NULL;
			}
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
		pCurSocket = pCurSocket->Flink;
	}
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