//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFEpsLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFEpsLineEditor.h>
#include <Imagepp/all/h/HRFEpsFile.h>

#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>


/** -----------------------------------------------------------------------------
    Line editor constructor.
    -----------------------------------------------------------------------------
*/
HRFEpsLineEditor::HRFEpsLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    // We only support one page and one resolution
    HASSERT(pi_Page == 0);
    HASSERT(pi_Resolution == 0);

    m_pRasterFile = (HFCPtr<HRFEpsFile>&) pi_rpRasterFile;

    m_CurrentLine = 0;

    HFCPtr<HRFPageDescriptor> pPageDescriptor = pi_rpRasterFile->GetPageDescriptor(pi_Page);
    HASSERT(pPageDescriptor != 0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(pi_Resolution);
    HASSERT(pResolutionDescriptor != 0);

    bool IsRGBFile  = pResolutionDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID);
    bool Is1BitFile = pResolutionDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeV1Gray1::CLASS_ID);

    if (IsRGBFile)
        m_InputBytesPerLine = 3 * (uint32_t)pResolutionDescriptor->GetWidth();
    else if (Is1BitFile)
        m_InputBytesPerLine = ((uint32_t)pResolutionDescriptor->GetWidth() + 7) / 8;
    else
        m_InputBytesPerLine = (uint32_t)pResolutionDescriptor->GetWidth();

    m_OutputBytesPerLine = m_InputBytesPerLine * 2;

    // One line of pixels ready to output. Need 2 ASCII characters
    // per input byte, plus 1 byte for the NULL that sprintf uses.
    m_pLineBuffer = new char[m_OutputBytesPerLine + 1];

    m_CurrentOutputPosition = 0;
    }


/** -----------------------------------------------------------------------------
    Line editor destructor
    -----------------------------------------------------------------------------
*/
HRFEpsLineEditor::~HRFEpsLineEditor()
    {
    }


/** -----------------------------------------------------------------------------
    Read an uncompressed line. Unsupported.
    -----------------------------------------------------------------------------
*/
HSTATUS HRFEpsLineEditor::ReadBlock(uint64_t     pi_PosBlockX,
                                    uint64_t     pi_PosBlockY,
                                    Byte*        pi_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    return H_DATA_NOT_AVAILABLE;
    }


/** -----------------------------------------------------------------------------
    Write an uncompressed line. In the output file, the lines must not be
    more than 255 characters long, so the pixel line will be broken if
    necessary. Subsequent lines start at the current position, there is no
    need to start at column 0.
    -----------------------------------------------------------------------------
*/
HSTATUS HRFEpsLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte*  pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    // Full lines only
    HASSERT(pi_PosBlockX == 0);

    HSTATUS Status = H_ERROR;

    if (pi_PosBlockY == m_CurrentLine)
        {
        // Prepare all the pixel line for output
        for (uint32_t i = 0 ; i < m_InputBytesPerLine ; ++i)
            {
            sprintf(&m_pLineBuffer[i*2], "%2.2X", pi_pData[i]);
            }

        uint32_t BytesToProcess = m_OutputBytesPerLine;
        uint32_t CurrentPositionInBuffer = 0;
        unsigned short BytesLeftInLine;
        unsigned short BytesToWrite;

        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
            pi_pSisterFileLock = &SisterFileLock;
            }

        // Write lines in output. Maximum 254 characters per line.
        while (BytesToProcess > 0)
            {
            BytesLeftInLine = 254 - m_CurrentOutputPosition;
            BytesToWrite = (unsigned short)MIN(BytesLeftInLine, BytesToProcess);

            if(m_pRasterFile->m_pFile->Write(&m_pLineBuffer[CurrentPositionInBuffer], BytesToWrite) != BytesToWrite)
                goto WRAPUP;    // H_ERROR

            CurrentPositionInBuffer += BytesToWrite;
            BytesLeftInLine -= BytesToWrite;
            BytesToProcess -= BytesToWrite;
            m_CurrentOutputPosition += BytesToWrite;

            if (BytesLeftInLine == 0)
                {
                if(m_pRasterFile->m_pFile->Write("\r\n", 2) != 2)
                    goto WRAPUP;    // H_ERROR

                m_CurrentOutputPosition = 0;
                }
            }

        GetRasterFile()->SharingControlIncrementCount();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        ++m_CurrentLine;
        }
    else
        goto WRAPUP;

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }


