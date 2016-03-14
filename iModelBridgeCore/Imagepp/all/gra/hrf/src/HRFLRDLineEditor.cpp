//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFLRDLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFLRDLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <Imagepp/all/h/HRFLRDLineEditor.h>
#include <Imagepp/all/h/HRFLRDFile.h>
#include <Imagepp/all/h/HCDPacket.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------

HRFLRDLineEditor::HRFLRDLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t             pi_Page,
                                   uint16_t      pi_Resolution,
                                   HFCAccessMode        pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode),
      m_CompressPacket()
    {
        m_pLRDFile = const_cast<HFCBinStream*>(static_cast<HRFLRDFile*>(GetRasterFile().GetPtr())->GetLRDFilePtr());
    m_pCodec    = static_cast<HRFLRDFile*>(GetRasterFile().GetPtr())->GetLRDCodecPtr();

    // Reset the Codec because the user may not read the raster completely and
    // ask again the line zero.
    m_pCodec->Reset();

    m_CurrentReadLine = 0;

    m_BitPerPixel       = 1;
    m_WidthInByteToRead = (uint32_t)ceil((float)(m_pResolutionDescriptor->GetWidth()) * ((float)m_BitPerPixel / 8.0));

    m_RasterOffset    = 512;

    m_ResSizeInBytes  = (uint32_t)(m_pLRDFile->GetSize() - m_RasterOffset);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------

HRFLRDLineEditor::~HRFLRDLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by block
//-----------------------------------------------------------------------------

HSTATUS HRFLRDLineEditor::ReadBlock(uint64_t              pi_PosBlockX,
                                    uint64_t              pi_PosBlockY,
                                    Byte*                 po_pData)
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

            // this is the first line we move to the begin of image data
            m_pLRDFile->SeekToPos(m_RasterOffset);

            // Read the entire compressed pixels in memory
            Byte* pCompressedData = new Byte[m_ResSizeInBytes];

            if(m_pLRDFile->Read(pCompressedData, m_ResSizeInBytes) != m_ResSizeInBytes)
                goto WRAPUP;

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

        if (compressSubset.Decompress(&uncompress) == 0)
            return H_DATA_CORRUPTED;
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

HSTATUS HRFLRDLineEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                     uint64_t              pi_PosBlockY,
                                     const Byte*           pi_pData)
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
        RasterOffset = HRF_LRD_BLOCK_SIZE;
        m_CurrentReadLine = 0;

        m_pLRDFile->SeekToPos(RasterOffset);

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

    // Be sure to write the first line or the next line of the previous write...
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
            if (m_pLRDFile->Write(Compress.GetBufferAddress(), sizeof(Byte) * CompressedSize) != (sizeof(Byte) * CompressedSize))
                goto WRAPUP;    // H_ERROR

            if (pi_PosBlockY == m_pResolutionDescriptor->GetHeight() - 1)
                {
                uint16_t EndOfRasterMarker = 0x8000;
                if (m_pLRDFile->Write(&EndOfRasterMarker, sizeof(uint16_t)) != sizeof(uint16_t))
                    goto WRAPUP;
                }
            }

        // Remember what's the last line write into the file
        m_CurrentReadLine = pi_PosBlockY;
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }
