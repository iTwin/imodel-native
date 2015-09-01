//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCalsLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFCalsLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFCalsLineEditor.h>
#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------

HRFCalsLineEditor::HRFCalsLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode),
      m_CompressPacket()
    {
    m_pCalsFile = const_cast<HFCBinStream*>(static_cast<HRFCalsFile*>(GetRasterFile().GetPtr())->GetCalsFilePtr());
    m_pCodec    = static_cast<HRFCalsFile*>(GetRasterFile().GetPtr())->GetCalsCodecPtr();

    // Reset the Codec because the user may not read the raster completely and
    // ask again the line zero.
    m_pCodec->Reset();

    m_CurrentReadLine = 0;

    m_BitPerPixel       = 1;
    m_WidthInByteToRead = (uint32_t)ceil((float)(m_pResolutionDescriptor->GetWidth()) * ((float)m_BitPerPixel / 8.0));

    m_RasterOffset    = HRF_CALS_TYPE1_BLOCK_SIZE;

    // Lock the sister file before accessing the physical file..
    HFCLockMonitor SisterFileLock(GetRasterFile()->GetLockManager());

    m_ResSizeInBytes  = (uint32_t)(m_pCalsFile->GetSize() - m_RasterOffset);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------

HRFCalsLineEditor::~HRFCalsLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by block
//-----------------------------------------------------------------------------

HSTATUS HRFCalsLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                     uint64_t pi_PosBlockY,
                                     Byte*   po_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pCodec != 0);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Check if the user need a before line
    if (pi_PosBlockY < m_CurrentReadLine)
        {
        // reset to the begin to allow backward
        m_CompressPacket.SetBuffer(0, 0);
        m_CurrentReadLine = 0;
        }

    // Move to the needed line
    for (uint64_t Line = m_CurrentReadLine; Line <= pi_PosBlockY; Line++)
        {
        if (Line == 0)
            {
            // Reset the Codec because the user may not read the raster completely and
            // ask again the line zero.
            m_pCodec->Reset();

            // After a Reset the Codec subset need to be set again because by default it use
            // the whole raster area.
            m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), 1);

            // Set the padding bit if necessary...
            if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
                m_pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
            else
                m_pCodec->SetLinePaddingBits(0);

            // Lock the sister file
            HFCLockMonitor SisterFileLock;
            if(pi_pSisterFileLock == 0)
                {
                // Lock the file.
                AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
                pi_pSisterFileLock = &SisterFileLock;
                }

            // this is the first line we move to the begin of image data
            m_pCalsFile->SeekToPos(m_RasterOffset);

            // Read the entire compressed pixels in memory
            Byte* pCompressedData = new Byte[m_ResSizeInBytes];

            if(m_pCalsFile->Read(pCompressedData, m_ResSizeInBytes) != m_ResSizeInBytes)
                goto WRAPUP;

            // Unlock the sister file
            SisterFileLock.ReleaseKey();

            m_CompressPacket.SetBuffer(pCompressedData, m_ResSizeInBytes);
            m_CompressPacket.SetBufferOwnership(true);
            m_CompressPacket.SetDataSize(m_ResSizeInBytes);
            m_CompressPacket.SetCodec((HFCPtr<class HCDCodec>)(m_pCodec));
            }

        // We decompress the specified line from the image buffer
        HCDPacket uncompress(po_pData, m_WidthInByteToRead);
        HCDPacket compressSubset((HFCPtr<class HCDCodec>)(m_pCodec),
                                 m_CompressPacket.GetBufferAddress() + m_pCodec->GetCompressedImageIndex(),
                                 m_ResSizeInBytes - m_pCodec->GetCompressedImageIndex(),
                                 m_ResSizeInBytes - m_pCodec->GetCompressedImageIndex());

        compressSubset.Decompress(&uncompress);
        }

    // We free the buffer if this is the last read line on this image
    if (pi_PosBlockY == m_pResolutionDescriptor->GetHeight() - 1)
        {
        m_CompressPacket.SetBuffer(0, 0);
        m_CurrentReadLine = 0;
        }
    else
        m_CurrentReadLine = pi_PosBlockY + 1;

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by block
//-----------------------------------------------------------------------------
HSTATUS HRFCalsLineEditor::ReadBlockRLE(uint64_t              pi_PosBlockX,
                                        uint64_t              pi_PosBlockY,
                                        HFCPtr<HCDPacketRLE>& pio_rpPacketRLE,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pio_rpPacketRLE != 0);
    HPRECONDITION (m_pCodec != 0);
    HPRECONDITION(pio_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
    HPRECONDITION(pio_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(pio_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);
    HPRECONDITION(m_pCodec->GetRLEInterface() != 0);    // CCITTFax4 should have RLEInterface

    if(m_pCodec->GetRLEInterface() == 0)
        return HRFResolutionEditor::ReadBlockRLE(pi_PosBlockX, pi_PosBlockY, pio_rpPacketRLE, pi_pSisterFileLock);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Check if the user need a before line
    if (pi_PosBlockY < m_CurrentReadLine)
        {
        // reset to the begin to allow backward
        m_CompressPacket.SetBuffer(0, 0);
        m_CurrentReadLine = 0;
        }

    if (m_CurrentReadLine == 0)
        {
        // Reset the Codec because the user may not read the raster completely and
        // ask again the line zero.
        m_pCodec->Reset();

        // After a Reset the Codec subset need to be set again because by default it use
        // the whole raster area.
        m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), 1);

        // Set the padding bit if necessary...
        if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
            m_pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
        else
            m_pCodec->SetLinePaddingBits(0);

        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // this is the first line we move to the begin of image data
        m_pCalsFile->SeekToPos(m_RasterOffset);

        // Read the entire compressed pixels in memory
        Byte* pCompressedData = new Byte[m_ResSizeInBytes];

        if(m_pCalsFile->Read(pCompressedData, m_ResSizeInBytes) != m_ResSizeInBytes)
            goto WRAPUP;    // H_ERROR;

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        m_CompressPacket.SetBuffer(pCompressedData, m_ResSizeInBytes);
        m_CompressPacket.SetBufferOwnership(true);
        m_CompressPacket.SetDataSize(m_ResSizeInBytes);
        m_CompressPacket.SetCodec((HFCPtr<class HCDCodec>)(m_pCodec));
        }

    // Move to the needed line
    for (uint64_t Line = m_CurrentReadLine; Line <= pi_PosBlockY; Line++)
        {
        // Decompress directly in RLE.
        m_pCodec->GetRLEInterface()->DecompressSubsetToRLE(m_CompressPacket.GetBufferAddress() + m_pCodec->GetCompressedImageIndex(),
                                                           m_ResSizeInBytes - m_pCodec->GetCompressedImageIndex(),
                                                           pio_rpPacketRLE);
        }

    // We free the buffer if this is the last read line on this image
    if (pi_PosBlockY == m_pResolutionDescriptor->GetHeight() - 1)
        {
        m_CompressPacket.SetBuffer(0, 0);
        m_CurrentReadLine = 0;
        }
    else
        m_CurrentReadLine = pi_PosBlockY + 1;

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by block
//-----------------------------------------------------------------------------

