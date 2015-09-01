//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTgaCompressLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTgaCompressLineEditor
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFTgaFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecTgaRLE.h>
#include <Imagepp/all/h/HRFTgaCompressLineEditor.h>

#define  BLOCKSIZE  0x400000

#define TGA_RASTERFILE  static_cast<HRFTgaFile*>(GetRasterFile().GetPtr())

/**-----------------------------------------------------------------------------
 Constructor of the class HRFTgaLineEditor. This method initializes the intern
 attibutes for later use.

 @param pi_rpRasterFile A pointer to the associate raster file.
 @param pi_Page The number of the associate page descriptor.
 @param pi_Resolution The number of the associate resolution descriptor.
 @param pi_AccessMode The access and sharing modes for the file.
------------------------------------------------------------------------------*/
HRFTgaCompressLineEditor::HRFTgaCompressLineEditor(HFCPtr<HRFTgaFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   unsigned short       pi_Resolution,
                                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(static_cast<HFCPtr<HRFRasterFile> >(pi_rpRasterFile),
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    HFCPtr<HRFTgaFile> pTgaFile  = TGA_RASTERFILE;

    // Initialize the offset buffer for the begining of each line
    m_pLineOffsetTbl        = new uint32_t [(uint32_t)GetResolutionDescriptor()->GetHeight() + 1];
    memset (m_pLineOffsetTbl, 0x00, ((uint32_t)GetResolutionDescriptor()->GetHeight()+1) * sizeof(uint32_t));
    m_pLineOffsetTbl[0]     = pTgaFile->GetRasterDataOffset();

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        pTgaFile->m_pTgaExtTableArea->m_pScanLineTable = new uint32_t [(uint32_t)GetResolutionDescriptor()->GetHeight()];

    // Set the codec
    m_pCodec = new HCDCodecTGARLE();
    m_pCodec->SetDimensions((uint32_t)GetResolutionDescriptor()->GetWidth(),
                            (uint32_t)GetResolutionDescriptor()->GetHeight());
    if (pTgaFile->m_pTgaFileHeader->m_PixelDepth == 16)
        {
        m_pCodec->SetBitsPerPixel(24);
        m_pCodec->SetNumberOfBitsPerPixelInOutput(16);
        }
    else
        {
        m_pCodec->SetBitsPerPixel(pTgaFile->m_pTgaFileHeader->m_PixelDepth);
        m_pCodec->SetNumberOfBitsPerPixelInOutput(pTgaFile->m_pTgaFileHeader->m_PixelDepth);
        }

    m_pCodec->SetSubset((uint32_t)GetResolutionDescriptor()->GetWidth(), 1);
    m_pCodec->SetAlphaChannelBits(pTgaFile->m_pTgaFileHeader->m_ImageDescriptor & 0x0E);
    if (m_pCodec->GetAlphaChannelBits() != 0 && pTgaFile->m_pTgaExtentionArea && pTgaFile->m_pTgaExtentionArea->m_AttributesType != 3)
        m_pCodec->SetAlphaChannelBits(0);
    }

/**-----------------------------------------------------------------------------
 Public destructor for the class.
------------------------------------------------------------------------------*/
HRFTgaCompressLineEditor::~HRFTgaCompressLineEditor()
    {
    HFCPtr<HRFTgaFile> pTgaFile  = TGA_RASTERFILE;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        memcpy (pTgaFile->m_pTgaExtTableArea->m_pScanLineTable,
                m_pLineOffsetTbl,
                (uint32_t)GetResolutionDescriptor()->GetHeight() * sizeof(uint32_t));
        }
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.
 The block position must be specified and can be compruted by the resolution
 descriptor.
 The @i{po_pData} must be allocated by the user of this function.
 The size must be > ${#GetBlockSizeInByte()}.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param po_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the readint operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaCompressLineEditor::ReadBlock(uint64_t  pi_PosBlockX,
                                            uint64_t  pi_PosBlockY,
                                            Byte*     po_pData,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION(pi_PosBlockX == 0);

    HSTATUS Status = H_SUCCESS;
    HFCPtr<HRFResolutionDescriptor> pResolution = GetResolutionDescriptor();


    if (!(GetRasterFile()->GetAccessMode().m_HasCreateAccess))
        {
        // Create a packet to perform the decompression
        HFCPtr<HCDPacket> pCompressed = new HCDPacket;

        // Read the data into the compressed packet
        Status = ReadBlock(pi_PosBlockX, pi_PosBlockY, pCompressed, pi_pSisterFileLock);

        if (Status == H_SUCCESS)
            {
            // Decompress the data
            HCDPacket Uncompressed(po_pData, GetResolutionDescriptor()->GetBlockSizeInBytes());
            pCompressed->Decompress(&Uncompressed);
            }

        pCompressed = 0;
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.
 The block position must be specified and can be compruted by the resolution
 descriptor.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param po_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the readint operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaCompressLineEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                            uint64_t           pi_PosBlockY,
                                            HFCPtr<HCDPacket>& po_rpPacket,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacket != 0);
    HPRECONDITION(pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION(pi_PosBlockX == 0);

    HSTATUS Status = H_ERROR;
    HFCLockMonitor SisterFileLock;

    uint64_t           Line;
    uint32_t           NbBytesRead;
    HFCPtr<HRFTgaFile>  pTgaFile    = TGA_RASTERFILE;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Lock the sister file
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // If there is already a LineOffsetTbl in the file, copy it to the structure.
    // Otherwise, construct it from the file.
    if (m_pLineOffsetTbl[1] == 0x00)
        {
        if ((pTgaFile->m_pTgaExtTableArea != 0) && (pTgaFile->m_pTgaExtTableArea->m_pScanLineTable != 0))
            {
            memcpy (m_pLineOffsetTbl,
                    pTgaFile->m_pTgaExtTableArea->m_pScanLineTable,
                    (uint32_t)GetResolutionDescriptor()->GetHeight() * sizeof (uint32_t));
            m_pLineOffsetTbl[(uint32_t)GetResolutionDescriptor()->GetHeight()] = pTgaFile->m_RasterDataEndOffset;
            }
        else
            {
            if (!GetLineOffsetTableFromFile())
                goto WRAPUP; // H_ERROR;
            }
        }

    // If the lines are write from bottom to up, we convert the index of
    // the line to read from the end of the file.
    if ((pTgaFile->m_pTgaFileHeader->m_ImageDescriptor & 0x20) == 0)
        Line = (uint32_t)GetResolutionDescriptor()->GetHeight() - pi_PosBlockY - 1;
    else
        Line = pi_PosBlockY;

    // Set the current codec to the Packet.
    po_rpPacket->SetCodec ((HFCPtr<HCDCodec> &) m_pCodec);

    // Put the disired line into the buffer for decompression
    if (pTgaFile->m_pTgaFile->GetCurrentPos() != m_pLineOffsetTbl[Line])
        pTgaFile->m_pTgaFile->SeekToPos(m_pLineOffsetTbl[Line]);

    HASSERT(m_pLineOffsetTbl[Line] <= m_pLineOffsetTbl[Line+1]);
    NbBytesRead = m_pLineOffsetTbl[Line+1] - m_pLineOffsetTbl[Line];

    if (po_rpPacket->GetBufferSize() == 0)
        {
        Byte* pReturnBytes = new Byte[NbBytesRead];

        if (pTgaFile->m_pTgaFile->Read(pReturnBytes, NbBytesRead) != NbBytesRead)
            goto WRAPUP;    // H_ERROR

        po_rpPacket->SetBuffer (pReturnBytes, NbBytesRead);
        po_rpPacket->SetBufferOwnership (true);
        po_rpPacket->SetDataSize (NbBytesRead);
        }
    else
        {
        HASSERT (NbBytesRead <= po_rpPacket->GetBufferSize());

        if (pTgaFile->m_pTgaFile->Read(po_rpPacket->GetBufferAddress(), NbBytesRead) != NbBytesRead)
            goto WRAPUP;    // H_ERROR;

        po_rpPacket->SetDataSize (NbBytesRead);
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

/**-----------------------------------------------------------------------------
 Writes an uncompressed block of pixels on this resolution.
 The block position must be specified and can be computed by the resolution
 descripor.
 The value of @i{pi_pData} must be set by the user of this function.
 The size of the @i{pi_pData} buffer must be > ${#GetBlockSizeInByte()}.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param pi_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the writing operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaCompressLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                             uint64_t                 pi_PosBlockY,
                                             const Byte*              pi_pData,
                                             HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_PosBlockY < GetResolutionDescriptor()->GetHeight());
    HPRECONDITION(pi_PosBlockX == 0);

    HFCPtr<HRFResolutionDescriptor> pResolution = GetResolutionDescriptor();

    // Create a packet with the data to compress and write into the file
    HCDPacket Uncompressed((Byte*)pi_pData,
                           pResolution->GetBlockSizeInBytes(),
                           pResolution->GetBlockSizeInBytes());

    size_t MaxSubsetCompressed = m_pCodec->GetSubsetMaxCompressedSize();

    // Create a compression packet
    HFCPtr<HCDPacket> pCompressed(new HCDPacket((HFCPtr<HCDCodec>&)m_pCodec, new Byte[MaxSubsetCompressed], MaxSubsetCompressed));

    pCompressed->SetBufferOwnership(true);

    // Compress the data
    Uncompressed.Compress(pCompressed);

    // Write the compressed data to the file.
    return WriteBlock(pi_PosBlockX, pi_PosBlockY, pCompressed, pi_pSisterFileLock);
    }

/**-----------------------------------------------------------------------------
 Writes an uncompressed block of pixels on this resolution.
 The block position must be specified and can be computed by the resolution
 descripor.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param pi_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the writing operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaCompressLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                             uint64_t                 pi_PosBlockY,
                                             const HFCPtr<HCDPacket>& pi_rpPacket,
                                             HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_rpPacket != 0);
    HPRECONDITION(pi_PosBlockX == 0);
    HPRECONDITION(pi_PosBlockY < GetResolutionDescriptor()->GetHeight());

    HSTATUS             Status      = H_ERROR;
    HFCPtr<HRFTgaFile>  pTgaFile   = TGA_RASTERFILE;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Rewrite the file header if necessary...
    if (pi_PosBlockY == 0)
        {
        // Reset header if necessary.
        pTgaFile->SetFileHeaderToFile();

        m_pLineOffsetTbl[pi_PosBlockY] = pTgaFile->GetRasterDataOffset();

        if ((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) ||
            (pTgaFile->m_pTgaFile->GetCurrentPos() != pTgaFile->GetRasterDataOffset()))
            pTgaFile->m_pTgaFile->SeekToPos(pTgaFile->GetRasterDataOffset());
        }

    // Position the pointer at the right place into the file.
    pTgaFile->m_pTgaFile->SeekToPos(m_pLineOffsetTbl[pi_PosBlockY]);

    // Write the packet in the physical file.
    size_t DataSize = pi_rpPacket->GetDataSize();
    if (pTgaFile->m_pTgaFile->Write (pi_rpPacket->GetBufferAddress(), DataSize) != DataSize)
        goto WRAPUP;    // H_ERROR

    m_pLineOffsetTbl[pi_PosBlockY+1] = (uint32_t)pTgaFile->m_pTgaFile->GetCurrentPos();

    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    // If this is the last block, get the offset of the end of the data.
    if (pi_PosBlockY == GetResolutionDescriptor()->GetHeight() - 1)
        {
        if ((pTgaFile->m_pTgaExtTableArea != 0) && (pTgaFile->m_pTgaExtTableArea->m_pScanLineTable != 0))
            {
            if (pTgaFile->m_pTgaExtTableArea->m_pScanLineTable == 0)
                pTgaFile->m_pTgaExtTableArea->m_pScanLineTable = new uint32_t[(uint32_t)GetResolutionDescriptor()->GetHeight()];

            pTgaFile->m_RasterDataEndOffset = (uint32_t)pTgaFile->m_pTgaFile->GetCurrentPos();
            memcpy (pTgaFile->m_pTgaExtTableArea->m_pScanLineTable,
                    m_pLineOffsetTbl,
                    (uint32_t)GetResolutionDescriptor()->GetHeight() * sizeof (uint32_t));
            }
        }

    Status = H_SUCCESS;

WRAPUP:

    return Status;
    }

/**----------------------------------------------------------------------------
 Private
 Construct the LineOffsetTable from the tga file.
-----------------------------------------------------------------------------*/
bool HRFTgaCompressLineEditor::GetLineOffsetTableFromFile()
    {
    HPRECONDITION (GetRasterFile()->SharingControlIsLocked());

    HFCPtr<HRFTgaFile> pTgaFile = TGA_RASTERFILE;
    uint32_t Pos;
    uint32_t NbPixels;
    uint32_t SizeToProcess;
    uint32_t Index            = 0;
    uint32_t PosFromLastBlock = pTgaFile->GetRasterDataOffset();
    uint32_t SizeOfData       = pTgaFile->GetRasterDataEndOffset() - pTgaFile->GetRasterDataOffset();
    Byte  BytesPerPixel    = (pTgaFile->m_pTgaFileHeader->m_PixelDepth + 7) / 8;
    Byte* pBlockOfData;
    bool   FileIsValid = true;

    if (SizeOfData > BLOCKSIZE)
        pBlockOfData = new Byte [BLOCKSIZE];
    else
        pBlockOfData = new Byte [SizeOfData];

    m_pLineOffsetTbl[0] = pTgaFile->GetRasterDataOffset();

    // Process all the data;
    while (FileIsValid && PosFromLastBlock < pTgaFile->GetRasterDataEndOffset() && Index < GetResolutionDescriptor()->GetHeight())
        {
        SizeToProcess = pTgaFile->GetRasterDataEndOffset() - PosFromLastBlock;
        SizeToProcess = SizeToProcess < BLOCKSIZE ? SizeToProcess : BLOCKSIZE;
        pTgaFile->m_pTgaFile->SeekToPos (PosFromLastBlock);
        pTgaFile->m_pTgaFile->Read (pBlockOfData, SizeToProcess);
        Pos = 0;

        // Process a block
        while (FileIsValid && Pos < SizeToProcess)
            {
            NbPixels = 0;
            // Process a line
            while ((NbPixels < GetResolutionDescriptor()->GetWidth()) && (Pos < SizeToProcess))
                {
                NbPixels += ((pBlockOfData[Pos] & 0x7f) + 1);
                if (pBlockOfData[Pos] & 0x80)
                    Pos += BytesPerPixel + 1;
                else
                    Pos += (BytesPerPixel * ((pBlockOfData[Pos] & 0x7f) + 1)) + 1;
                }
            if (NbPixels == GetResolutionDescriptor()->GetWidth())
                m_pLineOffsetTbl[++Index] = PosFromLastBlock + Pos;
            else if (NbPixels > GetResolutionDescriptor()->GetWidth())
                FileIsValid = false;
            }
        PosFromLastBlock = m_pLineOffsetTbl[Index];
        }

    delete[] pBlockOfData;

    // Invalidate line table contents, they are not useful
    if (!FileIsValid)
        {
        m_pLineOffsetTbl[1] = 0x00;
        }

    return FileIsValid;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFTgaCompressLineEditor::OnSynchronizedSharingControl()
    {
    (TGA_RASTERFILE)->SaveTgaFile(true);
    (TGA_RASTERFILE)->Open();
    // Synch used to be called in this order. Does not look like it matter right now.
    // GetRasterFile()->SharingControlSynchronize();
    m_pLineOffsetTbl[1] = 0x00;
    }