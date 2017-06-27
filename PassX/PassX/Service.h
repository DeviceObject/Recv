
#define SERVICE_NAME			L"PasSend"
#define SERVICE_SHOW_NAME		L"PasSend File Transfer"
#define SERVICE_DEPENDENCIES	L"PasSend File Transfer is use Base64 and RSA public/private key and random block with files encrypt send."

BOOLEAN Install(WCHAR *wServiceName,WCHAR *wShowName,WCHAR *ServicePath,WCHAR *wDependencies);
BOOLEAN Uninstall(WCHAR *wServiceName);
void Init();
void WINAPI ServiceMain();
void WINAPI ServiceStrl(DWORD dwOpcode);
BOOLEAN IsInstalled(WCHAR *wServiceName);

extern ULONG g_ulThreadID;