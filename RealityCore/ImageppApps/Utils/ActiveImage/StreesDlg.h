/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/StreesDlg.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// StreesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStreesDlg dialog

class CStreesDlg : public CDialog
{
// Construction
public:
	CStreesDlg(CWnd* pParent = NULL);   // standard constructor
	void Run();
	BOOL m_IsStop;

// Dialog Data
	//{{AFX_DATA(CStreesDlg)
	enum { IDD = IDD_DSTRESS };
	CButton	m_btStop;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStreesDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStreesDlg)
	afx_msg void OnBtStop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
