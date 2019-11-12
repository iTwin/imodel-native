//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptLineToImage
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HFCAccessMode.h>

#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HRFAdaptLineToImage.h>

#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDCodecImage.h>
#include <ImagePP/all/h/HCDPacketRLE.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptLineToImageCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptLineToImageCapabilities::HRFAdaptLineToImageCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_LINE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_IMAGE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptLineToImageCreator)

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFAdaptLineToImageCreator::~HRFAdaptLineToImageCreator()
    {
    }

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptLineToImageCreator::GetCapabilities() const
    {
    return HRFAdaptLineToImageCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptLineToImageCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    uint16_t       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptLineToImage(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptLineToImage::HRFAdaptLineToImage(HRFBlockAdapterCapabilities*   pi_pCapabilities,
                                         HFCPtr<HRFRasterFile>          pi_rpRasterFile,
                                         uint32_t                       pi_Page,
                                         uint16_t                pi_Resolution,
                                         HFCAccessMode                  pi_AccessMode)
    : HRFBlockAdapter(pi_pCapabilities,
                      pi_rpRasterFile,
                      pi_Page,
                      pi_Resolution,
                      pi_AccessMode)
    {
    // Obtain the number of bytes per line
    m_ExactBytesPerImageWidth = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptLineToImage::~HRFAdaptLineToImage()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToImage::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte* po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status      = H_SUCCESS;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);

    uint32_t ImageHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Read these line from file and copy their data to the output buffer
    for (uint32_t NoLine=0; NoLine < ImageHeight && (Status == H_SUCCESS); NoLine++)
        Status = m_pAdaptedResolutionEditor->ReadBlock(0, NoLine, po_pData+(NoLine * m_ExactBytesPerImageWidth));

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToImage::ReadBlock(uint64_t             pi_PosBlockX,
                                       uint64_t             pi_PosBlockY,
                                       HFCPtr<HCDPacket>&   po_rpPacket)
    {
    return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToImage::ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                          uint64_t                 pi_PosBlockY,
                                          HFCPtr<HCDPacketRLE>&    po_rpPacketRLE)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= UINT32_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= UINT32_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= UINT32_MAX);

    HSTATUS Status      = H_SUCCESS;

    uint32_t ImageHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();
    uint32_t ImageWidth  = (uint32_t)m_pResolutionDescriptor->GetWidth();

    size_t               workBufferSize = (ImageWidth* 2 + 2)*sizeof(uint16_t);     // Worst case for one line.
    HFCPtr<HCDPacketRLE> pLinePacket(new HCDPacketRLE(ImageWidth, 1));
    pLinePacket->SetLineBuffer(0, new Byte[workBufferSize], workBufferSize, 0);
    pLinePacket->SetBufferOwnership(true);


    // Read these lines from file and copy their data to the output buffer
    for (uint32_t NoLine=0; NoLine < ImageHeight; ++NoLine)
        {
        if(H_SUCCESS != (Status = m_pAdaptedResolutionEditor->ReadBlockRLE(0, NoLine, pLinePacket)))
            break;  // We failed stop now!

        size_t LineDataSize = pLinePacket->GetLineDataSize(0);

        // Alloc buffer if it is not large enough.
        if(po_rpPacketRLE->GetLineBufferSize(NoLine) < LineDataSize)
            {
            HASSERT(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
            po_rpPacketRLE->SetLineBuffer(NoLine, new Byte[LineDataSize], LineDataSize, 0/*pi_DataSize*/);
            }

        // Copy from workBuffer to output packet.
        memcpy(po_rpPacketRLE->GetLineBuffer(NoLine), pLinePacket->GetLineBuffer(0), LineDataSize);
        po_rpPacketRLE->SetLineDataSize(NoLine, LineDataSize);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToImage::WriteBlock(uint64_t        pi_PosBlockX,
                                        uint64_t        pi_PosBlockY,
                                        const Byte*     pi_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess );
    HSTATUS Status      = H_SUCCESS;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);

    uint32_t  ImageHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Write these strip from file and copy their data to the output buffer
    for (uint32_t NoLine=0; NoLine < ImageHeight && (Status == H_SUCCESS); NoLine++)
        Status = m_pAdaptedResolutionEditor->WriteBlock(0, NoLine, pi_pData+(NoLine * m_ExactBytesPerImageWidth));

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToImage::WriteBlockRLE(uint64_t              pi_PosBlockX,
                                           uint64_t              pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess );
    HSTATUS Status      = H_SUCCESS;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);
    HASSERT(m_pResolutionDescriptor->GetWidth() <= UINT32_MAX);

    uint32_t  ImageHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Create a packet that will contains only the current line.
    HFCPtr<HCDPacketRLE> pOneLinePacketRLE(new HCDPacketRLE((uint32_t)m_pResolutionDescriptor->GetWidth(), 1));
    pOneLinePacketRLE->SetBufferOwnership(false);

    // Write these strip from file and copy their data to the output buffer
    for (uint32_t NoLine=0; NoLine < ImageHeight && (Status == H_SUCCESS); ++NoLine)
        {
        // Fill packet with current line
        pOneLinePacketRLE->SetLineBuffer(0, pi_rpPacketRLE->GetLineBuffer(NoLine), pi_rpPacketRLE->GetLineBufferSize(NoLine), pi_rpPacketRLE->GetLineDataSize(NoLine));

        Status = m_pAdaptedResolutionEditor->WriteBlockRLE(0, NoLine, pOneLinePacketRLE);
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToImage::WriteBlock(uint64_t                    pi_PosBlockX,
                                        uint64_t                    pi_PosBlockY,
                                        const HFCPtr<HCDPacket>&    pi_rpPacket)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_ERROR;

    return Status;
    }



