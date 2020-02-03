// TintDialog.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "TintDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTintDialog dialog

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
CTintDialog::CTintDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTintDialog::IDD, pParent)
{
    m_TintColor = 0x000000;

	//{{AFX_DATA_INIT(CTintDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CTintDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTintDialog)
    DDX_Control(pDX, IDC_BUTTON_TINT_COLOR, m_BtTintColor);
    DDX_Control(pDX, IDC_ED_TINT_COLOR_R   , m_EditTintColorRed);
    DDX_Control(pDX, IDC_ED_TINT_COLOR_G   , m_EditTintColorGreen);
    DDX_Control(pDX, IDC_ED_TINT_COLOR_B   , m_EditTintColorBlue);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTintDialog, CDialog)
	//{{AFX_MSG_MAP(CTintDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_BUTTON_TINT_COLOR, OnBnClickedBtTintColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTintDialog message handlers

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
BOOL CTintDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

    // Fill color fields and set button color
    SetColor(&m_BtTintColor, m_TintColor, &m_EditTintColorRed, &m_EditTintColorGreen, &m_EditTintColorBlue);

	return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CTintDialog::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    CDialog::OnOK();
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
void CTintDialog::OnBnClickedBtTintColor()
{
    CColorDialog ColorPicker(m_TintColor, CC_FULLOPEN, this);
    if (ColorPicker.DoModal() == IDOK)
    {
        m_TintColor = ColorPicker.GetColor();
        SetColor(&m_BtTintColor, m_TintColor, &m_EditTintColorRed, &m_EditTintColorGreen, &m_EditTintColorBlue);
    }
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------
BOOL CTintDialog::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

//------------------------------------------------------------------------------------
// Fill color fields and set button color
//------------------------------------------------------------------------------------
void CTintDialog::SetColor(ColorButtonCtrl* pButton, COLORREF srcCol, CEdit* pEditRed, CEdit* pEditGreen, CEdit* pEditBlue)
{
    pButton->SetBGColor(srcCol);
    pButton->RedrawWindow();

    CString DisplayBuffer;
    DisplayBuffer.Format(L"%u", GetRValue(srcCol));
    pEditRed->SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", GetGValue(srcCol));
    pEditGreen->SetWindowText(DisplayBuffer);

    DisplayBuffer.Format(L"%u", GetBValue(srcCol));
    pEditBlue->SetWindowText(DisplayBuffer);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
COLORREF CTintDialog::GetTintColor() const
{
    return m_TintColor;
}
