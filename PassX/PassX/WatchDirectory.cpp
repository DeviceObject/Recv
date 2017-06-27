#include "PassX.h"
#include "EnumDirectoryFile.h"
#include "WatchDirectory.h"


void RefreshDirectory(LPTSTR lpDir)
{
	// This is where you might place code to refresh your
	// directory listing, but not the subtree because it
	// would not be necessary.

	_tprintf(TEXT("Directory (%s) changed.\n"), lpDir);
}

void RefreshTree(LPTSTR lpDrive)
{
	// This is where you might place code to refresh your
	// directory listing, including the subtree.

	_tprintf(TEXT("Directory tree (%s) changed.\n"), lpDrive);
}
DWORD WINAPI DoFindAllFile(LPVOID lpParameter)
{
	EnumDirectoryFileEx((WCHAR*)lpParameter);
	return 0;
}

void WatchDirectory(LPTSTR lpDir)
{
	DWORD dwWaitStatus; 
	HANDLE dwChangeHandles;
	// Watch the directory for file creation and deletion. 

	dwChangeHandles = FindFirstChangeNotification( 
		lpDir,                         // directory to watch 
		FALSE,                         // do not watch subtree 
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME); // watch file name changes 

	if (dwChangeHandles == INVALID_HANDLE_VALUE) 
	{
		OutputDebugString(L"\n ERROR: FindFirstChangeNotification function failed.\n");
		ExitProcess(GetLastError()); 
	}
	// Make a final validation check on our handles.
	if (dwChangeHandles == NULL)
	{
		OutputDebugString(L"\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
		ExitProcess(GetLastError()); 
	}
	// Change notification is set. Now wait on both notification 
	// handles and refresh accordingly. 

	while (TRUE) 
	{
		OutputDebugString(L"\nWaiting for notification...\n");
		dwWaitStatus = WaitForSingleObject(dwChangeHandles,INFINITE); 
		if (dwWaitStatus == WAIT_OBJECT_0)
		{
			EnumDirectoryFileEx(lpDir);
			if (FindNextChangeNotification(dwChangeHandles) == FALSE)
			{
				OutputDebugString(L"\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError()); 
			}
		}
	}
}