/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/DisplayExtent.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// DisplayExtent.cpp : implementation file
//

#include "stdafx.h"
#include "activeimage.h"
#include "DisplayExtent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DisplayExtent dialog


DisplayExtent::DisplayExtent(CWnd* pParent /*=NULL*/)
	: CDialog(DisplayExtent::IDD, pParent)
{
	//{{AFX_DATA_INIT(DisplayExtent)
	m_Max_X = 0.0;
	m_Max_Y = 0.0;
	m_Min_X = 0.0;
	m_Min_Y = 0.0;
	//}}AFX_DATA_INIT
}


void DisplayExtent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DisplayExtent)
	DDX_Text(pDX, IDC_DISPLAY_EXTENT_MAX_X, m_Max_X);
	DDX_Text(pDX, IDC_DISPLAY_EXTENT_MAX_Y, m_Max_Y);
	DDX_Text(pDX, IDC_DISPLAY_EXTENT_MIN_X, m_Min_X);
	DDX_Text(pDX, IDC_DISPLAY_EXTENT_MIN_Y, m_Min_Y);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DisplayExtent, CDialog)
	//{{AFX_MSG_MAP(DisplayExtent)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DisplayExtent message handlers
