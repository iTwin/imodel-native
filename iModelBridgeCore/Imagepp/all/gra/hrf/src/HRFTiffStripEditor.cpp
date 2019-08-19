//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTiffStripEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFTiffStripEditor.h>
#include <ImagePP/all/h/HRFTiffFile.h>
#include <ImagePP/all/h/HTIFFFile.h>
#include <ImagePP/all/h/HCDCodecHMRRLE1.h>
#include <ImagePP/all/h/HCDCodecCCITTFax4.h>
#include <ImagePP/all/h/HCDCodecHMRCCITT.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDPacketRLE.h>



#define RASTER_FILE     static_cast<HRFTiffFile*>(GetRasterFile().GetPtr())
#define TIFF_FILE       (static_cast<HRFTiffFile*>(GetRasterFile().GetPtr())->GetFilePtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFTiffStripEditor::HRFTiffStripEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                       uint32_t              pi_Page,
                                       uint16_t       pi_Resolution,
                                       HFCAccessMode         pi_AccessMode)

    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    uint16_t Photometric;

    m_IndexOfPage  = RASTER_FILE->GetIndexOfPage(m_Page);

    HTIFFFile::DirectoryID CurDir = TIFF_FILE->CurrentDirectory();
    TIFF_FILE->SetDirectory(HTIFFFile::MakeDirectoryID(HTIFFFile::STANDARD, m_IndexOfPage+pi_Resolution));

    TIFF_FILE->GetField(PHOTOMETRIC, &Photometric);

    uint32_t Compression;
    TIFF_FILE->GetField(COMPRESSION, &Compression);
    m_HasDecompressToRLE = COMPRESSION_CCITTFAX4 == Compression || COMPRESSION_HMR_RLE1 == Compression || COMPRESSION_CCITTRLE == Compression;

    TIFF_FILE->SetDirectory(CurDir);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFTiffStripEditor::~HRFTiffStripEditor()
    {
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffStripEditor::ReadBlock(uint64_t pi_PosBlockX,
                                        uint64_t pi_PosBlockY,
                                        Byte*  po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;
    
    if (!GetAccessMode().m_HasCreateAccess)
        {
        // Select the page and the resolution
        RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

        // read the strip
        uint64_t StripIndex = m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);
        HASSERT_X64(StripIndex < UINT32_MAX);

        Status = TIFF_FILE->StripRead(po_pData, (StripIndex <= UINT32_MAX) ? (uint32_t)StripIndex : 0);
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffStripEditor::ReadBlock(uint64_t pi_PosBlockX,
                                      uint64_t pi_PosBlockY,
                                      HFCPtr<HCDPacket>& po_rpPacket)

    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;

    if (!GetAccessMode().m_HasCreateAccess)
        {
        // Select the page and the resolution
        RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

        // Set ownership to true is the buffer size is zero
        if (po_rpPacket->GetBufferSize() == 0)
            {
            po_rpPacket->SetBufferOwnership(true);
            }

        // Read the compressed tile
        uint64_t StripIndex = m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);
        HASSERT_X64(StripIndex <= UINT32_MAX);

        Status = TIFF_FILE->StripRead(po_rpPacket, (StripIndex < UINT32_MAX) ? (uint32_t)StripIndex : 0);
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlockRLE
//-----------------------------------------------------------------------------
HSTATUS HRFTiffStripEditor::ReadBlockRLE(uint64_t pi_PosBlockX,
                                         uint64_t pi_PosBlockY,
                                         HFCPtr<HCDPacketRLE>&    po_rpPacketRLE)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffers.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= UINT32_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= UINT32_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= UINT32_MAX);

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
        HASSERT(!"Warning! Block is being read twice");
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
HSTATUS HRFTiffStripEditor::WriteBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       const Byte* pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;

    // Select the page and the resolution
    RASTER_FILE->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // write the strip
    uint64_t StripIndex = m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);
    HASSERT_X64(StripIndex <= UINT32_MAX);

    Status = TIFF_FILE->StripWrite(pi_pData, (StripIndex <= UINT32_MAX) ? (uint32_t)StripIndex : 0);

    // Update the directories.
    RASTER_FILE->SaveDescriptors();

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffStripEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                       uint64_t                 pi_PosBlockY,
                                       const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HPRECONDITION (pi_rpPacket != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::STRIP);
    HSTATUS Status = H_SUCCESS;

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
        size_t BufferSize = pi_rpPacket->GetDataSize() + LineIndexesTableSize;
        pTmpBuffer = new Byte[BufferSize];

        // copy the line indexes table
        memcpy(pTmpBuffer, pCodec->GetLineIndexesTable(), LineIndexesTableSize);

        // copy the image data itself
        memcpy(pTmpBuffer + LineIndexesTableSize, pi_rpPacket->GetBufferAddress(), pi_rpPacket->GetDataSize());

        uint64_t StripIndex = m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY);
        HASSERT_X64(StripIndex < UINT32_MAX);
        Status = TIFF_FILE->StripWriteCompress(pTmpBuffer, (uint32_t)BufferSize, (StripIndex < UINT32_MAX) ? (uint32_t)StripIndex : 0);
        }
    else
        {
        HASSERT_X64(m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY) <= UINT32_MAX);
        Status = TIFF_FILE->StripWriteCompress(pi_rpPacket->GetBufferAddress(), (uint32_t)pi_rpPacket->GetDataSize(), (uint32_t)m_pResolutionDescriptor->ComputeBlockIndex(pi_PosBlockX, pi_PosBlockY));
        }

    // Update the directories.
    RASTER_FILE->SaveDescriptors();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
// Edition by block
//-----------------------------------------------------------------------------
HSTATUS HRFTiffStripEditor::WriteBlockRLE(uint64_t pi_PosBlockX,
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
