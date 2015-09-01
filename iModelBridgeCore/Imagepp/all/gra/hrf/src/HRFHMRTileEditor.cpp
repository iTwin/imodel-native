//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFHMRTileEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFHMRTileEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HTIFFFile.h>
#include <Imagepp/all/h/HRFHMRFile.h>
#include <Imagepp/all/h/HRFHMRTileEditor.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDPacket.h>

#define RASTERFILEPTR       (static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->GetFilePtr())
#define HMR_RASTERFILE      (static_cast<HRFHMRFile*>(GetRasterFile().GetPtr()))


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFHMRTileEditor::HRFHMRTileEditor( HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                    uint32_t              pi_Page,
                                    unsigned short       pi_Resolution,
                                    HFCAccessMode         pi_AccessMode,
                                    uint32_t              pi_PaddingLines)
    : HRFTiffTileEditor  (pi_rpRasterFile, pi_Page, pi_Resolution, pi_AccessMode)
    {
    HPRECONDITION(pi_PaddingLines > 0);

    m_PaddingLines = pi_PaddingLines;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFHMRTileEditor::~HRFHMRTileEditor()
    {
    }



//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFHMRTileEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte*  po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Will reload directories if necessary
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Select the page and the resolution
    static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // Read the first tile
    Status = RASTERFILEPTR->TileRead(HMR_RASTERFILE->m_pTileBuffer, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);

    if (Status == H_SUCCESS)
        {
        // Number of bytes to copy from the first tile.
        uint32_t Lenght        = ((m_pResolutionDescriptor->GetBlockHeight() - m_PaddingLines) *
                                m_pResolutionDescriptor->GetBlockWidth() *
                                HMR_RASTERFILE->m_BytesByPixel) / HMR_RASTERFILE->m_Pixel1_4Bits;

        // Number of bytes include in the padding lines.
        uint32_t LenghtPadding = (m_PaddingLines *
                                m_pResolutionDescriptor->GetBlockWidth() *
                                HMR_RASTERFILE->m_BytesByPixel) / HMR_RASTERFILE->m_Pixel1_4Bits;

        // Copy the last part of the tile. (skip padding)
        memcpy (po_pData, &(HMR_RASTERFILE->m_pTileBuffer[LenghtPadding]), Lenght);

        // Complete the tile with the next if present.
        if (pi_PosBlockY+m_pResolutionDescriptor->GetBlockHeight() < (m_pResolutionDescriptor->GetHeight()+m_PaddingLines))
            {
            Status = RASTERFILEPTR->TileRead(HMR_RASTERFILE->m_pTileBuffer,
                                             (uint32_t)pi_PosBlockX,
                                             (uint32_t)pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight());

            // Unlock the sister file
            SisterFileLock.ReleaseKey();

            if (Status == H_SUCCESS)
                memcpy (&(po_pData[Lenght]), HMR_RASTERFILE->m_pTileBuffer, LenghtPadding);
            }
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFHMRTileEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                    uint64_t           pi_PosBlockY,
                                    HFCPtr<HCDPacket>& po_rpPacket,
                                    HFCLockMonitor const* pi_pSisterFileLock)

    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (po_rpPacket != 0);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Will reload directories if necessary
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Select the page and the resolution
    static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // we take for granted that there is no compression
    // we get the data uncompressed ans associate to it an identity codec
    uint32_t BufferSize = (m_pResolutionDescriptor->GetBlockWidth() *
                         m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() + 7) / 8 *
                        m_pResolutionDescriptor->GetBlockHeight();

    // test if there is a buffer already defined
    if(po_rpPacket->GetBufferSize() == 0)
        {
        // if not, create a buffer
        po_rpPacket->SetBuffer(new Byte[BufferSize], BufferSize);
        po_rpPacket->SetBufferOwnership(true);
        }

    po_rpPacket->SetDataSize(BufferSize);
    po_rpPacket->SetCodec(new HCDCodecIdentity(BufferSize));

    return ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket->GetBufferAddress(), pi_pSisterFileLock);
    }



