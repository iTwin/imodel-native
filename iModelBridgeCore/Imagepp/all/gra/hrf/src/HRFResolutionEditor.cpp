//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFResolutionEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFResolutionEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFResolutionEditor.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecImage.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFResolutionDescriptor.h>
#include <Imagepp/all/h/HFCAccessMode.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFResolutionEditor::HRFResolutionEditor(HFCPtr<HRFRasterFile>    pi_rpRasterFile,
                                         uint32_t                 pi_Page,
                                         unsigned short          pi_Resolution,
                                         HFCAccessMode            pi_AccessMode)
    {
    HPRECONDITION (pi_rpRasterFile != 0);

    m_Page                    = pi_Page;
    m_Resolution              = pi_Resolution;
    m_ResolutionFactor        = 0.0;

    m_pRasterFile             = pi_rpRasterFile;
    m_pResolutionDescriptor   = m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(m_Resolution);

    m_pResolutionCapabilities = m_pRasterFile->GetCapabilities();
    m_AccessMode              = pi_AccessMode;

    m_pRasterFile->RegisterResolutionEditor(this);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFResolutionEditor::HRFResolutionEditor(HFCPtr<HRFRasterFile>    pi_rpRasterFile,
                                         uint32_t                 pi_Page,
                                         double                  pi_ResolutionFactor,
                                         HFCAccessMode            pi_AccessMode)
    {
    HPRECONDITION (pi_rpRasterFile != 0);
    HPRECONDITION (pi_ResolutionFactor > 0.0);

    m_Page                    = pi_Page;
    m_Resolution              = 0;
    m_ResolutionFactor        = pi_ResolutionFactor;
    m_pRasterFile             = pi_rpRasterFile;

    // find an index for this editor
    HRFRasterFile::ResolutionEditorRegistry::const_iterator Itr(m_pRasterFile->m_ResolutionEditorRegistry.begin());
    while (Itr != m_pRasterFile->m_ResolutionEditorRegistry.end())
        {
        if ((*Itr)->GetPage() == pi_Page && (*Itr)->GetResolutionIndex() == m_Resolution)
            {
            ++m_Resolution;
            Itr = m_pRasterFile->m_ResolutionEditorRegistry.begin();
            }
        else
            Itr++;
        }

    // for unlimited resolution raster, m_pResolutionDescriptor must be initialize by the child

    m_pResolutionCapabilities = m_pRasterFile->GetCapabilities();
    HPRECONDITION(m_pResolutionCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                                                                 pi_AccessMode) != 0);
    HPRECONDITION(static_cast<HRFMultiResolutionCapability*>(m_pResolutionCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                   pi_AccessMode).GetPtr())->IsUnlimitedResolution());

    m_AccessMode              = pi_AccessMode;

    m_pRasterFile->RegisterResolutionEditor(this);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFResolutionEditor::~HRFResolutionEditor()
    {
    m_pRasterFile->UnregisterResolutionEditor(this);
    }

