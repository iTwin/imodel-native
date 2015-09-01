//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFIntergraphLineEditor.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecCRL8.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <Imagepp/all/h/HCDCodecCCITTFax4.h>

#include <Imagepp/all/h/HGFTileIDDescriptor.h>



const unsigned int JPegTileTrailerSize     =   2;
const unsigned int JPegColorTileHeaderSize =  33;  // 35
const unsigned int JPegGrayTileHeaderSize  =  23;  // 25
const unsigned int JpegColorTablesSize     = 572;  // 574 - JPegTileTrailerSize
const unsigned int JpegGrayTablesSize      = 287;  // 289 - JPegTileTrailerSize

//-----------------------------------------------------------------------------
// public
// HRFIntergraphLineEditor: Construction
//-----------------------------------------------------------------------------

HRFIntergraphLineEditor::HRFIntergraphLineEditor(HFCPtr<HRFRasterFile>                     pi_rpRasterFile,
                                                 uint32_t                                  pi_Page,
                                                 unsigned short                           pi_Resolution,
                                                 HFCAccessMode                             pi_AccessMode,
                                                 HRFIntergraphFile::IntergraphResolutionDescriptor&
                                                 pi_rIntergraphResolutionDescriptor)
    :HRFResolutionEditor( pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode),
    m_IntergraphResolutionDescriptor(pi_rIntergraphResolutionDescriptor),
    m_CompressPacket()
    {
    // All kind of intergraph may need theses initialisations.
    m_PageIndex       = 0;
    m_BitPerPixel     = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBitPerPixel();
    m_pIntergraphFile = const_cast<HFCBinStream*>(static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetIntergraphFilePtr());
    m_CurrentReadLine = 0;

    // Get our internal copy for easier code reading and data manipulation.
    m_WidthInByteToRead = (uint32_t)ceil((float)(m_pResolutionDescriptor->GetWidth()) * ((float)m_BitPerPixel / 8.0));

#ifdef  HRF_DEBUG_DUMPSTATONDISK
    uint32_t w = m_pResolutionDescriptor->GetHeight();

    m_DumpStatArray = new int32_t[w];
    for (int32_t i=0; i<w; i++)
        m_DumpStatArray[i] = 0;
#endif

    // Calculate the correct pixels offfset. This information was always given according the
    // end of the header, not by the begining of the file...
    // NB. The offset is recomputed in the WriteBlock method.
    m_RasterOffset = (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() *
                      HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);

    if (m_IntergraphResolutionDescriptor.pOverviewEntry == 0)
        {
        //TR 109180
        m_ResSizeInBytes = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetFullResolutionSize();
        }
    else
        {
        m_RasterOffset += m_IntergraphResolutionDescriptor.pOverviewEntry->S;
        m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pOverviewEntry->U;
        }

    // HDEBUGCODE( if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0) HASSERT ( m_ResSizeInBytes > 0););

    // if (!m_ResSizeInBytes && m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
    // m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize() * m_pResolutionDescriptor->GetHeight();
    // m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pOverview->NumberPixels * ((float)m_BitPerPixel / 8.0);
    }

//-----------------------------------------------------------------------------
// public
// HRFIntergraphLineEditor: Destruction
//-----------------------------------------------------------------------------

