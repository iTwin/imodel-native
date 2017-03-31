//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFFliCompressLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPngLineEditor
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRFFliCompressLineEditor.h>
#include <ImagePP/all/h/HRFFliFile.h>
#include <ImagePP/all/h/HCDCodecFLIRLE8.h>


#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDCodecIdentity.h>



//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFFliCompressLineEditor::HRFFliCompressLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   uint16_t       pi_Resolution,
                                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFFliFile*>(GetRasterFile().GetPtr());

    // Position should be the first valid data in file.
    m_PosInFile   = m_pRasterFile->m_OffsetToData;

    if(GetResolutionDescriptor()->GetBitsPerPixel() == 8)
        {
        m_pCodec = new HCDCodecFLIRLE8();
        m_pCodec->SetBitsPerPixel(8);
        }
    else
        {
        //TODO@philG
        }

    m_pCodec->SetDimensions((uint32_t)GetResolutionDescriptor()->GetWidth(),
                            (uint32_t)GetResolutionDescriptor()->GetHeight());

    m_pCodec->SetSubset((uint32_t)GetResolutionDescriptor()->GetWidth(),
                        (uint32_t)GetResolutionDescriptor()->GetHeight());


    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFFliCompressLineEditor::~HRFFliCompressLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliCompressLineEditor::ReadBlock(uint64_t              pi_PosBlockX,
                                            uint64_t              pi_PosBlockY,
                                            HFCPtr<HCDPacket>&    po_rpPacket)
    {


    HPRECONDITION(po_rpPacket != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    Byte*    pCompressBuffer = 0;
    uint32_t CompressDateSize = 0;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    // Read the data from the file.
    if(m_pRasterFile->m_pFliFile->GetCurrentPos() != m_PosInFile)
        m_pRasterFile->m_pFliFile->SeekToPos(m_PosInFile);

    CompressDateSize = (uint32_t)(m_pRasterFile->GetChunkSize(1) - m_pRasterFile->GetChunkHeaderSize());

    pCompressBuffer   = new Byte[CompressDateSize];

    if(m_pRasterFile->m_pFliFile->Read(pCompressBuffer, CompressDateSize) != CompressDateSize)
        goto WRAPUP;    // H_ERROR

    // Set the current codec to the Packet.
    po_rpPacket->SetCodec((HFCPtr<HCDCodec> &) m_pCodec);

    // Test if there is a buffer already defined
    if(po_rpPacket->GetBufferSize() == 0)
        {
        po_rpPacket->SetBuffer(pCompressBuffer, CompressDateSize);
        po_rpPacket->SetBufferOwnership(true);
        po_rpPacket->SetDataSize(CompressDateSize);
        }
    else
        {
        HASSERT(CompressDateSize <= po_rpPacket->GetBufferSize());
        po_rpPacket->SetDataSize(CompressDateSize);
        memcpy(po_rpPacket->GetBufferAddress(), pCompressBuffer, CompressDateSize);
        }


    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliCompressLineEditor::WriteBlock(uint64_t              pi_PosBlockX,
                                             uint64_t              pi_PosBlockY,
                                             const Byte*           pi_pData)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    // Verify that the line number specified is the same as the next line
    // to be written, since PNG file don't have random access.
    Byte*  ppLine[1];
    ppLine[0] = (Byte*)pi_pData;

    return H_SUCCESS;
    }



//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliCompressLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                             uint64_t                 pi_PosBlockY,
                                             const HFCPtr<HCDPacket>& pi_rpPacket)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);

    return H_ERROR;
    }


