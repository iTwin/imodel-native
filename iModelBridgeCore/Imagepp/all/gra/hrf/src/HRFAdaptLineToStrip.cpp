//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptLineToStrip
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRFAdaptLineToStrip.h>
#include <ImagePP/all/h/HFCAccessMode.h>
#include <ImagePP/all/h/HRFRasterFile.h>
#include <ImagePP/all/h/HCDCodecImage.h>
#include <ImagePP/all/h/HCDPacket.h>
#include <ImagePP/all/h/HCDPacketRLE.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptLineToStripCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptLineToStripCapabilities::HRFAdaptLineToStripCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_LINE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_STRIP);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptLineToStripCreator)

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HRFAdaptLineToStripCreator::~HRFAdaptLineToStripCreator()
    {
    }

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptLineToStripCreator::GetCapabilities() const
    {
    return HRFAdaptLineToStripCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptLineToStripCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    uint16_t       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptLineToStrip(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptLineToStrip::HRFAdaptLineToStrip(HRFBlockAdapterCapabilities*   pi_pCapabilities,
                                         HFCPtr<HRFRasterFile>          pi_rpRasterFile,
                                         uint32_t                       pi_Page,
                                         uint16_t                pi_Resolution,
                                         HFCAccessMode                  pi_AccessMode)
    : HRFBlockAdapter(  pi_pCapabilities,
                        pi_rpRasterFile,
                        pi_Page,
                        pi_Resolution,
                        pi_AccessMode)
    {
    // Resolution dimension
    HASSERT(m_pResolutionDescriptor->GetHeight() <= UINT32_MAX);
    m_Height             = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // block dimension
    m_BlockHeight         = GetResolutionDescriptor()->GetBlockHeight();

    // Obtain the number of bytes per line Width
    m_ExactBytesPerWidth  = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptLineToStrip::~HRFAdaptLineToStrip()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToStrip::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte* po_pData)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    RESOLUTION_EDITOR_NOT_64_BITS_READY
    HSTATUS Status = H_SUCCESS;
    uint32_t NumberOfLines;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (pi_PosBlockY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - (uint32_t)pi_PosBlockY;
    else
        NumberOfLines = m_BlockHeight;

    Byte* pPosInBlock = po_pData;

    uint32_t NoLine=0;

    // Load the block from file to the client data out
    for (; (NoLine < NumberOfLines) && (Status == H_SUCCESS); NoLine++)
        {
        // adapt the line to the blocks
        Status = m_pAdaptedResolutionEditor->ReadBlock(0, pi_PosBlockY+NoLine, pPosInBlock);
        if (Status != H_SUCCESS)
            {
            break;
            }
        // Move inside the buffer to the current line
        pPosInBlock += m_ExactBytesPerWidth;
        }

    //If at least one line was read successfully, consider all
    //tiles intersecting the line valid.
    if ((Status != H_SUCCESS) && (NoLine > 0))
        {
        memset(pPosInBlock, 0, m_ExactBytesPerWidth * (NumberOfLines - NoLine));
        Status = H_SUCCESS;
        }

    return Status;
    }


//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToStrip::ReadBlockRLE(uint64_t pi_PosBlockX,
                                          uint64_t pi_PosBlockY,
                                          HFCPtr<HCDPacketRLE>& po_rpPacketRLE)
    {
    RESOLUTION_EDITOR_NOT_64_BITS_READY

    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= UINT32_MAX);
    HPRECONDITION(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth() <= UINT32_MAX);

    HSTATUS Status       = H_SUCCESS;
    uint32_t ImageWidth    = (uint32_t)m_pResolutionDescriptor->GetWidth();
    uint32_t NumberOfLines = m_BlockHeight;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (pi_PosBlockY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - (uint32_t)pi_PosBlockY;

    size_t               workBufferSize = (ImageWidth* 2 + 2)*sizeof(uint16_t);     // Worst case for one line.
    HFCPtr<HCDPacketRLE> pLinePacket(new HCDPacketRLE(ImageWidth, 1));
    pLinePacket->SetLineBuffer(0, new Byte[workBufferSize], workBufferSize, 0);
    pLinePacket->SetBufferOwnership(true);

    // Read these lines from file and copy their data to the output buffer
    for (uint32_t NoLine=0; NoLine < NumberOfLines; ++NoLine)
        {
        if(H_SUCCESS != (Status = m_pAdaptedResolutionEditor->ReadBlockRLE(0, pi_PosBlockY+NoLine, pLinePacket)))
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
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToStrip::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte* pi_pData)
    {
    RESOLUTION_EDITOR_NOT_64_BITS_READY

    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    uint32_t NumberOfLines;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (pi_PosBlockY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - (uint32_t)pi_PosBlockY;
    else
        NumberOfLines = m_BlockHeight;

    const Byte* pPosInBlock = pi_pData;

    // Write the client data to the file
    for (uint32_t NoLine=0; (NoLine < NumberOfLines) && (Status == H_SUCCESS); NoLine++)
        {
        // Write the line
        Status = m_pAdaptedResolutionEditor->WriteBlock(0, pi_PosBlockY+NoLine, pPosInBlock);
        // Move inside the buffer to the current line
        pPosInBlock += m_ExactBytesPerWidth;
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToStrip::WriteBlockRLE(uint64_t              pi_PosBlockX,
                                           uint64_t              pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE)
    {
    RESOLUTION_EDITOR_NOT_64_BITS_READY

    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    uint32_t NumberOfLines;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (pi_PosBlockY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - (uint32_t)pi_PosBlockY;
    else
        NumberOfLines = m_BlockHeight;

    // Create a packet that contains only the current line.
    HFCPtr<HCDPacketRLE> pOneLinePacketRLE(new HCDPacketRLE((uint32_t)m_pResolutionDescriptor->GetWidth(), 1));
    pOneLinePacketRLE->SetBufferOwnership(false);

    for (uint32_t NoLine=0; NoLine < NumberOfLines && (Status == H_SUCCESS); ++NoLine)
        {
        // Fill packet with current line
        pOneLinePacketRLE->SetLineBuffer(0, pi_rpPacketRLE->GetLineBuffer(NoLine), pi_rpPacketRLE->GetLineBufferSize(NoLine), pi_rpPacketRLE->GetLineDataSize(NoLine));

        Status = m_pAdaptedResolutionEditor->WriteBlockRLE(0, pi_PosBlockY+NoLine, pOneLinePacketRLE);
        }

    return Status;
    }

