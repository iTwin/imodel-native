//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCacheSequentialBlockEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFCacheSequentialBlockEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFCacheSequentialBlockEditor.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDPacket.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFCacheSequentialBlockEditor::HRFCacheSequentialBlockEditor(
    HFCPtr<HRFRasterFile> pi_rpRasterFile,
    uint32_t              pi_Page,
    unsigned short       pi_Resolution,
    HFCAccessMode         pi_AccessMode,
    HRFResolutionEditor*  pi_pSrcResolutionEditor,
    HRFResolutionEditor*  pi_pCacheResolutionEditor)
    : HRFResolutionEditor(  pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    HFCMonitor Monitor(GetRasterFile()->GetKey());

    // Keep the editor for the source and cache files
    m_pSrcResolutionEditor   = pi_pSrcResolutionEditor;
    m_pCacheResolutionEditor = pi_pCacheResolutionEditor;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFCacheSequentialBlockEditor::~HRFCacheSequentialBlockEditor()
    {
    if (GetResolutionDescriptor()->PaletteHasChanged())
        {
        m_pSrcResolutionEditor->SetPalette(GetResolutionDescriptor()->GetPixelType()->GetPalette());
        m_pCacheResolutionEditor->SetPalette(GetResolutionDescriptor()->GetPixelType()->GetPalette());
        }
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    }


//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheSequentialBlockEditor::ReadBlock(uint64_t                pi_PosBlockX,
                                                 uint64_t                pi_PosBlockY,
                                                 Byte*                   po_pData,
                                                 HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status = H_SUCCESS;

    // compute the index for this block
    uint64_t TileIndex = GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);

    // Copy the specified Block to the client buffer
    // due to the concurence we load the block in the cache
    if ((m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag()[TileIndex] & HRFDATAFLAG_EMPTY) != HRFDATAFLAG_EMPTY)
        Status = m_pCacheResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_pData);
    else
        Status = H_NOT_FOUND;

    HPRECONDITION(GetResolutionDescriptor()->CountBlocks() <= SIZE_MAX);
    if (memcmp(m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag(),
               GetResolutionDescriptor()->GetBlocksDataFlag(), (size_t)GetResolutionDescriptor()->CountBlocks()) != 0)
        {
        // Reload the flags
        GetResolutionDescriptor()->SetBlocksDataFlag(m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag());
        }

    if (Status != H_SUCCESS)
        {
        // the block is empty in the cache, generate the block
        if (!m_pSrcResolutionEditor->GetAccessMode().m_HasCreateAccess) // no block when we export
            {
            // the default value equal to the client position
            uint64_t PosBlockY = pi_PosBlockY;
            uint32_t PosBlockX = 0;
            uint32_t BlockWidth  = GetResolutionDescriptor()->GetBlockWidth();
            uint32_t BlockHeight = GetResolutionDescriptor()->GetBlockHeight();
            uint64_t BlocksPerWidth = GetResolutionDescriptor()->GetBlocksPerWidth();

            // Verify if we need to generate the block rows before the one needed
            // (The source is sequential...)
            while ((PosBlockY > 0) &&
                   (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(0, PosBlockY - BlockHeight)) & HRFDATAFLAG_EMPTY))
                {
                PosBlockY -= BlockHeight;
                }

            HSTATUS BlockStatus = H_SUCCESS;

            // Cache the empty blocks over the specified block position
            while ((PosBlockY <=  pi_PosBlockY) && (BlockStatus == H_SUCCESS))
                {
                PosBlockX = 0;
                for (uint32_t NoBlock = 0; (NoBlock < BlocksPerWidth) && (BlockStatus == H_SUCCESS); NoBlock++)
                    {
                    // Cache the empty blocks over the specified block position
                    BlockStatus = m_pSrcResolutionEditor->ReadBlock(PosBlockX, PosBlockY, po_pData);

                    if(BlockStatus == H_SUCCESS)
                        {
                        // Write the read data in the cache and if successful, update the dataflag to loaded
                        BlockStatus = m_pCacheResolutionEditor->WriteBlock(PosBlockX, PosBlockY, po_pData);
                        if(BlockStatus == H_SUCCESS)
                            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(PosBlockX, PosBlockY),
                                                                        HRFDATAFLAG_LOADED);
                        }

                    // Proceed to the next column of blocks
                    PosBlockX += BlockWidth;
                    }

                // proceed to the next row of blocks
                PosBlockY += BlockHeight;
                }

            if (BlockStatus == H_SUCCESS && pi_PosBlockY + BlockHeight >= m_pResolutionDescriptor->GetHeight())
                m_pSrcResolutionEditor->NoMoreRead();

            // Copy the specified Block to the client buffer
            if ((BlockStatus == H_SUCCESS) && ((PosBlockX != pi_PosBlockX) || (PosBlockY != pi_PosBlockY)))
                BlockStatus = m_pCacheResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_pData);

            Status = BlockStatus;
            }
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheSequentialBlockEditor::ReadBlock(uint64_t                pi_PosBlockX,
                                                 uint64_t                pi_PosBlockY,
                                                 HFCPtr<HCDPacket>&      po_rpPacket,
                                                 HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status = H_SUCCESS;

    // Compute the block index for this block
    uint64_t TileIndex = GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);

    // Copy the specified Block to the client buffer
    // due to the concurence we load the block in the cache
    if ((m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag()[TileIndex] & HRFDATAFLAG_EMPTY) != HRFDATAFLAG_EMPTY)
        Status = m_pCacheResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);
    else
        Status = H_NOT_FOUND;

    HPRECONDITION(GetResolutionDescriptor()->CountBlocks() <= SIZE_MAX);
    if (memcmp(m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag(),
               GetResolutionDescriptor()->GetBlocksDataFlag(), (size_t)GetResolutionDescriptor()->CountBlocks()) != 0)
        {
        // Reload the flags
        GetResolutionDescriptor()->SetBlocksDataFlag(m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag());
        }

    if (Status != H_SUCCESS)
        {
        // the block is empty in the cache, generate the block
        if (!m_pSrcResolutionEditor->GetAccessMode().m_HasCreateAccess) // no block when we export
            {
            // the default value equal to the client position
            uint64_t PosBlockY = pi_PosBlockY;
            uint32_t PosBlockX = 0;
            uint32_t BlockWidth  = GetResolutionDescriptor()->GetBlockWidth();
            uint32_t BlockHeight = GetResolutionDescriptor()->GetBlockHeight();
            uint64_t BlocksPerWidth = GetResolutionDescriptor()->GetBlocksPerWidth();

            // Verify if we need to generate the block rows before the one needed
            // (The source is sequential...)
            while ((PosBlockY > 0) &&
                   (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(0, PosBlockY - BlockHeight)) & HRFDATAFLAG_EMPTY))
                {
                PosBlockY -= BlockHeight;
                }

            HSTATUS BlockStatus = H_SUCCESS;

            HArrayAutoPtr<Byte> pDataBlock;
            pDataBlock = new Byte[m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()];

            // Cache the empty blocks over the specified block position
            while ((PosBlockY <=  pi_PosBlockY) && (BlockStatus == H_SUCCESS))
                {
                PosBlockX = 0;
                for (uint32_t NoBlock = 0; (NoBlock < BlocksPerWidth) && (BlockStatus == H_SUCCESS); NoBlock++)
                    {
                    BlockStatus  = m_pSrcResolutionEditor->ReadBlock(PosBlockX, PosBlockY, pDataBlock);

                    // Read the current block from the source
                    if(BlockStatus == H_SUCCESS)
                        {
                        // Write the read data in the cache and if successful, update the dataflag to loaded
                        BlockStatus = m_pCacheResolutionEditor->WriteBlock(PosBlockX, PosBlockY, pDataBlock);

                        if(BlockStatus == H_SUCCESS)
                            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(PosBlockX, PosBlockY),
                                                                        HRFDATAFLAG_LOADED);
                        }

                    // Proceed to the next column of blocks
                    PosBlockX += BlockWidth;
                    }

                // proceed to the next row of blocks
                PosBlockY += BlockHeight;
                }

            // Re-read as compressed
            if (BlockStatus == H_SUCCESS)
                BlockStatus = m_pCacheResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);

            Status = BlockStatus;
            }
        }
    return Status;
    }


//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheSequentialBlockEditor::WriteBlock(uint64_t                pi_PosBlockX,
                                                  uint64_t                pi_PosBlockY,
                                                  const Byte*             pi_pData,
                                                  HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status;

    // Copy the specified tile to the client buffer
    if (H_SUCCESS ==
        (Status = m_pCacheResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, pi_pData)))
        {
        if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY)
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY),
                                                        HRFDATAFLAG_LOADED);
        else
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY),
                                                        HRFDATAFLAG_OVERWRITTEN);
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheSequentialBlockEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                                  uint64_t                 pi_PosBlockY,
                                                  const HFCPtr<HCDPacket>& pi_rpPacket,
                                                  HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_rpPacket != 0);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status;

    // Copy the specified tile to the client buffer
    if (H_SUCCESS ==
        (Status = m_pCacheResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, pi_rpPacket)))
        {
        if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY)
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
        else
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
        }

    return Status;
    }