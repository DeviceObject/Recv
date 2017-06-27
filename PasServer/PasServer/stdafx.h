// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <windows.h>
#include <strsafe.h>
#include "ListSup.h"
#include <Shlwapi.h>

#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Shlwapi.lib")
// TODO: reference additional headers your program requires here


//#define SERVER_PORT 4040
#ifdef DEBUG
extern char g_szListenPort[MAX_PATH];
#else
extern char g_szListenPort[MAX_PATH];
#endif // DEBUG
#define BACKLOG		10
