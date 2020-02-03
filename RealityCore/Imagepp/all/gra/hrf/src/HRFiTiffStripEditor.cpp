//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFiTiffStripEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFiTiffStripEditor.h>
#include <ImagePP/all/h/HRFTiffFile.h>
#include <ImagePP/all/h/HCDCodecHMRRLE1.h>
#include <ImagePP/all/h/HRFiTiffFile.h>
#include <ImagePP/all/h/HRFcTiffFile.h>
#include <ImagePP/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HCDPacketRLE.h>
#include <ImagePP/all/h/HCDCodecCCITTFax4.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDPacket.h>

#define RASTER_FILE     (static_cast<HRFTiffFile*>(GetRasterFile().GetPtr()))
#define TIFF_FILE       (static_cast<HRFTiffFile*>(GetRasterFile().GetPtr())->GetFilePtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFiTiffStripEditor::HRFiTiffStripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                         uint32_t              pi_Page,
                                         uint16_t       pi_Resolution,
                                         HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_IndexOfPage  = RASTER_FILE->GetIndexOfPage(m_Page);
    m_IscTiff = pi_rpRasterFile->IsCompatibleWith(HRFcTiffFile::CLASS_ID);

    uint16_t Compression;
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
                                       Byte*  po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // read the strip
    HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
    HASSERT(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
    Status = TIFF_FILE->StripRead(po_pData, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));

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
                                       HFCPtr<HCDPacket>& po_rpPacket)

    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;


    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // Set ownership to true is the buffer size is zero
    if (po_rpPacket->GetBufferSize() == 0)
        {
        po_rpPacket->SetBufferOwnership(true);
        }

    // read the data
    HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
    Status = TIFF_FILE->StripRead (po_rpPacket, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));


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
                                          HFCPtr<HCDPacketRLE>&    po_rpPacketRLE)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffers.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == m_pResolutionDescriptor->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= m_pResolutionDescriptor->GetBlockHeight());
    HPRECONDITION(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);
    HPRECONDITION(m_pResolutionDescriptor->GetWidth() <= UINT32_MAX);
    HPRECONDITION(m_pResolutionDescriptor->GetBytesPerBlockWidth() <= UINT32_MAX);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);

    if(m_HasDecompressToRLE)
        {
        HSTATUS           Status = H_SUCCESS;
        HFCPtr<HCDPacket> pCompressData = new HCDPacket(0,0);

        // Read compress data.
        if(H_SUCCESS != (Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, pCompressData)))
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
    return HRFResolutionEditor::ReadBlockRLE(pi_PosBlockX, pi_PosBlockY, po_rpPacketRLE);
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte*  pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;

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
        HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
        if ((Status = TIFF_FILE->StripWrite(pi_pData, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY))) == H_SUCCESS)
            {
            if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY || !m_IscTiff)
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
            else
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
            }

        if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
            static_cast<HRFiTiffFile*>(GetRasterFile().GetPtr())->SaveDescriptors();
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                        uint64_t                 pi_PosBlockY,
                                        const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HPRECONDITION (pi_rpPacket != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;


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
            HASSERT_X64(pi_rpPacket->GetDataSize() + LineIndexesTableSize < UINT32_MAX);
            uint32_t BufferSize = (uint32_t)(pi_rpPacket->GetDataSize() + LineIndexesTableSize);
            pTmpBuffer = new Byte[BufferSize];

            // copy the line indexes table
            memcpy(pTmpBuffer, pCodec->GetLineIndexesTable(), LineIndexesTableSize);

            // copy the image data itself
            memcpy(pTmpBuffer + LineIndexesTableSize, pi_rpPacket->GetBufferAddress(), pi_rpPacket->GetDataSize());

            HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
            Status = TIFF_FILE->StripWriteCompress(pTmpBuffer, BufferSize, (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));
            }
        else
            {
            HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
            Status = TIFF_FILE->StripWriteCompress(pi_rpPacket->GetBufferAddress(), (uint32_t)pi_rpPacket->GetDataSize(), (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));
            }

        // Update the data flags
        if (Status == H_SUCCESS)
            {
            if (GetResolutionDescriptor()->GetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY)) & HRFDATAFLAG_EMPTY || m_IscTiff)
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_LOADED);
            else
                GetResolutionDescriptor()->SetBlockDataFlag(GetResolutionDescriptor()->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY), HRFDATAFLAG_OVERWRITTEN);
            
            if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
                static_cast<HRFiTiffFile*>(GetRasterFile().GetPtr())->SaveDescriptors();
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
// Edition by block
//-----------------------------------------------------------------------------
HSTATUS HRFiTiffStripEditor::WriteBlockRLE(uint64_t pi_PosBlockX,
                                           uint64_t pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE)
    {
    HPRECONDITION (pi_rpPacketRLE != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);

    // For now always use default implementation.
    // Problem: Htiff use internal compress routines. See m_pCompressFunc in HTIFFFileCodec.cpp
    return HRFResolutionEditor::WriteBlockRLE(pi_PosBlockX, pi_PosBlockY, pi_rpPacketRLE);
    }
