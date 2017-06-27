
// GenerateDlg.h : header file
//

#pragma once


typedef enum _GEN_TYPE
{
	IsClient,
	IsServer,
} GEN_TYPE,*PGEN_TYPE;
// CGenerateDlg dialog
class CGenerateDlg : public CDialogEx
{
// Construction
public:
	CGenerateDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GENERATE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedClient();
	afx_msg void OnBnClickedServer();
	BOOLEAN WriteBin(PCHAR pDat, ULONG ulWriteLen, ULONG ulFlags);
};
