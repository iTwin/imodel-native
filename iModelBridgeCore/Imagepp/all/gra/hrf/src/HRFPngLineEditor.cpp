//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPngLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPngLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFPngLineEditor.h>
#include <Imagepp/all/h/HRFPngFile.h>
#include <png/png.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>

#define PNG_RASTERFILE  static_cast<HRFPngFile*>(GetRasterFile().GetPtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFPngLineEditor::HRFPngLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    if (pi_rpRasterFile->GetAccessMode().m_HasCreateAccess)
        m_FirstWriteInCreateMode = true;
    else
        m_FirstWriteInCreateMode = false;
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFPngLineEditor::~HRFPngLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte*  po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_SUCCESS;

    // Temporary need for virtual ptr.
    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // Restart the file at the begining when the ask for a before block
        if (PNG_RASTERFILE->m_pPngFileStruct->row_number > pi_PosBlockY)
            {
            PNG_RASTERFILE->SavePngFile(true);
            PNG_RASTERFILE->Open();
            }
        // Verify that the line number specified is the same as the next line to
        // be read because we don't have random access in a PNG file.

        if (pi_PosBlockY == PNG_RASTERFILE->m_pPngFileStruct->row_number)
            png_read_row(PNG_RASTERFILE->m_pPngFileStruct, po_pData, NULL);
        else
            {
            if (pi_PosBlockY > PNG_RASTERFILE->m_pPngFileStruct->row_number)
                {
                // We pass all line before the asked line
                while (PNG_RASTERFILE->m_pPngFileStruct->row_number <= pi_PosBlockY)
                    png_read_row(PNG_RASTERFILE->m_pPngFileStruct, po_pData, NULL);
                }
            else
                Status = H_ERROR;
            }

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    else
        Status = H_NOT_FOUND;


    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_SUCCESS;

    // Write up-to-date header (no choice to do that here if we have a palette)
    // (png needs header to to be written before blocks)
    if (m_FirstWriteInCreateMode)
        {
        PNG_RASTERFILE->AssignStructTo(PNG_RASTERFILE->GetPageDescriptor(0));
        m_FirstWriteInCreateMode = false;
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Restart the file at the beginning when the ask for a before block
    if ((PNG_RASTERFILE->m_pPngFileStruct->row_number > pi_PosBlockY) ||
        ((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0)))
        {
        PNG_RASTERFILE->SavePngFile(true);
        PNG_RASTERFILE->Create();
        PNG_RASTERFILE->AssignStructTo(PNG_RASTERFILE->GetPageDescriptor(0));
        }

    // Verify that the line number specified is the same as the next line
    // to be written, since PNG file don't have random access.
    Byte*  ppLine[1];
    ppLine[0] = (Byte*)pi_pData;

    // If some line are missing...
    if (pi_PosBlockY > PNG_RASTERFILE->m_pPngFileStruct->row_number)
        {
        uint32_t BlockSize  = m_pResolutionDescriptor->GetBlockSizeInBytes();
        Byte* pFakeLine;
        pFakeLine = new Byte[BlockSize];

        // Fill the fake line buffer using the default color.
        HFCPtr<HRPPixelType> CurrentPixelType = m_pResolutionDescriptor->GetPixelType();
        uint32_t BitPerPixel     = CurrentPixelType->CountPixelRawDataBits();
        uint32_t BytePerPixel    = (uint32_t)MAX(MIN((BitPerPixel + 7) / 8.0, 4), 1);
        uint32_t PixelCount      = BlockSize;
        uint32_t BackgroundColor = 0;

        if (BitPerPixel >= 8)
            {
            PixelCount = (BlockSize / BytePerPixel);
            memcpy(&BackgroundColor, CurrentPixelType->GetDefaultRawData(), BytePerPixel);
            }

        for(uint32_t PixelIndex = 0; PixelIndex < PixelCount; PixelIndex++)
            {
            memcpy(&pFakeLine[PixelIndex * BytePerPixel], &BackgroundColor, BytePerPixel);

            // Be sure we dont pass the end of the pFakeLine buffer.
            HASSERT((PixelIndex * BytePerPixel) + BytePerPixel <= BlockSize);
            }
        ppLine[0] = (Byte*)pFakeLine; ;

        do
            {
            png_write_rows(PNG_RASTERFILE->m_pPngFileStruct, ppLine, 1);

            }
        while (pi_PosBlockY > PNG_RASTERFILE->m_pPngFileStruct->row_number);
        delete []pFakeLine;
        }


    if (pi_PosBlockY == PNG_RASTERFILE->m_pPngFileStruct->row_number)
        {
        ppLine[0] = (Byte*)pi_pData;
        png_write_rows(PNG_RASTERFILE->m_pPngFileStruct, ppLine, 1);
        }
    else
        {
        HASSERT(false);
        Status = H_ERROR;
        }

    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngLineEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                    uint64_t           pi_PosBlockY,
                                    HFCPtr<HCDPacket>& po_rpPacket,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    // The compession is handel by the pnglib
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
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                     uint64_t                 pi_PosBlockY,
                                     const HFCPtr<HCDPacket>& pi_rpPacket,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    // We decompress the data
    HCDPacket Uncompressed(new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()],
                           m_pResolutionDescriptor->GetBlockSizeInBytes());
    Uncompressed.SetBufferOwnership(true);

    pi_rpPacket->Decompress(&Uncompressed);

    WriteBlock(pi_PosBlockX, pi_PosBlockY, Uncompressed.GetBufferAddress(), pi_pSisterFileLock);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFPngLineEditor::OnSynchronizedSharingControl()
    {
    // The close-open sequence will reload the info of the file. This way, we
    // are sure that the information that will be read will be up to date.
    PNG_RASTERFILE->SavePngFile(true);
    PNG_RASTERFILE->Open();
    }