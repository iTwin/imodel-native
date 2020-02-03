//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTBlockSection.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// HUTBlockSection.h : header file
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_HUTBLOCKSECTION_H__0DCC1527_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTBLOCKSECTION_H__0DCC1527_A98B_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTBlockSection : public HUTSectionDialog
    {
public:
    HUTBlockSection(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTBlockSection)
    enum { IDD = IDD_DLG_BLOCK };
    CComboBox    m_comboBlockType;
    CSpinButtonCtrl    m_SpinWidth;
    CSpinButtonCtrl    m_SpinHeight;
    CEdit    m_EditWidth;
    CEdit    m_EditHeight;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTBlockSection)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(HUTBlockSection)
    afx_msg void OnSelchangeComboBlockType();
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditHeight();
    afx_msg void OnChangeEditWidth();
    afx_msg void OnKillFocusEditHeight();
    afx_msg void OnKillFocusEditWidth();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

#ifdef HUTEXPORT_MODE_LIB
#define AFX_EXT_CLASS AFX_EXT_CLASS_SAVED_DEF
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTBLOCKSECTION_H__0DCC1527_A98B_11D2_8400_0060082DE76F__INCLUDED_)
