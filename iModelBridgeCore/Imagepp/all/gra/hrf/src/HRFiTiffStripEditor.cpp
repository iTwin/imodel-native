//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFiTiffStripEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFiTiffStripEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFiTiffStripEditor.h>
#include <Imagepp/all/h/HRFTiffFile.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HRFiTiffFile.h>
#include <Imagepp/all/h/HRFcTiffFile.h>
#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HCDCodecCCITTFax4.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDPacket.h>



#define H_CONCURENCE 1

#define RASTER_FILE     (static_cast<HRFTiffFile*>(GetRasterFile().GetPtr()))
#define TIFF_FILE       (static_cast<HRFTiffFile*>(GetRasterFile().GetPtr())->GetFilePtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFiTiffStripEditor::HRFiTiffStripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
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

    unsigned short Compression;
    TIFF_FILE->GetField(COMPRESSION, &Compression);
    m_HasDecompressToRLE = COMPRESSION_CCITTFAX4 == Compression || COMPRESSION_HMR_RLE1 == Compression;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFiTiffStripEditor::~HRFiTiffStripEditor()
    {
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
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

    // read the strip
    HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
    HASSERT(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
    Status = TIFF_FILE->StripRead(po_pData, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));

    // Unlock the sister file
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
HSTATUS HRFiTiffStripEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                       uint64_t           pi_PosBlockY,
                                       HFCPtr<HCDPacket>& po_rpPacket,
                                       HFCLockMonitor const* pi_pSisterFileLock)

    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
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
    HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
    Status = TIFF_FILE->StripRead (po_rpPacket, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));

    // Unlock the sister file.
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
// ReadBlockRLE
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                          uint64_t                 pi_PosBlockY,
                                          HFCPtr<HCDPacketRLE>&    po_rpPacketRLE,
                                          HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffers.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == m_pResolutionDescriptor->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= m_pResolutionDescriptor->GetBlockHeight());
    HPRECONDITION(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);
    HPRECONDITION(m_pResolutionDescriptor->GetWidth() <= ULONG_MAX);
    HPRECONDITION(m_pResolutionDescriptor->GetBytesPerBlockWidth() <= ULONG_MAX);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);

    if(m_HasDecompressToRLE)
        {
        HSTATUS           Status = H_SUCCESS;
        HFCPtr<HCDPacket> pCompressData = new HCDPacket(0,0);

        // Read compress data.
        if(H_SUCCESS != (Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, pCompressData, pi_pSisterFileLock)))
            return Status;

        // Make sure we can do the DecompressRLE
        if(Status == H_SUCCESS && pCompressData->GetCodec() && pCompressData->GetCodec()->GetRLEInterface())
            {
            HCDCodecRLEInterface*  pRLECodecInterface = pCompressData->GetCodec()->GetRLEInterface();
            HFCPtr<HCDCodecImage>& pCodecImage = (HFCPtr<HCDCodecImage>&)pCompressData->GetCodec();

            // Use HCDCodecImage size because it might be adjusted for last strip.
            HASSERT(po_rpPacketRLE->GetCodec()->GetWidth() == pCodecImage->GetWidth());
            HASSERT(po_rpPacketRLE->GetCodec()->GetHeight() >= pCodecImage->GetHeight());

            // Decompress directly in RLE.
            pRLECodecInterface->DecompressSubsetToRLE(pCompressData->GetBufferAddress(), pCompressData->GetDataSize(), po_rpPacketRLE);
            return H_SUCCESS;
            }

        // *** FALLTHROUGH to the default implementation. ***
        // It should not happen but htiff can play tricks on us and return something that is not compressed even though
        // the file itself is compressed. If it does happen we will reread the block on disk.
        // Another possibility is at creation time, we receive H_DATA_NOT_AVAILABLE, but convert to H_SUCCESS with CODEC identity, see ReadBlock
        HASSERTSUPERDEBUG(!"Warning! Block is being read twice");
        m_HasDecompressToRLE = false;   // do not retry for subsequent read.
        }

    // Use default implementation.
    return HRFResolutionEditor::ReadBlockRLE(pi_PosBlockX, pi_PosBlockY, po_rpPacketRLE, pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte*  pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
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
        !(static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasCreateAccess)) &&
        ((GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY))) &
        (HRFDATAFLAG_LOADED | HRFDATAFLAG_OVERWRITTEN))))
        {
        // Select the page and the resolution
        RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

        // write the strip
        HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
        if ((Status = TIFF_FILE->StripWrite(pi_pData, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY))) == H_SUCCESS)
            {
            if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY || !m_IscTiff)
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
            else
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
            }

        // Write the directories to the file.
        GetRasterFile()->SharingControlIncrementCount();

        if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
            static_cast<HRFiTiffFile*>(GetRasterFile().GetPtr())->SaveDescriptors();
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
HSTATUS HRFiTiffStripEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                        uint64_t                 pi_PosBlockY,
                                        const HFCPtr<HCDPacket>& pi_rpPacket,
                                        HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (pi_rpPacket != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
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

    // If the original file is in read only, we do not have to rewrite the tile if it is already
    // written in the cache file.
    if (!(m_IscTiff &&
          (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) &
           (HRFDATAFLAG_LOADED | HRFDATAFLAG_OVERWRITTEN)) &&
          (!static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasWriteAccess &&
           !static_cast<HRFcTiffFile*>(GetRasterFile().GetPtr())->m_OriginalFileAccessMode.m_HasCreateAccess)))

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

            HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
            Status = TIFF_FILE->StripWriteCompress(pTmpBuffer, BufferSize, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));
            }
        else
            {
            HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
            Status = TIFF_FILE->StripWriteCompress(pi_rpPacket->GetBufferAddress(), (uint32_t)pi_rpPacket->GetDataSize(), (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));
            }

        // Update the data flags
        if (Status == H_SUCCESS)
            {
            if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY || m_IscTiff)
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
            else
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);

            GetRasterFile()->SharingControlIncrementCount();

            if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
                static_cast<HRFiTiffFile*>(GetRasterFile().GetPtr())->SaveDescriptors();
            }
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
// Edition by block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::WriteBlockRLE(uint64_t pi_PosBlockX,
                                           uint64_t pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (pi_rpPacketRLE != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);

    // For now always use default implementation.
    // Problem: Htiff use internal compress routines. See m_pCompressFunc in HTIFFFileCodec.cpp
    return HRFResolutionEditor::WriteBlockRLE(pi_PosBlockX, pi_PosBlockY, pi_rpPacketRLE, pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFiTiffStripEditor::OnSynchronizedSharingControl()
    {
    static_cast<HRFiTiffFile*>(GetRasterFile().GetPtr())->ReloadDescriptors();
    }
