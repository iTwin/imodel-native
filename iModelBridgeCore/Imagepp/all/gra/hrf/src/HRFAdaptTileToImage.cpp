//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptTileToImage.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptTileToImage
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptTileToImage.h>

#include <Imagepp/all/h/HCDPacket.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptTileToImageCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptTileToImageCapabilities::HRFAdaptTileToImageCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_TILE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_IMAGE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptTileToImageCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptTileToImageCreator::GetCapabilities() const
    {
    return HRFAdaptTileToImageCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptTileToImageCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    unsigned short       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptTileToImage(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptTileToImage::HRFAdaptTileToImage(
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

    HASSERT(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerHeight() <= ULONG_MAX);

    m_TilePerHeight      = (uint32_t)m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerHeight();

    HASSERT(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);

    m_Height             = (uint32_t)m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetHeight();

    // Calc the number of bytes per Tile Width
    m_ExactBytesPerTileWidth =  (uint32_t)ceil(m_TileWidth * DBytesByPixel);
    m_ExactBytesToCopyForTheLastRightTile = (uint32_t)ceil((m_pResolutionDescriptor->GetWidth() - ((m_TilePerWidth-1) * m_TileWidth)) * DBytesByPixel);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptTileToImage::~HRFAdaptTileToImage()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToImage::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status = H_SUCCESS;

    // Alloc working buffer
    m_pTile = new Byte[m_TileSize];

    Byte* pPosInBlock = po_pData;

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

        for (uint32_t NoRow=0; (NoRow< m_TilePerHeight) && (Status == H_SUCCESS); NoRow++)
            {
            Status = m_pAdaptedResolutionEditor->ReadBlock(NoCol*m_TileWidth, NoRow*m_TileHeight, m_pTile, pi_pSisterFileLock);

            if (Status == H_SUCCESS)
                {
                Byte* pPosInTile = m_pTile;

                uint32_t NumberOfScanLinesToCopy;
                if (NoRow == m_TilePerHeight - 1)
                    NumberOfScanLinesToCopy = m_Height - (NoRow*m_TileHeight);
                else
                    NumberOfScanLinesToCopy = m_TileHeight;

                // Convert the tile data to the image output buffer
                for (uint32_t NoLine=0; NoLine<NumberOfScanLinesToCopy; NoLine++)
                    {
                    memcpy(pPosInBlock, pPosInTile, ExactBytesToCopy);
                    // Move to the next line
                    pPosInTile += m_ExactBytesPerTileWidth;
                    pPosInBlock += m_ExactBytesPerWidth;
                    }
                }
            }
        }
    SisterFileLock.ReleaseKey();

    // Free working buffer
    m_pTile = 0;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToImage::ReadBlock(uint64_t            pi_PosBlockX,
                                       uint64_t            pi_PosBlockY,
                                       HFCPtr<HCDPacket>&  po_rpPacket,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToImage::WriteBlock(uint64_t    pi_PosBlockX,
                                        uint64_t    pi_PosBlockY,
                                        const Byte* pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS         Status = H_SUCCESS;
    const Byte*    pPosInBlock = pi_pData;

    // Alloc working buffer
    m_pTile = new Byte[m_TileSize];

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
            memset(m_pTile, 0, m_TileSize);
            }
        else
            ExactBytesToCopy = m_ExactBytesPerTileWidth;

        for (uint32_t NoRow=0; (NoRow< m_TilePerHeight) && (Status == H_SUCCESS); NoRow++)
            {
            Byte* pPosInTile = m_pTile;
            uint32_t NumberOfScanLinesToCopy;
            if (NoRow == m_TilePerHeight - 1)
                {
                NumberOfScanLinesToCopy = m_Height - (NoRow*m_TileHeight);
                memset(m_pTile, 0, m_TileSize);
                }
            else
                NumberOfScanLinesToCopy = m_TileHeight;

            // Convert the tile data to the image output buffer
            for (uint32_t NoLine=0; NoLine<NumberOfScanLinesToCopy; NoLine++)
                {
                memcpy(pPosInTile, pPosInBlock, ExactBytesToCopy);
                // Move to the next line
                pPosInTile += m_ExactBytesPerTileWidth;
                pPosInBlock += m_ExactBytesPerWidth;
                }

            Status = m_pAdaptedResolutionEditor->WriteBlock(NoCol*m_TileWidth, NoRow*m_TileHeight, m_pTile, pi_pSisterFileLock);
            }
        }
    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    // Free working buffer
    m_pTile = 0;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToImage::WriteBlock(uint64_t                   pi_PosBlockX,
                                        uint64_t                   pi_PosBlockY,
                                        const HFCPtr<HCDPacket>&   pi_rpPacket,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status      = H_ERROR;

    return Status;
    }