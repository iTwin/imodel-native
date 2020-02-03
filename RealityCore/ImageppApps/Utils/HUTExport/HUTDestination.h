//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTDestination.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// HUTDestination.h : header file
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTDestination : public HUTSectionDialog
    {
public:
    HUTDestination(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTDestination)
    enum { IDD = IDD_DLG_DESTINATION };
    CEdit    m_EditDestination;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    void ForceVisibility(BOOL pi_IsVisible);
    Utf8String GetNewSelectedPath();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTDestination)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    //{{AFX_MSG(HUTDestination)
    virtual BOOL OnInitDialog();
    afx_msg void OnBtBrowse();
    afx_msg void OnDestroy();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    BOOL    m_ForcedToVisible;
    CString m_NewPath;
    };

#ifdef HUTEXPORT_MODE_LIB
#define AFX_EXT_CLASS AFX_EXT_CLASS_SAVED_DEF
#endif

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
