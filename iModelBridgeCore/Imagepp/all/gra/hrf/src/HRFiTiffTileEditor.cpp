//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFiTiffTileEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFiTiffTileEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFiTiffTileEditor.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDPacket.h>

#define H_CONCURENCE 1

#define RASTER_FILE static_cast<HRFiTiffFile*>(GetRasterFile().GetPtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFiTiffTileEditor::HRFiTiffTileEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                       uint32_t              pi_Page,
                                       unsigned short       pi_Resolution,
                                       HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_IndexOfPage  = RASTER_FILE->GetIndexOfPage(m_Page);
    m_IscTiff = pi_rpRasterFile->IsCompatibleWith(HRFcTiffFile::CLASS_ID);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFiTiffTileEditor::~HRFiTiffTileEditor()
    {
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffTileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                      uint64_t pi_PosBlockY,
                                      Byte*  po_pData,
                                      HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // read the tile
    Status = RASTER_FILE->GetFilePtr()->TileRead(po_pData,
                                                 (uint32_t)pi_PosBlockX,
                                                 (uint32_t)pi_PosBlockY);
    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    // Empty block: - First data access when we are in creation mode
    //              - No data(empty tile or strip)
    // This is not an error.
    if(Status == H_DATA_NOT_AVAILABLE)
        {
        memset(po_pData, 0, m_pResolutionDescriptor->GetBlockSizeInBytes());
        Status = H_SUCCESS;
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffTileEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                      uint64_t           pi_PosBlockY,
                                      HFCPtr<HCDPacket>& po_rpPacket,
                                      HFCLockMonitor const* pi_pSisterFileLock)

    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // Set ownership to true is the buffer size is zero
    if (po_rpPacket->GetBufferSize() == 0)
        {
        po_rpPacket->SetBufferOwnership(true);
        }

    // read the data
    Status = RASTER_FILE->GetFilePtr()->TileRead (po_rpPacket, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    // Empty block: - First data access when we are in creation mode
    //              - No data(empty tile or strip)
    // This is not an error.
    if(Status == H_DATA_NOT_AVAILABLE)
        {
        if (po_rpPacket->GetBufferSize() < m_pResolutionDescriptor->GetBlockSizeInBytes())
            {
            if(po_rpPacket->HasBufferOwnership())
                po_rpPacket->SetBuffer(new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()], m_pResolutionDescriptor->GetBlockSizeInBytes());
            else
                return Status;  //  <<<<< ERROR
            }

        po_rpPacket->SetCodec(new HCDCodecIdentity());
        memset(po_rpPacket->GetBufferAddress(), 0, m_pResolutionDescriptor->GetBlockSizeInBytes());
        po_rpPacket->SetDataSize(m_pResolutionDescriptor->GetBlockSizeInBytes());
        Status = H_SUCCESS;
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffTileEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                       uint64_t     pi_PosBlockY,
                                       const Byte*  pi_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (pi_pData != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, GetRasterFile()->GetAccessMode().m_HasWriteShare);
        pi_pSisterFileLock = &SisterFileLock;
        }
    // Special case for iTiff editor.  They need to synchronize the sharing on a write operation because they support multiple editors.
    else if(GetRasterFile()->GetAccessMode().m_HasWriteShare && GetRasterFile()->SharingControlNeedSynchronization())
        {
        // File is already locked it is ok to synchronize.
        OnSynchronizedSharingControl();
        GetRasterFile()->SharingControlSynchronize();
        }

    // If the file is a cache file
    //            (To be a cache file, we need (m_IscTiff && static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode != HFC_NO_ACCESS) == true)
    //    If the original file is in read only && tile is already written in the cache file
    //        We do not have to rewrite the tile. (if == false)
    //    Else
    //        Write the tile in the CTiff (if == true)
    // Else
    //     We are working with a CTiff file directly, like a Itiff, write the tile in the CTiff (if == true)
    //   
    if (!(m_IscTiff && static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode != HFC_NO_ACCESS &&
        (!(static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasWriteAccess) &&
        !(static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasCreateAccess) )  &&
        ((GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY))) &
        (HRFDATAFLAG_LOADED | HRFDATAFLAG_OVERWRITTEN))))
        {
        // Select the page and the resolution
        RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

        // write the tile
        if ((Status = RASTER_FILE->GetFilePtr()->TileWrite(pi_pData, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY)) == H_SUCCESS)
            {
            if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY || !m_IscTiff)
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
            else
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
            }

        // Export the directories to the file.
        GetRasterFile()->SharingControlIncrementCount();

        if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
            RASTER_FILE->SaveDescriptors(m_Page);
        }
    else
        {
        // Try to rewrite a block already written, that should not happen.
        HASSERT_DATA(0);
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffTileEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                       uint64_t                 pi_PosBlockY,
                                       const HFCPtr<HCDPacket>& pi_rpPacket,
                                       HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (pi_rpPacket != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, GetRasterFile()->GetAccessMode().m_HasWriteShare);
        pi_pSisterFileLock = &SisterFileLock;
        }
    // Special case for iTiff editor.  They need to synchronize the sharing on a write operation because they support multiple editors.
    else if(GetRasterFile()->GetAccessMode().m_HasWriteShare && GetRasterFile()->SharingControlNeedSynchronization())
        {
        // File is already locked it is ok to synchronize.
        OnSynchronizedSharingControl();
        GetRasterFile()->SharingControlSynchronize();
        }

    // If the file is a cache file
    //            (To be a cache file, we need (m_IscTiff && static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode != HFC_NO_ACCESS) == true)
    //    If the original file is in read only && tile is already written in the cache file
    //        We do not have to rewrite the tile. (if == false)
    //    Else
    //        Write the tile in the CTiff (if == true)
    // Else
    //     We are working with a CTiff file directly, like a Itiff, write the tile in the CTiff (if == true)
    //   
    if (!(m_IscTiff && static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode != HFC_NO_ACCESS &&
          (!(static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasWriteAccess) &&
           !(static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasCreateAccess) )  &&
          ((GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY))) &
            (HRFDATAFLAG_LOADED | HRFDATAFLAG_OVERWRITTEN))))
        {
        // Select the page and the resolution
        RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

        // test if this is a RLE1 codec
        if(pi_rpPacket->GetCodec()->GetClassID() == HCDCodecHMRRLE1::CLASS_ID)
            {
            HFCPtr<HCDCodecHMRRLE1> pCodec = (HFCPtr<HCDCodecHMRRLE1>&)(pi_rpPacket->GetCodec());

            HASSERT(pCodec->HasLineIndexesTable());

            // if yes, merge the line indexes table with the data

            // create a new buffer
            size_t LineIndexesTableSize = pCodec->GetHeight() * sizeof(uint32_t);
            HArrayAutoPtr<Byte> pTmpBuffer;
            HASSERT_X64(pi_rpPacket->GetDataSize() + LineIndexesTableSize < ULONG_MAX);
            uint32_t BufferSize = (uint32_t)(pi_rpPacket->GetDataSize() + LineIndexesTableSize);
            pTmpBuffer = new Byte[BufferSize];

            // copy the line indexes table
            memcpy(pTmpBuffer, pCodec->GetLineIndexesTable(), LineIndexesTableSize);

            // copy the image data itself
            memcpy(pTmpBuffer + LineIndexesTableSize, pi_rpPacket->GetBufferAddress(), pi_rpPacket->GetDataSize());

            Status = RASTER_FILE->GetFilePtr()->TileWriteCompress(pTmpBuffer,
                     BufferSize, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);
            }
        else
            {
            HASSERT_X64(pi_rpPacket->GetDataSize() < ULONG_MAX);
            Status = RASTER_FILE->GetFilePtr()->TileWriteCompress(pi_rpPacket->GetBufferAddress(),
                     (uint32_t)pi_rpPacket->GetDataSize(), (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);
            }

        // Update the data flag
        if (Status == H_SUCCESS)
            {
            if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY || !m_IscTiff)
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
            else
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);

            GetRasterFile()->SharingControlIncrementCount();

            if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
                RASTER_FILE->SaveDescriptors(m_Page);
            }
        }
    else
        HASSERT(0);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFiTiffTileEditor::OnSynchronizedSharingControl()
    {
    RASTER_FILE->ReloadDescriptors();
    }