HSTATUS HRFCalsLineEditor::WriteBlock(uint64_t                pi_PosBlockX,
                                      uint64_t                pi_PosBlockY,
                                      const Byte*             pi_pData,
                                      HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0));
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pi_pData != 0);

    HSTATUS Status = H_ERROR;
    uint32_t RasterOffset;
    HArrayAutoPtr<Byte>  pRasterData;


    // Be sure, even in release mode, for accessing an already open file and
    // for a correct access type (Lined vs Tiled)
    // If we read the first line seek to the correct offset in the file.
    if (pi_PosBlockY == 0)
        {
        RasterOffset = HRF_CALS_TYPE1_BLOCK_SIZE;
        m_CurrentReadLine = 0;

        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
            pi_pSisterFileLock = &SisterFileLock;
            }

        m_pCalsFile->SeekToPos(RasterOffset);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        // Reset all codec value to ensure correct writing startup.
        m_pCodec->Reset();

        // After a Reset the Codec subset need to be set again because by default it use
        // the whole raster area.
        m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), 1);

        // Set the padding bit if necessary...
        if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
            m_pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
        else
            m_pCodec->SetLinePaddingBits(0);
        }

    // Be sure to write the firts line or the next line of the previous write...
    if ((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0))
        {
        // For compress data, compress it into the correct codec,
        // then write it into the file.
        size_t  CompressedSize = 0;
        size_t  MaxCompressedSize = m_pCodec->GetSubsetMaxCompressedSize();
        pRasterData = new Byte[MaxCompressedSize];

        // Create a uncompress packet with original uncompress data
        HCDPacket UnCompress((Byte*)pi_pData,m_WidthInByteToRead, m_WidthInByteToRead);

        // Create a compress packet were the data will be compress
        HCDPacket Compress((HFCPtr<class HCDCodec>)(m_pCodec),
                           pRasterData,
                           MaxCompressedSize);

        // Compress the data and get it's new size.
        UnCompress.Compress(&Compress);
        CompressedSize = Compress.GetDataSize();
        // Write the line need into the file...
        if (CompressedSize)
            {
            // Lock the sister file
            HFCLockMonitor SisterFileLock;
            if(pi_pSisterFileLock == 0)
                {
                // Lock the file.
                AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
                pi_pSisterFileLock = &SisterFileLock;
                }

            if (m_pCalsFile->Write(Compress.GetBufferAddress(), sizeof(Byte) * CompressedSize) != (sizeof(Byte) * CompressedSize))
                goto WRAPUP;    // H_ERROR

            // Increment the counter of the sister file.
            GetRasterFile()->SharingControlIncrementCount();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        // Remember what's the last line write into the file
        m_CurrentReadLine = pi_PosBlockY;
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
// Edition by block
//-----------------------------------------------------------------------------
HSTATUS HRFCalsLineEditor::WriteBlockRLE(uint64_t                pi_PosBlockX,
                                         uint64_t                pi_PosBlockY,
                                         HFCPtr<HCDPacketRLE>&   pi_rpPacketRLE,
                                         HFCLockMonitor const*   pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0));
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (pi_rpPacketRLE != 0);
    HPRECONDITION(m_pCodec->GetRLEInterface() != 0);    // CCITTFax4 should have RLEInterface

    if(m_pCodec->GetRLEInterface() == 0)
        return HRFResolutionEditor::WriteBlockRLE(pi_PosBlockX, pi_PosBlockY, pi_rpPacketRLE, pi_pSisterFileLock);

    HSTATUS Status = H_ERROR;

    // Be sure, even in release mode, for accessing an already open file and
    // for a correct access type (Lined vs Tiled)
    // If we read the first line seek to the correct offset in the file.
    if (pi_PosBlockY == 0)
        {
        m_CurrentReadLine = 0;

        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
            pi_pSisterFileLock = &SisterFileLock;
            }

        m_pCalsFile->SeekToPos(HRF_CALS_TYPE1_BLOCK_SIZE);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        // Reset all codec value to ensure correct writing startup.
        m_pCodec->Reset();

        // After a Reset the Codec subset need to be set again because by default it use
        // the whole raster area.
        m_pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(), 1);

        // Set the padding bit if necessary...
        if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
            m_pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
        else
            m_pCodec->SetLinePaddingBits(0);
        }

    // Be sure to write the firts line or the next line of the previous write...
    if ((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0))
        {
        // For compress data, compress it into the correct codec,
        // then write it into the file.
        size_t  CompressedSize = 0;
        size_t  MaxCompressedSize = m_pCodec->GetSubsetMaxCompressedSize();
        HArrayAutoPtr<Byte>  pRasterData(new Byte[MaxCompressedSize]);

        // Compress the data
        CompressedSize = (uint32_t)m_pCodec->GetRLEInterface()->CompressSubsetFromRLE(pi_rpPacketRLE, pRasterData, MaxCompressedSize);

        // Write the line need into the file...
        if (CompressedSize)
            {
            // Lock the sister file
            HFCLockMonitor SisterFileLock;
            if(pi_pSisterFileLock == 0)
                {
                // Lock the file.
                AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
                pi_pSisterFileLock = &SisterFileLock;
                }

            if (m_pCalsFile->Write(pRasterData, CompressedSize) != CompressedSize)
                goto WRAPUP;    // H_ERROR

            // Increment the counter of the sister file.
            GetRasterFile()->SharingControlIncrementCount();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        // Remember what's the last line write into the file
        m_CurrentReadLine = pi_PosBlockY;
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }