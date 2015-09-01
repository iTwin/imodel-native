//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCacheRandomBlockEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFCacheRandomBlockEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFCacheRandomBlockEditor.h>
#include <Imagepp/all/h/HFCMonitor.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFCacheRandomBlockEditor::HRFCacheRandomBlockEditor(
    HFCPtr<HRFRasterFile> pi_rpRasterFile,
    uint32_t              pi_Page,
    unsigned short       pi_Resolution,
    HFCAccessMode         pi_AccessMode,
    HRFResolutionEditor*  pi_pSrcResolutionEditor,
    HRFResolutionEditor*  pi_pCacheResolutionEditor)
    : HRFResolutionEditor(  pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    HPRECONDITION(pi_pSrcResolutionEditor->GetResolutionDescriptor()->GetReaderBlockAccess() == HRFBlockAccess::RANDOM);
    HFCMonitor Monitor(GetRasterFile()->GetKey());

    // Keep the editor for the source and cache files
    m_pSrcResolutionEditor   = pi_pSrcResolutionEditor;
    m_pCacheResolutionEditor = pi_pCacheResolutionEditor;
    }


//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFCacheRandomBlockEditor::~HRFCacheRandomBlockEditor()
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
HSTATUS HRFCacheRandomBlockEditor::ReadBlock(uint64_t pi_PosBlockX,
                                             uint64_t pi_PosBlockY,
                                             Byte*  po_pData,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status = H_SUCCESS;

    uint64_t TileIndex = GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);

    // Copy the specified Block to the client buffer
    // due to the concurence we load the block in the cache
    if ((m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag()[TileIndex] & HRFDATAFLAG_EMPTY) != HRFDATAFLAG_EMPTY)
        Status = m_pCacheResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_pData);
    else
        Status = H_NOT_FOUND;

    HPRECONDITION(GetResolutionDescriptor()->CountBlocks() < SIZE_MAX);
    if (memcmp(m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag(),
               GetResolutionDescriptor()->GetBlocksDataFlag(), (size_t)GetResolutionDescriptor()->CountBlocks()) != 0)
        {
        // Reload the flags
        GetResolutionDescriptor()->SetBlocksDataFlag(m_pCacheResolutionEditor->GetResolutionDescriptor()->GetBlocksDataFlag());
        }

    if (Status != H_SUCCESS)
        {
        if (!m_pSrcResolutionEditor->GetAccessMode().m_HasCreateAccess) // no block when we export
            {
            // Cache the empty tiles over the client tile position to the client tile position
            Monitor.ReleaseKey();
            Status = m_pSrcResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_pData);
            Monitor.Assign(GetRasterFile()->GetKey());

            if(Status == H_SUCCESS)
                {
                try
                    {
                    Status = m_pCacheResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, po_pData);
                    if(Status == H_SUCCESS)
                        // Update cache flags
                        GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY),
                                                                    HRFDATAFLAG_LOADED);
                    }
                catch(...)
                    {
                    Status = H_ERROR;

                    // Stop exception here. We don't mind if the write in
                    // the cache fails. We can still return the data.
                    }
                }
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheRandomBlockEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                             uint64_t           pi_PosBlockY,
                                             HFCPtr<HCDPacket>& po_rpPacket,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status = H_SUCCESS;

    uint64_t TileIndex = GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);

    // Copy the specified Block to the client buffer
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

        if (!m_pSrcResolutionEditor->GetAccessMode().m_HasCreateAccess) // no block when we export
            {
            HArrayAutoPtr<Byte> pDataBlock;

            pDataBlock = new Byte[m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()];

            // Cache the empty tiles over the client tile position to the client tile position
            Monitor.ReleaseKey();
            Status = m_pSrcResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, pDataBlock);
            Monitor.Assign(GetRasterFile()->GetKey());

            if(Status == H_SUCCESS)
                {
                try
                    {
                    Status = m_pCacheResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, pDataBlock);

                    if(Status == H_SUCCESS)
                        {
                        // Update cache flags
                        GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY),
                                                                    HRFDATAFLAG_LOADED);
                        // Re-read as compressed
                        Status = m_pCacheResolutionEditor->ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket);
                        }
                    }
                catch(...)
                    {
                    Status = H_ERROR;

                    // when fail use identity compression
                    uint32_t BufferSize = m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes();
                    po_rpPacket->SetBuffer(pDataBlock, BufferSize);
                    po_rpPacket->SetDataSize(BufferSize);
                    po_rpPacket->SetCodec(new HCDCodecIdentity(BufferSize));
                    po_rpPacket->SetBufferOwnership(true);
                    pDataBlock.release();
                    }
                }
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheRandomBlockEditor::WriteBlock(uint64_t      pi_PosBlockX,
                                              uint64_t      pi_PosBlockY,
                                              const Byte*   pi_pData,
                                              HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HFCMonitor Monitor(GetRasterFile()->GetKey());
    HSTATUS Status;

    // Copy the specified tile to the client buffer
    Status = m_pCacheResolutionEditor->WriteBlock(pi_PosBlockX, pi_PosBlockY, pi_pData);
    if (Status == H_SUCCESS)
        {
        if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY)
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
        else
            GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFCacheRandomBlockEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                              uint64_t                 pi_PosBlockY,
                                              const HFCPtr<HCDPacket>& pi_rpPacket,
                                              HFCLockMonitor const* pi_pSisterFileLock)
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