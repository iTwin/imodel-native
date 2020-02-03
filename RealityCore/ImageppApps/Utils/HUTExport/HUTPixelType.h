//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTPixelType.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#if !defined(AFX_HUTPIXELTYPE_H__0DCC152A_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTPIXELTYPE_H__0DCC152A_A98B_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTPixelType : public HUTSectionDialog
    {
public:
    HUTPixelType(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTPixelType)
    enum { IDD = IDD_DLG_PIXEL_TYPE };
    CComboBox    m_comboPixelType;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTPixelType)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    CString GetPixelTypeIdentifier(HCLASS_ID pi_PixelType);

    // Generated message map functions
    //{{AFX_MSG(HUTPixelType)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeCmbPixelType();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTPIXELTYPE_H__0DCC152A_A98B_11D2_8400_0060082DE76F__INCLUDED_)
