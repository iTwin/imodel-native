/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/RotationDialog.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// RotationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "RotationDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRotationDialog dialog


CRotationDialog::CRotationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRotationDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRotationDialog)
	m_Angle = 0.0;
	//}}AFX_DATA_INIT
}


void CRotationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRotationDialog)
	DDX_Text(pDX, IDC_ANGLE, m_Angle);
	DDX_Text(pDX, IDC_AFFINITY, m_Affinity);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRotationDialog, CDialog)
	//{{AFX_MSG_MAP(CRotationDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRotationDialog message handlers
