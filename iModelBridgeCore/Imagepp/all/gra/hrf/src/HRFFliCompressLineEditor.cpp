//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFFliCompressLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPngLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFFliCompressLineEditor.h>
#include <Imagepp/all/h/HRFFliFile.h>
#include <Imagepp/all/h/HCDCodecFLIRLE8.h>


#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>



//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFFliCompressLineEditor::HRFFliCompressLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                   uint32_t              pi_Page,
                                                   unsigned short       pi_Resolution,
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
                                            HFCPtr<HCDPacket>&    po_rpPacket,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {


    HPRECONDITION(po_rpPacket != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    HFCLockMonitor SisterFileLock;
    Byte*    pCompressBuffer = 0;
    uint32_t CompressDateSize = 0;

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

    // Read the data from the file.
    if(m_pRasterFile->m_pFliFile->GetCurrentPos() != m_PosInFile)
        m_pRasterFile->m_pFliFile->SeekToPos(m_PosInFile);

    CompressDateSize = (uint32_t)(m_pRasterFile->GetChunkSize(1) - m_pRasterFile->GetChunkHeaderSize());

    pCompressBuffer   = new Byte[CompressDateSize];

    if(m_pRasterFile->m_pFliFile->Read(pCompressBuffer, CompressDateSize) != CompressDateSize)
        goto WRAPUP;    // H_ERROR

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

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
                                             const Byte*           pi_pData,
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

    // Verify that the line number specified is the same as the next line
    // to be written, since PNG file don't have random access.
    Byte*  ppLine[1];
    ppLine[0] = (Byte*)pi_pData;

    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }



//-----------------------------------------------------------------------------
// Public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFFliCompressLineEditor::WriteBlock(uint64_t                 pi_PosBlockX,
                                             uint64_t                 pi_PosBlockY,
                                             const HFCPtr<HCDPacket>& pi_rpPacket,
                                             HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);

    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// public
// OnSynchronizedSharingControl
//-----------------------------------------------------------------------------
void HRFFliCompressLineEditor::OnSynchronizedSharingControl()
    {
    // The close-open sequence will reload the info of the file. This way, we
    // are sure that the information that will be read will be up to date.
    static_cast<HRFFliFile*>(GetRasterFile().GetPtr())->SaveFliFile(true);
    static_cast<HRFFliFile*>(GetRasterFile().GetPtr())->Open();
    }


