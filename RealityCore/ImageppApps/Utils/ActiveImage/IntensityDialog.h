/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/IntensityDialog.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// IntensityDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIntensityDialog dialog

class CIntensityDialog : public CDialog
{
// Construction
public:
	CIntensityDialog(int pi_Min, int pi_Max, int pi_Origine, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIntensityDialog)
	enum { IDD = IDD_INTENSITY };
	CSliderCtrl	m_IntensitySlider;
	//}}AFX_DATA
    int m_Pos;
    int m_Min;
    int m_Max;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntensityDialog)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIntensityDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
