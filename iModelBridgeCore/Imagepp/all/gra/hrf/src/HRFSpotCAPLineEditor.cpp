//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSpotCAPLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFSpotCAPLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFSpotCAPLineEditor.h>
#include <Imagepp/all/h/HRFSpotCAPFile.h>
#include <Imagepp/all/h/HTIFFUtils.h>




//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------

HRFSpotCAPLineEditor::HRFSpotCAPLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                           uint32_t              pi_Page,
                                           unsigned short       pi_Resolution,
                                           HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile   = static_cast<HRFSpotCAPFile*>(GetRasterFile().GetPtr());

    m_Offset        = 0;
    m_BandNumber    = m_pRasterFile->GetNbBands();
    m_LineWidth     = m_pRasterFile->GetImageWidth();
    m_nNbChannel    = GetResolutionDescriptor()->GetPixelType()->GetChannelOrg().CountChannels();
    m_nbBitsPerBandPerPixel = GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits() / m_nNbChannel;


    if ( 3 <= m_nNbChannel)
        {
        m_pRedLineBuffer    = new Byte[m_LineWidth];
        m_pGreenLineBuffer  = new Byte[m_LineWidth];
        m_pBlueLineBuffer   = new Byte[m_LineWidth];
        }
    else
        {
        m_pRedLineBuffer    = NULL;
        m_pGreenLineBuffer  = NULL;
        m_pBlueLineBuffer   = NULL;
        }

    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------

