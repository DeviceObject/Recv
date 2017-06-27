
// GenerateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Generate.h"
#include "GenerateDlg.h"
#include "afxdialogex.h"
#include "ServerDat.h"
#include "ClientDat.h"
#include <strsafe.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGenerateDlg dialog



CGenerateDlg::CGenerateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGenerateDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGenerateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGenerateDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(BTN_CLIENT, &CGenerateDlg::OnBnClickedClient)
	ON_BN_CLICKED(BTN_SERVER, &CGenerateDlg::OnBnClickedServer)
END_MESSAGE_MAP()


// CGenerateDlg message handlers

BOOL CGenerateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGenerateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGenerateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGenerateDlg::OnBnClickedClient()
{
	WCHAR wServerUrl[MAX_PATH];
	WCHAR wMonitorPath[MAX_PATH];

	CHAR aServerUrl[MAX_PATH];
	CHAR aMonitorPath[MAX_PATH];
	ULONG uli;
	ULONG ulCount;
	ULONG ulFinder;


	uli = 0;
	ulFinder = 0;
	ulCount = 0;
	RtlZeroMemory(aServerUrl,sizeof(CHAR) * MAX_PATH);
	RtlZeroMemory(aMonitorPath,sizeof(CHAR) * MAX_PATH);

	RtlZeroMemory(wServerUrl,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(wMonitorPath,sizeof(WCHAR) * MAX_PATH);

	GetDlgItemText(EDT_SERVERURL,wServerUrl,MAX_PATH);
	GetDlgItemText(EDT_MONITOR,wMonitorPath,MAX_PATH);
	if (wServerUrl[0] == NULL)
	{
		MessageBox(L"Server Url isn't empty!",MB_OK);
		return;
	}
	if (wMonitorPath[0] == NULL)
	{
		MessageBox(L"Monitor Directory isn't empty!",MB_OK);
		return;
	}
	ulCount = strlen(CLIENT_URL_STRING_CMP);
	WideCharToMultiByte(CP_ACP,0,wServerUrl,wcslen(wServerUrl) * sizeof(WCHAR),aServerUrl,MAX_PATH,NULL,NULL);
	WideCharToMultiByte(CP_ACP,0,wMonitorPath,wcslen(wMonitorPath) * sizeof(WCHAR),aMonitorPath,MAX_PATH,NULL,NULL);

	while (uli < SERVER_DAT_LENGTH)
	{
		if (_strnicmp((char*)((ULONG)g_ClientDat + uli),CLIENT_URL_STRING_CMP,ulCount) == 0 && \
			_strnicmp((char*)((ULONG)g_ClientDat + uli + ulCount),CLIENT_URL_STRING_CMP,ulCount) == 0 && \
			_strnicmp((char*)((ULONG)g_ClientDat + uli + ulCount * 2),CLIENT_URL_STRING_CMP,ulCount) == 0)
		{
			RtlZeroMemory((char*)((ULONG)g_ClientDat + uli),MAX_PATH - 1);
			RtlCopyMemory((char*)((ULONG)g_ClientDat + uli),aServerUrl,strlen(aServerUrl));
			ulFinder++;
			break;
		}
		uli++;
	}
	uli = 0;
	ulCount = strlen(CLIENT_MONITOR_STRING_CMP);
	while (uli < SERVER_DAT_LENGTH)
	{

		if (_strnicmp((char*)((ULONG)g_ClientDat + uli),CLIENT_MONITOR_STRING_CMP,ulCount) == 0 && \
			_strnicmp((char*)((ULONG)g_ClientDat + uli + ulCount),CLIENT_MONITOR_STRING_CMP,ulCount) == 0 && \
			_strnicmp((char*)((ULONG)g_ClientDat + uli + ulCount * 2),CLIENT_MONITOR_STRING_CMP,ulCount) == 0)
		{
			RtlZeroMemory((char*)((ULONG)g_ClientDat + uli),MAX_PATH - 1);
			RtlCopyMemory((char*)((ULONG)g_ClientDat + uli),aMonitorPath,strlen(aMonitorPath));
			ulFinder++;
			break;
		}
		uli++;
	}
	if (ulFinder == 2)
	{
		if (WriteBin((PCHAR)g_ClientDat,CLIENT_DAT_LENGTH,IsClient) == FALSE)
		{
			MessageBox(L"Generate Client Failed.",NULL,MB_OK);
			return;
		}
		else
		{
			MessageBox(L"Generate Client Success.",NULL,MB_OK);
		}
	}
	else
	{
		MessageBox(L"Isn't ServerUrl or Monitor.",NULL,MB_OK);
	}

}


void CGenerateDlg::OnBnClickedServer()
{
	WCHAR wListenPort[MAX_PATH];
	CHAR aListenPort[MAX_PATH];
	ULONG uli;
	ULONG ulCount;

	uli = 0;
	ulCount = 0;
	RtlZeroMemory(wListenPort,sizeof(WCHAR) * MAX_PATH);
	RtlZeroMemory(aListenPort,sizeof(CHAR) * MAX_PATH);

	ulCount = strlen(SERVER_STRING_CMP);
	GetDlgItemText(EDT_LISTEN_PORT,wListenPort,MAX_PATH);
	if (wListenPort[0] == NULL)
	{
		MessageBox(L"Listen Port isn't empty!",MB_OK);
		return;
	}
	WideCharToMultiByte(CP_ACP,0,wListenPort,wcslen(wListenPort) * sizeof(WCHAR),aListenPort,MAX_PATH,NULL,NULL);
	while (uli < SERVER_DAT_LENGTH)
	{
		if (_strnicmp((char*)((ULONG)g_ServerDat + uli),SERVER_STRING_CMP,ulCount) == 0 && \
			_strnicmp((char*)((ULONG)g_ServerDat + uli + ulCount),SERVER_STRING_CMP,ulCount) == 0 && \
			_strnicmp((char*)((ULONG)g_ServerDat + uli + ulCount * 2),SERVER_STRING_CMP,ulCount) == 0)
		{
			RtlZeroMemory((char*)((ULONG)g_ServerDat + uli),MAX_PATH - 1);
			RtlCopyMemory((char*)((ULONG)g_ServerDat + uli),aListenPort,strlen(aListenPort));
			if (WriteBin((PCHAR)g_ServerDat,SERVER_DAT_LENGTH,IsServer) == FALSE)
			{
				MessageBox(L"Generate Server Failed.",NULL,MB_OK);
			}
			else
			{
				MessageBox(L"Generate Server Success.",NULL,MB_OK);
			}

		}
		uli++;
	}
}


BOOLEAN CGenerateDlg::WriteBin(PCHAR pDat, ULONG ulWriteLen, ULONG ulFlags)
{
	HANDLE hFile;
	WCHAR wFilePath[MAX_PATH];
	BOOLEAN bRet;
	ULONG ulRetByteSize;

	bRet = FALSE;
	ulRetByteSize = 0;
	hFile = INVALID_HANDLE_VALUE;
	RtlZeroMemory(wFilePath,sizeof(WCHAR) * MAX_PATH);

	GetCurrentDirectory(MAX_PATH,wFilePath);

	if (ulFlags == IsClient)
	{
		StringCchCat(wFilePath,MAX_PATH,L"\\Client.exe");
	}
	else if (ulFlags == IsServer)
	{
		StringCchCat(wFilePath,MAX_PATH,L"\\Server.exe");
	}
	else
	{
		MessageBox(L"Generate Type Unknow",NULL,MB_OK);
		return bRet;
	}
	hFile = CreateFile(wFilePath, \
		GENERIC_ALL, \
		FILE_SHARE_READ | FILE_SHARE_WRITE, \
		NULL, \
		CREATE_ALWAYS, \
		FILE_ATTRIBUTE_NORMAL, \
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return bRet;
	}
	bRet = WriteFile(hFile,pDat,ulWriteLen,&ulRetByteSize,NULL);
	CloseHandle(hFile);
	return bRet;
}
