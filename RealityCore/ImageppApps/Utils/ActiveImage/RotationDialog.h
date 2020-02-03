/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/RotationDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// RotationDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRotationDialog dialog

class CRotationDialog : public CDialog
{
// Construction
public:
	CRotationDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRotationDialog)
	enum { IDD = IDD_ROTATION };
	double	m_Angle;
	double	m_Affinity;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRotationDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRotationDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
