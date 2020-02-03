//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTProgressDialog.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#if !defined(AFX_HUTPROGRESSDIALOG_H__CFF3CF61_EEC0_11D3_B59C_0060082BD950__INCLUDED_)
#define AFX_HUTPROGRESSDIALOG_H__CFF3CF61_EEC0_11D3_B59C_0060082BD950__INCLUDED_

#pragma once

// HUTProgressDialog.h : header file
//
#include "HUTClassID.h"
#include <ImagePP/all/h/HFCProgressIndicator.h>

/////////////////////////////////////////////////////////////////////////////
// HUTProgressDialog dialog

class HUTEXPORT_LINK_MODE HUTProgressDialog : public CDialog, public HFCProgressDurationListener
    {
// Construction
public:
    HUTProgressDialog(CWnd* pParent = NULL);   // standard constructor
    virtual ~HUTProgressDialog();
// Dialog Data
    //{{AFX_DATA(HUTProgressDialog)
    enum { IDD = IDD_DLG_PROGRESS };
    CButton    m_StopButton;
    CProgressCtrl    m_ProgressBar;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTProgressDialog)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL


    virtual void Progression(HFCProgressIndicator* pi_pProgressIndicator, // Indicator
                             uint32_t                pi_Processed,          // Total processed items count.
                             uint32_t                pi_CountProgression);  // number of items.

// Implementation
protected:
    uint32_t                      m_EyePos;
    bool                       m_LeftDir;

    HFCProgressIndicator* m_pProgressIndicator;
    HFCProgressIndicator* m_pLifeProgressIndicator;
    CWnd*                 m_pParent;


    void PumpMessages();

    // Generated message map functions
    //{{AFX_MSG(HUTProgressDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonStop();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTPROGRESSDIALOG_H__CFF3CF61_EEC0_11D3_B59C_0060082BD950__INCLUDED_)

//#endif // _HUTProgressDialog_H__