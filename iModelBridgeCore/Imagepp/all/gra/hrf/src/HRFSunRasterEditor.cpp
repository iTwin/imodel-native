//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSunRasterEditor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSunRasterLineEditor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFSunRasterFile.h>
#include <Imagepp/all/h/HRFSunRasterEditor.h>

//---------------------------------------------- HRFSunRasterLineEditor

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFSunRasterLineEditor::HRFSunRasterLineEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                               uint32_t              pi_Page,
                                               unsigned short       pi_Resolution,
                                               HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile           = static_cast<HRFSunRasterFile*>(GetRasterFile().GetPtr());

    uint32_t UsedBitsPerRow  = GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits() *
                              (uint32_t)GetResolutionDescriptor()->GetWidth();

    uint32_t BitsPerRow       = UsedBitsPerRow + m_pRasterFile->m_PaddingBitsPerRow;
    m_ExactBytesPerRow      = BitsPerRow / 8;

    m_DataOffset            = HRFSunRasterFile::COLORMAP_OFFSET;
    if (!(m_pRasterFile->m_FileHeader.m_Maptype == HRFSunRasterFile::RMT_NOMAP))
        m_DataOffset += m_pRasterFile->m_FileHeader.m_Maplen;

    m_pLineBuffer           = new Byte[m_ExactBytesPerRow];
    // Init padding bits to 0
    memset(m_pLineBuffer + GetResolutionDescriptor()->GetBytesPerBlockWidth(),
           0, m_pRasterFile->m_PaddingBitsPerRow / 8);

    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFSunRasterLineEditor::~HRFSunRasterLineEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFSunRasterLineEditor::ReadBlock(uint64_t pi_PosBlockX,
                                          uint64_t pi_PosBlockY,
                                          Byte*  po_pData,
                                          HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(po_pData != 0);
    HPRECONDITION(m_AccessMode.m_HasReadAccess);

    HSTATUS Status = H_ERROR;
    HFCLockMonitor SisterFileLock;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        return Status;
        }

    uint64_t offsetToLine;
    offsetToLine = m_DataOffset + (pi_PosBlockY * m_ExactBytesPerRow);

    // Lock the sister file if needed
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    m_pRasterFile->m_pSunRasterFile->SeekToPos(offsetToLine);

    uint32_t DataSize = GetResolutionDescriptor()->GetBytesPerBlockWidth();
    if(m_pRasterFile->m_pSunRasterFile->Read(po_pData, DataSize) != DataSize)
        return Status;    // H_ERROR

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    // Switch reb end blue.
    if(m_pRasterFile->m_IsBGR)
        {
        Byte temporaryValue;
        for(uint32_t PixelIndex = 0; PixelIndex < GetResolutionDescriptor()->GetBytesPerBlockWidth(); PixelIndex+=3)
            {
            temporaryValue         = po_pData[PixelIndex];
            po_pData[PixelIndex]   = po_pData[PixelIndex+2];
            po_pData[PixelIndex+2] = temporaryValue;
            }
        }

    Status = H_SUCCESS;
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS HRFSunRasterLineEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                           uint64_t     pi_PosBlockY,
                                           const Byte*  pi_pData,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_pData != 0);

    HSTATUS Status = H_SUCCESS;
    uint64_t offsetToLine;

    offsetToLine = m_DataOffset + (pi_PosBlockY * m_ExactBytesPerRow);

    memcpy(m_pLineBuffer, pi_pData, GetResolutionDescriptor()->GetBytesPerBlockWidth());

    // Switch reb end blue.
    if(m_pRasterFile->m_IsBGR)
        {
        Byte temporaryValue;
        for(uint32_t PixelIndex = 0; PixelIndex < GetResolutionDescriptor()->GetBytesPerBlockWidth(); PixelIndex+=3)
            {
            temporaryValue              = m_pLineBuffer[PixelIndex];
            m_pLineBuffer[PixelIndex]   = m_pLineBuffer[PixelIndex+2];
            m_pLineBuffer[PixelIndex+2] = temporaryValue;
            }
        }

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    m_pRasterFile->m_pSunRasterFile->SeekToPos(offsetToLine);

    if(m_pRasterFile->m_pSunRasterFile->Write(m_pLineBuffer, m_ExactBytesPerRow) != m_ExactBytesPerRow)
        Status = H_ERROR;

    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return Status;
    }



