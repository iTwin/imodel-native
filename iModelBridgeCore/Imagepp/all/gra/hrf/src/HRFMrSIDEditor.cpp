//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMrSIDEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFMrSIDEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFMrSIDFile.h>
#include <Imagepp/all/h/HRFMrSIDEditor.h>

#if defined(IPP_HAVE_MRSID_SUPPORT) 

#include <MrSid/lt_base.h>
#include <MrSid/lt_fileSpec.h>
#include <MrSid/lt_types.h>

#include <MrSid/lti_imageReader.h>
#include <MrSid/lti_metadataUtils.h>
#include <MrSid/lti_Navigator.h>
#include <MrSid/lti_metadataReader.h>
#include <MrSid/lti_metadataDatabase.h>
#include <MrSid/lti_SceneBuffer.h>
#include <MrSid/lti_Pixel.h>
#include <MrSid/lti_metadataRecord.h>
#include <MrSid/lti_types.h>
#include <MrSid/lti_viewerImageFilter.h>

using namespace LizardTech;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const unsigned short m_LinePadBits = 32;

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFMrSIDEditor::HRFMrSIDEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                               uint32_t              pi_Page,
                               unsigned short       pi_Resolution,
                               HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile = pi_rpRasterFile;

    HASSERT(m_pRasterFile != 0);
    HASSERT(pi_Resolution < ((HRFMrSIDFile*)m_pRasterFile.GetPtr())->m_ResCount);

    m_ResNb = pi_Resolution;

    m_pTileIDDesc = new HGFTileIDDescriptor(m_pResolutionDescriptor->GetWidth(),
                                            m_pResolutionDescriptor->GetHeight(),
                                            m_pResolutionDescriptor->GetBlockWidth(),
                                            m_pResolutionDescriptor->GetBlockHeight());
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFMrSIDEditor::~HRFMrSIDEditor()
    {
    }



//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFMrSIDEditor::ReadBlock(uint64_t pi_PosBlockX,
                                  uint64_t pi_PosBlockY,
                                  Byte*  po_pData,
                                  HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_ERROR;
    HRFMrSIDFile* pMrSIDFile = (HRFMrSIDFile*)m_pRasterFile.GetPtr();

    HRFMrSIDFile::TilePool::iterator Itr(pMrSIDFile->m_TilePool.find(m_pTileIDDesc->ComputeID(pi_PosBlockX, pi_PosBlockY, m_Resolution)));
    if (Itr != pMrSIDFile->m_TilePool.end())
        {
        memcpy(po_pData, Itr->second, m_pResolutionDescriptor->GetBlockSizeInBytes());
        delete[] Itr->second;
        pMrSIDFile->m_TilePool.erase(Itr);
        Status = H_SUCCESS;
        }
    else
        {
        HFCLockMonitor SisterFileLock;
        try
            {
            LT_STATUS sts = LT_STS_Uninit;

            int BlockHeight = MIN(MAX((int)pMrSIDFile->m_pStdViewHeight[m_ResNb] - (int)pi_PosBlockY, 0), BLOCK_HEIGHT);
            int BlockWidth  = MIN(MAX((int)pMrSIDFile->m_pStdViewWidth[m_ResNb]  - (int)pi_PosBlockX, 0), BLOCK_WIDTH);

            // Lock the sister file
            if(pi_pSisterFileLock == 0)
                {
                // Lock the file.
                AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
                pi_pSisterFileLock = &SisterFileLock;
                }

            delete pMrSIDFile->m_pSceneBuffer;
            pMrSIDFile->m_pSceneBuffer = new LTISceneBuffer(pMrSIDFile->m_pImageReader->getPixelProps(), BlockWidth, BlockHeight, 0);

            const LTIScene scene((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, BlockWidth, BlockHeight,  pMrSIDFile->m_pRatio[m_ResNb]);
            sts = pMrSIDFile->m_pImageReader->read(scene, *pMrSIDFile->m_pSceneBuffer);

            if (LT_SUCCESS(sts))
                {
                sts = pMrSIDFile->m_pSceneBuffer->exportData(po_pData,
                                                             pMrSIDFile->m_pImageReader->getNumBands(),
                                                             BLOCK_WIDTH * pMrSIDFile->m_pImageReader->getNumBands(),
                                                             1);
                if (LT_SUCCESS(sts))
                    Status = H_SUCCESS;
                }
            }
        catch (...)
            {
            _ASSERT(false);
            }
        SisterFileLock.ReleaseKey();
        }


    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFMrSIDEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                   uint64_t     pi_PosBlockY,
                                   const Byte*  pi_pData,
                                   HFCLockMonitor const* pi_pSisterFileLock)
    {
    HASSERT(0); // not supported

    return H_ERROR;
    }
#endif // IPP_HAVE_MRSID_SUPPORT
