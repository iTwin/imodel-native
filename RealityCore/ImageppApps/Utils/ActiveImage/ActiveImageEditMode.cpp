/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageEditMode.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageEditMode.cpp,v 1.12 2011/07/18 21:10:39 Donald.Morissette Exp $
//
// Class: ActiveImageEditMode
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "ActiveImage.h"
#include "ActiveImageFrame.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageEditMode.h"
#include "RotationDialog.h"
#include "LineTracker.h"
#include <Imagepp/all/h/HVEShape.h>
#include <Imagepp/all/h/HVE2DEllipse.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DGrid.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include "ActiveImageFunctionMode.h"
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRAClearOptions.h>
#include <Imagepp/all/h/HRATransaction.h>
#include <Imagepp/all/h/HRATransactionRecorder.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <ImagePP/all/h/HRACopyFromOptions.h>

#include <Imagepp/all/h/HRAPyramidRaster.h>


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BITMAP_ROWALIGNMENT  4
#define BITMAP_ROWBITS      (BITMAP_ROWALIGNMENT*8)
#define WIDTHBYTES(i)       (((i) + BITMAP_ROWBITS-1)/BITMAP_ROWBITS * BITMAP_ROWALIGNMENT)

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
    ID_EDIT_CLEAR_MODE,
    ID_EDIT_COPY_MODE,
    ID_EDIT_PASTE_MODE,
    ID_EDIT_ADD_RECT_SHAPE,
    ID_EDIT_ADD_RECT_HOLED_SHAPE,
    ID_EDIT_ADD_CIRC_SHAPE,
    ID_EDIT_ADD_CIRC_HOLED_SHAPE,
    ID_EDIT_ADD_POLY_SHAPE,
    ID_EDIT_ADD_POLY_HOLED_SHAPE,
    ID_EDIT_REMOVE_CLIPPING,
    ID_EDIT_CHANGESUBSAMPLING_NEAREST,
    ID_EDIT_CHANGESUBSAMPLING_AVERAGE
};


//-----------------------------------------------------------------------------
// Static Variables
//-----------------------------------------------------------------------------
CToolBar CActiveImageEditMode::m_Toolbar;



//-----------------------------------------------------------------------------
// Public
// Default Constructor
//-----------------------------------------------------------------------------
CActiveImageEditMode::CActiveImageEditMode()
{
    m_CurFunction = 0;
}


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
CActiveImageEditMode::~CActiveImageEditMode()
{
}






