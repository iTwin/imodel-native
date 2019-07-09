//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPngImageEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFPngImageEditor.h>
#include <ImagePP/all/h/HRFPngFile.h>
#include <png/png.h>
#include <png/pngstruct.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>


#define PNG_RASTERFILE  static_cast<HRFPngFile*>(GetRasterFile().GetPtr())

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFPngImageEditor::HRFPngImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                     uint32_t              pi_Page,
                                     uint16_t       pi_Resolution,
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
                                       Byte*  po_pData)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_SUCCESS;

    // Temporary need for virtual ptr.
    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
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
                                        const Byte*  pi_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_SUCCESS;

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

    return Status;
    }

//-----------------------------------------------------------------------------
// Public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngImageEditor::ReadBlock(uint64_t           pi_PosBlockX,
                                     uint64_t           pi_PosBlockY,
                                     HFCPtr<HCDPacket>& po_rpPacket)
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

    return ReadBlock(pi_PosBlockX, pi_PosBlockY, po_rpPacket->GetBufferAddress());
    }

//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFPngImageEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                      uint64_t                 pi_PosBlockY,
                                      const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    // We decompress the data
    HCDPacket Uncompressed(new Byte[m_pResolutionDescriptor->GetBlockSizeInBytes()], m_pResolutionDescriptor->GetBlockSizeInBytes());
    Uncompressed.SetBufferOwnership(true);

    pi_rpPacket->Decompress(&Uncompressed);

    WriteBlock(pi_PosBlockX, pi_PosBlockY, Uncompressed.GetBufferAddress());

    return Status;
    }
