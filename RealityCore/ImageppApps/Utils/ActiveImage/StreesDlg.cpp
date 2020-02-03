/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/StreesDlg.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// StreesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "StreesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStreesDlg dialog


CStreesDlg::CStreesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStreesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStreesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_IsStop = false;
}


void CStreesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStreesDlg)
	DDX_Control(pDX, ID_BT_STOP, m_btStop);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStreesDlg, CDialog)
	//{{AFX_MSG_MAP(CStreesDlg)
	ON_BN_CLICKED(ID_BT_STOP, OnBtStop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStreesDlg message handlers

void CStreesDlg::OnBtStop() 
{
	m_IsStop = true;
}
