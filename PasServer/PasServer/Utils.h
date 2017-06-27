LPWSTR UTF8_2_UTF16(LPSTR strSource);
LPSTR UnicodeToANSI(LPWSTR str);
ULONG GetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength);
ULONG SetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength);
ULONG MyGetFileSize(WCHAR *wFilePath);