//---------------------------------------------- HRFSunRasterImageEditor


//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFSunRasterImageEditor::HRFSunRasterImageEditor(HFCPtr<HRFRasterFile> pi_rpRasterFile,
                                                 uint32_t              pi_Page,
                                                 unsigned short       pi_Resolution,
                                                 HFCAccessMode         pi_AccessMode)
    : HRFResolutionEditor(pi_rpRasterFile,
                          pi_Page,
                          pi_Resolution,
                          pi_AccessMode)
    {
    m_pRasterFile = static_cast<HRFSunRasterFile*>(GetRasterFile().GetPtr());
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFSunRasterImageEditor::~HRFSunRasterImageEditor()
    {
    }

//-----------------------------------------------------------------------------
// public
// ReadBlock
// Edition by Block
//-----------------------------------------------------------------------------
// RLE Algorythme
//  If the first byte is not 0x80, the record is one byte long, and
//    contains a pixel value.  Output 1 pixel of that value.
//  If the first byte is 0x80 and the second byte is zero, the record
//    is two bytes long.  Output 1 pixel with value 0x80.
//  If the first byte is 0x80, and the second byte is not zero, the
//    record is three bytes long.  The second byte is a count and the
//    third byte is a value.  Output (count+1) pixels of that value.

HSTATUS HRFSunRasterImageEditor::ReadBlock(uint64_t       pi_PosBlockX,
                                           uint64_t       pi_PosBlockY,
                                           Byte*          po_pData,
                                           HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION(m_AccessMode.m_HasReadAccess);
    HPRECONDITION(po_pData != 0);

    HSTATUS Status = H_ERROR;
    HFCLockMonitor SisterFileLock;
    HArrayAutoPtr<Byte>   pCompressImage;

    if (GetRasterFile()->GetAccessMode().m_HasCreateAccess)
        {
        Status = H_NOT_FOUND;
        return Status;
        }

    uint32_t                ImageLen = m_pRasterFile->m_FileHeader.m_Length;
    pCompressImage = new Byte[ImageLen];

    // Offset Data begin
    uint32_t DataOffset = HRFSunRasterFile::COLORMAP_OFFSET;
    if (!(m_pRasterFile->m_FileHeader.m_Maptype == HRFSunRasterFile::RMT_NOMAP))
        DataOffset += m_pRasterFile->m_FileHeader.m_Maplen;

    // Lock the sister file if needed
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, true);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Read the compressed image in memory
    m_pRasterFile->m_pSunRasterFile->SeekToPos(DataOffset);
    if (m_pRasterFile->m_pSunRasterFile->Read(pCompressImage, ImageLen) != ImageLen)
        return Status;    // H_ERROR

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    Byte* pSrcData = pCompressImage;
    Byte* pDstData = po_pData;
    while (ImageLen--)
        {
        if (*pSrcData == 0x80)
            {
            if (*(pSrcData+1) == 0x00)
                {
                *pDstData = 0x80;
                ++pDstData;
                pSrcData += 2;
                --ImageLen;
                }
            else
                {
                memset(pDstData, *(pSrcData+2), (*(pSrcData+1))+1);
                pDstData += (*(pSrcData+1))+1;
                pSrcData += 3;
                ImageLen -= 2;
                }
            }
        else
            {
            *pDstData = *pSrcData;
            ++pDstData;
            ++pSrcData;
            }
        }

    Status = H_SUCCESS;
    return Status;
    }


