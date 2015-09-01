//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptNTileToTile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptNTileToTile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptNTileToTile.h>


HFC_IMPLEMENT_SINGLETON(HRFAdaptNTileToTileCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptNTileToTileCapabilities::HRFAdaptNTileToTileCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_TILE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_TILE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_N_BLOCK);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptNTileToTileCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptNTileToTileCreator::GetCapabilities() const
    {
    return HRFAdaptNTileToTileCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptNTileToTileCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    unsigned short       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptNTileToTile(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptNTileToTile::HRFAdaptNTileToTile(  HRFBlockAdapterCapabilities* pi_pCapabilities,
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
    //N-Tiles to 1 Tile
    HPRECONDITION(GetResolutionDescriptor()->GetBlockHeight() >= m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetBlockWidth() >= m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockWidth());

    // Source Tile information
    m_TileHeight        = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();
    m_TilePerBlockHeight= GetResolutionDescriptor()->GetBlockHeight() / m_TileHeight;

    m_TileWidth         = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockWidth();
    m_TilePerBlockWidth = GetResolutionDescriptor()->GetBlockWidth() / m_TileWidth;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptNTileToTile::~HRFAdaptNTileToTile()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptNTileToTile::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status = H_SUCCESS;

    Byte* pPosInBigTile = po_pData;
    uint32_t NumberOfBlockToAccessPerHeight = m_TilePerBlockHeight;
    uint32_t NumberOfBlockToAccessPerWidth  = m_TilePerBlockWidth;

    // Adjust if necessary the number of Tile at the resolution height
    if (pi_PosBlockY + (m_TilePerBlockHeight*m_TileHeight) >= m_pResolutionDescriptor->GetHeight())
        {
        HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

        NumberOfBlockToAccessPerHeight = (((uint32_t)m_pResolutionDescriptor->GetHeight() + m_TileHeight -1) - (uint32_t)pi_PosBlockY) / m_TileHeight;
        }

    // Adjust if necessary the number of Tile at the resolution width
    if (pi_PosBlockX + (m_TilePerBlockWidth*m_TileWidth) >= m_pResolutionDescriptor->GetWidth())
        {
        HASSERT(m_pResolutionDescriptor->GetWidth() <= ULONG_MAX);

        NumberOfBlockToAccessPerWidth = (((uint32_t)m_pResolutionDescriptor->GetWidth() + m_TileWidth -1) - (uint32_t)pi_PosBlockX) / m_TileWidth;
        }

    HArrayAutoPtr<Byte> pSmallTile(new Byte[m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()]);
    Byte* pPosInSmallTile = pSmallTile;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Load the block from file to the client data out
    for (uint32_t NoTileWidth=0; (NoTileWidth < NumberOfBlockToAccessPerWidth) && (Status == H_SUCCESS); NoTileWidth++)
        {
        // reset to the first line in the buffer
        pPosInBigTile = po_pData;

        // Move to the current NoTileWidth
        pPosInBigTile += (m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth() * NoTileWidth);

        for (uint32_t NoTileHeight=0; (NoTileHeight < NumberOfBlockToAccessPerHeight) && (Status == H_SUCCESS); NoTileHeight++)
            {
            Status = m_pAdaptedResolutionEditor->ReadBlock(pi_PosBlockX + (NoTileWidth  * m_TileWidth),
                                                           pi_PosBlockY + (NoTileHeight * m_TileHeight),
                                                           pSmallTile, pi_pSisterFileLock);
            if (Status == H_SUCCESS)
                {
                // reset to the first line in the buffer
                pPosInSmallTile = pSmallTile;

                for (uint32_t Line=0; Line < m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight(); Line++)
                    {
                    memcpy(pPosInBigTile, pPosInSmallTile, m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth());

                    // Move inside the buffer to the next line
                    pPosInBigTile   += m_pResolutionDescriptor->GetBytesPerBlockWidth();
                    pPosInSmallTile += m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth();
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
HSTATUS HRFAdaptNTileToTile::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte*  pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    const Byte* pPosInBigTile = pi_pData;

    uint32_t NumberOfBlockToAccessPerHeight = m_TilePerBlockHeight;
    uint32_t NumberOfBlockToAccessPerWidth  = m_TilePerBlockWidth;

    // Adjust if necessary the number of Tile at the resolution height
    if (pi_PosBlockY + (m_TilePerBlockHeight*m_TileHeight) >= m_pResolutionDescriptor->GetHeight())
        {
        HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

        NumberOfBlockToAccessPerHeight = (((uint32_t)m_pResolutionDescriptor->GetHeight() + m_TileHeight -1) - (uint32_t)pi_PosBlockY) / m_TileHeight;
        }

    // Adjust if necessary the number of Tile at the resolution width
    if (pi_PosBlockX + (m_TilePerBlockWidth*m_TileWidth) >= m_pResolutionDescriptor->GetWidth())
        {
        HASSERT(m_pResolutionDescriptor->GetWidth() <= ULONG_MAX);

        NumberOfBlockToAccessPerWidth = (((uint32_t)m_pResolutionDescriptor->GetWidth() + m_TileWidth -1) - (uint32_t)pi_PosBlockX) / m_TileWidth;
        }

    HArrayAutoPtr<Byte> pSmallTile(new Byte[m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()]);
    Byte* pPosInSmallTile = pSmallTile;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Load the block from file to the client data out
    for (uint32_t NoTileWidth=0; (NoTileWidth < NumberOfBlockToAccessPerWidth) && (Status == H_SUCCESS); NoTileWidth++)
        {
        // reset to the first line in the buffer
        pPosInBigTile = pi_pData;

        // Move to the current NoTileWidth
        pPosInBigTile += (m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth() * NoTileWidth);

        for (uint32_t NoTileHeight=0; (NoTileHeight < NumberOfBlockToAccessPerHeight) && (Status == H_SUCCESS); NoTileHeight++)
            {
            // reset to the first line in the buffer
            pPosInSmallTile = pSmallTile;

            for (uint32_t Line=0; Line < m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight(); Line++)
                {
                memcpy(pPosInSmallTile, pPosInBigTile, m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth());

                // Move inside the buffer to the next line
                pPosInBigTile   += m_pResolutionDescriptor->GetBytesPerBlockWidth();
                pPosInSmallTile += m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth();
                }

            Status = m_pAdaptedResolutionEditor->WriteBlock(pi_PosBlockX + (NoTileWidth  * m_TileWidth),
                                                            pi_PosBlockY + (NoTileHeight * m_TileHeight),
                                                            pSmallTile, pi_pSisterFileLock);
            }
        }
    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }

