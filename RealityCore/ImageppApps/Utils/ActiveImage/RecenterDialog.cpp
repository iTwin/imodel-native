/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/RecenterDialog.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// RecenterDialog.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "RecenterDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRecenterDialog dialog


CRecenterDialog::CRecenterDialog(double pi_MinX, double pi_MaxX,
                                 double pi_MinY, double pi_MaxY,
                                 CWnd* pParent /*=NULL*/)
	: CDialog(CRecenterDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRecenterDialog)
	m_RecenterX = 0.0;
	m_RecenterY = 0.0;
	//}}AFX_DATA_INIT

    // save the range
    m_MinX = pi_MinX;
    m_MaxX = pi_MaxX;
    m_MinY = pi_MinY;
    m_MaxY = pi_MaxY;
}


void CRecenterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecenterDialog)
	DDX_Text(pDX, IDC_X, m_RecenterX);
	DDX_Text(pDX, IDC_Y, m_RecenterY);
	//}}AFX_DATA_MAP

    // verify range
	DDV_MinMaxDouble(pDX, m_RecenterX, m_MinX, m_MaxX);
	DDV_MinMaxDouble(pDX, m_RecenterY, m_MinY, m_MaxY);
}


BEGIN_MESSAGE_MAP(CRecenterDialog, CDialog)
	//{{AFX_MSG_MAP(CRecenterDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecenterDialog message handlers
