//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIntergraphTileEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIntergraphTileEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFIntergraphTileEditor.h>
#include <Imagepp/all/h/HRFIntergraphFile.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecCRL8.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HGFTileIDDescriptor.h>
#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HFCMath.h>

const unsigned int JPegTileTrailerSize     =   2;
const unsigned int JPegColorTileHeaderSize =  33;
const unsigned int JPegGrayTileHeaderSize  =  23;

const unsigned int JpegColorTablesSize     = 572;
const unsigned int JpegGrayTablesSize      = 287;


//-----------------------------------------------------------------------------
// public
// HRFIntergraphTileEditor: Construction
//-----------------------------------------------------------------------------

HRFIntergraphTileEditor::HRFIntergraphTileEditor(
    HFCPtr<HRFRasterFile>                     pi_rpRasterFile,
    uint32_t                                  pi_Page,
    unsigned short                           pi_Resolution,
    HFCAccessMode                             pi_AccessMode,
    HRFIntergraphFile::IntergraphResolutionDescriptor&
    pi_rIntergraphResolutionDescriptor,
    HRFIntergraphFile::ListOfFreeBlock&       pio_rListOfFreeBlock)
    : HRFResolutionEditor( pi_rpRasterFile,
                           pi_Page,
                           pi_Resolution,
                           pi_AccessMode),
    m_ListOfFreeBlock(pio_rListOfFreeBlock),
    m_IntergraphResolutionDescriptor(pi_rIntergraphResolutionDescriptor)

    {
    // All kind of intergraph may need theses initialisations.

    m_PageIndex       = 0;
    m_BitPerPixel     = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBitPerPixel();
    m_pIntergraphFile = const_cast<HFCBinStream*>(static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetIntergraphFilePtr());

    // Get our internal copy for easier code reading and data manipulation.
    // Raster specilized initilisation...
    // For tile stored raster

    // Needed in read and write access mode
    // Precompute some operation for optimisation and code readability.
    m_TileWidthInByte = (uint32_t)ceil((float)(m_pResolutionDescriptor->GetBlockWidth()) * ((float)m_BitPerPixel / 8.0));
    m_TileHeight      = m_pResolutionDescriptor->GetBlockHeight();
    m_TileSizeInByte  = m_TileWidthInByte * m_TileHeight;

    m_pTileIdDescriptor = new HGFTileIDDescriptor(m_pResolutionDescriptor->GetWidth(),
                                                  m_pResolutionDescriptor->GetHeight(),
                                                  m_pResolutionDescriptor->GetBlockWidth(),
                                                  m_pResolutionDescriptor->GetBlockHeight());

    // Verrify Tile structure integrity
    HASSERT(VerrifyBlockOverlap());

    // Required only in write access mode
    if (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess)
        {
        // Allocate memory for an unused Compraison Buffer
        m_pCompBuffer = new Byte[m_TileWidthInByte];

        // Map the stream to find empty block
        //TR 109180, Remove the scan for the free block
        //      MapStreamHole();
        }
    }

//-----------------------------------------------------------------------------
// public
// HRFIntergraphTileEditor: Destruction
//-----------------------------------------------------------------------------

