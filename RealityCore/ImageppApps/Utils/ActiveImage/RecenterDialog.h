/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/RecenterDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// RecenterDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRecenterDialog dialog

class CRecenterDialog : public CDialog
{
// Construction
public:
	CRecenterDialog(double pi_MinX, double pi_MaxX,
                    double pi_MinY, double pi_MaxY,
                    CWnd* pParent = NULL);   // standard constructor

    double m_MinX;
    double m_MaxX;
    double m_MinY;
    double m_MaxY;

// Dialog Data
	//{{AFX_DATA(CRecenterDialog)
	enum { IDD = IDD_RECENTER };
	double	m_RecenterX;
	double	m_RecenterY;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecenterDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecenterDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