//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
uint32_t CActiveImageEditMode::GetType() const
{
    return (AIFM_EDITCOPYPASTE);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnDraw(CDC* pDC)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnUndraw(CDC* pDC)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::Setup()
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnLButtonDblClk(uint32_t pi_Flags, 
                                              CPoint& pi_rPoint)
{
}
                                            
//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnLButtonDown(uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
    HTREEITEM         SelObject;
    HFCPtr<HRARaster> pSelectedObject;
    
    // Verify if the current selection is the document item
    SelObject = m_pDoc->GetSelectedObject();
    if ((SelObject != NULL) &&
        (SelObject != m_pDoc->GetDocumentObject()))
    {
        // get the currently selected object from the document
        pSelectedObject = m_pDoc->GetRaster(SelObject);
        HASSERT(pSelectedObject != 0);
    }
    else
        goto WRAPUP;        


    switch (m_CurFunction)
    {
        case ID_MENU_EDIT_MOVEREFERENCETORASTER:
            {
            KLineTracker tracker;
	        if(tracker.TrackRubberBand(m_pView, pi_rPoint))
                {
                POINT begin, end;
                tracker.GetPos(&begin, &end);

                HGF2DLocation startLocation = m_pView->GetLocation(begin);
                HGF2DLocation endLocation = m_pView->GetLocation(end);

                double offsetX = endLocation.GetX()-startLocation.GetX();
                double offsetY = endLocation.GetY()-startLocation.GetY();

                HGF2DDisplacement trans(offsetX, offsetY);

                HRAReferenceToRaster* pRefToRaster = dynamic_cast<HRAReferenceToRaster*>(pSelectedObject.GetPtr());
                if(pRefToRaster == NULL)
                    {
                    pRefToRaster = new HRAReferenceToRaster(pSelectedObject);
                    HFCPtr<HRARaster> pRaster = pRefToRaster;
                    m_pDoc->ReplaceRaster(SelObject, pRaster);
                    }

                pRefToRaster->Move(trans);

                m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                }

            }
            break;


        case ID_EDIT_CLEAR_MODE:
        case ID_EDIT_COPY_MODE:
        case ID_EDIT_ADD_RECT_SHAPE:
        case ID_EDIT_ADD_RECT_HOLED_SHAPE:
        {
            CRectTracker tracker;
	        if (tracker.TrackRubberBand(m_pView, pi_rPoint, true))
            {
                CRect rc (tracker.m_rect); 
                
                rc.NormalizeRect();
                
                HGF2DLocation MinCorner = m_pView->GetLocation(rc.TopLeft());
                HGF2DLocation MaxCorner = m_pView->GetLocation(rc.BottomRight());

                HVEShape RectShape(min(MinCorner.GetX(), MaxCorner.GetX()),
                                   min(MinCorner.GetY(), MaxCorner.GetY()),
                                   max(MinCorner.GetX(), MaxCorner.GetX()),
                                   max(MinCorner.GetY(), MaxCorner.GetY()),
                                   MinCorner.GetCoordSys());


                if (m_CurFunction == ID_EDIT_CLEAR_MODE)
                {
                    if (pSelectedObject->IsCompatibleWith(HRAStoredRaster::CLASS_ID))                    
                    {
                        HFCPtr<HRAStoredRaster> pStoredRaster((HFCPtr<HRAStoredRaster>&)pSelectedObject);
                        bool CommitTransaction = false;
                        if (pStoredRaster->GetTransactionRecorder() != 0)
                        {
                            pStoredRaster->StartTransaction();
                            CommitTransaction = true;
                        }                      
                        
                        if (pStoredRaster->IsResizable())
                        {
                            HGF2DExtent RasterExtent(pStoredRaster->GetRasterExtent());
                            RectShape.ChangeCoordSys(pStoredRaster->GetPhysicalCoordSys());
                            HGF2DGrid Grid(RectShape.GetExtent());
                            
                            if (Grid.GetXMin() < RasterExtent.GetXMin() ||
                                Grid.GetYMin() < RasterExtent.GetYMin() ||
                                Grid.GetXMax() > RasterExtent.GetXMax() ||
                                Grid.GetYMax() > RasterExtent.GetYMax())
                            {
                                RasterExtent.SetXMin(MIN(Grid.GetXMin(), RasterExtent.GetXMin()));
                                RasterExtent.SetYMin(MIN(Grid.GetYMin(), RasterExtent.GetYMin()));
                                RasterExtent.SetXMax(MAX(Grid.GetXMax(), RasterExtent.GetXMax()));
                                RasterExtent.SetYMax(MAX(Grid.GetYMax(), RasterExtent.GetYMax()));

                                pStoredRaster->SetRasterExtent(RasterExtent);
                            }
                        }

                        RectShape.Intersect(*pStoredRaster->GetEffectiveShape());

                        size_t NumberOfBytes = (pStoredRaster->GetPixelType()->CountPixelRawDataBits() + 7) / 8;
                        Byte* pDefaultRawData = (Byte*)_alloca(NumberOfBytes);

                        COLORREF defaultColor = m_pView->GetDefaultColor();
                        HRPPixelTypeV24R8G8B8().GetConverterTo(pStoredRaster->GetPixelType())->Convert(&defaultColor, pDefaultRawData);

                        HRAClearOptions Options;
                        Options.SetShape(&RectShape);
                        Options.SetApplyRasterClipping(false);
                        Options.SetRawDataValue(pDefaultRawData);
                        pStoredRaster->Clear(Options);

                        if (CommitTransaction)
                            pStoredRaster->EndTransaction();    
                    }
                    else
                        {
                        size_t NumberOfBytes = (pSelectedObject->GetPixelType()->CountPixelRawDataBits() + 7) / 8;
                        Byte* pDefaultRawData = (Byte*)_alloca(NumberOfBytes);

                        COLORREF defaultColor = m_pView->GetDefaultColor();
                        HRPPixelTypeV24R8G8B8().GetConverterTo(pSelectedObject->GetPixelType())->Convert(&defaultColor, pDefaultRawData);

                        HRAClearOptions Options;
                        Options.SetShape(&RectShape);
                        Options.SetApplyRasterClipping(false);
                        Options.SetRawDataValue(pDefaultRawData);
                        pSelectedObject->Clear(Options);
                        }
                    m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                }
                else if (m_CurFunction == ID_EDIT_COPY_MODE)
                {
                    // Clipborad Disponible
                    if (m_pView->OpenClipboard())
                    {   
                        HGF2DStretch flipModel(HGF2DDisplacement(0.0, rc.Height()), 1.0, -1.0);
                        HFCPtr<HRABitmap> pDibRaster =  HRABitmap::Create (rc.Width(), 
                                                                      rc.Height(), 
                                                                      &flipModel, 
                                                                      m_pView->GetCoordSys(),
                                                                      new HRPPixelTypeV24B8G8R8(),
                                                                      32); 

                        RectShape.ChangeCoordSys(pDibRaster->GetCoordSys());
                        pDibRaster->Move(HGF2DDisplacement(RectShape.GetExtent().GetXMin(), 
                                                           RectShape.GetExtent().GetYMin()));

                        COLORREF DefaultColor = m_pView->GetDefaultColor();
                        uint32_t bgr;
                        ((Byte*)&bgr)[0] = ((Byte*)&DefaultColor)[2];
                        ((Byte*)&bgr)[1] = ((Byte*)&DefaultColor)[1];
                        ((Byte*)&bgr)[2] = ((Byte*)&DefaultColor)[0];

                        HRAClearOptions clearOpts;
                        clearOpts.SetRawDataValue(&bgr);
                        pDibRaster->Clear(clearOpts);

                        pDibRaster->CopyFrom(*pSelectedObject, HRACopyFromOptions());
                       
                        assert(WIDTHBYTES((DWORD)rc.Width() * 24) * (DWORD)rc.Height() == pDibRaster->GetPacket()->GetDataSize());     // Calculate a buffer size where each line is DWORD aligned.

                        BITMAPINFOHEADER dib;
                        memset(&dib, 0, sizeof(BITMAPINFOHEADER));
                        dib.biSize          = sizeof(BITMAPINFOHEADER);
                        dib.biWidth         = rc.Width();
                        dib.biHeight        = rc.Height();
                        dib.biPlanes        = 1;
                        dib.biBitCount      = 24;
                        dib.biCompression   = BI_RGB;
                        dib.biSizeImage     = (DWORD)pDibRaster->GetPacket()->GetDataSize();
                        
                        HANDLE hMem = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + pDibRaster->GetPacket()->GetDataSize());
                        Byte* clipDataP = static_cast<Byte*>(GlobalLock (hMem));
                        memcpy (clipDataP, &dib, sizeof(BITMAPINFOHEADER));
                        memcpy (clipDataP + sizeof(BITMAPINFOHEADER), pDibRaster->GetPacket()->GetBufferAddress(), pDibRaster->GetPacket()->GetDataSize());
                        GlobalUnlock (hMem);

                        EmptyClipboard();
                        SetClipboardData(CF_DIB, hMem);
                        CloseClipboard();
                    }
                }
                else if (m_CurFunction == ID_EDIT_ADD_RECT_SHAPE)
                {
                    HVEShape Shape(pSelectedObject->GetShape());
                    Shape.Intersect(RectShape);

                    pSelectedObject->SetShape (Shape);
                    m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                }
                else if (m_CurFunction == ID_EDIT_ADD_RECT_HOLED_SHAPE)
                {
                    HVEShape Shape(pSelectedObject->GetShape());
                    Shape.Differentiate (RectShape);

                    pSelectedObject->SetShape (Shape);
                    m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                }
            }
            m_pDoc->SetModifiedFlag();
        }
        break;

        case ID_EDIT_ADD_CIRC_SHAPE:
        case ID_EDIT_ADD_CIRC_HOLED_SHAPE:
        {
        CRectTracker tracker;
        if (tracker.TrackRubberBand(m_pView, pi_rPoint, true))
            {
            CRect rc (tracker.m_rect); 
            rc.NormalizeRect();

            HGF2DLocation MinCorner = m_pView->GetLocation(rc.TopLeft());
            HGF2DLocation MaxCorner = m_pView->GetLocation(rc.BottomRight());

            HVE2DEllipse ellipse(HVE2DRectangle(min(MinCorner.GetX(), MaxCorner.GetX()),
                                                min(MinCorner.GetY(), MaxCorner.GetY()),
                                                max(MinCorner.GetX(), MaxCorner.GetX()),
                                                max(MinCorner.GetY(), MaxCorner.GetY()),
                                                MinCorner.GetCoordSys()));
            HVE2DPolygon polygonEllipse(ellipse.GetLinear());
            HVEShape ellipseShape(polygonEllipse);

            if (m_CurFunction == ID_EDIT_ADD_CIRC_SHAPE)
                {
                HVEShape Shape(pSelectedObject->GetShape());
                Shape.Intersect(ellipseShape);

                pSelectedObject->SetShape (Shape);
                m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                }
            else if (m_CurFunction == ID_EDIT_ADD_CIRC_HOLED_SHAPE)
                {
                HVEShape Shape(pSelectedObject->GetShape());
                Shape.Differentiate (ellipseShape);

                pSelectedObject->SetShape (Shape);
                m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                }
            }
        }
        break;

        case ID_EDIT_ADD_POLY_SHAPE:
        case ID_EDIT_ADD_POLY_HOLED_SHAPE:
        {
        HVE2DPolySegment AddPolySegment(m_pView->GetCoordSys());

        GetPolygonPoints(pSelectedObject->GetExtentInCs(m_pView->GetCoordSys()));

        for(UINT32 i = 0; i < m_polygonPoints.size(); i++)
            AddPolySegment.AppendPoint(m_polygonPoints[i]);

        //Close the polygon
        AddPolySegment.AppendPoint(m_polygonPoints[0]);

        HVE2DShape* p2DShape;
        p2DShape = new HVE2DPolygonOfSegments(AddPolySegment);
        HFCPtr<HVEShape> pPolyShape = new HVEShape(*p2DShape);

        if (m_CurFunction == ID_EDIT_ADD_POLY_SHAPE)
            {
            HVEShape Shape(pSelectedObject->GetShape());
            Shape.Intersect(*pPolyShape);

            pSelectedObject->SetShape (Shape);
            m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
            }
        else if (m_CurFunction == ID_EDIT_ADD_POLY_HOLED_SHAPE)
            {
            HVEShape Shape(pSelectedObject->GetShape());
            Shape.Differentiate (*pPolyShape);

            pSelectedObject->SetShape (Shape);
            m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
            }
        }
        break;

        case ID_EDIT_PASTE_MODE:
        {
            // Clipborad Disponible
            if (m_pView->OpenClipboard())
            {   
                HANDLE HClipB;
                if ((HClipB = GetClipboardData(CF_DIB)) != NULL)
                {
                   LPBITMAPINFO pBitmapInfo = (LPBITMAPINFO)::GlobalLock ((HGLOBAL)HClipB);
          
                    // Check if 24 bits and RGB
                    if ((pBitmapInfo->bmiHeader.biBitCount == 24) &&
                        (pBitmapInfo->bmiHeader.biCompression == BI_RGB))
                    {
                        HGF2DLocation MinCorner = m_pView->GetLocation(pi_rPoint);

                        // Create the model between Image and Scroll
                        HGF2DStretch flipModel(HGF2DDisplacement(0.0, pBitmapInfo->bmiHeader.biHeight), 1.0, -1.0);

                        HFCPtr<HRABitmap> pDibRaster = HRABitmap::Create (pBitmapInfo->bmiHeader.biWidth,
                                                                      pBitmapInfo->bmiHeader.biHeight,                                                                     
                                                                      &flipModel,
                                                                      m_pView->GetCoordSys(),
                                                                      new HRPPixelTypeV24B8G8R8(),
                                                                      32);
                                  
                        MinCorner.ChangeCoordSys(pDibRaster->GetCoordSys());
                        pDibRaster->Move(HGF2DDisplacement(MinCorner.GetX(), MinCorner.GetY()));

                        // set the data
                        HFCPtr<HCDPacket> pPacket = pDibRaster->GetPacket();

                        size_t dataSize = pBitmapInfo->bmiHeader.biSizeImage;
                        if(0 == dataSize)  // DataSize is optional when uncompressed.
                            {
                            size_t BytesPerRow = (pBitmapInfo->bmiHeader.biWidth * pDibRaster->GetPixelType()->CountPixelRawDataBits() + 31) / 32;
                            BytesPerRow = (BytesPerRow * 32 + 7) / 8;
                            dataSize = BytesPerRow * pBitmapInfo->bmiHeader.biHeight;
                            }

                        // set the buffer into the packet
                        pPacket->SetBuffer(pBitmapInfo->bmiColors, dataSize);
                        pPacket->SetDataSize(dataSize);

                        // set packet has a ownership of the buffer
                        pPacket->SetBufferOwnership(false);

                        if (pSelectedObject->IsCompatibleWith(HRAStoredRaster::CLASS_ID))                    
                        {
                            bool CommitTransaction = false;
                            if (((HFCPtr<HRAStoredRaster>&)pSelectedObject)->GetTransactionRecorder() != 0)
                            {
                                ((HFCPtr<HRAStoredRaster>&)pSelectedObject)->StartTransaction();
                                CommitTransaction = true;
                            }

                            pSelectedObject->CopyFrom(*pDibRaster, HRACopyFromOptions());
                            if (CommitTransaction)
                                ((HFCPtr<HRAStoredRaster>&)pSelectedObject)->EndTransaction();
                        }
                        else
                        {
                            pSelectedObject->CopyFrom(*pDibRaster, HRACopyFromOptions());
                        }


                        m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
                    }
 
                   ::GlobalUnlock ((HGLOBAL)HClipB);
                }

                CloseClipboard();
            }
            m_pDoc->SetModifiedFlag();            
        }
        break;

    }


