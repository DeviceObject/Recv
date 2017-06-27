// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <strsafe.h>
#include "ListSup.h"

extern WCHAR g_wMonitorPath[MAX_PATH];
extern char g_ServerPort[MAX_PATH];
void InitializeGlobalDat();