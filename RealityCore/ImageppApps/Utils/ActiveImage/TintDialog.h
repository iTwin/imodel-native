
// CTintDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTintDialog dialog

#include "ColorButtonCtrl.h"

class CTintDialog : public CDialog
{
// Construction
public:
	CTintDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTintDialog)
	enum { IDD = IDD_DLG_TINT };
	//}}AFX_DATA

    COLORREF GetTintColor() const;

    COLORREF        m_TintColor;
    CEdit           m_EditTintColorRed;
    CEdit           m_EditTintColorGreen;
    CEdit           m_EditTintColorBlue;
    ColorButtonCtrl m_BtTintColor;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTintDialog)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTintDialog)
	virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtTintColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void SetColor(ColorButtonCtrl* pButton, COLORREF srcCol, CEdit* pEditRed, CEdit* pEditGreen, CEdit* pEditBlue);
};
