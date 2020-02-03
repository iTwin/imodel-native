//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTDestination.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// HUTDestination.cpp : implementation file
//

#include "stdafx.h"

#include "HUTDestination.h"
#include "HUTDialogContainer.h"
#include <ImagePP/all/h/HFCURLFile.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HUTDestination dialog


HUTDestination::HUTDestination(HRFImportExport* pi_pImportExport, CWnd* pParent)
    : HUTSectionDialog(pi_pImportExport, pParent)
    {
    //{{AFX_DATA_INIT(HUTDestination)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_ForcedToVisible = false;
    m_NewPath         = "c:\\";
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTDestination::DoDataExchange(CDataExchange* pDX)
    {
    HUTSectionDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTDestination)
    DDX_Control(pDX, IDC_EDIT_NEW_PATH, m_EditDestination);
    //}}AFX_DATA_MAP
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HUTDestination, HUTSectionDialog)
    //{{AFX_MSG_MAP(HUTDestination)
    ON_BN_CLICKED(IDC_BT_BROWSE, OnBtBrowse)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------
// HUTDestination message handlers
//-------------------------------------------------------------------

BOOL HUTDestination::OnInitDialog()
    {
    HUTSectionDialog::OnInitDialog();
    RefreshControls();

    return true;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTDestination::OnBtBrowse()
    {
    m_NewPath = DirectoryBrowser("Choose destination folder");
    m_EditDestination.SetWindowText(m_NewPath);
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTDestination::RefreshControls()
    {
    if (!m_ForcedToVisible)
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_DESTINATION_SECTION, false);
        SetShowState(false);
        }
    else
        {
        ((HUTDialogContainer*)m_pParentContainer)->SetOpenSectionShowState(HUTEXPORT_DESTINATION_SECTION, true);
        SetShowState(true);
        }
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTDestination::RefreshConfig()
    {

    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTDestination::ForceVisibility(BOOL pi_IsVisible)
    {
    m_ForcedToVisible = pi_IsVisible;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

Utf8String HUTDestination::GetNewSelectedPath()
    {
    Utf8String NewPath;

    if (m_ForcedToVisible)
        NewPath = Utf8String(m_NewPath);

    return NewPath;
    }

//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void HUTDestination::OnDestroy()
    {
    HASSERT(&m_EditDestination != 0);
    HASSERT(m_EditDestination.GetSafeHwnd() != 0);

    m_EditDestination.GetWindowText(m_NewPath);
    HUTSectionDialog::OnDestroy();
    }