HRFSpotCAPLineEditor::~HRFSpotCAPLineEditor()
    {
    if (NULL != m_pRedLineBuffer)
        delete [] m_pRedLineBuffer;
    if (NULL != m_pGreenLineBuffer)
        delete [] m_pGreenLineBuffer;
    if (NULL != m_pBlueLineBuffer)
        delete [] m_pBlueLineBuffer;
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFSpotCAPLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                        uint64_t pi_PosBlockY,
                                        Byte*   po_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);
    HPRECONDITION (pi_PosBlockX <= ULONG_MAX && pi_PosBlockY <= ULONG_MAX);

    HSTATUS Status = H_ERROR;

    // Temporary need for virtual ptr.
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    switch(m_nNbChannel)
        {
        case 1:
            if (m_nbBitsPerBandPerPixel == 8)
                Status = Read8BitGrayBlock((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, po_pData, pi_pSisterFileLock);
            break;

        case 3:
            if (m_nbBitsPerBandPerPixel == 8)
                Status = Read8BitGrayBlock((uint32_t)pi_PosBlockX, (uint32_t)pi_PosBlockY, po_pData, pi_pSisterFileLock);
            break;

        case 4:
            //not supported yet
            HASSERT(0);
            break;
        default:
            // return the default value : H_ERROR
            break;

        }

WRAPUP:

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFSpotCAPLineEditor::Read8BitGrayBlock(uint32_t pi_PosBlockX,
                                                uint32_t pi_PosBlockY,
                                                Byte*   po_pData,
                                                HFCLockMonitor const* pi_pSisterFileLock)
    {
    HSTATUS Status = H_SUCCESS;

    // Temporary need for virtual ptr.
    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        m_Offset =  m_pRasterFile->GetOffset() +
                    (pi_PosBlockY * m_pRasterFile->GetTotalBytesPerRow())
                    + m_pRasterFile->GetHeaderSize() + m_pRasterFile->GetNbBytesPrefixDataPerRecord();

        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        if (m_pRasterFile->m_pImagFile->GetCurrentPos() != m_Offset)
            m_pRasterFile->m_pImagFile->SeekToPos(m_Offset);

        uint32_t DataSize = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
        if(m_pRasterFile->m_pImagFile->Read(po_pData, DataSize) != DataSize)
            Status = H_ERROR;

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    else
        Status = H_NOT_FOUND;

    return Status;
    }

//-----------------------------------------------------------------------------
// protected
// Read uncompressed 24 bit RGB Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFSpotCAPLineEditor::Read24BitRgbBlock(uint32_t pi_PosBlockX,
                                                uint32_t pi_PosBlockY,
                                                Byte*   po_pData,
                                                HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    uint32_t PixelIndex, BytePos = 0;

    int64_t RedChannel    = m_pRasterFile->GetRedChannel();
    int64_t GreenChannel  = m_pRasterFile->GetGreenChannel();
    int64_t BlueChannel   = m_pRasterFile->GetBlueChannel();

    if (NULL == m_pRedLineBuffer)
        m_pRedLineBuffer = new Byte[m_LineWidth];
    if (NULL == m_pGreenLineBuffer)
        m_pGreenLineBuffer = new Byte[m_LineWidth];
    if (NULL == m_pBlueLineBuffer)
        m_pBlueLineBuffer = new Byte[m_LineWidth];

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    if (GetResolutionDescriptor()->GetScanlineOrientation().IsUpper())
        {
        uint64_t offSetToLine = m_Offset + (pi_PosBlockY * m_pRasterFile->GetTotalBytesPerRow()) +
                                m_pRasterFile->GetHeaderSize() + m_pRasterFile->GetNbBytesPrefixDataPerRecord();
        offSetToLine += m_LineWidth * (RedChannel - 1);
        m_pRasterFile->m_pImagFile->SeekToPos(offSetToLine);

        if(m_pRasterFile->m_pImagFile->Read(m_pRedLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        m_pRasterFile->m_pImagFile->Seek((GreenChannel - RedChannel - 1) * m_pRasterFile->GetTotalBytesPerRow());

        if(m_pRasterFile->m_pImagFile->Read(m_pGreenLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        m_pRasterFile->m_pImagFile->Seek((BlueChannel - GreenChannel - 1) * m_pRasterFile->GetTotalBytesPerRow());

        if(m_pRasterFile->m_pImagFile->Read(m_pBlueLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;
        }
    else
        {
        HASSERT(0);
        goto WRAPUP;
        }

    SisterFileLock.ReleaseKey();

    if (GetResolutionDescriptor()->GetScanlineOrientation().IsLeft())
        {
        for (PixelIndex = 0; PixelIndex < m_LineWidth; PixelIndex++)
            {
            // po_pData[BytePos++]= (m_pRedLineBuffer  [PixelIndex] - m_pRasterFile->GetRedMinValue()  ) * m_RedBandScaling;
            // po_pData[BytePos++]= (m_pGreenLineBuffer[PixelIndex] - m_pRasterFile->GetGreenMinValue()) * m_GreenBandScaling;
            // po_pData[BytePos++]= (m_pBlueLineBuffer [PixelIndex] - m_pRasterFile->GetBlueMinValue() ) * m_BlueBandScaling;

            po_pData[BytePos++]= m_pRedLineBuffer  [PixelIndex];
            po_pData[BytePos++]= m_pGreenLineBuffer[PixelIndex];
            po_pData[BytePos++]= m_pBlueLineBuffer [PixelIndex];
            }
        }
    else
        {
        for (PixelIndex = m_LineWidth - 1; PixelIndex >= 0; PixelIndex--)
            {
            // po_pData[BytePos++]= (m_pRedLineBuffer  [PixelIndex] - m_pRasterFile->GetRedMinValue() )  * m_RedBandScaling;
            // po_pData[BytePos++]= (m_pGreenLineBuffer[PixelIndex] - m_pRasterFile->GetGreenMinValue()) * m_GreenBandScaling;
            // po_pData[BytePos++]= (m_pBlueLineBuffer [PixelIndex] - m_pRasterFile->GetBlueMinValue() ) * m_BlueBandScaling;

            po_pData[BytePos++]= m_pRedLineBuffer  [PixelIndex];
            po_pData[BytePos++]= m_pGreenLineBuffer[PixelIndex];
            po_pData[BytePos++]= m_pBlueLineBuffer [PixelIndex];
            }
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }



//-----------------------------------------------------------------------------
// public
// Write uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFSpotCAPLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                         uint64_t     pi_PosBlockY,
                                         const Byte*  pi_pData,
                                         HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);

    //not supported
    HASSERT(0);

    return H_ERROR;
    }


