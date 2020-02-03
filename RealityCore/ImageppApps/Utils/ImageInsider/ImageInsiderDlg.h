/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/ImageInsiderDlg.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ImageInsiderDlg.h : header file
//

#if !defined(AFX_IMAGEINSIDERDLG_H__70E2D9AA_ABEB_11D3_B587_0060082BD950__INCLUDED_)
#define AFX_IMAGEINSIDERDLG_H__70E2D9AA_ABEB_11D3_B587_0060082BD950__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CImageInsiderDlg dialog

class CImageInsiderDlg : public CDialog
{
// Construction
public:
	CImageInsiderDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CImageInsiderDlg)
	enum { IDD = IDD_IMAGEINSIDER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageInsiderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CImageInsiderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEINSIDERDLG_H__70E2D9AA_ABEB_11D3_B587_0060082BD950__INCLUDED_)
