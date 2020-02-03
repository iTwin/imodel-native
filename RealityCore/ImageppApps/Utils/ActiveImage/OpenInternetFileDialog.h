/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/OpenInternetFileDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// OpenInternetFileDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// OpenInternetFileDialog dialog

class OpenInternetFileDialog : public CDialog
{
// Construction
public:
	OpenInternetFileDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OpenInternetFileDialog)
	enum { IDD = IDD_INTERNET_FILE_OPEN };	
	CString	m_InternetUrl;			
	//}}AFX_DATA

    virtual BOOL        OnInitDialog() override;
	const CString& GetUrl() {return m_InternetUrl;}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OpenInternetFileDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OpenInternetFileDialog)	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpen();
public:
	afx_msg void OnBnClickedCancel();
};
