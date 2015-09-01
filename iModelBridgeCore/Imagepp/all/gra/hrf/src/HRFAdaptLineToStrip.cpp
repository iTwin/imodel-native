//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptLineToStrip.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptLineToStrip
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFAdaptLineToStrip.h>
#include <Imagepp/all/h/HFCAccessMode.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HCDCodecImage.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

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
                                                    unsigned short       pi_Resolution,
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
                                         unsigned short                pi_Resolution,
                                         HFCAccessMode                  pi_AccessMode)
    : HRFBlockAdapter(  pi_pCapabilities,
                        pi_rpRasterFile,
                        pi_Page,
                        pi_Resolution,
                        pi_AccessMode)
    {
    // Resolution dimension
    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);
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
                                       Byte* po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
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

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    uint32_t NoLine=0;

    // Load the block from file to the client data out
    for (; (NoLine < NumberOfLines) && (Status == H_SUCCESS); NoLine++)
        {
        // adapt the line to the blocks
        Status = m_pAdaptedResolutionEditor->ReadBlock(0, pi_PosBlockY+NoLine, pPosInBlock, pi_pSisterFileLock);
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
                                          HFCPtr<HCDPacketRLE>& po_rpPacketRLE,
                                          HFCLockMonitor const* pi_pSisterFileLock)
    {
    RESOLUTION_EDITOR_NOT_64_BITS_READY

    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffer.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);
    HPRECONDITION(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);

    HSTATUS Status       = H_SUCCESS;
    uint32_t ImageWidth    = (uint32_t)m_pResolutionDescriptor->GetWidth();
    uint32_t NumberOfLines = m_BlockHeight;

    // Adjust if necessary the NumberOfLines to the resolution height
    if (pi_PosBlockY+m_BlockHeight > m_Height)
        NumberOfLines = m_Height - (uint32_t)pi_PosBlockY;

    size_t               workBufferSize = (ImageWidth* 2 + 2)*sizeof(unsigned short);     // Worst case for one line.
    HFCPtr<HCDPacketRLE> pLinePacket(new HCDPacketRLE(ImageWidth, 1));
    pLinePacket->SetLineBuffer(0, new Byte[workBufferSize], workBufferSize, 0);
    pLinePacket->SetBufferOwnership(true);

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Read these lines from file and copy their data to the output buffer
    for (uint32_t NoLine=0; NoLine < NumberOfLines; ++NoLine)
        {
        if(H_SUCCESS != (Status = m_pAdaptedResolutionEditor->ReadBlockRLE(0, pi_PosBlockY+NoLine, pLinePacket, pi_pSisterFileLock)))
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
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToStrip::WriteBlock(uint64_t     pi_PosBlockX,
                                        uint64_t     pi_PosBlockY,
                                        const Byte* pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
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

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Write the client data to the file
    for (uint32_t NoLine=0; (NoLine < NumberOfLines) && (Status == H_SUCCESS); NoLine++)
        {
        // Write the line
        Status = m_pAdaptedResolutionEditor->WriteBlock(0, pi_PosBlockY+NoLine, pPosInBlock, pi_pSisterFileLock);
        // Move inside the buffer to the current line
        pPosInBlock += m_ExactBytesPerWidth;
        }
    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptLineToStrip::WriteBlockRLE(uint64_t              pi_PosBlockX,
                                           uint64_t              pi_PosBlockY,
                                           HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                           HFCLockMonitor const* pi_pSisterFileLock)
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

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Create a packet that contains only the current line.
    HFCPtr<HCDPacketRLE> pOneLinePacketRLE(new HCDPacketRLE((uint32_t)m_pResolutionDescriptor->GetWidth(), 1));
    pOneLinePacketRLE->SetBufferOwnership(false);

    for (uint32_t NoLine=0; NoLine < NumberOfLines && (Status == H_SUCCESS); ++NoLine)
        {
        // Fill packet with current line
        pOneLinePacketRLE->SetLineBuffer(0, pi_rpPacketRLE->GetLineBuffer(NoLine), pi_rpPacketRLE->GetLineBufferSize(NoLine), pi_rpPacketRLE->GetLineDataSize(NoLine));

        Status = m_pAdaptedResolutionEditor->WriteBlockRLE(0, pi_PosBlockY+NoLine, pOneLinePacketRLE, pi_pSisterFileLock);
        }

    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }

