//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTSmallCompressionSection.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#if !defined(AFX_HUTSMALLCOMPRESSIONSECTION_H__704480C3_AFA9_11D2_8AFF_0060082BD950__INCLUDED_)
#define AFX_HUTSMALLCOMPRESSIONSECTION_H__704480C3_AFA9_11D2_8AFF_0060082BD950__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTSmallCompressionSection : public HUTSectionDialog
    {
public:
    HUTSmallCompressionSection(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTSmallCompressionSection)
    enum { IDD = IDD_DLG_SMALL_COMPRESSION };
    CComboBox    m_ComboCompression;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTSmallCompressionSection)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    // Generated message map functions
    //{{AFX_MSG(HUTSmallCompressionSection)
    afx_msg void OnSelchangeCmbCompression();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTSMALLCOMPRESSIONSECTION_H__704480C3_AFA9_11D2_8AFF_0060082BD950__INCLUDED_)
