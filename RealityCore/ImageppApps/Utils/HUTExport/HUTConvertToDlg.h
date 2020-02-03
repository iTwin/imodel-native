//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTConvertToDlg.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-------------------------------------------------------------------
//
//-------------------------------------------------------------------
// HUTConvertToDlg.h : header file
//-------------------------------------------------------------------
#pragma once

#include "HUTDialogContainer.h"
#include "HUTDestination.h"
#include "HUTSmallCompressionSection.h"
#include "HUTGeoReferenceSection.h"
#include "HUTCompressionSection.h"
#include "HUTBlockSection.h"
#include "HUTEncodingSection.h"
#include "HUTImageSize.h"
#include "HUTPixelType.h"
#include "HUTSubresPixelType.h"
#include "HUTClassID.h"

BEGIN_IMAGEPP_NAMESPACE
class HRFImportExport;
END_IMAGEPP_NAMESPACE

class HUTEXPORT_LINK_MODE HUTConvertToDlg : public HUTDialogContainer
    {
public:
    HUTConvertToDlg(HRFImportExport* pi_pImportExport, BOOL pi_MultiSelectionMode, CWnd* pParent = NULL);   // standard constructor
    virtual ~HUTConvertToDlg();

    HRFImportExport*
    GetImportExport() const;
    //void    SetImportExport(HRFImportExport* pi_pImportExport);

    void    RefreshDisplaySection(UINT pi_SectionID);
    void    SetOpenSectionShowState(UINT pi_SectionIndex, bool pi_Show);
    void    HideAllSection(BOOL pi_HideAllSection = true);

    Utf8String GetNewSelectedPath();

    BOOL    IsReplaceExtension()    const;
    BOOL    IsResumeInFilename()    const;
    BOOL    IsExportWithAllOption() const;
    BOOL    IsExportToSourceFormat()const;
    BOOL    IsChkBestOption()       const;
    BOOL    IsChkExcludeJPEG()      const;

    BOOL    IsGeoReferenceCompare() const;
    BOOL    IsImageCompare()        const;

    BOOL IsSaveImageCompResult()    const;
    BOOL IsUsingThreshold()         const;

    CString GetImageResultPath()    const;
    CString GetImageCompareMethod() const;

    long GetThresholdMax()          const;
    long GetThresholdMin()          const;
    long GetErrorTolerence()        const;

    //{{AFX_DATA(HUTConvertToDlg)
    enum { IDD = IDD_DLG_CONVERT_TO };
    CButton    m_ChkExportIntegrity;
    CButton    m_CbResumeInFileName;
    CButton    m_CbAllPossibility;
    CButton    m_RDRetainExt;
    CButton    m_RDReplaceExt;
    CButton    m_BtUseBestOption;
    CButton    m_BtExcludeJPEG;
    CButton    m_BtExportToSourceFormat;
    CButton    m_CbKeepSrcPath;
    CComboBox    m_CmbFileFormat;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTConvertToDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(HUTConvertToDlg)
    afx_msg void OnChkBestOption();
    afx_msg void OnChkExcludeJPEG();
    afx_msg void OnChkKeepSrcpath();
    afx_msg void OnBtCancel();
    afx_msg void OnBtOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeCmbFileFormat();
    afx_msg void OnRdRetainExt();
    afx_msg void OnRdReplaceExtension();
    afx_msg void OnChkAllPossibility();
    afx_msg void OnChkResumeInFilename();
    afx_msg void OnChkExportIntegrity();
    afx_msg void OnExportToSourceFormat();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual UINT        GetID();

private:

    void    FillComboFileFormat();
    bool    RefreshAllDisplaySection();
    void    CreateSections();

    HRFImportExport*        m_pImportExport;
    HUTDestination*         m_pDestinationSection;
    HUTSmallCompressionSection*
    m_pSmallCompressionSection;
    HUTCompressionSection*
    m_pCompressionSection;
    HUTBlockSection*        m_pBlockSection;
    HUTEncodingSection*     m_pEncodingSection;
    HUTImageSize*           m_pImageSize;
    HUTPixelType*           m_pPixelType;
    HUTSubresPixelType*     m_pSubresPixelType;
    HUTGeoReferenceSection* m_pGeoReferenceSection;

    CRect                   m_HUTExportDialogClientRect;
    CRect                   m_HUTExportDialogWindowRect;
    BOOL                    m_ReplaceExtension;
    BOOL                    m_ExportWithAllOption;
    BOOL                    m_ResumeInFilename;
    BOOL                    m_MultiSelectionMode;
    BOOL                    m_ExportToSourceFormat;
    BOOL                    m_ChkBestOption;
    BOOL                    m_ChkExcludeJPEG;

    BOOL                    m_IsImageCompare;
    BOOL                    m_IsGeoRefCompare;
    BOOL                    m_IsSaveImageCompResult;
    BOOL                    m_IsUsingThreshold;

    long                    m_ThresholdMax;
    long                    m_ThresholdMin;
    long                    m_ErrorTolerence;

    CString                 m_ImageResultPath;
    CString                 m_ImageCompareMethod;

    char                    m_openSection[HUTEXPORT_MAX_SECTION];
    };

/////////////////////////////////////////////////////////////////////////////
// HUTExportDialog::GetID
/////////////////////////////////////////////////////////////////////////////

inline UINT HUTConvertToDlg::GetID()
    {
    return IDD;
    }

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.
