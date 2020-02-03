//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTDialogContainer.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HUTSectionDialog.h"
#include "HUTClassID.h"

class HUTEXPORT_LINK_MODE HUTDialogContainer : public CDialog
    {
public:
    HUTDialogContainer(UINT pi_DialogD, CWnd* pParent = NULL);   // standard constructor

    //{{AFX_DATA(HUTDialogContainer)
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    void SetShowState(BOOL pi_State);
    BOOL IsShowContainer();
    virtual void RefreshDisplaySection(UINT pi_SectionID) = 0;
    void RefreshControlsTo(UINT pi_SectionID);

    virtual void SetOpenSectionShowState(UINT pi_SectionIndex, bool pi_Show) = 0;

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(HUTDialogContainer)
public:
    virtual INT_PTR DoModal();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(HUTDialogContainer)
    // NOTE: the ClassWizard will add member functions here
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    INT_PTR             GetHowManySection ();
    virtual unsigned int
    GetID() = 0;

    BOOL                AddSection(HUTSectionDialog* pi_pDialog);
    BOOL                AddSection(HUTSectionDialog* pi_pDialog, BOOL pi_IsShow);
    BOOL                ShowSection  (INT_PTR pi_SectionIndex);
    BOOL                HideSection  (INT_PTR pi_SectionIndex);
    BOOL                RemoveSection(INT_PTR pi_SectionIndex);
    BOOL                RedrawAllSection( CPoint pi_StartingSection, CRect pi_ContainerRect, int pi_ContainerFrameHeight);

    HUTSectionDialog*      GetSectionPtr(INT pi_SectionIndex);

    CArray <HUTSectionDialog*, HUTSectionDialog* >
    m_SectionArray;

private:
    BOOL                m_IsShow;
    };

/////////////////////////////////////////////////////////////////////////////
// SetShowState
/////////////////////////////////////////////////////////////////////////////

inline void HUTDialogContainer::SetShowState(BOOL pi_State)
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

inline BOOL HUTDialogContainer::IsShowContainer()
    {
    return m_IsShow;
    }
