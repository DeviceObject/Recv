
#define SERVER_URL_MAXIMUM 0x04
typedef LPVOID (WINAPI * pInternetOpen)(LPCSTR,DWORD,LPCSTR,LPCSTR,DWORD);
typedef LPVOID (WINAPI * pInternetOpenUrl)(LPVOID,LPCSTR,LPCSTR,DWORD,DWORD,DWORD);
typedef BOOL (WINAPI * pInternetCloseHandle)(LPVOID);
typedef BOOL (WINAPI * pInternetReadFile)(LPVOID,LPVOID,DWORD,LPDWORD) ;

BOOLEAN GetSyncIp(WCHAR *wServerIp);
int GetHttpHead(char *buf,char *myurl);
char* HttpGetFile(CHAR Url[]);
extern CHAR g_szServerUrl[MAX_PATH];
extern LPSTR g_ServerIp;