HRFIntergraphLineEditor::~HRFIntergraphLineEditor()
    {

#ifdef  HRF_DEBUG_DUMPSTATONDISK
    DumpStatOnDisk();
    delete []m_DumpStatArray;
#endif

    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
//-----------------------------------------------------------------------------

HSTATUS HRFIntergraphLineEditor::ReadBlock(uint64_t  pi_PosBlockX,
                                           uint64_t  pi_PosBlockY,
                                           Byte*      po_pData,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (po_pData != 0);
    HSTATUS Status = H_ERROR;

    HFCLockMonitor SisterFileLock;
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;   // we are in creation mode
        goto WRAPUP;
        }

    // Lock the sister file if needed
    if (pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (m_IntergraphResolutionDescriptor.pCodec != 0)
        {
        // We have a compressed file
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
                // this is the first line we move to the begin of image data
                m_pIntergraphFile->SeekToPos(m_RasterOffset);

                // Reset the Codec because the user may not read the raster completely and
                // ask again the line zero.
                m_IntergraphResolutionDescriptor.pCodec->Reset();
                m_IntergraphResolutionDescriptor.pCodec->SetDimensions((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                                       (uint32_t)m_pResolutionDescriptor->GetHeight());

                // After a Reset the Codec subset need to be set again because by default it use
                // the whole raster area.
                m_IntergraphResolutionDescriptor.pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                                   1);

                // We set the padding bit if necessary...
                if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
                    m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
                else
                    m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(0);

                if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
                    ((HFCPtr<HCDCodecHMRRLE1> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

                if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecCRL8::CLASS_ID) &&
                    static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_IntergraphHeader.IBlock1.scn == 1)
                    ((HFCPtr<HCDCodecCRL8> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

                // Special case for JPEG compression (Type 30 and 31)
                if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode == 30 || static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode == 31)
                    {
                    unsigned int JPegTileHeaderSize;
                    unsigned int JPegTableSize;
                    unsigned int QualityFactor = 15; // Default value for Intergraph File.

                    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode == 30)
                        JPegTileHeaderSize = JPegGrayTileHeaderSize;
                    else
                        JPegTileHeaderSize = JPegColorTileHeaderSize;

                    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode == 30)
                        JPegTableSize = JpegGrayTablesSize;
                    else
                        JPegTableSize = JpegColorTablesSize;

                    unsigned int TotalJpegHeaderSize  = JPegTileHeaderSize + JPegTileTrailerSize + JPegTableSize;

                    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_pJpegPacketPacket)
                        {
                        QualityFactor = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_pJpegPacketPacket->QualityFactor;
                        }

                    Byte* pCompressedData = new Byte[TotalJpegHeaderSize + m_ResSizeInBytes];

                    // Pre initialize the tile buffer (build header information).
                    InitializeJpegDecompTable(QualityFactor, pCompressedData, m_ResSizeInBytes);

                    // Read the entire compressed pixels in memory
                    if (m_pIntergraphFile->Read((pCompressedData + JPegTableSize + JPegTileHeaderSize), m_ResSizeInBytes) != m_ResSizeInBytes)
                        goto WRAPUP; // H_ERROR

                    m_CompressPacket.SetBuffer(pCompressedData, m_ResSizeInBytes + JPegTableSize + JPegTileHeaderSize);
                    m_CompressPacket.SetBufferOwnership(true);
                    m_CompressPacket.SetDataSize(m_ResSizeInBytes + JPegTableSize + JPegTileHeaderSize);

                    m_CompressPacket.SetCodec((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec));
                    }
                else
                    {
                    // m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pOverviewEntry->U;

                    // Read the entire compressed pixels in memory
                    Byte* pCompressedData = new Byte[m_ResSizeInBytes];

                    if (m_pIntergraphFile->Read(pCompressedData, m_ResSizeInBytes) != m_ResSizeInBytes)
                        goto WRAPUP;    // H_ERROR

                    m_CompressPacket.SetBuffer(pCompressedData, m_ResSizeInBytes);
                    m_CompressPacket.SetBufferOwnership(true);
                    m_CompressPacket.SetDataSize(m_ResSizeInBytes);
                    m_CompressPacket.SetCodec((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec));
                    }
                }

            // We decompress the specified line from the image buffer
            HCDPacket uncompress(po_pData, m_WidthInByteToRead);
            HCDPacket compressSubset((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                                     m_CompressPacket.GetBufferAddress() + m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                                     m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                                     m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex());

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

#ifdef  HRF_DEBUG_DUMPSTATONDISK
        m_DumpStatArray[pi_PosBlockY] = m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex() - LastCompressIndex;
#endif
        }
    else
        {
        // We have a non compressed file

        // repositionate the file cursor to the current line
        // We have random access because the data is uncompressed
        m_pIntergraphFile->SeekToPos(m_RasterOffset + (pi_PosBlockY * m_WidthInByteToRead));

        if (m_pIntergraphFile->Read(po_pData, sizeof(Byte) * m_WidthInByteToRead) != sizeof(Byte) * m_WidthInByteToRead)
            goto WRAPUP;    // H_ERROR

#ifdef  HRF_DEBUG_DUMPSTATONDISK
        m_DumpStatArray[pi_PosBlockY] = m_pResolutionDescriptor->GetWidth();
#endif
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->HasLUTColorCorrection())
        {
        ApplyLUTColorCorrection(po_pData, (uint32_t)m_pResolutionDescriptor->GetWidth());
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
//-----------------------------------------------------------------------------
HSTATUS HRFIntergraphLineEditor::ReadBlockRLE(uint64_t pi_PosBlockX,
                                              uint64_t pi_PosBlockY,
                                              HFCPtr<HCDPacketRLE>& pio_rpPacketRLE,
                                              HFCLockMonitor const* pi_pSisterFileLock)
    {

    // We assume that we have check the header file integrity in the
    // constructor for the release version.
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pio_rpPacketRLE != 0);
    HPRECONDITION(pio_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
    HPRECONDITION(pio_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(pio_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);

    HSTATUS Status = H_ERROR;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // If codec support decompression directly into RLE.
    if (m_IntergraphResolutionDescriptor.pCodec != 0 && m_IntergraphResolutionDescriptor.pCodec->GetRLEInterface())
        {
        HCDCodecRLEInterface* pRLECodecInterface = m_IntergraphResolutionDescriptor.pCodec->GetRLEInterface();

        // We have a compressed file
        // Check if the user need a before line
        if (pi_PosBlockY < m_CurrentReadLine)
            {
            // reset to the begin to allow backward
            m_CompressPacket.SetBuffer(0, 0);
            m_CurrentReadLine = 0;
            }

        if (m_CurrentReadLine == 0)
            {
            // Lock the sister file if needed
            HFCLockMonitor SisterFileLock;
            if (pi_pSisterFileLock == 0)
                {
                // Get lock and synch.
                AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
                pi_pSisterFileLock = &SisterFileLock;
                }

            // this is the first line we move to the begin of image data
            m_pIntergraphFile->SeekToPos(m_RasterOffset);

            // Reset the Codec because the user may not read the raster completely and
            // ask again the line zero.
            m_IntergraphResolutionDescriptor.pCodec->Reset();
            m_IntergraphResolutionDescriptor.pCodec->SetDimensions((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                                   (uint32_t)m_pResolutionDescriptor->GetHeight());

            // After a Reset the Codec subset need to be set again because by default it use
            // the whole raster area.
            m_IntergraphResolutionDescriptor.pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                               1);

            // We set the padding bit if necessary...
            if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
                m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
            else
                m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(0);

            // m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pOverviewEntry->U;

            // Read the entire compressed pixels in memory
            Byte* pCompressedData = new Byte[m_ResSizeInBytes];

            if (m_pIntergraphFile->Read(pCompressedData, m_ResSizeInBytes) != m_ResSizeInBytes)
                goto WRAPUP;    // H_ERROR

            m_CompressPacket.SetBuffer(pCompressedData, m_ResSizeInBytes);
            m_CompressPacket.SetBufferOwnership(true);
            m_CompressPacket.SetDataSize(m_ResSizeInBytes);
            m_CompressPacket.SetCodec((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec));

            // Unlock the sister file
            SisterFileLock.ReleaseKey();
            }

        // Move to the needed line
        for (uint64_t Line = m_CurrentReadLine; Line <= pi_PosBlockY; Line++)
            {
            // Decompress directly in RLE.
            pRLECodecInterface->DecompressSubsetToRLE(m_CompressPacket.GetBufferAddress() + m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(),
                                                      m_ResSizeInBytes - m_IntergraphResolutionDescriptor.pCodec->GetCompressedImageIndex(), pio_rpPacketRLE);
            }

        // We free the buffer if this is the last read line on this image
        if (pi_PosBlockY == m_pResolutionDescriptor->GetHeight() - 1)
            {
            m_CompressPacket.SetBuffer(0, 0);
            m_CurrentReadLine = 0;
            }
        else
            {
            m_CurrentReadLine = pi_PosBlockY + 1;
            }

        Status = H_SUCCESS;
        }
    else
        {
        // Use default implementation.
        Status = HRFResolutionEditor::ReadBlockRLE(pi_PosBlockX, pi_PosBlockY, pio_rpPacketRLE, pi_pSisterFileLock);
        }

WRAPUP:
    return Status;

    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
//-----------------------------------------------------------------------------

HSTATUS HRFIntergraphLineEditor::WriteBlock(uint64_t       pi_PosBlockX,
                                            uint64_t       pi_PosBlockY,
                                            const Byte*    pi_pData,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0));
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pi_pData != 0);

    HSTATUS Status = H_ERROR;
    Byte*  pRasterData;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock without synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Be sure, even in release mode, for accessing an already open file and
    // for a correct access type (Lined vs Tiled)
    // If we write the first line seek to the correct offset in the file.
    if (pi_PosBlockY == 0)
        {
        m_CurrentReadLine = 0;

        //TR 109180
        // We need to recompute the offset, because the previous resolution has possibly changed the
        // pOverviewEntry->S value.
        m_RasterOffset = (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() *
                          HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
        if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
            m_RasterOffset += m_IntergraphResolutionDescriptor.pOverviewEntry->S;


        m_pIntergraphFile->SeekToPos(m_RasterOffset);
        if (m_IntergraphResolutionDescriptor.pCodec != 0)
            {
            // Reset the Codec because the user may not read the raster completely and
            // ask again the line zero.
            m_IntergraphResolutionDescriptor.pCodec->Reset();
            m_IntergraphResolutionDescriptor.pCodec->SetDimensions((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                                   (uint32_t)m_pResolutionDescriptor->GetHeight());

            // After a Reset the Codec subset need to be set again because by default it use
            // the whole raster area.
            m_IntergraphResolutionDescriptor.pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                               1);

            // We set the padding bit if necessary...
            if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
                m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
            else
                m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(0);

            if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
                ((HFCPtr<HCDCodecHMRRLE1> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

            if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecCRL8::CLASS_ID) &&
                static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_IntergraphHeader.IBlock1.scn == 1)
                ((HFCPtr<HCDCodecCRL8> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

            if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
                {
                // Be sure to reset current overeview allocated and used space.
                m_IntergraphResolutionDescriptor.pOverviewEntry->A = 0;
                m_IntergraphResolutionDescriptor.pOverviewEntry->U = 0;
                }
            }
        }

    // Be sure to write the firts line or the next line of the previous write...
    if ((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0))
        {
        // Check if we write a non-compressed images
        if (m_IntergraphResolutionDescriptor.pCodec == 0)
            {
            if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
                m_pIntergraphFile->SeekToPos(m_RasterOffset + (pi_PosBlockY * m_WidthInByteToRead));

            // For uncompress data, simply write it to the file.
            if (m_pIntergraphFile->Write(pi_pData, m_WidthInByteToRead) != (m_WidthInByteToRead))
                goto WRAPUP;    // H_ERROR

            if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
                {
                m_IntergraphResolutionDescriptor.pOverviewEntry->A +=  m_WidthInByteToRead;
                m_IntergraphResolutionDescriptor.pOverviewEntry->U +=  m_WidthInByteToRead;
                }
            }
        else
            {
            // For compress data, compress it into the correct codec,
            // then write it into the file.
            uint32_t CompressedSize = 0;
            HASSERT_X64(m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize() * 3 < ULONG_MAX);
            uint32_t MaxCompressedSize = (uint32_t)m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize() * 3;
            pRasterData = new Byte[MaxCompressedSize];

            // Create a uncompress packet with original uncompress data
            HCDPacket UnCompress((Byte*)pi_pData,m_WidthInByteToRead, m_WidthInByteToRead);

            // Create a compress packet were the data will be compress
            HCDPacket Compress((HFCPtr<class HCDCodec> &)(m_IntergraphResolutionDescriptor.pCodec),
                               pRasterData, MaxCompressedSize);

            // Compress the data and get it's new size.
            UnCompress.Compress(&Compress);
            CompressedSize = (uint32_t)Compress.GetDataSize();

            HASSERT (MaxCompressedSize >= CompressedSize);

            // Write the line need into the file...
            if (CompressedSize)
                {
                if (m_pIntergraphFile->Write(Compress.GetBufferAddress(), CompressedSize) != (CompressedSize))
                    goto WRAPUP;    // H_ERROR

                if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
                    {
                    m_IntergraphResolutionDescriptor.pOverviewEntry->A +=  CompressedSize;
                    m_IntergraphResolutionDescriptor.pOverviewEntry->U +=  CompressedSize;
                    }
                }

            delete []pRasterData;
            }
        // Remember what's the last line write into the file
        m_CurrentReadLine = pi_PosBlockY;

        //TR 109180
        // If the resolution is now bigger, we need to update the offset of the next resolution.
        // (we skip this code in creation mode)
        if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess &&
            (m_CurrentReadLine == m_pResolutionDescriptor->GetHeight() - 1))
            {
            if ((m_RasterOffset+m_ResSizeInBytes) < m_pIntergraphFile->GetCurrentPos())
                {
                static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->UpdateOffsetNextResolutions(m_Resolution,
                                                                                            (uint32_t)(m_pIntergraphFile->GetCurrentPos() - (m_RasterOffset+m_ResSizeInBytes)));
                }

            if (m_IntergraphResolutionDescriptor.pOverviewEntry == 0)
                m_ResSizeInBytes = (uint32_t)(m_pIntergraphFile->GetCurrentPos() - m_RasterOffset);
            else
                m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pOverviewEntry->A;
            }


        // At last line of the the very last resolution, the packet overview information MUST be updated.
        if ((m_IntergraphResolutionDescriptor.pOverviewEntry != 0)          &&
            (m_CurrentReadLine == m_pResolutionDescriptor->GetHeight() - 1) &&
            m_Resolution == static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_SubResolution)
            {
            // Refresh the file header.
            static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->UpdatePacketOverview(m_IntergraphResolutionDescriptor.pOverviewEntry->S, m_Resolution);
            }
        }


    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
//-----------------------------------------------------------------------------
HSTATUS HRFIntergraphLineEditor::WriteBlockRLE(uint64_t               pi_PosBlockX,
                                               uint64_t               pi_PosBlockY,
                                               HFCPtr<HCDPacketRLE>&  pi_rpPacketRLE,
                                               HFCLockMonitor const*  pi_pSisterFileLock)
    {
    HPRECONDITION((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0));
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::LINE);
    HPRECONDITION (pi_rpPacketRLE != 0);
    HPRECONDITION (pi_rpPacketRLE->GetCodec()->GetWidth() == m_pResolutionDescriptor->GetWidth());

    // If output codec does not support compression directly from RLE use default implementation
    if (m_IntergraphResolutionDescriptor.pCodec == 0 || m_IntergraphResolutionDescriptor.pCodec->GetRLEInterface() == 0)
        {
        return HRFResolutionEditor::WriteBlockRLE(pi_PosBlockX, pi_PosBlockY, pi_rpPacketRLE, pi_pSisterFileLock);
        }

    HSTATUS Status = H_ERROR;
    Byte*  pRasterData;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock without synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Be sure, even in release mode, for accessing an already open file and
    // for a correct access type (Lined vs Tiled)
    // If we write the first line seek to the correct offset in the file.
    if (pi_PosBlockY == 0)
        {
        m_CurrentReadLine = 0;

        //TR 109180
        // We need to recompute the offset, because the previous resolution has possibly changed the
        // pOverviewEntry->S value.
        m_RasterOffset = (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() *
                          HRF_INTERGRAGH_HEADER_BLOCK_LENGTH);
        if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
            m_RasterOffset += m_IntergraphResolutionDescriptor.pOverviewEntry->S;


        m_pIntergraphFile->SeekToPos(m_RasterOffset);
        if (m_IntergraphResolutionDescriptor.pCodec != 0)
            {
            // Reset the Codec because the user may not read the raster completely and
            // ask again the line zero.
            m_IntergraphResolutionDescriptor.pCodec->Reset();
            m_IntergraphResolutionDescriptor.pCodec->SetDimensions((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                                   (uint32_t)m_pResolutionDescriptor->GetHeight());

            // After a Reset the Codec subset need to be set again because by default it use
            // the whole raster area.
            m_IntergraphResolutionDescriptor.pCodec->SetSubset((uint32_t)m_pResolutionDescriptor->GetWidth(),
                                                               1);

            // We set the padding bit if necessary...
            if ((m_pResolutionDescriptor->GetWidth() % 8) != 0)
                m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(8 - ((uint32_t)m_pResolutionDescriptor->GetWidth() % 8));
            else
                m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(0);

            if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
                ((HFCPtr<HCDCodecHMRRLE1> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

            if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecCRL8::CLASS_ID) &&
                static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_IntergraphHeader.IBlock1.scn == 1)
                ((HFCPtr<HCDCodecCRL8> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

            if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
                {
                // Be sure to reset current overeview allocated and used space.
                m_IntergraphResolutionDescriptor.pOverviewEntry->A = 0;
                m_IntergraphResolutionDescriptor.pOverviewEntry->U = 0;
                }
            }
        }

    // Be sure to write the firts line or the next line of the previous write...
    if ((pi_PosBlockY == m_CurrentReadLine + 1) || (pi_PosBlockY == 0))
        {
        // For compress data, compress it into the correct codec, then write it into the file.
        uint32_t CompressedSize = 0;
        HASSERT_X64(m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize() * 3 < ULONG_MAX);
        uint32_t MaxCompressedSize = (uint32_t)m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize() * 3;
        pRasterData = new Byte[MaxCompressedSize];

        // Compress the data
        CompressedSize = (uint32_t)m_IntergraphResolutionDescriptor.pCodec->GetRLEInterface()->CompressSubsetFromRLE(pi_rpPacketRLE, pRasterData, MaxCompressedSize);

        HASSERT (MaxCompressedSize >= CompressedSize);

        // Write the line need into the file...
        if (CompressedSize)
            {
            if (m_pIntergraphFile->Write(pRasterData, CompressedSize) != (CompressedSize))
                goto WRAPUP;    // H_ERROR

            if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
                {
                m_IntergraphResolutionDescriptor.pOverviewEntry->A +=  CompressedSize;
                m_IntergraphResolutionDescriptor.pOverviewEntry->U +=  CompressedSize;
                }
            }

        delete []pRasterData;

        // Remember what's the last line write into the file
        m_CurrentReadLine = pi_PosBlockY;

        //TR 109180
        // If the resolution is now bigger, we need to update the offset of the next resolution.
        // (we skip this code in creation mode)
        if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess &&
            (m_CurrentReadLine == m_pResolutionDescriptor->GetHeight() - 1))
            {
            if ((m_RasterOffset+m_ResSizeInBytes) < m_pIntergraphFile->GetCurrentPos())
                {
                static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->UpdateOffsetNextResolutions(m_Resolution,
                                                                                            (uint32_t)(m_pIntergraphFile->GetCurrentPos() - (m_RasterOffset+m_ResSizeInBytes)));
                }

            if (m_IntergraphResolutionDescriptor.pOverviewEntry == 0)
                m_ResSizeInBytes = (uint32_t)(m_pIntergraphFile->GetCurrentPos() - m_RasterOffset);
            else
                m_ResSizeInBytes = m_IntergraphResolutionDescriptor.pOverviewEntry->A;
            }


        // At last line of the the very last resolution, the packet overview information MUST be updated.
        if ((m_IntergraphResolutionDescriptor.pOverviewEntry != 0)          &&
            (m_CurrentReadLine == m_pResolutionDescriptor->GetHeight() - 1) &&
            m_Resolution == static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_SubResolution)
            {
            // Refresh the file header.
            static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->UpdatePacketOverview(m_IntergraphResolutionDescriptor.pOverviewEntry->S, m_Resolution);
            }
        }


    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// DumpStatOnDisk
//-----------------------------------------------------------------------------

#ifdef  HRF_DEBUG_DUMPSTATONDISK

void HRFIntergraphLineEditor::DumpStatOnDisk(void)
    {
    FILE* FilePtr = _wfopen(L"e:\\SebDump.txt", L"wt");

    if (FilePtr)
        {
        for (uint32_t i=0; i< m_pResolutionDescriptor->GetHeight(); i++)
            _ftprintf(FilePtr, L"%d\n", m_DumpStatArray[i]);
        fclose(FilePtr);
        }
    }

#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFIntergraphLineEditor::InitializeJpegDecompTable(double pi_QualityFactor, Byte* po_pTileBuffer, uint32_t pi_DataSize)
    {
    static const Byte ColorTileHeader[JPegColorTileHeaderSize] =
        {   0xff, 0xc0,         // Marker SOF : Base line DCT
        0x00, 0x11,         // Header length= 17 => 15 bytes follow
        0x08,               // 8-bit precision
        0x00, 0x00,         // Number of Line
        0x00, 0x00,         // Pixel per line
        0x03,               // Nbr of Components
        0x01, 0x21, 0x00,   // Component #1, subsampling H,V.(2:1),Table 0
        0x02, 0x11, 0x01,   // Component #2, no subsampling  (1:1), Table 1
        0x03, 0x11, 0x01,   // Component #3, no subsampling  (1:1), Table 1
        0xff, 0xda,         // Marker M_SOS : Start of scan
        0x00, 0x0c,         // Length = 12 => 10 bytes follow.
        0x03,               // Nbr of Component
        0x01, 0x00,
        0x02, 0x11,
        0x03, 0x11,
        0x00, 0x3f,
        0x00
        };             // ?????

    static const Byte GrayTileHeader[JPegGrayTileHeaderSize] =
        {   0xff, 0xc0,          // Marker SOF : Base line DCT
        0x00, 0x0b,          // Header length= 11 => 9 bytes follow
        0x08,                // 8-bit precision
        0x00, 0x00,          // Number of Line
        0x00, 0x00,          // Pixel per line
        0x01,                // # of components
        0x00, 0x11, 0x00,    // Comp #0, no subsampling  (1:1), Table 0
        0xff, 0xda,          // Marker M_SOS
        0x00, 0x08,          // Length = 8 => 6 bytes follow.
        0x01, 0x00, 0x00,    // ?????
        0x00, 0x3f, 0x00
        };  // ?????

    static const Byte ColorTablesHeader[] =
        {   0xff, 0xd8,     // Marker SOI : Start of image
        0xff, 0xdb,     // Marker DQT : Define Quantitization Table
        0x00, 0x43,     // Length = 67 => 65 bytes follow. This may be 0x84 (132) for Color
        0x00,           // 8-bit Precision, Table #0,
        0x10, 0x0b, 0x0c, 0x0e, 0x0c, 0x0a, 0x10, 0x0e,
        0x0d, 0x0e, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28,
        0x1a, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25,
        0x1d, 0x28, 0x3a, 0x33, 0x3d, 0x3c, 0x39, 0x33,
        0x38, 0x37, 0x40, 0x48, 0x5c, 0x4e, 0x40, 0x44,
        0x57, 0x45, 0x37, 0x38, 0x50, 0x6d, 0x51, 0x57,
        0x5f, 0x62, 0x67, 0x68, 0x67, 0x3e, 0x4d, 0x71,
        0x79, 0x70, 0x64, 0x78, 0x5c, 0x65, 0x67, 0x63,

        0xff, 0xdb,     // Marker DQT : Define Quantitization Table
        0x00, 0x43,     // Length = 67 => 65 bytes follow. This may be 0x84 (132) for Color
        0x01,           // 8-bit Precision, Table #1,
        0x11, 0x12, 0x12, 0x18, 0x15, 0x18, 0x2f, 0x1a,
        0x1a, 0x2f, 0x63, 0x42, 0x38, 0x42, 0x63, 0x63,
        0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
        0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
        0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
        0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
        0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,
        0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63,

        0xff, 0xc4,     // Marker DHT : Define Huffman table - Luminace
        0x00, 0x1f,     // Length = 31 => 29 bytes follow
        0x00,           // DC Table, Table 0  16 bytes of "BITS", 12 bytes of "HUFVAL"
        0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

        0xff, 0xc4,     // Marker DHT : Define Huffman table - Luminace
        0x00, 0xb5,     // Length = 181 => 179 bytes follow
        0x10,           // AC Luminace Table, #1 16 bytes of "BITS" and 162 bytes of "HUFVAL"
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,

        0xff, 0xc4,      // Marker DHT : Define Huffman table - Chrominance
        0x00, 0x1F,      // Length = 31 => 29 bytes follow
        0x01,            // DC Table, Table 1  16 bytes of "BITS", 12 bytes of "HUFVAL"

        //  Intergraph Working copy
        0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

        0xff, 0xc4,      // Marker DHT : Define Huffman table - Chrominance
        0x00, 0xb5,      // Length = 181 => 179 bytes follow
        0x11,            // AC Chrominance Table, #1 16 bytes of "BITS" and 162 bytes of "HUFVAL"
        0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
        0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
        0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
        0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
        0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
        0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
        0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,
        0xff, 0xd9
        };    // Marker EOI : End of image // */

    static const Byte GrayTablesHeader[] =            // SOI
        {   0xff, 0xd8,     // DQT  Offset : 2
        0xff, 0xdb,     // Offset : 4
        0x00, 0x43,     // Length = 67 => 65 bytes follow. This may be 0x84 (132) for Color
        0x00,           // 8-bit Precision, Table #0,  Offset : 7
        0x10, 0x0b, 0x0c, 0x0e, 0x0c, 0x0a, 0x10, 0x0e,
        0x0d, 0x0e, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28,
        0x1a, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25,
        0x1d, 0x28, 0x3a, 0x33, 0x3d, 0x3c, 0x39, 0x33,
        0x38, 0x37, 0x40, 0x48, 0x5c, 0x4e, 0x40, 0x44,
        0x57, 0x45, 0x37, 0x38, 0x50, 0x6d, 0x51, 0x57,
        0x5f, 0x62, 0x67, 0x68, 0x67, 0x3e, 0x4d, 0x71,
        0x79, 0x70, 0x64, 0x78, 0x5c, 0x65, 0x67, 0x63, // Offset : 71

        0xff, 0xc4,      // DHT
        0x00, 0x1f,      // Length = 31 => 29 bytes follow
        0x00,            // DC Table
        0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,

        0xff, 0xc4,      // DHT
        0x00, 0xb5,      // Length = 181 => 179 bytes follow*/
        0x10,
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
        0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
        0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
        0xf9, 0xfa,
        0xff, 0xd9
        };     // EOI

    // Ugly patch which corrected over color saturation (too much contrast)
    // sacrificing some image quality...
    pi_QualityFactor = 9;

    // If Grayscale...
    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode == 30)
        {
        memcpy(po_pTileBuffer, GrayTablesHeader, JpegGrayTablesSize);
        memcpy(po_pTileBuffer + JpegGrayTablesSize, GrayTileHeader, JPegGrayTileHeaderSize);

        // Initialise Tile height within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        Byte SizeHI  = (Byte)(m_pResolutionDescriptor->GetHeight() >> 8);
        Byte SizeLOW = (Byte)m_pResolutionDescriptor->GetHeight();
        memcpy(po_pTileBuffer + JpegGrayTablesSize + 5, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegGrayTablesSize + 6, &SizeLOW, 1);

        // Initialise Tile width within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        SizeHI  = (Byte)(m_pResolutionDescriptor->GetWidth() >> 8);
        SizeLOW = (Byte)m_pResolutionDescriptor->GetWidth();
        memcpy(po_pTileBuffer + JpegGrayTablesSize + 7, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegGrayTablesSize + 8, &SizeLOW, 1);

        // Set Jpeg end trailer after the compressed data.
        po_pTileBuffer[pi_DataSize + JPegGrayTileHeaderSize + JpegGrayTablesSize + 0] = 0xff;
        po_pTileBuffer[pi_DataSize + JPegGrayTileHeaderSize + JpegGrayTablesSize + 1] = 0xd9;

        // Build luminance table directlty into the jpeg header giving the right pointer
        // location.  Note, there is no Chroma table within a gray scale image.
        BuildJpegLumiChromaTable(pi_QualityFactor, po_pTileBuffer + 7, 0);
        }
    else
        {
        memcpy(po_pTileBuffer, ColorTablesHeader, JpegColorTablesSize);
        memcpy(po_pTileBuffer + JpegColorTablesSize, ColorTileHeader, JPegColorTileHeaderSize);

        // Initialise Tile height within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        Byte SizeHI  = (Byte)(m_pResolutionDescriptor->GetHeight() >> 8);
        Byte SizeLOW = (Byte)m_pResolutionDescriptor->GetHeight();
        memcpy(po_pTileBuffer + JpegColorTablesSize + 5, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegColorTablesSize + 6, &SizeLOW, 1);

        // Initialise Tile width within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        SizeHI  = (Byte)(m_pResolutionDescriptor->GetWidth() >> 8);
        SizeLOW = (Byte)m_pResolutionDescriptor->GetWidth();
        memcpy(po_pTileBuffer + JpegColorTablesSize + 7, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegColorTablesSize + 8, &SizeLOW, 1);

        // Set Jpeg end trailer after the compressed data.
        po_pTileBuffer[pi_DataSize + JPegColorTileHeaderSize + JpegColorTablesSize + 0] = 0xff;
        po_pTileBuffer[pi_DataSize + JPegColorTileHeaderSize + JpegColorTablesSize + 1] = 0xd9;

        // Build chromance and luminance table directlty into the jpeg header
        // giving for each the right pointer location.
        BuildJpegLumiChromaTable(pi_QualityFactor, po_pTileBuffer + 7, po_pTileBuffer + 76);
        }
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFIntergraphLineEditor::BuildJpegLumiChromaTable(double pi_QualityFactor, Byte* po_pLuminance, Byte* po_pChroma)
    {
    // Give at least one valid outup data pointer.
    HASSERT( po_pLuminance || po_pChroma);

    const unsigned int TableWidth  = 8;
    const unsigned int TableHeight = 8;

    uint32_t Line;
    uint32_t Column;
    uint32_t LumiIndex;
    uint32_t ChromaIndex;

    uint32_t TmpLuminance [64] = {0};
    uint32_t TmpChrominace[64] = {0};

    uint32_t Luminance [64] = { 16,  11,  10,  16,  24,  40,  51,  61,
                             12,  12,  14,  19,  26,  58,  60,  55,
                             14,  13,  16,  24,  40,  57,  69,  56,
                             14,  17,  22,  29,  51,  87,  80,  62,
                             18,  22,  37,  56,  68, 109, 103,  77,
                             24,  35,  55,  64,  81, 104, 113,  92,
                             49,  64,  78,  87, 103, 121, 120, 101,
                             72,  92,  95,  98, 112, 100, 103,  99
                           };

    uint32_t Chrominace[64] = { 17,  18,  24,  47,  99,  99,  99,  99,
                             18,  21,  26,  66,  99,  99,  99,  99,
                             24,  26,  56,  99,  99,  99,  99,  99,
                             47,  66,  99,  99,  99,  99,  99,  99,
                             99,  99,  99,  99,  99,  99,  99,  99,
                             99,  99,  99,  99,  99,  99,  99,  99,
                             99,  99,  99,  99,  99,  99,  99,  99,
                             99,  99,  99,  99,  99,  99,  99,  99
                           };

    uint32_t NaturalOrder[64] = {  0,   1,   8,  16,   9,   2,   3,  10,
                                17,  24,  32,  25,  18,  11,   4,   5,
                                12,  19,  26,  33,  40,  48,  41,  34,
                                27,  20,  13,   6,   7,  14,  21,  28,
                                35,  42,  49,  56,  57,  50,  43,  36,
                                29,  22,  15,  23,  30,  37,  44,  51,
                                58,  59,  52,  45,  38,  31,  39,  46,
                                53,  60,  61,  54,  47,  55,  62,  63
                             };

    // Scale the Lumi/Chroma table to the 8Bits scale.
    for (Line = 0; Line < TableHeight; Line++)
        {
        for (Column = 0; Column < TableWidth; Column++)
            {
            LumiIndex   = (Line   * 8) + Column;
            ChromaIndex = (Column * 8) + Line;

            TmpLuminance [ChromaIndex] = (uint32_t)(Luminance [LumiIndex] * pi_QualityFactor /  50L);
            TmpChrominace[ChromaIndex] = (uint32_t)(Chrominace[LumiIndex] * pi_QualityFactor /  50L);

            // Trim minimum/maximum value.
            if (TmpLuminance[ChromaIndex] < 2)
                TmpLuminance[ChromaIndex] = 2;
            else if (TmpLuminance[ChromaIndex] > 254)
                TmpLuminance[ChromaIndex] = 254;

            // Trim minimum/maximum value.
            if (TmpChrominace[ChromaIndex] < 2)
                TmpChrominace[ChromaIndex] = 2;
            else if (TmpChrominace[ChromaIndex] > 254)
                TmpChrominace[ChromaIndex] = 254;
            }
        }

    // Transpose tables into the 'natural' order (Zigzag it!!)
    for (Line = 0; Line < TableHeight; Line++)
        {
        for (Column = 0; Column < TableWidth; Column++)
            {
            LumiIndex   = (Line * 8) + Column;

            HASSERT( LumiIndex < TableHeight * TableWidth);

            if (po_pLuminance)
                po_pLuminance[LumiIndex] = (unsigned char)TmpLuminance [NaturalOrder[LumiIndex]];

            if (po_pChroma)
                po_pChroma[LumiIndex]    = (unsigned char)TmpChrominace[NaturalOrder[LumiIndex]];
            }
        }
    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResolutionDescriptor.pCodec)->SetOptimizeCoding(true);
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFIntergraphLineEditor::OnSynchronizedSharingControl()
    {
    if (m_IntergraphResolutionDescriptor.pCodec != 0)
        {
        // We have a compressed file. We must reread all the file.
        m_CompressPacket.SetBuffer(0, 0);
        m_CurrentReadLine = 0;
        }
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------

void HRFIntergraphLineEditor::ApplyLUTColorCorrection(Byte* pio_pData, uint32_t pi_pixelCount)
    {
    HPRECONDITION(static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->HasLUTColorCorrection());
    HPRECONDITION(m_BitPerPixel == 24 || m_BitPerPixel == 8);

    // Be sure we have a Value pixel type, on a palette pixel type, the correction
    // has already been applied.
    const Byte* pRedLUT   = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetRedLUTColorTablePtr  ();
    const Byte* pGreenLUT = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetGreenLUTColorTablePtr();
    const Byte* pBlueLUT  = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlueLUTColorTablePtr ();

    uint32_t ByteIndex = 0;

    // For performance purpose, duplicate the Pixel loop instead of using
    if (m_BitPerPixel == 24)
        {
        HASSERT(pRedLUT   != 0);
        HASSERT(pGreenLUT != 0);
        HASSERT(pBlueLUT  != 0);

        for (uint32_t PixelIndex = 0; PixelIndex < pi_pixelCount; PixelIndex++)
            {
            pio_pData[ByteIndex] = pRedLUT  [pio_pData[ByteIndex++]];
            pio_pData[ByteIndex] = pGreenLUT[pio_pData[ByteIndex++]];
            pio_pData[ByteIndex] = pBlueLUT [pio_pData[ByteIndex++]];
            }
        }
    else if (m_BitPerPixel == 8)
        {
        HASSERT(pRedLUT   != 0);

        for (uint32_t PixelIndex = 0; PixelIndex < pi_pixelCount; PixelIndex++)
            {
            pio_pData[ByteIndex] = pRedLUT  [pio_pData[ByteIndex++]];
            }
        }
    // Should not occur.  This situation mean we have an Intergraph file with an LUT
    // not handled properly.  In release the LUT will be simply ignored.
    HDEBUGCODE(else HASSERT(false);)
        }
