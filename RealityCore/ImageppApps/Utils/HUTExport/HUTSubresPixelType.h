//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTSubresPixelType.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"
#include <ImagePP/all/h/HPMClassKey.h>

class HUTEXPORT_LINK_MODE HUTSubresPixelType : public HUTSectionDialog
    {
public:
    HUTSubresPixelType(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTSubresPixelType)
    enum { IDD = IDD_DLG_SUBRES_PIXEL_TYPE };
    CComboBox    m_comboPixelType;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTSubresPixelType)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    CString GetPixelTypeIdentifier(HCLASS_ID pi_PixelType);

    // Generated message map functions
    //{{AFX_MSG(HUTSubresPixelType)
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeCmbPixelType();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

