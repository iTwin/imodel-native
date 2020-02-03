//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTSectionDialog.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HUTClassID.h"

class HUTDialogContainer;

BEGIN_IMAGEPP_NAMESPACE
class HRFImportExport;
END_IMAGEPP_NAMESPACE

// Do not want to override CWD::Create method. Disable warning messages 4266 - no override available for virtual member function from base 'type'; function is hidden
#pragma warning(push)
#pragma warning( disable : 4266 )

class HUTEXPORT_LINK_MODE HUTSectionDialog : public CDialog
    {
public:
    HUTSectionDialog(HRFImportExport* pi_pImportExport, CWnd* pParent = NULL);   // standard constructor

    // Dialog Data
    //{{AFX_DATA(HUTSectionDialog)
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    BOOL  IsShowSection ();
    void  SetShowState(BOOL pi_State);
    CRect GetSectionSize();

    void HiddenSection(BOOL pi_HiddenSection);

    virtual void RefreshControls() = 0;
    virtual void RefreshConfig()   = 0;

    CPoint GetSectionUpperLeftCoordinate ();
    void   SetSectionUpperLeftCoordinate (CPoint pi_UpperLeftCoordinate);

    HRFImportExport* GetImportExport() const;

    void RefreshDisplaySection(UINT pi_SectionID);
    void RefreshControlsTo(UINT pi_SectionID);

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTSectionDialog)
public:
    virtual BOOL Create( LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL ) override;
    virtual BOOL Create( UINT nIDTemplate, CWnd* pParentWnd = NULL ) override;
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:
    // Generated message map functions
    //{{AFX_MSG(HUTSectionDialog)
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    CString DirectoryBrowser(CString pi_Title);

    HUTDialogContainer* m_pParentContainer;
    BOOL                m_HiddenSection;

private:

    HRFImportExport*    m_pImportExport;

    CPoint              m_UpperLeftCoordinate;
    BOOL                m_IsShow;

    };

#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////
// SetShowState
/////////////////////////////////////////////////////////////////////////////

inline void HUTSectionDialog::HiddenSection(BOOL pi_HiddenSection)
    {
    m_HiddenSection = pi_HiddenSection;
    }

/////////////////////////////////////////////////////////////////////////////
// SetShowState
/////////////////////////////////////////////////////////////////////////////

inline void HUTSectionDialog::SetShowState(BOOL pi_State)
    {
    if (pi_State)                  // Show the Window..
        {
        if (!m_IsShow)
            ShowWindow(SW_SHOWNA);
        }
    else                           // Hide the Window...
        {
        if (m_IsShow)
            ShowWindow(SW_HIDE);
        }
    m_IsShow = pi_State;
    }

/////////////////////////////////////////////////////////////////////////////
// IsShowSection
/////////////////////////////////////////////////////////////////////////////

inline BOOL HUTSectionDialog::IsShowSection ()
    {
    return m_IsShow;
    }

/////////////////////////////////////////////////////////////////////////////
// SetSectionUpperLeftCoordinate
/////////////////////////////////////////////////////////////////////////////

inline void HUTSectionDialog::SetSectionUpperLeftCoordinate(CPoint pi_UpperLeftCoordinate)
    {
    m_UpperLeftCoordinate.x = pi_UpperLeftCoordinate.x;
    m_UpperLeftCoordinate.y = pi_UpperLeftCoordinate.y;
    }

/////////////////////////////////////////////////////////////////////////////
// SetSectionUpperLeftCoordinate
/////////////////////////////////////////////////////////////////////////////

inline CPoint HUTSectionDialog::GetSectionUpperLeftCoordinate()
    {
    return m_UpperLeftCoordinate;
    }