//-----------------------------------------------------------------------------
// public
// WriteBlock
// Edition by Block
//-----------------------------------------------------------------------------
HSTATUS  HRFSunRasterImageEditor::WriteBlock(uint64_t     pi_PosBlockX,
                                             uint64_t     pi_PosBlockY,
                                             const Byte* pi_pData,
                                             HFCLockMonitor const* pi_pSisterFileLock)
    {
    HPRECONDITION (m_AccessMode.m_HasWriteAccess || m_AccessMode.m_HasCreateAccess);
    HPRECONDITION (m_pResolutionDescriptor->GetCodec() != 0);

    HSTATUS Status = H_ERROR;

    HArrayAutoPtr<Byte>   pCompressSection;
    uint32_t                SizeMaxCompressSection = 2048;
    pCompressSection = new Byte[SizeMaxCompressSection+4];

    // Lock the sister file if needed
    HFCLockMonitor SisterFileLock;
    if(pi_pSisterFileLock == 0)
        {
        // Get lock and synch.
        AssignRasterFileLock(GetRasterFile(), SisterFileLock, false);
        pi_pSisterFileLock = &SisterFileLock;
        }

    // Offset Data begin
    uint32_t DataOffset            = HRFSunRasterFile::COLORMAP_OFFSET;
    if (!(m_pRasterFile->m_FileHeader.m_Maptype == HRFSunRasterFile::RMT_NOMAP))
        DataOffset += m_pRasterFile->m_FileHeader.m_Maplen;
    m_pRasterFile->m_pSunRasterFile->SeekToPos(DataOffset);

    uint32_t ImageSizeInByte = ((GetResolutionDescriptor()->GetPixelType()->CountPixelRawDataBits() *
                               (uint32_t)GetResolutionDescriptor()->GetWidth()) / 8) *
                             (uint32_t)GetResolutionDescriptor()->GetHeight();

    Byte* pDstData                = pCompressSection;
    uint32_t CompressedSize          = 0;
    uint32_t CurCompressSizeSection  = 0;
    while (ImageSizeInByte--)
        {
        if (*pi_pData == 0x80)
            {
            *pDstData               = 0x80;
            *(pDstData+1)           = 0x00;
            CurCompressSizeSection  += 2;
            pDstData                += 2;
            ++pi_pData;
            }
        else if (*pi_pData == *(pi_pData+1))
            {
            Byte  Count = 0;
            for(Count=0;
                (*pi_pData == *(pi_pData+1)) && (Count<255) && (ImageSizeInByte>0);
                ++pi_pData, ++Count, --ImageSizeInByte);

            *pDstData               = 0x80;
            *(pDstData+1)           = Count;
            *(pDstData+2)           = *pi_pData;
            CurCompressSizeSection  += 3;
            pDstData                += 3;
            ++pi_pData;
            }
        else
            {
            *pDstData = *pi_pData;
            ++CurCompressSizeSection;
            ++pDstData;
            ++pi_pData;
            }

        // Buffer full, write it
        if (CurCompressSizeSection >= SizeMaxCompressSection)
            {
            if (m_pRasterFile->m_pSunRasterFile->Write(pCompressSection, CurCompressSizeSection) != CurCompressSizeSection)
                return Status;    // H_ERROR

            CompressedSize          += CurCompressSizeSection;
            CurCompressSizeSection  = 0;
            pDstData                = pCompressSection;
            }
        }

    // Write the buffer if not empty
    if (CurCompressSizeSection > 0)
        {
        if (m_pRasterFile->m_pSunRasterFile->Write(pCompressSection, CurCompressSizeSection) != CurCompressSizeSection)
            return Status;    // H_ERROR

        CompressedSize          += CurCompressSizeSection;
        }

    // Increment the counters
    GetRasterFile()->SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    m_pRasterFile->m_FileHeader.m_Length = CompressedSize;
    m_pRasterFile->m_HeaderChanged       = true;

    Status = H_SUCCESS;
    return Status;
    }