HRFIntergraphTileEditor::~HRFIntergraphTileEditor()
    {
    delete m_pTileIdDescriptor;
    // Required only in write access mode
    if (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess)
        {
        delete []m_pCompBuffer;
        }
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock: Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFIntergraphTileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                           uint64_t pi_PosBlockY,
                                           Byte* po_pData,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION(pi_PosBlockX <= m_pResolutionDescriptor->GetWidth());
    HPRECONDITION(pi_PosBlockY <= m_pResolutionDescriptor->GetHeight());
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    // For performance reason and code clarity, we initialize Status to success
    // because we dont have many event who will invalidate it.
    HSTATUS Status = H_ERROR;

    uint32_t FileOffset;
    uint64_t TileIndex;
    uint32_t BufferIndex;
    uint32_t i;
    uint32_t BufferSize;

    uint32_t RealTileWidthPixel  = 0;
    uint32_t RealTileHeightPixel = 0;

    HRFIntergraphFile::TileEntry* TileEntryInformation;

    HFCLockMonitor SisterFileLock;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Lock the sister file if needed
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Be sure to have the right to be there !
    // Retrieve the right tile entry info
    TileIndex = m_pTileIdDescriptor->ComputeIndex(pi_PosBlockX, pi_PosBlockY);
    // If we have a subimage, take the correct TileEntry...
    TileEntryInformation = &(m_IntergraphResolutionDescriptor.pTileDirectoryEntry[TileIndex]);

    if (m_IntergraphResolutionDescriptor.pCodec != 0 &&
        (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode != 30 && static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode != 31))
        {
        // Theses values must be recalculate each time we have a tile read and we have a compress
        // raster file.
        if (((uint32_t)pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth()) > m_pResolutionDescriptor->GetWidth())
            RealTileWidthPixel = (uint32_t)m_pResolutionDescriptor->GetWidth() - (uint32_t)pi_PosBlockX;
        else
            RealTileWidthPixel = m_pResolutionDescriptor->GetBlockWidth();

        if (((uint32_t)pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight()) > m_pResolutionDescriptor->GetHeight())
            RealTileHeightPixel = (uint32_t)m_pResolutionDescriptor->GetHeight() - (uint32_t)pi_PosBlockY;
        else
            RealTileHeightPixel = m_pResolutionDescriptor->GetBlockHeight();

        // We always must resize the codec...
        // This reset may be not needed, keep it at this time to be sure...
        m_IntergraphResolutionDescriptor.pCodec->Reset();
        m_IntergraphResolutionDescriptor.pCodec->SetDimensions(RealTileWidthPixel, RealTileHeightPixel);
        m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(
            (m_pResolutionDescriptor->GetBlockWidth() - RealTileWidthPixel) * m_BitPerPixel);
        }

    // Calculate the correct tile offfset. This information was always given according the
    // end of the header, not by the begining of the file...
    FileOffset = (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH) + TileEntryInformation->S;

    // Check if we have a un-instantiated tile
    if (TileEntryInformation->S == 0)
        memset(po_pData, TileEntryInformation->U, m_TileSizeInByte);
    else
        {
        // Positionnate the cursor in the file according the tile position.
        m_pIntergraphFile->SeekToPos(FileOffset);

        // Check if we have an incomplete tile and if the missing portion
        // is on the right side (actually we dont care of the bottom side).
        if ((m_IntergraphResolutionDescriptor.pCodec == 0) && (pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth()) > m_pResolutionDescriptor->GetWidth())
            {
            HArrayAutoPtr<Byte> pTempBuffer(new Byte[m_TileSizeInByte]);
            if (m_IntergraphResolutionDescriptor.pCodec == 0)
                {
                // Read the available data into the temp buffer.
                if (m_pIntergraphFile->Read(pTempBuffer, TileEntryInformation->U) != TileEntryInformation->U)
                    goto WRAPUP;    // H_ERROR

                uint32_t NewTileWidthInByte = (uint32_t)((m_pResolutionDescriptor->GetWidth() - pi_PosBlockX) * ((float)m_BitPerPixel / 8.0));

                BufferIndex = 0;
                memset(po_pData, 0, m_TileSizeInByte);

                // Fill the buffer properly...
                for (i=0; i< m_TileHeight; i++)
                    {
                    memcpy(&po_pData[i * m_TileWidthInByte], &pTempBuffer[BufferIndex], NewTileWidthInByte);
                    BufferIndex += NewTileWidthInByte;
                    }
                }
            }
        else
            {
            // If the tile is enterely full, simply read it directly into the buffer
            if (m_IntergraphResolutionDescriptor.pCodec == 0)
                {
                if (m_pIntergraphFile->Read(po_pData, TileEntryInformation->U) != TileEntryInformation->U)
                    goto WRAPUP;    // H_ERROR
                }
            else
                {
                HAutoPtr<HCDPacket> pCompressedPacket;

                // If we pass here, we have a complete compress tile.  Read the data, decompress it
                // and put the decompress data to the output buffer : po_pData
                HArrayAutoPtr<Byte> pTempBuffer;
                BufferSize = TileEntryInformation->U;

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
                    pTempBuffer = new Byte[BufferSize + TotalJpegHeaderSize];

                    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_pJpegPacketPacket)
                        {
                        QualityFactor = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_pJpegPacketPacket->QualityFactor;
                        }

                    // Pre initialize the tile buffer (build header information).
                    InitializeJpegDecompTable(QualityFactor, pTempBuffer, BufferSize);

                    pCompressedPacket = new HCDPacket ((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                                                       pTempBuffer,
                                                       BufferSize + TotalJpegHeaderSize,
                                                       BufferSize + TotalJpegHeaderSize);

                    if (m_pIntergraphFile->Read(pTempBuffer + JPegTableSize + JPegTileHeaderSize, BufferSize) != BufferSize)
                        goto WRAPUP;    // H_ERROR
                    }
                else
                    {
                    // All other compression case will pass tight here.... (RLE, CCITT, RLE8, etc....)
                    // The temporary buffer will be used to hold the compress data
                    pTempBuffer = new Byte[BufferSize];
                    pCompressedPacket = new HCDPacket ((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                                                       pTempBuffer,
                                                       BufferSize,
                                                       BufferSize);

                    if (m_pIntergraphFile->Read(pTempBuffer.get(), BufferSize) != BufferSize)
                        goto WRAPUP;    // H_ERROR
                    }

                // If the temporary buffer has been read correctly,
                // decompress it and put the data into the po_pData.
                HCDPacket uncompress(po_pData, m_TileSizeInByte);
                pCompressedPacket->Decompress(&uncompress);
                }
            }
        }

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->HasLUTColorCorrection())
        {
        ApplyLUTColorCorrection(po_pData, m_pResolutionDescriptor->GetBlockWidth() * m_TileHeight);
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock: Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFIntergraphTileEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                           uint64_t           pi_PosBlockY,
                                           HFCPtr<HCDPacket>& po_rpPacket,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION(pi_PosBlockX <= m_pResolutionDescriptor->GetWidth());
    HPRECONDITION(pi_PosBlockY <= m_pResolutionDescriptor->GetHeight());

    HSTATUS Status = H_NOT_FOUND;

    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        // Setup the packet
        if(po_rpPacket->GetBufferSize() == 0)
            {
            po_rpPacket->SetBuffer(new Byte[m_TileSizeInByte], m_TileSizeInByte);
            po_rpPacket->SetBufferOwnership(true);
            }
        // Read the tile
        Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket->GetBufferAddress(), pi_pSisterFileLock);
        if (Status == H_SUCCESS)
            {
            po_rpPacket->SetDataSize(m_TileSizeInByte);
            po_rpPacket->SetCodec(new HCDCodecIdentity(m_TileSizeInByte));
            }
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock: Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFIntergraphTileEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                            uint64_t     pi_PosBlockY,
                                            const Byte*  pi_pData,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_pData != 0);
    HPRECONDITION (pi_PosBlockX <= m_pResolutionDescriptor->GetWidth());
    HPRECONDITION (pi_PosBlockY <= m_pResolutionDescriptor->GetHeight());
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_ERROR;

    uint32_t TileIndex;
    uint32_t RealTileWidthPixel;
    uint32_t RealTileHeightPixel;
    uint32_t FileOffset     = 0;
    uint32_t TileSizeInByte = 0;
    uint32_t IncompleteTileWidth;
    uint32_t IncompleteTileHeight;

    HArrayAutoPtr<Byte> pTempRasterBuffer;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Retrieve the right tile entry info
    HASSERT_X64(m_pTileIdDescriptor->ComputeIndex(pi_PosBlockX, pi_PosBlockY) <= ULONG_MAX);
    TileIndex = (uint32_t)m_pTileIdDescriptor->ComputeIndex(pi_PosBlockX, pi_PosBlockY);

    HRFIntergraphFile::TileEntry& TileEntryInformation = m_IntergraphResolutionDescriptor.pTileDirectoryEntry[TileIndex];

    if (m_IntergraphResolutionDescriptor.pCodec != 0)
        {
        if (((uint32_t)pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth()) > m_pResolutionDescriptor->GetWidth())
            RealTileWidthPixel = (uint32_t)m_pResolutionDescriptor->GetWidth() - (uint32_t)pi_PosBlockX;
        else
            RealTileWidthPixel = m_pResolutionDescriptor->GetBlockWidth();

        if (((uint32_t)pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight()) > m_pResolutionDescriptor->GetHeight())
            RealTileHeightPixel = (uint32_t)m_pResolutionDescriptor->GetHeight() - (uint32_t)pi_PosBlockY;
        else
            RealTileHeightPixel = m_pResolutionDescriptor->GetBlockHeight();

        // We always must resize the codec...
        // This reset may be not needed, keep it at this time to be sure...
        m_IntergraphResolutionDescriptor.pCodec->Reset();
        m_IntergraphResolutionDescriptor.pCodec->SetDimensions(RealTileWidthPixel,
                                                               RealTileHeightPixel);

        // The next condition have been removed to correct an edition error on format c29
        // if (RealTileWidthPixel % 4)
        //     m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits( RealTileWidthPixel  % 4);

        m_IntergraphResolutionDescriptor.pCodec->SetLinePaddingBits(RealTileWidthPixel % 4);

        // if (((HFCPtr<HRFIntergraphFile > &)GetRasterFile())->GetDatatypeCode() == HRFIntergraphFile::RLE)
        if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecHMRRLE1::CLASS_ID))
            ((HFCPtr<HCDCodecHMRRLE1> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);

        if (m_IntergraphResolutionDescriptor.pCodec->IsCompatibleWith(HCDCodecCRL8::CLASS_ID) &&
            static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_IntergraphHeader.IBlock1.scn == 1)
            ((HFCPtr<HCDCodecCRL8> &)m_IntergraphResolutionDescriptor.pCodec)->SetLineHeader(true);
        }
    else
        {
        TileSizeInByte = GetCurrentTileSizeInByte((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);
        FindFileFreeSpace(TileEntryInformation.S, TileEntryInformation.A, TileSizeInByte);

        m_pIntergraphFile->SeekToPos(TileEntryInformation.S + (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH));
        }

    // Check if we have an incomplete tile and if the missing portion
    // is on the right side (actually we dont care of the bottom side).
    if ( (((uint32_t)pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth ()) > m_pResolutionDescriptor->GetWidth ()) )
        {
        IncompleteTileWidth = MIN((uint32_t)m_pResolutionDescriptor->GetWidth(), (uint32_t)m_pResolutionDescriptor->GetWidth() - (uint32_t)pi_PosBlockX);
        IncompleteTileWidth = (uint32_t)ceil((float)(IncompleteTileWidth) * ((float)m_BitPerPixel / 8.0));

        HASSERT( IncompleteTileWidth > 0);

        if ((uint32_t)pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight() > m_pResolutionDescriptor->GetHeight())
            IncompleteTileHeight = (uint32_t)m_pResolutionDescriptor->GetHeight() - (uint32_t)pi_PosBlockY;
        else
            IncompleteTileHeight = m_pResolutionDescriptor->GetBlockHeight();

        TileSizeInByte = IncompleteTileWidth * IncompleteTileHeight;

        pTempRasterBuffer = new Byte[TileSizeInByte];

        // Reformat the buffer to remove the waste space...
        uint32_t BufferIndex = 0;
        for (uint32_t i=0; i < IncompleteTileHeight; i++)
            {
            memcpy(&pTempRasterBuffer[BufferIndex] ,
                   &pi_pData[i * m_TileWidthInByte],
                   IncompleteTileWidth);
            BufferIndex += IncompleteTileWidth;
            }

        if (m_IntergraphResolutionDescriptor.pCodec == 0)
            {
            if(m_pIntergraphFile->Write(pTempRasterBuffer, TileSizeInByte) != TileSizeInByte)
                goto WRAPUP;    // H_ERROR;

            // Make sure data is aligned to 32 bits boundary
            if (TileSizeInByte % 4)
                {
                int32_t TilePaddindByte = 4 - (TileSizeInByte % 4);

                uint32_t dummy=0;
                if(m_pIntergraphFile->Write(&dummy, TilePaddindByte) != TilePaddindByte)
                    goto WRAPUP;    // H_ERROR;

                TileSizeInByte += TilePaddindByte;
                }
            }
        else
            {
            size_t  MaxCompressedSize = m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize();
            HArrayAutoPtr<Byte> pTempCompressRasterBuffer(new Byte[MaxCompressedSize]);

            // Create a uncompress packet with original uncompress data
            HCDPacket UnCompress(pTempRasterBuffer,TileSizeInByte, TileSizeInByte);

            // Create a compress packet were the data will be compress
            HCDPacket Compress((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                               pTempCompressRasterBuffer,
                               m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize());

            // Compress the data and get it's new size.
            UnCompress.Compress(&Compress);
            HASSERT_X64(Compress.GetDataSize() < ULONG_MAX);
            TileSizeInByte = (uint32_t)Compress.GetDataSize();

            // Write the line need into the file...
            if (TileSizeInByte)
                {
                int32_t TilePaddindByte = TileSizeInByte % 4 ? (4 - (TileSizeInByte % 4)) : 0;    // Data must be aligned to 32 bits boundary

                FindFileFreeSpace(TileEntryInformation.S, TileEntryInformation.A, TileSizeInByte + TilePaddindByte);
                m_pIntergraphFile->SeekToPos(TileEntryInformation.S + (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH));

                if (m_pIntergraphFile->Write(Compress.GetBufferAddress(), TileSizeInByte) != TileSizeInByte)
                    goto WRAPUP;

                // Make sure data is aligned to 32 bits boundary
                if (TilePaddindByte)
                    {
                    uint32_t dummy=0;
                    if(m_pIntergraphFile->Write(&dummy, TilePaddindByte) != TilePaddindByte)
                        goto WRAPUP;    // H_ERROR;

                    TileSizeInByte += TilePaddindByte;
                    }
                }
            }
        }
    else    // For complete tile
        {
        // Write the complete compressed tile data into the raster file
        if (m_IntergraphResolutionDescriptor.pCodec != 0)
            {
            // For compress data, compress it into the correct codec,
            // then write it into the file.
            size_t  MaxCompressedSize = m_IntergraphResolutionDescriptor.pCodec->GetSubsetMaxCompressedSize();
            pTempRasterBuffer = new Byte[MaxCompressedSize];

            // Create a uncompress packet with original uncompress data
            HCDPacket UnCompress((const_cast<Byte*> (pi_pData)),m_TileSizeInByte, m_TileSizeInByte);

            // Create a compress packet were the data will be compress
            HCDPacket Compress((HFCPtr<class HCDCodec>)(m_IntergraphResolutionDescriptor.pCodec),
                               pTempRasterBuffer, MaxCompressedSize);

            // Compress the data and get it's new size.
            UnCompress.Compress(&Compress);
            HASSERT_X64(Compress.GetDataSize() < ULONG_MAX);
            TileSizeInByte = (uint32_t)Compress.GetDataSize();

            // Write the line need into the file...
            if (TileSizeInByte)
                {
                HASSERT(TileEntryInformation.A >= TileEntryInformation.U);

                int32_t TilePaddindByte = TileSizeInByte % 4 ? (4 - (TileSizeInByte % 4)) : 0;        // Data must be aligned to 32 bits boundary

                FindFileFreeSpace(TileEntryInformation.S, TileEntryInformation.A, TileSizeInByte + TilePaddindByte);
                m_pIntergraphFile->SeekToPos(TileEntryInformation.S + (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH));

                if(m_pIntergraphFile->Write(Compress.GetBufferAddress(), TileSizeInByte) != TileSizeInByte)
                    goto WRAPUP;    // H_ERROR;

                // Make sure data is aligned to 32 bits boundary
                if (TilePaddindByte)
                    {
                    uint32_t dummy=0;
                    if(m_pIntergraphFile->Write(&dummy, TilePaddindByte) != TilePaddindByte)
                        goto WRAPUP;    // H_ERROR;

                    TileSizeInByte += TilePaddindByte;
                    }

                // Check if we have enough free space allocated to write the data
                // and finalyse the file cursor position.
                }
            }
        // Write the complete uncompressed tile data into the raster file
        else
            {
            if (m_pIntergraphFile->Write(pi_pData, TileSizeInByte) != TileSizeInByte)
                goto WRAPUP;    // H_ERROR;
            }
        }

    // Debugging helper to trap invalid tile size...
    // HASSERT (TileSizeInByte <= (1024 * 512 * 6));

    HASSERT( !(TileSizeInByte % 4));
    HASSERT( TileSizeInByte <= TileEntryInformation.A);

    TileEntryInformation.A = TileSizeInByte;    //TR 109180
    TileEntryInformation.U = TileSizeInByte;

    // Positionnate the file cursor into the file directory structure for an update...
    FileOffset = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH;

    if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
        FileOffset += m_IntergraphResolutionDescriptor.pOverviewEntry->S;

    FileOffset += (sizeof(HRFIntergraphFile::TileDirectoryInfo) + (sizeof(HRFIntergraphFile::TileEntry) * TileIndex));
    m_pIntergraphFile->SeekToPos(FileOffset);

    if (m_pIntergraphFile->Write(&(m_IntergraphResolutionDescriptor.pTileDirectoryEntry[TileIndex]),
                                 sizeof(Byte) * sizeof(HRFIntergraphFile::TileEntry)) !=
        sizeof(Byte) * sizeof(HRFIntergraphFile::TileEntry))
        goto WRAPUP;


    //TR 109180 - Remove the update of pOverviewEntry->A,U, why do we need to keep it updated ?

    // At last TILE of the the very last resolution, the packet overview information MUST be updated.
    if ((m_IntergraphResolutionDescriptor.pOverviewEntry != 0)          &&
        m_Resolution == static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_SubResolution)
        {
        // Refresh the file header.
        static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->UpdatePacketOverview(m_IntergraphResolutionDescriptor.pOverviewEntry->S,
                                                                             m_Resolution);
        }

    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    // Verify Tile structure integrity
    // HASSERT(VerrifyBlockOverlap());

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

bool HRFIntergraphTileEditor::IsUnInstanciateTile (uint32_t     pi_PosTileX,
                                                    uint32_t     pi_PosTileY,
                                                    const Byte* pi_pData)
    {
    HPRECONDITION ((m_BitPerPixel == 8) || (m_BitPerPixel == 1));
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_pData != 0);
    HPRECONDITION (pi_PosTileX <= m_pResolutionDescriptor->GetWidth());
    HPRECONDITION (pi_PosTileY <= m_pResolutionDescriptor->GetHeight());

    bool Status         = false;
    bool ValidPixelType = true;
    bool DifferentFound = false;

    Byte color;

    // !!!! We may put the following code elsewhere to eliminate the need to do it every
    //      time we pass in this method.
    // By this first test, eliminate all unsupported Bit/Pixel and all 1 bit
    // raster who doesnt have there 8 first pixel equal
    if (m_BitPerPixel == 8)
        color = pi_pData[0];
    else
        {
        if (m_BitPerPixel == 1)
            {
            // Verrify if all pixel were the same...
            if ((pi_pData[0] == 0xff) || (pi_pData[0] == 0x00))
                color = pi_pData[0];
            else
                ValidPixelType = false;
            }
        else
            {
            // We dont support any other pixel type at this time...
            ValidPixelType = false;
            }
        }
    // !!!! End of dode moving commentary.

    if (ValidPixelType)
        {
        // Compute some operation for optimisation
        uint32_t RealTileWidthPixel;
        uint32_t RealTileHeightPixel;
        uint32_t RealTileWidthByte;

        // We can't precompute elsewhere these value because they may change every time...
        if ((pi_PosTileX + m_pResolutionDescriptor->GetBlockWidth()) > m_pResolutionDescriptor->GetWidth())
            RealTileWidthPixel = (uint32_t)m_pResolutionDescriptor->GetWidth() - pi_PosTileX;
        else
            RealTileWidthPixel = m_pResolutionDescriptor->GetBlockWidth();

        if ((pi_PosTileY + m_pResolutionDescriptor->GetBlockHeight()) > m_pResolutionDescriptor->GetHeight())
            RealTileHeightPixel = (uint32_t)m_pResolutionDescriptor->GetHeight() - pi_PosTileY;
        else
            RealTileHeightPixel = m_pResolutionDescriptor->GetBlockHeight();

        // Check all pixel of the firts line...
        RealTileWidthByte = (uint32_t)ceil((float)RealTileWidthPixel * ((float)m_BitPerPixel / 8.0));

        HASSERT(m_pCompBuffer != 0);

        // Build an homogenious line sample with all pixel with the same value.
        memset(m_pCompBuffer, pi_pData[0], RealTileWidthByte);

        // Compair all tile line per line with the homogenious line
        // sample to see if they are build with a unique pixel color.
        uint32_t Index = 0;
        uint32_t BufferIndex = 0;
        do
            {
            DifferentFound = memcmp(m_pCompBuffer, &pi_pData[BufferIndex], RealTileWidthByte) != 0;
            BufferIndex += RealTileWidthByte;

            }
        while(!DifferentFound && (++Index < RealTileHeightPixel));
        if (!DifferentFound)
            Status = true;
        }
    return Status;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFIntergraphTileEditor::FindFileFreeSpace(uint32_t& pio_rOldBlockOffset, uint32_t& pio_rOldBlockSize, uint32_t pi_RequestedSize)
    {
    // TR#245365: I do not understand why this would be required here. It is EXTREMELY slow to call this each time we rewrite a tile.
    //m_pIntergraphFile->Flush();

    // Then ask for free space and return the file Offset
    CheckAlloc (&pio_rOldBlockOffset, pio_rOldBlockSize, pi_RequestedSize);

    // Update the allocated Tile size. ( ->A )
    pio_rOldBlockSize   = pi_RequestedSize;

    // Verrify Tile structure integrity
    // HASSERT(VerrifyBlockOverlap());
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFIntergraphTileEditor::InitializeJpegDecompTable(double pi_QualityFactor, Byte* po_pTileBuffer, unsigned int pi_DataSize)
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

        //  Standard JPEG
        //  0x00, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,

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

    // Ugly patch wich corrected over color sturation (too much contrast)
    // sacrificing some image quality...
    pi_QualityFactor = 9;

    // If Grayscale...
    if (static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_DataTypeCode == 30)
        {
        memcpy(po_pTileBuffer, GrayTablesHeader, JpegGrayTablesSize);
        memcpy(po_pTileBuffer + JpegGrayTablesSize, GrayTileHeader, JPegGrayTileHeaderSize);

        // Initialise Tile height within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        Byte SizeHI  = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockHeight() >> 8);
        Byte SizeLOW = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockHeight());
        memcpy(po_pTileBuffer + JpegGrayTablesSize + 5, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegGrayTablesSize + 6, &SizeLOW, 1);

        // Initialise Tile width within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        SizeHI  = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockWidth() >> 8);
        SizeLOW = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockWidth());
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
        Byte SizeHI  = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockHeight() >> 8);
        Byte SizeLOW = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockHeight());
        memcpy(po_pTileBuffer + JpegColorTablesSize + 5, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegColorTablesSize + 6, &SizeLOW, 1);

        // Initialise Tile width within the Tile Header, warning,
        // do not forget to set the HI/LOW WORD order properly.
        SizeHI  = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockWidth() >> 8);
        SizeLOW = CONVERT_TO_BYTE(m_pResolutionDescriptor->GetBlockWidth());
        memcpy(po_pTileBuffer + JpegColorTablesSize + 7, &SizeHI , 1);
        memcpy(po_pTileBuffer + JpegColorTablesSize + 8, &SizeLOW, 1);

        // Set Jpeg end trailer after the compressed data.
        po_pTileBuffer[pi_DataSize + JPegColorTileHeaderSize + JpegColorTablesSize + 0] = 0xff;
        po_pTileBuffer[pi_DataSize + JPegColorTileHeaderSize + JpegColorTablesSize + 1] = 0xd9;

        // Build chromance and luminance table directlty into the jpeg header
        // giving for each the right pointer location.
        BuildJpegLumiChromaTable(pi_QualityFactor, po_pTileBuffer + 7, po_pTileBuffer + 76);
        }

    // Reset codec options.
    // Set codec options
    /*
    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResolutionDescriptor.pCodec)->Reset();
    ((HFCPtr<HCDCodecIJG> &)m_IntergraphResolutionDescriptor.pCodec)->SetDimensions(m_pResolutionDescriptor->GetBlockWidth(),
                                                                                        m_pResolutionDescriptor->GetBlockHeight()); // */
    // ((HFCPtr<HCDCodecIJG> &)m_IntergraphResolutionDescriptor.pCodec)->SetQuality(pi_QualityFactor * 2);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFIntergraphTileEditor::BuildJpegLumiChromaTable(double pi_QualityFactor, Byte* po_pLuminance, Byte* po_pChroma)
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

    /*
    UInt32 NaturalOrder[64] ={  0,  1,  5,  6, 14, 15, 27, 28,
                               2,  4,  7, 13, 16, 26, 29, 42,
                               3,  8, 12, 17, 25, 30, 41, 43,
                               9, 11, 18, 24, 31, 40, 44, 53,
                              10, 19, 23, 32, 39, 45, 52, 54,
                              20, 22, 33, 38, 46, 51, 55, 60,
                              21, 34, 37, 47, 50, 56, 59, 61,
                              35, 36, 48, 49, 57, 58, 62, 63 };  // */

    // Scale the Lumi/Chroma table to the 8Bits scale.
    for (Line = 0; Line < TableHeight; Line++)
        {
        for (Column = 0; Column < TableWidth; Column++)
            {
            LumiIndex   = (Line   * 8) + Column;
            ChromaIndex = (Column * 8) + Line;

            // !!!!HChkSebG!!!!
            //
            // Here we modify the ratio to avoid over contrast color.  But this an ugly patch had
            // a flaw, the picture quality is decrease. We should find why this append, and correct it.
            //
            TmpLuminance [ChromaIndex] = (uint32_t)(Luminance [LumiIndex] * pi_QualityFactor /  50L); // 75.0 !!!!HChkSebG!!!!
            TmpChrominace[ChromaIndex] = (uint32_t)(Chrominace[LumiIndex] * pi_QualityFactor /  50L); // 75.0 !!!!HChkSebG!!!!

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
//
//-----------------------------------------------------------------------------

uint32_t HRFIntergraphTileEditor::GetCurrentTileSizeInByte( const uint32_t pi_PosBlockX, const uint32_t pi_PosBlockY) const
    {
    uint32_t TileSizeInByte;
    uint32_t IncompleteTileWidth;
    uint32_t IncompleteTileHeight;

    // Special case for incomplete tile.
    if ( ((pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth ()) > m_pResolutionDescriptor->GetWidth ()) ||
         ((pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight()) > m_pResolutionDescriptor->GetHeight()) )
        {
        if ((pi_PosBlockX + m_pResolutionDescriptor->GetBlockWidth ()) > m_pResolutionDescriptor->GetWidth ())
            IncompleteTileWidth  = (uint32_t)m_pResolutionDescriptor->GetWidth() - pi_PosBlockX;
        else
            IncompleteTileWidth  = m_pResolutionDescriptor->GetBlockWidth();

        IncompleteTileWidth = (uint32_t)ceil((float)IncompleteTileWidth * ((float)m_BitPerPixel / 8.0));

        // Pad the TileSize to longword bondary. ( an even multiple of 32 bits)
        if (IncompleteTileWidth % 4)
            IncompleteTileWidth += 4 - (IncompleteTileWidth % 4);

        if (pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight() > m_pResolutionDescriptor->GetHeight())
            IncompleteTileHeight = (uint32_t)m_pResolutionDescriptor->GetHeight() - pi_PosBlockY;
        else
            IncompleteTileHeight = m_pResolutionDescriptor->GetBlockHeight();

        TileSizeInByte = IncompleteTileWidth * IncompleteTileHeight;
        }
    else
        {
        HDEBUGCODE( uint32_t TotalSize = (uint32_t)(ceil((float)m_pResolutionDescriptor->GetBlockWidth() * ((float)m_BitPerPixel / 8.0))) * m_pResolutionDescriptor->GetBlockHeight());
        HASSERT( m_TileSizeInByte == TotalSize);

        TileSizeInByte = m_TileSizeInByte;
        }

    // Pad the TileSize to longword bondary. ( an even multiple of 32 bits)
    if (TileSizeInByte % 4)
        TileSizeInByte += 4 - (TileSizeInByte % 4);

    return TileSizeInByte;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void HRFIntergraphTileEditor::AddInitialFreeBlock (const uint32_t* pi_pOffset, const uint32_t* pi_pSize, uint32_t pi_Count)
    {
    HRFIntergraphFile::StreamFreeBlock Block;

    for (uint32_t i=0; i<pi_Count; i++)
        {
        Block.Offset = pi_pOffset[i];
        Block.Size   = pi_pSize[i];

        m_ListOfFreeBlock.push_back(Block);
        }
    }

//-----------------------------------------------------------------------------
// The caller must delete the lists.
// Return true if the list has been modifying
//-----------------------------------------------------------------------------

bool HRFIntergraphTileEditor::GetListFreeBlock (uint32_t** pi_ppOffset, uint32_t** pi_ppSize, uint32_t* pi_pCount)
    {
    if ((*pi_pCount = (uint32_t)m_ListOfFreeBlock.size()) > 0)
        {
        *pi_ppOffset = new uint32_t[*pi_pCount];
        *pi_ppSize   = new uint32_t[*pi_pCount];

        HRFIntergraphFile::ListOfFreeBlock::iterator Itr;
        uint32_t i=0;
        for (Itr=m_ListOfFreeBlock.begin(); Itr != m_ListOfFreeBlock.end(); Itr++,i++)
            {
            (*pi_ppOffset)[i] = (*Itr).Offset;
            (*pi_ppSize)[i]   = (*Itr).Size;
            }
        }
    else
        {
        *pi_ppOffset = 0;
        *pi_ppSize   = 0;
        *pi_pCount   = 0;
        }

    return m_ListDirty;
    }

//-----------------------------------------------------------------------------
// CheckAlloc : Check if you can rewrite at the same place, or try to allocate
//              an other block. (take a free block, or append at the end of file)
//
//  If a new place is found, pio_pOffset receive the new offset.
//  If pi_CurSize == 0 no previous block.
//-----------------------------------------------------------------------------

void HRFIntergraphTileEditor::CheckAlloc (uint32_t* pio_pOffset, uint32_t pi_CurSize, uint32_t pi_NewSize)
    {
    // If new block is ==, rewrite at the same place, do nothing
    // Attention: this return is lost in code......
    if (pi_NewSize == pi_CurSize)
        return;

    // NB. This list is kept sorted by offset.
    HRFIntergraphFile::ListOfFreeBlock::iterator Itr;
    HRFIntergraphFile::ListOfFreeBlock::iterator BestSizeItr;

    uint32_t BestSize     = (uint32_t)-1;
    bool   FirstBlock   = false;
    bool   BlockMerge   = false;
    uint32_t OldEndOffset = *pio_pOffset + pi_CurSize;  // if this value = 0 then

    // Specify the list is dirty
    m_ListDirty = true;

    // If it is my first free block, add it
    if ((m_ListOfFreeBlock.size() == 0) && (pi_CurSize != 0))
        {
        HRFIntergraphFile::StreamFreeBlock Fb;

        Fb.Offset = *pio_pOffset;
        Fb.Size   = pi_CurSize;
        m_ListOfFreeBlock.push_front(Fb);
        FirstBlock = true;
        }

    // Take the offset of the last block in the list.
    HRFIntergraphFile::StreamFreeBlock   LastBlock;
    if (m_ListOfFreeBlock.size() > 0)
        LastBlock = m_ListOfFreeBlock.back();

    // Add new free block, Try to merge it
    for (Itr=m_ListOfFreeBlock.begin(); Itr != m_ListOfFreeBlock.end(); Itr++)
        {
        // Check if I can merge/add my old block
        // Don't try if no old block present.
        if (!BlockMerge && (pi_CurSize != 0))
            {
            // Try to merge with the previous block
            if (*pio_pOffset == ((*Itr).Offset+(*Itr).Size))
                {
                // Merge with the previous block
                (*Itr).Size   += pi_CurSize;
                BlockMerge  = true;

                // If merged with the previous, now check for the next block too
                HRFIntergraphFile::ListOfFreeBlock::iterator NextItr = Itr;
                NextItr++;
                if ((NextItr != m_ListOfFreeBlock.end()) &&
                    (((*Itr).Offset+(*Itr).Size) == (*NextItr).Offset) )
                    {
                    (*Itr).Size   += (*NextItr).Size;
                    m_ListOfFreeBlock.erase(NextItr);
                    }
                }

            // Try to merge with the next block only
            else if (OldEndOffset == (*Itr).Offset)
                {
                // Merge with the Next block
                (*Itr).Offset  = *pio_pOffset;
                (*Itr).Size   +=  pi_CurSize;
                BlockMerge   = true;
                }

            // Add the block
            else if ((*pio_pOffset < (*Itr).Offset) && !FirstBlock)
                {
                // Before the current Block
                //
                HRFIntergraphFile::StreamFreeBlock Fb;

                Fb.Offset = *pio_pOffset;
                Fb.Size   = pi_CurSize;

                m_ListOfFreeBlock.insert(Itr, Fb);
                BlockMerge   = true;
                }

            // Append the block you are at the end of the list
            else if (!FirstBlock && ((*Itr).Offset == LastBlock.Offset))
                {
                HRFIntergraphFile::StreamFreeBlock Fb;

                Fb.Offset = *pio_pOffset;
                Fb.Size   = pi_CurSize;
                m_ListOfFreeBlock.push_back(Fb);
                BlockMerge   = true;
                }
            }

        // Check for the best Size
        if (((*Itr).Size >= pi_NewSize) &&
            ((*Itr).Size < BestSize))
            {
            BestSize    = (*Itr).Size;
            BestSizeItr = Itr;
            }
        }

    // Check if block found
    if (BestSize != (uint32_t)-1)
        {
        // Set new offset
        *pio_pOffset = (*BestSizeItr).Offset;

        // Block, check if you use the complete block
        if (pi_NewSize == (*BestSizeItr).Size)
            {
            m_ListOfFreeBlock.erase(BestSizeItr);
            }
        else
            {
            // Ajuste the block
            (*BestSizeItr).Offset += pi_NewSize;
            (*BestSizeItr).Size   -= pi_NewSize;
            }
        }
    else
        {
        // No block found, extented the file.
        *pio_pOffset = (uint32_t)m_pIntergraphFile->GetSize();
        }

    HPOSTCONDITION(*pio_pOffset <= m_pIntergraphFile->GetSize());
    }

/**----------------------------------------------------------------------------
 Check if you can rewrite at the same place, else allocate a new block at the
 end.(in this case, the old block is lost)

 @param pio_pOffset receive the offset to use.
 @param pi_CurSize Size of the previous block.(==0 means no previous block)

-----------------------------------------------------------------------------*/

void HRFIntergraphTileEditor::CheckAllocWithoutUpdate (uint32_t* pio_pOffset, uint32_t pi_CurSize, uint32_t pi_NewSize)
    {
    // If new block is <=, rewrite at the same place, do nothing
    // Attention: this return is lost in code......
    if (pi_NewSize > pi_CurSize)
        {
        // add at the end of the file.
        *pio_pOffset = (uint32_t)m_pIntergraphFile->GetSize();
        }
    }


/**----------------------------------------------------------------------------
 Verify that the specified block (position and size) does not overlap one
 of our free blocks. This can happen in files that were edited prior to
 adding the CheckAllocWithoutUpdate method for free blocks saving.

 @param pio_Offset receive the offset of the block to check.
 @param pi_Size Size of the block.
 @return true if there is an overlap, false otherwise.

-----------------------------------------------------------------------------*/

bool HRFIntergraphTileEditor::OverlapsFreeBlocks(uint32_t pi_Offset, uint32_t pi_Size) const
    {
    bool Result = false;

    HRFIntergraphFile::ListOfFreeBlock::const_iterator Itr(m_ListOfFreeBlock.begin());
    while (!Result && Itr != m_ListOfFreeBlock.end())
        {
        // Test block overlapping
        if (pi_Offset < ((*Itr).Offset+(*Itr).Size) &&
            pi_Offset+pi_Size > (*Itr).Offset)
            {
            Result = true;
            }

        ++Itr;
        }
    return Result;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HRFIntergraphTileEditor::MapStreamHole()
    {
    HPRECONDITION(m_IntergraphResolutionDescriptor.pTileDirectoryEntry != 0);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Scan the whole directory. For each TileEntry, map the used memory.
    HRFIntergraphFile::ListOfFreeBlock ListOfUsedBlock;
    HRFIntergraphFile::TileEntry* pTileEntryInformation;

    HASSERT_X64(m_pTileIdDescriptor->GetTileCount() < ULONG_MAX);
    uint32_t TotalTileCount   = (uint32_t)m_pTileIdDescriptor->GetTileCount();

    uint32_t OverviewOutbound = (uint32_t)m_pIntergraphFile->GetSize();
    uint32_t OverviewInbound  = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH;


    if ( ((static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->m_SubResolution) > 1) &&
         (!m_IntergraphResolutionDescriptor.pOverviewEntry))
        {
        OverviewOutbound = 0;
        OverviewInbound  = static_cast<HRFIntergraphFile*>(GetRasterFile().GetPtr())->GetBlockNumInHeader() * HRF_INTERGRAGH_HEADER_BLOCK_LENGTH;
        }

    if (m_IntergraphResolutionDescriptor.pOverviewEntry != 0)
        {
        OverviewOutbound = m_IntergraphResolutionDescriptor.pOverviewEntry->S +
                           m_IntergraphResolutionDescriptor.pOverviewEntry->A;

        OverviewInbound  = m_IntergraphResolutionDescriptor.pOverviewEntry->S;
        }

    for (uint32_t TileIndex = 0; TileIndex < TotalTileCount; TileIndex++)
        {
        pTileEntryInformation = &(m_IntergraphResolutionDescriptor.pTileDirectoryEntry[TileIndex]);

        // Verrify if our block is contigious of anoher one. (previous and or next)
        HRFIntergraphFile::ListOfFreeBlock::iterator Itr(ListOfUsedBlock.begin());
        //HDEBUGCODE(UInt32 DebugTest = (*Itr).Offset);

        while ( (Itr != ListOfUsedBlock.end()) && (pTileEntryInformation->S > (*Itr).Offset))
            {
            Itr++;
            }

        // Insert the used block in order if it's within the orginal overview packet
        if (((pTileEntryInformation->S + pTileEntryInformation->A) < OverviewOutbound) && (pTileEntryInformation->S >= OverviewInbound))
            {
            ListOfUsedBlock.insert(Itr, HRFIntergraphFile::StreamFreeBlock( pTileEntryInformation->S, pTileEntryInformation->A));
            }
        }

    ListOfUsedBlock.sort();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Scan all block stored into the list to merge contigious one.
    HRFIntergraphFile::ListOfFreeBlock::iterator UsedBlocItr;
    HRFIntergraphFile::ListOfFreeBlock::iterator NextUsedBloc(ListOfUsedBlock.begin());

    while (NextUsedBloc != ListOfUsedBlock.end())
        {
        UsedBlocItr = NextUsedBloc;
        NextUsedBloc++;

        HDEBUGCODE( HRFIntergraphFile::StreamFreeBlock UsedB(*UsedBlocItr););
        HDEBUGCODE( HRFIntergraphFile::StreamFreeBlock NextB(*NextUsedBloc););

        if (NextUsedBloc != ListOfUsedBlock.end())
            {
            // Block cannot overlap !!!!
            HASSERT( ((*UsedBlocItr).Offset + (*UsedBlocItr).Size) <= (*NextUsedBloc).Offset );

            // if contigious found, merge!
            if ( ((*UsedBlocItr).Offset + (*UsedBlocItr).Size) == (*NextUsedBloc).Offset )
                {
                (*NextUsedBloc).Offset  = (*UsedBlocItr).Offset;
                (*NextUsedBloc).Size   += (*UsedBlocItr).Size;

                ListOfUsedBlock.erase(UsedBlocItr);
                }
            }
        }

    // Debugging code to evaluate a particular situation that could occur
    // when editing the first resolution of a multi-resolution file.
    // HASSERT(ListOfUsedBlock.size() > 0);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Find hole and add them to the AddInitialFreeBlock
    // More than one block mean a discontinuity in the stream.
    if (ListOfUsedBlock.size() > 1)
        {
        // Reset the next block.
        NextUsedBloc = ListOfUsedBlock.begin();

        uint32_t FreeBlockOffset;
        uint32_t FreeBlockSize;

        while (NextUsedBloc != ListOfUsedBlock.end())
            {
            UsedBlocItr = NextUsedBloc;
            NextUsedBloc++;

            HDEBUGCODE( HRFIntergraphFile::StreamFreeBlock UsedB(*UsedBlocItr););
            HDEBUGCODE( HRFIntergraphFile::StreamFreeBlock NextB(*NextUsedBloc););

            if (NextUsedBloc != ListOfUsedBlock.end())
                {
                FreeBlockOffset = ((*UsedBlocItr).Offset + (*UsedBlocItr).Size);
                FreeBlockSize   = (*NextUsedBloc).Offset - FreeBlockOffset;

                // A negative block size should never happen,
                // that's why we verrify it! ;-)
                HASSERT( (*NextUsedBloc).Offset > FreeBlockOffset);

                AddInitialFreeBlock(&FreeBlockOffset, &FreeBlockSize, 1);
                }
            }
        HASSERT(m_ListOfFreeBlock.size() > 0);
        }
    // Free our list of used block.
    ListOfUsedBlock.clear();
    }

//-----------------------------------------------------------------------------
// Verrify Tile structure integrity.
//-----------------------------------------------------------------------------

bool HRFIntergraphTileEditor::VerrifyBlockOverlap()
    {
    bool BlockAreNotOverlapped = true;

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // Scan the whole directory. For each TileEntry, map the used memory.
    HRFIntergraphFile::TileEntry* pTileEntryInformation;
    HRFIntergraphFile::TileEntry* pOverlappedTileEntryInformation;

    HASSERT_X64(m_pTileIdDescriptor->GetTileCount() <= ULONG_MAX);
    uint32_t TotalTileCount = (uint32_t)m_pTileIdDescriptor->GetTileCount();
    uint32_t OverlapTileIndex;
    uint32_t TileIndex;

    // For each Tile into the current resolution.
    for (TileIndex = 0; TileIndex < TotalTileCount && BlockAreNotOverlapped; TileIndex++)
        {
        pTileEntryInformation = &(m_IntergraphResolutionDescriptor.pTileDirectoryEntry[TileIndex]);

        // Verrify if we overlap with other blocks.
        for (OverlapTileIndex = TileIndex + 1; OverlapTileIndex < TotalTileCount && BlockAreNotOverlapped; OverlapTileIndex++)
            {
            pOverlappedTileEntryInformation = &(m_IntergraphResolutionDescriptor.pTileDirectoryEntry[OverlapTileIndex]);

            if (pTileEntryInformation->S < pOverlappedTileEntryInformation->S)
                {
                if (pTileEntryInformation->S + pTileEntryInformation->A > pOverlappedTileEntryInformation->S)
                    {
                    BlockAreNotOverlapped = false;
                    HDEBUGTEXT(L" ************ HRFIntergraphTileEditor::VerrifyBlockOverlap : Tile overlapped case 1 ************ \r\n");
                    }
                }
            else
                {
                if (pOverlappedTileEntryInformation->S + pOverlappedTileEntryInformation->A > pTileEntryInformation->S)
                    {
                    BlockAreNotOverlapped = false;
                    HDEBUGTEXT(L" ************ HRFIntergraphTileEditor::VerrifyBlockOverlap : Tile overlapped case 2 ************ \r\n");
                    }
                }
            }
        }
    return BlockAreNotOverlapped;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------

void HRFIntergraphTileEditor::ApplyLUTColorCorrection(Byte* pio_pData, uint32_t pi_pixelCount)
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
            pio_pData[ByteIndex] = pRedLUT  [pio_pData[ByteIndex++] ];
            pio_pData[ByteIndex] = pGreenLUT[pio_pData[ByteIndex++] ];
            pio_pData[ByteIndex] = pBlueLUT [pio_pData[ByteIndex++] ];
            }
        }
    else if (m_BitPerPixel == 8)
        {
        HASSERT(pRedLUT   != 0);

        for (uint32_t PixelIndex = 0; PixelIndex < pi_pixelCount; PixelIndex++)
            {
            pio_pData[ByteIndex] = pRedLUT  [pio_pData[ByteIndex++] ];
            }
        }
    // Should not occur.  This situation mean we have an Intergraph file with an LUT
    // not handled properly.  In release the LUT will be simply ignored.
    HDEBUGCODE(else HASSERT(false);)
        }
