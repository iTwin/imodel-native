//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFMrSIDEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFMrSIDEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFMrSIDFile.h>
#include <ImagePP/all/h/HRFMrSIDEditor.h>

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
static const uint16_t m_LinePadBits = 32;

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFMrSIDEditor::HRFMrSIDEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                               uint32_t              pi_Page,
                               uint16_t       pi_Resolution,
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
                                  Byte*  po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);

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
        try
            {
            LT_STATUS sts = LT_STS_Uninit;

            int32_t BlockHeight = MIN(MAX((int32_t)pMrSIDFile->m_pStdViewHeight[m_ResNb] - (int32_t)pi_PosBlockY, 0), BLOCK_HEIGHT);
            int32_t BlockWidth  = MIN(MAX((int32_t)pMrSIDFile->m_pStdViewWidth[m_ResNb]  - (int32_t)pi_PosBlockX, 0), BLOCK_WIDTH);

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
                                   const Byte*  pi_pData)
    {
    HASSERT(0); // not supported

    return H_ERROR;
    }
#endif // IPP_HAVE_MRSID_SUPPORT
