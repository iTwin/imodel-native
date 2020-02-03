/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ScaleDialog.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ScaleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "ScaleDialog.h"


// ScaleDialog dialog
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ScaleDialog::ScaleDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ScaleDialog::IDD, pParent)
    {
    m_scale = 0.0;
    }


void ScaleDialog::DoDataExchange(CDataExchange* pDX)
    {
	CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_SCALE, m_scale);
    }

BEGIN_MESSAGE_MAP(ScaleDialog, CDialog)
END_MESSAGE_MAP()


// ScaleDialog message handlers
