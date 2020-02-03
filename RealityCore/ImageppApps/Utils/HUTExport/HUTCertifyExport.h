//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTCertifyExport.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// HUTCertifyExport.h : header file
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "HUTClassID.h"

class HUTCertifyExport : public CDialog
    {
public:
    HUTCertifyExport(CWnd* pParent = NULL);   // standard constructor

    BOOL IsCertifyExportEnabled();

    BOOL IsGeoReferenceCompare() const;
    BOOL IsImageCompare()        const;
    BOOL IsSaveImageCompResult() const;
    BOOL IsUsingThreshold()      const;

    CString GetImageResultPath() const;
    CString GetImageCompareMethod() const;

    long GetThresholdMax()       const;
    long GetThresholdMin()       const;
    long GetErrorTolerence()     const;


    //{{AFX_DATA(HUTCertifyExport)
    enum { IDD = IDD_DLG_EXPORT_INTEGRITY };
    CStatic    m_StaticThresholdMin;
    CStatic    m_StaticThresholdMax;
    CStatic    m_StaticErrorTolerence;
    CButton    m_BtBrowse;
    CSpinButtonCtrl    m_SpinThresholdMin;
    CSpinButtonCtrl    m_SpinThresholdMax;
    CSpinButtonCtrl    m_SpinErrorTolerence;
    CEdit    m_EditThresholdMin;
    CEdit    m_EditThresholdMax;
    CEdit    m_EditImageResultPath;
    CEdit    m_EditErrorTolerence;
    CComboBox    m_CmbImageCompareMethod;
    CButton    m_ChkThreshold;
    CButton    m_ChkSaveImageResult;
    CButton    m_ChkImageCompare;
    CButton    m_ChkGeoRefCompare;
    //}}AFX_DATA


    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTCertifyExport)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(HUTCertifyExport)
    virtual BOOL OnInitDialog();
    afx_msg void OnChkGeorefComp();
    afx_msg void OnChkImageComp();
    afx_msg void OnChkTreshold();
    afx_msg void OnChkSaveImageResult();
    virtual void OnOK();
    afx_msg void OnBtBrowse();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:
    void EnableImageCompSection(BOOL pi_EnableSection);
    CString DirectoryBrowser(CString pi_Title);

    BOOL m_IsGeoRefCompare;
    BOOL m_IsImageCompare;
    BOOL m_IsSaveImageCompResult;
    BOOL m_IsUsingThreshold;

    long m_ThresholdMax;
    long m_ThresholdMin;
    long m_ErrorTolerence;

    CString m_ImageResultPath;
    CString m_ImageCompareMethod;
    };

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
