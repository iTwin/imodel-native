//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptTileToLine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptTileToLine
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptTileToLine.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptTileToLineCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptTileToLineCapabilities::HRFAdaptTileToLineCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_TILE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_LINE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptTileToLineCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptTileToLineCreator::GetCapabilities() const
    {
    return HRFAdaptTileToLineCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptTileToLineCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   unsigned short       pi_Resolution,
                                                   HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptTileToLine(GetCapabilities(),
                                  pi_rpRasterFile,
                                  pi_Page,
                                  pi_Resolution,
                                  pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptTileToLine::HRFAdaptTileToLine(
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
    m_BufferedTileIndexY = -1;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    m_RasterHeight       = (uint32_t)m_pResolutionDescriptor->GetHeight();
    m_NextLineToWrite  = 0;


    // Calc the number of bytes per Tile Width
    m_ExactBytesPerTileWidth =  (uint32_t)ceil(m_TileWidth * DBytesByPixel);
    m_ExactBytesToCopyForTheLastRightTile = (uint32_t)ceil((m_pResolutionDescriptor->GetWidth() - ((m_TilePerWidth-1) * m_TileWidth)) * DBytesByPixel);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptTileToLine::~HRFAdaptTileToLine()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToLine::ReadBlock(uint64_t pi_PosBlockX,
                                      uint64_t pi_PosBlockY,
                                      Byte*  po_pData,
                                      HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    uint32_t TileIndexY = (uint32_t)pi_PosBlockY / m_TileHeight;
    uint32_t LineInTile = (uint32_t)pi_PosBlockY % m_TileHeight;

    if (TileIndexY != m_BufferedTileIndexY)
        {
        ReadAStripOfTiles (TileIndexY, pi_pSisterFileLock);
        m_BufferedTileIndexY = TileIndexY;
        }

    HASSERT(m_ppTiles != 0);

    // Process each tiles in a line
    for (uint32_t NoCol=0; (NoCol<m_TilePerWidth) && (Status == H_SUCCESS); NoCol++)
        {
        HASSERT(m_ppTiles[NoCol] != 0);

        Byte* pPosInBlock = po_pData + (m_ExactBytesPerTileWidth * NoCol);

        uint32_t ExactBytesToCopy;
        if (NoCol == m_TilePerWidth - 1)
            ExactBytesToCopy = m_ExactBytesToCopyForTheLastRightTile;
        else
            ExactBytesToCopy = m_ExactBytesPerTileWidth;

        Byte* pPosInTile = m_ppTiles[NoCol] + LineInTile * m_ExactBytesPerTileWidth;

        // Copy the tile data to the line output buffer
        memcpy(pPosInBlock, pPosInTile, ExactBytesToCopy);
        }

    if (pi_PosBlockY == m_RasterHeight-1)
        Delete_m_ppTiles();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToLine::WriteBlock(uint64_t     pi_PosBlockX,
                                       uint64_t     pi_PosBlockY,
                                       const Byte*  pi_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    HASSERT (m_NextLineToWrite == pi_PosBlockY);
    m_NextLineToWrite++;

    uint32_t TileIndexY = (uint32_t)pi_PosBlockY / m_TileHeight;
    uint32_t LineInTile = (uint32_t)pi_PosBlockY % m_TileHeight;

    // Alloc working buffer
    if ( m_ppTiles == 0)
        Alloc_m_ppTiles ();

    // Check if we need to fill the buffer, padding at the end, because tile data is padded.
    // If we begin a new Strip of tile
    if (LineInTile == 0)
        {
        // Check if this strip of tile is the last to write.
        if (((TileIndexY+1) * m_TileHeight) > m_pResolutionDescriptor->GetHeight())
            {
            // Fill the padding at bottom with 0
            for (uint32_t NoCol=0; NoCol<m_TilePerWidth; ++NoCol)
                memset(m_ppTiles[NoCol], 0, m_TileHeight * m_ExactBytesPerTileWidth);
            }
        }

    // Process each tiles in a line
    for (uint32_t NoCol=0; (NoCol<m_TilePerWidth) && (Status == H_SUCCESS); NoCol++)
        {
        HASSERT(m_ppTiles[NoCol] != 0);

        const Byte* pPosInBlock = pi_pData + (m_ExactBytesPerTileWidth * NoCol);

        uint32_t ExactBytesToCopy;
        if (NoCol == m_TilePerWidth - 1)
            ExactBytesToCopy = m_ExactBytesToCopyForTheLastRightTile;
        else
            ExactBytesToCopy = m_ExactBytesPerTileWidth;

        Byte* pPosInTile = m_ppTiles[NoCol] + LineInTile * m_ExactBytesPerTileWidth;

        // Copy the tile data to the line output buffer
        memcpy( pPosInTile, pPosInBlock, ExactBytesToCopy );
        }

    if (LineInTile == m_TileHeight - 1 || pi_PosBlockY == m_RasterHeight-1)
        Status = WriteAStripOfTiles (TileIndexY, pi_pSisterFileLock);

    if (pi_PosBlockY == m_RasterHeight-1)
        Delete_m_ppTiles();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// Alloc_m_ppTiles : Allocate memory if not already done
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptTileToLine::Alloc_m_ppTiles()
    {
    m_ppTiles = new HArrayAutoPtr<Byte> [m_TilePerWidth];

    // Allocate blocks
    for (uint32_t NoTile=0; NoTile < m_TilePerWidth; NoTile++)
        m_ppTiles[NoTile] = new Byte[m_TileHeight * m_ExactBytesPerTileWidth];

    // Fill the padding at right with 0
    memset(m_ppTiles[m_TilePerWidth-1], 0, m_TileHeight * m_ExactBytesPerTileWidth);
    }
//-----------------------------------------------------------------------------
// public
// Delete_m_ppTiles : Release memory
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptTileToLine::Delete_m_ppTiles()
    {
    m_ppTiles = 0;
    m_BufferedTileIndexY = -1;
    }

//-----------------------------------------------------------------------------
// Private
// ReadAStripOfTiles :
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToLine::ReadAStripOfTiles(uint32_t pi_TileIndexY, HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS Status   = H_SUCCESS;
    uint32_t PosBlocY = pi_TileIndexY * m_TileHeight;

    HASSERT(PosBlocY < m_pResolutionDescriptor->GetHeight());

    // Alloc working buffer
    if (m_ppTiles == 0)
        Alloc_m_ppTiles ();

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }
    // Read each tiles in a line
    for (uint32_t NoCol=0; (NoCol<m_TilePerWidth) && (Status == H_SUCCESS); NoCol++)
        {
        Status = m_pAdaptedResolutionEditor->ReadBlock(NoCol*m_TileWidth, PosBlocY, m_ppTiles[NoCol], pi_pSisterFileLock);
        }

    return Status;
    }
//-----------------------------------------------------------------------------
// Private
// WriteAStripOfTiles :
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptTileToLine::WriteAStripOfTiles (uint32_t pi_TileIndexY, HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS Status   = H_SUCCESS;
    uint32_t PosBlocY = pi_TileIndexY * m_TileHeight;

    HASSERT (PosBlocY < m_pResolutionDescriptor->GetHeight());
    HASSERT (m_ppTiles != 0);

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Write each tiles in a line
    for (uint32_t NoCol=0; (NoCol<m_TilePerWidth) && (Status == H_SUCCESS); NoCol++)
        {
        HASSERT (m_ppTiles[NoCol] != 0);

        Status = m_pAdaptedResolutionEditor->WriteBlock(NoCol*m_TileWidth, PosBlocY, m_ppTiles[NoCol], pi_pSisterFileLock);
        }
    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }
