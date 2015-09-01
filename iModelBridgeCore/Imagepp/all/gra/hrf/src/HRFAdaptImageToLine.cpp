//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFAdaptImageToLine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFAdaptImageToLine
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFAdaptImageToLine.h>

HFC_IMPLEMENT_SINGLETON(HRFAdaptImageToLineCapabilities)

//-----------------------------------------------------------------------------
// This specific implementation of this object add
// the supported thing to the list.
//-----------------------------------------------------------------------------
HRFAdaptImageToLineCapabilities::HRFAdaptImageToLineCapabilities()
    : HRFBlockAdapterCapabilities()
    {
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::FROM_IMAGE);
    m_ListOfCapabilities.push_back(HRFBlockAdapterCapabilities::TO_LINE);
    }

//-----------------------------------------------------------------------------
// This is a utility class to create the specific Implementation object.
// It is used by the Stretcher factory.
//-----------------------------------------------------------------------------

HFC_IMPLEMENT_SINGLETON(HRFAdaptImageToLineCreator)

//-----------------------------------------------------------------------------
// Obtain the capabilities of stretcher
//-----------------------------------------------------------------------------
HRFBlockAdapterCapabilities* HRFAdaptImageToLineCreator::GetCapabilities() const
    {
    return HRFAdaptImageToLineCapabilities::GetInstance();
    }

//-----------------------------------------------------------------------------
// Creation of implementator
//-----------------------------------------------------------------------------
HRFBlockAdapter* HRFAdaptImageToLineCreator::Create(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                    uint32_t              pi_Page,
                                                    unsigned short       pi_Resolution,
                                                    HFCAccessMode         pi_AccessMode) const
    {
    return new HRFAdaptImageToLine(GetCapabilities(),
                                   pi_rpRasterFile,
                                   pi_Page,
                                   pi_Resolution,
                                   pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFAdaptImageToLine::HRFAdaptImageToLine(
    HRFBlockAdapterCapabilities* pi_pCapabilities,
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

    HASSERT(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlocksPerWidth() <= ULONG_MAX);
    HASSERT(m_pResolutionDescriptor->GetHeight() <= ULONG_MAX);

    m_RasterHeight       = (uint32_t)m_pResolutionDescriptor->GetHeight();
    m_NextLineToWrite  = 0;

    // Calc the number of bytes per Image Width
    double DBytesByPixel= m_pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() / 8.0;
    // Calc the number of bytes per Tile Width
    m_ExactBytesPerLine =  (uint32_t)ceil(m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockWidth() * DBytesByPixel);
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFAdaptImageToLine::~HRFAdaptImageToLine()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptImageToLine::ReadBlock(uint64_t pi_PosBlockX,
                                       uint64_t pi_PosBlockY,
                                       Byte*  po_pData,
                                       HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HSTATUS Status = H_SUCCESS;

    if (m_pImage == 0)
        {
        Alloc_m_pImage();

        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        Status = m_pAdaptedResolutionEditor->ReadBlock(0, 0, m_pImage, pi_pSisterFileLock);
        }
    HASSERT(m_pImage != 0);

    memcpy(po_pData, &(m_pImage[pi_PosBlockY*m_ExactBytesPerLine]), m_ExactBytesPerLine);

    if (pi_PosBlockY == m_RasterHeight-1)
        Delete_m_pImage();

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFAdaptImageToLine::WriteBlock(uint64_t    pi_PosBlockX,
                                        uint64_t    pi_PosBlockY,
                                        const Byte* pi_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HSTATUS Status = H_SUCCESS;

    HASSERT (m_NextLineToWrite == pi_PosBlockY);
    m_NextLineToWrite++;

    if (m_pImage == 0)
        Alloc_m_pImage ();
    HASSERT(m_pImage != 0);

    memcpy(&(m_pImage[pi_PosBlockY*m_ExactBytesPerLine]), pi_pData, m_ExactBytesPerLine);

    if (pi_PosBlockY == m_RasterHeight-1)
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Lock the file.
            AssignRasterFileLock(m_pTheTrueOriginalFile, SisterFileLock, false);
            pi_pSisterFileLock = &SisterFileLock;
            }

        Status = m_pAdaptedResolutionEditor->WriteBlock(0, 0, m_pImage, pi_pSisterFileLock);

        m_pTheTrueOriginalFile->SharingControlIncrementCount();
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// Alloc_m_pStrip : Allocate memory if not already done
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptImageToLine::Alloc_m_pImage()
    {
    m_pImage = new Byte[m_pAdaptedResolutionEditor->GetResolutionDescriptor()->GetBlockHeight() * m_ExactBytesPerLine];
    }
//-----------------------------------------------------------------------------
// public
// Delete_m_pStrip : Release memory
// Edition by Block
//-----------------------------------------------------------------------------
void HRFAdaptImageToLine::Delete_m_pImage()
    {
    m_pImage = 0;
    }


