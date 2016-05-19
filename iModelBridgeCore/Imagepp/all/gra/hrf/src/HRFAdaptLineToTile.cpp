//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptLineToTile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptLineToTile
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptLineToTile.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptLineToTileCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptLineToTileCapabilities::HRFAdaptLineToTileCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_LINE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_TILE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptLineToTileCreator)

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFAdaptLineToTileCreator::~HRFAdaptLineToTileCreator()
    {
    }

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptLineToTileCreator::GetCapabilities() const
    {
    return HRFAdaptLineToTileCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptLineToTileCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   uint16_t       pi_Resolution,
                                                   HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptLineToTile(GetCapabilities(),
                                  pi_rpRasterFile,
                                  pi_Page,
                                  pi_Resolution,
                                  pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptLineToTile::HRFAdaptLineToTile(HRFBlockAdapterCapabilities* pi_pCapabilities,
                                       HFCPtr<HRFRasterFile>        pi_rpRasterFile,
                                       uint32_t                     pi_Page,
                                       uint16_t              pi_Resolution,
                                       HFCAccessMode                pi_AccessMode)

    : HRFBlockAdapter(pi_pCapabilities,
                      pi_rpRasterFile,
                      pi_Page,
                      pi_Resolution,
                      pi_AccessMode)
    {
    // Resolution dimension
    HASSERT(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);
    HASSERT(m_pResolutionDescriptor->GetWidth() <= UINT32_MAX);
    HASSERT(m_pResolutionDescriptor->GetBlocksPerWidth() <= UINT32_MAX);

    m_Height             = (uint32_t)m_pResolutionDescriptor->GetHeight();
    m_Width              = (uint32_t)m_pResolutionDescriptor->GetWidth();

    // block dimension
    m_BlockHeight         = m_pResolutionDescriptor->GetBlockHeight();
    m_BlockWidth          = m_pResolutionDescriptor->GetBlockWidth();

    // Calc the number of bytes per Image Width
    m_DBytesByPixel        = m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() / 8.0;
    m_ExactBytesPerWidth = (uint32_t)ceil((double)m_Width * m_DBytesByPixel);

    // Calc the number of blocks per width
    m_BlocksPerWidth = (uint32_t)m_pResolutionDescriptor->GetBlocksPerWidth();

    // Calc the number of bytes per block Width
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
HRFAdaptLineToTile::~HRFAdaptLineToTile()
    {
    // Check to save the intern blocks
    if ( ( m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess )  && m_IsBlocksOverwritten )
        SaveTiles();
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToTile::ReadBlock(uint64_t pi_PosBlockX,
                                      uint64_t pi_PosBlockY,
                                      Byte*  po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);

    HSTATUS Status = m_LoadTilesStatus;

    // Check if the current location of intern blocks change
    if (pi_PosBlockY != m_PosTileY)
        {
        // if the intern blocks are overwritten we save these blocks
        if (m_IsBlocksOverwritten)
            Status = SaveTiles();

        // Load the client Block and beside blocks into the intern tile cache
        if (Status == H_SUCCESS)
            Status = LoadTiles((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);
        }
    else
        {
        // if the client Block is not loaded in the intern blocks we load the block into the intern cache
        if (m_IsBlocksEmpty)
            Status = LoadTiles((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);
        }

    // Alloc memory if not already done
    Alloc_m_ppBlocks();

    // Copy the data to the client buffer
    memcpy(po_pData, m_ppBlocks[pi_PosBlockX/m_BlockWidth], m_ExactBytesPerBlockWidth*m_BlockHeight);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToTile::WriteBlock(uint64_t    pi_PosBlockX,
                                       uint64_t    pi_PosBlockY,
                                       const Byte* pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_PosBlockX <= UINT32_MAX && pi_PosBlockY <= UINT32_MAX);
    
    HSTATUS Status = H_SUCCESS;

    // Check to save the intern blocks
    if ((pi_PosBlockY != m_PosTileY) && m_IsBlocksOverwritten)
        Status = SaveTiles();

    if ((Status == H_SUCCESS) && (m_AccessMode.m_HasReadAccess))
        {
        // Check if the client tile is loaded in the intern tiles
        if ((pi_PosBlockY != m_PosTileY) || m_IsBlocksEmpty)
            // Load the client block and beside blocks into the intern blocks cache
            Status = LoadTiles((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);
        }

    // Alloc memory if not already done
    Alloc_m_ppBlocks();

    // Copy the client buffer to the intern blocks
    memcpy(m_ppBlocks[pi_PosBlockX/m_BlockWidth], pi_pData, m_ExactBytesPerBlockWidth*m_BlockHeight);

    m_IsBlocksOverwritten = true;
    m_IsBlocksEmpty = false;
    m_PosTileY = (uint32_t)pi_PosBlockY;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// LoadTiles
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToTile::LoadTiles(uint32_t pi_PosBlockX,
                                      uint32_t pi_PosBlockY)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    m_LoadTilesStatus = H_SUCCESS;
    uint32_t NumberOfLines;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (pi_PosBlockY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - pi_PosBlockY;
    else
        NumberOfLines = m_BlockHeight;

    // allocate working buffer
    m_LineBuffer = new Byte[m_ExactBytesPerBlockWidth * m_BlocksPerWidth];
    Alloc_m_ppBlocks();

    // Load the client block and beside blocks into the intern blocks cache
    for (uint32_t NoLine = 0; (NoLine < NumberOfLines) &&
         (pi_PosBlockY+NoLine < m_Height) &&
         (m_LoadTilesStatus == H_SUCCESS); NoLine++)
        {
        m_LoadTilesStatus = m_pAdaptedResolutionEditor->ReadBlock(0, pi_PosBlockY+NoLine, m_LineBuffer);

        if (m_LoadTilesStatus == H_SUCCESS)
            {
            // adapt the line to the blocks
            uint32_t LinePosInTile = (NoLine*m_ExactBytesPerBlockWidth);
            Byte* pInLineBuffer = m_LineBuffer;
            for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
                {
                memcpy(m_ppBlocks[NoTile]+LinePosInTile,
                       pInLineBuffer,
                       m_ExactBytesPerBlockWidth);
                pInLineBuffer += m_ExactBytesPerBlockWidth;
                }
            }
        else
            {
            //If at least one line was read successfully, consider all
            //tiles intersecting the line valid.
            if (NoLine > 0)
                {
                // adapt the line to the blocks
                uint32_t LinePosInTile = (NoLine*m_ExactBytesPerBlockWidth);
                for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
                    {
                    memset(m_ppBlocks[NoTile]+LinePosInTile,
                           0,
                           m_ExactBytesPerBlockWidth * (NumberOfLines - NoLine));
                    }
                m_LoadTilesStatus = H_SUCCESS;
                }
            }
        }


    m_PosTileY = pi_PosBlockY;
    m_IsBlocksEmpty = false;
    m_IsBlocksOverwritten = false;
    m_LineBuffer    = 0;                    // Free working buffer

    return m_LoadTilesStatus;
    }

//-----------------------------------------------------------------------------
// public
// SaveTiles
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToTile::SaveTiles()
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;
    uint32_t NumberOfLines;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (m_PosTileY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - m_PosTileY;
    else
        NumberOfLines = m_BlockHeight;

    // allocate working buffer
    m_LineBuffer = new Byte[m_ExactBytesPerBlockWidth * m_BlocksPerWidth];
    Alloc_m_ppBlocks();

    // Save the intern blocks to the file
    for (uint32_t NoLine=0; (NoLine < NumberOfLines) &&
         (m_PosTileY+NoLine < m_Height) &&
         (Status == H_SUCCESS); NoLine++)
        {
        // adapt the blocks to the line
        uint32_t LinePosInTile = (NoLine*m_ExactBytesPerBlockWidth);
        Byte* pInLineBuffer = m_LineBuffer;
        for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
            {
            memcpy(pInLineBuffer,
                   m_ppBlocks[NoTile]+LinePosInTile,
                   m_ExactBytesPerBlockWidth);
            pInLineBuffer += m_ExactBytesPerBlockWidth;
            }
        // Write the line
        Status = m_pAdaptedResolutionEditor->WriteBlock(0, m_PosTileY+NoLine, m_LineBuffer);
        }

    m_IsBlocksOverwritten = false;
    m_LineBuffer    = 0;                    // Free working buffer

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
void HRFAdaptLineToTile::NoMoreRead()
    {
    HSTATUS Status = H_SUCCESS;

    if (m_IsBlocksOverwritten)
        Status = SaveTiles();

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
void HRFAdaptLineToTile::Alloc_m_ppBlocks()
    {
    if (m_ppBlocks[0] == 0)
        {
        // Allocate blocks
        for (uint32_t NoTile=0; NoTile < m_BlocksPerWidth; NoTile++)
            m_ppBlocks[NoTile] = new Byte[m_BlockHeight * m_ExactBytesPerBlockWidth];
        }
    }