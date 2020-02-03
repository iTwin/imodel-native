//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFHMRTileEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFPWRasterFile.h>
#include <ImagePP/all/h/HRFPWEditor.h>

#if defined(IPP_HAVE_PROJECTWISE_SUPPORT) 

#include <ImagePP/all/h/interface/IHRFPWFileHandler.h>


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFPWEditor::HRFPWEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                         uint32_t              pi_Page,
                         uint16_t       pi_Resolution,
                         HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    m_DocumentID = ((HFCPtr<HRFPWRasterFile>&)m_pRasterFile)->m_OriginalFileInfo.DocumentID;
    m_DocumentTimestamp = ((HFCPtr<HRFPWRasterFile>&)m_pRasterFile)->m_OriginalFileInfo.Timestamp;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFPWEditor::~HRFPWEditor()
    {
    }



//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPWEditor::ReadBlock(uint64_t pi_PosBlockX,
                               uint64_t pi_PosBlockY,
                               Byte* po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);

    HSTATUS RetValue = H_SUCCESS;
    // must call IHRFPWFileHandler interface
    HASSERT_X64(m_DocumentTimestamp < UINT32_MAX);  // see the cast below
    if (((HFCPtr<HRFPWRasterFile>&)m_pRasterFile)->m_pPWHandler->GetTile(m_DocumentID,
                                                                         (__time32_t)m_DocumentTimestamp,
                                                                         m_Page,
                                                                         m_Resolution,
                                                                         (uint32_t)pi_PosBlockX,
                                                                         (uint32_t)pi_PosBlockY,
                                                                         po_pData) != IHRFPWFileHandler::GETTILE_Success)
        RetValue = H_ERROR;

    return RetValue;
    }



//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPWEditor::WriteBlock(uint64_t    pi_PosBlockX,
                                uint64_t    pi_PosBlockY,
                                const Byte* pi_pData)
    {
    HASSERT(0);
    return H_NOT_SUPPORTED;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPWEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                uint64_t                 pi_PosBlockY,
                                const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HASSERT(0);
    return H_NOT_SUPPORTED;
    }


//-----------------------------------------------------------------------------
// protected section
//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
// for test
//-----------------------------------------------------------------------------

#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFRasterFileCache.h>
#include <ImagePP/all/h/HRFiTiffCacheFileCreator.h>

#if 0
HRFPWHandler::HRFPWHandler(const HFCURL& pi_rPWFile)
    {
    Utf8String URL = pi_rPWFile.GetURL();
    URL.erase(URL.length() - 3);

    HFCPtr<HRFRasterFile> pTrueRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(HFCURL::Instanciate(URL));
    m_pRasterFile = new HRFRasterFileCache(pTrueRasterFile, HRFiTiffCacheFileCreator::GetInstance());

    }

HRFPWHandler::~HRFPWHandler()
    {
    }

IHRFPWFileHandler::GETTILE_STATUS HRFPWHandler::GetTile(uint32_t pi_Page,
                                                        uint16_t pi_Res,
                                                        uint32_t pi_PosX,
                                                        uint32_t pi_PosY,
                                                        Byte*  po_pData)
    {
    HAutoPtr<HRFResolutionEditor> pEditor(m_pRasterFile->CreateResolutionEditor(pi_Page, pi_Res, HFC_READ_ONLY));

    if (pEditor->ReadBlock(pi_PosX, pi_PosY, po_pData) == H_SUCCESS)
        return GETTILE_Success;
    else
        return GETTILE_Error;
    }

#endif

#endif  // IPP_HAVE_PROJECTWISE_SUPPORT
