//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTEncodingSection.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#if !defined(AFX_HUTENCODINGSECTION_H__0DCC1528_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTENCODINGSECTION_H__0DCC1528_A98B_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTEncodingSection : public HUTSectionDialog
    {
public:
    HUTEncodingSection(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTEncodingSection)
    enum { IDD = IDD_DLG_ENCODING };
    CComboBox    m_comboResampling;
    CComboBox    m_comboEncoding;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTEncodingSection)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(HUTEncodingSection)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeCmbEncoding();
    afx_msg void OnSelchangeCmbResampling();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTENCODINGSECTION_H__0DCC1528_A98B_11D2_8400_0060082DE76F__INCLUDED_)
