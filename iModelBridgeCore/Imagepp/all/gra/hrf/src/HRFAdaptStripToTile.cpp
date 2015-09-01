//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptStripToTile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptStripToTile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCMemcpy.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptStripToTile.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToTileCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptStripToTileCapabilities::HRFAdaptStripToTileCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_STRIP);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_TILE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToTileCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptStripToTileCreator::GetCapabilities() const
    {
    return HRFAdaptStripToTileCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptStripToTileCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    unsigned short       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptStripToTile(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptStripToTile::HRFAdaptStripToTile(
    HRFBlockAdapterCapabilities* pi_pCapabilities,
    HFCPtr<HRFRasterFile>        pi_rpRasterFile,
    uint32_t                     pi_Page,
    unsigned short              pi_Resolution,
    HFCAccessMode                pi_AccessMode)
    : HRFBlockAdapter(pi_pCapabilities,
                      pi_rpRasterFile,
                      pi_Page,
                      pi_Resolution,
                      pi_AccessMode)
    {
    // Resolution dimension

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);
    HASSERT(m_pResolutionDescriptor->GetWidth() <= ULONG_MAX);
    HASSERT(m_pResolutionDescriptor->GetBytesPerWidth() <= ULONG_MAX);

    m_Height             = (uint32_t)m_pResolutionDescriptor->GetHeight();
    m_Width              = (uint32_t)m_pResolutionDescriptor->GetWidth();

    // Block dimension
    m_StripHeight        = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();
    m_BlockHeight         = m_pResolutionDescriptor->GetBlockHeight();
    m_BlockWidth          = m_pResolutionDescriptor->GetBlockWidth();

    // Calc the number of bytes per Image Width
    m_DBytesByPixel         = m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() / 8.0;
    m_ExactBytesPerWidth = (uint32_t)m_pResolutionDescriptor->GetBytesPerWidth();

    // Calc the number of blocks per width
    m_BlocksPerWidth = (uint32_t)m_pResolutionDescriptor->GetBlocksPerWidth();

    // Calc the number of bytes per Block Width
    m_ExactBytesPerBlockWidth =  (uint32_t)ceil(m_BlockWidth * m_DBytesByPixel);

    m_PosTileY = 0;
    m_IsBlocksEmpty = true;
    m_IsBlocksOverwritten = false;

    // Allocate a list of pointers on blocks
    m_ppBlocks = new HArrayAutoPtr<Byte>[m_BlocksPerWidth];
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptStripToTile::~HRFAdaptStripToTile()
    {
    m_pStripBuffer = 0;

    // Check to save the intern blocks
    if ( ( m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess ) && m_IsBlocksOverwritten )
        SaveBlocks(0);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToTile::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte* po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Check if the current location of intern blocks change
    if (pi_PosBlockY != m_PosTileY)
        {
        // if the intern blocks are overwritten we save these blocks
        if (m_IsBlocksOverwritten)
            Status = SaveBlocks(pi_pSisterFileLock);

        // Load the client Block and beside blocks into the intern tile cache
        if (Status == H_SUCCESS)
            Status = LoadBlocks((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, pi_pSisterFileLock);
        }
    else
        {
        // if the client Block is not loaded in the intern blocks we load the block into the intern cache
        if (m_IsBlocksEmpty)
            Status = LoadBlocks((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, pi_pSisterFileLock);
        }

    // Alloc memory if not already done
    Alloc_m_ppBlocks();

    // Copy the data to the client buffer
    HFCMemcpy(po_pData, m_ppBlocks[pi_PosBlockX/m_BlockWidth], m_ExactBytesPerBlockWidth*m_BlockHeight);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToTile::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte* pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Check to save the intern blocks
    if ((pi_PosBlockY != m_PosTileY) && m_IsBlocksOverwritten)
        Status = SaveBlocks(pi_pSisterFileLock);

    if ((Status == H_SUCCESS) && (m_AccessMode.m_HasReadAccess))
        {
        // Check if the client Block is loaded in the intern blocks
        if ((pi_PosBlockY != m_PosTileY) || m_IsBlocksEmpty)
            // Load the client Block and beside blocks into the intern tile cache
            Status = LoadBlocks((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, pi_pSisterFileLock);
        }

    // Alloc memory if not already done
    Alloc_m_ppBlocks();

    // Copy the client buffer to the intern blocks
    HFCMemcpy(m_ppBlocks[pi_PosBlockX/m_BlockWidth], pi_pData, m_ExactBytesPerBlockWidth*m_BlockHeight);

    m_IsBlocksOverwritten = true;
    m_IsBlocksEmpty = false;
    m_PosTileY = (uint32_t)pi_PosBlockY;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// LoadBlocks
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToTile::LoadBlocks(uint32_t pi_PosBlockX,
                                        uint32_t pi_PosBlockY,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status = H_SUCCESS;

    if (m_pStripBuffer == 0)
        {
        // Alloc working buffer if not already allocated.
        m_pStripBuffer = new Byte[m_ExactBytesPerWidth * m_StripHeight];
        }

    // Alloc memory if not already done
    Alloc_m_ppBlocks();

    // Calc the first and last strip to read
    uint32_t FirstStrip = pi_PosBlockY / m_StripHeight;
    uint32_t LastStrip  = (pi_PosBlockY + m_BlockHeight + m_StripHeight - 1) / m_StripHeight;

    // Adjust if necessary the last strip to the resolution height
    if (((LastStrip * m_StripHeight) - m_StripHeight) >= m_Height)
        LastStrip  = (m_Height + m_StripHeight - 1) / m_StripHeight;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Load the client Block and beside blocks into the intern blocks
    for (uint32_t NoStrip=FirstStrip; (NoStrip < LastStrip) && (Status == H_SUCCESS); NoStrip++)
        {
        Status = m_pAdaptedResolutionEditor->ReadBlock(0, NoStrip*m_StripHeight, m_pStripBuffer, pi_pSisterFileLock);

        if (Status == H_SUCCESS)
            {
            uint32_t StartPositionInStrip;
            uint32_t EndPositionInStrip;
            uint32_t StartPositionInTile;
            uint32_t StripPositionY = NoStrip*m_StripHeight;

            // Calc the StartPositionInStrip
            if (StripPositionY < pi_PosBlockY)
                {
                StartPositionInStrip = pi_PosBlockY - StripPositionY;
                StartPositionInTile = 0;
                }
            else
                {
                StartPositionInStrip = 0;
                StartPositionInTile = StripPositionY - pi_PosBlockY;
                }

            // Calc the EndPositionInStrip
            if ((StripPositionY+m_StripHeight) > (pi_PosBlockY+m_BlockHeight))
                EndPositionInStrip = m_StripHeight - ((StripPositionY+m_StripHeight) - (pi_PosBlockY+m_BlockHeight));
            else
                EndPositionInStrip = m_StripHeight;

            // Adapt the strip to the blocks
            for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
                {
                // adapt the line to the Block
                Byte* pInLineBuffer = m_pStripBuffer + (m_ExactBytesPerWidth * StartPositionInStrip) // Skip some lines
                                       + (m_ExactBytesPerBlockWidth*NoTile);

                // Adjust if necessary the number of byte to copy
                uint32_t ExactBytesToCopy;
                if (NoTile == m_BlocksPerWidth-1)
                    ExactBytesToCopy = m_ExactBytesPerWidth - (m_ExactBytesPerBlockWidth * NoTile);
                else
                    ExactBytesToCopy = m_ExactBytesPerBlockWidth;

                for (uint32_t NoLine =0; NoLine < (EndPositionInStrip - StartPositionInStrip); NoLine++)
                    {
                    uint32_t LinePosInTile = (StartPositionInTile+NoLine)*m_ExactBytesPerBlockWidth;

                    HFCMemcpy(m_ppBlocks[NoTile]+LinePosInTile,
                              pInLineBuffer,
                              ExactBytesToCopy);
                    pInLineBuffer += m_ExactBytesPerWidth;
                    }
                }
            }
        }
    SisterFileLock.ReleaseKey();

    m_PosTileY = pi_PosBlockY;
    m_IsBlocksEmpty = false;
    m_IsBlocksOverwritten = false;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// SaveBlocks
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToTile::SaveBlocks(HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS Status = H_SUCCESS;

    if (m_pStripBuffer == 0)
        {
        // Alloc working buffer if not already allocated.
        m_pStripBuffer = new Byte[m_ExactBytesPerWidth * m_StripHeight];
        }

    // Alloc memory if not already done
    Alloc_m_ppBlocks();

    // Calc the first and last strip to read
    uint32_t FirstStrip = m_PosTileY / m_StripHeight;
    uint32_t LastStrip  = (m_PosTileY + m_BlockHeight + m_StripHeight - 1) / m_StripHeight;

    // Adjust if necessary the last strip to the resolution height
    if (((LastStrip * m_StripHeight) - m_StripHeight) >= m_Height)
        LastStrip  = (m_Height + m_StripHeight - 1) / m_StripHeight;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Load the client Block and beside blocks into the intern blocks
    for (uint32_t NoStrip=FirstStrip; (NoStrip < LastStrip) && (Status == H_SUCCESS); NoStrip++)
        {
        uint32_t StartPositionInStrip;
        uint32_t EndPositionInStrip;
        uint32_t StartPositionInTile;
        uint32_t StripPositionY = NoStrip*m_StripHeight;

        // Calc the StartPositionInStrip
        if (StripPositionY < m_PosTileY)
            {
            StartPositionInStrip = m_PosTileY - StripPositionY;
            StartPositionInTile = 0;
            }
        else
            {
            StartPositionInStrip = 0;
            StartPositionInTile = StripPositionY - m_PosTileY;
            }

        // Calc the EndPositionInStrip
        if ((StripPositionY+m_StripHeight) > (m_PosTileY+m_BlockHeight))
            EndPositionInStrip = m_StripHeight - ((StripPositionY+m_StripHeight) - (m_PosTileY+m_BlockHeight));
        else
            EndPositionInStrip = m_StripHeight;

        // Adapt the strip to the blocks
        for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
            {
            // Be sure to remain inbound.
            HASSERT( m_ExactBytesPerWidth* m_StripHeight >= ((m_ExactBytesPerWidth * (StartPositionInStrip + 1)) + (m_ExactBytesPerBlockWidth*NoTile)));

            // adapt the line to the Block
            Byte* pInLineBuffer = m_pStripBuffer + (m_ExactBytesPerWidth * StartPositionInStrip) // Skip some lines
                                   + (m_ExactBytesPerBlockWidth*NoTile);

            // Adjust if necessary the number of byte to copy
            uint32_t ExactBytesToCopy;
            if (NoTile == m_BlocksPerWidth-1)
                ExactBytesToCopy = m_ExactBytesPerWidth - (m_ExactBytesPerBlockWidth * NoTile);
            else
                ExactBytesToCopy = m_ExactBytesPerBlockWidth;

            for (uint32_t NoLine =0; NoLine < (EndPositionInStrip - StartPositionInStrip); NoLine++)
                {
                uint32_t LinePosInTile = (StartPositionInTile+NoLine)*m_ExactBytesPerBlockWidth;

                HFCMemcpy(pInLineBuffer,
                          m_ppBlocks[NoTile]+LinePosInTile,
                          ExactBytesToCopy);
                pInLineBuffer += m_ExactBytesPerWidth;
                }
            }
        Status = m_pAdaptedResolutionEditor->WriteBlock(0, NoStrip*m_StripHeight, m_pStripBuffer, pi_pSisterFileLock);
        }

    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();
    m_IsBlocksOverwritten = false;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// NoMoreRead
//
// This method is called by the cache. We deallocate all working buffer.
// If a ReadBlock() is called after, working buffer will be re-allocated
// and kept until a NoMoreRead was called.
//-----------------------------------------------------------------------------
void HRFAdaptStripToTile::NoMoreRead()
    {
    HSTATUS Status = H_SUCCESS;

    if (m_IsBlocksOverwritten)
        Status = SaveBlocks(0);

    if (Status == H_SUCCESS)
        for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
            m_ppBlocks[NoTile] = 0;
    }
//------------------------------------------------------------------------- Private

//-----------------------------------------------------------------------------
// public
// Alloc_m_ppBlocks : Allocate memory if not already done
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptStripToTile::Alloc_m_ppBlocks()
    {
    if (m_ppBlocks[0] == 0)
        {
        // Allocate blocks
        for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
            m_ppBlocks[NoTile] = new Byte[m_BlockHeight * m_ExactBytesPerBlockWidth];
        }
    }