//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptStripToImage.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptStripToImage
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptStripToImage.h>

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

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToImageCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptStripToImageCapabilities::HRFAdaptStripToImageCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_STRIP);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_IMAGE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToImageCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptStripToImageCreator::GetCapabilities() const
    {
    return HRFAdaptStripToImageCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptStripToImageCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                     uint32_t              pi_Page,
                                                     unsigned short       pi_Resolution,
                                                     HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptStripToImage(GetCapabilities(),
                                    pi_rpRasterFile,
                                    pi_Page,
                                    pi_Resolution,
                                    pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptStripToImage::HRFAdaptStripToImage(HRFBlockAdapterCapabilities* pi_pCapabilities,
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
    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);
    HASSERT(m_pResolutionDescriptor->GetBytesPerWidth() <= ULONG_MAX);

    m_ImageHeight = (uint32_t)m_pResolutionDescriptor->GetHeight();

    // Calc the number of bytes per Image Width
    m_ExactBytesPerImageWidth = (uint32_t)m_pResolutionDescriptor->GetBytesPerWidth();

    // Obtain the strip height
    m_StripHeight = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();

    // Calc the number of bytes per Strip
    m_ExactBytesPerStrip = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes();

    // Calc the number of Strip by tile Image height
    HASSERT(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerHeight() <= ULONG_MAX);

    m_NumberOfStripByImageHeight = (uint32_t)m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerHeight();
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptStripToImage::~HRFAdaptStripToImage()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToImage::ReadBlock(uint64_t pi_PosBlockX,
                                        uint64_t pi_PosBlockY,
                                        Byte*  po_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (m_NumberOfStripByImageHeight > 0);
    HSTATUS Status = H_SUCCESS;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (m_NumberOfStripByImageHeight > 1)
        {
        // Alloc working buffer
        m_pStripsData = new Byte[m_ExactBytesPerStrip];

        // Read these strip from file and copy their data to the output buffer
        for (uint32_t NoStrip=0; NoStrip < m_NumberOfStripByImageHeight - 1 && (Status == H_SUCCESS); NoStrip++)
            Status = m_pAdaptedResolutionEditor->ReadBlock(0, NoStrip * m_StripHeight, po_pData+(NoStrip * m_ExactBytesPerStrip), pi_pSisterFileLock);

        if (Status == H_SUCCESS)
            {
            uint32_t LastStripNumber = (m_NumberOfStripByImageHeight - 1);

            // If the strip intersect with the image extent we clip the height of the strip
            uint32_t NumberOfLineToCopy = m_ImageHeight - (LastStripNumber * m_StripHeight);
            Status = m_pAdaptedResolutionEditor->ReadBlock(0, LastStripNumber * m_StripHeight, m_pStripsData, pi_pSisterFileLock);

            if (Status == H_SUCCESS)
                // Copy a part from last strip
                for (uint32_t NoLine=0; NoLine < NumberOfLineToCopy; NoLine++)
                    memcpy(po_pData+(LastStripNumber * m_ExactBytesPerStrip)+(NoLine * m_ExactBytesPerImageWidth),
                           m_pStripsData+(NoLine * m_ExactBytesPerImageWidth), m_ExactBytesPerImageWidth);
            }
        }
    else
        {
        if (m_StripHeight > m_ImageHeight)
            {
            // Alloc working buffer
            m_pStripsData = new Byte[m_ExactBytesPerStrip];

            // Read these strip from file and copy their data to the output buffer
            Status = m_pAdaptedResolutionEditor->ReadBlock((uint32_t)0, (uint32_t)0, m_pStripsData, pi_pSisterFileLock);
            if (Status == H_SUCCESS)
                {
                HASSERT(m_pResolutionDescriptor->GetSizeInBytes() <= ULONG_MAX);
                memcpy(po_pData, m_pStripsData, (uint32_t)m_pResolutionDescriptor->GetSizeInBytes());
                }
            }
        else
            // Read these strip from file and copy their data to the output buffer
            Status = m_pAdaptedResolutionEditor->ReadBlock((uint32_t)0, (uint32_t)0, po_pData, pi_pSisterFileLock);
        }
    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    // Free working buffer
    m_pStripsData = 0;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToImage::ReadBlock(uint64_t            pi_PosBlockX,
                                        uint64_t            pi_PosBlockY,
                                        HFCPtr<HCDPacket>&  po_rpPacket,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    return T_Super::ReadBlock(pi_PosBlockX,pi_PosBlockY,po_rpPacket,pi_pSisterFileLock);
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToImage::WriteBlock(uint64_t     pi_PosBlockX,
                                         uint64_t     pi_PosBlockY,
                                         const Byte*  pi_pData,
                                         HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status      = H_SUCCESS;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (m_NumberOfStripByImageHeight > 1)
        {
        // Alloc working buffer
        m_pStripsData = new Byte[m_ExactBytesPerStrip];

        // Read these strip from file and copy their data to the output buffer
        for (uint32_t NoStrip=0; NoStrip < m_NumberOfStripByImageHeight - 1 && (Status == H_SUCCESS); NoStrip++)
            Status = m_pAdaptedResolutionEditor->WriteBlock(0, NoStrip * m_StripHeight, pi_pData+(NoStrip * m_ExactBytesPerStrip), pi_pSisterFileLock);

        if (Status == H_SUCCESS)
            {
            uint32_t LastStripNumber = (m_NumberOfStripByImageHeight - 1);

            // If the strip intersect with the image extent we clip the height of the strip
            uint32_t NumberOfLineToCopy = m_ImageHeight - (LastStripNumber * m_StripHeight);

            // Copy a part from last strip
            for (uint32_t NoLine=0; NoLine < NumberOfLineToCopy; NoLine++)
                memcpy(m_pStripsData+(NoLine * m_ExactBytesPerImageWidth),
                       pi_pData+(LastStripNumber * m_ExactBytesPerStrip)+(NoLine * m_ExactBytesPerImageWidth),
                       m_ExactBytesPerImageWidth);

            Status = m_pAdaptedResolutionEditor->WriteBlock(0, LastStripNumber * m_StripHeight, m_pStripsData, pi_pSisterFileLock);
            }
        }
    else
        {
        if ((m_NumberOfStripByImageHeight = 1) && (m_StripHeight > m_ImageHeight))
            {
            // Alloc working buffer
            m_pStripsData = new Byte[m_ExactBytesPerStrip];

            memset(m_pStripsData, 0, m_ExactBytesPerStrip);

            HASSERT(m_pResolutionDescriptor->GetSizeInBytes() <= ULONG_MAX);
            memcpy(m_pStripsData, pi_pData, (uint32_t)m_pResolutionDescriptor->GetSizeInBytes());
            // Read these strip from file and copy their data to the output buffer
            Status = m_pAdaptedResolutionEditor->WriteBlock(0, 0, m_pStripsData, pi_pSisterFileLock);
            }
        else
            // Read these strip from file and copy their data to the output buffer
            Status = m_pAdaptedResolutionEditor->WriteBlock(0, 0, pi_pData, pi_pSisterFileLock);
        }

    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    // Free working buffer
    m_pStripsData = 0;

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// ReadImage
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToImage::WriteBlock(uint64_t                 pi_PosBlockX,
                                         uint64_t                 pi_PosBlockY,
                                         const HFCPtr<HCDPacket>& pi_rpPacket,
                                         HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_ERROR;
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlockRLE
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToImage::WriteBlockRLE(uint64_t              pi_PosBlockX,
                                            uint64_t              pi_PosBlockY,
                                            HFCPtr<HCDPacketRLE>& pi_rpPacketRLE,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_rpPacketRLE != 0);
    HPRECONDITION (pi_rpPacketRLE->GetCodec()->GetHeight() == GetResolutionDescriptor()->GetHeight());
    HPRECONDITION (pi_rpPacketRLE->GetCodec()->GetWidth() == GetResolutionDescriptor()->GetWidth());
    HSTATUS Status      = H_SUCCESS;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Read these strips from file and copy their data to the output packet
    for (uint32_t NoStrip=0; NoStrip < m_NumberOfStripByImageHeight; ++NoStrip)
        {
        uint32_t CurrentStripStartPos = NoStrip * m_StripHeight;
        uint32_t NumberOfLineInStrip = m_StripHeight;

        // Adjust for last strip.
        if(NoStrip == (m_NumberOfStripByImageHeight - 1))
            NumberOfLineInStrip = (uint32_t)GetResolutionDescriptor()->GetHeight() - CurrentStripStartPos;

        // Create a packet that contains only the current strip block.
        HFCPtr<HCDPacketRLE> pOneStripPacketRLE(new HCDPacketRLE((uint32_t)m_pResolutionDescriptor->GetWidth(), NumberOfLineInStrip));
        pOneStripPacketRLE->SetBufferOwnership(false);
        for (uint32_t NoLine=0; NoLine < NumberOfLineInStrip; ++NoLine)
            pOneStripPacketRLE->SetLineBuffer(NoLine,
                                              pi_rpPacketRLE->GetLineBuffer(CurrentStripStartPos+NoLine),
                                              pi_rpPacketRLE->GetLineBufferSize(CurrentStripStartPos+NoLine),
                                              pi_rpPacketRLE->GetLineDataSize(CurrentStripStartPos+NoLine));

        if(H_SUCCESS != (Status = m_pAdaptedResolutionEditor->WriteBlockRLE(0, CurrentStripStartPos, pOneStripPacketRLE, pi_pSisterFileLock)))
            break;  // We failed stop now!
        }

    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }

//-----------------------------------------------------------------------------
// Private
// ReadBlockRLE
// Edition by Image
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToImage::ReadBlockRLE(uint64_t                 pi_PosBlockX,
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

    HSTATUS Status = H_SUCCESS;

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

    // Read these strips from file and copy their data to the output packet
    for (uint32_t NoStrip=0; NoStrip < m_NumberOfStripByImageHeight; ++NoStrip)
        {
        uint32_t CurrentStripStartPos = NoStrip * m_StripHeight;
        uint32_t NumberOfLineInStrip = m_StripHeight;

        // Adjust for last strip.
        if(NoStrip == (m_NumberOfStripByImageHeight - 1))
            NumberOfLineInStrip = (uint32_t)GetResolutionDescriptor()->GetHeight() - CurrentStripStartPos;

        if(H_SUCCESS != (Status = m_pAdaptedResolutionEditor->ReadBlockRLE(0, CurrentStripStartPos, pWorkPacket, pi_pSisterFileLock)))
            break;  // We failed stop now!

        // Recopy lines buffer pointers to the output packet, it will take ownership of the line buffers.
        AutoRestoreBufferOwnership  __OwnershipOFF(pWorkPacket);        // Turn off ownership so memory is not freed when we reset a line buffer.
        for(uint32_t NoLine=0; NoLine < NumberOfLineInStrip; ++NoLine)
            {
            po_rpPacketRLE->SetLineBuffer(CurrentStripStartPos+NoLine, pWorkPacket->GetLineBuffer(NoLine), pWorkPacket->GetLineBufferSize(NoLine), pWorkPacket->GetLineDataSize(NoLine));
            pWorkPacket->SetLineBuffer(NoLine,0,0,0);   // Reset this line since po_rpPacketRLE took ownership of this buffer.
            }
        }

    SisterFileLock.ReleaseKey();

    return Status;
    }
