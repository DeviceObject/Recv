


#define CONNECT_TIMEOUT 10000
#define RESOLVE_TIMEOUT 10000
#define SEND_TIMEOUT 600000
#define RECV_TIMEOUT 600000

#define USER_AGENT		L"Mozilla/5.0 (Windows NT 6.3; rv:36.0) Gecko/20100101 Firefox/36.0"		//WinHTTPSetup			in file winhttp.cpp	
#define POST_PAGE		L"/rss.asp"



extern WCHAR *g_StrServerURL;


BOOLEAN ResolveName(PBYTE pServerUrl,PBYTE pAddrToConnect,ULONG uBufLen);
BOOLEAN WinHttpSetup(PBYTE pServerUrl,PBYTE pAddrToConnect,ULONG ulBufLen,PULONG pPortToConnect);