//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBmpCompressImageEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFBmpCompressImageEditor
//---------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDCodecBMPRLE8.h>
#include <Imagepp/all/h/HCDCodecBMPRLE4.h>
#include <Imagepp/all/h/HRFBmpCompressImageEditor.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const unsigned short m_LinePadBits = 32;

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFBmpCompressImageEditor::HRFBmpCompressImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                     uint32_t              pi_Page,
                                                     unsigned short       pi_Resolution,
                                                     HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFBmpFile*>(GetRasterFile().GetPtr());

    // Position should be the first valid data in file.
    m_PosInFile   = m_pRasterFile->m_BmpFileHeader.m_OffBitsToData;

    if(GetResolutionDescriptor()->GetBitsPerPixel() == 4)
        {
        m_pCodec = new HCDCodecBMPRLE4();
        m_pCodec->SetBitsPerPixel(4);
        }
    else
        {
        m_pCodec = new HCDCodecBMPRLE8();
        m_pCodec->SetBitsPerPixel(8);
        }

    m_pCodec->SetDimensions((uint32_t)GetResolutionDescriptor()->GetWidth(),
                            (uint32_t)GetResolutionDescriptor()->GetHeight());

    m_pCodec->SetLinePaddingBits(m_pRasterFile->m_PaddingBitsPerRow);
    m_pCodec->SetSubset((uint32_t)GetResolutionDescriptor()->GetWidth(),
                        (uint32_t)GetResolutionDescriptor()->GetHeight());
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFBmpCompressImageEditor::~HRFBmpCompressImageEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFBmpCompressImageEditor::ReadBlock(uint64_t            pi_PosBlockX,
                                             uint64_t            pi_PosBlockY,
                                             HFCPtr<HCDPacket>&  po_rpPacket,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacket != 0);

    HSTATUS Status = H_ERROR;

    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Byte* pCompressBuffer = 0;
        uint32_t CompressDateSize = 0;

        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // Read the data from the file.
        if(m_pRasterFile->m_pBmpFile->GetCurrentPos() != m_PosInFile)
            m_pRasterFile->m_pBmpFile->SeekToPos(m_PosInFile);

        CompressDateSize = (uint32_t)(m_pRasterFile->m_pBmpFile->GetSize() -
                                    m_pRasterFile->m_pBmpFile->GetCurrentPos());

        pCompressBuffer   = new Byte[CompressDateSize];

        if(m_pRasterFile->m_pBmpFile->Read(pCompressBuffer, CompressDateSize) == CompressDateSize)
            {
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
            }
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
HSTATUS  HRFBmpCompressImageEditor::WriteBlock(uint64_t       pi_PosBlockX,
                                               uint64_t       pi_PosBlockY,
                                               const Byte*    pi_pData,
                                               HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetCodec() != 0);
    
    // We take for granted that there is compression
    // we compress the data
    HCDPacket Uncompressed((Byte*)pi_pData,
                           m_pResolutionDescriptor->GetBlockSizeInBytes(),
                           m_pResolutionDescriptor->GetBlockSizeInBytes());

    // Assign a default Codec
    HFCPtr<HCDCodec> pCodec = (HCDCodec*)m_pResolutionDescriptor->GetCodec()->Clone();

    HCDCodecImage::SetCodecForImage(pCodec,
                                    m_pResolutionDescriptor->GetBlockWidth(),
                                    m_pResolutionDescriptor->GetBlockHeight(),
                                    m_pResolutionDescriptor->GetBitsPerPixel(),
                                    32);  // Padding per width for BMP

    size_t MaxSubsetCompressed = pCodec->GetSubsetMaxCompressedSize();

    HFCPtr<HCDPacket> pCompressed(new HCDPacket(pCodec, new Byte[MaxSubsetCompressed], MaxSubsetCompressed));

    pCompressed->SetBufferOwnership(true);

    Uncompressed.Compress(pCompressed);

    return WriteBlock(pi_PosBlockX, pi_PosBlockY, pCompressed, pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFBmpCompressImageEditor::WriteBlock(uint64_t                  pi_PosBlockX,
                                              uint64_t                  pi_PosBlockY,
                                              const HFCPtr<HCDPacket>&  pi_rpPacket,
                                              HFCLockMonitor const*     pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_rpPacket != 0);

    HSTATUS Status = H_ERROR;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (pi_PosBlockY == 0)
        m_pRasterFile->m_pBmpFile->SeekToPos(m_pRasterFile->m_BmpFileHeader.m_OffBitsToData);

    HASSERT_X64(pi_rpPacket->GetDataSize() < ULONG_MAX);
    uint32_t DataSize = (uint32_t)pi_rpPacket->GetDataSize();

    if(m_pRasterFile->m_pBmpFile->Write(pi_rpPacket->GetBufferAddress(), DataSize) == DataSize)
        {

        GetRasterFile()->SharingControlIncrementCount();

        // Unlock the sister file
        SisterFileLock.ReleaseKey();
        Status = H_SUCCESS;
        }

    return Status;
    }