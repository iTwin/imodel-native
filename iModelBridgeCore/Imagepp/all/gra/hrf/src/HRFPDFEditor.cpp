//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPDFEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPDFEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFPDFFile.h>

#if defined(IPP_HAVE_PDF_SUPPORT) 

#include <Imagepp/all/h/HRFPDFEditor.h>

#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRFRawFile.h>
#include <Imagepp/all/h/HMDContext.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HMDVolatileLayers.h>

#include "HRFPDFLibInterface.h"

class UStationPDFEditorWrapper : public PDFEditorWrapper
    {
public:
    UStationPDFEditorWrapper(void*      pi_Document,
                             uint32_t   pi_Page,
                             double    pi_ScaleFactor)
        {
        m_pPDFEditor = HRFPDFLibInterface::CreateEditor((PDDoc)pi_Document,
                                                        pi_Page,
                                                        pi_ScaleFactor);

        if (PDDocPermRequest((PDDoc)pi_Document, PDPermReqObjDoc,
                             PDPermReqOprCopy, NULL) == PDPermReqGranted)
            {
            m_UseDrawContentToMem = true;
            }
        else
            {
            m_UseDrawContentToMem = false;

            PDPrefSetAntialiasLevel(kPDPageDrawSmoothText |
                                    kPDPageDrawSmoothLineArt |
                                    kPDPageDrawSmoothImage);
            }

        PDPrefSetEnableThinLineHeuristics(true);
        };

    virtual ~UStationPDFEditorWrapper()
        {
        HRFPDFLibInterface::ReleaseEditor(&m_pPDFEditor);
        };

    void  GetResolutionSize(uint32_t* po_pResWidth,
                            uint32_t* po_pResHeight) const override
    {
        HPRECONDITION(po_pResWidth != 0);
        HPRECONDITION(po_pResHeight != 0);

        *po_pResWidth = m_pPDFEditor->m_ResWidth;
        *po_pResHeight = m_pPDFEditor->m_ResHeight;
    };

    bool ReadBlock(uint64_t            pi_PosX,
                   uint64_t            pi_PosY,
                   const HFCPtr<HMDContext>& pi_rpContext,
                   Byte*               po_pData)
    {
        HPRECONDITION(m_pPDFEditor->m_pPDDoc != 0);
        HPRECONDITION(po_pData != 0);
        HPRECONDITION(pi_PosX <= ULONG_MAX && pi_PosY <= ULONG_MAX);

        return HRFPDFLibInterface::ReadBlock(m_pPDFEditor,
                                             m_UseDrawContentToMem,
                                             (uint32_t)pi_PosX,
                                             (uint32_t)pi_PosY,
                                             1024,
                                             1024,
                                             pi_rpContext,
                                             po_pData);
    };

    bool ReadBlock(uint64_t            pi_MinX,
                   uint64_t            pi_MinY,
                   uint64_t            pi_MaxX,
                   uint64_t            pi_MaxY,
                   const HFCPtr<HMDContext>& pi_rpContext,
                   Byte*               po_pData)
    {
        HPRECONDITION(m_pPDFEditor->m_pPDDoc != 0);
        HPRECONDITION(po_pData != 0);
        HPRECONDITION(pi_MinX <= ULONG_MAX && pi_MinY <= ULONG_MAX);

        ASInt32 Width = (uint32_t)pi_MaxX - (uint32_t)pi_MinX;
        ASInt32 Height = (uint32_t)pi_MaxY - (uint32_t)pi_MinY;

        return HRFPDFLibInterface::ReadBlockByExtent(m_pPDFEditor,
                                                     m_UseDrawContentToMem,
                                                     (uint32_t)pi_MinX,
                                                     (uint32_t)pi_MinY,
                                                     Width,
                                                     Height,
                                                     pi_rpContext,
                                                     po_pData);
    };

    HRFPDFLibInterface::Editor* GetPDFEditor()
        {
        return m_pPDFEditor;
        }

private:

    HRFPDFLibInterface::Editor* m_pPDFEditor;
    bool                       m_UseDrawContentToMem;
    }; // class UStationPDFEditorWrapper



#define PDF_RASTERFILE      static_cast<HRFPDFFile*>(GetRasterFile().GetPtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFPDFEditor::HRFPDFEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                           uint32_t              pi_Page,
                           double               pi_Resolution,
                           HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    if (m_Resolution > 255)
        throw HRFBadSubImageException(pi_rpRasterFile->GetURL()->GetURL());

    CreateEditorWrapper();

    uint32_t ResWidth;
    uint32_t ResHeight;
    m_pPDFEditorWrapper->GetResolutionSize(&ResWidth, &ResHeight);

    HFCPtr<HRFResolutionDescriptor> pMainResDesc(pi_rpRasterFile->GetPageDescriptor(pi_Page)->GetResolutionDescriptor(0));
    m_pResolutionDescriptor = new HRFResolutionDescriptor(GetAccessMode(),
                                                          pi_rpRasterFile->GetCapabilities(),
                                                          pi_Resolution,
                                                          pi_Resolution,
                                                          pMainResDesc->GetPixelType(),
                                                          pMainResDesc->GetCodec(),
                                                          pMainResDesc->GetReaderBlockAccess(),
                                                          pMainResDesc->GetWriterBlockAccess(),
                                                          pMainResDesc->GetScanlineOrientation(),
                                                          pMainResDesc->GetInterleaveType(),
                                                          pMainResDesc->IsInterlace(),
                                                          ResWidth,
                                                          ResHeight,
                                                          pMainResDesc->GetBlockWidth(),
                                                          pMainResDesc->GetBlockHeight());


    m_BlockWidth = m_pResolutionDescriptor->GetBlockWidth();
    m_BlockHeight = m_pResolutionDescriptor->GetBlockHeight();
    m_BytesPerBlockWidth = m_pResolutionDescriptor->GetBytesPerBlockWidth();
    m_BlockSizeInBytes = m_pResolutionDescriptor->GetBlockSizeInBytes();

    m_pTileIDDescriptor = new HGFTileIDDescriptor(ResWidth,
                                                  ResHeight,
                                                  m_BlockWidth,
                                                  m_BlockHeight);

    if (PDF_RASTERFILE->GetMainThreadId() != m_WrapperCreatorThreadId)
        {
        PDDocClose(((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->GetPDFEditor()->m_pPDDoc);
        ((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->GetPDFEditor()->m_pPDDoc = 0;
        }
    }

//-----------------------------------------------------------------------------
// private
// Destruction
//-----------------------------------------------------------------------------
void HRFPDFEditor::CreateEditorWrapper()
    {
    m_pPDFEditorWrapper = new UStationPDFEditorWrapper(PDF_RASTERFILE->GetDocument(m_Page),
                                                       m_Page,
                                                       m_ResolutionFactor);

    m_WrapperCreatorThreadId = GetCurrentThreadId();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFPDFEditor::~HRFPDFEditor()
    {
    PDF_RASTERFILE->RemoveLookAhead(m_Page, m_Resolution);
    }


HSTATUS HRFPDFEditor::ReadBlockPDF(uint32_t    pi_MinX,
                                   uint32_t    pi_MinY,
                                   uint32_t    pi_MaxX,
                                   uint32_t    pi_MaxY,
                                   Byte*        po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(pi_MaxX > pi_MinX);
    HPRECONDITION(pi_MaxY > pi_MinY);
    HPRECONDITION((pi_MinX % m_pResolutionDescriptor->GetBlockWidth()) == 0);
    HPRECONDITION((pi_MinY % m_pResolutionDescriptor->GetBlockHeight()) == 0);
    HPRECONDITION((pi_MaxX % m_pResolutionDescriptor->GetBlockWidth()) == 0);
    HPRECONDITION((pi_MaxY % m_pResolutionDescriptor->GetBlockHeight()) == 0);

    HSTATUS RetStatus = H_SUCCESS;

    if (m_WrapperCreatorThreadId != GetCurrentThreadId())
        {
        CreateEditorWrapper();
        }

    if (((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->GetPDFEditor()->m_pPDDoc == 0)
        {
        ((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->GetPDFEditor()->m_pPDDoc = (PDDoc)PDF_RASTERFILE->GetDocument(m_Page);
        }


    if (m_pPDFEditorWrapper->ReadBlock(pi_MinX,
                                         pi_MinY,
                                         pi_MaxX,
                                         pi_MaxY,
                                         PDF_RASTERFILE->GetContext(GetPage()),
                                         po_pData) == false)
    {
        RetStatus = H_ERROR;
        }

    if (PDF_RASTERFILE->GetMainThreadId() != GetCurrentThreadId())
        {
        PDDocClose(((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->
                   GetPDFEditor()->m_pPDDoc);
        ((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->
        GetPDFEditor()->m_pPDDoc = 0;
        }

    return RetStatus;
    }


//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPDFEditor::ReadBlock(uint64_t pi_PosBlockX,
                                  uint64_t pi_PosBlockY,
                                  Byte*  po_pData,
                                  HFCLockMonitor const* pi_pSisterFileLock)
    {
    //Something with more bits than UInt64 should be used on architecture with pointer greater than 64 bits.
    HPRECONDITION(sizeof(PDF_RASTERFILE->GetContext(m_Page).GetPtr()) <= 8);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);

    HSTATUS Status = H_NOT_FOUND;

    HFCMonitor Monitor(PDF_RASTERFILE->m_TilePoolKey);

    HRFPDFFile::ContextPageTilePool::iterator ContextTilePool(PDF_RASTERFILE->m_ContextPageTilePool.find((uint64_t)PDF_RASTERFILE->GetContext(m_Page).GetPtr()));

    if (ContextTilePool != PDF_RASTERFILE->m_ContextPageTilePool.end())
        {
        HRFPDFFile::PageTilePool::iterator TilePool(ContextTilePool->second.find(m_Page));
        if (TilePool != ContextTilePool->second.end())
            {
            HRFPDFFile::TilePool::iterator Tile(TilePool->second.find(m_pTileIDDescriptor->ComputeID(pi_PosBlockX, pi_PosBlockY, m_Resolution)));
            if (Tile != TilePool->second.end())
                {
                memcpy(po_pData, Tile->second, m_pResolutionDescriptor->GetBlockSizeInBytes());
                Status = H_SUCCESS;

                // Normally, we don't request this tile anymore.
                delete[] Tile->second;
                TilePool->second.erase(Tile);
                }
            }
        }

    if (Status == H_NOT_FOUND)
        {
        if (m_WrapperCreatorThreadId != GetCurrentThreadId())
            {
            CreateEditorWrapper();
            }

        if (((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->
            GetPDFEditor()->m_pPDDoc == 0)
            {
            ((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->
            GetPDFEditor()->m_pPDDoc = (PDDoc)PDF_RASTERFILE->GetDocument(m_Page);

            }
        if (!m_pPDFEditorWrapper->ReadBlock(pi_PosBlockX,
                                            pi_PosBlockY,
                                            PDF_RASTERFILE->GetContext(GetPage()),
                                            po_pData))
            {
            Status = H_ERROR;
            }
        else
            {
            Status = H_SUCCESS;
            }

        if (PDF_RASTERFILE->GetMainThreadId() != GetCurrentThreadId())
            {
            PDDocClose(((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->GetPDFEditor()->m_pPDDoc);
            ((UStationPDFEditorWrapper*)m_pPDFEditorWrapper.get())->GetPDFEditor()->m_pPDDoc = 0;
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPDFEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                   uint64_t     pi_PosBlockY,
                                   const Byte*  pi_pData,
                                   HFCLockMonitor const* pi_pSisterFileLock)
    {
    HASSERT(0); // not supported

    return H_ERROR;
    }

#endif