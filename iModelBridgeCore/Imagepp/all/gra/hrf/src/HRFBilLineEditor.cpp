//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBilLineEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFBilLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFBilLineEditor.h>
#include <Imagepp/all/h/HRFBilFile.h>
#include <Imagepp/all/h/HTIFFUtils.h>

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------

HRFBilLineEditor::HRFBilLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                   uint32_t              pi_Page,
                                   unsigned short       pi_Resolution,
                                   HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_Offset        = GetBilRasterFile()->GetHeaderLength();
    m_LineWidth     = GetBilRasterFile()->GetBandRowBytes();
    m_nNbChannel    = GetResolutionDescriptor()->GetPixelType()->GetChannelOrg().CountChannels();
    m_nbBitsPerBandPerPixel = GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits() / m_nNbChannel;
    m_pRedLineBuffer    = new Byte[m_LineWidth];

    if ( 3 <= m_nNbChannel)
        {
        m_pGreenLineBuffer  = new Byte[m_LineWidth];
        m_pBlueLineBuffer   = new Byte[m_LineWidth];
        }
    else
        {
        m_pGreenLineBuffer = 0;
        m_pBlueLineBuffer  = 0;
        }
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------

HRFBilLineEditor::~HRFBilLineEditor()
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

HSTATUS HRFBilLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                    uint64_t pi_PosBlockY,
                                    Byte* po_pData,
                                    HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_SUCCESS;

    // Temporary need for virtual ptr.
    if (!GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        if (3 <= m_nNbChannel)
            {
            if (8 == m_nbBitsPerBandPerPixel)
                Status = Read24BitRgbBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
            else if (16 == m_nbBitsPerBandPerPixel)
                Status = Read48BitRgbBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
            }
        else
            {
            if (8 == m_nbBitsPerBandPerPixel)
                Status = Read8BitGrayBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
            else if (16 == m_nbBitsPerBandPerPixel)
                Status = Read16BitGrayBlock(pi_PosBlockX, pi_PosBlockY, po_pData, pi_pSisterFileLock);
            }
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

HSTATUS HRFBilLineEditor::Read24BitRgbBlock(uint64_t pi_PosBlockX,
                                            uint64_t pi_PosBlockY,
                                            Byte* po_pData,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    uint32_t PixelIndex, BytePos = 0;

    int64_t RedChannel    = GetBilRasterFile()->GetRedChannel();
    int64_t GreenChannel  = GetBilRasterFile()->GetGreenChannel();
    int64_t BlueChannel   = GetBilRasterFile()->GetBlueChannel();
    int64_t BytesPerBandRow = m_LineWidth;

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
        uint64_t offSetToLine = m_Offset + (pi_PosBlockY * GetBilRasterFile()->GetTotalRowBytes());
        offSetToLine += m_LineWidth * (RedChannel - 1);
        GetBilRasterFile()->m_pBilFile->SeekToPos(offSetToLine);

        if(GetBilRasterFile()->m_pBilFile->Read(m_pRedLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        GetBilRasterFile()->m_pBilFile->Seek((GreenChannel - RedChannel - 1) * BytesPerBandRow);

        if(GetBilRasterFile()->m_pBilFile->Read(m_pGreenLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        GetBilRasterFile()->m_pBilFile->Seek((BlueChannel - GreenChannel - 1) * BytesPerBandRow);

        if(GetBilRasterFile()->m_pBilFile->Read(m_pBlueLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        }
    else
        {
        GetBilRasterFile()->m_pBilFile->SeekToEnd();
        GetBilRasterFile()->m_pBilFile->Seek(BytesPerBandRow * (- RedChannel));

        if(GetBilRasterFile()->m_pBilFile->Read(m_pRedLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        GetBilRasterFile()->m_pBilFile->Seek((RedChannel - GreenChannel - 1) * BytesPerBandRow);

        if(GetBilRasterFile()->m_pBilFile->Read(m_pGreenLineBuffer, m_LineWidth) != m_LineWidth)
            goto WRAPUP;

        GetBilRasterFile()->m_pBilFile->Seek((GreenChannel - BlueChannel - 1) * BytesPerBandRow);

        if(GetBilRasterFile()->m_pBilFile->Read(m_pBlueLineBuffer, m_LineWidth) != m_LineWidth)
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
// protected
// Read uncompressed 48 bit RGB Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFBilLineEditor::Read48BitRgbBlock(uint64_t pi_PosBlockX,
                                            uint64_t pi_PosBlockY,
                                            Byte* po_pData,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HASSERT(!"Read48BitRgbBlock not supported");
    // The code below is not good. Nothing get copied over to the output buffer(po_pData).
    // Probably never worked. We'll fix it after a user complain.
    return H_ERROR;

#if 0
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(GetResolutionDescriptor()->GetScanlineOrientation().IsUpper());
    HPRECONDITION(GetResolutionDescriptor()->GetScanlineOrientation().IsLeft());

    HSTATUS Status = H_ERROR;
    int64_t RedChannel    = GetBilRasterFile()->GetRedChannel();
    int64_t GreenChannel  = GetBilRasterFile()->GetGreenChannel();
    int64_t BlueChannel   = GetBilRasterFile()->GetBlueChannel();
    int64_t BytesPerBandRow = m_LineWidth;

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

    uint64_t offSetToLine = m_Offset + (pi_PosBlockY * GetBilRasterFile()->GetTotalRowBytes());
    offSetToLine += m_LineWidth * (RedChannel - 1);
    GetBilRasterFile()->m_pBilFile->SeekToPos(offSetToLine);

    if(GetBilRasterFile()->m_pBilFile->Read(m_pRedLineBuffer, m_LineWidth) != m_LineWidth)
        goto WRAPUP;

    GetBilRasterFile()->m_pBilFile->Seek((GreenChannel - RedChannel - 1) * BytesPerBandRow);

    if(GetBilRasterFile()->m_pBilFile->Read(m_pGreenLineBuffer, m_LineWidth) != m_LineWidth)
        goto WRAPUP;

    GetBilRasterFile()->m_pBilFile->Seek((BlueChannel - GreenChannel - 1) * BytesPerBandRow);

    if(GetBilRasterFile()->m_pBilFile->Read(m_pBlueLineBuffer, m_LineWidth) != m_LineWidth)
        goto WRAPUP;

    SisterFileLock.ReleaseKey();

    unsigned short* pRedBuffer = (unsigned short*)m_pRedLineBuffer;
    unsigned short* pGreenBuffer = (unsigned short*)m_pGreenLineBuffer;
    unsigned short* pBlueBuffer = (unsigned short*)m_pBlueLineBuffer;
    uint32_t PixelCount = m_LineWidth / 2;

    if (GetBilRasterFile()->IsMsByteFirst())
        {
        // Big Endian
        for(uint32_t i(0); i < PixelCount; ++i)
            {
            pRedBuffer[i]   = SWAPBYTE_SHORT(pRedBuffer[i]);
            pGreenBuffer[i] = SWAPBYTE_SHORT(pGreenBuffer[i]);
            pBlueBuffer[i]  = SWAPBYTE_SHORT(pBlueBuffer[i]);
            }
        }
    else
        {
        // Little Endian
        for(uint32_t i(0); i < PixelCount; ++i)
            {
            pRedBuffer[i]   = pRedBuffer[i];
            pGreenBuffer[i] = pGreenBuffer[i];
            pBlueBuffer[i]  = pBlueBuffer[i];
            }
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
#endif
    }

//-----------------------------------------------------------------------------
// protected
// Read uncompressed 8 bit Grayscale Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFBilLineEditor::Read8BitGrayBlock(uint64_t pi_PosBlockX,
                                            uint64_t pi_PosBlockY,
                                            Byte* po_pData,
                                            HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS         Status = H_ERROR;
    uint32_t        PixelIndex, BytePos = m_LineWidth - 1;

    int64_t GrayChannel    = GetBilRasterFile()->GetRedChannel();
    int64_t BytesPerBandRow = m_LineWidth;

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
        uint64_t offSetToLine = m_Offset + (pi_PosBlockY * GetBilRasterFile()->GetTotalRowBytes());
        offSetToLine += m_LineWidth * (GrayChannel - 1);
        GetBilRasterFile()->m_pBilFile->SeekToPos(offSetToLine);
        }
    else
        {
        GetBilRasterFile()->m_pBilFile->SeekToEnd();
        GetBilRasterFile()->m_pBilFile->Seek(BytesPerBandRow * (- GrayChannel));
        }

    if (GetBilRasterFile()->m_pBilFile->Read(po_pData, m_LineWidth) != m_LineWidth)
        goto WRAPUP;    // H_ERROR

    SisterFileLock.ReleaseKey();

    if (GetResolutionDescriptor()->GetScanlineOrientation().IsRight())
        {
        for (PixelIndex = 0; PixelIndex < m_LineWidth / 2; ++PixelIndex)
            {
            Byte temp = po_pData[PixelIndex];
            po_pData[PixelIndex] = po_pData[BytePos];
            po_pData[BytePos --] = temp;
            }
        }

    Status = H_SUCCESS;

WRAPUP:
    return Status;
    }

//-----------------------------------------------------------------------------
// protected
// Read uncompressed 16 bit Grayscale Block
// Edition by Block
//-----------------------------------------------------------------------------

HSTATUS HRFBilLineEditor::Read16BitGrayBlock(uint64_t pi_PosBlockX,
                                             uint64_t pi_PosBlockY,
                                             Byte* po_pData,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(GetResolutionDescriptor()->GetScanlineOrientation().IsUpper());
    HPRECONDITION(GetResolutionDescriptor()->GetScanlineOrientation().IsLeft());
    HPRECONDITION(m_LineWidth % 2 == 0);

    HSTATUS Status = H_ERROR;
    int64_t GrayChannel     = GetBilRasterFile()->GetRedChannel();
    int64_t BytesPerBandRow = m_LineWidth;

    // Lock the sister file
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Lock the file.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    uint64_t offSetToLine = m_Offset + (pi_PosBlockY * GetBilRasterFile()->GetTotalRowBytes());
    offSetToLine += BytesPerBandRow * (GrayChannel - 1);
    GetBilRasterFile()->m_pBilFile->SeekToPos(offSetToLine);

    if(GetBilRasterFile()->m_pBilFile->Read(po_pData, m_LineWidth) != m_LineWidth)
        goto WRAPUP;    // H_ERROR;

    SisterFileLock.ReleaseKey();

    if (GetBilRasterFile()->IsMsByteFirst())
        {
        unsigned short* pBuffer = (unsigned short*)po_pData;
        uint32_t PixelCount = m_LineWidth / 2;

        // Big endian
        for(uint32_t i(0); i < PixelCount; ++i)
            {
            // convert to little endian
            pBuffer[i] = SWAPBYTE_SHORT(pBuffer[i]);
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
HSTATUS HRFBilLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                     uint64_t     pi_PosBlockY,
                                     const Byte* pi_pData,
                                     HFCLockMonitor const* pi_pSisterFileLock)
    {
    HASSERT(0); // not supported

    return H_ERROR;
    }

//-----------------------------------------------------------------------------
// private
//-----------------------------------------------------------------------------
HFCPtr<HRFBilFile>  HRFBilLineEditor::GetBilRasterFile()
    {
    HPRECONDITION(GetRasterFile()->IsCompatibleWith(HRFBilFile::CLASS_ID));
    return static_cast<HRFBilFile*>(GetRasterFile().GetPtr());
    }