//-----------------------------------------------------------------------------
// Public
// GetAccessMode
// Access mode management
//-----------------------------------------------------------------------------
HFCAccessMode HRFResolutionEditor::GetAccessMode() const
    {
    return m_AccessMode;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResolutionEditor::ReadBlock(uint64_t             pi_PosBlockX,
                                       uint64_t             pi_PosBlockY,
                                       Byte*                po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth()  <= ULONG_MAX) &&
                  (pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight() <= ULONG_MAX));
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_SUCCESS;

    // Display a message that we pass through the default ReadBlock with
    // a packet, which decompressed the data into an Identify packet
    HDEBUGTEXT(L"Warning! Using the default ReadBlock(Byte*)\r\n");

    // we take for granted that there is compression
    // we decompress the data in the output buffer
    HFCPtr<HCDPacket> pCompressed = new HCDPacket();
    Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, pCompressed, pi_pSisterFileLock);

    if (Status == H_SUCCESS)
        {
        HCDPacket Uncompressed(po_pData, m_pResolutionDescriptor->GetBlockSizeInBytes());
        pCompressed->Decompress(&Uncompressed);
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResolutionEditor::ReadBlock(uint64_t             pi_PosBlockX,
                                         uint64_t             pi_PosBlockY,
                                         HFCPtr<HCDPacket>&    po_rpPacket,
                                         HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth()  <= ULONG_MAX) &&
                  (pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight() <= ULONG_MAX));
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    // Display a message that we pass through the default ReadBlock with
    // a packet, which decompressed the data into an Identify packet
    HDEBUGTEXT(L"Warning! Using the default ReadBlock(Packet) which uncompresses the data\r\n");

    // we take for granted that there is no compression
    // we get the data uncompressed and associate to it an identity codec
    // test if there is a buffer already defined
    if(po_rpPacket->GetBufferSize() == 0)
        {
        // if not, create a buffer
        po_rpPacket->SetBuffer(new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()],
            m_pResolutionDescriptor->GetBlockSizeInBytes());
        po_rpPacket->SetBufferOwnership(true);
        }

    po_rpPacket->SetDataSize(m_pResolutionDescriptor->GetBlockSizeInBytes());
    po_rpPacket->SetCodec(new HCDCodecIdentity(m_pResolutionDescriptor->GetBlockSizeInBytes()));

    return ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket->GetBufferAddress(), pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResolutionEditor::ReadBlockRLE(uint64_t                pi_PosBlockX,
                                          uint64_t                pi_PosBlockY,
                                          HFCPtr<HCDPacketRLE>&   po_rpPacketRLE,
                                          HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockX + m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(m_Resolution)->GetBlockWidth() <= ULONG_MAX) &&
                  (pi_PosBlockY + m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(m_Resolution)->GetBlockHeight() <= ULONG_MAX));
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);

    // Display a message that we pass through the default ReadBlock with
    // a packet, which decompressed the data into an Identify packet
    //HDEBUGTEXT(L"Warning! Using the default ReadBlockRLE() which uncompresses and compress(RLE) the data\r\n");

    HSTATUS Status = H_ERROR;

    // Alloc buffer for uncompress data.
    HCDPacket UncompressPacket(new Byte[GetResolutionDescriptor()->GetBlockSizeInBytes()], GetResolutionDescriptor()->GetBlockSizeInBytes());
    UncompressPacket.SetBufferOwnership(true);

    if(UncompressPacket.GetBufferAddress() == 0)
        return H_NOT_ENOUGH_MEMORY;

    // Read without compression
    Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, UncompressPacket.GetBufferAddress(), pi_pSisterFileLock);

    if(H_SUCCESS == Status)
        {
        // Adjust to last block so we do not compress more data than what we need.
        uint32_t ActualBlockHeight(GetResolutionDescriptor()->GetBlockHeight());
        if(pi_PosBlockY + GetResolutionDescriptor()->GetBlockHeight() > GetResolutionDescriptor()->GetHeight())
            {
            ActualBlockHeight = (uint32_t)GetResolutionDescriptor()->GetHeight() - (uint32_t)pi_PosBlockY;
            }

        HFCPtr<HCDCodecHMRRLE1> pCodecRLE = new HCDCodecHMRRLE1((uint32_t)GetResolutionDescriptor()->GetBlockWidth(), 1);              // Compress one line at a time.
        size_t                  WorkLineBufferSize = ((uint32_t)GetResolutionDescriptor()->GetBlockWidth() * 2 + 2)*sizeof(unsigned short);   // Worst case for one line.
        HArrayAutoPtr<Byte>   pWorkLineBuffer (new Byte[WorkLineBufferSize]);
        uint32_t                BytesPerBlockWidth = GetResolutionDescriptor()->GetBytesPerBlockWidth();

        for(uint32_t NoLine=0; NoLine < ActualBlockHeight; ++NoLine)
            {
            size_t compressDataSize = pCodecRLE->CompressSubset(UncompressPacket.GetBufferAddress() + (NoLine*BytesPerBlockWidth),
                BytesPerBlockWidth,
                pWorkLineBuffer,
                WorkLineBufferSize);

            // Alloc buffer if it is not large enough.
            if(po_rpPacketRLE->GetLineBufferSize(NoLine) < compressDataSize)
                {
                HASSERT(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
                po_rpPacketRLE->SetLineBuffer(NoLine,
                    new Byte[compressDataSize],
                    compressDataSize, 0/*pi_DataSize*/);
                }

            // Copy from workBuffer to output packet.
            memcpy(po_rpPacketRLE->GetLineBuffer(NoLine), pWorkLineBuffer, compressDataSize);
            po_rpPacketRLE->SetLineDataSize(NoLine, compressDataSize);
            }
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResolutionEditor::WriteBlockRLE(uint64_t             pi_PosBlockX,
                                           uint64_t             pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockX + m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(m_Resolution)->GetBlockWidth() <= ULONG_MAX) &&
                  (pi_PosBlockY + m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(m_Resolution)->GetBlockHeight() <= ULONG_MAX));
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetCodec() != 0);
    HPRECONDITION(pi_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(pi_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight() ||             // Must contains block height,
        pi_PosBlockY + pi_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetHeight());    // except for last block.
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);

    // Alloc buffer for uncompress data.
    HCDPacket UncompressPacket(new Byte[GetResolutionDescriptor()->GetBlockSizeInBytes()], GetResolutionDescriptor()->GetBlockSizeInBytes());
    UncompressPacket.SetBufferOwnership(true);

    if(UncompressPacket.GetBufferAddress() == 0)
        return H_NOT_ENOUGH_MEMORY;

    uint32_t BytesPerWidth = GetResolutionDescriptor()->GetBytesPerBlockWidth();

    HFCPtr<HCDCodecHMRRLE1> pRLECodec = pi_rpPacketRLE->GetCodec();
    pRLECodec->SetSubset(pRLECodec->GetWidth(), 1);    // Subset by line.

    uint32_t LineNo;
    for(LineNo=0; LineNo < pRLECodec->GetHeight(); ++LineNo)
        {
        pRLECodec->DecompressSubset(pi_rpPacketRLE->GetLineBuffer(LineNo), pi_rpPacketRLE->GetLineDataSize(LineNo),  UncompressPacket.GetBufferAddress() + LineNo*BytesPerWidth, BytesPerWidth);
        }

    // Fill last strip with '0' padding. It compress better and avoid mismatch in ATP.
    if (LineNo < GetResolutionDescriptor()->GetBlockHeight())
        {
        memset(UncompressPacket.GetBufferAddress()+(LineNo*BytesPerWidth), 0, BytesPerWidth*(GetResolutionDescriptor()->GetBlockHeight()-LineNo));
        }

    // After a Reset the Codec subset need to be set again because by default it use the whole raster area.
    pRLECodec->SetSubset(pRLECodec->GetWidth(), 1);    // Subset by line.

    // Do standard write block with uncompress packet.
    return WriteBlock(pi_PosBlockX, pi_PosBlockY, UncompressPacket.GetBufferAddress(), pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResolutionEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                        uint64_t              pi_PosBlockY,
                                        const Byte*           pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetCodec() != 0);

    HSTATUS Status = H_SUCCESS;

    // we take for granted that there is compression
    // we compress the data
    HCDPacket Uncompressed((Byte*)pi_pData,
                            m_pResolutionDescriptor->GetBlockSizeInBytes(),
                            m_pResolutionDescriptor->GetBlockSizeInBytes());

    // Assign a default Codec
    HFCPtr<HCDCodec> pCodec = (HCDCodec*)m_pResolutionDescriptor->GetCodec()->Clone();

    HCDCodecImage::SetCodecForImage(pCodec,
                                    m_pResolutionDescriptor->GetBlockWidth(),
                                    m_pResolutionDescriptor->GetBlockHeight(),
                                    m_pResolutionDescriptor->GetBitsPerPixel(),
                                    m_pResolutionDescriptor->GetPaddingBitsPerBlockWidth());

    size_t MaxSubsetCompressed = pCodec->GetSubsetMaxCompressedSize();

    HFCPtr<HCDPacket> pCompressed(new HCDPacket(pCodec, new Byte[MaxSubsetCompressed], MaxSubsetCompressed));

    pCompressed->SetBufferOwnership(true);

    Uncompressed.Compress(pCompressed);

    WriteBlock(pi_PosBlockX, pi_PosBlockY, pCompressed, pi_pSisterFileLock);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFResolutionEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                        uint64_t                 pi_PosBlockY,
                                        const HFCPtr<HCDPacket>& pi_rpPacket,
                                        HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    // we decompress the data
    HCDPacket Uncompressed(new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()],
                                    m_pResolutionDescriptor->GetBlockSizeInBytes());
    Uncompressed.SetBufferOwnership(true);

    pi_rpPacket->Decompress(&Uncompressed);

    WriteBlock(pi_PosBlockX, pi_PosBlockY, Uncompressed.GetBufferAddress(), pi_pSisterFileLock);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// AssignRasterFileLock
//-----------------------------------------------------------------------------
void HRFResolutionEditor::AssignRasterFileLock(const HFCPtr<HRFRasterFile>& pi_rpRasterFile,
                                               HFCLockMonitor&              pio_rSisterFileLock,
                                               bool                         pi_SynchronizedSharingControl)
    {
    // First lock the file.
    pio_rSisterFileLock.Assign(pi_rpRasterFile->GetLockManager());

    // Synch the file if asked and needed.
    if (pi_SynchronizedSharingControl && pi_rpRasterFile->SharingControlNeedSynchronization())
        {
        // Some editors needs to react on synchronization.
        OnSynchronizedSharingControl();

        pi_rpRasterFile->SharingControlSynchronize();
        }
    }


//-----------------------------------------------------------------------------
// public
// ResolutionSizeHasChanged
//
// Notify that the resolution size has changed
//-----------------------------------------------------------------------------
void HRFResolutionEditor::ResolutionSizeHasChanged() const
    {
    (const_cast<HRFResolutionEditor*>(this))->m_pResolutionDescriptor = m_pRasterFile->GetPageDescriptor(m_Page)->GetResolutionDescriptor(m_Resolution);
    }
