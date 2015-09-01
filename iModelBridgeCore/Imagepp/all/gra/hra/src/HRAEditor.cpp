//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hra/src/HRAEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
// Class HRAEditor
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRAEditor.h>

#include <Imagepp/all/h/HRASurface.h>
#include <Imagepp/all/h/HGSMemorySurfaceDescriptor.h>
#include <Imagepp/all/h/HGSMemoryRLESurfaceDescriptor.h>
#include <Imagepp/all/h/HRAEditorRLE1Line.h>
#include <Imagepp/all/h/HRAEditorRLE1.h>
#include <Imagepp/all/h/HRAEditorN8.h>
#include <Imagepp/all/h/HRAEditorN1.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8RLE.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8RLE.h>
#include <Imagepp/all/h/HGFScanLines.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HRAGenEditor.h>
#include <Imagepp/all/h/HGSRegion.h>
#include <Imagepp/all/h/HVEShape.h>


//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HRAEditor::HRAEditor(HRASurface& pi_surface, bool gridMode)
    :m_surface(pi_surface)
    {
    HPRECONDITION(pi_surface.GetSurfaceDescriptor() != 0);
    HPRECONDITION(pi_surface.GetSurfaceDescriptor()->IsCompatibleWith(HGSMemoryBaseSurfaceDescriptor::CLASS_ID));

    // compute some useful information
    const HFCPtr<HGSMemoryBaseSurfaceDescriptor>& rpDescriptor =
        (const HFCPtr<HGSMemoryBaseSurfaceDescriptor>&)pi_surface.GetSurfaceDescriptor();

    m_Height    = rpDescriptor->GetHeight();
    m_Width     = rpDescriptor->GetWidth();

    // test if there is a region
    HFCPtr<HGSRegion> pClipRegion(pi_surface.GetRegion());
    m_pScanLines = 0;
    if (pClipRegion != 0)
        {
        if (pClipRegion->IsScanlinesShape())
            m_pScanLines = const_cast<HGFScanLines*>(pClipRegion->GetScanlines());
        else
            {
            HFCPtr<HVEShape> pShape = pClipRegion->GetShape();
            HGF2DExtent ShapeExtent(pShape->GetExtent());
            if (ShapeExtent.GetXMin() < 0.0 ||
                ShapeExtent.GetYMin() < 0.0 ||
                ShapeExtent.GetXMax() > (double)m_Width ||
                ShapeExtent.GetYMax() > (double)m_Height)
                pShape->Intersect(HGF2DExtent(0.0,
                                              0.0,
                                              (double)m_Width,
                                              (double)m_Height,
                                              pShape->GetCoordSys()));

            m_pScanLinesSmartPtr = new HGFScanLines(gridMode);
            m_pScanLines = m_pScanLinesSmartPtr.get();
            if(!pShape->IsEmpty())
                pShape->GenerateScanLines(*m_pScanLines);
            else
                m_pScanLines->SetLimits(0,0,0); // Generate empty scanlines.
            }
        }

    if(rpDescriptor->IsCompatibleWith(HGSMemoryRLESurfaceDescriptor::CLASS_ID))
        {
        m_pEditor = new HRAEditorRLE1Line(*(const HFCPtr<HGSMemoryRLESurfaceDescriptor>&)rpDescriptor);
        }
    else
        {
        HASSERT(rpDescriptor->IsCompatibleWith(HGSMemorySurfaceDescriptor::CLASS_ID));

        // determine if this is an N8 or RLE1 editor, filtered or not
        if (rpDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8A8RLE::CLASS_ID) ||
            rpDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeI1R8G8B8RLE::CLASS_ID))
            m_pEditor = new HRAEditorRLE1(*(const HFCPtr<HGSMemorySurfaceDescriptor>&)rpDescriptor);
        else if (rpDescriptor->GetPixelType()->CountPixelRawDataBits() < 8)
            m_pEditor = new HRAEditorN1(*(const HFCPtr<HGSMemorySurfaceDescriptor>&)rpDescriptor);
        else
            m_pEditor = new HRAEditorN8(*(const HFCPtr<HGSMemorySurfaceDescriptor>&)rpDescriptor);
        }
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRAEditor::~HRAEditor()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRASurface& HRAEditor::GetSurface() const {return m_surface;}

//-----------------------------------------------------------------------------
// public
// GetFirstRun
//-----------------------------------------------------------------------------
void* HRAEditor::GetFirstRun(HUINTX*    po_pStartPosX,
                             HUINTX*    po_pStartPosY,
                             size_t*    po_pPixelCount,
                             void*      pi_pTransaction) const
    {
    HPRECONDITION(po_pStartPosX != 0);
    HPRECONDITION(po_pStartPosY != 0);
    HPRECONDITION(po_pPixelCount != 0);

    void* pRawData = 0;

    // test if there is a shape
    if (m_pScanLines != 0)
        {
        // goto the first run
        if (m_pScanLines->GotoFirstRun())
            {
#ifdef __HMR_DEBUG
            HSINTX PosX;
            HSINTX PosY;

            m_pScanLines->GetCurrentRun(&PosX,
                                        &PosY,
                                        po_pPixelCount);
            HPOSTCONDITION(PosX >= 0L && PosY >= 0L && (uint32_t)PosY < m_Height);
            HPOSTCONDITION(PosX + *po_pPixelCount <= m_Width);
#endif // __HMR_DEBUG

            // get the run
            m_pScanLines->GetCurrentRun((HSINTX*)po_pStartPosX,
                                        (HSINTX*)po_pStartPosY,
                                        po_pPixelCount);

            pRawData = m_pEditor->GetRun(*po_pStartPosX,
                                         *po_pStartPosY,
                                         *po_pPixelCount,
                                         pi_pTransaction);
            }
        }
    else
        {
        // goto the first line
        m_PosY = 0;

        // fill the information
        *po_pStartPosX = 0;
        *po_pStartPosY = m_PosY;
        *po_pPixelCount = m_Width;

        // get the run
        pRawData = m_pEditor->GetRun(0L,
                                     0L,
                                     m_Width,
                                     pi_pTransaction);
        }

    return pRawData;
    }



//-----------------------------------------------------------------------------
// public
// GetNextRun
//-----------------------------------------------------------------------------
void* HRAEditor::GetNextRun(HUINTX* po_pStartPosX,
                            HUINTX* po_pStartPosY,
                            size_t* po_pPixelCount,
                            void*   pi_pTransaction) const
    {
    HPRECONDITION(po_pStartPosX != 0);
    HPRECONDITION(po_pStartPosY != 0);
    HPRECONDITION(po_pPixelCount != 0);

    void* pRawData = 0;

    // test if there is a shape
    if (m_pScanLines != 0)
        {
        // goto the next run
        if (m_pScanLines->GotoNextRun())
            {
#ifdef __HMR_DEBUG
            HSINTX PosX;
            HSINTX PosY;

            m_pScanLines->GetCurrentRun(&PosX,
                                        &PosY,
                                        po_pPixelCount);
            HPOSTCONDITION(PosX >= 0L && PosY >= 0L && (uint32_t)PosY < m_Height);
            HPOSTCONDITION(PosX + *po_pPixelCount <= m_Width);
#endif // __HMR_DEBUG

            // get the run
            m_pScanLines->GetCurrentRun((HSINTX*)po_pStartPosX,
                                        (HSINTX*)po_pStartPosY,
                                        po_pPixelCount);

            pRawData = m_pEditor->GetRun(*po_pStartPosX,
                                         *po_pStartPosY,
                                         *po_pPixelCount,
                                         pi_pTransaction);
            }
        }
    else
        {
        // goto the next line
        m_PosY++;

        // test if we reach the height of the image
        if (m_PosY < m_Height)
            {
            // fill the information
            *po_pStartPosX = 0;
            *po_pStartPosY = m_PosY;
            *po_pPixelCount = m_Width;

            // get the run
            pRawData = m_pEditor->GetRun(0L,
                                         *po_pStartPosY,
                                         *po_pPixelCount,
                                         pi_pTransaction);
            }
        }

    return pRawData;
    }

//-----------------------------------------------------------------------------
// public
// GetRun
//-----------------------------------------------------------------------------
void* HRAEditor::GetRun(HUINTX  pi_StartPosX,
                        HUINTX  pi_StartPosY,
                        size_t  pi_PixelCount,
                        void*   pi_pTransaction) const
    {
    HPRECONDITION(pi_StartPosX >= 0L && pi_StartPosX < m_Width);
    HPRECONDITION(pi_StartPosY >= 0L && pi_StartPosY < m_Height);
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);

    return m_pEditor->GetRun(pi_StartPosX,
                             pi_StartPosY,
                             pi_PixelCount,
                             pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// SetRun
//-----------------------------------------------------------------------------
void HRAEditor::SetRun(HUINTX       pi_StartPosX,
                       HUINTX       pi_StartPosY,
                       size_t       pi_PixelCount,
                       const void*  pi_pRun,
                       void*        pi_pTransaction)
    {
    HPRECONDITION(pi_pRun != 0);
    HPRECONDITION(pi_StartPosX >= 0L && pi_StartPosX < m_Width);
    HPRECONDITION(pi_StartPosY >= 0L && pi_StartPosY < m_Height);
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);

    m_pEditor->SetRun(pi_StartPosX,
                      pi_StartPosY,
                      pi_PixelCount,
                      pi_pRun,
                      pi_pTransaction);
    }

//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
void HRAEditor::GetPixels(const HUINTX* pi_pPositionsX,
                          const HUINTX* pi_pPositionsY,
                          size_t        pi_PixelCount,
                          void*         po_pBuffer) const
    {
    HPRECONDITION(pi_pPositionsX != 0);
    HPRECONDITION(pi_pPositionsY != 0);
    HPRECONDITION(po_pBuffer != 0);

    m_pEditor->GetPixels(pi_pPositionsX,
                         pi_pPositionsY,
                         pi_PixelCount,
                         po_pBuffer);
    }


//-----------------------------------------------------------------------------
// public
// GetPixels
//-----------------------------------------------------------------------------
void HRAEditor::GetPixels(HUINTX    pi_PositionX,
                          HUINTX    pi_PositionY,
                          HSINTX    pi_DeltaX,
                          HSINTX    pi_DeltaY,
                          size_t    pi_PixelCount,
                          void*     po_pBuffer) const
    {
    HPRECONDITION(po_pBuffer != 0);

    m_pEditor->GetPixels(pi_PositionX,
                         pi_PositionY,
                         pi_DeltaX,
                         pi_DeltaY,
                         pi_PixelCount,
                         po_pBuffer);
    }


//-----------------------------------------------------------------------------
// public
// GetFirstPixel
//-----------------------------------------------------------------------------
void* HRAEditor::GetFirstPixel(HUINTX*  po_pPosX,
                               HUINTX*  po_pPosY) const
    {
    HPRECONDITION(po_pPosX != 0);
    HPRECONDITION(po_pPosY != 0);

    void* pRawData = 0;

    // test if there is a shape
    if (m_pScanLines != 0)
        {
        // goto the first run
        if (m_pScanLines->GotoFirstRun())
            {
#ifdef __HMR_DEBUG
            HSINTX PosX;
            HSINTX PosY;

            m_pScanLines->GetCurrentRun(&PosX,
                                        &PosY,
                                        &m_RunPixelCount);
            HPOSTCONDITION(PosX >= 0L && PosY >= 0L && (uint32_t)PosY < m_Height);
            HPOSTCONDITION(PosX + m_RunPixelCount <= m_Width);
#endif // __HMR_DEBUG

            // get the run
            m_pScanLines->GetCurrentRun((HSINTX*)po_pPosX,
                                        (HSINTX*)po_pPosY,
                                        &m_RunPixelCount);

            pRawData = m_pEditor->GetPixel(*po_pPosX,
                                           *po_pPosY);

            m_RunPixelCount--;
            }
        }
    else
        {
        // goto the first pixel
        m_PosX = 0L;
        m_PosY = 0L;
        *po_pPosX = 0L;
        *po_pPosY = 0L;

        // get the pixel
        pRawData = m_pEditor->GetPixel(*po_pPosX, *po_pPosY);
        }

    return pRawData;
    }


//-----------------------------------------------------------------------------
// public
// GetNextPixel
//-----------------------------------------------------------------------------
void* HRAEditor::GetNextPixel(HUINTX*   po_pPosX,
                              HUINTX*   po_pPosY) const
    {
    void* pRawData = 0;

    // test if there is a shape
    if (m_pScanLines != 0)
        {
        if (m_RunPixelCount != 0)
            {
            pRawData = m_pEditor->GetNextPixel();
            m_PosX++;
            m_RunPixelCount--;
            }
        else
            {
            // goto the next run
            if (m_pScanLines->GotoNextRun())
                {
#ifdef __HMR_DEBUG
                HSINTX PosX;
                HSINTX PosY;

                m_pScanLines->GetCurrentRun(&PosX,
                                            &PosY,
                                            &m_RunPixelCount);
                HPOSTCONDITION(PosX >= 0L && PosY >= 0L && (uint32_t)PosY < m_Height);
                HPOSTCONDITION(PosX + m_RunPixelCount <= m_Width);
#endif // __HMR_DEBUG

                m_pScanLines->GetCurrentRun((HSINTX*)po_pPosX,
                                            (HSINTX*)po_pPosY,
                                            &m_RunPixelCount);

                pRawData = m_pEditor->GetPixel(*po_pPosX, *po_pPosY);

                m_RunPixelCount--;
                }
            }
        }
    else
        {
        // goto the next pixel
        m_PosX++;

        // test if we are at the end of the line
        if (m_PosX == m_Width)
            {
            // if yes, go to next line
            m_PosX = 0;
            m_PosY++;

            // test if this is a valid line
            if (m_PosY != m_Height)
                pRawData = m_pEditor->GetPixel(0L, m_PosY);
            }
        else
            {
            pRawData = m_pEditor->GetNextPixel();
            }
        *po_pPosX = m_PosX;
        *po_pPosY = m_PosY;
        }

    return pRawData;
    }


//-----------------------------------------------------------------------------
// public
// GetPixel
// long version
//-----------------------------------------------------------------------------
void* HRAEditor::GetPixel(HUINTX    pi_PosX,
                          HUINTX    pi_PosY) const
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);

    return m_pEditor->GetPixel(pi_PosX, pi_PosY);
    }


//-----------------------------------------------------------------------------
// public
// SetPixel
// long version
//-----------------------------------------------------------------------------
void HRAEditor::SetPixel(HUINTX       pi_PosX,
                         HUINTX       pi_PosY,
                         const void*  pi_pValue)
    {
    HPRECONDITION(pi_PosX < m_Width);
    HPRECONDITION(pi_PosY < m_Height);
    HPRECONDITION(pi_pValue != 0);

    m_pEditor->SetPixel(pi_PosX, pi_PosY, pi_pValue);
    }


//-----------------------------------------------------------------------------
// public
// Clear
//-----------------------------------------------------------------------------
void HRAEditor::Clear(const void*  pi_pValue,
                      void*        pi_pTransaction)
    {
    if (m_pScanLines == 0 || !m_pEditor->HasData())
        {
        // unshaped case
        m_pEditor->Clear(pi_pValue, pi_pTransaction);
        }
    else if (m_pScanLines->GotoFirstRun())
        {
        do
            {
            size_t  PixelCount;
            HSINTX  StartPosX;
            HSINTX  StartPosY;

            m_pScanLines->GetCurrentRun(&StartPosX, &StartPosY, &PixelCount);
            HPOSTCONDITION(StartPosX >= 0L && StartPosY >= 0L && (uint32_t)StartPosY < m_Height);
            HPOSTCONDITION(StartPosX + PixelCount <= m_Width);

            m_pEditor->ClearRun(StartPosX, StartPosY, PixelCount, pi_pValue, pi_pTransaction);

            }
        while(m_pScanLines->GotoNextRun());
        }
    }

//-----------------------------------------------------------------------------
// public
// ClearRun
//-----------------------------------------------------------------------------
void HRAEditor::ClearRun(HUINTX       pi_StartPosX,
                         HUINTX       pi_StartPosY,
                         size_t       pi_PixelCount,
                         const void*  pi_pValue,
                         void*        pi_pTransaction)
    {
    HPRECONDITION(pi_StartPosX < m_Width);
    HPRECONDITION(pi_StartPosY < m_Height);
    HPRECONDITION(pi_StartPosX + pi_PixelCount <= m_Width);
    HPRECONDITION(pi_pValue != 0);

    m_pEditor->ClearRun(pi_StartPosX, pi_StartPosY, pi_PixelCount, pi_pValue, pi_pTransaction);
    }


//-----------------------------------------------------------------------------
// public
// GetScanlines. Retrieve the scanlines object used by the editor.
//
// Public because HRABlitter needs to access the scanlines.
// Should not be used otherwise.
//-----------------------------------------------------------------------------
const HGFScanLines* HRAEditor::GetScanlines() const
    {
    return m_pScanLines;
    }


//-----------------------------------------------------------------------------
// public
// MergeRuns
//-----------------------------------------------------------------------------
void HRAEditor::MergeRuns(HUINTX         pi_StartPosX,
                          HUINTX         pi_StartPosY,
                          size_t         pi_Width,
                          size_t         pi_Height,
                          const void*    pi_pRun,
                          void*          pi_pTransaction)
    {
    m_pEditor->MergeRuns(pi_StartPosX,
                         pi_StartPosY,
                         pi_Width,
                         pi_Height,
                         pi_pRun,
                         pi_pTransaction);
    }


