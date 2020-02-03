//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/HUTExport/HUTDialogContainer.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-------------------------------------------------------------------
// HUTDialogContainer.cpp : implementation file
//-------------------------------------------------------------------
#include "stdafx.h"

#ifndef HUTEXPORT_LINK_MODE_LIB
#include "HUTExtDLLState.h"
#endif

#include "HUTDialogContainer.h"

#ifdef __HMR_DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//-------------------------------------------------------------------
//  public HUTDialogContainer
//
//  This is the main constructor of the class. It perform member
//  initialisation. So at this step of realisation we dont perform
//  any initialisation.
//-------------------------------------------------------------------

HUTDialogContainer::HUTDialogContainer (UINT pi_DialogD, CWnd* pParent)
    : CDialog(pi_DialogD, pParent)
    {
    //{{AFX_DATA_INIT(HUTDialogContainer)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    }

//-------------------------------------------------------------------
//
// DESCRIPTION: See MFC Documentation for the desciption of this
//              method.
//-------------------------------------------------------------------

void HUTDialogContainer::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(HUTDialogContainer)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
    }

//-------------------------------------------------------------------
// DESCRIPTION: See MFC Documentation for the desciption of this method.
//-------------------------------------------------------------------

BEGIN_MESSAGE_MAP(HUTDialogContainer, CDialog)
    //{{AFX_MSG_MAP(HUTDialogContainer)
    // NOTE: the ClassWizard will add message map macros here
    ON_WM_CLOSE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-------------------------------------------------------------------
//  public AddSection
//
//  This method is use to add another section to the container. We
//  have two version of this method, the other one have an extra
//  parameter (see it for more information)
//
//  INPUT:
//        CSectionDialog *pi_pDialog : this is a section (dialog who MUST
//                                    derrive from::SectionDialog) pointor.
//  OUTPUT:
//     BOOL status : true for success
//                   false for an invalid section ptr or a memory exception.
//-------------------------------------------------------------------

BOOL HUTDialogContainer::AddSection(HUTSectionDialog* pi_pDialog)
    {
    BOOL Status=true;

    if (pi_pDialog)
        {
        try
            {
            m_SectionArray.Add(pi_pDialog);
            }
        catch(CMemoryException* e)
            {
            // ******************************************//
            // * !!!! We must do something here... !!!! *//
            // ******************************************//
            e->Delete();
            HASSERT(false);

            }
        }
    return Status;
    }

//-------------------------------------------------------------------
//  public AddSection
//
// This method is use to add another section to the container. We
// have two version of this method, the other one have only one parameter
// (see it for more information).
//
// INPUT:
//  SectionDialog *pi_pDialog : This is a section (dialog who MUST derrive
//                              from CSectionDialog) pointor.
//    BOOL isShow : This parameter tell if we want (or not) show the section
//
// OUTPUT:
//    BOOL status : true for success
//                  false for an invalid section ptr or a memory exception.
//-------------------------------------------------------------------

BOOL HUTDialogContainer::AddSection(HUTSectionDialog* pi_pDialog, BOOL pi_IsShow)
    {
    BOOL     Status=true;
    INT_PTR  ElementIndex;

    if (pi_pDialog)
        {
        try
            {
            ElementIndex = m_SectionArray.Add(pi_pDialog);
            if (pi_IsShow)
                ShowSection (ElementIndex);
            }
        catch(CMemoryException* e)
            {
            // ****************************************** //
            // * !!!! We must do something here... !!!! * //
            // ****************************************** //
            e->Delete();
            HASSERT(false);
            Status = false;
            }
        }
    else
        Status = false;

    return Status;
    }

//-------------------------------------------------------------------
// public RemoveSection
//
// This method is use to remove a section to the container array.
//
// INPUT:
//    INT pi_SectionIndex : This parameter is used to select the section to be
//                            remove.
//
// OUTPUT:
//    BOOL status : true for success
//                  false for an invalid section ptr or an invalid
//                        pi_SectionIndex
//-------------------------------------------------------------------

BOOL HUTDialogContainer::RemoveSection(INT_PTR pi_SectionIndex)
    {
    BOOL Status = false;
    HUTSectionDialog* pDialog = 0;

    if ((pi_SectionIndex >= 0) && (pi_SectionIndex < m_SectionArray.GetSize()))
        {
        pDialog = m_SectionArray.GetAt(pi_SectionIndex);
        m_SectionArray.RemoveAt(pi_SectionIndex);
        pDialog->SendMessage(WM_CLOSE,0,0);
        delete pDialog;
        Status = true;
        }
    return Status;
    }

//-------------------------------------------------------------------
// public ShowSection
//
// This method is use to set the show attribut for a specific section.
//
// INPUT:
//    INT pi_SectionIndex : This parameter is used to select the section to be
//                            show.
//
// OUTPUT:
//    BOOL status : true for success
//                  false for an invalid section ptr or an invalid
//                        pi_SectionIndex
//
//-------------------------------------------------------------------

BOOL HUTDialogContainer::ShowSection  (INT_PTR pi_SectionIndex)
    {
    BOOL Status = false;
    HUTSectionDialog* pDialog = 0;

    if ((pi_SectionIndex >= 0) && (pi_SectionIndex < m_SectionArray.GetSize()))
        {
        pDialog = m_SectionArray.GetAt(pi_SectionIndex);
        if (pDialog)
            {
            pDialog->SetShowState(true);
            Status=true;
            }
        }
    return Status;
    }

//-------------------------------------------------------------------
//  public HideSection
//
//  This method is use to reset the show attribut for a specific section.
//
//  INPUT:
//        INT pi_SectionIndex : This parameter is used to select the section
//                            to be hide.
//
// OUTPUT:
//    BOOL status : true for success
//                  false for an invalid section ptr or an invalid
//                        pi_SectionIndex
//-------------------------------------------------------------------

BOOL HUTDialogContainer::HideSection  (INT_PTR pi_SectionIndex)
    {
    BOOL Status = false;
    HUTSectionDialog* pDialog = 0;

    if ((pi_SectionIndex >= 0) && (pi_SectionIndex < m_SectionArray.GetSize()))
        {
        pDialog = m_SectionArray.GetAt(pi_SectionIndex);
        if (pDialog)
            {
            pDialog->SetShowState(false);
            Status = true;
            }
        }
    return Status;
    }

//-------------------------------------------------------------------
// public GetSectionPtr
//
// This method is use to get the pointer of a specified section
//
// INPUT:
//    INT pi_SectionIndex : This parameter is used to select the section
//                        pointor to be return.
//
// OUTPUT:
//    CSectionDialog *sectionPtr : pointor to a section (implicit cast as a
//                                 CSectionDialog*)
//
//-------------------------------------------------------------------

HUTSectionDialog* HUTDialogContainer::GetSectionPtr(INT pi_SectionIndex)
    {
    HUTSectionDialog* pSectionPtr = 0;

    if ((pi_SectionIndex >= 0) && (pi_SectionIndex < m_SectionArray.GetSize()))
        pSectionPtr = m_SectionArray.GetAt(pi_SectionIndex);

    return pSectionPtr;
    }

//-------------------------------------------------------------------
// public GetHowManySection
//
// This method is use to know how many section is contained in the array
//
// OUTPUT:
//      INT m_SectionArray.GetSize() : return the size of the section array
//
//-------------------------------------------------------------------

INT_PTR HUTDialogContainer::GetHowManySection ()
    {
    return m_SectionArray.GetSize();
    }

//-------------------------------------------------------------------
// public RedrawAllSection
//
// This method is use to
//
// INPUT:
//      CPoint pi_StartingSection   :
//      CRect pi_ContainerRect      :
//      INT pi_ContainerFrameHeight :
//
//-------------------------------------------------------------------

BOOL HUTDialogContainer::RedrawAllSection( CPoint pi_StartingSection, CRect pi_ContainerRect, INT pi_ContainerFrameHeight)
    {
    BOOL Status = true;
    HUTSectionDialog* pDialog = 0;
    CRect  SectionRect;
    CPoint SectionUppperLeft;

    SectionUppperLeft.x = pi_StartingSection.x;  // Its's where we begin to add the section to the container
    SectionUppperLeft.y = pi_StartingSection.y;

    for (int i = 0; i < m_SectionArray.GetSize(); i++)
        {
        pDialog = m_SectionArray.GetAt(i);
        pDialog->RefreshControls();
        if (pDialog->IsShowSection())
            {
            SectionRect = pDialog->GetSectionSize();
            pDialog->SetSectionUpperLeftCoordinate (SectionUppperLeft);
            pDialog->SetWindowPos(NULL,SectionUppperLeft.x,SectionUppperLeft.y,0,0,SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
            pDialog->RedrawWindow();
            pDialog->SetShowState(true);
            SectionUppperLeft.y += SectionRect.Height();
            }
        else
            pDialog->SetShowState(false);
        }
    SetWindowPos(NULL,0,0,pi_ContainerRect.Width(),SectionUppperLeft.y + pi_ContainerFrameHeight,SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);
    return Status;
    }

//-------------------------------------------------------------------
//  public : RefreshControlsTo
//-------------------------------------------------------------------

void HUTDialogContainer::RefreshControlsTo(UINT pi_SectionID)
    {
    HASSERT((UINT)m_SectionArray.GetSize() > pi_SectionID);

    HUTSectionDialog* pDialog = m_SectionArray.GetAt(pi_SectionID);
    if (pDialog->IsShowSection())
        pDialog->RefreshControls();
    }

//-------------------------------------------------------------------
//  protected : ReadParameter
//-------------------------------------------------------------------

void HUTDialogContainer::OnClose()
    {
//    for (int i = m_SectionArray.GetSize(); i >= 0; i--)
//      RemoveSection(i);

    CDialog::OnClose();
    }


//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

INT_PTR HUTDialogContainer::DoModal()
    {
#ifndef HUTEXPORT_LINK_MODE_LIB
    HUTExtDLLState State;
#endif
    return CDialog::DoModal();
    }
