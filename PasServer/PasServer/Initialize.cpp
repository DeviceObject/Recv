#include "stdafx.h"
#include "Recv.h"
#include "Initialize.h"

#ifdef _DEBUG
char g_szListenPort[MAX_PATH] = "4040";
#else
char g_szListenPort[MAX_PATH] = {"ListenPortListenPortListenPortListenPortListenPortListenPortListenPortListenPortListenPortListenPortListenPortListenPort"};
#endif // DEBUG
void InitializeGlobal()
{
	InitializeListHead(&g_RecvFileList);
	InitializeListHead(&g_RecvSocket);
	InitializeCriticalSection(&g_CriticalSection);
	InitializeCriticalSection(&g_CriticalSection_Socket);
	InterlockedExchange(&g_ulRecvThreadCount,0);
	RtlZeroMemory(g_wCurPath,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(g_wRecvPath,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(g_wMoveFilePath,sizeof(WCHAR) * MAX_PATH);

	GetCurrentDirectory(MAX_PATH,g_wCurPath);
	StringCchCatW(g_wCurPath,MAX_PATH,L"\\Cache\\");
	if (!PathFileExists(g_wCurPath))
	{
		_wmkdir(g_wCurPath);
	}

	GetCurrentDirectory(MAX_PATH,g_wRecvPath);
	StringCchCatW(g_wRecvPath,MAX_PATH,L"\\RecvFiles\\");
	if (!PathFileExists(g_wRecvPath))
	{
		_wmkdir(g_wRecvPath);
	}

	GetCurrentDirectory(MAX_PATH,g_wMoveFilePath);
	StringCchCatW(g_wMoveFilePath,MAX_PATH,L"\\MoveFiles\\");
	if (!PathFileExists(g_wMoveFilePath))
	{
		_wmkdir(g_wMoveFilePath);
	}

	

}
SOCKET InitializeSocket()
{
	SOCKADDR_IN LocalAddr;
	SOCKET ListenSocket;
	WSADATA WSAData;
	ULONG ulListenPort;
	
	ulListenPort = 0;
	if(WSAStartup(MAKEWORD(2,2),&WSAData))
	{
		printf("initializationing error!\n");
		WSACleanup();
	}
	ulListenPort = atoi(g_szListenPort);
	if (ulListenPort <= 0)
	{
		return NULL;
	}
	printf("Initialize socket done...\n");
	ListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(ulListenPort);
	printf("Socket bind port : %08d\n",ulListenPort);
	LocalAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	bind(ListenSocket,(sockaddr* )&LocalAddr,sizeof(sockaddr));
	listen(ListenSocket,BACKLOG);
	printf("Start listen\r\n");
	return ListenSocket;
}
void StartRecv(SOCKET ListenSocket)
{
	SOCKADDR_IN ServerAddr;
	SOCKET RecvSocket;
	PRECV_SOCKET pRecvSocket;
	int nSize = sizeof(struct sockaddr_in);
	while (TRUE)
	{
		RecvSocket = accept(ListenSocket,(sockaddr *)&ServerAddr,&nSize);
		if (RecvSocket == -1)
		{
			printf("accept failed....\n");
		}
		do 
		{
			pRecvSocket = (PRECV_SOCKET)LocalAlloc(LPTR,sizeof(RECV_SOCKET));
		} while (NULL == pRecvSocket);
		pRecvSocket->bIsOk = FALSE;
		pRecvSocket->bIsWork = FALSE;
		//InitializeListHead(&pRecvSocket->ListSocket);
		pRecvSocket->socket = RecvSocket;
		printf("Start recv client data.\r\n");
		if (g_ulRecvThreadCount < RECV_THREAD_MAXIMUM_NUM)
		{
			CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RecvWorkThread,pRecvSocket,0,NULL);
			InterlockedIncrement(&g_ulRecvThreadCount);
		}
		//else
		//{
		//	EnterCriticalSection(&g_CriticalSection_Socket);
		//	InsertHeadList(&g_RecvSocket,&pRecvSocket->ListSocket);
		//	memcpy(&pRecvSocket->ServerAddr,&ServerAddr,sizeof(ServerAddr));
		//	LeaveCriticalSection(&g_CriticalSection_Socket);
		//}
	}
	return;
}
void UnInitializeSocket(SOCKET ListenSocket)
{
	if (ListenSocket)
	{
		closesocket(ListenSocket);
	}
	WSACleanup();
}