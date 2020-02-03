//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTSectionDialog.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "stdafx.h"

#include "HUTSectionDialog.h"
#include "HUTDialogContainer.h"
#include <ImagePP/all/h/HRFImportExport.h>

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HUTSectionDialog constructor
/////////////////////////////////////////////////////////////////////////////

HUTSectionDialog::HUTSectionDialog(HRFImportExport* pi_pImportExport, CWnd* pParent)
    : m_IsShow(false)
    {
    m_pImportExport = pi_pImportExport;
    m_HiddenSection = false;

    //{{AFX_DATA_INIT(HUTSectionDialog)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

/////////////////////////////////////////////////////////////////////////////
// DoDataExchange
/////////////////////////////////////////////////////////////////////////////

void HUTSectionDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTSectionDialog)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
    }

/////////////////////////////////////////////////////////////////////////////
// MESSAGE MAP
/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(HUTSectionDialog, CDialog)
    //{{AFX_MSG_MAP(HUTSectionDialog)
    // NOTE: the ClassWizard will add message map macros here
    ON_WM_CLOSE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GetSectionSize
/////////////////////////////////////////////////////////////////////////////

CRect HUTSectionDialog::GetSectionSize ()
    {
    CRect SectionRect;

    GetWindowRect(SectionRect);
    return SectionRect;
    }

/////////////////////////////////////////////////////////////////////////////
// Create
/////////////////////////////////////////////////////////////////////////////

BOOL HUTSectionDialog::Create( LPCTSTR lpszTemplateName, CWnd* pParentWnd)
    {
    m_pParentContainer = (HUTDialogContainer*)pParentWnd;

    return CDialog::Create( lpszTemplateName, pParentWnd);
    }

/////////////////////////////////////////////////////////////////////////////
// Create
/////////////////////////////////////////////////////////////////////////////

BOOL HUTSectionDialog::Create( UINT nIDTemplate, CWnd* pParentWnd)
    {
    m_pParentContainer = (HUTDialogContainer*)pParentWnd;

    return CDialog::Create( nIDTemplate, pParentWnd);
    }

/////////////////////////////////////////////////////////////////////////////
// OnInitDialog
/////////////////////////////////////////////////////////////////////////////

BOOL HUTSectionDialog::OnInitDialog()
    {
    CDialog::OnInitDialog();

    // ReadParameter is a pure virtual method who must be redefine into
    // each section for initialisation.
    RefreshControls();

    return true;  // return true unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return false
    }

/////////////////////////////////////////////////////////////////////////////
// OnClose
/////////////////////////////////////////////////////////////////////////////

void HUTSectionDialog::OnClose()
    {
    // WriteParameter is a pure virtual method who must be redefine into
    // each section for saving dialog configuration.
    RefreshConfig();
    CDialog::OnClose();
    }

/////////////////////////////////////////////////////////////////////////////
// DirectoryBrowser
/////////////////////////////////////////////////////////////////////////////

CString HUTSectionDialog::DirectoryBrowser(CString pi_Title)
    {
    CString PathDirectory;

    // Setup the Browse info structure
    WChar sDisplayName[255];

    BROWSEINFO BrowseInfoStruct;
    BrowseInfoStruct.hwndOwner  = m_hWnd;
    BrowseInfoStruct.pidlRoot   = NULL;
    BrowseInfoStruct.pszDisplayName = sDisplayName;
    BrowseInfoStruct.lpszTitle  =  LPCWSTR(pi_Title);
    BrowseInfoStruct.ulFlags    = BIF_RETURNONLYFSDIRS;
    BrowseInfoStruct.lpfn       = NULL;
    BrowseInfoStruct.lParam     = NULL;

    // Browse for the folder
    LPITEMIDLIST pItemIDList;
    if (pItemIDList = SHBrowseForFolder(&BrowseInfoStruct))
        {
        // get the folder and set the new string
        WChar sPath[260];

        if (SHGetPathFromIDList(pItemIDList, sPath))
            PathDirectory = sPath;

        // Avoid memory leaks by deleting the PIDL
        // using the shells task allocator
        IMalloc* pMalloc;
        if (SHGetMalloc(&pMalloc) != NOERROR)
            {
            TRACE(L"Failed to get pointer to shells task allocator");
            return L"Error in ConfigSection::DirectoryBrowser";
            }
        pMalloc->Free(pItemIDList);
        if (pMalloc)
            pMalloc->Release();
        }
    return PathDirectory;
    }

/////////////////////////////////////////////////////////////////////////////
// RefreshDisplaySection
/////////////////////////////////////////////////////////////////////////////

void HUTSectionDialog::RefreshDisplaySection(UINT pi_SectionID)
    {
    m_pParentContainer->RefreshDisplaySection(pi_SectionID);
    }

/////////////////////////////////////////////////////////////////////////////
// RefreshDisplaySection
/////////////////////////////////////////////////////////////////////////////

void HUTSectionDialog::RefreshControlsTo(UINT pi_SectionID)
    {
    m_pParentContainer->RefreshControlsTo(pi_SectionID);
    }
/////////////////////////////////////////////////////////////////////////////

HRFImportExport* HUTSectionDialog::GetImportExport() const
    {
    return m_pImportExport;
    }
