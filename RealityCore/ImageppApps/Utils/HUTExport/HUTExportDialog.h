//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTExportDialog.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#if !defined(AFX_HUTEXPORTDIALOG_H__0DCC1525_A98B_11D2_8400_0060082DE76F__INCLUDED_)
#define AFX_HUTEXPORTDIALOG_H__0DCC1525_A98B_11D2_8400_0060082DE76F__INCLUDED_

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

class HUTEXPORT_LINK_MODE HUTExportDialog : public HUTDialogContainer
    {
    // Construction
public:
    HUTExportDialog(HRFImportExport* pi_pImportExport, CWnd* pParent);

    virtual ~HUTExportDialog();


    void             RefreshDisplaySection(UINT pi_SectionID);
    HRFImportExport* GetImportExport() const;

    WString          GetExportFileName();
    bool            VerifyFile (CString pi_FileName);

    void SetOpenSectionShowState(UINT pi_SectionIndex, bool pi_Show);

    //{{AFX_DATA(HUTExportDialog)
    enum { IDD = IDD_DLG_EXPORT_DIALOG };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

protected:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTExportDialog)
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Generated message map functions
    //{{AFX_MSG(HUTExportDialog)
    afx_msg void OnExportasBestOption();
    afx_msg void OnExportasExport();
    afx_msg void OnExportasSelect();
    virtual BOOL OnInitDialog();
    afx_msg void OnSelchangeExportasFormat();
    afx_msg void OnChangeExportasFolder();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    virtual UINT GetID();

private:

    HRFImportExport*            m_pImportExport;
    HUTDestination*             m_pDestinationSection;
    HUTSmallCompressionSection* m_pSmallCompressionSection;
    HUTCompressionSection*      m_pCompressionSection;
    HUTBlockSection*            m_pBlockSection;
    HUTEncodingSection*         m_pEncodingSection;
    HUTImageSize*               m_pImageSize;
    HUTPixelType*               m_pPixelType;
    HUTGeoReferenceSection*     m_pGeoReferenceSection;
    HUTSubresPixelType*         m_pSubresPixelType;

    CRect  m_HUTExportDialogClientRect;
    CRect  m_HUTExportDialogWindowRect;

    char   m_openSection[HUTEXPORT_MAX_SECTION];

    bool   RefreshAllDisplaySection(void);
    void   HideDisplaySection(void);

    void   CreateSections();

    BOOL    m_UseBestOption;
    CEdit   m_Folder;
    CString m_SelectedFolder;
    };


/////////////////////////////////////////////////////////////////////////////
// HUTExportDialog::GetID
/////////////////////////////////////////////////////////////////////////////

inline UINT HUTExportDialog::GetID()
    {
    return IDD;
    }



//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUTEXPORTDIALOG_H__0DCC1525_A98B_11D2_8400_0060082DE76F__INCLUDED_)