//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFHMRTileEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);

    HSTATUS   Status = H_SUCCESS;

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock but do not synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Select the page and the resolution
    static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // Read the tiles and update it
    Status = RASTERFILEPTR->TileRead(HMR_RASTERFILE->m_pTileBuffer, (uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY);


    if ((Status == H_SUCCESS) || (Status == H_DATA_NOT_AVAILABLE))
        {
        // Number of bytes to copy from the first tile.
        uint32_t Lenght        = ((m_pResolutionDescriptor->GetBlockHeight() - m_PaddingLines) *
                                m_pResolutionDescriptor->GetBlockWidth() *
                                HMR_RASTERFILE->m_BytesByPixel) / HMR_RASTERFILE->m_Pixel1_4Bits;

        // Number of bytes include in the padding lines.
        uint32_t LenghtPadding = (m_PaddingLines *
                                m_pResolutionDescriptor->GetBlockWidth() *
                                HMR_RASTERFILE->m_BytesByPixel) / HMR_RASTERFILE->m_Pixel1_4Bits;

        // Clear uninitialized padding.
        if(H_DATA_NOT_AVAILABLE == Status && LenghtPadding)
            memset (HMR_RASTERFILE->m_pTileBuffer, 0, LenghtPadding);
            
        // Copy the last part of the tile. (skip padding)
        memcpy (&(HMR_RASTERFILE->m_pTileBuffer[LenghtPadding]), pi_pData, Lenght);

        Status = RASTERFILEPTR->TileWrite(HMR_RASTERFILE->m_pTileBuffer,
                                          (uint32_t)pi_PosBlockX,
                                          (uint32_t)pi_PosBlockY);

        // Complete the tile with the next if present.
        if (Status == H_SUCCESS)
            {
            // Complete the tile with the next if present.
            if (pi_PosBlockY+m_pResolutionDescriptor->GetBlockHeight() < m_pResolutionDescriptor->GetHeight())
                {
                Status = RASTERFILEPTR->TileRead(HMR_RASTERFILE->m_pTileBuffer,
                                                 (uint32_t)pi_PosBlockX,
                                                 (uint32_t)pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight());

                if ((Status == H_SUCCESS) || (Status == H_DATA_NOT_AVAILABLE))
                    {
                    // Clear uninitialized padding.
                    if(H_DATA_NOT_AVAILABLE == Status)
                        {
                        size_t tileSize = (m_pResolutionDescriptor->GetBlockWidth() * m_pResolutionDescriptor->GetBlockHeight() * HMR_RASTERFILE->m_BytesByPixel) / HMR_RASTERFILE->m_Pixel1_4Bits;
                        memset(HMR_RASTERFILE->m_pTileBuffer + LenghtPadding, 0, tileSize-LenghtPadding);
                        }

                    memcpy (HMR_RASTERFILE->m_pTileBuffer, &(pi_pData[Lenght]), LenghtPadding);

                    Status = RASTERFILEPTR->TileWrite(HMR_RASTERFILE->m_pTileBuffer,
                                                      (uint32_t)pi_PosBlockX,
                                                      (uint32_t)pi_PosBlockY + m_pResolutionDescriptor->GetBlockHeight());
                    }
                }
            }
        }

    // Update the directories.
    GetRasterFile()->SharingControlIncrementCount();
    static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->SaveDescriptors();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFHMRTileEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                     uint64_t                 pi_PosBlockY,
                                     const HFCPtr<HCDPacket>& pi_rpPacket,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (pi_rpPacket != 0);
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetBlockType() == HRFBlockType::TILE);
    HSTATUS Status = H_SUCCESS;

    // Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock but do not synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Select the page and the resolution
    static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->SetImageInSubImage (m_IndexOfPage+m_Resolution);

    // If the codec is not IDENTITY, decompress the data in order to decompose
    // the tile via the uncompressed WriteBlock
    if (pi_rpPacket->GetCodec()->GetClassID() != HCDCodecIdentity::CLASS_ID)
        {
        // Prepare the uncompressed packet
        size_t UncompressedSize = (m_pResolutionDescriptor->GetBlockWidth() *
                                   m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() + 7) / 8 *
                                   m_pResolutionDescriptor->GetBlockHeight();

        HCDPacket Uncompressed(new Byte[UncompressedSize], UncompressedSize);
        Uncompressed.SetBufferOwnership(true);

        // Decompress the data
        ((HCDPacket*)pi_rpPacket)->Decompress(&Uncompressed);

        // Write the uncompressed data
        Status = WriteBlock(pi_PosBlockX, pi_PosBlockY, Uncompressed.GetBufferAddress(), pi_pSisterFileLock);
        }
    else
        Status = WriteBlock(pi_PosBlockX, pi_PosBlockY, pi_rpPacket->GetBufferAddress(), pi_pSisterFileLock);

    // Update the directories.
    GetRasterFile()->SharingControlIncrementCount();
    static_cast<HRFHMRFile*>(GetRasterFile().GetPtr())->SaveDescriptors();

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return Status;
    }