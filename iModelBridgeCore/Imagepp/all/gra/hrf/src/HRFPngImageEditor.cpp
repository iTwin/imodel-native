//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPngImageEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPngImageEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFPngImageEditor.h>
#include <Imagepp/all/h/HRFPngFile.h>
#include <png/png.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>


#define PNG_RASTERFILE  static_cast<HRFPngFile*>(GetRasterFile().GetPtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFPngImageEditor::HRFPngImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     unsigned short       pi_Resolution,
                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    // Set the number of pass need to read a line.
    png_set_interlace_handling(PNG_RASTERFILE->m_pPngFileStruct);

    m_BytesByRow  = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFPngImageEditor::~HRFPngImageEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngImageEditor::ReadBlock(uint64_t pi_PosBlockX,
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

        // Restart the file at the beginning when the ask for a before block
        if ((PNG_RASTERFILE->m_InterlaceFileReaded))
            {
            PNG_RASTERFILE->SavePngFile(true);
            PNG_RASTERFILE->Open();

            // Set the number of pass need to read a line.
            png_set_interlace_handling(PNG_RASTERFILE->m_pPngFileStruct);

            m_BytesByRow  = ((uint32_t)GetResolutionDescriptor()->GetWidth() *
                             GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits()) / 8;

            //  The entire image is not read
            PNG_RASTERFILE->m_InterlaceFileReaded = false;
            }

        // Create the lines pointer array that will contains the image and connect each
        // line pointer on the data that it represent.
        HArrayAutoPtr<Byte*> ppLines(new Byte*[(uint32_t)GetResolutionDescriptor()->GetHeight()]);

        // Set the pointer lines in the output buffer
        for (uint32_t line = 0; line < (uint32_t)GetResolutionDescriptor()->GetHeight(); line++)
            ppLines[line] = po_pData + (line * m_BytesByRow);

        png_read_image(PNG_RASTERFILE->m_pPngFileStruct, ppLines);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        //  The entire image is read
        PNG_RASTERFILE->m_InterlaceFileReaded = true;
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
HSTATUS HRFPngImageEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte*  pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_SUCCESS;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Restart the file at the begining when the ask for a before block
    if ((PNG_RASTERFILE->m_pPngFileStruct->row_number > pi_PosBlockY) ||
        ((!GetRasterFile()->GetAccessMode().m_HasCreateAccess) && (pi_PosBlockY == 0)))
        {
        PNG_RASTERFILE->SavePngFile(true);
        PNG_RASTERFILE->Create();
        PNG_RASTERFILE->AssignStructTo(PNG_RASTERFILE->GetPageDescriptor(0));

        // Set the number of pass need to read a line.
        png_set_interlace_handling(PNG_RASTERFILE->m_pPngFileStruct);

        m_BytesByRow  = ((uint32_t)GetResolutionDescriptor()->GetWidth() *
                         GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits()) / 8;
        }

    // Create the lines pointer array that will contains the image and connect each
    // line pointer on the data that it represent.
    HArrayAutoPtr<Byte*> ppLines(new Byte*[(uint32_t)GetResolutionDescriptor()->GetHeight()]);

    // Set the pointer lines in buffer
    for (uint32_t line = 0; line < GetResolutionDescriptor()->GetHeight(); line++)
        ppLines[line] = (Byte*)pi_pData + (line * m_BytesByRow);

    png_write_image(PNG_RASTERFILE->m_pPngFileStruct, ppLines);

    // Increment the counters.
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
HSTATUS HRFPngImageEditor::ReadBlock(uint64_t           pi_PosBlockX,
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
HSTATUS HRFPngImageEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                      uint64_t                 pi_PosBlockY,
                                      const HFCPtr<HCDPacket>& pi_rpPacket,
                                      HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    // We decompress the data
    HCDPacket Uncompressed(new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()], m_pResolutionDescriptor->GetBlockSizeInBytes());
    Uncompressed.SetBufferOwnership(true);

    pi_rpPacket->Decompress(&Uncompressed);

    WriteBlock(pi_PosBlockX, pi_PosBlockY, Uncompressed.GetBufferAddress(), pi_pSisterFileLock);

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFPngImageEditor::OnSynchronizedSharingControl()
    {
    PNG_RASTERFILE->SavePngFile(true);
    PNG_RASTERFILE->Open();

    // Set the number of pass need to read a line.
    png_set_interlace_handling(PNG_RASTERFILE->m_pPngFileStruct);

    m_BytesByRow  = ((uint32_t)GetResolutionDescriptor()->GetWidth() *
                     GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits()) / 8;

    //  The entire image is not read
    PNG_RASTERFILE->m_InterlaceFileReaded = false;

    // If the file has been modified by another process, the counters will not
    // be equal and we will need to synchronize it. It also indicates that we
    // should refresh the file info structure and restart the reading operation
    // of the data. This has been done by the close-open sequence.
    }