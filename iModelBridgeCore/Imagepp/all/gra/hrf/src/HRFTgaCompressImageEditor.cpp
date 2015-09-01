//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTgaCompressImageEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTgaCompressImageEditor
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFTgaFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecTgaRLE.h>
#include <Imagepp/all/h/HRFTgaCompressImageEditor.h>


#define TGA_RASTERFILE  static_cast<HRFTgaFile*>(GetRasterFile().GetPtr())

/**-----------------------------------------------------------------------------
 Constructor of the class HRFTgaCompressImageEditor. This method initializes the intern
 attibutes for later use.

 @param pi_rpRasterFile A pointer to the associate raster file.
 @param pi_Page The number of the associate page descriptor.
 @param pi_Resolution The number of the associate resolution descriptor.
 @param pi_AccessMode The access and sharing modes for the file.
------------------------------------------------------------------------------*/
HRFTgaCompressImageEditor::HRFTgaCompressImageEditor(HFCPtr<HRFTgaFile>  pi_rpRasterFile,
                                                     uint32_t              pi_Page,
                                                     unsigned short       pi_Resolution,
                                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(static_cast<HFCPtr<HRFRasterFile> >(pi_rpRasterFile),
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    HFCPtr<HRFTgaFile> pTgaFile  = TGA_RASTERFILE;

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
HRFTgaCompressImageEditor::~HRFTgaCompressImageEditor()
    {
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.
 The @i{po_pData} must be allocated by the user of this function.
 The size must be > ${#GetBlockSizeInByte()}.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param po_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the read operation worked.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaCompressImageEditor::ReadBlock(uint64_t pi_PosBlockX,
                                             uint64_t pi_PosBlockY,
                                             Byte*    po_pData,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(pi_PosBlockY == 0);
    HPRECONDITION(pi_PosBlockX == 0);

    HSTATUS Status = H_SUCCESS;
    HFCPtr<HRFResolutionDescriptor> pResolution = GetResolutionDescriptor();
    HFCPtr<HRFTgaFile> pTgaFile = TGA_RASTERFILE;


    if (!(GetRasterFile()->GetAccessMode().m_HasCreateAccess))
        {
        // Create a packet to perform the decompression
        HFCPtr<HCDPacket> pCompressed = new HCDPacket;

        // Read the data into the compressed packet
        Status = ReadCompressedImage(pCompressed, pi_pSisterFileLock);

        if (Status == H_SUCCESS)
            {
            // Decompress the data
            HCDPacket Uncompressed(po_pData, GetResolutionDescriptor()->GetBlockSizeInBytes());
            pCompressed->Decompress(&Uncompressed);
            }

        pCompressed = 0;

        // Swap the lines if necessary
        if ((pTgaFile->m_pTgaFileHeader->m_ImageDescriptor & 0x20) == 0)
            {
            Byte BytesPerPixel = (Byte)((GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits() + 7) / 8);
            uint32_t ImageByteWidth = (uint32_t)GetResolutionDescriptor()->GetWidth() * BytesPerPixel;

            HAutoPtr<Byte> pTempLineData(new Byte[ImageByteWidth]);

            uint32_t TopLine = 0;
            uint32_t BottomLine = (uint32_t)GetResolutionDescriptor()->GetHeight() - 1;

            while (BottomLine > TopLine)
                {
                // Copy top line in temp buffer
                memcpy(pTempLineData,
                       po_pData + (TopLine * ImageByteWidth),
                       ImageByteWidth);

                // Copy bottom line into top line
                memcpy(po_pData + (TopLine * ImageByteWidth),
                       po_pData + (BottomLine * ImageByteWidth),
                       ImageByteWidth);

                // Copy temp buffer into bottom line
                memcpy(po_pData + (BottomLine * ImageByteWidth),
                       pTempLineData,
                       ImageByteWidth);

                ++TopLine;
                --BottomLine;
                }
            }
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

/**-----------------------------------------------------------------------------
 Read an uncompressed block of pixels on this resolution.

 @param pi_PosBlockX The X position of the block in the file.
 @param pi_PosBlockY The Y position of the block in the file.
 @param po_pData The buffer to be returned with the raster data.

 @return HSTATUS H_SUCCESS if the readint operation went right.
------------------------------------------------------------------------------*/
HSTATUS HRFTgaCompressImageEditor::ReadCompressedImage(HFCPtr<HCDPacket>& po_rpPacket,
                                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacket != 0);

    HSTATUS Status = H_ERROR;

    uint32_t          NbBytesRead;
    HFCPtr<HRFTgaFile> pTgaFile    = TGA_RASTERFILE;

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Set the current codec to the Packet.
    po_rpPacket->SetCodec ((HFCPtr<HCDCodec> &) m_pCodec);

    pTgaFile->m_pTgaFile->SeekToPos(pTgaFile->GetRasterDataOffset());

    NbBytesRead = pTgaFile->GetRasterDataEndOffset() - pTgaFile->GetRasterDataOffset();

    if (po_rpPacket->GetBufferSize() == 0)
        {
        Byte* pReturnBytes = new Byte[NbBytesRead];

        if(pTgaFile->m_pTgaFile->Read (pReturnBytes, NbBytesRead) != NbBytesRead)
            goto WRAPUP;    // H_ERROR

        po_rpPacket->SetBuffer (pReturnBytes, NbBytesRead);
        po_rpPacket->SetBufferOwnership (true);
        po_rpPacket->SetDataSize (NbBytesRead);
        }
    else
        {
        HASSERT (NbBytesRead <= po_rpPacket->GetBufferSize());

        if(pTgaFile->m_pTgaFile->Read (po_rpPacket->GetBufferAddress(), NbBytesRead) != NbBytesRead)
            goto WRAPUP;    // H_ERROR

        po_rpPacket->SetDataSize (NbBytesRead);
        }

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFTgaCompressImageEditor::OnSynchronizedSharingControl()
    {
    (TGA_RASTERFILE)->SaveTgaFile(true);
    (TGA_RASTERFILE)->Open();
    }
