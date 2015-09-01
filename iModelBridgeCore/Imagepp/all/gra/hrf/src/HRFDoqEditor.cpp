//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFDoqEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRawLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFDoqEditor.h>
#include <Imagepp/all/h/HRFDoqFile.h>



//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFDoqEditor::HRFDoqEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                           uint32_t              pi_Page,
                           unsigned short       pi_Resolution,
                           HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {

    m_pRasterFile   = static_cast<HRFDoqFile*>(GetRasterFile().GetPtr());
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
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFDoqEditor::~HRFDoqEditor()
    {

    }

//-----------------------------------------------------------------------------
// public
// Read uncompressed Block
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFDoqEditor::ReadBlock(uint64_t pi_PosBlockX,
                                uint64_t pi_PosBlockY,
                                Byte*  po_pData,
                                HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (po_pData != 0);
    HPRECONDITION (pi_PosBlockY >= 0);
    HPRECONDITION (m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;

    // Temporary need for virtual ptr.
    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        goto WRAPUP;
        }

    m_Offset =  m_pRasterFile->GetOffset() +
                (pi_PosBlockY * GetResolutionDescriptor()->GetBytesPerWidth())
                + m_pRasterFile->GetHeaderSize();

    if(m_nNbChannel == 1)
        {
        // Lock the sister file if needed
        HFCLockMonitor SisterFileLock;
        if(pi_pSisterFileLock == 0)
            {
            // Get lock and synch.
            AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
            pi_pSisterFileLock = &SisterFileLock;
            }

        if (m_pRasterFile->m_pDoqFile->GetCurrentPos() != m_Offset)
            m_pRasterFile->m_pDoqFile->SeekToPos(m_Offset);


        uint32_t DataSize = (uint32_t)GetResolutionDescriptor()->GetBytesPerWidth();
        if (m_pRasterFile->m_pDoqFile->Read(po_pData, DataSize) != DataSize)
            goto WRAPUP;    // H_ERROR

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    else if(m_nNbChannel == 3)
        {
        if (8 == m_nbBitsPerBandPerPixel)
            Status = Read24BitRgbBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
        else
            goto WRAPUP; // H_ERROR //TODO : tko
        }
    else
        goto WRAPUP;    // H_ERROR //TODO: tko


    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// protected
// Read uncompressed 24 bit RGB Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFDoqEditor::Read24BitRgbBlock(uint64_t pi_PosBlockX,
                                        uint64_t pi_PosBlockY,
                                        Byte*  po_pData,
                                        HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    uint32_t PixelIndex, BytePos = 0;

    //TODO : implement access methods
    int64_t RedChannel    = 1;
    int64_t GreenChannel  = 2;
    int64_t BlueChannel   = 3;
    int64_t BytesPerBandRow = m_LineWidth;


    m_pRedLineBuffer = new Byte[m_LineWidth];
    m_pGreenLineBuffer = new Byte[m_LineWidth];
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
        uint64_t offSetToLine = m_Offset + (pi_PosBlockY * m_pRasterFile->GetTotalRowBytes());
        offSetToLine += m_LineWidth * (RedChannel - 1);
        m_pRasterFile->m_pDoqFile->SeekToPos(offSetToLine);

        if(m_pRasterFile->m_pDoqFile->Read(m_pRedLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        m_pRasterFile->m_pDoqFile->Seek((GreenChannel - RedChannel - 1) * BytesPerBandRow);

        if(m_pRasterFile->m_pDoqFile->Read(m_pGreenLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        m_pRasterFile->m_pDoqFile->Seek((BlueChannel - GreenChannel - 1) * BytesPerBandRow);

        if(m_pRasterFile->m_pDoqFile->Read(m_pBlueLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;
        }
    else
        {
        m_pRasterFile->m_pDoqFile->SeekToEnd();
        m_pRasterFile->m_pDoqFile->Seek(BytesPerBandRow * (- RedChannel));

        if(m_pRasterFile->m_pDoqFile->Read(m_pRedLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        m_pRasterFile->m_pDoqFile->Seek((RedChannel - GreenChannel - 1) * BytesPerBandRow);

        if(m_pRasterFile->m_pDoqFile->Read(m_pGreenLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        m_pRasterFile->m_pDoqFile->Seek((GreenChannel - BlueChannel - 1) * BytesPerBandRow);

        if(m_pRasterFile->m_pDoqFile->Read(m_pBlueLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;
        }

    SisterFileLock.ReleaseKey();

    if (GetResolutionDescriptor()->GetScanlineOrientation().IsLeft())
        {
        for (PixelIndex = 0; PixelIndex < m_LineWidth; PixelIndex++)
            {

            po_pData[BytePos++]= m_pRedLineBuffer  [PixelIndex];
            po_pData[BytePos++]= m_pGreenLineBuffer[PixelIndex];
            po_pData[BytePos++]= m_pBlueLineBuffer [PixelIndex];
            }
        }
    else
        {
        for (PixelIndex = m_LineWidth - 1; PixelIndex >= 0; PixelIndex--)
            {
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
HSTATUS HRFDoqEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                 uint64_t     pi_PosBlockY,
                                 const Byte*  pi_pData,
                                 HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess);
    HPRECONDITION (pi_pData != 0);

    HASSERT(false);


    return H_ERROR;
    }
