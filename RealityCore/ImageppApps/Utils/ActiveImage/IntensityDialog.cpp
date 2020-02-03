/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/IntensityDialog.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// IntensityDialog.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "IntensityDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntensityDialog dialog


CIntensityDialog::CIntensityDialog(int pi_Min, int pi_Max, int pi_Pos, CWnd* pParent /*=NULL*/)
	: CDialog(CIntensityDialog::IDD, pParent)
{
    m_Min = pi_Min;
    m_Max = pi_Max;
    m_Pos = pi_Pos;

	//{{AFX_DATA_INIT(CIntensityDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CIntensityDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIntensityDialog)
	DDX_Control(pDX, IDC_SLIDER1, m_IntensitySlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIntensityDialog, CDialog)
	//{{AFX_MSG_MAP(CIntensityDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntensityDialog message handlers

BOOL CIntensityDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_IntensitySlider.SetRange(m_Min, m_Max, true);
	m_IntensitySlider.SetPos(m_Pos);	

    return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}

BOOL CIntensityDialog::DestroyWindow() 
{
	m_Pos = m_IntensitySlider.GetPos();
	// TODO: Add your specialized code here and/or call the base class
	
	return CDialog::DestroyWindow();
}
