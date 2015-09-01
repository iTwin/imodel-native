//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptStripToNStrip.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptStripToNStrip
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptStripToNStrip.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

//-----------------------------------------------------------------------------
// Utility class no turn off buffer ownership.
//-----------------------------------------------------------------------------
struct AutoRestoreBufferOwnership
    {
    AutoRestoreBufferOwnership(HCDPacketRLE* pRLEPacket)
        {
        m_pRLEPacket = pRLEPacket;
        m_pRLEPacket->SetBufferOwnership(false);
        }
    ~AutoRestoreBufferOwnership()
        {
        m_pRLEPacket->SetBufferOwnership(true);
        }

    HCDPacketRLE* m_pRLEPacket;
    };


HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToNStripCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptStripToNStripCapabilities::HRFAdaptStripToNStripCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_STRIP);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_STRIP);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_N_BLOCK);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToNStripCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptStripToNStripCreator::GetCapabilities() const
    {
    return HRFAdaptStripToNStripCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptStripToNStripCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                      uint32_t              pi_Page,
                                                      unsigned short       pi_Resolution,
                                                      HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptStripToNStrip(GetCapabilities(),
                                     pi_rpRasterFile,
                                     pi_Page,
                                     pi_Resolution,
                                     pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptStripToNStrip::HRFAdaptStripToNStrip(  HRFBlockAdapterCapabilities* pi_pCapabilities,
                                               HFCPtr<HRFRasterFile>        pi_rpRasterFile,
                                               uint32_t                     pi_Page,
                                               unsigned short              pi_Resolution,
                                               HFCAccessMode                pi_AccessMode)

    : HRFBlockAdapter(pi_pCapabilities,
                      pi_rpRasterFile,
                      pi_Page,
                      pi_Resolution,
                      pi_AccessMode)
    {
    // Source Strip information
    m_StripHeight        = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();
    m_StripPerBlock      = GetResolutionDescriptor()->GetBlockHeight() / m_StripHeight;
    m_ExactBytesPerStrip = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes();

    // Old way of computing m_ExactBytesPerStrip
    HDEBUGCODE(double DBytesByPixel= m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() / 8.0;)
    HASSERT(((uint32_t)(ceil((double)m_pResolutionDescriptor->GetWidth() * DBytesByPixel)) * m_StripHeight) == m_ExactBytesPerStrip);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptStripToNStrip::~HRFAdaptStripToNStrip()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToNStrip::ReadBlock(uint64_t pi_PosBlockX,
                                         uint64_t pi_PosBlockY,
                                         Byte* po_pData,
                                         HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status = H_SUCCESS;
    Byte* pPosInBlock = po_pData;

    uint32_t NumberOfBlockToAccess = m_StripPerBlock;

    // Adjust if necessary the number of strip at the resolution height
    if (pi_PosBlockY + (m_StripPerBlock*m_StripHeight) >= m_pResolutionDescriptor->GetHeight())
        {
        HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

        NumberOfBlockToAccess = (((uint32_t)m_pResolutionDescriptor->GetHeight() + m_StripHeight -1) - (uint32_t)pi_PosBlockY) / m_StripHeight;
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Load the block from file to the client data out
    for (uint32_t NoStrip=0; (NoStrip < NumberOfBlockToAccess) && (Status == H_SUCCESS); NoStrip++)
        {
        Status = m_pAdaptedResolutionEditor->ReadBlock(0, pi_PosBlockY+(NoStrip*m_StripHeight), pPosInBlock, pi_pSisterFileLock);
        // Move inside the buffer to the current Strip
        pPosInBlock += m_ExactBytesPerStrip;
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlockRLE
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToNStrip::ReadBlockRLE(uint64_t                 pi_PosBlockX,
                                            uint64_t                 pi_PosBlockY,
                                            HFCPtr<HCDPacketRLE>&    po_rpPacketRLE,
                                            HFCLockMonitor const*    pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_rpPacketRLE->HasBufferOwnership());    // Must be owner of buffers.
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetBlockWidth());
    HPRECONDITION(po_rpPacketRLE->GetCodec()->GetHeight() >= GetResolutionDescriptor()->GetBlockHeight());
    HPRECONDITION(GetResolutionDescriptor()->GetHeight() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetWidth() <= ULONG_MAX);
    HPRECONDITION(GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);
    HPRECONDITION(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBytesPerBlockWidth() <= ULONG_MAX);
    HPRECONDITION (pi_PosBlockY % GetResolutionDescriptor()->GetBlockHeight() == 0); // PosY is aligned to a block boundary.

    HSTATUS Status = H_SUCCESS;

    uint32_t NumberOfBlockToAccess = m_StripPerBlock;
    uint32_t NumberOfLineInBlock = GetResolutionDescriptor()->GetBlockHeight();

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    // Adjust if necessary the number of strip at the resolution height
    if (pi_PosBlockY + (m_StripPerBlock*m_StripHeight) >= m_pResolutionDescriptor->GetHeight())
        {
        NumberOfBlockToAccess = (((uint32_t)m_pResolutionDescriptor->GetHeight() + m_StripHeight -1) - (uint32_t)pi_PosBlockY) / m_StripHeight;
        NumberOfLineInBlock = (uint32_t)GetResolutionDescriptor()->GetHeight() - (uint32_t)pi_PosBlockY;
        }

    HFCPtr<HCDPacketRLE> pWorkPacket(new HCDPacketRLE((uint32_t)GetResolutionDescriptor()->GetWidth(), m_StripHeight));
    pWorkPacket->SetBufferOwnership(true);  // Owns empty buffers.

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Read these strip from file and copy their data to the output buffer
    for (uint32_t NoStrip(0); NoStrip < NumberOfBlockToAccess; ++NoStrip)
        {
        uint32_t CurrentStripStartPos = NoStrip * m_StripHeight;
        uint32_t NumberOfLineInStrip(m_StripHeight);

        // Last strip ?
        if(pi_PosBlockY + ((NoStrip+1) * m_StripHeight) > GetResolutionDescriptor()->GetHeight())
            {
            NumberOfLineInStrip = (uint32_t)GetResolutionDescriptor()->GetHeight() - ((uint32_t)pi_PosBlockY + CurrentStripStartPos);
            }

        if(H_SUCCESS != (Status = m_pAdaptedResolutionEditor->ReadBlockRLE(0, pi_PosBlockY + CurrentStripStartPos, pWorkPacket, pi_pSisterFileLock)))
            break;  // We failed stop now!

        // Recopy lines buffer pointers to the output packet, it will take ownership of the line buffers.
        AutoRestoreBufferOwnership  __OwnershipOFF(pWorkPacket);        // Turn off ownership so memory is not freed when we reset a line buffer.
        for(uint32_t CurrentStripLine=0; CurrentStripLine < NumberOfLineInStrip; ++CurrentStripLine)
            {
            po_rpPacketRLE->SetLineBuffer(CurrentStripStartPos+CurrentStripLine, pWorkPacket->GetLineBuffer(CurrentStripLine), pWorkPacket->GetLineBufferSize(CurrentStripLine), pWorkPacket->GetLineDataSize(CurrentStripLine));
            pWorkPacket->SetLineBuffer(CurrentStripLine,0,0,0);   // Reset this line since po_rpPacketRLE took ownership of this buffer.
            }
        }

    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToNStrip::WriteBlock(uint64_t     pi_PosBlockX,
                                          uint64_t     pi_PosBlockY,
                                          const Byte*  pi_pData,
                                          HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;
    const Byte* pPosInBlock = pi_pData;

    uint32_t NumberOfBlockToAccess = m_StripPerBlock;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    // Adjust if necessary the number of strip at the resolution height
    if (pi_PosBlockY + (m_StripPerBlock*m_StripHeight) >= (uint32_t)m_pResolutionDescriptor->GetHeight())
        {
        NumberOfBlockToAccess = (((uint32_t)m_pResolutionDescriptor->GetHeight() + m_StripHeight -1) - (uint32_t)pi_PosBlockY) / m_StripHeight;
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Write the client data to the file
    for (uint32_t NoStrip=0; (NoStrip < NumberOfBlockToAccess) && (Status == H_SUCCESS); NoStrip++)
        {
        // Move inside the buffer to the current Strip
        Status = m_pAdaptedResolutionEditor->WriteBlock(0, pi_PosBlockY+(NoStrip*m_StripHeight), pPosInBlock, pi_pSisterFileLock);
        pPosInBlock += m_ExactBytesPerStrip;
        }
    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToNStrip::WriteBlockRLE(uint64_t              pi_PosBlockX,
                                             uint64_t              pi_PosBlockY,
                                             HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    uint32_t NumberOfBlockToAccess = m_StripPerBlock;

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    // Adjust if necessary the number of strip at the resolution height
    if (pi_PosBlockY + (m_StripPerBlock*m_StripHeight) >= m_pResolutionDescriptor->GetHeight())
        {
        NumberOfBlockToAccess = (((uint32_t)m_pResolutionDescriptor->GetHeight() + m_StripHeight -1) - (uint32_t)pi_PosBlockY) / m_StripHeight;
        }

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Write the client data to the file
    for (uint32_t NoStrip=0; (NoStrip < NumberOfBlockToAccess) && (Status == H_SUCCESS); ++NoStrip)
        {
        uint32_t CurrentStripStartPos = NoStrip * m_StripHeight;
        uint32_t NumberOfLineInStrip = m_StripHeight;

        // Last strip ?
        if(pi_PosBlockY + ((NoStrip+1) * m_StripHeight) > (uint32_t)GetResolutionDescriptor()->GetHeight())
            NumberOfLineInStrip = (uint32_t)GetResolutionDescriptor()->GetHeight() - ((uint32_t)pi_PosBlockY + CurrentStripStartPos);

        // Create a packet that contains only the current strip block.
        HFCPtr<HCDPacketRLE> pOneStripPacketRLE(new HCDPacketRLE((uint32_t)m_pResolutionDescriptor->GetWidth(), NumberOfLineInStrip));
        pOneStripPacketRLE->SetBufferOwnership(false);
        for (uint32_t NoLine=0; NoLine < NumberOfLineInStrip; ++NoLine)
            pOneStripPacketRLE->SetLineBuffer(NoLine, pi_rpPacketRLE->GetLineBuffer(CurrentStripStartPos+NoLine), pi_rpPacketRLE->GetLineBufferSize(CurrentStripStartPos+NoLine),
                                              pi_rpPacketRLE->GetLineDataSize(CurrentStripStartPos+NoLine));

        Status = m_pAdaptedResolutionEditor->WriteBlockRLE(0, pi_PosBlockY+CurrentStripStartPos, pOneStripPacketRLE, pi_pSisterFileLock);
        }

    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }