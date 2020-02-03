//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTImageSize.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#if !defined(AFX_HUTIMAGESIZE_H__0DCC1529_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTIMAGESIZE_H__0DCC1529_A98B_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTImageSize : public HUTSectionDialog
    {
public:
    // Standard Constructor
    HUTImageSize(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);
    ~HUTImageSize();

    //{{AFX_DATA(HUTImageSize)
    enum { IDD = IDD_DLG_IMG_SIZE };

    CButton  m_CheckBoxResample;

    CEdit    m_EditWidth;
    CEdit    m_EditHeight;
    CButton  m_CheckBoxFixImageSize;

    CEdit    m_EditScaleX;
    CEdit    m_EditScaleY;
    CButton  m_CheckBoxFixScaling;

    CButton  m_CheckBoxAspectRatio;
    CButton  m_ButtonReset;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTImageSize)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    CString GetImageSizeModeIdentifier(HRFImageSizeMode pi_SizeMode);
    bool   m_IsDestroy;

    // Generated message map functions
    //{{AFX_MSG(HUTImageSize)
    afx_msg void OnStatusChangeChkResample();

    afx_msg void OnKillFocusEditHeight();
    afx_msg void OnKillFocusEditWidth();
    afx_msg void OnStatusChangeChkFixImageSize();

    afx_msg void OnKillFocusEditScaleX();
    afx_msg void OnKillFocusEditScaleY();
    afx_msg void OnStatusChangeChkFixScaling();

    afx_msg void OnStatusChangeChkAspectRatio();
    afx_msg void OnClickedButtonReset();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTIMAGESIZE_H__0DCC1529_A98B_11D2_8400_0060082DE76F__INCLUDED_)