WRAPUP:
    // Cancel the Function
    m_CurFunction = 0;
    m_pView->SetFunctionMode(0);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnLButtonUp  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnRButtonDblClk(uint32_t pi_Flags, 
                                              CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnRButtonDown(uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnRButtonUp  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnMouseMove  (uint32_t pi_Flags, 
                                            CPoint& pi_rPoint)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageEditMode::OnKeyDown(uint32_t pi_Char, 
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
void CActiveImageEditMode::OnKeyUp  (uint32_t pi_Char, 
                                        uint32_t pi_RepeatCount, 
                                        uint32_t pi_Flags)
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageEditMode::OnCommandUpdate(CCmdUI* pi_pCmdUI,
                                            CActiveImageView* pi_pView,
                                            CActiveImageDoc*  pi_pDoc)
{
    HPRECONDITION(pi_pDoc != 0);
    HPRECONDITION(pi_pView != 0);

    m_pDoc = pi_pDoc;
    m_pView = pi_pView;
    bool Result = true;
    HTREEITEM SelObject;

    switch (pi_pCmdUI->m_nID)
    {
        case ID_EDIT_CLEAR_MODE:
        case ID_EDIT_PASTE_MODE:
            pi_pCmdUI->Enable(!m_pDoc->IsReadOnly());
            break;

        case ID_MENU_EDIT_ADDREFERENCETORASTER:
        case ID_MENU_EDIT_MOVEREFERENCETORASTER:
        case ID_MENU_EDIT_UNDO:
        case ID_MENU_EDIT_REDO:

        case ID_EDIT_COPY_MODE:

        case ID_EDIT_ADD_RECT_SHAPE:
        case ID_EDIT_ADD_RECT_HOLED_SHAPE:
        case ID_EDIT_ADD_CIRC_SHAPE:
        case ID_EDIT_ADD_CIRC_HOLED_SHAPE:
        case ID_EDIT_ADD_POLY_SHAPE:
        case ID_EDIT_ADD_POLY_HOLED_SHAPE:
            pi_pCmdUI->Enable(true);
            break;

        case ID_EDIT_REMOVE_CLIPPING:
            SelObject = m_pDoc->GetSelectedObject();
            if ((SelObject == NULL) || (SelObject == m_pDoc->GetDocumentObject()))
                pi_pCmdUI->Enable(false);
            else
                pi_pCmdUI->Enable(true);
            break;

        case ID_EDIT_CHANGESUBSAMPLING_NEAREST:
        case ID_EDIT_CHANGESUBSAMPLING_AVERAGE:
            SelObject = m_pDoc->GetSelectedObject();
            if ((SelObject == NULL) || (SelObject == m_pDoc->GetDocumentObject()))
                pi_pCmdUI->Enable(false);
            else
                pi_pCmdUI->Enable(true);

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
bool CActiveImageEditMode::OnCommand(uint32_t pi_CommandID,
                                      CActiveImageView* pi_pView,
                                      CActiveImageDoc*  pi_pDoc)
{
    HPRECONDITION(pi_pDoc != 0);
    HPRECONDITION(pi_pView != 0);

    m_pDoc = pi_pDoc;
    m_pView = pi_pView;

    bool Result = true;

    switch (pi_CommandID)
    {
        case ID_MENU_EDIT_ADDREFERENCETORASTER:
            {
            HTREEITEM SelObject;
            SelObject = m_pDoc->GetSelectedObject();
            if (SelObject != NULL && SelObject != m_pDoc->GetDocumentObject())
                {
                //Get the currently selected object from the document
                HFCPtr<HRARaster> pRaster = new HRAReferenceToRaster(m_pDoc->GetRaster(SelObject));
                m_pDoc->ReplaceRaster(SelObject, pRaster);
                }
            }
            break;

        case ID_MENU_EDIT_UNDO:
        case ID_MENU_EDIT_REDO:
        {
            HTREEITEM SelObject;
            SelObject = m_pDoc->GetSelectedObject();
            if ((SelObject == NULL) || (SelObject == m_pDoc->GetDocumentObject()))
            {
                HASSERT(0);
            }

            HFCPtr<HRARaster> pSelectedObject;

            // get the currently selected object from the document
            pSelectedObject = m_pDoc->GetRaster(SelObject);
            HASSERT(pSelectedObject != 0);
            if (pSelectedObject->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
            {
                if (pi_CommandID == ID_MENU_EDIT_UNDO)
                    ((HFCPtr<HRAStoredRaster>&)pSelectedObject)->Undo();
                else
                    ((HFCPtr<HRAStoredRaster>&)pSelectedObject)->Redo();

                m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
            }
        }
            break;


        case ID_EDIT_CLEAR_MODE:
        case ID_EDIT_COPY_MODE:
        case ID_EDIT_PASTE_MODE:
        case ID_EDIT_ADD_RECT_SHAPE:
        case ID_EDIT_ADD_RECT_HOLED_SHAPE:
        case ID_EDIT_ADD_CIRC_SHAPE:
        case ID_EDIT_ADD_CIRC_HOLED_SHAPE:
        case ID_EDIT_ADD_POLY_SHAPE:
        case ID_EDIT_ADD_POLY_HOLED_SHAPE:
        case ID_MENU_EDIT_MOVEREFERENCETORASTER:
        {
            const CActiveImageFunctionMode* pFunction=pi_pView->GetFunctionMode();

            if ((pFunction == 0) || (pFunction->GetType() != AIFM_EDITCOPYPASTE))
                pi_pView->SetFunctionMode(this);

            m_CurFunction = pi_CommandID;
        }
            break;

        case ID_EDIT_REMOVE_CLIPPING:
        {
            HTREEITEM SelObject;
            SelObject = m_pDoc->GetSelectedObject();
            if ((SelObject == NULL) || (SelObject == m_pDoc->GetDocumentObject()))
                HASSERT(0);
            
            //Get the currently selected object from the document
            HFCPtr<HRARaster> pRaster = m_pDoc->GetRaster(SelObject);
            if (pRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
                pRaster->SetShape(HVEShape(((HFCPtr<HRAStoredRaster>&)pRaster)->GetPhysicalExtent()));
            else
                pRaster->SetShape(HVE2DUniverse(pRaster->GetCoordSys()));

            m_pView->OnUpdate(0, UPDATE_REDRAW | UPDATE_KEEPZOOM, 0);
        }
        break;

        case ID_EDIT_CHANGESUBSAMPLING_NEAREST:
        case ID_EDIT_CHANGESUBSAMPLING_AVERAGE:
        {
            HTREEITEM SelObject;
            SelObject = m_pDoc->GetSelectedObject();
            if ((SelObject == NULL) || (SelObject == m_pDoc->GetDocumentObject()))
            {
                HASSERT(0);
            }
                
            HFCPtr<HRARaster> pSelectedObject;
    
            // get the currently selected object from the document
            pSelectedObject = m_pDoc->GetRaster(SelObject);
            HASSERT(pSelectedObject != 0);

            if (pSelectedObject->IsCompatibleWith(HRAPyramidRaster::CLASS_ID))
            {
                HGSResampling::ResamplingMethod ResamplingMethod;
                if (pi_CommandID == ID_EDIT_CHANGESUBSAMPLING_NEAREST)
                    ResamplingMethod = HGSResampling::NEAREST_NEIGHBOUR;
                else
                    ResamplingMethod = HGSResampling::AVERAGE;

                ((HFCPtr<HRAPyramidRaster>&)pSelectedObject)->
                        SetResamplingForSubResolution(ResamplingMethod,
                                                      true);
            }
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
// GetCursor
//-----------------------------------------------------------------------------
HCURSOR CActiveImageEditMode::GetCursor() const
{
    // return the default cursor, The Arrow
    return AfxGetApp()->LoadStandardCursor(IDC_CROSS);
}

void CActiveImageEditMode::EndCommand ()
{
    m_CurFunction = 0;
}

void CActiveImageEditMode::GetPolygonPoints(const HGF2DExtent& pi_extent)
{
    bool    finished = false;
    UINT32  numberOfPoints = 0;

    m_polygonPoints.clear();

    // Set the pen
    CPen ContourPen(PS_INSIDEFRAME, 2, RGB(255, 0, 0));
    CClientDC dc(m_pView);
    dc.SelectObject(&ContourPen);

    CPoint initialCursorPosition;
    do
        {
        //This is to remove the default windows idle mode (window not responding) that happens within about 5 seconds
        //because this is an infinite loop until the right mouse button is clicked
        MSG msg;
        PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE);
        
        //Check for mouse left button click
        if (GetAsyncKeyState(VK_LBUTTON) < 0)
            {
            CPoint cursorPosition;
            GetCursorPos(&cursorPosition);
            //Get cursor location relative to the ActiveImage window
            if (ScreenToClient(m_pView->GetSafeHwnd(), &cursorPosition))
                {
                HGF2DLocation currentCursorLocation = HGF2DLocation(m_pView->GetLocation(cursorPosition));
        
                //Check if location is within the raster extent before adding it
                if(0 == numberOfPoints && pi_extent.IsPointIn(currentCursorLocation))
                    {
                    initialCursorPosition.x = cursorPosition.x;
                    initialCursorPosition.y = cursorPosition.y;
                    m_polygonPoints.push_back(currentCursorLocation);
                    numberOfPoints++;
                    dc.MoveTo(cursorPosition.x, cursorPosition.y);
                    }
                //Avoid duplicate coordinates
                else if(numberOfPoints > 0 && m_polygonPoints[numberOfPoints - 1] != currentCursorLocation && pi_extent.IsPointIn(currentCursorLocation))
                    {
                    m_polygonPoints.push_back(currentCursorLocation);
                    dc.LineTo(cursorPosition.x, cursorPosition.y);
                    dc.MoveTo(cursorPosition.x, cursorPosition.y);
                    numberOfPoints++;
                    }
                }
            } 
        //Check for mouse right button click
        else if (GetAsyncKeyState(VK_RBUTTON) < 0)
            {
            finished = true;
            dc.LineTo(initialCursorPosition.x, initialCursorPosition.y);
            }

        } while (!finished);
    }
