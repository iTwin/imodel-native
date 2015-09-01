//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptStripToLine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptStripToLine
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptStripToLine.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToLineCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptStripToLineCapabilities::HRFAdaptStripToLineCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_STRIP);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_LINE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptStripToLineCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptStripToLineCreator::GetCapabilities() const
    {
    return HRFAdaptStripToLineCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptStripToLineCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    unsigned short       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptStripToLine(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptStripToLine::HRFAdaptStripToLine(
    HRFBlockAdapterCapabilities* pi_pCapabilities,
    HFCPtr<HRFRasterFile>        pi_rpRasterFile,
    uint32_t                     pi_Page,
    unsigned short              pi_Resolution,
    HFCAccessMode                pi_AccessMode)

    : HRFBlockAdapter(pi_pCapabilities,
                      pi_rpRasterFile,
                      pi_Page,
                      pi_Resolution,
                      pi_AccessMode),
    m_BufferedStripIndexY(-1)
    {
    // Calc the number of bytes per Image Width
    double DBytesByPixel= m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() / 8.0;

    m_StripHeight         = m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight();

    HASSERT(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerWidth() <= ULONG_MAX);

    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    m_RasterHeight       = (uint32_t)m_pResolutionDescriptor->GetHeight();
    m_NextLineToWrite  = 0;

    // Calc the number of bytes per Tile Width
    m_ExactBytesPerStripWidth =  (uint32_t)ceil(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockWidth() * DBytesByPixel);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptStripToLine::~HRFAdaptStripToLine()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToLine::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    uint32_t StripIndexY = (uint32_t)pi_PosBlockY / m_StripHeight;

    if (StripIndexY != m_BufferedStripIndexY)
        {
        ReadAStrip (StripIndexY, pi_pSisterFileLock);
        m_BufferedStripIndexY = StripIndexY;
        }
    HASSERT(m_pStrip != 0);

    Byte* pPosInStrip = m_pStrip + (pi_PosBlockY % m_StripHeight) * m_ExactBytesPerStripWidth;
    memcpy(po_pData, pPosInStrip, m_ExactBytesPerStripWidth);

    if (pi_PosBlockY == m_RasterHeight-1)
        Delete_m_pStrip();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToLine::WriteBlock(uint64_t    pi_PosBlockX,
                                        uint64_t    pi_PosBlockY,
                                        const Byte* pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_SUCCESS;

    HASSERT (m_NextLineToWrite == pi_PosBlockY);
    m_NextLineToWrite++;

    if ( m_pStrip == 0)
        Alloc_m_pStrip ();
    HASSERT(m_pStrip != 0);

    Byte* pPosInStrip = m_pStrip + ((pi_PosBlockY % m_StripHeight) * m_ExactBytesPerStripWidth);
    memcpy(pPosInStrip, pi_pData, m_ExactBytesPerStripWidth);

    if ((pi_PosBlockY % m_StripHeight) == m_StripHeight - 1 ||
        pi_PosBlockY == m_RasterHeight-1)
        Status = WriteAStrip ((uint32_t)pi_PosBlockY / m_StripHeight, pi_pSisterFileLock);

    if (pi_PosBlockY == m_RasterHeight-1)
        Delete_m_pStrip();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// Alloc_m_pStrip : Allocate memory if not already done
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptStripToLine::Alloc_m_pStrip()
    {
    m_pStrip = new Byte[m_StripHeight * m_ExactBytesPerStripWidth];
    }
//-----------------------------------------------------------------------------
// public
// Delete_m_pStrip : Release memory
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptStripToLine::Delete_m_pStrip()
    {
    m_pStrip = 0;
    m_BufferedStripIndexY = -1;
    }

//-----------------------------------------------------------------------------
// Private
// ReadAStrip :
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToLine::ReadAStrip(uint32_t pi_StripIndexY, HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS Status   = H_SUCCESS;
    uint32_t PosBlocY = pi_StripIndexY * m_StripHeight;

    HASSERT(PosBlocY < m_pResolutionDescriptor->GetHeight());

    // Alloc working buffer
    if (m_pStrip == 0)
        Alloc_m_pStrip ();

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    Status = m_pAdaptedResolutionEditor->ReadBlock(0, PosBlocY, m_pStrip, pi_pSisterFileLock);

    return Status;
    }
//-----------------------------------------------------------------------------
// Private
// WriteAStrip :
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptStripToLine::WriteAStrip (uint32_t pi_StripIndexY, HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS Status   = H_SUCCESS;
    uint32_t PosBlocY = pi_StripIndexY * m_StripHeight;

    HASSERT (PosBlocY < m_pResolutionDescriptor->GetHeight());
    HASSERT (m_pStrip != 0);

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    Status = m_pAdaptedResolutionEditor->WriteBlock(0, PosBlocY, m_pStrip, pi_pSisterFileLock);

    m_pTheTrueOriginalFile->SharingControlIncrementCount();

    return Status;
    }
