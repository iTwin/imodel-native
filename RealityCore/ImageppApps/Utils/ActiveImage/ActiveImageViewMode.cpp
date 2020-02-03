/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageViewMode.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageViewMode.cpp,v 1.4 2011/07/18 21:10:23 Donald.Morissette Exp $
//
// Class: ActiveImageViewMode
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <tlhelp32.h>
#include "ActiveImage.h"
#include "ActiveImageFrame.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageViewMode.h"
#include "RotationDialog.h"
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HGF2DStretch.h>

#include "ActiveImageFunctionMode.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Messages definition
// Define in RC file. ID_.._MODE

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Global Reference Coord Sys
extern HFCPtr<HGF2DCoordSys> g_pAIRefCoordSys;

// Toolbar definition Array
// toolbar buttons - IDs are command buttons
static UINT BASED_CODE ToolbarButtons[] =
{
    ID_VIEW_ZOOMAREA,
    ID_VIEW_ZOOM11,
    ID_VIEW_ZOOMIN,
    ID_VIEW_ZOOMOUT,
    ID_VIEW_FIT,
    ID_VIEW_REDRAW,
    ID_VIEW_MAGNIFIER
};


//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
CToolBar CActiveImageViewMode::m_Toolbar;



//-----------------------------------------------------------------------------
// Public
// Default Constructor
//-----------------------------------------------------------------------------
CActiveImageViewMode::CActiveImageViewMode()
{
    m_CurFunction = 0;
    m_MagnifierState = false;
}


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
CActiveImageViewMode::~CActiveImageViewMode()
{
}






//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
uint32_t CActiveImageViewMode::GetType() const
{
    return (AIFM_NAVIGATION);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnDraw(CDC* pDC)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnUndraw(CDC* pDC)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::Setup()
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CToolBar* CActiveImageViewMode::SetupToolbar(CActiveImageFrame* pi_pFrame,
                                                uint32_t            pi_ID)
{
    CToolBar* pToolbar = 0;
    bool Result;
    HPRECONDITION(pi_pFrame != 0);

    // Create the toolbar
    Result = m_Toolbar.Create(pi_pFrame, 
                              WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY, 
                              pi_ID) &&
             m_Toolbar.LoadBitmap(IDR_NAV_TOOLBAR) &&
             m_Toolbar.SetButtons(ToolbarButtons, 
                                   sizeof(ToolbarButtons)/sizeof(UINT));

    // Setup the toolbar
    if (Result)
    {
        m_Toolbar.SetBarStyle(m_Toolbar.GetBarStyle() | CBRS_FLYBY | CBRS_TOOLTIPS);
        m_Toolbar.SetWindowText(_TEXT("Navigation"));
        pToolbar = &m_Toolbar;
    }


    return (pToolbar);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnLButtonDblClk(uint32_t pi_Flags, 
                                              CPoint& pi_rPoint)
{
}

                                            
//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnLButtonDown(uint32_t pi_Flags, 
                                         CPoint& pi_rPoint)
    {
    switch (m_CurFunction)
        {
        // The users is determining a region to zoom
        case ID_VIEW_ZOOMAREA:
            {
            HFCPtr<HRARaster> pRaster = m_pView->GetSelectedRaster();
            CRectTracker tracker;
            if (pRaster != NULL && tracker.TrackRubberBand(m_pView, pi_rPoint, true))
                {
                RECT rect;
                tracker.GetTrueRect(&rect);     // A properly oriented rect even if rect was drawn bottom -> up.

                HGF2DLocation topLeft(rect.left, rect.top, m_pView->GetCoordSys());
                topLeft.ChangeCoordSys(m_pView->GetLocalCoordSys());

                HGF2DLocation bottomRight(rect.right, rect.bottom, m_pView->GetCoordSys());
                bottomRight.ChangeCoordSys(m_pView->GetLocalCoordSys());

                if(!topLeft.IsEqualToAutoEpsilonSCS(bottomRight))
                    {
                    CRect   DimView;
                    m_pView->GetClientRect(DimView);
                    double scaleFactor = max((bottomRight.GetX() - topLeft.GetX()) / DimView.Width(), 
                        (bottomRight.GetY() - topLeft.GetY()) / DimView.Height());

                    HGF2DStretch Scale(HGF2DDisplacement(topLeft.GetX(), topLeft.GetY()), scaleFactor, scaleFactor);

                    HFCPtr<HGF2DCoordSys> pNewCoordSys(new HGF2DCoordSys(Scale, m_pView->GetLocalCoordSys()));
                    m_pView->SetCoordSys(pNewCoordSys);
                    m_pView->RedrawWindow();
                    }
                }
            break;
            }

            // The user has selectionned a center point to zoom in / zoom out
        case ID_VIEW_ZOOMIN:
        case ID_VIEW_ZOOMOUT:
            {
            CRect   DimView;
            m_pView->GetClientRect(DimView);

            HFCPtr<HGF2DTransfoModel> pLocalToView = m_pView->GetCoordSys()->GetTransfoModelTo(m_pView->GetLocalCoordSys());

            HGF2DLocation newCenter(pi_rPoint.x, pi_rPoint.y, m_pView->GetCoordSys());
            newCenter.ChangeCoordSys(m_pView->GetLocalCoordSys());

            HGF2DLocation oldCenter(DimView.Width()*0.5, DimView.Height()*0.5, m_pView->GetCoordSys());
            oldCenter.ChangeCoordSys(m_pView->GetLocalCoordSys());

            // Recenter view to the requested pt.
            HGF2DTranslation translationToNewCenter(newCenter - oldCenter);
            pLocalToView = pLocalToView->ComposeInverseWithDirectOf(translationToNewCenter);

            // Move center to origin
            HGF2DTranslation transfoToOrigin(HGF2DDisplacement(newCenter.GetX(), newCenter.GetY()));
            pLocalToView = pLocalToView->ComposeInverseWithInverseOf(transfoToOrigin);    // >>> inverseOf !!!

            // Scale
            double scale = ID_VIEW_ZOOMIN == m_CurFunction ? 0.5 : 2.0;
            HFCPtr<HGF2DTransfoModel> pScale = new HGF2DStretch(HGF2DDisplacement(0.0, 0.0), scale, scale);
            pLocalToView = pLocalToView->ComposeInverseWithDirectOf(*pScale);

            // Move back to the center.
            pLocalToView = pLocalToView->ComposeInverseWithDirectOf(transfoToOrigin);

            HFCPtr<HGF2DCoordSys> pNewCoordSys(new HGF2DCoordSys(*pLocalToView, m_pView->GetLocalCoordSys()));
            m_pView->SetCoordSys(pNewCoordSys);
            m_pView->RedrawWindow();
            }            
            break;

        case ID_VIEW_ZOOM11:
            {
            HFCPtr<HRARaster> pRaster = m_pView->GetSelectedRaster();
            if(pRaster != NULL)
                {
                CRect ClientRect;
                m_pView->GetClientRect(ClientRect);

                HGF2DLocation newCenter(pi_rPoint.x, pi_rPoint.y, m_pView->GetCoordSys());
                newCenter.ChangeCoordSys(m_pView->GetLocalCoordSys());

#if (0)
                HGF2DLocation offsetToOrigin(round(ClientRect.Width()*0.5), round(ClientRect.Height()*0.5), m_pView->GetLocalCoordSys());

                HGF2DDisplacement newOrigin;
                newOrigin.SetDeltaX(round(newCenter.GetX() - offsetToOrigin.GetX()));
                newOrigin.SetDeltaY(round(newCenter.GetY() - offsetToOrigin.GetY()));
#else
				// This code is maintained because it causes the maximum
				// remainders in output image placement stressing more effctively the rendering engine.
                HGF2DLocation offsetToOrigin(ClientRect.Width()*0.5, ClientRect.Height()*0.5, m_pView->GetLocalCoordSys());

                HGF2DDisplacement newOrigin = newCenter - offsetToOrigin;
#endif
                HGF2DStretch Scale(newOrigin, 1.0, 1.0);

                HFCPtr<HGF2DCoordSys> pNewCoordSys(new HGF2DCoordSys(Scale, m_pView->GetLocalCoordSys()));
                m_pView->SetCoordSys(pNewCoordSys);
                m_pView->RedrawWindow();
                }
            }
            break;

        default:
            break;
        }
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnLButtonUp  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnRButtonDblClk(uint32_t pi_Flags, 
                                              CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnRButtonDown(uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnRButtonUp  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnMouseMove  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnKeyDown(uint32_t pi_Char, 
                                        uint32_t pi_RepeatCount, 
                                        uint32_t pi_Flags)
{
    // if there is a raster being moved and the character pressed is escape,
	// stop the move
    if (pi_Char == VK_ESCAPE)
    {
        m_CurFunction = 0;

        m_pView->SetFunctionMode(0);
    }

}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::OnKeyUp  (uint32_t pi_Char, 
                                        uint32_t pi_RepeatCount, 
                                        uint32_t pi_Flags)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageViewMode::OnCommand(uint32_t pi_CommandID,
                                      CActiveImageView* pi_pView,
                                      CActiveImageDoc*  pi_pDoc)
{

    HPRECONDITION(pi_pDoc != 0);
    HPRECONDITION(pi_pView != 0);

    m_pDoc = pi_pDoc;
    m_pView = pi_pView;

    bool Result = true;

    const CActiveImageFunctionMode* pFunction=pi_pView->GetFunctionMode();

    switch (pi_CommandID)
    {
        case ID_VIEW_ZOOMAREA:
        case ID_VIEW_ZOOMIN:
        case ID_VIEW_ZOOMOUT:
        case ID_VIEW_ZOOM11:
            if ((pFunction == 0) || (pFunction->GetType() != AIFM_NAVIGATION))
                pi_pView->SetFunctionMode(this);

            m_CurFunction = pi_CommandID;
            break;

        case ID_VIEW_FIT:
            m_CurFunction = 0;
            FitAllToView();
            break;

        case ID_VIEW_REDRAW:
            m_CurFunction = 0;
            m_pView->RedrawWindow();
            break;

        case ID_VIEW_MAGNIFIER:
            m_CurFunction = 0;
            StartMagnifier();
            break;

        case ID_VIEW_NAVIGATIONTOOLBAR:
        {
            CActiveImageFrame* pFrame = (CActiveImageFrame*)AfxGetMainWnd();
            HASSERT(pFrame != 0);
            
            if (m_Toolbar.IsWindowVisible())
                pFrame->ShowControlBar(&m_Toolbar, false, false);
            else
                pFrame->ShowControlBar(&m_Toolbar, true, false);
        }
            break;


        default:
            Result = false;
            break;
    }

    return (Result);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageViewMode::OnCommandUpdate(CCmdUI* pi_pCmdUI,
                                            CActiveImageView* pi_pView,
                                            CActiveImageDoc*  pi_pDoc)
{
    HPRECONDITION(pi_pDoc != 0);
    HPRECONDITION(pi_pView != 0);

    m_pDoc = pi_pDoc;
    m_pView = pi_pView;

    bool Result = true;

    HTREEITEM        SelObject;
    
    switch (pi_pCmdUI->m_nID)
    {
        case ID_VIEW_ZOOMAREA:
        case ID_VIEW_ZOOMIN:
        case ID_VIEW_ZOOMOUT:
        case ID_VIEW_FIT:
        case ID_VIEW_ZOOM11:
        case ID_VIEW_REDRAW:
        
            // Verify if the current selection is the document item
            SelObject = m_pDoc->GetSelectedObject();
            pi_pCmdUI->Enable((SelObject != NULL) && (SelObject != m_pDoc->GetDocumentObject()));
    
            pi_pCmdUI->SetCheck(m_CurFunction == pi_pCmdUI->m_nID);
            break;
       
       case ID_VIEW_MAGNIFIER:
           SelObject = m_pDoc->GetSelectedObject();
           pi_pCmdUI->Enable((SelObject != NULL) && (SelObject != m_pDoc->GetDocumentObject()));

           pi_pCmdUI->SetCheck(m_MagnifierState == true);
           break;

       case ID_VIEW_NAVIGATIONTOOLBAR:
          pi_pCmdUI->SetCheck(m_Toolbar.IsWindowVisible());
          break;

        default:
            Result = false;
            break;
    }

    return (Result);
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::FitAllToView()
    {
    if(m_pView == NULL)
        return;

    HFCPtr<HRARaster> pRaster = m_pView->GetSelectedRaster();
    if(pRaster == NULL)
        return;

    CRect ClientRect;
    m_pView->GetClientRect(&ClientRect);

    HFCPtr<HVEShape> pEffectiveShape = new HVEShape(*pRaster->GetEffectiveShape());
    pEffectiveShape->ChangeCoordSys(m_pView->GetLocalCoordSys());

    HGF2DExtent extent = pEffectiveShape->GetExtent();
    if (extent.IsDefined())
        {
        double scaleX = (extent.GetWidth() - HGLOBAL_EPSILON) / ClientRect.Width();
        double scaleY = (extent.GetHeight() - HGLOBAL_EPSILON) / ClientRect.Height();

        // proportional scale
        double scaleFactor = scaleX > scaleY ? scaleX : scaleY;

        HGF2DStretch fitTransfo(HGF2DDisplacement(extent.GetXMin(), extent.GetYMin()), scaleFactor, scaleFactor);

        HFCPtr<HGF2DCoordSys> pNewCoordSys(new HGF2DCoordSys(fitTransfo, m_pView->GetLocalCoordSys()));
        m_pView->SetCoordSys(pNewCoordSys);

        m_pView->RedrawWindow();
        }
    }


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void CActiveImageViewMode::StartMagnifier()
    {
    if (m_MagnifierState)
        {
        m_MagnifierState = false;


        HANDLE hProcessSnap;
        HANDLE hProcess;
        PROCESSENTRY32 pe32;

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

        pe32.dwSize = sizeof(PROCESSENTRY32);

        if(!Process32First(hProcessSnap, &pe32)){
            CloseHandle(hProcessSnap);     
            }

        do{

            if(!wcscmp(pe32.szExeFile, L"Magnify.exe"))
                {  
                hProcess = OpenProcess(PROCESS_TERMINATE,0, pe32.th32ProcessID); 
                TerminateProcess(hProcess,0);  
                CloseHandle(hProcess); 
                } 
            }while(Process32Next(hProcessSnap,&pe32));  

        CloseHandle(hProcessSnap);  
        }
    else
        {
        m_MagnifierState = true;
        ShellExecute(0, 0, L"C:\\Windows\\system32\\magnify.exe", 0, 0, SW_SHOWDEFAULT);
        }  
    }

//-----------------------------------------------------------------------------
// Public
// GetCursor
//-----------------------------------------------------------------------------
HCURSOR CActiveImageViewMode::GetCursor() const
{

    switch (m_CurFunction)
    {
        case ID_VIEW_ZOOMAREA:
            return AfxGetApp()->LoadStandardCursor(IDC_CROSS);
            break;

        case ID_VIEW_ZOOMIN:
            return AfxGetApp()->LoadCursor(IDC_ZOOM_IN);
            break;

        case ID_VIEW_ZOOMOUT:
            return AfxGetApp()->LoadCursor(IDC_ZOOM_OUT);
            break;

        default:
            return AfxGetApp()->LoadStandardCursor(IDC_ARROW);
            break;
    }
}


void CActiveImageViewMode::EndCommand ()
{
    m_CurFunction = 0;
}