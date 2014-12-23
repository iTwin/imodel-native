// metadataDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "MetaDataGrid.h"

// CMetaDataDlg dialog
class CMetaDataDlg : public CDialog
{
// Construction
public:
	CMetaDataDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_METADATA_DIALOG };

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

	CMetaDataGrid m_grid;
public:
	CEdit m_edtFilename;	// edit control for filename
	afx_msg void OnBnClickedBtnOpenFile();

	bool InitializeVortexAPI();	// Load and Initialise the Pointools API
	void DisplayPODMetaData( CString filename );	// load a file and display meta
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedBtnSave();
};
