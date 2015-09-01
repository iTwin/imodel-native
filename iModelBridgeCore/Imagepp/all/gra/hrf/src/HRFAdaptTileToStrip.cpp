//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptTileToStrip.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptTileToStrip
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptTileToStrip.h>
#include <Imagepp/all/h/HCDCodecImage.h>
#include <Imagepp/all/h/HCDPacket.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptTileToStripCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptTileToStripCapabilities::HRFAdaptTileToStripCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_TILE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_STRIP);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptTileToStripCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptTileToStripCreator::GetCapabilities() const
    {
    return HRFAdaptTileToStripCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptTileToStripCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    unsigned short       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptTileToStrip(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptTileToStrip::HRFAdaptTileToStrip(
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
    // Calc the number of bytes per Image Width
    double DBytesByPixel= m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() / 8.0;
    m_ExactBytesPerWidth = (uint32_t)ceil((double)m_pResolutionDescriptor->GetWidth() * DBytesByPixel);

    m_TileWidth          = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockWidth();
    m_TileHeight         = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();

    HASSERT(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerWidth() <= ULONG_MAX);

    m_TilePerWidth       = (uint32_t)m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerWidth();
    m_TileSize           = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes();
    m_TilePerBlock       = m_pResolutionDescriptor->GetBlockHeight() / m_TileHeight;

    // We want to be sure that nobody try to create a tile to strip adapter that adapt
    // to a strip higther than the tile. HRFBlockAdapterFactory::CanAdapt() function must
    // be used to valide if we can or can't used adapters.
    // Ex: We can adapt from tile of 256 x 256 to strip of 1024 x XXXX, but we
    //     can't adapt from tile of 256 x 256 to strip of 128 x XXXX.
    HASSERT(m_TilePerBlock >= 1);

    // Calc the number of bytes per Tile Width
    m_ExactBytesPerTileWidth =  (uint32_t)ceil(m_TileWidth * DBytesByPixel);
    m_ExactBytesToCopyForTheLastRightTile = (uint32_t)ceil((m_pResolutionDescriptor->GetWidth() - ((m_TilePerWidth-1) * m_TileWidth)) * DBytesByPixel);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptTileToStrip::~HRFAdaptTileToStrip()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToStrip::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status = H_SUCCESS;
    Byte* pPosInBlock = po_pData;

    // Alloc working buffer, if not already done
    if (m_pTile == 0)
        m_pTile = new Byte[m_TileSize];

    uint32_t CurrentTilePerBlock = m_TilePerBlock;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    if (pi_PosBlockY+(m_TilePerBlock*m_TileHeight) > m_pResolutionDescriptor->GetHeight())
        CurrentTilePerBlock = ((uint32_t)m_pResolutionDescriptor->GetHeight() - (uint32_t)pi_PosBlockY + m_TileHeight - 1) / m_TileHeight;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Load the block from file to the client data out
    for (uint32_t NoCol=0; (NoCol<m_TilePerWidth) && (Status == H_SUCCESS); NoCol++)
        {
        // Move to the Col at the first line
        pPosInBlock = po_pData + (m_ExactBytesPerTileWidth * NoCol);

        uint32_t ExactBytesToCopy;
        if (NoCol == m_TilePerWidth - 1)
            ExactBytesToCopy = m_ExactBytesToCopyForTheLastRightTile;
        else
            ExactBytesToCopy = m_ExactBytesPerTileWidth;

        for (uint32_t NoRow=0; (NoRow< CurrentTilePerBlock) && (Status == H_SUCCESS); NoRow++)
            {
            // Check if the block exist in the file, before read it
            uint32_t YPos = (uint32_t)pi_PosBlockY+(NoRow*m_TileHeight);
            if (YPos <= m_pResolutionDescriptor->GetHeight())
                Status = m_pAdaptedResolutionEditor->ReadBlock(NoCol*m_TileWidth, YPos, m_pTile, pi_pSisterFileLock);
            else
                NoRow = m_TilePerBlock;        // exit

            if (Status == H_SUCCESS)
                {
                Byte* pPosInTile = m_pTile;

                // Convert the tile data to the strip output buffer
                for (uint32_t NoLine=0; NoLine<m_TileHeight; NoLine++)
                    {
                    memcpy(pPosInBlock, pPosInTile, ExactBytesToCopy);
                    // Move to the next line
                    pPosInTile += m_ExactBytesPerTileWidth;
                    pPosInBlock += m_ExactBytesPerWidth;
                    }
                }
            }
        }
    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToStrip::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte*  pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS         Status = H_SUCCESS;
    const Byte*    pPosInBlock = pi_pData;
    uint32_t        CurrentTilePerBlock = m_TilePerBlock;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    uint32_t        ResolutionHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();
    bool           LastLineOfTile = false;

    // Alloc working buffer, if not already done
    if (m_pTile == 0)
        m_pTile = new Byte[m_TileSize];

    if (pi_PosBlockY+(m_TilePerBlock*m_TileHeight) > ResolutionHeight)
        {
        CurrentTilePerBlock = (ResolutionHeight - (uint32_t)pi_PosBlockY + m_TileHeight - 1) / m_TileHeight;
        LastLineOfTile = true;
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Write the block to the file
    for (uint32_t NoCol=0; (NoCol<m_TilePerWidth) && (Status == H_SUCCESS); NoCol++)
        {
        // Move to the Col at the first line
        pPosInBlock = pi_pData + (m_ExactBytesPerTileWidth * NoCol);

        uint32_t ExactBytesToCopy;
        if (NoCol == m_TilePerWidth - 1)
            {
            ExactBytesToCopy = m_ExactBytesToCopyForTheLastRightTile;
            // Clear the last Tile, on the right border
            memset(m_pTile, 0, m_TileSize);
            }
        else
            ExactBytesToCopy = m_ExactBytesPerTileWidth;

        for (uint32_t NoRow=0; (NoRow< CurrentTilePerBlock) && (Status == H_SUCCESS); NoRow++)
            {
            Byte* pPosInTile = m_pTile;

            uint32_t NumberOfScanLinesToCopy;
            if (LastLineOfTile && (NoRow == (CurrentTilePerBlock - 1)))
                {
                NumberOfScanLinesToCopy = ResolutionHeight - ((uint32_t)pi_PosBlockY + (NoRow*m_TileHeight));
                // The strip don't fill the tile completely, init it
                memset(m_pTile, 0, m_TileSize);
                }
            else
                NumberOfScanLinesToCopy = m_TileHeight;

            // Convert the tile data to the strip output buffer
            for (uint32_t NoLine=0; NoLine<NumberOfScanLinesToCopy; NoLine++)
                {
                memcpy(pPosInTile, pPosInBlock, ExactBytesToCopy);
                // Move to the next line
                pPosInTile += m_ExactBytesPerTileWidth;
                pPosInBlock += m_ExactBytesPerWidth;
                }

            Status = m_pAdaptedResolutionEditor->WriteBlock(NoCol*m_TileWidth, pi_PosBlockY+(NoRow*m_TileHeight), m_pTile, pi_pSisterFileLock);
            }
        }
    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }
