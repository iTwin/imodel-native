//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTCompressionSection.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#if !defined(AFX_HUTCOMPRESSIONSECTION_H__0DCC1526_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTCOMPRESSIONSECTION_H__0DCC1526_A98B_11D2_8400_0060082DE76F__INCLUDED_

#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTCompressionSection : public HUTSectionDialog
    {
public:
    HUTCompressionSection(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTCompressionSection)
    enum { IDD = IDD_DLG_COMPRESSION };
    CSpinButtonCtrl    m_SpinQuality;
    CSliderCtrl        m_QualitySlider;
    CEdit              m_EditQuality;
    CComboBox          m_ComboCompression;
    //}}AFX_DATA

    virtual void RefreshControls();
    virtual void RefreshConfig();

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTCompressionSection)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    // Generated message map functions
    //{{AFX_MSG(HUTCompressionSection)
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeEditQuality();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();
    afx_msg void OnSelchangeCmbCompression();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    uint32_t CountCompressionStep();
    void   SelectCompressionLevel(uint32_t pi_Position);
    uint32_t GetSelectedCompressionLevel() const;

private:

    UINT_PTR m_uTimerID;
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTCOMPRESSIONSECTION_H__0DCC1526_A98B_11D2_8400_0060082DE76F__INCLUDED_)
