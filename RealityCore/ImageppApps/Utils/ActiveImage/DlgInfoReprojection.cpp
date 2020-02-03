/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/DlgInfoReprojection.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// DlgInfoReprojection.cpp : implementation file
//

#include "stdafx.h"
#include "ActiveImage.h"
#include "DlgInfoReprojection.h"
#include "afxdialogex.h"


// DlgInfoReprojection dialog

IMPLEMENT_DYNAMIC(DlgInfoReprojection, CDialog)

DlgInfoReprojection::DlgInfoReprojection(CWnd* pParent /*=NULL*/) : CDialog(DlgInfoReprojection::IDD, pParent)
    {
    Create(DlgInfoReprojection::IDD, pParent);
    }

DlgInfoReprojection::~DlgInfoReprojection()
{
}

BEGIN_MESSAGE_MAP(DlgInfoReprojection, CDialog)
END_MESSAGE_MAP()

BOOL DlgInfoReprojection::DestroyWindow()
    {
    return CDialog::DestroyWindow();
    }

BOOL DlgInfoReprojection::OnInitDialog()
    {
    CDialog::OnInitDialog();
    return TRUE;  // return TRUE unless you set the focus to a control
    }

BOOL DlgInfoReprojection::Create(UINT nID, CWnd * pParentWnd)
    {
    return CDialog::Create(nID, pParentWnd);
    }

void DlgInfoReprojection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


// DlgInfoReprojection message handlers
 